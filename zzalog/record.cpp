/*
ZZALOG - Amateur radio log
© - Copyright 2018, Philip Rose, GM3ZZA
All Rights Reserved

record.cpp - Individual record data item: implementation file
*/

#include "record.h"
#include "../zzalib/utils.h"
#include "pfx_data.h"
#include "spec_data.h"
#include "../zzalib/rig_if.h"
#include "status.h"
#include "view.h"
#include "../zzalib/formats.h"
#include "book.h"

#include <ctime>
#include <chrono>
#include <ratio>
#include <cmath>

#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_utf8.h>

using namespace std;
using namespace zzalog;
using namespace zzalib;

extern pfx_data* pfx_data_;
extern spec_data* spec_data_;
extern Fl_Preferences* settings_;
extern rig_if* rig_if_;
extern status* status_;

// initialise the static variables
bool record::expecting_header_ = true;
bool record::inhibit_error_reporting_ = false;

// Comparison operator - compares QSO_DATE and TIME_ON - orders the records by time.
bool record::operator > (record& them) {
	// Basic string comparison "YYYYMMDDHHMMSS"
	string my_time = item("QSO_DATE") + item("TIME_ON");
	string their_time = them.item("QSO_DATE") + them.item("TIME_ON");
	if (my_time > their_time) {
		return true;
	}
	else {
		return false;
	}
}

// Default constructor
record::record()
{
	delete_contents();
}

// Copy constructor
record::record(const record& rhs) {
	// Uses assignment operator
	*this = rhs;
}

// Assignment operator
record& record::operator= (const record& rhs) {
	// Copy if they are different - else do nothing
	if (this != &rhs) {
		// delete any existing contents
		this->delete_contents();
		// copy over member variables
		this->is_header_ = rhs.is_header_;
		this->header_comment_ = rhs.header_comment_;
		// Copy over the mapped items
		for (auto iter = rhs.begin(); iter != rhs.end(); iter++) {
			string field = iter->first;
			string value = iter->second;
			item(field, value);
		}
	}
	return *this;
}

// Destructor
record::~record() {
	delete_contents();
}

// Delete all the contents
void record::delete_contents() {
	is_header_ = false;
	header_comment_ = "";
	is_incomplete_ = false;
	is_dirty_ = false;
	// Delete items
	clear();
}

// Set an item pair.
// Return true if successful, false if not
void record::item(string field, string value, bool formatted/* = false*/, bool dirty /*=true*/) {
	// Check we are not deleting an important field - crash the program if this was unintentional
	char message[256];
	snprintf(message, 256, "You are deleting %s, are you sure", field.c_str());
	if ((field == "CALL" || field == "QSO_DATE" || field == "QSO_DATE_OFF" || field == "TIME_ON" ||
		field == "TIME_OFF" || field == "FREQ" || field == "MODE") &&
		item(field).length() && !value.length() &&
		fl_choice(message, "Yes", "No", nullptr) == 1) {
		snprintf(message, 256, "Unexpected deletion of the field, %s", field.c_str());
		status_->misc_status(ST_FATAL, message);
		return;
	}
	// Certain fields - always log in upper case
	string upper_value;
	if (field == "CALL" ||
		field == "CONT" ||
		field == "MY_CALL" ||
		field == "GRIDSQUARE" ||
		field == "MY_GRIDSQUARE" ||
		field == "OPERATOR" ||
		field == "OWNER_CALLSIGN" ||
		field == "STATION_CALLSIGN" ||
		field == "IOTA" ||
		field == "MY_IOTA" ||
		field == "COUNTRY" ||
		field == "MY_COUNTRY" ||
		field == "APP_ZZA_PFX") {
		// Force upper case for these fields
		upper_value = to_upper(value);
	}
	else {
		if (field[0] == '!') {
			// Special field names used for temporary values
			upper_value = value;
		}
		else {
			// Get the type of data. Different processing for different types
			string datatype = spec_data_->datatype(field);
			if (datatype == "PositiveInteger") {
				// Always strip off leading zeros
				size_t dummy;
				int int_value;
				try {
					int_value = stoi(value, &dummy);
					if (dummy == value.length()) {
						// The whole string is an integer - convert it back to string
						upper_value = to_string(int_value);
					}
					else {
						// Use the original string
						upper_value = value;
					}
				}
				catch (invalid_argument&) {
					// Empty string, so use that
					upper_value = value;
				}
			}
			else if (datatype == "Enumeration") {
				// Treat all enumerations as upper case. ADIF can accept either
				upper_value = to_upper(value);
			}
			else if (datatype == "Date") {
				// Do not convert to upper case to preserve lower-case month names
				upper_value = value;
			} else {
				// Default leave as is
				upper_value = value;
			}
		}
	}
	string formatted_value;
	if (formatted) {
		// Convert from the displayed format to ADIF format
		if (upper_value == "") {
			// empty string gives empty string
			formatted_value = "";
		}
		else {
			char type_indicator = spec_data_->datatype_indicator(field);
			double as_d = 0.0;
			int as_i = 0;
			Fl_Preferences display_settings(settings_, "Display");
			switch (type_indicator) {
			case 'D': {
				// Date type
				display_date_t format;
				display_settings.get("Date", (int&)format, DATE_YYYYMMDD);
				formatted_value = unformat_date(format, upper_value);
				break;
			}
			case 'T': {
				bool ok = false;
				// Time type
				display_time_t format;
				display_settings.get("Time", (int&)format, TIME_HHMMSS);
				formatted_value = unformat_time(format, upper_value);
				break;
				break;
			}
			case 'N':
			case 'S':
			case 'I':
			case 'M': 
			case 'G':
			case 'B':
				if (field == "GRIDSQUARE" || field == "MY_GRIDSQUARE") {
					if (upper_value.length() > 8) {
						// GRIDSQUARE limited to first 8 characters
						item(field + "_EXT", upper_value.substr(8), formatted);
						formatted_value = upper_value.substr(0, 8);
					}
				}
				else if (field == "FREQ" || field == "FREQ_RX") {
					display_freq_t format;
					display_settings.get("Frequency", (int&)format, FREQ_MHz);
					formatted_value = unformat_freq(format, upper_value);
				}
				else {
					// No formatting
					formatted_value = upper_value;
				}
				break;
			case 'E':
				// Enumeration: convert to uppercase
				formatted_value = to_upper(upper_value);
				if (field == "MODE") {
					if (spec_data_->is_submode(formatted_value)) {
						// Set submode to this value and the mode to its parent mode
						item("SUBMODE", formatted_value, formatted);
						formatted_value = spec_data_->mode_for_submode(formatted_value);
					}
					else {
						// set submode to ""
						item("SUBMODE", string(""));
					}
				}
				break;
			case 'L':
				// Convert signed decimal degree to ADIF spec LAT or LON value.
				as_d = stod(upper_value);
				char c;
				if (as_d < 0.0) {
					// Negative - West for LON, South for LAT. Covert degrees to positive number
					as_d = -as_d;
					if (field == "LON" || field == "MY_LON") {
						c = 'W';
					}
					else {
						c = 'S';
					}
				}
				else {
					// Positive - East for LON, North for LAT
					if (field == "LON" || field == "MY_LON") {
						c = 'E';
					}
					else {
						c = 'N';
					}
				}
				// Get integer part of number
				as_i = (int)as_d;
				// Convert decimal degree to minutes
				as_d -= (double)as_i;
				as_d *= 60.0;
				// Convert to [ENSW]ddd mm.mmm
				char as_s[12];
				snprintf(as_s, 12, "%c%03d %2.3f", c, as_i, as_d);
				formatted_value = as_s;
			default:
				formatted_value = upper_value;
				break;
			}
		}
	}
	else {
	 // Use the data directly in the item
		formatted_value = upper_value;
	}
	// SEt dirty flag if contents are changing
	if (dirty) {
		string orig_value;
		if (find(field) != end()) {
			orig_value = at(field);
		}
		else {
			orig_value = "";
		}
		if (orig_value != formatted_value) {
			is_dirty_ = true;
		}
	}
	// Set in item
	(*this)[field] = formatted_value;
}

// Get an item - as string
string record::item(string field, bool formatted/* = false*/, bool indirect/* = false*/) {
	string result;
	if (indirect) {
		// Get the formatted version if exists
		result = item(field, formatted);
		// Get the item value based on APP_ZZA_QTH if it doesn't exist
		if (result == "") {
			// Not set in the item
			char * temp;
			// If station location details fetch them the settings.
			int station_id = -1;
			string qth_name;
			// Read APP_ZZA_QTH field and get the settings for that value 
			Fl_Preferences stations_settings(settings_, "Stations");
			Fl_Preferences qths_settings(stations_settings, "QTHs");
			qth_name = item("APP_ZZA_QTH", false, false);
			Fl_Preferences qth_settings(qths_settings, qth_name.c_str());
			if (field == "MY_NAME") {
				// Get operator's name
				qth_settings.get("Operator Name", temp, "John Doe");
				result = temp;
				free(temp);
			}
			else if (field == "MY_STREET") {
				// Get operator'sstreet  address
				qth_settings.get("Street", temp, "99 Any Street");
				result = temp;
				free(temp);
			}
			else if (field == "MY_CITY") {
				// Get operator's home town
				qth_settings.get("Town", temp, "Any Town");
				result = temp;
				free(temp);
			}
			else if (field == "MY_CNTY") {
				// Get operator's county
				qth_settings.get("County", temp, "nowhereshire");
				result = temp;
				free(temp);
			}
			else if (field == "MY_COUNTRY") {
				// Get operator's country - from the DXCC Entity Code
				int entity_id;
				qth_settings.get("DXCC Id", entity_id, 0);
				spec_dataset* entities = spec_data_->dataset("DXCC_Entity_Code");
				map<string, string>* entity_data = entities->data.at(to_string(entity_id));
				result = entity_data->at("Entity Name");
			}
			else if (field == "MY_POSTAL_CODE") {
				// Get operator's post-code
				qth_settings.get("Post Code", temp, "XX1 1XX");
				result = temp;
				free(temp);
			}
			else if (field == "MY_CONT") {
				// Get operator's continent
				qth_settings.get("Continent", temp, "XX");
				result = temp;
				free(temp);
			}
			else if (field == "MY_GRIDSQUARE") {
				// Get operator's gridsquare
				qth_settings.get("Locator", temp, "RR73TU");
				if (strlen(temp) > 8) {
					result = string(temp, 8);
				}
				else {
					result = temp;
				}
				free(temp);
			}
			else if (field == "MY_GRIDSQUARE_EXT") {
				// Get operator's gridsquare extenion
				qth_settings.get("Locator", temp, "RR73TU");
				if (strlen(temp) > 8) {
					result = temp + 8;
				}
				else {
					result = "";
				}
			}
			else if (field == "MY_DXCC") {
				// Get operator's DXCC entity code
				int entity_id;
				qth_settings.get("DXCC", entity_id, 0);
				result = to_string(entity_id);
				result = "";
			}
			else if (field == "MY_CQ_ZONE") {
				// Get operator's CQ Zone
				int zone_id;
				qth_settings.get("CQ Zone", zone_id, 0);
				result = to_string(zone_id);
			}
			else if (field == "MY_ITU_ZONE") {
				// Get operator's ITU Zone
				int zone_id;
				qth_settings.get("ITU Zone", zone_id, 0);
				result = to_string(zone_id);
			}
			else if (field == "MY_IOTA") {
				// Get operator's IOTA number
				qth_settings.get("IOTA", temp, "XX-001");
				result = temp;
				free(temp);
			}
			else {
				// Otherwise it is still the empty string
				result = "";
			}
		}
	}
	else if (formatted) {
		// Return the display format for the field
		string unformatted_value = item(field);
		// Convert empty string to empty string
		if (unformatted_value == "") {
			result = unformatted_value;
		}
		else {
			// Get display settings
			Fl_Preferences display_settings(settings_, "Display");

			// Get the type indicator of the field
			char type_indicator = spec_data_->datatype_indicator(field);
			double as_d = 0.0;
			char as_c[15];
			switch (type_indicator) {
			case 'D': {
				display_date_t format;
				display_settings.get("Date", (int&)format, DATE_YYYYMMDD);
				result = format_date(format, unformatted_value);
				break;
			}
			case 'T': {
				display_time_t format;
				display_settings.get("Time", (int&)format, TIME_HHMMSS);
				result = format_time(format, unformatted_value);
				break;
			}
			case 'N':
			case ' ':
			case 'S':
			case 'I':
			case 'M':
			case 'G':
			case 'B':
				if (field == "GRIDSQUARE" || field == "MY_GRIDSQUARE") {
					// Concatenate base and extension fields
					if (unformatted_value.length() < 8) {
						result = unformatted_value;
					} 
					else if (item_exists(field + "_EXT")) {
						result = unformatted_value + item(field + "_EXT", formatted, indirect);
					}
				}
				else if (field == "FREQ" || field == "FREQ_RX") {
					display_freq_t format;
					display_settings.get("Frequency", (int&)format, FREQ_MHz);
					result = format_freq(format, unformatted_value);
				}
				else {
					// No formatting
					result = unformatted_value;
				}
				break;
			case 'E':
				// Special case for MODE - use SUBMODE if it exists
				if (field == "MODE" && item_exists("SUBMODE")) {
					result = item("SUBMODE", formatted, indirect);
				}
				// No other formatting
				else {
					result = unformatted_value;
				}
				break;
			case 'L':
				// LAT/LON value return signed decimal degree
				as_d = stod(unformatted_value.substr(1, 3));
				as_d += (stod(unformatted_value.substr(5)) / 60.0);
				switch (unformatted_value[0]) {
				case 'W':
				case 'S':
					as_d = -as_d;
				}
				snprintf(as_c, 15, "%g", as_d);
				result = string(as_c);
			default:
				// If any other data indicator gets returned - it shouldn't
				result = unformatted_value;
				break;
			}
		}
	}
	else {
		// Use the field directly
		auto it = find(field);
		if (it == end()) {
			// Field not present return empty string
			result = "";
		}
		else {
			// Return field value
			result = it->second;
		}
	}
	return result;
}

// get an item - as an integer, default 0
void record::item(string field, int& value) {
	if (item_exists(field)) {
		try {
			// Return integer value
			value = stoi(item(field));
		}
		catch (invalid_argument&) {
			// Not a valid integer
			value = 0;
		}
	}
	else {
		// Field not present
		value = 0;
	}
}

// get an item - as a double, default "not-a-number"
void record::item(string field, double& value) {
	if (item_exists(field)) {
		try {
			// return double value
			value = stod(item(field));
		}
		catch (invalid_argument&) {
			// If it's not a valid decimal it may be in LAT/LON format
			if (field == "LAT" || field == "LON" || field == "MY_LAT" || field == "MY_LON") {
				// Get formatted version
				string item_value = item(field, true, true);
				try {
					value = stod(item_value);
				}
				catch (invalid_argument&) {
					value = nan("");
				}
			}
			else {
				// Not a valid double
				value = nan("");
			}
		}
	}
	else {
		// Field not present
		value = nan("");
	}

}

// is the QSO valid - has a minimum subset of fields - QSO start time, call and frequency or band
bool record::is_valid() {
	if (item_exists("QSO_DATE") &&
		item_exists("TIME_ON") &&
		item_exists("CALL")  &&
		(item_exists("FREQ") || item_exists("BAND"))) {
		return true;
	}
	else {
		return false;
	}
}

// does the item exist - in the map and not an empty string
bool record::item_exists(string field) {
	return find(field) != end() && at(field) != "";
}

// set the header information
void record::header(string comment) {
	is_header_ = true;
	header_comment_ = comment;
}

// get the header information
string record::header() {
	return header_comment_;
}

// return the is_header flag
bool record::is_header() {
	return this->is_header_;
}

// Delete all parsed fields
void record::unparse() {
	// Band - only if frequency is there
	if (item_exists("FREQ")) item("BAND", string(""));
	if (item_exists("FREQ_RX")) item("BAND_RX", string(""));
	// Geography fields - based on parsing the callsign
	item("DXCC", string(""));
	item("COUNTRY", string(""));
	item("CONT", string(""));
	item("CQZ", string(""));
	item("ITUZ", string(""));
	item("APP_ZZA_PFX", string(""));
	// bearing and distance - based on home location and DX location (from record or prefix)
	item("ANT_AZ", string(""));
	item("DISTANCE", string(""));
	char message[100];
	sprintf(message,"LOG: %s %s %s - parsed data removed",
		item("QSO_DATE").c_str(), item("TIME_ON").c_str(), item("CALL").c_str());
	status_->misc_status(ST_LOG, message);
}

// Return longitude and latitude
lat_long_t record::location(bool my_station) {
	location_t dummy;
	return location(my_station, dummy);
}

// Return longitude and latitude from the record
lat_long_t record::location(bool my_station, location_t& source) {
	// Set a bad coordinate
	lat_long_t lat_long = { nan(""), nan("") };

	// now look at the various sources of lat/lon
	// By preference use LAT/LON
	string value_1;
	string value_2;
	// Use LAT/LON fields
	if (my_station) {
		item("MY_LAT", lat_long.latitude);
		item("MY_LON", lat_long.longitude);
	}
	else {
		item("LAT", lat_long.latitude);
		item("LON", lat_long.longitude);
	}
	if (isnan(lat_long.latitude) || isnan(lat_long.longitude)) {
		// Use either Gridsquare or prefix centre if that is more accurate
		if (my_station) {
			value_1 = item("MY_GRIDSQUARE", false, true);
			// No gridsquare for user. 
			if (value_1.length() == 0) {
				return lat_long;
			}
		}
		else {
			value_1 = item("GRIDSQUARE");
		}
		// Get the lat/long of the centre of the grid-square 
		lat_long = grid_to_latlong(value_1);
		// 2 or 4 character grid square - use centre of grid square
		if (value_1.length() <= 4) {
			// Get prefix coordinates
			lat_long_t prefix_lat_long = { nan(""), nan("") };
			string prefix_code = item("APP_ZZA_PFX");
			if (prefix_code != "") {
				prefix* prefix = pfx_data_->get_prefix(prefix_code);
				if (prefix != nullptr) {
					prefix_lat_long.latitude = prefix->latitude_;
					prefix_lat_long.longitude = prefix->longitude_;
				}
			}
			if (value_1.length()) {
				double side = 0.0;
				switch (value_1.length()) {
				case 2:
					side = 18.0;
					break;
				case 4:
					side = 1.0;
					break;
				}
			}
			else {
				lat_long = prefix_lat_long;
			}
		}
		switch (value_1.length()) {
		case 0:
		case 1:
			source = LOC_NONE;
			break;
		case 2:
		case 3:
			source = LOC_GRID2;
			break;
		case 4:
		case 5:
			source = LOC_GRID4;
			break;
		case 6:
		case 7:
			source = LOC_GRID6;
				break;
		default:
			source = LOC_GRID8;
			break;
		}
	}
	else {
		source = LOC_LATLONG;
	}
	return lat_long;
}

// Look up band in ADIF specification database - returns true if the record was changed
bool record::update_band(bool force /*=false*/) {
	// Get band from frequency
	if (item("FREQ") == "") {
		return false;
	}
	else {
		bool updated = false;
		// Update BAND from FREQ
		if (force || item("BAND") == "") {
			// Get frequency
			double frequency;
			item("FREQ", frequency);
			// Look it up
			string band = spec_data_->band_for_freq(frequency);
			item("BAND", band);
			updated = true;
		}
		// Update BAND_RX from FREQ_RX if that has been logged
		if (force || item("BAND_RX") == "") {
			// Get frequency
			double frequency;
			item("FREQ_RX", frequency);
			// If it exists - look up the band
			if (!isnan(frequency)) {
				string band = spec_data_->band_for_freq(frequency);
				item("BAND_RX", band);
				updated = true;
			}
			else {
				if (item("BAND_RX") != "") {
					item("BAND_RX", string(""), true);
					updated = true;
				}
			}
		}
		return updated;
	}

}

// Merge fields from another record
bool record::merge_records(record* record, bool allow_loc_mismatch /* = false */, hint_t* result /*= nullptr*/)
{
	bool merged = false;
	bool bearing_change = false;
	// For each field in other record
	for (auto it = record->begin(); it != record->end(); it++) {
		string field_name = it->first;
		string merge_data = it->second;
		// Get my value for the field
		string my_data = item(field_name);
		bool item_match = items_match(record, field_name);
		bool is_location = false;
		bool is_grid_4_to_6 = false;
		bool is_qsl_rcvd = false;
		// Select on field name
		if (field_name == "CNTY" ||
			field_name == "CONT" ||
			field_name == "COUNTRY" ||
			field_name == "CQZ" ||
			field_name == "DXCC" ||
			field_name == "IOTA" ||
			field_name == "ITUZ" ||
			field_name == "PFX" ||
			field_name == "STATE") {
			// Geographic location field
			is_location = true;
			if (allow_loc_mismatch) {
				bearing_change = true;
			}
		}
		else if (field_name == "GRIDSQUARE") {
			// Gridsquare: location field - RR73 is an almost fictitious location 
			// Actually it's to the north of the Bering Straits, but can be captured if
			// RR73 is used in JT exchanges
			if (merge_data != "RR73" || 
				fl_choice("Location in record is RR73 - please confirm?", fl_ok, fl_cancel, "") == 0) {
				// Grid is good (RR73 confirmed by user)
				is_location = true;
				// Updating a 4-character grid with a 6-character one and the first 4 agree
				if (my_data.length() < merge_data.length() && merge_data.substr(0, my_data.length()) == my_data) {
					is_grid_4_to_6 = true;
					bearing_change = true;
				}
			}
		}
		else if (field_name == "EQSL_QSL_RCVD" ||
			field_name == "QSL_RCVD") {
			// QSL received - need to replace N with Y
			is_qsl_rcvd = true;
		}
		else if (field_name == "LOTW_QSL_RCVD") {
			// Change club log upload status
			if (item("CLUBLOG_QSO_UPLOAD_STATUS") == "Y") {
				item("CLUBLOG_QSO_UPLOAD_STATUS", string("M"));
				merged = true;
			}
			is_qsl_rcvd = true;
		}
		else if (field_name == "TIME_OFF" ||
			field_name == "QSO_DATE_OFF") {
			// Update from the other record - erstwhile bug overwrote previous import
			if (!item_match) {
				item(field_name, merge_data);
				merged = true;
			}
			// Match but merge data is more accurate
			else if (field_name == "TIME_OFF" && merge_data.length() > my_data.length()) {
				item(field_name, merge_data);
				merged = true;
			}
		}
		else if (item_match  && (field_name == "TIME_ON" || field_name == "FREQ" || field_name == "FREQ_RX") && merge_data.length() > my_data.length()) {
			// Merge data is more accurate
			item(field_name, merge_data);
			merged = true;
		}
		else {
			// All other fields
			is_location = false;
		}
		// Merge if current field is "", is a location field being updated by LotW or 
		// is updating from 4-char gridsquare to 6-char or QSL received
		if (my_data == "" || (is_location && allow_loc_mismatch) || is_grid_4_to_6 || is_qsl_rcvd) {
			// Can safely merge - use un-case-converted
			item(field_name, merge_data);
			merged = true;
		}
	}
	// Recalculate bearing and distance as location has changed
	if (bearing_change) {
		pfx_data_->update_bearing(this);
	}
	// If the location has changed we want to redraw DxAtlas, otherwise we don't.
	if (result) {
		if (bearing_change) {
			*result = hint_t::HT_CHANGED;
		} {
			*result = hint_t::HT_MINOR_CHANGE;
		}
	}
	return merged;
}

// Change a field name
void record::change_field_name(string from, string to) {
	// Set the item changing it to
	item(to, item(from));
	// Clear the existing one
	item(from, string(""));
}

// records are potential duplicates - returns result
	//MT_NOMATCH,          // No matching record found in log
	//MT_EXACT,            // An exact match found in log
	//MT_PROBABLE,         // A close match - same band/date/call but time out by upto 30 minutes or mode
	//MT_POSSIBLE,         // call found but something important differs
	//MT_LOC_MISMATCH,     // A close match but a location field differs
	//MT_SWL_MATCH,        // An SWL report that is a close match to existing activity
	//MT_SWL_NOMATCH       // An SWL report that is no match for any activity
	//MT_2XSWL_MATCH       // An SWL report matches an existing SWL report
match_result_t record::match_records(record* record) {
	// return NOMATCH if no record to compare
	if (record == nullptr || this == nullptr) {
		return MT_NOMATCH;
	}
	// Match conditions
	int nondate_mismatch_count = 0; // Important fields mismatch
	int trivial_mismatch_count = 0; // Trivial fields mismatch
	bool dates_match = true;
	bool location_match = true;
	bool swl_match = true;
	// For each field in this record
	for (auto it = begin(); it != end(); it++) {
		string field_name = it->first;
		string value = it->second;
		// Time/date match
		if (field_name == "QSO_DATE" ||
			field_name == "TIME_ON" ) {
			dates_match &= items_match(record, field_name);
		}
		else if (
			// Location information matches
			field_name == "CNTY" ||
			field_name == "CONT" ||
			field_name == "COUNTRY" ||
			field_name == "CQZ" ||
			field_name == "DXCC" ||
			field_name == "GRIDSQUARE" ||
			field_name == "IOTA" ||
			field_name == "ITUZ" ||
			field_name == "PFX" ||
			field_name == "STATE") {
			location_match &= items_match(record, field_name);
		}
		else if (
			// Ignore QSL information
			field_name == "EQSL_QSLRDATE" ||
			field_name == "EQSL_QSL_RCVD" ||
			field_name == "EQSL_QSLSDATE" ||
			field_name == "EQSL_QSL_SENT" ||
			field_name == "LOTW_QSLRDATE" ||
			field_name == "LOTW_QSL_RCVD" ||
			field_name == "LOTW_QSLSDATE" ||
			field_name == "LOTW_QSL_SENT" ||
			field_name == "QSLRDATE" ||
			field_name == "QSL_RCVD" ||
			field_name == "QSLSDATE" ||
			field_name == "QSL_SENT") {
		}
		else if (
			// Band, mode MUST match
			field_name == "BAND" ||
			field_name == "MODE") {
			if (!(items_match(record, field_name))) {
				swl_match = false;
				nondate_mismatch_count++;
			}
		}
		else if (
			// call MUST match - but not for SWL
			field_name == "CALL") {
			if (!(items_match(record, field_name))) {
				nondate_mismatch_count++;
			}
		}
		else {
			// Other matches ignored for SWL reports
			if (!(items_match(record, field_name))) {
				trivial_mismatch_count++;
			}
		}
	}
	// All non-trivial fields match - EXACT
	if (dates_match && location_match && nondate_mismatch_count == 0) {
		return MT_EXACT;
	}
	// One or more location field mismatches
	else if (dates_match && !location_match && nondate_mismatch_count == 0) {
		return MT_LOC_MISMATCH;
	}
	// Need more detailed analysis 
	else {
		// Get time difference betweem two records
		double time_difference = difftime(timestamp(), record->timestamp());
		// Time difference is within +/- 30 mins (fairly arbitrary)
		bool within_30mins = abs(time_difference) <= (30.0 * 60.0);
		bool is_swl = (item("SWL") == "Y");
		bool other_swl = (record->item("SWL") == "Y");
		// Both records are SWL and match
		if (is_swl && other_swl && swl_match && within_30mins) {
			return MT_2XSWL_MATCH;
		}
		// is_swl and time within 30 - SWL_MATCH
		else if (is_swl && !other_swl && swl_match && within_30mins) {
			return MT_SWL_MATCH;
		}
		// is_swl - no match
		else if (is_swl && !other_swl && !(swl_match && within_30mins)) {
			return MT_SWL_NOMATCH;
		}
		else if (within_30mins && nondate_mismatch_count == 0) {
			// Date fields match within 30 minutes and all fields match - PROBABLE
			if (trivial_mismatch_count == 0) {
				return MT_PROBABLE;
			}
			// Date fields match within 30 minutes and important fields match match - POSSIBLE
			else {
				return MT_POSSIBLE;
			}
		}
		// Date fields out by > 30 mins and other fields agree - POSSIBLE
		else if (nondate_mismatch_count == 0) {
			return MT_POSSIBLE;
		}
		// Significant difference - NOMATCH
		else {
			return MT_NOMATCH;
		}
	}
}

// compare the item between this record and supplied record
bool record::items_match(record* record, string field_name) {
	string lhs = item(field_name);
	string rhs = record->item(field_name);
	// Convert both fields to upper case
	for (unsigned int i = 0; i < lhs.length(); i++) {
		lhs[i] = toupper(lhs[i]);
	}
	for (unsigned int i = 0; i < rhs.length(); i++) {
		rhs[i] = toupper(rhs[i]);
	}
	if (lhs == rhs ||
		lhs == "" ||
		rhs == "") {
		// Fields are equal or either is blank
		return true;
	}
	// Special cases
	else if (field_name == "GRIDSQUARE" || field_name == "MY_GRIDSQUARE" || field_name == "FREQ" || field_name == "FREQ_RX") {
		// Special case for GRIDSQUARE and FREQ
		// they compare if they are equal for the length
		// of the shorter.
		int iLength = min(lhs.length(), rhs.length());
		if (lhs.substr(0, iLength) == rhs.substr(0, iLength)) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (field_name == "TIME_ON" || field_name == "TIME_OFF") {
		// TIME_ON or TIME_OFF - 
		// they compare if they are equal to the minute
		if (lhs.substr(0, 4) == rhs.substr(0, 4)) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (field_name == "MODE" || field_name == "SUBMODE") {
		// Special case for MODE - check against SUBMODE as well (both ways)
		if (lhs == to_upper(record->item("SUBMODE")) || lhs == to_upper(record->item("MODE"))) {
			return true;
		}
		// The case where the input record has deprecated MODE against existing record SUBMODE
		else if (field_name == "MODE" &&
			to_upper(item("SUBMODE")) == rhs) {
			return true;
		}
		else if (field_name == "MODE" && 
			((spec_data_->dxcc_mode(lhs) == record->item("MODE")) || (spec_data_->dxcc_mode(record->item("MODE")) == lhs))) {
			// Some records from LotW only have generic modes - both ways
			return true;
		}
		else {
			return false;
		}

	}
	return false;
}

// get the date and time as a time_t object
time_t record::timestamp(bool time_off /*= false*/) {
	try {
		// Convert date and time to a tm struct
		tm qso_time;
		if (time_off) {
			// Get end timestamp
			if (item_exists("QSO_DATE_OFF")) {
				// Use QSO_DATE_OFF if it exists
				qso_time.tm_year = stoi(item("QSO_DATE_OFF").substr(0, 4)) - 1900;
				qso_time.tm_mon = stoi(item("QSO_DATE_OFF").substr(4, 2)) - 1;
				qso_time.tm_mday = stoi(item("QSO_DATE_OFF").substr(6, 2));
			}
			else {
				// Use QSO_DATE if it doesn't
				qso_time.tm_year = stoi(item("QSO_DATE").substr(0, 4)) - 1900;
				qso_time.tm_mon = stoi(item("QSO_DATE").substr(4, 2)) - 1;
				qso_time.tm_mday = stoi(item("QSO_DATE").substr(6, 2));
				if (item("TIME_ON") > item("TIMEOFF")) {
					// QSO_DATE_OFF show be inferred to be the day after - increment date
					if (qso_time.tm_mday > days_in_month(&qso_time)) {
						qso_time.tm_mday = 1;
						if (qso_time.tm_mon == 11) {
							qso_time.tm_mon = 0;
							qso_time.tm_year += 1;
						}
						else {
							qso_time.tm_mon += 1;
						}
					}
					else {
						qso_time.tm_mday += 1;
					}
				}
			}
			// Add time on
			qso_time.tm_hour = stoi(item("TIME_OFF").substr(0, 2));
			qso_time.tm_min = stoi(item("TIME_OFF").substr(2, 2));
			qso_time.tm_sec = 0;
			if (item("TIME_OFF").length() == 6) {
				qso_time.tm_sec = stoi(item("TIME_OFF").substr(4, 2));
			}
		}
		else {
			// Get start timestamp
			qso_time.tm_year = stoi(item("QSO_DATE").substr(0, 4)) - 1900;
			qso_time.tm_mon = stoi(item("QSO_DATE").substr(4, 2)) - 1;
			qso_time.tm_mday = stoi(item("QSO_DATE").substr(6, 2));
			qso_time.tm_hour = stoi(item("TIME_ON").substr(0, 2));
			qso_time.tm_min = stoi(item("TIME_ON").substr(2, 2));
			qso_time.tm_sec = 0;
			if (item("TIME_ON").length() == 6) {
				qso_time.tm_sec = stoi(item("TIME_ON").substr(4, 2));
			}
		}
		// To stop it getting randomly set in implementations that do not consistently initialise structures
		qso_time.tm_isdst = false;

		return mktime(&qso_time);
	}
	catch (invalid_argument&) {
		// Return an invalid time
		return time_t(-1);
	}
}

// Update the time the QSO finishes
void record::update_timeoff() {
	if (!is_header_ && item("TIME_OFF").length() == 0) {
		// TIME_OFF has no meaning in a header record
		// Set TIME_OFF to TIME_ON plus 10 s.
		chrono::system_clock::time_point time_on = chrono::system_clock::from_time_t(timestamp());
		chrono::duration<int, ratio<1> > ten_seconds(10);
		time_t time_off = chrono::system_clock::to_time_t(time_on + ten_seconds);
		char temp[10];
		// Convert to date YYYYMMDD and time HHMMSS and update record
		strftime(temp, sizeof(temp), "%Y%m%d", gmtime(&time_off));
		item("QSO_DATE_OFF", string(temp));
		//Fl_Preferences log_settings(settings_, "Log");
		strftime(temp, sizeof(temp), "%H%M%S", gmtime(&time_off));
		item("TIME_OFF", string(temp));
	}
}

// merge data from a number of items - equivalent to a mail-merge 
// replace <FIELD> with the value of that field.
string record::item_merge(string data, bool indirect /*=false*/) {
	string result = data;
	size_t left = result.find('<');
	size_t right = result.find('>');
	// while we still have an unprocessed <> pair
	while (left != result.npos && right != result.npos) {
		string field_name = result.substr(left + 1, right - left - 1);
		result.replace(left, right - left + 1, item(field_name, indirect, true));
		left = result.find('<');
		right = result.find('>');
	}
	return result;
}

// Set MY_RIG, MY_ANTENNA, APP_ZZA_QTH and STATION_CALLSIGN from the current settings
bool record::user_details() {
	//// Add rig & QTH details - note this is current location only
	//Fl_Preferences stations_settings(settings_, "Stations");
	//Fl_Preferences rigs_settings(stations_settings, "Rigs");
	//Fl_Preferences aerials_settings(stations_settings, "Aerials");
	//Fl_Preferences callsigns_settings(stations_settings, "Callsigns");
	//Fl_Preferences qths_settings(stations_settings, "QTHs");
	//bool modified = false;
	//char message[256];
	//// Get rig
	//char * rig;
	//rigs_settings.get("Default", rig, "");
	//if (item("MY_RIG").length()) {
	//	snprintf(message, 256, "LOG: MY_RIG already set to %s, ignoring current value %s", item("MY_RIG").c_str(), rig);
	//	status_->misc_status(ST_WARNING, message);
	//}
	//else {
	//	item("MY_RIG", string(rig));
	//	modified = true;
	//}
	//free(rig);
	//// Get aerial
	//char * aerial;
	//aerials_settings.get("Default", aerial, "");
	//if (item("MY_ANTENNA").length()) {
	//	snprintf(message, 256, "LOG: MY_ANTENNA already set to %s, ignoring current value %s", item("MY_ANTENNA").c_str(), aerial);
	//	status_->misc_status(ST_WARNING, message);
	//}
	//else {
	//	item("MY_ANTENNA", string(aerial));
	//	modified = true;
	//}
	//free(aerial);
	//// Get QTH
	//char * qth;
	//qths_settings.get("Default", qth, "");
	//if (item("APP_ZZA_QTH").length()) {
	//	snprintf(message, 256, "LOG: APP_ZZA_QTH already set to %s, ignoring current value %s", item("APP_ZZA_QTH").c_str(), qth);
	//	status_->misc_status(ST_WARNING, message);
	//}
	//else {
	//	item("APP_ZZA_QTH", string(qth));
	//	modified = true;
	//}
	//Fl_Preferences current_settings(qths_settings, qth);
	//free(qth);
	//// Get callsign
	//char* callsign;
	//callsigns_settings.get("Default", callsign, "");
	//if (item("STATION_CALLSIGN").length()) {
	//	snprintf(message, 256, "LOG: STATION_CALLSIGN already set to %s, ignoring current value %s", item("STATION_CALLSIGN").c_str(), callsign);
	//	status_->misc_status(ST_WARNING, message);
	//}
	//else {
	//	item("STATION_CALLSIGN", string(callsign));
	//	modified = true;
	//}

	return false;
}

// Convert frequency from ADIF (MHz) to display format
string record::format_freq(display_freq_t format, string value) {
	double frequency = stod(value);
	char temp[25] = "";
	switch (format) {
	case FREQ_Hz:
		snprintf(temp, 25, "%.0f", frequency * 1000000);
		break;
	case FREQ_kHz:
		snprintf(temp, 25, "%.3f", frequency * 1000);
		break;
	case FREQ_MHz:
		snprintf(temp, 25, "%.6f", frequency);
		break;
	default:
		strcpy(temp, "Invalid");
		break;
	}
	return string(temp);
}

// Convert date from ADIF (YYYYMMDD) to display format
string record::format_date(display_date_t format, string value) {
	
	// Convert date to display format
	tm date;
	// Get start timestamp
	if (value.length() >= 8) {
		date.tm_year = stoi(value.substr(0, 4)) - 1900;
		date.tm_mon = stoi(value.substr(4, 2)) - 1;
		date.tm_mday = stoi(value.substr(6, 2));
		date.tm_hour = 0;
		date.tm_min = 0;
		date.tm_sec = 0;
		date.tm_isdst = false;
		char temp[20] = "";
		if (date.tm_mon < 13 && date.tm_mday <= days_in_month(&date)) {
			switch (format) {
			case DATE_YYYYMMDD:
				strftime(temp, 20, "%Y%m%d", &date);
				break;
			case DATE_YYYY_MM_DD:
				strftime(temp, 20, "%Y_%m_%d", &date);
				break;
			case DATE_DD_MM_YYYY:
				strftime(temp, 20, "%d_%m_%Y", &date);
				break;
			case DATE_MM_DD_YYYY:
				strftime(temp, 20, "%m_%d_%Y", &date);
				break;
			case DATE_DD_MON_YYYY:
				strftime(temp, 20, "%d-%b-%Y", &date);
				break;
			}
			return string(temp);
		} else {
			char message[100];
			snprintf(message, 100, "LOG: Invalid date %s", value.c_str());
			status_->misc_status(ST_ERROR, message);
			return "";
		}
	}
	else {
		char message[100];
		snprintf(message, 100, "LOG: Invalid date %s", value.c_str());
		status_->misc_status(ST_ERROR, message);
		return "";
	}
}

// Convert time from ADIF (HHMM or HHMMSS) to display format
string record::format_time(display_time_t format, string value) {
	tm date;
	if (value.length() >= 4) {
		date.tm_year = 70;
		date.tm_mon = 0;
		date.tm_mday = 1;
		date.tm_hour = stoi(value.substr(0, 2));
		date.tm_min = stoi(value.substr(2, 2));
		if (value.length() == 6) {
			date.tm_sec = stoi(value.substr(4, 2));
		}
		else {
			date.tm_sec = 0;
		}
		date.tm_isdst = false;
		char temp[20] = "";
		if (date.tm_hour < 24 && date.tm_min < 60 && date.tm_sec < 61) {
			switch (format) {
			case TIME_HHMMSS:
				strftime(temp, 20, "%H%M%S", &date);
				break;
			case TIME_HHMM:
				strftime(temp, 20, "%H%M", &date);
				break;
			case TIME_HH_MM_SS:
				strftime(temp, 20, "%H:%M:%S", &date);
				break;
			case TIME_HH_MM:
				strftime(temp, 20, "%H:%M", &date);
				break;
			}
			return string(temp);
		}
		else {
			char message[100];
			snprintf(message, 100, "LOG: Invalid time %s", value.c_str());
			status_->misc_status(ST_ERROR, message);
			return "";
		}
	}
	else {
		char message[100];
		snprintf(message, 100, "LOG: Invalid time %s", value.c_str());
		status_->misc_status(ST_ERROR, message);
		return "";
	}
}


// Convert frequency from display format to ADIF (MHz)
string record::unformat_freq(display_freq_t format, string value) {
	double frequency = stod(value);
	char temp[25];
	switch (format) {
	case FREQ_Hz:
		snprintf(temp, 25, "%.6f", frequency / 1000000);
		break;
	case FREQ_kHz:
		snprintf(temp, 25, "%.6f", frequency / 1000);
		break;
	case FREQ_MHz:
		snprintf(temp, 25, "%.6f", frequency);
		break;
	default:
		strcpy(temp, "Invalid");
		break;
	}
	return string(temp);
}

// Convert date from display format to ADIF (YYYYMMDD)
string record::unformat_date(display_date_t format, string value) {
	tm date;
	bool ok;
	switch (format) {
	case DATE_YYYYMMDD:
		ok = string_to_tm(value, date, "%Y%m%d");
		break;
	case DATE_YYYY_MM_DD:
		ok = string_to_tm(value, date, "%Y_%m_%d");
		break;
	case DATE_DD_MM_YYYY:
		ok = string_to_tm(value, date, "%d_%m_%Y");
		break;
	case DATE_MM_DD_YYYY:
		ok = string_to_tm(value, date, "%m_%d_%Y");
		break;
	case DATE_DD_MON_YYYY:
		ok = string_to_tm(value, date, "%d-%b-%YYYY");
		break;
	default:
		value = "Invalid";
		ok = false;
		date = tm();
		break;
	}
	char temp[9];
	strftime(temp, 9, "%Y%m%d", &date);
	if (!ok) {
		char message[120];
		snprintf(message, 120, "LOG: Invalid date %s for display format", value.c_str());
		status_->misc_status(ST_ERROR, message);
	}
	return string(temp);
}

// Convert time from display format to ADIF (HHMMSS or HHMM)
string record::unformat_time(display_time_t format, string value) {
	string temp;
	bool ok = false;
	switch (format) {
	case TIME_HHMMSS:
	case TIME_HHMM:
		switch (value.length()) {
		case 4:
		case 6:
			temp = value;
			ok = true;
			break;
		}
		break;
	case TIME_HH_MM_SS:
		if (value.length() == 8) {
			temp = value.substr(0, 2) + value.substr(3, 2) + value.substr(6, 2);
			ok = true;
		}
		break;
	case TIME_HH_MM:
		if (value.length() == 5) {
			temp = value.substr(0, 2) + value.substr(3, 2);
			ok = true;
		}
		break;
	default:
		temp = "";
		break;
	}
	if (!ok) {
		char message[120];
		snprintf(message, 120, "LOG: Invalid time %s for display format", value.c_str());
		status_->misc_status(ST_ERROR, message);
	}
	return string(temp);
}

// Return dirtty flag
bool record::is_dirty() { return is_dirty_; }

// Clear dirty flag
void record::clean() { is_dirty_ = false; }

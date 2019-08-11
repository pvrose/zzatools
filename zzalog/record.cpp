/*
ZZALOG - Amateur radio log
© - Copyright 2018, Philip Rose, GM3ZZA
All Rights Reserved

record.cpp - Individual record data item: implementation file
*/

#include "record.h"
#include "utils.h"
#include "pfx_data.h"
#include "spec_data.h"
#include "rig_if.h"
#include "status.h"
#include "view.h"
#include "formats.h"

#include <ctime>
#include <chrono>
#include <ratio>
#include <cmath>

#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>

using namespace std;
using namespace zzalog;

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

// Constructor that pre-populates certain fields depending on the logging mode
record::record(logging_mode_t type) {
	// Reset record
	delete_contents();
	
	switch (type) {
	case LM_RADIO_CONN:
	case LM_RADIO_DISC: {
		// Interactive mode - start QSO 
		char temp[20];
		// Get current date and time in UTC
		string timestamp = now(false, "%Y%m%d%H%M%S");
		item("QSO_DATE", timestamp.substr(0,8));
		// Time as HHMMSS - always log seconds.
		item("TIME_ON", timestamp.substr(8));  
		item("QSO_DATE_OFF", string(""));
		item("TIME_OFF", string(""));
		item("CALL", string(""));
		// If rig is connected - get information from rig
		if (rig_if_ != nullptr) {
			frequency_t freq_format;
			Fl_Preferences log_settings(settings_, "Log");
			// Get the logging precision from settings and set text format accordingly.
			log_settings.get("Frequency Precision", (int&)freq_format, FREQ_Hz);
			string item_format;
			switch (freq_format) {
			case FREQ_Hz:
				item_format = "%0.6f";
				break;
			case FREQ_Hz10:
				item_format = "%0.5f";
				break;
			case FREQ_Hz100:
				item_format = "%0.4f";
				break;
			case FREQ_kHz:
				item_format = "%0.3f";
				break;
			case FREQ_kHz10:
				item_format = "%0.2f";
				break;
			case FREQ_kHz100:
				item_format = "%0.1f";
				break;
			case FREQ_MHz:
				item_format = "%0.0f";
				break;
			}
			// Get frequency, mode and transmit power from rig
			sprintf(temp, item_format.c_str(), rig_if_->tx_frequency() / 1000000);
			item("FREQ", string(temp));
			// Get mode - NB USB/LSB need further processing
			string mode;
			string submode;
			rig_if_->get_string_mode(mode, submode);
			item("MODE", mode);
			item("SUBMODE", submode);
			sprintf(temp, "%0.0f", rig_if_->power_drive());
			item("TX_PWR", string(temp));
		}
		else {
			// otherwise we enter it manually later.
			item("FREQ", string(""));
			item("MODE", string(""));
			item("SUBMODE", string(""));
			item("TX_PWR", string(""));
		}
		// initialise fields
		item("RX_PWR", string(""));
		item("RST_SENT", string(""));
		item("RST_RCVD", string(""));
		item("NAME", string(""));
		item("QTH", string(""));
		item("GRIDSQUARE", string(""));
		break;
	}
	case LM_OFF_AIR:
	case LM_IMPORTED:
		// Just initialise to empty strings
		item("QSO_DATE", string(""));
		item("TIME_ON", string(""));
		item("QSO_DATE_OFF", string(""));
		item("TIME_OFF", string(""));
		item("CALL", string(""));
		item("FREQ", string(""));
		item("MODE", string(""));
		item("TX_PWR", string(""));
		item("RX_PWR", string(""));
		item("RST_SENT", string(""));
		item("RST_RCVD", string(""));
		item("NAME", string(""));
		item("QTH", string(""));
		item("GRIDSQUARE", string(""));
		break;
	}
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
	// Delete items
	clear();
}

// Set an item pair.
void record::item(string field, string value, bool formatted/* = false*/) {
	// Certain fields - always log in upper case
	string upper_value;
	if (field == "CALL" ||
		field == "CONT" ||
		field == "EQ_CALL" ||
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
				upper_value = to_upper(value);
			} else {
				upper_value = value;
			}
		}
	}
	if (formatted) {
		// Convert from the displayed format to ADIF format
		string formatted_value;
		if (upper_value == "") {
			// empty string gives empty string
			formatted_value = "";
		}
		else {
			char type_indicator = spec_data_->datatype_indicator(field);
			switch (type_indicator) {
			case 'D':
			case 'T':
			case 'N':
			case 'L':
			case 'S':
			case 'I':
			case 'M': 
			case 'G':
			case 'B':
				// String leave as is
				formatted_value = upper_value;
				break;
			case 'E':
				// Boolean or Enumeration: convert to uppercase
				formatted_value = to_upper(upper_value);
				if (field == "MODE" && spec_data_->is_submode(formatted_value)) {
					item("SUBMODE", formatted_value, formatted);
					formatted_value = spec_data_->mode_for_submode(formatted_value);
				}
				break;
			default:
				formatted_value = upper_value;
				break;
			}
		}
		// Set in item
		(*this)[field] = formatted_value;
	}
	else {
	 // Use the data directly in the item
	 (*this)[field] = upper_value;
	}
}

// Get an item - as string
string record::item(string field, bool formatted/* = false*/, bool indirect/* = false*/) {
	string result;
	if (indirect) {
		// Get the item value baed on APP_ZZA_QTH
		result = item(field);
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
			if (field == "STATION_CALLSIGN" || field == "OPERATOR") {
				// Get station callsign
				qth_settings.get("Callsign", temp, "QX1XXX");
				result = temp; 
				free(temp);
			}
			else if (field == "MY_NAME") {
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
				qth_settings.get("County", temp, "Nowhereshire");
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
				result = temp;
				free(temp);
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
			switch (type_indicator) {
			case ' ':
			case 'D':
			case 'T':
			case 'N':
			case 'S':
			case 'I':
			case 'M': 
			case 'G':
			case 'B':
			case 'L':
				result = unformatted_value;
				break;
			case 'E':
				if (field == "MODE" && item_exists("SUBMODE")) {
					result = item("SUBMODE", formatted, indirect);
				}
				else {
					result = unformatted_value;
				}
				break;
			default:
				// If any other data indicator gets return - it shouldn't
				result = unformatted_value;
				break;
			}
		}
	}
	else {
		// Use the field directly
		auto it = find(field);
		if (it == end()) {
			// Field not present returm empty string
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
			value = nan("");
			// Special cases
			if (field == "LAT" || field == "LON") {
				try {
					string item_value = item(field);
					double degrees = stod(item_value.substr(1, 3)) + (stod(item_value.substr(5, 6)) / 60.0);
					switch (item_value[0]) {
					case 'S':
					case 'W':
						value = 0.0 - degrees;
						break;
					case 'N':
					case 'E':
						value = degrees;
						break;
					}
				}
				catch (invalid_argument&) {
					value = nan("");
				}
			}
			else if (field == "MY_LAT" || field == "MY_LON") {
				try {
					// Get the inherited value if necessary
					string item_value = item(field, false, true);
					double degrees = stod(item_value.substr(1, 3)) + (stod(item_value.substr(5, 6)) / 60.0);
					switch (item_value[0]) {
					case 'S':
					case 'W':
						value = 0.0 - degrees;
						break;
					case 'N':
					case 'E':
						value = degrees;
						break;
					}
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
	sprintf(message,"UNPARSE QSO: %s %s %s - unparsed",
		item("QSO_DATE").c_str(), item("TIME_ON").c_str(), item("CALL").c_str());
	status_->misc_status(ST_LOG, message);
}

// Return laongitude and latitude
lat_long_t record::location(bool my_station) {
	location_t dummy;
	return location(my_station, dummy);
}

// Return longitude and latitude from the record
lat_long_t record::location(bool my_station, location_t& source) {
	// Set a bad coordinate
	lat_long_t lat_long = { nan(""), nan("") };

	// Now look at the various sources of lat/lon
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
		}
		else {
			value_1 = item("GRIDSQUARE");
		}
		// Get the lat/long of the centre of the grid-square 
		lat_long = grid_to_latlong(value_1);
		// 2 or 4 character grid square - use centre of grid square or centre of prefix area if that is inside the gridsquare
		if (value_1.length() <= 4) {
			// Get prefix coordinates
			lat_long_t prefix_lat_long = { nan(""), nan("") };
			string prefix_code = item("APP_ZZA_PFX");
			if (prefix_code != "") {
				prefix* prefix = pfx_data_->get_prefix(prefix_code);
				if (prefix != NULL) {
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
				// Test to see if the prefix lat/long is within the 4-character gridsquare
				if (!isnan(prefix_lat_long.longitude) &&
					!isnan(prefix_lat_long.latitude)) {
					if ((isnan(lat_long.longitude) ||
						isnan(lat_long.longitude)) &&
						abs(lat_long.longitude - prefix_lat_long.longitude) < side &&
						abs(lat_long.latitude - prefix_lat_long.latitude) < (side * 0.5)) {
						// If so use it instead of GRIDSQUARE
						lat_long = prefix_lat_long;
					}
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
bool record::merge_records(record* record, bool allow_loc_mismatch /* = false */)
{
	bool merged = false;
	bool bearing_change = false;
	// For each field in other record
	for (auto it = record->begin(); it != record->end(); it++) {
		string field_name = it->first;
		string merge_data = it->second;
		// Get my value for the field
		string my_data = item(field_name);
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
				// Grid not is good (RR73 confirmed by user)
				is_location = true;
				// Updating a 4-character grid with a 6-character one and the first 4 agree
				if (my_data.length() < merge_data.length() && merge_data.substr(0, my_data.length()) == my_data) {
					is_grid_4_to_6 = true;
					bearing_change = true;
				}
			}
		}
		else if (field_name == "EQSL_QSL_RCVD" ||
			field_name == "LOTW_QSL_RCVD" ||
			field_name == "QSL_RCVD") {
			// QSL received - need to replace N with Y
			is_qsl_rcvd = true;
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
	//MT_PROBABLE,         // A close match - same band/date/call but time out by upto 30 minutes
	//MT_POSSIBLE,         // call found but something important differs
	//MT_LOC_MISMATCH,     // A close match but a location field differs
	//MT_SWL_MATCH,        // An SWL report that is a close match to existing activity
	//MT_SWL_NOMATCH       // An SWL report that is no match for any activity
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
			field_name == "QSO_DATE_OFF" ||
			field_name == "TIME_ON" ||
			field_name == "TIME_OFF") {
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
			field_name == "QSL_SENT" ||
			field_name == "APP_ZZA_EQSL_TS") {
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
		// is_swl and time within 30 - SWL_MATCH
		if (is_swl && swl_match && within_30mins) {
			return MT_SWL_MATCH;
		}
		// is_swl - no match
		else if (is_swl && !(swl_match && within_30mins)) {
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

// items match between records
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
	else if (field_name == "GRIDSQUARE" || field_name == "MY_GRIDSQUARE" ||
		field_name == "TIME_ON" || field_name == "TIME_OFF") {
		// Special case for GRIDSQUARE, TIME_ON or TIME_OFF - 
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
	else if (field_name == "MODE") {
		// Special case for MODE - check against SUBMODE as well
		if (rhs == item("SUBMODE") || lhs == record->item("SUBMODE")) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

// get the date and time as a time_t object
time_t record::timestamp(bool time_off /*= false*/) {
	try {
		// Convert date and time to a tm struct
		tm qso_time;
		if (time_off) {
			if (item_exists("QSO_DATE_OFF")) {
				qso_time.tm_year = stoi(item("QSO_DATE_OFF").substr(0, 4)) - 1900;
				qso_time.tm_mon = stoi(item("QSO_DATE_OFF").substr(4, 2)) - 1;
				qso_time.tm_mday = stoi(item("QSO_DATE_OFF").substr(6, 2));
			}
			else {
				qso_time.tm_year = stoi(item("QSO_DATE_ON").substr(0, 4)) - 1900;
				qso_time.tm_mon = stoi(item("QSO_DATE_ON").substr(4, 2)) - 1;
				qso_time.tm_mday = stoi(item("QSO_DATE_ON").substr(6, 2));
			}
			qso_time.tm_hour = stoi(item("TIME_OFF").substr(0, 2));
			qso_time.tm_min = stoi(item("TIME_OFF").substr(2, 2));
			qso_time.tm_sec = 0;
			if (item("TIME_OFF").length() == 6) {
				qso_time.tm_sec = stoi(item("TIME_OFF").substr(4, 2));
			}
		}
		else {
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
	if (!is_header_) {
		// Only do this for a QSO record - get the start time and add 10 seconds
		chrono::system_clock::time_point time_on = chrono::system_clock::from_time_t(timestamp());
		chrono::duration<int, ratio<1> > ten_seconds(10);
		time_t time_off = chrono::system_clock::to_time_t(time_on + ten_seconds);
		char temp[10];
		// Convert to date YYYYMMDD and time HHMMSS and update record
		strftime(temp, sizeof(temp), "%Y%m%d", gmtime(&time_off));
		item("QSO_DATE_OFF", string(temp));
		Fl_Preferences log_settings(settings_, "Log");
		strftime(temp, sizeof(temp), "%H%M%S", gmtime(&time_off));
		item("TIME_OFF", string(temp));
	}
}

// End record by adding certain fields 
void record::end_record(logging_mode_t mode) {
	if (!is_header_) {
		// Always update MY_RIG ets.
		user_details();
		// On-air logging add date/time off
		if (mode == LM_RADIO_CONN || mode == LM_RADIO_DISC) {
			if (item("TIME_OFF") == "") {
				// Add end date/time - current time of interactive entering
				// Get current date and time in UTC
				string timestamp = now(false, "%Y%m%d%H%M%S");
				item("QSO_DATE_OFF", timestamp.substr(0, 8));
				// Time as HHMMSS - always log seconds.
				item("TIME_OFF", timestamp.substr(8));
			}
		}
	}
}

// merge data from a number of items - equivalent to a mail-merge 
// replace <FIELD> with the value of that field.
string record::item_merge(string data) {
	string result = data;
	size_t left = result.find('<');
	size_t right = result.find('>');
	while (left != result.npos && right != result.npos) {
		string field_name = result.substr(left + 1, right - left - 1);
		result.replace(left, right - left + 1, item(field_name, false, true));
		left = result.find('<');
		right = result.find('>');
	}
	return result;
}

// Set MY_RIG, MY_ANTENNA, APP_ZZA_QTH and PROP_MODE from the current settings
bool record::user_details() {
	// Add rig & QTH details - note this is current location only
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	Fl_Preferences aerials_settings(stations_settings, "Aerials");
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	Fl_Preferences prop_settings(stations_settings, "Propagation Mode");
	bool modified = false;
	// Get rig
	char * rig;
	rigs_settings.get("Current", rig, "");
	if (string(rig) != item("MY_RIG")) modified = true;
	item("MY_RIG", string(rig));
	free(rig);
	// Get aerial
	char * aerial;
	aerials_settings.get("Current", aerial, "");
	if (string(aerial) != item("MY_ANTENNA")) modified = true;
	item("MY_ANTENNA", string(aerial));
	free(aerial);
	// Get QTH
	char * qth;
	qths_settings.get("Current", qth, "");
	if (string(qth) != item("APP_ZZA_QTH")) modified = true;
	item("APP_ZZA_QTH", string(qth));
	free(qth);
	// Get propagation mode
	char* mode;
	prop_settings.get("Current", mode, "");
	if (string(mode) != item("PROP_MODE")) modified = true;
	item("PROP_MODE", string(mode));
	free(mode);
	return modified;
}
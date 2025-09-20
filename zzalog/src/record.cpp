/*
ZZALOG - Amateur radio log
� - Copyright 2018, Philip Rose, GM3ZZA
All Rights Reserved

record.cpp - Individual record data item: implementation file
*/

#include "record.h"
#include "utils.h"
#include "cty_data.h"
#include "spec_data.h"
#include "status.h"
#include "view.h"
#include "book.h"

#include <ctime>
#include <chrono>
#include <ratio>
#include <cmath>

#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_utf8.h>





extern cty_data* cty_data_;
extern spec_data* spec_data_;
extern status* status_;
extern book* book_;
extern std::string VENDOR;
extern std::string PROGRAM_ID;

// initialise the static variables
bool record::expecting_header_ = true;
bool record::inhibit_error_reporting_ = false;

// Comparison operator - compares QSO_DATE and TIME_ON - orders the records by time.
bool record::operator > (record& them) {
	// Basic std::string comparison "YYYYMMDDHHMMSS"
	std::string my_time = item("QSO_DATE") + item("TIME_ON");
	std::string their_time = them.item("QSO_DATE") + them.item("TIME_ON");
	if (my_time > their_time) {
		return true;
	}
	else if (my_time == their_time && item("CALL") > them.item("CALL")) {
		// If times are equal then sort on call
		return true;
	} else {
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
			std::string field = iter->first;
			std::string value = iter->second;
			item(field, value, false, false);
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
	// clear();
}

// Set an item pair.
void record::item(std::string field, std::string value, bool formatted/* = false*/, bool dirty /*=true*/) {
	// Check we are not deleting an important field - crash the program if this was unintentional
	char message[256];
	snprintf(message, 256, "You are deleting %s, are you sure", field.c_str());
	if ((field == "CALL" || field == "QSO_DATE" || field == "QSO_DATE_OFF" || field == "TIME_ON" ||
		field == "TIME_OFF") &&
		item(field).length() && !value.length() && item("QSO_COMPLETE") == "" &&
		fl_choice(message, "Yes", "No", nullptr) == 1) {
		snprintf(message, 256, "Unexpected deletion of the field, %s", field.c_str());
		status_->misc_status(ST_FATAL, message);
		return;
	}
	// Otherwise if writing to "", erase the item
	if (!value.length()) {
		// SEt dirty flag if contents are changing
		if (dirty) {
			std::string orig_value;
			if (find(field) != end()) {
				orig_value = at(field);
			}
			else {
				orig_value = "";
			}
			if (orig_value != "") {
				char msg[32];
				snprintf(msg, sizeof(msg), "%s=%s", field.c_str(), value.c_str());
				book_->add_dirty_record(this, msg);
			}
		}
		erase(field);
		return;
	}
	// Certain fields - always log in upper case
	std::string upper_value;
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
			std::string datatype = spec_data_->datatype(field);
			if (datatype == "PositiveInteger") {
				// Always strip off leading zeros
				size_t dummy;
				int int_value;
				try {
					int_value = std::stoi(value, &dummy);
					if (dummy == value.length()) {
						// The whole std::string is an integer - convert it back to std::string
						upper_value = to_string(int_value);
					}
					else {
						// Use the original std::string
						upper_value = value;
					}
				}
				catch (invalid_argument&) {
					// Empty std::string, so use that
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
	std::string formatted_value;
	if (formatted) {
		// Convert from the displayed format to ADIF format
		if (upper_value == "") {
			// empty std::string gives empty std::string
			formatted_value = "";
		}
		else {
			char type_indicator = spec_data_->datatype_indicator(field);
			double as_d = 0.0;
			int as_i = 0;
			char c;
			switch (type_indicator) {
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
						// std::set submode to ""
						item("SUBMODE", std::string(""));
					}
				}
				break;
			case 'L':
				// Convert signed decimal degree to ADIF spec LAT or LON value.
				as_d = std::stod(upper_value);
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
		std::string orig_value;
		if (find(field) != end()) {
			orig_value = at(field);
		}
		else {
			orig_value = "";
		}
		if (orig_value != formatted_value) {
			char msg[32];
			snprintf(msg, sizeof(msg), "%s=%s", field.c_str(), value.c_str());
			book_->add_dirty_record(this, msg);
		}
	}
	// Set in item
	(*this)[field] = formatted_value;
}

// Get an item - as std::string
std::string record::item(std::string field, bool formatted/* = false*/) {
	std::string result;
	if (formatted) {
		// Return the display format for the field
		std::string unformatted_value = item(field);
		// Convert empty std::string to empty std::string
		if (unformatted_value == "") {
			result = unformatted_value;
		}
		else {

			// Get the type indicator of the field
			char type_indicator = spec_data_->datatype_indicator(field);
			double as_d = 0.0;
			char as_c[15];
			switch (type_indicator) {
			case 'N':
			case ' ':
			case 'S':
			case 'I':
			case 'M':
			case 'G':
			case 'B':
				if (field == "GRIDSQUARE" || field == "MY_GRIDSQUARE") {
					// Concatenate base and extension fields
					if (unformatted_value.length() <= 8) {
						result = unformatted_value;
					} 
					else if (item_exists(field + "_EXT")) {
						result = unformatted_value + item(field + "_EXT", formatted);
					}
				}
				else {
					// No formatting
					result = unformatted_value;
				}
				break;
			case 'E':
				// Special case for MODE - use SUBMODE if it exists
				if (field == "MODE" && item_exists("SUBMODE")) {
					result = item("SUBMODE", formatted);
				}
				// No other formatting
				else {
					result = unformatted_value;
				}
				break;
			case 'L':
				// LAT/LON value return signed decimal degree
				as_d = std::stod(unformatted_value.substr(1, 3));
				as_d += (std::stod(unformatted_value.substr(5)) / 60.0);
				switch (unformatted_value[0]) {
				case 'W':
				case 'S':
					as_d = -as_d;
				}
				snprintf(as_c, 15, "%g", as_d);
				result = std::string(as_c);
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
			// Field not present return empty std::string
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
void record::item(std::string field, int& value) {
	if (item_exists(field)) {
		try {
			// Return integer value
			value = std::stoi(item(field));
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

// get an item - as an unsigned long long, default 0
void record::item(std::string field, unsigned long long& value) {
	if (item_exists(field)) {
		try {
			// Return integer value
			value = stoull(item(field));
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
void record::item(std::string field, double& value) {
	if (item_exists(field)) {
		try {
			// return double value
			value = std::stod(item(field));
		}
		catch (invalid_argument&) {
			// If it's not a valid decimal it may be in LAT/LON format
			if (field == "LAT" || field == "LON" || field == "MY_LAT" || field == "MY_LON") {
				// Get formatted version
				std::string item_value = item(field, true);
				try {
					value = std::stod(item_value);
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

// does the item exist - in the std::map and not an empty std::string
bool record::item_exists(std::string field) {
	return find(field) != end() && at(field) != "";
}

// std::set the header information
void record::header(std::string comment) {
	is_header_ = true;
	header_comment_ = comment;
	book_->add_dirty_record(this, comment);
}

// get the header information
std::string record::header() {
	return header_comment_;
}

// return the is_header flag
bool record::is_header() {
	return this->is_header_;
}

// Delete all parsed fields
void record::unparse() {
	// Band - only if frequency is there
	if (item_exists("FREQ")) item("BAND", std::string(""));
	if (item_exists("FREQ_RX")) item("BAND_RX", std::string(""));
	// Geography fields - based on parsing the callsign
	item("DXCC", std::string(""));
	item("COUNTRY", std::string(""));
	item("CONT", std::string(""));
	item("CQZ", std::string(""));
	item("ITUZ", std::string(""));
	item("APP_ZZA_PFX", std::string(""));
	// bearing and distance - based on home location and DX location (from record or prefix)
	item("ANT_AZ", std::string(""));
	item("DISTANCE", std::string(""));
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
	std::string value_1;
	std::string value_2;
	// Use LAT/LON fields
	if (my_station) {
		item("MY_LAT", lat_long.latitude);
		item("MY_LON", lat_long.longitude);
	}
	else {
		item("LAT", lat_long.latitude);
		item("LON", lat_long.longitude);
	}
	if (std::isnan(lat_long.latitude) || std::isnan(lat_long.longitude)) {
		// Use either Gridsquare or prefix centre if that is more accurate
		if (my_station) {
			value_1 = item("MY_GRIDSQUARE", false);
			// No gridsquare for user. 
			if (value_1.length() == 0) {
				source = LOC_NONE;
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
				lat_long.latitude += (side / 4.0);
				lat_long.longitude += (side / 2.0);
			}
			else {
				lat_long = cty_data_->location(this);
				source = LOC_PREFIX;
				return lat_long;
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
			std::string band = spec_data_->band_for_freq(frequency);
			item("BAND", band);
			updated = true;
		}
		// Update BAND_RX from FREQ_RX if that has been logged
		if (force || item("BAND_RX") == "") {
			// Get frequency
			double frequency;
			item("FREQ_RX", frequency);
			// If it exists - look up the band
			if (!std::isnan(frequency)) {
				std::string band = spec_data_->band_for_freq(frequency);
				item("BAND_RX", band);
				updated = true;
			}
			else {
				if (item("BAND_RX") != "") {
					item("BAND_RX", std::string(""), true);
					updated = true;
				}
			}
		}
		return updated;
	}

}

// Merge fields from another record
bool record::merge_records(record* merge_record, match_flags_t flags, hint_t* result /*= nullptr*/)
{
	bool merged = false;
	bool bearing_change = false;
	// For each field in other merge_record
	for (auto it = merge_record->begin(); it != merge_record->end(); it++) {
		std::string field_name = it->first;
		std::string merge_data = it->second;
		// Get my value for the field
		std::string my_data = item(field_name);
		bool item_match = items_match(merge_record, field_name);
		bool is_location = false;
		bool is_grid_4_to_6 = false;
		bool is_qsl_rcvd = false;
		bool ignore_import = false;
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
			if (flags & MR_ALLOW_LOC) {
				bearing_change = true;
			}
		}
		else if (field_name == "GRIDSQUARE") {
			// Gridsquare: location field - RR73 is an almost fictitious location 
			// Actually it's to the north of the Bering Straits, but can be captured if
			// RR73 is used in JT exchanges
			if (merge_data != "RR73" || 
				fl_choice("Location in merge_record is RR73 - please confirm?", fl_ok, fl_cancel, "") == 0) {
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
				item("CLUBLOG_QSO_UPLOAD_STATUS", std::string("M"));
				merged = true;
			}
			is_qsl_rcvd = true;
		}
		else if (field_name == "QSL_SENT" ||
			field_name == "QSL_SENT_VIA" ||
			field_name == "QSLMSG") {
			// Fields added by eQSL - ignore these
			// Also used by OQRS - ignore if we've already sent a bureau card
			if (flags & MR_ALLOW_QSLS) {
				if (item("QSL_SENT") == "Y" && item("QSL_SENT_VIA") == "B") ignore_import = true;
			} else ignore_import = true;
		}
		else if (field_name == "TIME_OFF" ||
			field_name == "QSO_DATE_OFF") {
			// Update from the other merge_record - erstwhile bug overwrote previous import
			if (!item_match) {
				// Use the later of the two
				if (merge_data > my_data) {
					item(field_name, merge_data);
					merged = true;
				}
			}
			// Match but merge data is more accurate
			else if (field_name == "TIME_OFF" && merge_data.length() > my_data.length()) {
				item(field_name, merge_data);
				merged = true;
			}
		}
		else if (field_name == "TIME_ON" ||
			field_name == "QSO_DATE") {
			// Update from the other merge_record - erstwhile bug overwrote previous import
			if (!item_match) {
				// Use the earlier of the two
				if (merge_data < my_data) {
					item(field_name, merge_data);
					merged = true;
				}
			}
			// Match but merge data is more accurate
			else if (field_name == "TIME_ON" && merge_data.length() > my_data.length()) {
				item(field_name, merge_data);
				merged = true;
			}
		}
		else if (item_match  && (field_name == "TIME_ON" || field_name == "FREQ" || field_name == "FREQ_RX") && merge_data.length() > my_data.length()) {
			// Merge data is more accurate
			item(field_name, merge_data);
			merged = true;
		}
		else if (!item_match && (field_name == "RST_RVCD" || field_name == "RST_SENT")) {
			// Use merge data if the values match numerically but differ in std::string value
			int l_value;
			int r_value;
			item(field_name, l_value);
			merge_record->item(field_name, r_value);
			if (l_value == r_value) {
				item(field_name, merge_data);
				merged = true;
			}
		}
		else {
			// All other fields
			is_location = false;
		}
		// Merge if current field is "", is a location field being updated by LotW or 
		// is updating from 4-char gridsquare to 6-char or QSL received
		if ((my_data == "" && !ignore_import) ||
			(is_location && (flags & MR_ALLOW_LOC)) || is_grid_4_to_6 || is_qsl_rcvd) {
			// Can safely merge - use un-case-converted
			item(field_name, merge_data);
			merged = true;
		}
	}
	// Recalculate bearing and distance as location has changed
	if (bearing_change) {
		update_bearing();
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

// Update the ANT_AZ and DISTANCE fields based on record data or entity data
void record::update_bearing() {
	lat_long_t their_location = location(false);
	lat_long_t my_location = location(true);
	// Need both user and contact locations
	if (!std::isnan(my_location.latitude) && !std::isnan(my_location.longitude) &&
		!std::isnan(their_location.latitude) && !std::isnan(their_location.longitude)) {
		double bearing;
		double distance;
		// Calculate bearing and distance
		great_circle(my_location, their_location, bearing, distance);
		char azimuth[16];
		sprintf(azimuth, "%0.f", bearing);
		char distance_string[16];
		sprintf(distance_string, "%0.f", distance);
		// Update if different
		if (item("ANT_AZ") != azimuth) {
			item("ANT_AZ", std::string(azimuth));
		}
		if (item("DISTANCE") != distance_string) {
			item("DISTANCE", std::string(distance_string));
		}
	}
}


// Change a field name
void record::change_field_name(std::string from, std::string to) {
	// Set the item changing it to
	item(to, item(from));
	// Clear the existing one
	item(from, std::string(""));
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
	int net_mismatch_count = 0; // QSOs do not form part of a net
	bool dates_match = true;
	bool location_match = true;
	bool swl_match = true;
	bool call_match = true;
	// For each field in this record
	for (auto it = begin(); it != end(); it++) {
		std::string field_name = it->first;
		std::string value = it->second;
		// Time/date match
		if (field_name == "QSO_DATE" ||
			field_name == "TIME_ON" ) {
			dates_match &= items_match(record, field_name);
		}
		else if (
			// Location information matches
			field_name == "CONT" ||
			field_name == "DXCC" ||
			field_name == "GRIDSQUARE" ||
			field_name == "IOTA" ||
			field_name == "PFX") {
			location_match &= items_match(record, field_name);
		}
		else if (
			// Ignore QSL information
			field_name.substr(0,7) == "QRZCOM_" ||
			field_name.substr(0,8) == "CLUBLOG_" ||
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
				net_mismatch_count++;
			}
		}
		else if (
			// CALL and STATION_CALLSIGN _MUST_ match - but not for SWL
			field_name == "CALL" ||
			field_name == "STATION_CALLSIGN") {
			if (!(items_match(record, field_name))) {
				nondate_mismatch_count++;
				call_match = false;
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
		// The two records overlap
		std::chrono::system_clock::time_point this_on = ctimestamp();
		std::chrono::system_clock::time_point this_off = ctimestamp(true);
		std::chrono::system_clock::time_point that_on = record->ctimestamp();
		std::chrono::system_clock::time_point that_off = record->ctimestamp(true);
		std::chrono::seconds min30(1800);
		// Make the QSO lengths >= 30 minutes
		if (this_off - this_on < min30) this_off = this_on + min30;
		if (that_off - that_on < min30) that_off = that_on + min30;
		bool overlap =
			(that_on >= this_on && that_on <= this_off) ||
			(that_off >= this_on && that_off <= this_off) ||			
			(this_on >= that_on && this_on <= that_off) ||
			(this_off >= that_on && this_off <= that_off);
		bool is_swl = (item("SWL") == "Y");
		bool other_swl = (record->item("SWL") == "Y");
		// Both records are SWL and match
		if (is_swl && other_swl && swl_match && overlap) {
			return MT_2XSWL_MATCH;
		}
		// is_swl and time within 30 - SWL_MATCH
		else if (is_swl && !other_swl && swl_match && overlap) {
			return MT_SWL_MATCH;
		}
		// is_swl - no match
		else if (is_swl && !other_swl && !(swl_match && overlap)) {
			return MT_SWL_NOMATCH;
		}
		else if (overlap && nondate_mismatch_count == 0) {
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
			return MT_UNLIKELY;
		}
		// The two records overlap their times on and off 
		else if (overlap && net_mismatch_count == 0 && !call_match) {
			return MT_OVERLAP;
		}
		// Significant difference - NOMATCH
		else {
			return MT_NOMATCH;
		}
	}
}

// compare the item between this record and supplied record
bool record::items_match(record* record, std::string field_name) {
	std::string lhs = item(field_name);
	std::string rhs = record->item(field_name);
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
	else if (field_name == "GRIDSQUARE" || field_name == "MY_GRIDSQUARE") {
		// Special case for GRIDSQUARE
		// they compare if they are equal for the length
		// of the shorter.
		int iLength = min(lhs.length(), rhs.length());
		iLength = min(iLength, 4);
		if (lhs.substr(0, iLength) == rhs.substr(0, iLength)) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (field_name == "CALL") {
		// Ignore /M or /P
		if (lhs.length() == rhs.length() - 2 && rhs[lhs.length()] == '/') {
			return true;
		} else if (rhs.length() == lhs.length() - 2 && lhs[rhs.length()] == '/') {
			return true;
		} else {
			// Remove intermediate dashes in SWL calls
			std::string lhs1;
			for (int ix = 0; ix < lhs.length(); ix++) {
				if (lhs[ix] != '-') lhs1 += lhs[ix];
			}
			std::string rhs1;
			for (int ix = 0; ix < rhs.length(); ix++) {
				if (rhs[ix] != '-') rhs1 += rhs[ix];
			}
			if (lhs1 == rhs1) return true;
			else return false;
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
	else if (field_name == "FREQ" || field_name == "FREQ_RX") {
		// Frequency may be given to fewer decimal places
		// Match if in same band
		double f1, f2;
		item(field_name, f1);
		record->item(field_name, f2);
		if (spec_data_->band_for_freq(f1) == spec_data_->band_for_freq(f2)) {
			return true;
		} else {
			return false;
		}
	}
	return false;
}

// Get the date and time as a std::chrono::system_clock::timepoisnt
std::chrono::system_clock::time_point record::ctimestamp(bool time_off) {
	return std::chrono::system_clock::from_time_t(timestamp(time_off));
}

// get the date and time as a time_t object
time_t record::timestamp(bool time_off /*= false*/) {
	try {
		// Convert date and time to a tm struct
		tm qso_time;
		if (time_off) {
			if (item("TIME_OFF").length()) {
				// Get end timestamp
				if (item_exists("QSO_DATE_OFF")) {
					// Use QSO_DATE_OFF if it exists
					qso_time.tm_year = std::stoi(item("QSO_DATE_OFF").substr(0, 4)) - 1900;
					qso_time.tm_mon = std::stoi(item("QSO_DATE_OFF").substr(4, 2)) - 1;
					qso_time.tm_mday = std::stoi(item("QSO_DATE_OFF").substr(6, 2));
				}
				else {
					// Use QSO_DATE if it doesn't
					qso_time.tm_year = std::stoi(item("QSO_DATE").substr(0, 4)) - 1900;
					qso_time.tm_mon = std::stoi(item("QSO_DATE").substr(4, 2)) - 1;
					qso_time.tm_mday = std::stoi(item("QSO_DATE").substr(6, 2));
					if (item("TIME_ON") > item("TIME_OFF")) {
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
			} else {
				// Assume QSO is 10 minutes long
				// Check this is 10 mins
				std::chrono::system_clock::time_point time_on = std::chrono::system_clock::from_time_t(timestamp());
				std::chrono::seconds ten_minutes(600);
				time_t time_off = std::chrono::system_clock::to_time_t(time_on + ten_minutes);
				return time_off;
			}
			// Add time on
			qso_time.tm_hour = std::stoi(item("TIME_OFF").substr(0, 2));
			qso_time.tm_min = std::stoi(item("TIME_OFF").substr(2, 2));
			qso_time.tm_sec = 0;
			if (item("TIME_OFF").length() == 6) {
				qso_time.tm_sec = std::stoi(item("TIME_OFF").substr(4, 2));
			}
		}
		else {
			// Get start timestamp
			qso_time.tm_year = std::stoi(item("QSO_DATE").substr(0, 4)) - 1900;
			qso_time.tm_mon = std::stoi(item("QSO_DATE").substr(4, 2)) - 1;
			qso_time.tm_mday = std::stoi(item("QSO_DATE").substr(6, 2));
			qso_time.tm_hour = std::stoi(item("TIME_ON").substr(0, 2));
			qso_time.tm_min = std::stoi(item("TIME_ON").substr(2, 2));
			qso_time.tm_sec = 0;
			if (item("TIME_ON").length() == 6) {
				qso_time.tm_sec = std::stoi(item("TIME_ON").substr(4, 2));
			}
		}
		// To stop it getting randomly std::set in implementations that do not consistently initialise structures
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
		std::chrono::system_clock::time_point time_on = std::chrono::system_clock::from_time_t(timestamp());
		std::chrono::seconds ten_seconds(10);
		time_t time_off = std::chrono::system_clock::to_time_t(time_on + ten_seconds);
		char temp[10];
		// Convert to date YYYYMMDD and time HHMMSS and update record
		strftime(temp, sizeof(temp), "%Y%m%d", gmtime(&time_off));
		item("QSO_DATE_OFF", std::string(temp));
		strftime(temp, sizeof(temp), "%H%M%S", gmtime(&time_off));
		item("TIME_OFF", std::string(temp));
	}
}

// merge data from a number of items - equivalent to a mail-merge 
// replace <FIELD> with the value of that field.
std::string record::item_merge(std::string data, bool indirect /*=false*/) {
	std::string result = data;
	size_t left = result.find('<');
	size_t right = std::string::npos;
	if (left != result.npos) right = result.substr(left).find_first_of(":>");
	// while we still have an unprocessed <> pair
	while (left != result.npos && right != result.npos) {
		std::string field_name = result.substr(left + 1, right - 1);
		if (result[left + right] == ':') {
			size_t close = result.substr(left + right + 1).find('>');
			int len = 0;
			if (close != result.npos) {
				len = atoi(result.substr(left + right + 1, close).c_str());
			}
			result.replace(left, right + close + 2, item(field_name, indirect).substr(0, len));
		}
		else {
			result.replace(left, right + 1, item(field_name, indirect));
		}
		left = result.find('<');
		if (left != result.npos) right = result.substr(left).find_first_of(":>");
	}
	return result;
}

// Invalidate QSL status
void record::invalidate_qsl_status() {
	// eQSL
	if (item("EQSL_QSL_SENT") == "Y") {
		item("EQSL_QSL_SENT", std::string("I"));
	}
	if (item("LOTW_QSL_SENT") == "Y") {
		item("LOTW_QSL_SENT", std::string("I"));
	}
	if (item("CLUBLOG_QSO_UPLOAD_STATUS") == "Y") {
		item("CLUBLOG_QSO_UPLOAD_STATUS", std::string("M"));
	}
}
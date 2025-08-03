#include "cty1_data.h"

#include "cty_data.h"
#include "cty1_reader.h"
#include "club_handler.h"
#include "status.h"
#include "callback.h"
#include "utils.h"
#include "spec_data.h"
#include "record.h"

#include <fcntl.h>
#include <fstream>
#include <ctime>
#ifdef _WIN32
#include <io.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

extern club_handler* club_handler_;
extern status* status_;
extern spec_data* spec_data_;
extern bool closing_;
extern string VENDOR;
extern string PROGRAM_ID;
extern string default_user_directory_;

// Constructor - Download new data and build database
cty1_data::cty1_data() :
	cty_data(),
	parse_result_(nullptr),
	qso_(nullptr)
{
	capabilities_ = HAS_HISTORY;
	// Get the filename
	string filename = get_filename();
	if (!data_valid(filename)) {
		// Exception list has expired or didn't exist
		status_->misc_status(ST_NOTE, "CTY DATA: Downloading exception file again as doesn't exist or is > 7 days old");
		club_handler_->download_exception(filename);
		filename.replace(filename.length() - 3, 3, "xml");
	}
	// Load the exception data
	load_data(filename);
}

// Destructor
cty1_data::~cty1_data() {
	delete_contents();
}

// Remove all the entries in the database
void cty1_data::delete_contents() {
	// Delete all the exception entries
	for (auto it1 = entries_.begin(); it1 != entries_.end(); it1++) {
		list<exc_entry*>* list_entries = &(it1->second);
		for (auto it2 = list_entries->begin(); it2 != list_entries->end(); it2++) {
			delete (*it2);
		}
		list_entries->clear();
	}
	entries_.clear();
	// Delete all the invali entries
	for (auto it1 = invalids_.begin(); it1 != invalids_.end(); it1++) {
		list<invalid_entry*>* list_invalids = &(it1->second);
		for (auto it2 = list_invalids->begin(); it2 != list_invalids->end(); it2++) {
			delete (*it2);
		}
		list_invalids->clear();
	}
	invalids_.clear();
	// Delete all the exception entries
	for (auto it1 = zones_.begin(); it1 != zones_.end(); it1++) {
		list<zone_entry*>* list_zones = &(it1->second);
		for (auto it2 = list_zones->begin(); it2 != list_zones->end(); it2++) {
			delete (*it2);
		}
		list_zones->clear();
	}
	zones_.clear();
	// Delete all the invalid entries
}

// Return the cty1_data entry for the record (call and date) - nullptr if one doesn't exist
cty1_data::exc_entry* cty1_data::except(record* qso) {
	// Does the call exist in the list
	string call = qso->item("CALL");
	time_t timestamp = qso->timestamp();
	if (entries_.find(call) == entries_.end()) {
		// The call is not mentioned at all
		return nullptr;
	}
	else {
		list<exc_entry*>* list_entries = &(entries_.at(call));
		// Exceptions are time limited - return the entry if the call was in the appropriate time-period
		for (auto it = list_entries->begin(); it != list_entries->end(); it++) {
			exc_entry* entry = *it;
			if ((entry->end == -1 || entry->end > timestamp) && 
				(entry->start == -1 || timestamp > entry->start)) {
				char message[160];
				snprintf(message, 160, "CTY DATA: Contact %s at %s %s is in exceptions list in DXCC %d (%s)", 
					call.c_str(), qso->item("QSO_DATE").c_str(), 
					qso->item("TIME_ON").c_str(), entry->adif_id, 
					spec_data_->entity_name(entry->adif_id).c_str());
				status_->misc_status(ST_NOTE, message);
				return entry;
			}
		}
		// Does not match any of the timeframes
		return nullptr;
	}
}

// Check timeliness of data
bool cty1_data::data_valid(string filename) {
#ifdef _WIN32
	int fd = _sopen(filename.c_str(), _O_RDONLY, _SH_DENYNO);
#else
	int fd = open(filename.c_str(), O_RDONLY);
#endif
	if (fd == -1) {
		// File doesn't exist
		return false;
	}
	// Get file status
#ifdef _WIN32
	struct _stat status;
	_fstat(fd, &status);
	_close(fd);
#else
	struct stat status;
	fstat(fd, &status);
	close(fd);
#endif
	time_t now;
	time(&now);
	if (difftime(now, status.st_mtime) > 7 * 24 * 3600) {
		// File is over 7 days old
		return false;
	}
	else {
		return true;
	}
}

// Is the call in the list of invalid calls - unauthorised DXpeditions
bool cty1_data::invalid(record* qso) {
	string call = qso->item("CALL");
	time_t timestamp = qso->timestamp();
	if (invalids_.find(call) == invalids_.end()) {
		// The call is not mentioned at all
		return false;
	}
	else {
		list<invalid_entry*>* list_invalids = &(invalids_.at(call));
		// Exceptions are time limited - return the entry if the call was in the appropriate time-period
		for (auto it = list_invalids->begin(); it != list_invalids->end(); it++) {
			invalid_entry* inval = *it;
			if ((inval->end == -1 || inval->end > timestamp) &&
				(inval->start == -1 || timestamp > inval->start)) {
				char message[160];
				snprintf(message, 160, "CTY DATA: Contact %s at %s %s is an invalid operation", 
					call.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str());
				status_->misc_status(ST_NOTE, message);
				return true;
			}
		}
		// Does not match any of the timeframes
		return false;
	}

}

// Is the call  in the list of zone_exceptions - return 0 if not and the zone if it is
cty1_data::zone_entry* cty1_data::zone_except(record* qso) {
	string call = qso->item("CALL");
	time_t timestamp = qso->timestamp();
	if (zones_.find(call) == zones_.end()) {
		// The call is not mentioned at all
		return nullptr;
	}
	else {
		list<zone_entry*>* list_zones = &(zones_.at(call));
		// Exceptions are time limited - return the entry if the call was in the appropriate time-period
		for (auto it = list_zones->begin(); it != list_zones->end(); it++) {
			zone_entry* zone = *it;
			if ((zone->end == -1 || zone->end > timestamp) &&
				(zone->start == -1 || timestamp > zone->start)) {
				char message[160];
				snprintf(message, 160, "CTY DATA: Contact %s at %s %s is in zone %d", 
					call.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str(), zone->cq_zone);
				status_->misc_status(ST_NOTE, message);
				return zone;
			}
		}
	}
	return nullptr;
}

// Load data
bool cty1_data::load_data(string filename) {
	ifstream is(filename.c_str(), ios_base::in);
	// File is either XML (a new download) or TSV (local copy to load faster)
	if (filename.substr(filename.length() - 3) == "xml") {
		cty1_reader* reader = new cty1_reader();
		char message[160];
		snprintf(message, 160, "CTY DATA: Loading exception file %s", filename.c_str());
		status_->misc_status(ST_NOTE, message);
		if (reader->load_data(this, is, file_created_)) {
			// OK - data loaded into database
			snprintf(message, 160, "CTY DATA: Loaded exception file %s", filename.c_str());
			status_->misc_status(ST_OK, message);
			is.close();
		}
		else {
			// Failed - delete what may have been loaded - this ensures no exception is reported
			snprintf(message, 160, "CTY DATA: Failed to load %s", filename.c_str());
			status_->misc_status(closing_ ? ST_WARNING : ST_ERROR, message);
			delete_contents();
			return false;
		}
	}
	else {
		char message[160];
		snprintf(message, 160, "CTY DATA: Not an XML exception file %s", filename.c_str());
		status_->misc_status(ST_ERROR, message);
		return false;
	}
	return true;
}

// Get the filename {REFERENCE DIR}/cty.xml
string cty1_data::get_filename() {
	return default_user_directory_ + "cty.xml";
}

// Parse the record
void cty1_data::parse(record* qso) {
	if (qso != qso_ || qso->item("CALL") != parse_call_) {
		qso_ = qso;
		parse_call_ = qso->item("CALL");
		delete parse_result_;
		parse_result_ = new parse_result;
		parse_result_->invalid = invalid(qso);
		parse_result_->exception = except(qso);
		parse_result_->zone_exception = zone_except(qso);
		parse_result_->prefix = prefix(qso);
	}
}

// Get prefix
cty1_data::prefix_entry* cty1_data::prefix(record* qso) {
	time_t timestamp = qso->timestamp();
	string dxcc = qso->item("DXCC");
	if (dxcc.length()) {
		int idxcc = stoi(dxcc);
		// Find a prefix with that DXCC and use it
		for (auto it1 = prefixes_.begin(); it1 != prefixes_.end(); it1++) {
			for (auto it2 = (*it1).second.begin(); it2 != (*it1).second.end(); it2++) {
				prefix_entry* prefix = *it2;
				if (prefix->adif_id == idxcc) return prefix;
			}
		}
	}
	// Split the callsign into its various components
	vector<string> words;
	split_line(parse_call_, words, '/');
	string suffix = words.back();
	string body = "";
	string alt = "";
	// Try and work out which bit of the call is which
	switch (words.size()) {
	case 1:
		// Simple callsign
		body = words[0];
		suffix = "";
		break;
	case 2:
		if (suffix == "MM") {
			// /MM - not in a DXCC entity so no need to parse the callsign further
			prefix_entry* prefix = new prefix_entry;
			prefix->adif_id = 0;
			return prefix;
		}
		else if (suffix.length() == 1 || suffix == "MM" || suffix == "AM") {
			// Callsign has a roving style suffix - e.g. /M, /1 etc.
			suffix = "";
			body = words[0];
		}
		// Use the longer of the first two as callsign body, the shorter as the prefix of operation
		else if (words[0].length() > words[1].length()) {
			suffix = "";
			body = words[0];
			alt = words[1];
		}
		else {
			suffix = "";
			body = words[1];
			alt = words[0];
		}
		break;
	case 3:
		if (words[0].length() > words[1].length()) {
			body = words[0];
			alt = words[1];
		}
		else {
			body = words[1];
			alt = words[0];
		}
		break;
	}
	// Try alternate 
	if (alt.length()) {
		// Start comparing alt callsign and then reduce in length until found
		for (int len = alt.length(); len > 0; len--) {
			string test = alt.substr(0, len);
			if (prefixes_.find(test) != prefixes_.end()) {
				list<prefix_entry*>& list_entries = prefixes_.at(test);
				// Exceptions are time limited - return the entry if the call was in the appropriate time-period
				for (auto it = list_entries.begin(); it != list_entries.end(); it++) {
					prefix_entry* prefix = *it;
					if ((prefix->end == -1 || prefix->end > timestamp) &&
						(prefix->start == -1 || timestamp > prefix->start)) {
						return prefix;
					}
				}
			}
		}
	}
	// Start comparing full callsign and then reduce in length until found
	for (int len = body.length(); len > 0; len--) {
		// Now try and match the callsign body
		for (auto it = prefixes_.begin(); it != prefixes_.end(); it++) {
			string test = body.substr(0, len);
			if (prefixes_.find(test) != prefixes_.end()) {
				list<prefix_entry*>& list_entries = prefixes_.at(test);
				// Exceptions are time limited - return the entry if the call was in the appropriate time-period
				for (auto it = list_entries.begin(); it != list_entries.end(); it++) {
					prefix_entry* prefix = *it;
					if ((prefix->end == -1 || prefix->end > timestamp) &&
						(prefix->start == -1 || timestamp > prefix->start)) {
						return prefix;
					}
				}
			}
		}
	}
	char message[160];
	snprintf(message, sizeof(message), "CTY DATA: Contact %s at %s %s cannot be parsed",
		parse_call_.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str());
	return nullptr;
}

// Return entity 
int cty1_data::entity(record* qso) {
	parse(qso);
	if (parse_result_->invalid) return -1;
	else if (parse_result_->exception) return parse_result_->exception->adif_id;
	else if (parse_result_->prefix) return parse_result_->prefix->adif_id;
	else {
		char msg[256];
		snprintf(msg, sizeof(msg), "CTY DATA: Contact %s at %s %s has no entity",
			parse_call_.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str());
		return -1;
	}
}

// Return entity prefix
string cty1_data::nickname(record* qso) {
	int adif_id = entity(qso);
	if (adif_id <= 0) return "";
	else return entities_.at(adif_id)->prefix;
}

/// Return entity name
string cty1_data::name(record* qso) {
	int adif_id = entity(qso);
	if (adif_id <= 0) return "";
	else return entities_.at(adif_id)->name;
}

// Return entity continent
string cty1_data::continent(record* qso) {
	parse(qso);
	if (parse_result_->invalid) return "";
	else if (parse_result_->exception) return parse_result_->exception->continent;
	else if (parse_result_->prefix) return parse_result_->prefix->continent;
	else {
		char msg[256];
		snprintf(msg, sizeof(msg), "CTY DATA: Contact %s at %s %s has no continent",
			parse_call_.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str());
		return "";
	}
}

// Return CQ zone
int cty1_data::cq_zone(record* qso) {
	parse(qso);
	if (parse_result_->invalid) return -1;
	else if (parse_result_->exception) return parse_result_->exception->cq_zone;
	else if (parse_result_->zone_exception) return parse_result_->zone_exception->cq_zone;
	else if (parse_result_->prefix) return parse_result_->prefix->cq_zone;
	else {
		char msg[256];
		snprintf(msg, sizeof(msg), "CTY DATA: Contact %s at %s %s has no CQ zone",
			parse_call_.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str());
		return -1;
	}
}

// Not supported
int cty1_data::itu_zone(record* qso) { return 0; }

// Return location (latitude and longitude)
lat_long_t cty1_data::location(record* qso) {
	parse(qso);
	lat_long_t result = { nan(""), nan("") };
	if (parse_result_->invalid) return result;
	else if (parse_result_->exception)
		result = { parse_result_->exception->latitude, parse_result_->exception->longitude };
	else if (parse_result_->prefix)
		result = { parse_result_->prefix->latitude, parse_result_->prefix->longitude };
	else {
		char msg[256];
		snprintf(msg, sizeof(msg), "CTY DATA: Contact %s at %s %s has no location",
			parse_call_.c_str(), qso->item("QSO_DATE").c_str(), qso->item("TIME_ON").c_str());
	}
	return result;
}

// Parse source
cty1_data::parse_source_t cty1_data::get_source(record* qso) {
	parse(qso);
	if (parse_result_->invalid) return INVALID;
	if (parse_result_->exception) return EXCEPTION;
	if (parse_result_->zone_exception) return ZONE_EXCEPTION;
	return DEFAULT;
}

// Update record based on parsing
bool cty1_data::update_qso(record* qso, bool my_call) {
	// Remove previous QSO as it falsely keeps previous parse result.
	qso_ = nullptr;

	parse(qso);
	// Use the values in the exceptions entry
	if (my_call) {
		qso->item("MY_DXCC", to_string(entity(qso_)));
		if (cq_zone(qso_) > 0) qso->item("MY_CQ_ZONE", to_string(cq_zone(qso_)));
		qso->item("MY_COUNTRY", name(qso));
		qso->item("APP_ZZA_MY_CONT", continent(qso));
	}
	else {
		qso->item("DXCC", to_string(entity(qso_)));
		if (cq_zone(qso_) > 0) qso->item("CQZ", to_string(cq_zone(qso_)));
		qso->item("COUNTRY", name(qso));
		qso->item("CONT", continent(qso));
		qso->update_bearing();
	}
	if (parse_result_->invalid) return false;
	return true;
}

// Get location details
string cty1_data::get_tip(record* qso) {
	parse(qso);
	string message;
	char text[160];
	snprintf(text, sizeof(text), "%s: %s", nickname(qso).c_str(), name(qso).c_str());
	switch (get_source(qso)) {
	case INVALID:
		message = "Unauthorised callsign";
		break;
	case EXCEPTION:
		message = "Details from exception list\n" + string(text);
		break;
	case ZONE_EXCEPTION:
	case DEFAULT:
		message = "Details from prefix list\n" + string(text);
		break;
	}
	return message;
}

// Get default name for entity n
string cty1_data::name(int adif_id) {
	if (adif_id == 0) {
		return "";
	}
	else if (adif_id == -1) {
		status_->misc_status(ST_WARNING, "CTY DATA: -1 is an invalid DXCC");
		return "";
	}
	if (entities_.find(adif_id) == entities_.end()) {
		char msg[160];
		snprintf(msg, sizeof(msg), "CTY DATA: %d is not a valid DXCC entity number", adif_id);
		status_->misc_status(ST_ERROR, msg);
		return "";
	}
	else {
		return entities_.at(adif_id)->name;
	}
}

// Get the continent for entity n
string cty1_data::continent(int adif_id) {
	if (adif_id == 0) {
		return "";
	}
	else if (adif_id == -1) {
		status_->misc_status(ST_WARNING, "CTY DATA: -1 is an invalid DXCC");
		return "";
	}
	if (entities_.find(adif_id) == entities_.end()) {
		char msg[160];
		snprintf(msg, sizeof(msg), "CTY DATA: %d is not a valid DXCC entity number", adif_id);
		status_->misc_status(ST_ERROR, msg);
		return "";
	}
	else {
		return entities_.at(adif_id)->continent;
	}
}

// Return the nickname for entity n
string cty1_data::nickname(int adif_id) {
	if (adif_id == 0) {
		return "";
	} 
	else if (adif_id == -1) {
		status_->misc_status(ST_WARNING, "CTY DATA: -1 is an invalid DXCC");
		return "";
	}
	if (entities_.find(adif_id) == entities_.end()) {
		char msg[160];
		snprintf(msg, sizeof(msg), "CTY DATA: %d is not a valid DXCC entity number", adif_id);
		status_->misc_status(ST_ERROR, msg);
		return "";
	}
	else {
		return entities_.at(adif_id)->prefix;
	}
}

// Return the default CQ zone for entity n
int cty1_data::cq_zone(int adif_id) {
	if (adif_id == 0) {
		return 0;
	}
	else if (adif_id == -1) {
		status_->misc_status(ST_WARNING, "CTY DATA: -1 is an invalid DXCC");
		return 0;
	}
	if (entities_.find(adif_id) == entities_.end()) {
		char msg[160];
		snprintf(msg, sizeof(msg), "CTY DATA: %d is not a valid DXCC entity number", adif_id);
		status_->misc_status(ST_ERROR, msg);
		return 0;
	}
	else {
		return entities_.at(adif_id)->cq_zone;
	}
};

// Return the entity number for the nickname
int cty1_data::entity(string nickname) {
	for (auto it = entities_.begin(); it != entities_.end(); it++) {
		if ((*it).second->prefix == nickname) {
			return (*it).first;
		}
	}
	return -1;
}


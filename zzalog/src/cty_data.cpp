#include "cty_data.h"

#include "club_handler.h"
#include "cty_element.h"
#include "cty1_reader.h"
#include "cty2_reader.h"
#include "cty3_reader.h"
#include "record.h"
#include "spec_data.h"
#include "status.h"

#include <cctype>
#include <chrono>
#include <ostream>
#include <fstream>
#include <string>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

extern club_handler* club_handler_;
extern spec_data* spec_data_;
extern status* status_;
extern string default_data_directory_;
extern bool DEBUG_PARSE;

cty_data::cty_data() {
	data_ = new all_data;
	import_ = new all_data;
	now_ = chrono::system_clock::now();
	bool loaded;
	string rep_fn = default_data_directory_ + "cty_load.rpt";
	os_.open(rep_fn);
	os_ << "Loading from ADIF\n";
	// Load all the daat
	type_ = ADIF;
	load_data("");
	merge_data();
	dump_database();
	delete_data(import_);
	os_ << "Loading from Clublog.org\n";
	type_ = CLUBLOG;
	string filename = get_filename();
	loaded = load_data(filename);
	merge_data();
	dump_database();
	delete_data(import_);
	if (loaded) timestamps_[type_] = get_timestamp(filename, 7);
	else timestamps_[type_] = chrono::system_clock::from_time_t(-1);
	os_ << "Loading from Country-files.com\n";
	type_ = COUNTRY_FILES;
	filename = get_filename();
	loaded = load_data(filename);
	merge_data();
	dump_database();
	delete_data(import_);
	if (loaded) timestamps_[type_] = get_timestamp(filename, 7);
	else timestamps_[type_] = chrono::system_clock::from_time_t(-1);
	os_ << "Loading from DxAtlas\n";
	type_ = DXATLAS;
	filename = get_filename();
	loaded = load_data(filename);
	merge_data();
	dump_database();
	delete_data(import_);
	if (loaded) timestamps_[type_] = get_timestamp(filename, 365);
	else timestamps_[type_] = chrono::system_clock::from_time_t(-1);
	os_.close();

}

cty_data::~cty_data() {
	delete_data(data_);
	delete_data(import_);
}

void cty_data::delete_data(all_data* data) {
	if (data) {
		for (auto it : data->entities) {
			delete it.second;
		}
		data->entities.clear();
		for (auto it : data->prefixes) {
			for (auto ita : it.second) {
				delete ita;
			}
		}
		data->prefixes.clear();
		for (auto it : data->exceptions) {
			for (auto ita : it.second) {
				delete ita;
			}
		}
		data->exceptions.clear();
		//for (auto it : data->filters) {
		//	delete it;
		//}
		//data->filters.clear();
	}
}

cty_exception* cty_data::exception() {
	if (parse_result_.decode_element == nullptr) return nullptr;
	if (parse_result_.decode_element->type_ == cty_element::CTY_EXCEPTION) {
		return (cty_exception*)parse_result_.decode_element;
	}
	else {
		return nullptr;
	}
}

cty_prefix* cty_data::prefix() {
	if (parse_result_.decode_element == nullptr) return nullptr;
	if (parse_result_.decode_element->type_ == cty_element::CTY_PREFIX) {
		return (cty_prefix*)parse_result_.decode_element;
	}
	else {
		return nullptr;
	}
}

// Return various fields of entity
string cty_data::nickname(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->nickname_;
	return "";
}

string cty_data::name(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->name_;
	else return "";
}

string cty_data::continent(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->continent_;
	else return "";
}

int cty_data::cq_zone(record* qso) {
	parse(qso);
	// Check exception
	cty_exception* except = exception();
	if (except && except->cq_zone_ >= 0) return except->cq_zone_;
	// Check if any geographic sub-entity has CQ Zone 
	cty_prefix* pfx = prefix();
	string call = qso->item("CALL");
	if (pfx && parse_result_.geography) {
		if (parse_result_.geography->cq_zone_ >= 0) return parse_result_.geography->cq_zone_;
	}
	if (pfx && pfx->cq_zone_ >= 0) return pfx->cq_zone_;
	// Not overridden
	if (parse_result_.entity) return parse_result_.entity->cq_zone_;
	// Default
	return -1;
}

int cty_data::itu_zone(record* qso) {
	parse(qso);
	// Check exception
	cty_exception* except = exception();
	if (except && except->itu_zone_ >= 0) return except->itu_zone_;
	// Check if any geographic sub-entity has CQ Zone 
	cty_prefix* pfx = prefix();
	string call = qso->item("CALL");
	if (pfx && parse_result_.geography) {
		if (parse_result_.geography->itu_zone_ >= 0) return parse_result_.geography->itu_zone_;
	}
	if (pfx && pfx->itu_zone_ >= 0) return pfx->itu_zone_;
	// Not overridden
	if (parse_result_.entity) return parse_result_.entity->itu_zone_;
	return -1;
}

// Get location
lat_long_t cty_data::location(record* qso) {
	parse(qso);
	// Check exception
	cty_exception* except = exception();
	if (except && !(except->coordinates_.is_nan())) return except->coordinates_;
	if (parse_result_.entity) return parse_result_.entity->coordinates_;
	else return { nan(""), nan("") };
}

// Get geography
string cty_data::geography(record* qso) {
	string result;
	parse(qso);
	if (parse_result_.geography) {
		result = parse_result_.geography->nickname_ + ": " + parse_result_.geography->name_;
	}
	else {
		result = "Geography N/A";
	}
	return result;
}

// Get usage
string cty_data::usage(record* qso) {
	string result;
	parse(qso);
	if (parse_result_.usage) {
		result = parse_result_.usage->nickname_ + ": " + parse_result_.usage->name_;
	}
	else {
		result = "Usage N/A";
	}
	return result;
}

// Update record based on parsing
bool cty_data::update_qso(record* qso, bool my_call) {
	// Remove previous QSO as it falsely keeps previous parse result.
	current_qso_ = nullptr;

	parse(qso);
	// Use the values in the exceptions entry
	if (my_call) {
		qso->item("MY_DXCC", to_string(entity(current_qso_)));
		if (cq_zone(current_qso_) > 0) qso->item("MY_CQ_ZONE", to_string(cq_zone(current_qso_)));
		qso->item("MY_COUNTRY", name(current_qso_));
		qso->item("APP_ZZA_MY_CONT", continent(current_qso_));
	}
	else {
		qso->item("DXCC", to_string(entity(current_qso_)));
		if (cq_zone(current_qso_) > 0) qso->item("CQZ", to_string(cq_zone(current_qso_)));
		qso->item("COUNTRY", name(current_qso_));
		qso->item("CONT", continent(current_qso_));
		qso->update_bearing();
	}
	return true;
}

// Get location details
string cty_data::get_tip(record* qso) {
	parse(qso);
	string message;
	char text[160];
	snprintf(text, sizeof(text), "%s: %s", nickname(qso).c_str(), name(qso).c_str());
	switch (get_source(qso)) {
	case INVALID:
		message = "Unauthorised callsign";
		break;
	case PREVIOUS:
		message = "Obtained from QSO record";
		break;
	case NO_DECODE:
		message = "Unable to decode callsign";
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

// Parsing source
cty_data::parse_source_t cty_data::get_source(record* qso) {
	parse(qso);
	// If no parse result
	if (parse_result_.entity == nullptr) return NO_DECODE;
	// If an exception
	cty_exception* except = exception();
	if (except && (
		( except->cq_zone_ >= 0 && except->cq_zone_ != parse_result_.entity->cq_zone_) || 
		( except->itu_zone_ >= 0 && except->itu_zone_ != parse_result_.entity->itu_zone_))) return ZONE_EXCEPTION;
	if (except) return EXCEPTION;
	cty_prefix* pfx = prefix();
	string call = qso->item("CALL");
	if (pfx && parse_result_.geography) {
		cty_geography* it = parse_result_.geography;
		if ((it->cq_zone_ >= 0 && it->cq_zone_ != parse_result_.entity->cq_zone_) ||
			(it->itu_zone_ >= 0 && it->itu_zone_ != parse_result_.entity->itu_zone_)) return ZONE_EXCEPTION;
	}
	if (pfx && (
		(pfx->cq_zone_ >= 0 && pfx->cq_zone_ != parse_result_.entity->cq_zone_) ||
		(pfx->itu_zone_ >= 0 && pfx->itu_zone_ != parse_result_.entity->itu_zone_))) return ZONE_EXCEPTION;
	// Default from entity
	return DEFAULT;
}

// Return entity 
int cty_data::entity(record* qso) {
	parse(qso);
	// We should have the entity descriptor but may not
	if (parse_result_.entity) return parse_result_.entity->dxcc_id_;
	// No decode
	return -1;
}

// Get entity for nickname and vice-versa
int cty_data::entity(string nickname) {
	if (data_) {
		for (auto it : data_->entities) {
			if (it.second->nickname_ == nickname) {
				return it.first;
			}
		}
	}
	return -1;
}

string cty_data::nickname(int adif_id) {
	if (data_) {
		if (data_->entities.find(adif_id) == data_->entities.end()) {
			return data_->entities.at(adif_id)->nickname_;
		}
	}
	return "";
}

// Load the data 
bool cty_data::load_data(string filename) {
	if (type_ == ADIF) {
		load_adif_data();
		return true;
	}
	// else
	char msg[128];
	ifstream in(filename.c_str(), ios_base::in);
	string version;
	bool ok;
	switch (type_) {
	case CLUBLOG: {
		cty1_reader* reader = new cty1_reader;
		import_ = new all_data;
		status_->misc_status(ST_NOTE, "CTY DATA: Loading data supplied by clublog.org");
		ok = reader->load_data(this, in, version);
		break;
	}
	case COUNTRY_FILES: {
		cty2_reader* reader = new cty2_reader;
		import_ = new all_data;
		status_->misc_status(ST_NOTE, "CTY DATA: Loading data supplied by www.country-files.com");
		ok = reader->load_data(this, in, version);
		// Get version from database
		int vdxcc = import_->exceptions.at("VERSION").front()->dxcc_id_;
		version = import_->entities.at(vdxcc)->name_ + ", " +
			import_->entities.at(vdxcc)->nickname_;
		break;
	}
	case DXATLAS: {
		cty3_reader* reader = new cty3_reader;
		import_ = new all_data;
		status_->misc_status(ST_NOTE, "CTY DATA: Loading data supplied by dxatlas.com");
		ok = reader->load_data(this, in, version);
		break;
	}
	default:
		status_->misc_status(ST_WARNING, "Attempting to load unsupported country file");
		return false;
	}
	versions_[type_] = version;

	if (ok) {
		snprintf(msg, sizeof(msg), "CTY DATA: File %s read OK - version: %s", filename.c_str(), version.c_str());
		status_->misc_status(ST_OK, msg);

		return true;
	}
	else {
		snprintf(msg, sizeof(msg), "CTY DATA: Failed to read %s", filename.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

// Get the filename
string cty_data::get_filename() {
	switch (type_) {
	case COUNTRY_FILES:
		return default_data_directory_ + "cty.csv";
	case CLUBLOG:
		return default_data_directory_ + "cty.xml";
	case DXATLAS:
		return default_data_directory_ + "Prefix.lst";
	default:
		return "";
	}
}

// Find the entity, pattern and sub-patterns for the supplied QSO
void cty_data::parse(record* qso) {
	if (data_) {
		if (qso == current_qso_ && qso->item("CALL") == current_call_) {
			return;
		}
		// else 
		current_qso_ = qso;
		current_call_ = qso->item("CALL");
		string when = qso->item("QSO_DATE") + qso->item("TIME_ON").substr(0,4);
		int dxcc_id;
		qso->item("DXCC", dxcc_id);
		if (DEBUG_PARSE) printf("%s: QSO has DXCC %d\n", current_call_.c_str(), dxcc_id);
		string matched_call;
		parse_result_.decode_element = match_pattern(current_call_, when, matched_call);
		
		if (parse_result_.decode_element) {
			int dxcc_id = parse_result_.decode_element->dxcc_id_;
			if (data_->entities.find(dxcc_id) != data_->entities.end()) {
				parse_result_.entity = data_->entities[dxcc_id];
				if (parse_result_.entity) {
					parse_result_.geography = (cty_geography*)match_filter(parse_result_.entity, cty_filter::FT_GEOGRAPHY, matched_call, when);
					parse_result_.usage = match_filter(parse_result_.entity, cty_filter::FT_USAGE, matched_call, when);
				} else {
					parse_result_.geography = nullptr;
					parse_result_.usage = nullptr;
				}
			}
			else {
				parse_result_.entity = nullptr;
				parse_result_.geography = nullptr;
				parse_result_.usage = nullptr;
			}
		}
		else {
			parse_result_.entity = nullptr;
			parse_result_.geography = nullptr;
			parse_result_.usage = nullptr;
		}
	}
	else {
		status_->misc_status(ST_WARNING, "No country data is loaded");
	}
}

cty_element* cty_data::match_pattern(string call, string when, string& matched_call) {
	// Look in exceptions
	if (data_->exceptions.find(call) != data_->exceptions.end()) {
		list<cty_exception*>& exceptions = data_->exceptions.at(call);
		for (auto it : exceptions) {
			if (it->time_contains(when)) {
				if (DEBUG_PARSE) {
					printf("%s - Exception %s CQ%d ITU%d\n", call.c_str(), it->name_.c_str(), it->cq_zone_, it->itu_zone_);
				}
				return it;
			}
		}
	}
	// Otherwise start looking in prefixes
	string alt;
	string body;
	// Split call accoridng to slashes in it. 
	split_call(call, alt, body);
	if (alt.length()) {
		cty_element* pfx = match_prefix(alt, when);
		matched_call = alt;
		if (pfx && pfx->dxcc_id_ > 0) return pfx;
	}
	matched_call = body;
	return match_prefix(body, when);
}

cty_element* cty_data::match_prefix(string call, string when) {
	// Start matching prefixes - from full length of call down
	string test = call;
	while (test.length() > 0) {
		if (data_->prefixes.find(test) != data_->prefixes.end()) {
			list<cty_prefix*>& prefixes = data_->prefixes.at(test);
			for (auto it : prefixes) {
				if (it->time_contains(when)) {
					// Prefix has a match
					return it;
				}
			}
		}
		test = test.substr(0, test.length() - 1);
	}
	return nullptr;
}

cty_filter* cty_data::match_filter(cty_element* element, cty_filter::filter_t type, string call, string when) {
	if (DEBUG_PARSE) printf("DEBUG: Match call %s: \n", call.c_str());
	for (auto it : element->filters_) {
		// Check filter type
		if (it->pattern_.length()) {
			if (DEBUG_PARSE) printf("DEBUG: Against pattern %s (%s:%s) for type %d\n", 
				it->pattern_.c_str(), 
				it->nickname_.c_str(),
				it->name_.c_str(),
				(int)type);
			// Now match character by character
			bool match = true;
			bool found = false;
			size_t pos_c = 0; // Position on call
			bool seq = false;
			bool braced = false;
			bool brace_match = false;
			char last_c = '\0';
			for (int ix = 0; ix < it->pattern_.length() && !found; ix++) {
				if (match || it->pattern_[ix] == ',') {
					switch (it->pattern_[ix]) {
					case '[':
						// Starting braced code
						braced = true;
						brace_match = false;
						seq = false;
						break;
					case ']':
						// Ending braced code
						braced = false;
						if (!brace_match) match = false;;
						pos_c++;
						break;
					case '-':
						// Staring a sequence of letters or numbers
						if (braced) seq = true;
						break;
					case ',':
						// Reached the end of a pattern - we match then look no further
						if (match) found = true;
						// Otherwise start again
						else {
							match = true;
							pos_c = 0;
						}
						break;
					case '.':
						// Must match the call execatly
						if (pos_c != call.length()) match = false;
						break;
					case '#':
						// Numeric
						if (braced) {
							if (isdigit(call[pos_c])) brace_match = true;
						}
						else {
							if (!isdigit(call[pos_c])) match = false;
							pos_c++;
						}
						break;
					case '@':
						if (braced) {
							if (isalpha(call[pos_c])) brace_match = true;
						}
						else {
							if (!isalpha(call[pos_c])) match = false;
							pos_c++;
						}
						break;
					case '?':
						if (braced) {
							if (isalnum(call[pos_c])) brace_match = true;
						}
						else {
							if (!isalnum(call[pos_c])) match = false;
							pos_c++;
						}
						break;
					default:
						if (braced) {
							if (seq) {
								brace_match |= (call[pos_c] > last_c && call[pos_c] <= it->pattern_[ix]);
							}
							else {
								brace_match |= (call[pos_c] == it->pattern_[ix]);
								last_c = it->pattern_[ix];
							}
							seq = false;
						}
						else {
							match &= (call[pos_c] == it->pattern_[ix]);
							pos_c++;
						}
						break;
					}
					if (DEBUG_PARSE) printf("%s vs %s - %s %s\n",
						it->pattern_.substr(0, ix + 1).c_str(),
						call.substr(0, pos_c).c_str(),
						brace_match ? "Brace match" : "",
						match ? "match" : "no match");
				}
			}
			if (match) {
				if (DEBUG_PARSE) printf(" - matched\n");
				cty_filter* subfilter = match_filter(it, type, call, when);
				if (subfilter) return subfilter;
				else if (it->filter_type_ == type)	return it;
			} else {
				if (DEBUG_PARSE) printf("\n");
			}
		}
	}
	return nullptr;
}

void cty_data::split_call(string call, string& alt, string& body) {
	// Split the callsign into its various components
	vector<string> words;
	split_line(call, words, '/');
	string suffix = words.back();
	// Try and work out which bit of the call is which
	switch (words.size()) {
	case 1:
		// Simple callsign
		body = words[0];
		alt = "";
		suffix = "";
		break;
	case 2:
		if (suffix == "MM") {
			// /MM - not in a DXCC entity so no need to parse the callsign further
			// TODO: Needs further work
			return;
		}
		else if (suffix.length() == 1 || suffix == "MM" || suffix == "AM") {
			// Callsign has a roving style suffix - e.g. /M, /1 etc.
			suffix = "";
			body = words[0];
			alt = "";
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

}

// Add an entity to import data
void cty_data::add_entity(cty_entity* entry) {
	int dxcc = entry->dxcc_id_;
	if (import_->entities.find(dxcc) == import_->entities.end()) {
		import_->entities[dxcc] = entry;
	}
	else {
		os_ << "Multiple entity entry for DXCC " << dxcc << "\n";
		os_ << "Original: " << *(import_->entities.at(dxcc)) << "\n";
		os_ << "Duplicat: " << *entry << "\n";
		report_errors_ = true;
	}
}

// Add a prefix
void cty_data::add_prefix(string pattern, cty_prefix* entry) {
	if (import_->prefixes.find(pattern) == import_->prefixes.end()) {
		import_->prefixes[pattern] = { entry };
	}
	else {
		bool exists = false;
		for (auto ita : import_->prefixes.at(pattern)) {
			if (ita->time_overlap(entry)) {
				os_ << "Overlapping prefix entry for " << pattern << "\n";
				os_ << "Original: " << *ita << "\n";
				os_ << "Overlap : " << *entry << "\n";
				report_warnings_ = true;
				exists = true;

			} 
		}
		if (!exists) {
			import_->prefixes[pattern].push_back(entry);
		}
	}
}

// Add an exception
void cty_data::add_exception(string pattern, cty_exception* entry) {
	if (import_->exceptions.find(pattern) == import_->exceptions.end()) {
		import_->exceptions[pattern] = { entry };
	}
	else {
		bool exists = false;
		for (auto ita : import_->exceptions.at(pattern)) {
			if (ita->time_overlap(entry)) {
				os_ << "Overlapping exception entry for " << pattern << "\n";
				os_ << "Original: " << *ita << "\n";
				os_ << "Overlap : " << *entry << "\n";
				report_warnings_ = true;
				exists = true;
			}
		}
		if (!exists) {
			import_->exceptions[pattern].push_back(entry);
		}
	}
}

// Add a filter
void cty_data::add_filter(cty_element* element, cty_filter* entry) {
	if (element) element->filters_.push_back(entry);
}

// Load entities as defined in the ADIF specification
// TODO: move this to spec_data?
void cty_data::load_adif_data() {
	spec_dataset* dxccs = spec_data_->dataset("DXCC_Entity_Code");
	for (auto it : dxccs->data) {
		int dxcc = stod(it.first);
		string name = it.second->at("Entity Name");
		cty_entity* entry = new cty_entity;
		entry->dxcc_id_ = dxcc;
		entry->name_ = name;
		add_entity(entry);
	}
}

// Dump database
void cty_data::dump_database() {
	os_ << "Contents of data - entities\n";
	for (auto it : data_->entities) {
		os_ << "DXCC " << it.first << ":" << *it.second << "\n";
		for (auto ita : it.second->filters_) {
			os_ << "  FILT " << ita << "\n";
		}
	}
	os_ << "Contents of data - prefixes\n";
	for (auto ita : data_->prefixes) {
		for (auto itb : ita.second) {
			os_ << "PFX " << ita.first << ":" << *itb << "\n";
		}
	}
	os_ << "Contents of data - exceptions\n";
	for (auto ita : data_->exceptions) {
		for (auto itb : ita.second) {
			os_ << "EXCN " << ita.first << ":" << *itb << "\n";
		}
	}
}

// Merge import into data
void cty_data::merge_data() {
	// Merge entities
	for (auto it : import_->entities) {
		if (data_->entities.find(it.first) == data_->entities.end()) {
			os_ << "DXCC " << it.first << " not in data: adding it\n";
			os_ << *it.second << "\n";
			// Need to copy contents rather than pointer
			data_->entities[it.first] = new cty_entity;
			*(data_->entities[it.first]) = *it.second;
		} else {
			cty_element::error_t error = data_->entities.at(it.first)->merge(it.second);
			if (error != cty_element::CE_OK) {
				os_ << "DXCC " << it.first << "clashes ";
				if (error & cty_element::CE_NAME_CLASH) os_ << "Name; ";
				if (error & cty_element::CE_CQ_CLASH) os_ << "CQZ; ";
				if (error & cty_element::CE_ITU_CLASH) os_ << "ITUZ: ";
				if (error & cty_element::CE_CONT_CLASH) os_ << "Cont; ";
				if (error & cty_element::CE_COORD_CLASH) os_ << "Coords: ";
				os_ << "\n";
				os_ << "Original: " << *data_->entities.at(it.first) << "\n";
				os_ << "Merging : " << *it.second << "\n";
			}
		}
	}
	// Merge prefixes
	for (auto it : import_->prefixes) {
		if (data_->prefixes.find(it.first) == data_->prefixes.end()) {
			os_ << "PFX " << it.first << " not in data: adding it\n";
			for (auto ita : it.second) {
				os_ << ita << "\n";
				// Copy the data not the pointer
				cty_prefix* new_pfx = new cty_prefix;
				*new_pfx = *ita;
				data_->prefixes[it.first].push_back(new_pfx);
			}
		}
		else {
			// For each descriptor for imported prefix
			for (auto ita : it.second) {
				bool matches = false;
				// Check against each prefix in data
				for (auto itb : data_->prefixes.at(it.first)) {
					if (itb->time_overlap(ita)) {
						cty_element::error_t error = itb->merge(ita);
						if (error != cty_element::CE_OK) {
							os_ << "Prefix " << it.first << "clashes ";
							if (error & cty_element::CE_NAME_CLASH) os_ << "Name; ";
							if (error & cty_element::CE_CQ_CLASH) os_ << "CQZ; ";
							if (error & cty_element::CE_ITU_CLASH) os_ << "ITUZ: ";
							if (error & cty_element::CE_CONT_CLASH) os_ << "Cont; ";
							if (error & cty_element::CE_COORD_CLASH) os_ << "Coords: ";
							os_ << "\n";
							os_ << "Original: " << *itb << "\n";
							os_ << "Merging : " << *ita << "\n";
						}
						matches = true;
					}
				}
				if (!matches) {
					os_ << "PFX " << ita << " not in data: adding it\n";
					os_ << ita << "\n";
					cty_prefix* new_pfx = new cty_prefix(*ita);
					data_->prefixes[it.first].push_back(new_pfx);
				}
			}
		}
	}
	// Merge exceptions
	for (auto it : import_->exceptions) {
		if (data_->exceptions.find(it.first) == data_->exceptions.end()) {
			os_ << "EXCN " << it.first << " not in data: adding it\n";
			for (auto ita : it.second) {
				os_ << ita << "\n";
				// Copy the data not the pointer
				cty_exception* new_excn = new cty_exception;
				*new_excn = *ita;
				data_->exceptions[it.first].push_back(new_excn);
			}
		}
		else {
			// For each descriptor for imported prefix
			for (auto ita : it.second) {
				bool matches = false;
				// Check against each prefix in data
				for (auto itb : data_->exceptions.at(it.first)) {
					if (itb->time_overlap(ita)) {
						cty_element::error_t error = itb->merge(ita);
						if (error != cty_element::CE_OK) {
							os_ << "Prefix " << it.first << "clashes ";
							if (error & cty_element::CE_NAME_CLASH) os_ << "Name; ";
							if (error & cty_element::CE_CQ_CLASH) os_ << "CQZ; ";
							if (error & cty_element::CE_ITU_CLASH) os_ << "ITUZ: ";
							if (error & cty_element::CE_CONT_CLASH) os_ << "Cont; ";
							if (error & cty_element::CE_COORD_CLASH) os_ << "Coords: ";
							os_ << "\n";
							os_ << "Original: " << *itb << "\n";
							os_ << "Merging : " << *ita << "\n";
						}
						matches = true;
					}
				}
				if (!matches) {
					os_ << "PFX " << ita << " not in data: adding it\n";
					os_ << ita << "\n";
					cty_exception* new_excn = new cty_exception(*ita);
					data_->exceptions[it.first].push_back(new_excn);
				}
			}
		}
	}
}

chrono::system_clock::time_point cty_data::get_timestamp(string filename, int old_age) {
#ifdef _WIN32
	int fd = _sopen(filename.c_str(), _O_RDONLY, _SH_DENYNO);
#else
	int fd = open(filename.c_str(), O_RDONLY);
#endif
	if (fd == -1) {
		// File doesn't exist
		return chrono::system_clock::from_time_t(-1);
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
	chrono::system_clock::time_point ts = chrono::system_clock::from_time_t(status.st_mtime);
	if ((now_ - ts) > chrono::hours(old_age * 24)) {
		char msg[128];
		snprintf(msg, sizeof(msg), "CTY DATA: Data file %s is > %d days old", filename.c_str(), old_age);
		status_->misc_status(ST_WARNING, msg);
	}
	return ts;
}

// Return the recorded timestamp
chrono::system_clock::time_point cty_data::timestamp(cty_type_t type) {
	if (timestamps_.find(type) != timestamps_.end()) {
		return timestamps_[type];
	}
	else {
		return chrono::system_clock::from_time_t(-1);
	}
}

// Fetch the appropriate data
bool cty_data::fetch_data(cty_type_t type) {
	type_ = type;
	string filename = get_filename();
	switch (type) {
	case CLUBLOG:
		return club_handler_->download_exception(filename);
	case COUNTRY_FILES:
		status_->misc_status(ST_WARNING, "CTY DATA: Downloading country-files.com not yet implemented");
		return true;
	case DXATLAS:
		status_->misc_status(ST_WARNING, "CTY DATA: Downloading DxAtlas data not yet implemented");
		return true;
	default:
		return false;	
	}
}

// Return the version
string cty_data::version(cty_type_t type) {
	return versions_.at(type);
}
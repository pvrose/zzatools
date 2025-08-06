#include "cty_data.h"

#include "cty1_reader.h"
#include "cty2_reader.h"
#include "record.h"
#include "status.h"

extern status* status_;
extern string default_user_directory_;

cty_data::cty_data() {
	capabilities_ = NO_DECODE;
	data_ = nullptr;
}

cty_data::~cty_data() {
	delete_data();
}

void cty_data::delete_data() {
	if (data_) {
		for (auto it : data_->entities) {
			delete it.second;
		}
		for (auto it : data_->patterns) {
			for (auto ita : it.second) {
				delete ita;
			}
		}
		for (auto it : data_->sub_patterns) {
			for (auto ita : it.second) {
				delete ita;
			}
		}
	}
}

void cty_data::type(cty_type_t t) {
	type_ = t;
	switch (type_) {
	case INVALID_CTY:
		capabilities_ = NO_DECODE;
		break;
	case CLUBLOG:
		capabilities_ = HAS_HISTORY | HAS_CURRENCY;
		break;
	case COUNTRY_FILES:
		capabilities_ = HAS_ITU_ZONE | HAS_TIMEZONE | HAS_CURRENCY;
		break;
	case DXATLAS:
		capabilities_ = HAS_ITU_ZONE | HAS_TIMEZONE;
		break;
	}
	string filename = get_filename();
	load_data(filename);
}

// Return various fields of entity
string cty_data::nickname(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->nickname;
	return "";
}

string cty_data::name(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->name;
	else return "";
}

string cty_data::continent(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->continent;
	else return "";
}

int cty_data::cq_zone(record* qso) {
	parse(qso);
	// Check if any geographic sub-entity has CQ Zone
	if (parse_result_.sub_patterns) {
		for (auto it : *parse_result_.sub_patterns) {
			if (it->type & CQZ_EXCEPTION) return it->cq_zone;
		}
	}
	// Check if any prefix overrides CQ Zone
	if (parse_result_.pattern) {
		if (parse_result_.pattern->type & CQZ_EXCEPTION) return parse_result_.pattern->cq_zone;
	}
	// return CQ Zone from entiity
	if (parse_result_.entity) {
		return parse_result_.entity->cq_zone;
	}
	// Otherwise invalid
	return -1;
}

int cty_data::itu_zone(record* qso) {
	parse(qso);
	// Check if any geographic sub-entity has CQ Zone
	if (parse_result_.sub_patterns) {
		for (auto it : *parse_result_.sub_patterns) {
			if (it->type & ITUZ_EXCEPTION) return it->itu_zone;
		}
	}
	// Check if any prefix overrides CQ Zone
	if (parse_result_.pattern) {
		if (parse_result_.pattern->type & ITUZ_EXCEPTION) return parse_result_.pattern->itu_zone;
	}
	// return CQ Zone from entiity
	if (parse_result_.entity) {
		return parse_result_.entity->itu_zone;
	}
	// Otherwise invalid
	return -1;
}

// Get location
lat_long_t cty_data::location(record* qso) {
	parse(qso);
	if (parse_result_.entity) return parse_result_.entity->location;
	else return { nan(""), nan("") };
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
	if (parse_result_.pattern == nullptr) return DEFAULT;
	// Parse results has set DXCC excpetion (call outwith usual DXCC for pattern)
	if (parse_result_.pattern->type & DXCC_EXCEPTION) return EXCEPTION;
	// Either the pattern or sub-pattern has indicated a CQ or ITU Zone exception
	if (parse_result_.pattern->type & (CQZ_EXCEPTION | ITUZ_EXCEPTION)) return ZONE_EXCEPTION;
	if (parse_result_.sub_patterns) {
		for (auto it : *parse_result_.sub_patterns) {
			if (it->type & (CQZ_EXCEPTION | ITUZ_EXCEPTION)) return ZONE_EXCEPTION;
		}
	}
	// Otherwise its the entity description
	return DEFAULT;
}

// Return entity 
int cty_data::entity(record* qso) {
	parse(qso);
	// Pattern indicates different DXCC from entity decode
	if (parse_result_.pattern && (parse_result_.pattern->type & DXCC_EXCEPTION))
		return parse_result_.pattern->dxcc_id;
	// Otherwise take from entity decode
	if (parse_result_.entity) return parse_result_.entity->dxcc_id;
	// No decode
	return -1;
}

// Get entity for nickname and vice-versa
int cty_data::entity(string nickname) {
	if (data_) {
		for (auto it : data_->entities) {
			if (it.second->nickname == nickname) {
				return it.first;
			}
		}
	}
	return -1;
}

string cty_data::nickname(int adif_id) {
	if (data_) {
		if (data_->entities.find(adif_id) == data_->entities.end()) {
			return data_->entities.at(adif_id)->nickname;
		}
	}
	return "";
}

// Load the data 
bool cty_data::load_data(string filename) {
	delete_data();
	char msg[128];
	ifstream in(filename.c_str(), ios_base::in);
	string version;
	bool ok;
	switch (type_) {
	case CLUBLOG: {
		cty1_reader* reader = new cty1_reader;
		data_ = new all_data;
		status_->misc_status(ST_NOTE, "CTY DATA: Loading data supplied by clublog.org");
		ok = reader->load_data(data_, in, version);
		break;
	}
	case COUNTRY_FILES: {
		cty2_reader* reader = new cty2_reader;
		data_ = new all_data;
		status_->misc_status(ST_NOTE, "CTY DATA: Loading data supplied by www.country-files.com");
		ok = reader->load_data(data_, in, version);
		break;
	}
	default:
		status_->misc_status(ST_WARNING, "Attempting to load unsupported country file");
		return false;
	}

	if (ok) {
		snprintf(msg, sizeof(msg), "CTY DATA: File %s read OK", filename.c_str());
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
		return default_user_directory_ + "bigcty/cty.csv";
	case CLUBLOG:
		return default_user_directory_ + "cty.xml";
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
		time_t when = qso->timestamp();
		int dxcc_id;
		qso->item("DXCC", dxcc_id);
		// If the DXCC has already been found
		//if (data_->entities.find(dxcc_id) != data_->entities.end()) {
		//	// Possible matching patterns
		//	ent_entry* entity = data_->entities.at(dxcc_id);
		//	parse_result_.entity = entity;
		//	patt_matches* matches = match_pattern(current_call_, entity->patterns, when);
		//	parse_result_.pattern = matches->size() ? matches->front() : nullptr;
		//	parse_result_.sub_patterns = match_pattern(current_call_, entity->sub_patterns, when);
		//	delete matches;
		//}
		//else {
			patt_matches* matches = match_pattern(current_call_, data_->patterns, when);
			if (matches && matches->size()) {
				parse_result_.pattern = matches->front();
				ent_entry* entity = data_->entities.at(parse_result_.pattern->dxcc_id);
				parse_result_.entity = entity;
				parse_result_.sub_patterns = match_pattern(current_call_, entity->sub_patterns, when);
			}
			else {
				parse_result_ = { nullptr, nullptr, nullptr };
			}
			delete matches;
		//}
	}
	else {
		status_->misc_status(ST_WARNING, "No country data is loaded");
	}
}

cty_data::patt_matches* cty_data::match_pattern(string call, map<string, patt_matches> patterns, time_t when) {
	patt_matches* result = new patt_matches;
	// First look for a match on the whole call
	if (patterns.find(call) != patterns.end()) {
		for (auto it : patterns.at(call)) {
			if (valid_pattern(it, when)) {
				result->push_back(it);
			}
		}
		return result;
	}
	// else
	// Split the callsign into its various components
	vector<string> words;
	split_line(call, words, '/');
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
			// TODO: Needs further work
			return nullptr;
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
			if (patterns.find(test) != patterns.end()) {
				for (auto it : patterns.at(test)) {
					if (valid_pattern(it, when)) {
						result->push_back(it);
					}
				}
				return result;
			}
		}
	}
	// Start comparing full callsign and then reduce in length until found
	for (int len = body.length(); len > 0; len--) {
		// Now try and match the callsign body
		string test = body.substr(0, len);
		if (patterns.find(test) != patterns.end()) {
			for (auto it : patterns.at(test)) {
				if (valid_pattern(it, when)) {
					result->push_back(it);
				}
			}
			return result;
		}
	}
	char message[160];
	snprintf(message, sizeof(message), "CTY DATA: Contact %s cannot be parsed",
		call.c_str());
	return nullptr;

}

// Is valid pattern
bool cty_data::valid_pattern(patt_entry* entry, time_t when) {
	if (capabilities_ == HAS_HISTORY && (entry->type & TIME_DEPENDENT)) {
		if (when >= entry->validity.start && when <= entry->validity.end) return true;
		else return false;
	}
	else return true;
}

// Is valid entity
bool cty_data::valid_entity(ent_entry* entry, time_t when) {
	if (capabilities_ == HAS_HISTORY && entry->has_validity) {
		if (when >= entry->validity.start && when <= entry->validity.end) return true;
		else return false;
	}
	else return true;
}

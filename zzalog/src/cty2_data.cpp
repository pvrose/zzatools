#include "cty2_data.h"

#include "cty2_reader.h"
#include "record.h"
#include "status.h"

#include <istream>
#include <fstream>

extern status* status_;
extern string default_user_directory_;

cty2_data::cty2_data() {
	cty_data::capabilities_ = (cty_caps_t) (HAS_ITU_ZONE | HAS_TIMEZONE);
	parse_result_ = { nullptr, nullptr };
	data_ = nullptr;
	string filename = get_filename();
	load_data(filename);
}

cty2_data::~cty2_data() {
	if (data_) {
		for (auto it = data_->begin(); it != data_->end(); it++) {
			for (auto iu : (*it).second->patterns) {
				delete iu.second;
			}
			delete (*it).second;
		}
	}
}

// Return various fields of entity
string cty2_data::nickname(record* qso) {
	parse(qso);
	if (parse_result_.pfx_entry) return parse_result_.pfx_entry->nickname;
	else return "";
}

string cty2_data::name(record* qso) {
	parse(qso);
	if (parse_result_.pfx_entry) return parse_result_.pfx_entry->name;
	else return "";
}

string cty2_data::continent(record* qso) {
	parse(qso);
	if (parse_result_.pfx_entry) return parse_result_.pfx_entry->continent;
	else return "";
}

int cty2_data::cq_zone(record* qso) {
	parse(qso);
	if (parse_result_.patt_entry && parse_result_.patt_entry->cqzone > 0) return parse_result_.patt_entry->cqzone;
	else if (parse_result_.pfx_entry) return parse_result_.pfx_entry->cqzone;
	else return 0;
}

int cty2_data::itu_zone(record* qso) {
	parse(qso);
	if (parse_result_.patt_entry && parse_result_.patt_entry->ituzone > 0) return parse_result_.patt_entry->ituzone;
	else if (parse_result_.pfx_entry) return parse_result_.pfx_entry->ituzone;
	else return 0;
}

// Get location
lat_long_t cty2_data::location(record* qso) {
	parse(qso);
	if (parse_result_.pfx_entry) return parse_result_.pfx_entry->location;
	else return { nan(""), nan("") };
}

// Update record based on parsing
bool cty2_data::update_qso(record* qso, bool my_call) {
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
	return true;
}

// Get location details
string cty2_data::get_tip(record* qso) {
	parse(qso);
	string message;
	char text[160];
	snprintf(text, sizeof(text), "%s: %s", nickname(qso).c_str(), name(qso).c_str());
	switch (get_source(qso)) {
	case INVALID:
		message = "Unauthorised callsign";
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
cty_data::parse_source_t cty2_data::get_source(record* qso) {
	parse(qso);
	if (parse_result_.pfx_entry == nullptr || parse_result_.patt_entry == nullptr) return NO_DECODE;
	if (parse_result_.patt_entry->call_exception) return EXCEPTION;
	else if (parse_result_.patt_entry->cqzone > 0 || parse_result_.patt_entry->ituzone > 0) return ZONE_EXCEPTION;
	else return DEFAULT;
}

// Return entity 
int cty2_data::entity(record* qso) {
	parse(qso);
	if (parse_result_.patt_entry) return parse_result_.patt_entry->dxcc;
	else if (qso && qso->item("DXCC").length()) return stoi(qso->item("DXCC"));
	else return -1;
}

// Get entity for nickname and vice-versa
int cty2_data::entity(string nickname) {
	for (auto it : *data_) {
		if (it.second->nickname == nickname) {
			return it.first;
		}
	}
	return -1;
}

string cty2_data::nickname(int adif_id) {
	if (data_->find(adif_id) == data_->end()) {
		return data_->at(adif_id)->nickname;
	}
	return "";
}

bool cty2_data::load_data(string filename) {
	char msg[128];
	ifstream in(filename.c_str(), ios_base::in);
	string version;
	cty2_reader* reader = new cty2_reader;
	data_ = new map<int, prefix_entry*>;
	bool ok = reader->load_data(data_, in, version);
	if (ok) {
		snprintf(msg, sizeof(msg), "CTY DATA: File %s read OK", filename.c_str());
		status_->misc_status(ST_OK, msg);

		// Now accumulate all the patterns
		patterns_.clear();
		for (auto it : *data_) {
			for (auto iu : it.second->patterns) {
				patterns_[iu.first] = iu.second;
			}
		}

		return true;
	}
	else {
		snprintf(msg, sizeof(msg), "CTY DATA: Failed to read %s", filename.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

string cty2_data::get_filename() {
	return default_user_directory_ + "bigcty/cty.csv";
}

void cty2_data::parse(record* qso) {
	if (data_) {
		if (qso != qso_ || qso->item("CALL") != parse_call_) {
			qso_ = qso;
			string dxcc = qso->item("DXCC");
			if (dxcc.length()) {
				int idxcc = stoi(dxcc);
				// Find a prefix with that DXCC and use it
				if (data_->find(idxcc) != data_->end()) {
					parse_result_.pfx_entry = data_->at(idxcc);
					parse_result_.patt_entry = pattern(qso->item("CALL"), data_->at(idxcc)->patterns);
				}
			}
			else {
				parse_result_.patt_entry = pattern(qso->item("CALL"), patterns_);
				if (parse_result_.patt_entry) parse_result_.pfx_entry = data_->at(parse_result_.patt_entry->dxcc);
				else parse_result_.pfx_entry = nullptr;
			}
		}
	}
	else {
		parse_result_ = { nullptr, nullptr };
	}
}

cty2_data::pattern_entry* cty2_data::pattern(string call, map<string, pattern_entry*> patterns) {
	// First try whole call
	if (patterns.find(call) != patterns.end()) {
		return patterns.at(call);
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
				return patterns.at(test);
			}
		}
	}
	// Start comparing full callsign and then reduce in length until found
	for (int len = body.length(); len > 0; len--) {
		// Now try and match the callsign body
		string test = body.substr(0, len);
		if (patterns.find(test) != patterns.end()) {
			return patterns.at(test);
		}
	}
	char message[160];
	snprintf(message, sizeof(message), "CTY DATA: Contact %s cannot be parsed",
		call.c_str());
	return nullptr;

}

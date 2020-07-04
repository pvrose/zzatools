#include "pfx_data.h"
#include "pfx_dialog.h"
#include "pfx_reader.h"

#include "../zzalib/utils.h"
#include "spec_data.h"
#include "status.h"
#include "../zzalib/callback.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_File_Chooser.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern spec_data* spec_data_;
extern status* status_;

// Constructor
pfx_data::pfx_data()
{
	// initialise maps
	clear();
	prefixes_by_nickname_.clear();

	// get data and load it.
	string filename = get_file(false);
	pfx_reader* reader = new pfx_reader;
	// Carry attempting to read the file until successful or user cancels
	bool carry_on = true;
	while (!reader->load_data(*this, filename) && carry_on) {
		status_->misc_status(ST_ERROR, "PREFIX: Unable to load prefix reference - please reselect directory!");
		if (fl_choice("Do you want to continue!", "Yes", "No", nullptr) == 1) {
			carry_on = false;
			status_->misc_status(ST_FATAL, "PREFIX: Load cancelled by user");
		}
		else {
			filename = get_file(true);
		}
	}

	delete reader;

	exceptions_ = new exc_data;

}

// Destructor
pfx_data::~pfx_data()
{
	// delete all prefix records - a prefix will delete its own children
	for (auto it = begin(); it != end(); it++) {
		delete it->second;
	}
	// clear storage in maps
	clear();
	prefixes_by_nickname_.clear();

	delete exceptions_;
}

// get the prefix data filename
string pfx_data::get_file(bool force) {
	// Get the reference data directory from the settings
	Fl_Preferences datapath(settings_, "Datapath");
	string dirname;
	string result;
	char * temp = nullptr;
	// If the path is not in the settings
	if (force || !datapath.get("Reference", temp, "")) {
		// Open a chooser to get it
		Fl_File_Chooser* chooser = new Fl_File_Chooser("", nullptr, Fl_File_Chooser::DIRECTORY,
			"Select reference file directory");
		chooser->callback(cb_chooser, &dirname);
		chooser->textfont(FONT);
		chooser->textsize(FONT_SIZE);
		chooser->show();
		while (chooser->visible()) Fl::wait();
		delete chooser;
		datapath.set("Reference", dirname.c_str());
	}
	else {
		dirname = temp;
	}
	// Concatenate directory name with prefix file name
	result = dirname + PREFIX_FILE;
	if (temp) free(temp);
	return result;

}

// add the prefix to the map of nickname->prefix
void pfx_data::add_pfx_to_nickname(prefix* prefix) {
	prefixes_by_nickname_[prefix->nickname_] = prefix;
}

// Returns the prefix data based on its nickname
prefix* pfx_data::get_prefix(string nickname) {
	if (prefixes_by_nickname_.find(nickname) != prefixes_by_nickname_.end()) {
		return prefixes_by_nickname_[nickname];
	}
	else {
		// It's not there return nullptr
		return nullptr;
	}
}

// Reaturns the prefix data based on the DXCC code
prefix* pfx_data::get_prefix(unsigned int dxcc_code) {
	if (find(dxcc_code) != end()) {
		return at(dxcc_code);
	}
	else {
		// No data for this DXCC code 
		return nullptr;
	}
}

//Returns the prefix data based on DXCC code and state/province code
prefix* pfx_data::get_prefix(unsigned int dxcc_code, string state) {
	// First get the DXCC prefix
	if (find(dxcc_code) != end()) {
		prefix* dxcc_prefix = at(dxcc_code);
		// DXCC record is also supplied state
		if (dxcc_prefix->state_ == state) {
			return dxcc_prefix;
		}
		else {
			// Search children for state
			vector<prefix*>* children = &dxcc_prefix->children_;
				// default to not found
			// For each child prefix 
			for (auto it = children->begin(); it != children->end(); it++) {
				if ((*it)->state_ == state) {
					// if the child has the state return it
					return *it;
				}
				else if ((*it)->state_ == "") {
					// If the child's state field is empty 
					// look in the child's children if that child is not a province
					vector<prefix*>* grandkids = &((*it)->children_);
					// For each grandchild
					for (auto it2 = grandkids->begin(); it2 != grandkids->end(); it2++) {
						// Set the prefix to that whose state is provided
						if ((*it2)->state_ == state) {
							return *it2;
						}
					}
				}
			}
			return nullptr;
		}
	}
	else {
		// DXCC not in map so return "not found"
		return nullptr;
	}

}

// Returns prefix after analysing record
prefix* pfx_data::get_prefix(record* record, bool special) {

	// Get all prefixes that match the callsign
	vector<prefix*> possibles;
	bool ok = all_prefixes(record, &possibles, special);
	switch (possibles.size()) {
	case 0:
		// No prefixes matched
		return nullptr;
		break;
	case 1:
		// One prefix only matched so return it
		return *(possibles.begin());
		break;
	default:
		// More than one prefix matched - so open the dialog to query the user
		pfx_dialog dialog;
		dialog.set_data(&possibles, record->item("CALL"));
		if (dialog.display() == BN_OK) {
			// User selected one to match
			prefix* prefix = dialog.get_prefix();
			// If user did not select one then look for a common entity prefix to use.
			if (prefix == nullptr) {
				// Check all prefixes have common entity
				bool ok = true;
				// For each possible prefix
				for (auto it = possibles.begin(); it != possibles.end(); it++) {
					if ((*it) != nullptr) {
						// Find the ultilate ancestor of the prefix
						while ((*it)->parent_ != nullptr) (*it) = (*it)->parent_;
						// If entity hasn't yet been found or is the same as that previously found
						if (prefix == nullptr || (*it) == prefix) {
							// Set it to this entity
							prefix = (*it);
						}
						else {
							// Entity differs from that previously found
							ok = false;
						}
					}
					else {
						// No entity found at all
						ok = false;
					}
				}
				// If they do return it else return 0
				if (ok) {
					char message[200];
					snprintf(message, 200, "PREFIX - User didn't select prefix for %s, prefix %s used",  record->item("CALL").c_str(), prefix->nickname_.c_str());
					status_->misc_status(ST_WARNING, message);
					return prefix;
				}
				else {
					char message[200];
					snprintf(message, 200, "PREFIX - User didn't select prefix for %s, no common prefix found", record->item("CALL").c_str());
					status_->misc_status(ST_WARNING, message);
					return nullptr;
				}
			}
			else {
				// user selected 1 prefix - return it
				char message[200];
				snprintf(message, 200, "PREFIX - User selected prefix for %s, prefix %s used", record->item("CALL").c_str(), prefix->nickname_.c_str());
				status_->misc_status(ST_NOTE, message);
				return prefix;
			}
		}
		else {
			// User cancelled the dialog
			return nullptr;
		}
		break;
	}
}

// Get all the prefixes that a callsign is parsed to. Returns true if any exist
bool pfx_data::all_prefixes(record* record, vector<prefix*>* prefixes, bool special) {
	string callsign = record->item("CALL");
	string date = record->item("QSO_DATE");
	int dxcc_code = -1;
	record->item("DXCC", dxcc_code);
	// Avoid special case where DXCC code is genuinely 0. If it's 0 because it is empty make -1
	if (dxcc_code == 0 && record->item("DXCC") == "") dxcc_code = -1;
	string state = record->item("STATE");
	// Get the type of callsign parsing needed
	parse_t type = get_parse_type(callsign);
	// Clear any existing data
	prefixes->clear();
	// Callsign to parse having regarded obliques
	string parse_callsign;
	bool do_parse = true;
	switch (type) {
	case PT_PLAIN:
	case PT_COMPOSITE:
	case PT_SPECIAL:
		// Parse the whole callsign
		parse_callsign = callsign;
		break;
	case PT_ROVING:
	case PT_COMPROVING:
		// Parse only the part before the last oblique stroke
	{
		int last_oblique = callsign.find_last_of('/');
		parse_callsign = callsign.substr(0, last_oblique);
		break;
	}
	case PT_MARITIME:
	case PT_COMPMARINE:
		// /MM so force DXCC = 0 (per ADIF spec) - add prefix entry at 0
		prefixes->push_back((*this)[0]);
		do_parse = false;
		break;
	case PT_AREA:
	{
		// replace the last numeral before the obliques with the numeral after the oblique
		size_t last_oblique = callsign.find_last_of('/');
		char area_number = callsign[callsign.length() - 1];
		string temp_call = callsign.substr(0,last_oblique);
		size_t last_pos_number = temp_call.find_first_of("0123456789");
		size_t temp;
		// Look for the last numeric
		while (temp = temp_call.substr(last_pos_number + 1).find_first_of("0123456789") != string::npos) {
			last_pos_number += temp;
		}
		// Replace last numeric with the character afterthe oblique
		parse_callsign = temp_call;
		parse_callsign[last_pos_number] = area_number;
		break;
	}
	case PT_AREAROVING:
	{
		// Strip the /A, /M or /P and replace the last numeral before the oblique with the numeral after the first oblique
		size_t last_oblique = callsign.find_last_of('/');
		string temp_call = callsign.substr(0,last_oblique);
		last_oblique = temp_call.find_last_of('/');
		char area_number = temp_call[temp_call.length() - 1];
		temp_call = callsign.substr(0,last_oblique);
		size_t last_pos_number = temp_call.find_first_of("0123456789");
		size_t temp;
		while (temp = temp_call.substr(last_pos_number + 1).find_first_of("0123456789") != string::npos) {
			last_pos_number += temp;
		}
		parse_callsign = temp_call;
		parse_callsign[last_pos_number] = area_number;
		break;
	}
	case PT_UNPARSED:
		// Cannot parse so don't do it
		do_parse = false;
		break;
	}
	int iRound = 0;
	// Continue 3 rounds or until parsed - 
	if (do_parse) {
		// Search the whole countries table
		bool parsed = false;
		for (auto it = begin(); it != end() && !parsed; it++) {
			unsigned int dxcc_test = -1;
			prefix* dxcc_prefix;
			if (dxcc_code == -1) {
				// If don't already know the DXCC - use the next entry in the table
				dxcc_test = it->first & 0x0000FFFF;
				dxcc_prefix = it->second;
			}
			else {
				// If we do know the DXCC, go straight to the entry
				dxcc_prefix = (*this)[dxcc_code];
				dxcc_test = dxcc_code;
				parsed = true;
			}
			// Generate results for this country
			vector<prefix*> entitys_prefixes;
			// Either the callsign matches the pattern in the prefix or we already know the DXCC code
			// and the record date is within the validity range for the prefix
			if ((match_callsign(dxcc_prefix, parse_callsign) || dxcc_test == dxcc_code) &&
				date >= dxcc_prefix->valid_from_ && date <= dxcc_prefix->valid_to_ &&
				(state == "" || dxcc_prefix->state_ == "" || state == dxcc_prefix->state_)) {
				// Get the children that match
				vector<prefix*> children;
				get_children(children, dxcc_prefix, dxcc_prefix->dxcc_code_, parse_callsign, date, state, special);
				// Add all the matching children to the result
				for(auto it = children.begin(); it != children.end(); it++) {
					entitys_prefixes.push_back(*it);
				}
				// Add the DXCC if it looks like a special call (at least two numbers after country) 
				// as geography based on first number may not be valid
				if (type == PT_SPECIAL) {
					entitys_prefixes.push_back(dxcc_prefix);
				}
				// If no children found - add the DXCC to the result
				if (entitys_prefixes.size() == 0 && !special) {
					entitys_prefixes.push_back(dxcc_prefix);
				}
			}
			// Add this country's result to overall result
			prefixes->insert(prefixes->end(), entitys_prefixes.begin(), entitys_prefixes.end());
		}
	}
	return prefixes->size() > 0;

}

// Return callsign parse type
parse_t pfx_data::get_parse_type(string& callsign) {
	// Test basic callsign just letters and numbers
	if (regex_match(callsign, REGEX_PLAIN)) {
		// prefix plus > 1 number
		if (regex_search(callsign, REGEX_SPECIAL)) {
			return PT_SPECIAL;
		}
		else {
			return PT_PLAIN;
		}
	}
	// callsign/A /P or /M - special case of composite
	else if (regex_match(callsign, REGEX_ROVING)) {
		return PT_ROVING;
	}
	// callsign/MM - special case of composite
	else if (regex_match(callsign, REGEX_MARITIME)) {
		return PT_MARITIME;
	}
	// e.g. callsign/1 - special case of composite
	else if (regex_match(callsign, REGEX_AREA)) {
		return PT_AREA;
	}
	// callsign/suffix or prefix/callsign
	else if (regex_match(callsign, REGEX_COMPOSITE)) {
		return PT_COMPOSITE;
	}
	// e.g. callsign/1/A - special case of comproving
	else if (regex_match(callsign, REGEX_AREAROVING)) {
		return PT_AREAROVING;
	}
	// callsign/suffix/P or prefix/callsign/P
	else if (regex_match(callsign, REGEX_COMPROVING)) {
		return PT_COMPROVING;
	}
	// callsign/suffix/MM
	else if (regex_match(callsign, REGEX_COMPMARINE)) {
		return PT_COMPMARINE;
	}
	// none of the above
	else {
		return PT_UNPARSED;
	}
}

// Get all the children that match callsign and date (geographic or special use)
void pfx_data::get_children(vector<prefix*>& children, prefix* parent, unsigned int dxcc_code, string& callsign, string& date, string& state, bool special) {
	// Get children
	vector<prefix*>* test_children = &parent->children_;
	// Scan the children
	for (auto it = test_children->begin(); it != test_children->end(); it++) {
		// If one matches callsign, valid date and geography or special
		if (match_callsign((*it), callsign) &&
			date >= (*it)->valid_from_ && date <= (*it)->valid_to_ &&
			(state == "" || (*it)->state_ == "" || state == (*it)->state_) &&
			((*it)->type_ == PX_GEOGRAPHY ||
				(*it)->type_ == PX_OLD_GEOGRAPHY ||
				(*it)->type_ == PX_CITY ||
				(*it)->type_ == PX_SPECIAL_USE)) {
			// Get next generation down
			vector<prefix*> grandkids;
			get_children(grandkids, (*it), dxcc_code, callsign, date, state, special);
			// Add all grandchildren that match
			for (auto it2 = grandkids.begin(); it2 != grandkids.end(); it2++) {
				// Test whether it's special or geography
				if ((special && (*it2)->type_ == PX_SPECIAL_USE) ||
					(!special && ((*it2)->type_ == PX_GEOGRAPHY ||
						(*it2)->type_ == PX_OLD_GEOGRAPHY ||
						(*it2)->type_ == PX_CITY ||
						// Special case for Antarctica and UK Antarctic Islands
						((*it2)->type_ == PX_SPECIAL_USE && (dxcc_code == 13 || dxcc_code == 238 || dxcc_code == 240 || dxcc_code == 241))))) {
					children.push_back(*it2);
				}
			}
			// Or the child if there are none or more than 1
			if (grandkids.size() != 1) {
				// Test whether it's special or geography
				if ((special && (*it)->type_ == PX_SPECIAL_USE) ||
					(!special && ((*it)->type_ == PX_GEOGRAPHY ||
						(*it)->type_ == PX_OLD_GEOGRAPHY ||
						(*it)->type_ == PX_CITY ||
						// Special case for Antarctica and UK Antarctic Islands
						((*it)->type_ == PX_SPECIAL_USE && (dxcc_code == 13 || dxcc_code == 238 || dxcc_code == 240 || dxcc_code == 241))))) {
					children.push_back(*it);
				}
			}
		}
	}
}

// Return true if the callsign matches the pattern in the prefix table
bool pfx_data::match_callsign(prefix* prefix, string& callsign) {
	bool match = false;
	for (unsigned int i = 0; i < prefix->patterns_.size() && !match; i++) {
		// Search through all the patterns in the prefix
		const string* test_pattern = &prefix->patterns_[i];
		bool call_matches = true;
		// Scan though the callsign and the pattern in step
		// We are not checking whether callsign gets exhausted before the pattern,
		// so don't match if we go beyond the end of the callsign except for .
		for (unsigned int i1 = 0, i2 = 0; i1 < test_pattern->length() && call_matches; i1++, i2++) {
			char test_char = (*test_pattern)[i1];
			char test_range_start;
			bool paranthesis;
			bool incrementing;
			bool match_in_parenthesis;
			// Select on the pattern character
			switch (test_char) {
			case '#':
				// # - matches any numeric character
				call_matches &= (i2 < callsign.length()) && callsign[i2] >= '0' && callsign[i2] <= '9';
				break;
			case '@':
				// @ - matches any alphabetic character
				call_matches &= (i2 < callsign.length()) && callsign[i2] >= 'A' && callsign[i2] <= 'Z';
				break;
			case '?':
				// ? - matches any character
				call_matches &= true;
				break;
			case '.':
				// . - matches end of callsign
				call_matches &= (i2 >= callsign.length());
				break;
			case '[':
				// [ starts a set of alternates in the pattern
				// Step along the pattern
				i1++;
				paranthesis = true;
				incrementing = false;
				match_in_parenthesis = false;
				// Until we hit the ] character
				while (paranthesis) {
					test_char = (*test_pattern)[i1];
					// Select the pattern character
					switch (test_char) {
					case '-':
						// - starts a range, step along the pattern
						incrementing = true;
						i1++;
						break;
					case ']':
						// ] ends the alternates 
						// we don't have a character to match 
						match_in_parenthesis |= (i2 >= callsign.length());
						paranthesis = false;
						break;
					case '#':
						// # - matches any numeric character
						match_in_parenthesis |= (i2 < callsign.length()) && callsign[i2] >= '0' && callsign[i2] <= '9';
						i1++;
						break;
					case '@':
						// @ - matches any alphabetic character
						match_in_parenthesis |= (i2 < callsign.length()) && callsign[i2] >= 'A' && callsign[i2] <= 'Z';
						i1++;
						break;
					case '?':
						// ? - matches any character
						match_in_parenthesis |= true;
						i1++;
						break;
					case '.':
						//. matches end of callsign
						match_in_parenthesis |= (i2 >= callsign.length());
						i1++;
						break;
					default:
						// Any other character
						if (incrementing) {
							// last character of range - match if the callsign is any character in the range
							for (char c = test_range_start; c <= test_char && !match_in_parenthesis; c++) {
								match_in_parenthesis |= (i2 < callsign.length()) && callsign[i2] == c;
							}
						}
						else {
							// match if the callsign is this character
							test_range_start = test_char;
							match_in_parenthesis |= (i2 < callsign.length()) && callsign[i2] == test_char;
						}
						incrementing = false;
						i1++;
						break;
					}
				}
				// 
				call_matches &= match_in_parenthesis;
				break;
			default:
				// Match the explicit character
				call_matches &= callsign[i2] == test_char;
				break;
			}
		}
		// We match this particular pattern, so will stop searching
		match = call_matches;
	}
	return match;
}

// Delete an individual prefix and its descendants
void pfx_data::delete_prefix(prefix* prefix) {
	// Delete the various components of a prefix entry
	prefix->continents_.clear();
	prefix->patterns_.clear();
	prefix->cq_zones_.clear();
	prefix->itu_zones_.clear();
	// Delete all children iteratively
	for (auto it = prefix->children_.begin(); it != prefix->children_.end(); it++) {
		delete_prefix(*it);
	}
	prefix->children_.clear();
	delete prefix;
}

// Update the DXCC - set the Country and Territory indices
bool pfx_data::update_dxcc(record* record, prefix*& in_prefix, bool& query, string& reason, bool query_error /*= true*/) {
	string callsign = record->item("CALL");
	string prefix_code = record->item("APP_ZZA_PFX");
	int dxcc_num = -1;
	string dxcc_string = record->item("DXCC");
	record->item("DXCC", dxcc_num);
	string state = record->item("STATE");
	// If a nickname is logged use that to get the prefix record
	if (prefix_code != "") {
		in_prefix = this->get_prefix(prefix_code);
	}
	// If not try DXCC
	if (in_prefix == nullptr && dxcc_string != "") {
		in_prefix = this->get_prefix(dxcc_num, state);
	}
	// Otherwise parse the callsign to get it
	if (in_prefix == nullptr) {
		string sDate = record->item("QSO_DATE");
		in_prefix = this->get_prefix(record, false);
	}
	// Still haven't parsed the callsign...
	if (query_error && in_prefix == nullptr) {
		char * message = new char[callsign.length() + 100];
		sprintf(message, "PREFIX: Unable to parse the callsign: %s", callsign.c_str());
		status_->misc_status(ST_WARNING, message);
		delete[] message;
		query = true;
		reason += "Prefix";
		return false;
	}
	else if (in_prefix == nullptr) {
		return false;
	}
	else {
		// Get the DXCC record
		prefix* dxcc_prefix = in_prefix;
		while (dxcc_prefix->parent_ != nullptr) {
			dxcc_prefix = dxcc_prefix->parent_;
		}
		// Get the ADIF code number for the parsed DXCC entity
		int dxcc_pfx_code = dxcc_prefix->dxcc_code_;
		bool updated = false;
		// If the code number is already logged
		if (dxcc_string != "") {
			// If it's not the same as parsed and parsed is not zero (probably multiple matches in parsing
			if (dxcc_pfx_code != dxcc_num) {
				if (query_error && dxcc_pfx_code != 0) {
					// Ask if OK to use logged code
					if (fl_choice("%s: Logged DXCC does not equal parsed DXCC, Retain Logged DXCC?", fl_yes, fl_no, "", callsign.c_str()) == 1) {
						char new_dxcc_code[16];
						sprintf(new_dxcc_code, "%d", dxcc_pfx_code);
						record->item("DXCC", string(new_dxcc_code));
						updated = true;
					}
					// Otherwise bounce back to user later
					else {
						query = true;
						reason += "DXCC";
					}
				}
			}
		}
		// Log parsed code
		else {
			char new_dxcc_code[16];
			sprintf(new_dxcc_code, "%d", dxcc_pfx_code);
			record->item("DXCC", string(new_dxcc_code));
			updated = true;
		}
		// Get parsed country name - preferably from ADIF enumerated type 
		if (record->item("COUNTRY") == "") {
			// Get it from ADIF spec
			spec_dataset* dxccs = spec_data_->dataset("DXCC_Entity_Code");
			map<string, string>* dxcc_data = dxccs->data[record->item("DXCC")];
			if (dxcc_data != nullptr) {
				record->item("COUNTRY", (*dxcc_data)["Entity Name"]);
			}
			else {
				record->item("COUNTRY", dxcc_prefix->name_);
			}
			updated = true;
		}
		// Get parsed state (Province) name
		if (record->item("STATE") == "") {
			string state = in_prefix->state_;
			if (state != "") {
				record->item("STATE", state);
				updated = true;
			}
		}
		// Get parsed nickname (smallest geographic entity)
		if (record->item("APP_ZZA_PFX") == "") {
			record->item("APP_ZZA_PFX", in_prefix->nickname_);
			updated = true;
		}
		// Get location of contact (from logged LOCATOR)
		lat_long_t location = record->location(false);
		// If these are valid -
		if (!isnan(location.latitude) && !isnan(location.longitude)) {
			// Get geographic location of prefix
			lat_long_t pfx_location = { in_prefix->latitude_, in_prefix->longitude_ };
			double bearing;
			double distance;

			great_circle(pfx_location, location, bearing, distance);
			// Ask user if it's OK that location is 1000 km from prefix centre
			// and flag QSO for update if it's not
			if (query_error && distance > 1000.0) {
				if (fl_choice("%s: distance from station to prefix centre is > 1000 km (%0.0f km) - is it valid?",
					fl_yes, fl_no, "", callsign.c_str(), distance) == 1) {
					query = true;
					reason += "LOC;";
					return false;
				}
			}
		}
		return updated;
	}
}

// Update the CQ Zone based on country and territory
bool pfx_data::update_cq_zone(record* record, prefix* prefix, bool& query, string& reason) {
	int logged_value = 0;
	record->item("CQZ", logged_value);
	if (logged_value == 0) {
		// No CQ Zone logged 
		// If only 1 zone in the prefix use that, else use nothing
		unsigned int zone = prefix->cq_zones_.size() == 1 ? prefix->cq_zones_[0] : 0;
		if (zone > 0) {
			char text[16];
			sprintf(text, "%d", zone);
			record->item("CQZ", string(text));
			reason += "CQ;";
			return true;
		}
		else {
			query = true;
			reason += "CQ;";
			return false;
		}
	}
	else {
		return false;
	}
}

// Update the ITU Zone based on country and territory
bool pfx_data::update_itu_zone(record* record, prefix* prefix, bool& query, string& reason) {
	int logged_value = 0;
	record->item("ITUZ", logged_value);
	if (logged_value == 0) {
		// No CQ Zone logged 
		// If only 1 zone in the prefix use that, else use nothing
		unsigned int zone = prefix->itu_zones_.size() == 1 ? prefix->itu_zones_[0] : 0;
		if (zone > 0) {
			char text[16];
			sprintf(text, "%d", zone);
			record->item("ITUZ", string(text));
			reason += "ITUZ;";
			return true;
		}
		else {
			query = true;
			reason += "ITUZ;";
			return false;
		}
	}
	else {
		return false;
	}
}

// Update the Continent based on country and territory
bool pfx_data::update_continent(record* record, prefix* prefix, bool& query, string& reason) {
	if (record->item("CONT") == "") {
		switch (prefix->continents_.size()) {
		case 1:
			record->item("CONT", prefix->continents_[0]);
			reason += "Cont;";
			return true;
		default:
			query = true;
			reason += "Cont;";
			return false;
		}
	}
	else {
		return false;
	}
}

// Update the ANT_AZ and DISTANCE fields based on record data or entity data
bool pfx_data::update_bearing(record* record) {
	lat_long_t their_location = record->location(false);
	lat_long_t my_location = record->location(true);
	bool updated = false;
	// Need both user and contact locations
	if (!isnan(my_location.latitude) && !isnan(my_location.longitude) &&
		!isnan(their_location.latitude) && !isnan(their_location.longitude)) {
		double bearing;
		double distance;
		// Calculate bearing and distance
		great_circle(my_location, their_location, bearing, distance);
		char azimuth[16];
		sprintf(azimuth, "%0.f", bearing);
		char distance_string[16];
		sprintf(distance_string, "%0.f", distance);
		// Update if different
		if (record->item("ANT_AZ") != azimuth) {
			record->item("ANT_AZ", string(azimuth));
			updated = true;
		}
		if (record->item("DISTANCE") != distance_string) {
			record->item("DISTANCE", string(distance_string));
			updated = true;
		}
	}
	// Enter QSO changed in status log message
	string date = record->item("QSO_DATE");
	string time = record->item("TIME_ON");
	string call = record->item("CALL");
	char* message = new char[50 + date.length() + time.length() + call.length()];
	if (updated) {
		sprintf(message, "LOG: %s %s %s - record changed: Bearing/distance",
			date.c_str(),
			time.c_str(),
			call.c_str());
		status_->misc_status(ST_LOG, message);
	}
	delete[] message;
	return updated;
}

// Update DXCC, CONT, CQZ and ITUZ based on callsign - unless already set in ADIF
bool pfx_data::update_geography(record* record, bool &query, string& reason, bool query_error /*= true*/) {
	bool changed = false;
	prefix* prefix = nullptr;
	if (record->item("CALL") != "" && record->item("SWL") != "Y") {
		// Only parse valid QSOs
		changed = update_dxcc(record, prefix, query, reason, query_error);
		if (prefix != nullptr) {
			changed = update_continent(record, prefix, query, reason) || changed;
			changed = update_cq_zone(record, prefix, query, reason) || changed;
			changed = update_itu_zone(record, prefix, query, reason) || changed;
		}
		string date = record->item("QSO_DATE");
		string time = record->item("TIME_ON");
		string call = record->item("CALL");
		char* message = new char[50 + date.length() + time.length() + call.length()];
		// Update status with a warning
		if (prefix == nullptr) {
			sprintf(message, "LOG: %s %s %s - unable to identify prefix",
				date.c_str(),
				time.c_str(),
				call.c_str());
			status_->misc_status(ST_WARNING, message);
		}
		// Enter QSO changed in status log message
		if (changed) {
			sprintf(message, "LOG: %s %s %s - record changed: %s",
				date.c_str(),
				time.c_str(),
				call.c_str(),
				reason.c_str());
			status_->misc_status(ST_LOG, message);
		}
		delete[] message;
	}
	return changed;
}

// parse a record
parse_result_t pfx_data::parse(record* record) {
	parse_result_t result = PR_UNCHANGED;
	// Returned when the QSO entry is modified
	bool updated = false;
	// Provides the reason for the query
	string query_reason = "";
	bool has_query = false;
	bool invalid_record = !record->is_valid();
	bool query_error = false;
	// Check if the call is listed in exceptions
	if (exceptions_->is_invalid(record)) {
		char message[150];
		snprintf(message, 150, "LOG: Contact %s %s %s - unapproved operation",
			record->item("QSO_DATE").c_str(),
			record->item("TIME_ON").c_str(),
			record->item("CALL").c_str());
		status_->misc_status(ST_ERROR, message);
	}
	else {
		exc_entry* exception = exceptions_->is_exception(record);
		if (exception) {
			string entity_name = spec_data_->entity_name(exception->adif_id);
			char message[160];
			snprintf(message, 160, "LOG: %s %s %s - exception in DXCC ID %d(%s)",
				record->item("QSO_DATE").c_str(),
				record->item("TIME_ON").c_str(),
				record->item("CALL").c_str(),
				exception->adif_id,
				entity_name.c_str());
			status_->misc_status(ST_NOTE, message);
			// Use the values in the exceptions entry
			record->item("DXCC", to_string(exception->adif_id));
			record->item("CQZ", to_string(exception->cq_zone));
			record->item("COUNTRY", entity_name);
		}
	}
	// Parse the various dependent fields
	updated = update_geography(record, has_query, query_reason, query_error) || updated;
	updated = update_bearing(record) || updated;
	// If missing some of the necessary QSO fields
	if (invalid_record) {
		query_reason += " BASICS";
	}
	if ( (query_error || invalid_record) &&
		fl_choice("Unable to parse record due to a problem with %s\nCancel stops all parsing", fl_ok, fl_cancel, "", query_reason.c_str()) == 1) {
		result = PR_ABANDONED;
	}
	else if (updated) {
		result = PR_CHANGED;
	}
	else {
		result = PR_UNCHANGED;
	}
	return result;
}

// Calculate the great circle bearing and distance between two locations on the Earth's surface
void pfx_data::great_circle(lat_long_t source, lat_long_t destination, double& bearing, double& distance)
{
	// difference in longitude in radians
	double d_long = (destination.longitude - source.longitude) * DEGREE_RADIAN;
	double cos_d_long = cos(d_long);
	double sin_d_long = sin(d_long);
	// Latitudes in radians
	double src_latitude = source.latitude * DEGREE_RADIAN;
	double dest_latitude = destination.latitude * DEGREE_RADIAN;
	double cos_s_lat = cos(src_latitude);
	double sin_s_lat = sin(src_latitude);
	double tan_d_lat = tan(dest_latitude);
	double cos_t_lat = cos(dest_latitude);
	double sin_t_lat = sin(dest_latitude);

	// bearing (azimuth)
	// tan(a) = ( sin(l2-l1) / ( cos(ph1).tan(ph2) - sin(ph1).cos(l2-l1) )
	// l(ambda) - longitude, ph(i) - latitude
	// calculate denominator and use atan2 to get correct quadrant
	double denominator = ((cos_s_lat * tan_d_lat) - (sin_s_lat * cos_d_long));
	bearing = atan2(sin_d_long, denominator) * RADIAN_DEGREE;
	// Convert from -180->0 to +180->+360
	if (bearing < 0.0) bearing += 360.0;

	// Great circle distance
	// s = angle at centre of sphere
	// cos(s) = ( sin(ph1).sin(ph2) + cos(ph1).cos(ph2).cos(l2-l1) )
	double cos_angle = (sin_s_lat * sin_t_lat) + (cos_s_lat * cos_t_lat * cos_d_long);
	// Convert to distance = radius * angle in radians
	distance = EARTH_RADIUS * acos(cos_angle);
}


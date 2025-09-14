#include "contest_data.h"

#include "contest_reader.h"
#include "contest_writer.h"
#include "status.h"

#include <fstream>

extern std::string default_data_directory_;
extern status* status_;

contest_data::contest_data() {
	load_data();
}

contest_data::~contest_data() {
	save_data();
	contests_.clear();
}

// Get contest data structure for contest "id" 
ct_data_t* contest_data::get_contest(std::string id, std::string ix, bool create) {
	if (contests_.find(id) == contests_.end()) {
		if (create && ix.length()) {
			ct_data_t* ct = new ct_data_t;
			contests_[id][ix] = ct;
			ct_entry_t* entry = new ct_entry_t({ id, ix, ct });
			contest_infos_.push_back(entry);
			return ct;
		}
		else {
			return nullptr;
		}
	} 
	if (contests_.at(id).find(ix) == contests_.at(id).end()) {
		if (create && ix.length()) {
			ct_data_t* ct = new ct_data_t;
			contests_.at(id)[ix] = ct;
			ct_entry_t* entry = new ct_entry_t({ id, ix, ct });
			contest_infos_.push_back(entry);
			return ct;
		}
		else {
			return nullptr;
		}
	}
	return contests_.at(id).at(ix);
}

std::set<std::string>* contest_data::get_contest_indices(std::string id) {
	if (contests_.find(id) == contests_.end()) {
		return nullptr;
	}
	std::set<std::string>* result = new std::set<std::string>;
	for (auto it : contests_.at(id)) {
		result->insert(it.first);
	}
	return result;
}

// Get the number of contests registered
int contest_data::get_contest_count() {
	return contest_infos_.size();
}

// Get the specified contests information
ct_entry_t* contest_data::get_contest_info(int number) {
	return contest_infos_[number];
}

// Load data
bool contest_data::load_data() {
	std::string filename = default_data_directory_ + "contests.xml";
	ifstream is;
	is.open(filename, std::ios_base::in);
	if (is.good()) {
		contest_reader* reader = new contest_reader();
		if (reader->load_data(this, is)) {
			status_->misc_status(ST_OK, "CONTEST: XML loaded OK");
			// Populate the info database
			for (auto it : contests_) {
				for (auto iu : it.second) {
					ct_entry_t* info = new ct_entry_t({ it.first, iu.first, iu.second });
					contest_infos_.push_back(info);
				}
			}
			return true;
		}
	}
	status_->misc_status(ST_WARNING, "CONTEST: XML data failed to load");
	return false;

}

// Save data
bool contest_data::save_data() {
	std::string filename = default_data_directory_ + "contests.xml";
	std::ofstream os;
	os.open(filename, std::ios_base::out);
	if (os.good()) {
		contest_writer* writer = new contest_writer();
		if (!writer->store_data(this, os)) {
			status_->misc_status(ST_OK, "CONTEST: Saved XML OK");
			return true;
		}
	}
	return false;
}

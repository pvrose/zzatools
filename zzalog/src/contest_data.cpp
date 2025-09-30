#include "contest_data.h"

#include "status.h"

#include <fstream>
#include <ctime>

extern std::string default_data_directory_;
extern status* status_;

//! Conversion of ct_data_ to JSON
void to_json(json& j, const ct_data_t& s) {
	j = json{
		{ "Algorithm", s.algorithm },
		{ "Dates", s.date }
	};
}

//! Conversion of JSON to ct_data_t
void from_json(const json& j, ct_data_t& s) {
	j.at("Algorithm").get_to(s.algorithm);
	j.at("Dates").get_to(s.date);
}

//! Conversion of ct_date_t to JSON
void to_json(json& j, const ct_date_t& s) {
	char temp[64];
	const std::time_t ts = std::chrono::system_clock::to_time_t(s.start);
	strftime(temp, sizeof(temp), "%FT%TZ", std::gmtime(&ts));
	j["Start"] = temp;
	const std::time_t tf = std::chrono::system_clock::to_time_t(s.finish);
	strftime(temp, sizeof(temp), "%FT%TZ", std::gmtime(&tf));
	j["Finish"] = temp;
}

//! Conversion of JSON to ct_date_t
void from_json(const json& j, ct_date_t& s) {
	string temps;
	j.at("Start").get_to(temps);
	std::time_t result = convert_iso_datetime(temps);
	s.start = std::chrono::system_clock::from_time_t(result);
	j.at("Finish").get_to(temps);
	result = convert_iso_datetime(temps);
	s.finish = std::chrono::system_clock::from_time_t(result);
}

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
	status_->misc_status(ST_NOTE, "CONTEST: loading contest data");
	std::string filename = default_data_directory_ + "contests.json";
	ifstream is;
	is.open(filename, std::ios_base::in);
	if (is.good()) {
		if (load_json(is)) {
			status_->misc_status(ST_OK, "CONTEST: Contest data loaded OK");
			return true;
		}
	}
	status_->misc_status(ST_WARNING, "CONTEST: Contest data failed to load");
	return false;

}

// Save data
bool contest_data::save_data() {
	std::string filename = default_data_directory_ + "contests.json";
	std::ofstream os;
	os.open(filename, std::ios_base::out);
	if (os.good()) {
		if (save_json(os)) {
			status_->misc_status(ST_OK, "CONTEST: Saved data OK");
			os.close();
			return true;
		}
	}
	return false;
}

// Load JSON
bool contest_data::load_json(std::ifstream& is) {
	json jall;
	try {
		is >> jall;
		for (auto itc : jall.at("Contests")) {
			std::map<std::string, ct_data_t*> contest;
			for (auto iti : itc.at("Instances")) {
				ct_data_t* cd = new ct_data_t(iti.at("Definition").template get<ct_data_t>());
				string index;
				iti.at("Index").get_to(index);
				contest[index] = cd;
			}
			string name;
			itc.at("Name").get_to(name);
			contests_[name] = contest;
		}
	}
	catch (const json::exception& e) {
		char msg[128];
		std::snprintf(msg, sizeof(msg), "CONTEST: Reading JSON failed %d (%s)\n",
			e.id, e.what());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
	if (is.fail()) return false;
	else return true;
}

// SAve JSON
bool contest_data::save_json(std::ofstream& os) {
	json jall;
	jall["Contests"].clear();
	for (auto itc : contests_) {
		json jc;
		jc["Name"] = itc.first;
		for (auto iti : itc.second) {
			json ji;
			ji["Index"] = iti.first;
			ji["Definition"] = *iti.second;
			jc["Instances"].push_back(ji);
		}
		jall["Contests"].push_back(jc);
	}
	os << std::setw(2) << jall << '\n';
	if (os.fail()) return false;
	else return true;
}

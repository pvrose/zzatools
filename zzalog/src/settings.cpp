#include "settings.h"

#include "contest_scorer.h"
#include "dxcc_table.h"
#include "log_table.h"
#include "qso_wx.h"
#include "report_tree.h"
#include "search.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <istream>
#include <ostream>
#include <string>

using json = nlohmann::json;

extern std::string default_data_directory_;
extern std::string PROGRAM_ID;

//! Basic constructor
settings::settings() {
	// Ddfault filename ZZALOG.json
	std::string filename = default_data_directory_ + PROGRAM_ID + ".json";
	std::ifstream i(filename);
	bool ok = false;
	parent_ = nullptr;
	if (i.good()) {
		ok = true;
		// Load in file
		try {
			json j;
			i >> j;
			if (j.find(PROGRAM_ID) != j.end() &&
				!j.at(PROGRAM_ID).is_null()) {
				data_ = new json(j.at(PROGRAM_ID));
			}
			else {
				data_ = new json;
			}
			i.close();
		}
		catch (const json::exception& e) {
			printf("SETTINGS: Reading JSON failed %d (%s)\n",
				e.id, e.what());
			i.close();
			ok = false;
		}
	}
	else {
		data_ = new json;
	}
}

//! Construct a sub-group of settings
settings::settings(settings* parent, const std::string name) {
	parent_ = parent;
	name_ = name;
	if (parent->data_->find(name) == parent->data_->end()) {
		(*parent_->data_)[name] = json();
	}
	data_ = &parent->data_->at(name);
}

//! DEstructor - copy data back to file
settings::~settings() {
	if (parent_ == nullptr) {
		std::string filename = default_data_directory_ + PROGRAM_ID + ".json";
		std::ofstream o(filename);
		bool ok = false;
		parent_ = nullptr;
		json j;
		j[PROGRAM_ID] = *data_;
		o << std::setw(2) << j << '\n';
		o.close();
	}
}

//! Clear the settings
void settings::clear() {
	// Do nothing for now
}

//! Get object
template <class T>
bool settings::get(std::string name, T& value, const T def) {
	bool exists = true;
	if (data_->find(name.c_str()) == data_->end()) {
		(*data_)[name.c_str()] = def;
		exists = false;
	}
	value = data_->at(name.c_str()).get<T>();
	return exists;
}
template bool settings::get<double>(std::string, double&, const double);
template bool settings::get<time_t>(std::string, time_t&, const time_t);
template bool settings::get<int>(std::string, int&, const int);
template bool settings::get<std::string>(std::string, std::string&, const std::string);
template bool settings::get<contest_scorer::ct_status>(std::string, contest_scorer::ct_status&, const contest_scorer::ct_status);
template bool settings::get<dxcc_table::display_t>(std::string, dxcc_table::display_t&, const dxcc_table::display_t);
template bool settings::get<dxcc_table::confirm_t>(std::string, dxcc_table::confirm_t&, const dxcc_table::confirm_t);
template bool settings::get<log_table::sort_order>(std::string, log_table::sort_order&, const log_table::sort_order);
template bool settings::get<float>(std::string, float&, const float);
template bool settings::get<std::list<std::string>>(std::string, std::list<std::string>&, const std::list<std::string>);
template bool settings::get<bool>(std::string, bool&, const bool);
template bool settings::get<qso_wx::speed_t>(std::string, qso_wx::speed_t&, const qso_wx::speed_t);
template bool settings::get<qso_wx::dirn_t>(std::string, qso_wx::dirn_t&, const qso_wx::dirn_t);
template bool settings::get<qso_wx::temp_t>(std::string, qso_wx::temp_t&, const qso_wx::temp_t);
template bool settings::get<qso_wx::press_t>(std::string, qso_wx::press_t&, const qso_wx::press_t);
template bool settings::get<qso_wx::cloud_t>(std::string, qso_wx::cloud_t&, const qso_wx::cloud_t);
template bool settings::get<report_filter_t>(std::string, report_filter_t&, const report_filter_t);
template bool settings::get<std::vector<report_cat_t>>(std::string, std::vector<report_cat_t>&, const std::vector<report_cat_t>);
template bool settings::get<search_comp_t>(std::string, search_comp_t&, const search_comp_t);
template bool settings::get<search_combi_t>(std::string, search_combi_t&, const search_combi_t);
template bool settings::get<search_cond_t>(std::string, search_cond_t&, const search_cond_t);

//! Set object
template <class T>
void settings::set(std::string name, const T value) {
	(*data_)[name.c_str()] = value;
}
template void settings::set<time_t>(std::string, const time_t);
template void settings::set<int>(std::string, const int);
template void settings::set<contest_scorer::ct_status>(std::string, const contest_scorer::ct_status);
template void settings::set<dxcc_table::display_t>(std::string, const dxcc_table::display_t);
template void settings::set<dxcc_table::confirm_t>(std::string, const dxcc_table::confirm_t);
template void settings::set<log_table::sort_order>(std::string, const log_table::sort_order);
template void settings::set<bool>(std::string, const bool);
template void settings::set<std::list<std::string>>(std::string, const std::list<std::string>);
template void settings::set<qso_wx::speed_t>(std::string, const qso_wx::speed_t);
template void settings::set<qso_wx::dirn_t>(std::string, const qso_wx::dirn_t);
template void settings::set<qso_wx::temp_t>(std::string, const qso_wx::temp_t);
template void settings::set<qso_wx::press_t>(std::string, const qso_wx::press_t);
template void settings::set<qso_wx::cloud_t>(std::string, const qso_wx::cloud_t);
template void settings::set<report_filter_t>(std::string, const report_filter_t);
template void settings::set<std::vector<report_cat_t>>(std::string, const std::vector<report_cat_t>);
template void settings::set<search_comp_t>(std::string, const search_comp_t);
template void settings::set<search_combi_t>(std::string, const search_combi_t);
template void settings::set<search_cond_t>(std::string, const search_cond_t);
template void settings::set<std::string>(std::string, const std::string);
template void settings::set<float>(std::string, const float);
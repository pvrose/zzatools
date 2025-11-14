#include "band_data.h"
#include "file_holder.h"
#include "main.h"
#include "status.h"
#include "spec_data.h"

#include "nlohmann/json.hpp"

#include <cfloat>
#include <fstream>
#include <iostream>

#include <FL/Fl_Native_File_Chooser.H>

using json = nlohmann::json;

// MAp band_data::entry_t
NLOHMANN_JSON_SERIALIZE_ENUM(band_data::entry_t, {
	{ band_data::UNKNOWN, nullptr },
	{ band_data::BAND, "Band" },
	{ band_data::SUB_BAND, "Sub-band" },
	{ band_data::SPOT, "Spot frequency" },
	{ band_data::SPOT_SET, "Set of spots"},
	{ band_data::USER_SPOT, "User frequency" }
})

void to_json(json& j, const range_t& r) {
	if (std::isnan(r.upper) || r.upper == r.lower) {
		j = json{
			{ "Lower", r.lower }
		};
	}
	else {
		j = json{
			{ "Lower", r.lower },
			{ "Upper", r.upper }
		};
	}
}

void from_json(const json& j, range_t& r) {
	if (j.find("Upper") == j.end()) r.upper = nan("");
	else j.at("Upper").get_to(r.upper);
	j.at("Lower").get_to(r.lower);
}


//! band_entry_t to json convertor
void to_json(json& j, const band_data::band_entry_t& e) {
	j = json{
		{ "Type", json(e.type) },
		{ "Range", json(e.range) },
		{ "Bandwidth", e.bandwidth },
		{ "Modes", e.modes },
		{ "Summary", e.summary }
	};
}
void from_json(const json& j, band_data::band_entry_t& e) {
	if (j.find("Type") == j.end()) e.type = band_data::UNKNOWN;
	else j.at("Type").get_to(e.type);
	j.at("Range").get_to(e.range);
	if (j.at("Bandwidth").is_null()) e.bandwidth = nan("");
	else j.at("Bandwidth").get_to(e.bandwidth);
	j.at("Modes").get_to(e.modes);
	j.at("Summary").get_to(e.summary);
	if (e.type == band_data::UNKNOWN) {
		if (e.range.lower == e.range.upper) {
			e.type = band_data::SPOT;
		}
		else if (e.modes.size() == 0) {
			e.type = band_data::SPOT_SET;
		}
		else {
			e.type = band_data::SUB_BAND;
		}
	}
}

// Constructor 
band_data::band_data()
{
	load_json();
	create_bands();

}

// Destructor
band_data::~band_data()
{
	save_json();
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		delete* it;
	}
	entries_.clear();
}

// Load data from JSON file
bool band_data::load_json() {
	json j;
	char msg[128];
	// Wrte JSON out to band_plan.json
	status_->misc_status(ST_NOTE, ("BAND: Loading band-plan data"));
	std::ifstream i;
	std::string filename;
	if (file_holder_->get_file(FILE_BANDPLAN, i, filename)) {
		try {
			i >> j;
			i.close();
			for (auto jt : j.at("Band plan")) {
				band_entry_t* e = new band_entry_t(jt.template get<band_entry_t>());
				entries_.insert(e);
			}
			snprintf(msg, sizeof(msg), "BAND: File %s loaded OK", filename.c_str());
			status_->misc_status(ST_OK, msg);
			return true;
		}
		catch (const json::exception& e) {
			char msg[128];
			snprintf(msg, sizeof(msg), "BAND: Failed to load %s: %d (%s)\n",
				filename.c_str(), e.id, e.what());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
	}
	else {
		status_->misc_status(ST_ERROR, "BAND: Load band-plan data failed");
		return false;
	}
}

// Save data in a JSON file
void band_data::save_json() {
	// Convert entries_ to a JSON object
	json j;

	for (auto it : entries_) {
		json j1(*it);
		j.push_back(j1);
	}
	json jout;
	jout["Band plan"] = j;
	// Wrte JSON out to band_plan.json
	std::string filename;
	std::ofstream o;
	file_holder_->get_file(FILE_BANDPLAN, o, filename);
	o << std::setw(4) << jout << '\n';
	o.close();
}

// Get the band plan data entry for the specified frequency
band_data::band_entry_t* band_data::get_entry(double frequency) {
	for (auto e : entries_) {
		if (e->range.lower <= frequency && e->range.upper >= frequency) {
			return e;
		}
	}
	return nullptr;
}

// Get the band plan data entries for the frequency range
std::set<band_data::band_entry_t*, band_data::ptr_lt>& band_data::get_entries() {
	return entries_;
}

// Is the supplied frequency in a band
bool band_data::in_band(double frequency) {
	if (get_entry(frequency)) {
		return true;
	}
	else {
		return false;
	}
}

// Return the band std::list
band_map<std::set<range_t >>& band_data::bands() {
	return bands_;
}

// Generate the band std::list
void band_data::create_bands() {
	bool has_bands = false;
	range_t current_range = { 0.0, 0.0 };
	std::string current_band = ""; 
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		if ((*it)->type == band_data::BAND) has_bands = true;
		if ((*it)->range.lower > current_range.upper) {
			std::string band = spec_data_->band_for_freq((*it)->range.lower);
			// New range - add the previous range
			if (current_band.length()) {
				if (bands_.at(current_band).size() == 1) {
					char msg[128];
					snprintf(msg, sizeof(msg), "BAND: Multiple ranges found for %s",
						band.c_str());
					status_->misc_status(ST_WARNING, msg);
				}
				bands_.at(current_band).insert(current_range); 
			}
			if (band != current_band) {
				// Create new band with an empty std::set of ranges
				current_band = band;
				bands_[current_band] = {};
			}
			current_range = (*it)->range;
		} else {
			if (!std::isnan((*it)->range.upper)) {
				current_range.upper = std::max<double>(current_range.upper, (*it)->range.upper);
			}
		}
		// Collate the modes
		for (auto m : (*it)->modes) {
			modes_.insert(m);
		}
	}
	if (!has_bands) {
		// Add entried for the bands
		for (auto b : bands_) {
			std::string band_name = b.first;
			band_entry_t* entry = new band_entry_t;
			entry->type = band_data::BAND;
			entry->range = { DBL_MAX, 0.0 };
			entry->bandwidth = nan("");
			entry->modes = {};
			entry->summary = band_name;
			for (auto b1 : b.second) {
				entry->range.lower = std::min<double>(entry->range.lower, b1.lower);
				entry->range.upper = std::max<double>(entry->range.upper, b1.upper);
			}
			entries_.insert(entry);
		}
	}
}

std::set<std::string> band_data::get_modes() {
	return modes_;
}
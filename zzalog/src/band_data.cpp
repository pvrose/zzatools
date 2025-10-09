#include "band_data.h"
#include "status.h"
#include "spec_data.h"

#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

using json = nlohmann::json;

extern status* status_;
extern spec_data* spec_data_;
extern std::string VENDOR;
extern std::string PROGRAM_ID;
extern std::string default_ref_directory_;

// MAp band_data::entry_t
NLOHMANN_JSON_SERIALIZE_ENUM(band_data::entry_t, {
	{ band_data::UNKNOWN, nullptr },
	{ band_data::BAND, "Band" },
	{ band_data::SUB_BAND, "Sub-band" },
	{ band_data::SPOT, "Spot frequency" },
	{ band_data::SPOT_SET, "Set of spots"}
})

void to_json(json& j, const range_t& r) {
	if (isnan(r.upper) || r.upper == r.lower) {
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
	j.at("Bandwidth").get_to(e.bandwidth);
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
	std::string filename = get_path() + "band_plan.json";
	status_->misc_status(ST_NOTE, ("BAND: Loading band-plan data"));
	ifstream i(filename);
	if (i.good()) {
		try {
			i >> j;
			i.close();
			for (auto jt : j.at("Band plan")) {
				band_entry_t* e = new band_entry_t(jt.template get<band_entry_t>());
				entries_.push_back(e);
			}
			std::snprintf(msg, sizeof(msg), "BAND: File %s loaded OK", filename.c_str());
			status_->misc_status(ST_OK, msg);
			return true;
		}
		catch (const json::exception& e) {
			char msg[128];
			std::snprintf(msg, sizeof(msg), "BAND: Failed to load %s: %d (%s)\n",
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
	std::string filename = get_path() + "band_plan.json";
	std::ofstream o(filename);
	o << std::setw(4) << jout << '\n';
	o.close();
}

// Get the directory of the reference files
std::string band_data::get_path() {
	return default_ref_directory_;
}

// Get the band plan data entry for the specified frequency
band_data::band_entry_t* band_data::get_entry(double frequency) {
	for (unsigned int ix = 0; ix < entries_.size(); ix++) {
		if (entries_[ix]->range.lower <= frequency && entries_[ix]->range.upper >= frequency) {
			return entries_[ix];
		}
	}
	return nullptr;
}

// Get the band plan data entries for the frequency range
std::set<band_data::band_entry_t*> band_data::get_entries() {
	std::set<band_entry_t*> result;
	for (unsigned int ix = 0; ix < entries_.size(); ix++) {
		result.insert(entries_[ix]);
	}
	return result;
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
	range_t current_range = { 0.0, 0.0 };
	std::string current_band = ""; 
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
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
			current_range.upper = max(current_range.upper, (*it)->range.upper);
		}
	}
}

// Locate the all.xml file and copy to default data directory
bool band_data::find_and_copy_data() {
	std::string source;
	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser->title("Select Band plan file band_plan.tsv");
	chooser->filter("TSV File\t*.tsv");
	if (chooser->show() == 0) {
		source = chooser->filename();
	}
	delete chooser;
	std::string target = get_path() + "band_plan.tsv";
	char msg[128];
	snprintf(msg, sizeof(msg), "BAND: Copying %s", source.c_str());
	status_->misc_status(ST_NOTE, msg);
	// In and out streams
	ifstream in(source);
	in.seekg(0, in.end);
	int length = (int)in.tellg();
	const int increment = 8000;
	in.seekg(0, in.beg);
	status_->progress(length, OT_BAND, "Copying data to backup", "bytes");
	std::ofstream out(target);
	bool ok = in.good() && out.good();
	char buffer[increment];
	int count = 0;
	// Copy file in 7999 byte chunks
	while (!in.eof() && ok) {
		in.read(buffer, increment);
		out.write(buffer, in.gcount());
		count += (int)in.gcount();
		ok = out.good() && (in.good() || in.eof());
		status_->progress(count, OT_BAND);
	}
	if (!ok) {
		status_->progress("Failed before completion", OT_BAND);
	}
	else {
		if (count != length) {
			status_->progress(length, OT_BAND);
		}
	}
	in.close();
	out.close();
	if (!ok) {
		// Report error
		status_->misc_status(ST_FATAL, "BAND: failed");
		return false;
	}
	else {
		status_->misc_status(ST_OK, "BAND: Copied");
		return true;
	}
}

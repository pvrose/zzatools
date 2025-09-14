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
extern string VENDOR;
extern string PROGRAM_ID;
extern string default_data_directory_;

void to_json(json& j, const range_t& r) {
	j = json {
		{ "lower", r.lower },
		{ "upper", r.upper }
	};
}

void from_json(const json& j, range_t& r) {
	j.at("upper").get_to(r.upper);
	j.at("lower").get_to(r.lower);
}


//! band_entry_t to json convertor
void to_json(json& j, const band_data::band_entry_t& e) {
	j = json{
		{ "range", json(e.range) },
		{ "bandwidth", e.bandwidth },
		{ "modes", e.modes },
		{ "summary", e.summary }
	};
}
void from_json(const json& j, band_data::band_entry_t& e) {
	j.at("range").get_to(e.range);
	j.at("bandwidth").get_to(e.bandwidth);
	j.at("modes").get_to(e.modes);
	j.at("summary").get_to(e.summary);
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
//	save_json();
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		delete* it;
	}
	entries_.clear();
}

// Load data from JSON file
bool band_data::load_json() {
	json j;
	// Wrte JSON out to band_plan.json
	string filename = get_path() + "band_plan.json";
	status_->misc_status(ST_NOTE, ("BAND: Loading band-plan data"));
	ifstream i(filename);
	if (i.good()) {
		i >> j;
		i.close();
		for (auto jt : j) {
			band_entry_t* e = new band_entry_t(jt.template get<band_entry_t>());
			entries_.push_back(e);
		}
		status_->misc_status(ST_OK, "BAND: Loaded band-plan data");
		return true;
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
	// Wrte JSON out to band_plan.json
	string filename = get_path() + "band_plan.json";
	ofstream o(filename);
	o << std::setw(4) << j << '\n';
	o.close();
}
// Decode an entry
band_data::band_entry_t* band_data::get_entry(string line) {
	vector<string> words;
	split_line(line, words, '\t');
	// Create the entry
	band_entry_t* result = new band_entry_t;
	// Lower->Upper->Bandwidth->Mode->Notes
	// First entry lower bound or spot entry
	result->range.lower = stod(words[0]) / 1000.0;
	// Second entry upper bound - if no entry then it's a spot frequency entry
	try {
		result->range.upper = stod(words[1]) / 1000.0;
	}
	catch (exception&) {
		result->range.upper = result->range.lower;
	}
	// Third entry is bandwidth
	try {
		result->bandwidth = stod(words[2]);
	}
	catch (exception&) {
		result->bandwidth = 0.0;
	}
	// Fourth is mode
	vector<string> modes;
	split_line(words[3], modes, ',');
	for (auto ix = modes.begin(); ix != modes.end(); ix++) {
		if (ix->length()) {
			result->modes.insert(*ix);
		}
	}
	// Fifth is notes
	result->summary = words[4];
	return result;
}

// Get the directory of the reference files
string band_data::get_path() {
	return default_data_directory_;
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
set<band_data::band_entry_t*> band_data::get_entries() {
	set<band_entry_t*> result;
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

// Return the band list
band_map<set<range_t >>& band_data::bands() {
	return bands_;
}

// Generate the band list
void band_data::create_bands() {
	range_t current_range = { 0.0, 0.0 };
	string current_band = ""; 
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		if ((*it)->range.lower > current_range.upper) {
			string band = spec_data_->band_for_freq((*it)->range.lower);
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
				// Create new band with an empty set of ranges
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
	string source;
	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser->title("Select Band plan file band_plan.tsv");
	chooser->filter("TSV File\t*.tsv");
	if (chooser->show() == 0) {
		source = chooser->filename();
	}
	delete chooser;
	string target = get_path() + "band_plan.tsv";
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
	ofstream out(target);
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

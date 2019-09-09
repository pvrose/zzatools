#include "power_matrix.h"

#include <map>
#include <set>
#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;
using namespace zzalog;

extern Fl_Preferences* settings_;

// Constructor - default
power_matrix::power_matrix() {
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	char* rig;
	rigs_settings.get("Current", rig, "");
	if (strlen(rig) > 0) {
		power_matrix(string(rig));
	} 
	free(rig);
}

// Constructor - takes rig and intialises the matrix from settings
power_matrix::power_matrix(string rig)
{
	map_.clear();
	rig_ = rig;
	// Get the settings for the named group
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	Fl_Preferences rig_settings(rigs_settings, rig.c_str());
	if (rig_settings.groupExists("Power Matrix")) {
		// Get the matrix settings
		Fl_Preferences matrix_settings(rig_settings, "Power Matrix");
		// For each band in the matrix - a band has its own group of entries
		for (int g = 0; g < matrix_settings.groups(); g++) {
			string band = matrix_settings.group(g);
			Fl_Preferences band_settings(matrix_settings, band.c_str());
			// Create the map of power-levels to drives
			power_lut* power_map = new power_lut;
			for (int e = 0; e < band_settings.entries(); e++) {
				// Get the power as text and convert to int
				string name = band_settings.entry(e);
				int drive = stoi(name);
				// Get the drive as int
				double power;
				band_settings.get(name.c_str(), drive, nan(""));
				// Map power to drive
				(*power_map)[drive] = power;
			}
			map_[band] = power_map;
		}
	}
	else {
		// Create a map - band = Any, points (0,0) and (100,100)
		power_lut* power_map = new power_lut;
		(*power_map)[0] = 0;
		(*power_map)[100] = 100.0;
		string band = "ANY";
		map_[band] = power_map;
	}
}

// Destructor
power_matrix::~power_matrix()
{
	// Save settings
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	Fl_Preferences rig_settings(rigs_settings, rig_.c_str());
	Fl_Preferences matrix_settings(rig_settings, "Power Matrix");
	// Clear the current entry
	matrix_settings.clear();
	// For each band
	for (auto it = map_.begin(); it != map_.end(); it++) {
		// For each item - write away the drive/power value
		power_lut* power_map = it->second;
		Fl_Preferences band_settings(matrix_settings, it->first.c_str());
		for (auto it_drive = power_map->begin(); it_drive != power_map->end(); it_drive++) {
			band_settings.set(to_string(it_drive->first).c_str(), it_drive->second);
		}
		power_map->clear();
		delete power_map;
	}
	map_.clear();
}

// Delete the rig entry in settings
void power_matrix::delete_rig() {
	// Save settings
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	Fl_Preferences rig_settings(rigs_settings, rig_.c_str());
	Fl_Preferences matrix_settings(rig_settings, "Power Matrix");
	// Clear the current entry
	matrix_settings.clear();
	map_.clear();
}

// Returns the power for the specific band and drive-level
double power_matrix::power(string band, double drive) {
	power_lut* power_map = nullptr;
	if (map_.find(band) != map_.end()) {
		power_map = map_[band];
	}
	else if (map_.find("ANY") != map_.end()) {
		power_map = map_["ANY"];
	} 
	if (power_map != nullptr) {
		double last_power = 0.0;
		int last_drive = 0;
		float slope = 0.0;
		auto it = power_map->begin();
		// Step through the map until we have passed the supplied drive
		for (; it != power_map->end() && it->first <= drive * 100; it++) {
			// Set slope to that between the last two 
			slope = (it->second - last_power) / (float)(it->first - last_drive);
		}
		// We have another entry at higher drive so set the slope based on the before and after
		if (it != power_map->end()) {
			slope = (it->second - last_power) / (float)(it->first - last_drive);
		}
		return last_power + (slope * (float)(drive - last_drive));
	}
	else {
		return nan("");
	}
}

// Returns the number of rows (bands)
vector<string> power_matrix::bands() {
	vector<string> result;
	for (auto it = map_.begin(); it != map_.end(); it++) {
		result.push_back(it->first);
	}
	return result;
}

// Get the matrix for the particular band
power_lut* power_matrix::get_map(string band) {
	if (map_.find(band) != map_.end()) {
		return map_[band];
	}
	else {
		return nullptr;
	}
}

// Add a default matrix for the band (0->0; 100->100)
void power_matrix::add_band(string band) {
	// Create a map - band = Any, points (0,0) and (100,100)
	power_lut* power_map = new power_lut;
	(*power_map)[0] = 0;
	(*power_map)[100] = 100;
	map_[band] = power_map;
}

// Delete band - returns true if successful
bool power_matrix::delete_band(string band) {
	if (map_.size() > 1) {
		//delete the band
		map_.erase(band);
		return true;
	}
	else {
		return false;
	}
}

#include "power_matrix.h"

#include <map>
#include <set>
#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;
using namespace zzalib;

extern Fl_Preferences* settings_;

power_matrix* power_matrix_;

// Constructor - default
power_matrix::power_matrix() {
	re_initialise();
}

// Re-read settings for the rig
void power_matrix::re_initialise() {
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences rigs_settings(stations_settings, "Rigs");
	char* rig;
	rigs_settings.get("Current", rig, "");
	if (strlen(rig) > 0) {
		initialise(string(rig));
	} 
	free(rig);
}

// Takes rig and intialises the matrix from settings
void power_matrix::initialise(string rig)
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
				// Get the drive as text and convert to int
				string name = band_settings.entry(e);
				int drive = stoi(name);
				// Get the power as aa double
				double power = nan("");
				band_settings.get(name.c_str(), power, nan(""));
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

// Returns the power for the specific band and drive-level - with limited precision (about 100th full-power)
double power_matrix::power(string band, int drive) {
	double power = look_up(band, drive);
	double precision = look_up(band, 100) / 100;
	if (precision < 0.001) {
		// Return value read
		return power;
	}
	else if (precision < 0.01) {
		// Return to closest 0.1 W
		int result = (int)(power * 100);
		return (double)result * 0.01;
	}
	else if (precision < 0.1) {
		// return to closest 0.1W
		int result = (int)(power * 10);
		return (double)result * 0.1;
	}
	else if (precision < 1.0) {
		int result = (int)power;
		return (double)result;
	}
	else if (precision < 10.0) {
		int result = (int)(power * 0.1);
		return (double)result * 10.0;
	}
	else if (precision < 100.0) {
		int result = (int)(power * 0.01);
		return (double)result * 100.0;
	}
	else {
		int result = (int)(power * 0.001);
		return (double)result * 1000.0;
	}

}

// Lookup the power for the specific band and drive level
double power_matrix::look_up(string band, int drive) {
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
		double slope = 0.0;
		bool carry_on = true;
		auto it = power_map->begin();
		// Step through the map until we have passed the supplied drive
		for (; it != power_map->end() && carry_on; it++) {
			// Set slope to that between the last two 
			if (it->first == last_drive) {
				slope = 0;
			}
			else {
				slope = (it->second - last_power) / (double)(it->first - last_drive);
			}
			last_power = it->second;
			last_drive = it->first;
			if (last_drive > drive) carry_on = false;
		}
		// Reurn the interpolated value
		return last_power + (slope * (double)(drive - last_drive));
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

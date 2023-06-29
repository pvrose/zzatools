#pragma once

#include <string>
#include <vector>

using namespace std;

class band_data
{
public:
	// Band entry structure
	struct band_entry_t {
		double lower;       // Lower end of sub-band (kHz)
		double upper;       // Upper end of sub-band (kHz)
		double bandwidth;   // Maximum bandwidth usable in sub-band
		string mode;        // Modes allowed
		string summary;     // Summary display
		string notes;       // Notes about usage
		// Default constructor
		band_entry_t() :
			lower(0.0),
			upper(0.0),
			bandwidth(0.0),
			mode(""),
			summary(""),
			notes("") {};
	};

	band_data();
	~band_data();

	// is the frequency supplied in band?
	bool in_band(double frequency);
	// Get the band entry for the frequency
	band_entry_t* get_entry(double frequency);

protected:
	// Read the data
	bool load_data();
	// Get the file name
	string get_path();
	// Parse the band entry
	band_entry_t* get_entry(string line);
	// The band entries
	vector<band_entry_t*> entries_;

};


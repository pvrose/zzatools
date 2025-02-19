#pragma once

#include <string>
#include <vector>
#include <set>

using namespace std;

// This class reads in the IARU band-plan in TSV form into a database
// It provides access to this database
class band_data
{
public:
	// Band entry structure
	struct band_entry_t {
		double lower;       // Lower end of sub-band (kHz)
		double upper;       // Upper end of sub-band (kHz)
		double bandwidth;   // Maximum bandwidth usable in sub-band
		set<string> modes;        // Modes allowed
		string summary;     // Summary display
		// Default constructor
		band_entry_t() :
			lower(0.0),
			upper(0.0),
			bandwidth(0.0),
			modes({}),
			summary("") {};
	};

	band_data();
	~band_data();

	// is the frequency supplied in band?
	bool in_band(double frequency);
	// Get the band entry for the frequency
	band_entry_t* get_entry(double frequency);
	// Get the set of entries for the frequency range
	set<band_entry_t*> get_entries(double lower, double upper);

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


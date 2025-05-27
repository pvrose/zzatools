#pragma once

#include "band.h"

#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

// data structures
struct range_t {
	double lower;
	double upper;

	// Provide comparison operator for set ordering
	bool operator< (const range_t& rhs) const {
		return lower < rhs.lower;
	}
};   

// Frqeuency range - MHz


// This class reads in the IARU band-plan in TSV form into a database
// It provides access to this database
class band_data
{
public:
	// Band entry structure
	struct band_entry_t {
		range_t range;      // Lower to upper frequency range (MHz)
		double bandwidth;   // Maximum bandwidth usable in sub-band
		set<string> modes;        // Modes allowed
		string summary;     // Summary display
		// Default constructor
		band_entry_t() :
			range({0.0, 0.0}),
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
	set<band_entry_t*> get_entries();
	// Get the bands data
	band_map<set<range_t> >& bands();

protected:
	// Read the data
	bool load_data();
	// Process the daat to bands
	void create_bands();
	// Get the file name
	string get_path();
	// Find the band_plan.tsv file
	bool find_and_copy_data();
	// Parse the band entry
	band_entry_t* get_entry(string line);
	// The band entries
	vector<band_entry_t*> entries_;
	// Full bands
	band_map<set<range_t> > bands_;

};


#pragma once

#include "band.h"

#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

// data structures
//! Structure represents a range of frequencies 
struct range_t {
	double lower; //!< Lower frequency (MHz)
	double upper; //!< Upper frequency (MHz)

	//! less-than (<) operator provided for ordering for set<range_t>.
	//! \param rhs RHS of &lt; operator
	//! \return comparison between the lower frequencies of the two ranges.
	bool operator< (const range_t& rhs) const {
		return lower < rhs.lower;
	}
};   

//! This class reads in the IARU band-plan in TSV form into a database.
//! It provides access to this database
class band_data
{
public:
	//! Band entry structure
	struct band_entry_t {
		range_t range;      //!< Lower to upper frequency range (MHz)
		double bandwidth;   //!< Maximum bandwidth usable in sub-band
		set<string> modes;  //!< Modes allowed
		string summary;     //!< Summary display

		//! Default Constructor 
		band_entry_t() :
			range({0.0, 0.0}),
			bandwidth(0.0),
			modes({}),
			summary("") {};
	};

	//! Constructor
	band_data();
	//! Destructor
	~band_data();

	//! Checks that the supplied frequency is within any band.
	//! \param frequency (MHz) to check.
	//! \return true if in any band.
	bool in_band(double frequency);
	//! Get the band entry for the frequency.
	//! \param frequency (MHz) to check.
	//! \return the band_entry_t structure containing \a frequency.
	band_entry_t* get_entry(double frequency);
	//! Get all the band_entry_t items.
	//! \return all band_entry_t items.
	set<band_entry_t*> get_entries();
	//! Get the bands data
	//! \return the band database.
	band_map<set<range_t> >& bands();

protected:
	//! Read the data
	bool load_data();
	//! Process the data to bands
	void create_bands();
	//! Get the file name
	//! \return directory containing the bandplan data.
	string get_path();
	//! Find the band_plan.tsv file
	//! If the bandplan data is not available, a file browser is opened to 
	//! and the selected file will be copied to the correct location
	//! \return true if successful, false if not.
	bool find_and_copy_data();
	//! Parse the band entry. Copies the line of data in the input file and
	//! creates a band_entry_t item for it.
	//! \return true if successful, false if not.
	band_entry_t* get_entry(string line);
	//! The database of band_entry_t items.
	vector<band_entry_t*> entries_;
	//! A map referencing band names to frequency ranges.
	band_map<set<range_t> > bands_;

};


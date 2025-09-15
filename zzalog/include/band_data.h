#pragma once

#include "band.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>
#include <set>
#include <map>

using json = nlohmann::json;

// data structures
//! Structure represents a range of frequencies 
struct range_t {
	double lower; //!< Lower frequency (MHz)
	double upper; //!< Upper frequency (MHz)

	//! less-than (<) operator provided for ordering for std::set<range_t>.
	
	//! \param rhs RHS of &lt; operator
	//! \return comparison between the lower frequencies of the two ranges.
	bool operator< (const range_t& rhs) const {
		return lower < rhs.lower;
	}
};  

//! This class reads in the IARU band-plan in JSON into a database.

//! It provides access to this database
class band_data
{
public:
	//! Band entry structure
	struct band_entry_t {
		range_t range;      //!< Lower to upper frequency range (MHz)
		double bandwidth;   //!< Maximum bandwidth usable in sub-band
		std::set<std::string> modes;  //!< Modes allowed
		std::string summary;     //!< Summary display

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
	std::set<band_entry_t*> get_entries();
	//! Get the bands data
	
	//! \return the band database.
	band_map<std::set<range_t> >& bands();

protected:
	//! Store data to JSON file
	void save_json();
	//! Read data from JSON file
	bool load_json();
	//! Process the data to bands
	void create_bands();
	//! Get the file name
	
	//! \return directory containing the bandplan data.
	std::string get_path();
	//! Find the band_plan.json file
	
	//! If the bandplan data is not available, a file browser is opened to 
	//! and the selected file will be copied to the correct location
	//! \return true if successful, false if not.
	bool find_and_copy_data();
	//! The database of band_entry_t items.
	std::vector<band_entry_t*> entries_;
	//! A std::map referencing band names to frequency ranges.
	band_map<std::set<range_t> > bands_;

};

//! Convert range_t to JSON object
void to_json(json& j, const range_t& r);
//! Convert JSON object to range_t
void from_json(const json& j, range_t& r);
//! Convert band_data::band_entry_t to JSON object
void to_json(json & j, const band_data::band_entry_t & e);
//! Convert JSON object to band_data::band_entry_t
void from_json(const json & j, band_data::band_entry_t & e);

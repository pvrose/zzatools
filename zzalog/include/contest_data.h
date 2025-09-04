#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace std::chrono;

class contest_reader;
class contest_score;
class contest_write;

//! Definition of contest timeframe
struct ct_date_t {
	system_clock::time_point start;     //!< Start of the contest period.
	system_clock::time_point finish;    //!< End of the contest period.

	//! Tests if the specified timepoint \p tp is within the contest period.
	bool has_inside(system_clock::time_point tp) {
		return (tp > start) && (tp < finish);
	}

	//! Constructor - defaults start and finish to a valid timepoint.
	ct_date_t() {
		start = system_clock::now();
		finish = system_clock::now();
	}
};

//! Definition of the contest.
struct ct_data_t {
	string algorithm;        //!< Scoring and exchange algorithm.
	ct_date_t date;          //!< Period of the contest.
};

//! Amalgamated contest list entry
struct ct_entry_t {
	string id;               //!< Identifier for the contest.
	string index;            //!< Speciifc instance of the contest (eg 2025)
	ct_data_t* definition;   //!< Contest definition.
};

//! The contest definition database
class contest_data
{
	friend class contest_reader;
	friend class contest_writer;
	friend class contest_dialog;

public:
	//! Constructor.
	contest_data();
	//! Destructor.
	~contest_data();

	//! Get contest data structure for specific contest.
	
	//! \param id Identifier for the contest.
	//! \param ix Identfier of the specific instance.
	//! \param create Create the entry in the database for later editing.
	//! \return contest database entry for the specified contest.
	ct_data_t* get_contest(string id, string ix, bool create = false);
	//! Get the number of contests registered.

	//! \return number of entries in the contest database.
	int get_contest_count();
	//! Get the contest entry as indexed.
	
	//! \param number the index of the entry.
	ct_entry_t* get_contest_info(int number);
	//! Get all the instance identifiers for specific contest
	
	//! \param id identifier of the contest.
	//! \return the set of instances of this contest.
	set<string>* get_contest_indices(string id);


protected:
	//! Load data from contests.xml into the internal database.
	bool load_data();
	//! Store the internal database into contests.xml.
	bool save_data();

	// The databases 
	//! Individual contests mapped by ID and index (e.g. year)
	
	//! - <B>Outer map</B> Addressed by instance identifier.
	//! - <B>Inner map</B> Addressed by contest identifier.
	map<string, map<string, ct_data_t*> > contests_;
	//! Consolidated database of all contest entries.
	vector<ct_entry_t*> contest_infos_;

};


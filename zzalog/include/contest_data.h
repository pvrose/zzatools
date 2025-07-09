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

// Definition of contest timeframe
struct ct_date_t {
	system_clock::time_point start;
	system_clock::time_point finish;

	bool has_inside(system_clock::time_point tp) {
		return (tp > start) && (tp < finish);
	}

	ct_date_t() {
		start = system_clock::now();
		finish = system_clock::now();
	}
};

// Definition of the contest
struct ct_data_t {
	string fields;           // "Pointer" to displayed fields list
	ct_date_t date;          // Date of the contest
	string exchange;         // "Pointer" to exchange description
	string scoring;          // "Pointer" to scoring description
};

// Exchange data
struct ct_exch_t {
	string sending;         // "Mail-merge" type description for sending
	string receive;         // "Mail-merge" type description for receiving
};

const vector < pair<string, string> > SCORING_ALGOS = {
	{"Basic", "1 point per QSO (not own DXCC), multiplier = number of DXCCs per band"}
};

// Amalgamated contest list
struct ct_entry_t {
	string id;
	string index;
	ct_data_t* definition;
};

// The cpntest definition database
class contest_data
{
	friend class contest_reader;
	friend class contest_writer;
	friend class contest_dialog;

public:
	contest_data();
	~contest_data();

	// Get contest data structure for contest "id" 
	ct_data_t* get_contest(string id, string ix, bool create = false);

	int get_contest_count();
	ct_entry_t* get_contest_info(int number);
	set<string>* get_contest_indices(string id);

	// Get exchanged data for exchange "id"
	ct_exch_t* get_exchange(string id, bool create = false);
	set<string>* get_exchange_indices();


protected:
	// Load data
	bool load_data();
	// Save data
	bool save_data();

	// The databases 
	// Contests mapped by ID and index (e.g. year)
	map<string, map<string, ct_data_t*> > contests_;
	// Exchanges mapped by ID
	map<string, ct_exch_t*> exchanges_;
	// Contest info
	vector<ct_entry_t*> contest_infos_;

};


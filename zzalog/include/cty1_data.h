#pragma once

#include "cty_data.h"

#include "utils.h"

#include <string>
#include <ctime>
#include <map>
#include <list>
#include <cmath>

using namespace std;

class record;


// This class provides a wrapper for all  the callsign exception data in cty.xml
class cty1_data : public cty_data
{
public:

	// List of entities
	struct entity_entry {
		int adif_id;
		string name;
		string prefix;
		int cq_zone;
		string continent;
		double longitude;
		double latitude;
		bool deleted;
		time_t start;
		time_t end;
		bool whitelist;
		time_t whitelist_start;

		entity_entry() {
			adif_id = 0;
			name = "";
			prefix = "";
			cq_zone = 0;
			continent = "";
			longitude = nan("");
			latitude = nan("");
			deleted = false;
			start = -1;
			end = -1;
			whitelist = false;
			whitelist_start = -1;
		}
	};

	// An exception entry - it will be mapped by callsign to a list of them
	struct exc_entry {
		string call;
		int adif_id;
		int cq_zone;
		string continent;
		double longitude;
		double latitude;
		time_t start;
		time_t end;

		exc_entry() {
			call = "";
			adif_id = 0;
			cq_zone = 0;
			continent = "";
			longitude = nan("");
			latitude = nan("");
			start = -1;
			end = -1;
		}
	};

	// A prefix entry
	struct prefix_entry {
		string call;
		string entity;
		int adif_id;
		int cq_zone;
		string continent;
		double longitude;
		double latitude;
		time_t start;
		time_t end;

		prefix_entry() {
			call = "";
			entity = "";
			adif_id = 0;
			cq_zone = 0;
			continent = "";
			longitude = nan("");
			latitude = nan("");
			start = -1;
			end = -1;
		}

	};

	// An invalid entry
	struct invalid_entry {
		string call;
		time_t start;
		time_t end;

		invalid_entry() {
			call = "";
			start = -1;
			end = -1;
		}
	};

	// Zone exception
	struct zone_entry {
		string call;
		int cq_zone;
		time_t start;
		time_t end;

		zone_entry() {
			call = "";
			cq_zone = 0;
			start = -1;
			end = -1;
		}
	};

	// Parse results
	struct parse_result {
		bool invalid;
		exc_entry* exception;
		zone_entry* zone_exception;
		prefix_entry* prefix;
	};

	cty1_data();
	~cty1_data();

	// Check timeliness of data
	bool data_valid(string filename);
	// Load data 
	bool load_data(string filename);
	// Return various fields of entity
	string nickname(record* qso);
	string name(record* qso);
	string continent(record* qso);
	int cq_zone(record* qso);
	// Get entity for nickname and vice-versa
	int entity(string nickname);
	string nickname(int adif_id);
	// Get location
	lat_long_t location(record* qso);
	// Update record based on parsing
	bool update_qso(record* qso, bool my_call = false);
	// Get location details
	string get_tip(record* qso);
	// Parsing source
	parse_source_t get_source(record* qso);
	// Return entity 
	int entity(record* qso);
	// Get default values for entity n
	string name(int adif_id);
	string continent(int adif_id);
	int cq_zone(int adif_id);
	
	// Allow the reader to access the data directlt
	friend class cty1_reader;
	friend class pfx_tree;

protected:
	// Get the filename {REFERENCE DIR}/cty.xml
	string get_filename();
	// Delete contents
	void delete_contents();
	// Get parse results
	void parse(record* qso);
	// invalid
	bool invalid(record* qso);
	// Exception
	exc_entry* except(record* qso);
	// Zone exception
	zone_entry* zone_except(record* qso);
	// Get prefix
	prefix_entry* prefix(record* qso);

	// Entity data
	map<int, entity_entry* > entities_;
	// The cty_data data
	map <string, list<exc_entry*> > entries_;
	// Prefix data
	map<string, list<prefix_entry*> > prefixes_;
	// Invalid operations 
	map<string, list<invalid_entry*> > invalids_;
	// Zone exceptions
	map < string, list<zone_entry*> > zones_;
	// File created
	string file_created_;

	// Parse result
	parse_result* parse_result_;
	// Record for last parse result
	record* qso_;
	// Call sign for the previous parse result
	string parse_call_;
};



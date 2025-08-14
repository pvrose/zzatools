#pragma once

#include "cty_element.h"

#include "utils.h"

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include <ostream>
#include <string>

using namespace std;

class record;

// This class provides a wrapper for all  the callsign exception data in cty.xml
class cty_data
{

public:

	// Source of the data - set by type()
	// These can be orred in whcich case the various data are merged (TODO:)
	enum cty_type_t : uint8_t {
		ADIF = 0,
		CLUBLOG,
		COUNTRY_FILES,
		DXATLAS,
		CTY_LIMIT
	};

protected:

	//cty_caps_t capabilities_;
	cty_type_t type_ = ADIF;

	// Data structure
	struct all_data {
		// All the entities - indexed by dxcc_id
		map < int, cty_entity* > entities;
		// All the entity level prefixes - indexed by starting string
		map < string, list<cty_prefix*> > prefixes;
		// All the exceptions
		map < string, list<cty_exception*> > exceptions;
	};

public:

	// Parse source
	enum parse_source_t {
		INVALID,
		NO_DECODE,
		EXCEPTION,
		ZONE_EXCEPTION,
		PREVIOUS,
		DEFAULT
	};

	cty_data();

	virtual ~cty_data();

	// Return various fields of entity
	string nickname(record* qso);
	string name(record* qso);
	string continent(record* qso);
	int cq_zone(record* qso);
	int itu_zone(record* qso);
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
	// Return geography info
	string geography(record* qso);
	// Return usage info
	string usage(record* qso);


	// Get entity for nickname and vice-versa
	virtual int entity(string nickname);
	virtual string nickname(int adif_id);

	// Add an entity
	void add_entity(cty_entity* entry);
	// Add a prefix
	void add_prefix(string pattern, cty_prefix* entry);
	// Add an exception
	void add_exception(string pattern, cty_exception* entry);
	// Add a filter
	void add_filter(cty_entity* entity, cty_filter* entry);

	ostream& out() { return os_; };
	// Get the recorded timestamp
	chrono::system_clock::time_point timestamp(cty_type_t type);
	// get latest data for the type
	bool fetch_data(cty_type_t type);

protected:

	// Load the data 
	bool load_data(string filename);
	// Delete data
	void delete_data(all_data* data);
	// Get the filename
	string get_filename();
	// Merge imported data
	void merge_data();
	// Prepouplate from ADIF
	void load_adif_data();
	// Find the entity, pattern and sub-patterns for the supplied QSO
	void parse(record* qso);

	// Get all the elements that may match
	cty_element* match_pattern(string call, string when, string& matched_call);
	cty_element* match_prefix(string call, string when);
	cty_filter* match_filter(cty_entity* entity, cty_filter::filter_t type, string call, string when);

	// Split call into call body and alternate
	void split_call(string call, string& alt, string& body);

	void dump_database();

	// Parse result is exception - nullptr if not
	cty_exception* exception();
	// Parse result is prefix
	cty_prefix* prefix();
	
	// Get the system timestamp for the named file
	chrono::system_clock::time_point get_timestamp(string filename, int old_age);

	struct {
		// The entity definition
		cty_entity* entity = nullptr;
		// Either an exception or prefix
		cty_element* decode_element = nullptr;
		// Seeecetd geographic filter
		cty_geography* geography = nullptr;
		// Usage filter
		cty_filter* usage = nullptr;
	} parse_result_;

	// Current parse objects
	string current_call_ = "";
	record* current_qso_ = nullptr;

	// The country data
	all_data* data_ = nullptr;

	// The data being imported
	all_data* import_ = nullptr;

	// Merge report
	ofstream os_;

	// Merge report is there
	bool report_warnings_ = false;
	bool report_errors_ = false;

	map<cty_type_t, chrono::system_clock::time_point> timestamps_;
	// Launch time
	chrono::system_clock::time_point now_;

};



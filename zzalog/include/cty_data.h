#pragma once

#include "utils.h"

#include <string>
#include <ctime>
#include <map>
#include <list>
#include <cmath>

using namespace std;

class record;

// This class provides a wrapper for all  the callsign exception data in cty.xml
class cty_data
{

public:

	// Capabailities
	typedef uchar cty_caps_t;
	static const cty_caps_t HAS_NONE = 0;
	// Database has ITU zone data
	static const cty_caps_t HAS_ITU_ZONE = 1;
	// Database has timezone data
	static const cty_caps_t HAS_TIMEZONE = 1 << 1;
	// Database has historical prefixes and usages
	static const cty_caps_t HAS_HISTORY = 1 << 2;
	// Database includes timely exceptions
	static const cty_caps_t HAS_CURRENCY = 1 << 3;

	// Source of the data - set by type()
	// These can be orred in whcich case the various data are merged (TODO:)
	typedef uchar cty_type_t;
	// No data
	static const cty_type_t INVALID_CTY = 0;
	// Data from www.clublog.org
	static const cty_type_t	CLUBLOG = 1;
	// Data from www.country-files.com
	static const cty_type_t COUNTRY_FILES = 1 << 1;
	// Data from dxatlas.com
	static const cty_type_t DXATLAS = 1 << 2;

protected:

	cty_caps_t capabilities_;
	cty_type_t type_;

	// Pattern type
	typedef uint16_t patt_type_t;
	// Pattern represents a call that is unauthorised
	static const patt_type_t INVALID_CALL = 1 << 0;
	// Pattern represents that the CQ Zone is different from the prefix
	static const patt_type_t CQZ_EXCEPTION = 1 << 1;
	// Pattern represents that the ITU zone is different from the prefix
	static const patt_type_t ITUZ_EXCEPTION = 1 << 2;
	// Pattern represents that the entity is different from the what the prefix suggest
	static const patt_type_t DXCC_EXCEPTION = 1 << 3;
	// Pattern represents a division of an entity
	static const patt_type_t GEOG_SUBPATTERN = 1 << 4;
	// Patteren represents a license class or usage for an entoity
	static const patt_type_t CLASS_SUBPATTERN = 1 << 5;
	// Pattern represents a continent exception
	static const patt_type_t CONT_EXCEPTION = 1 << 6;
	// Pattern has time conditions
	static const patt_type_t TIME_DEPENDENT = 1 << 7;
	// Pattern is now deleted
	static const patt_type_t PTN_DELECETED = 1 << 8;

	// Validity time period - where applicable
	struct time_period {
		time_t start = -1;
		time_t end = -1;
	};

	// Pattern entry - indexed by string pattern
	struct patt_entry {
		// Pattern type
		patt_type_t type = 0;
		// DXCC identifier - defined by ARRL DXCC - used as general entity identifier
		int dxcc_id = -1;
		// Pattern name
		string name = "";
		// CQ Zone of different from prefix
		int cq_zone = -1;
		// ITU Zone is different from prefix
		int itu_zone = -1;
		// Continent is different from entity
		string continent = "";
		// Location
		lat_long_t location = { nan(""), nan("") };
		// pattern validity period
		time_period validity;
	};

	// List of patterns that may match but differ by time validity
	typedef list < patt_entry* > patt_matches;

	// Entity entry - indexed by dxcc_id
	struct ent_entry {
		// Contains ID
		int dxcc_id = -1;
		// Default prefix
		string nickname = "";
		// Entity name
		string name = "";
		// Default CQ ZOne
		int cq_zone = -1;
		// Default ITU zZone
		int itu_zone = -1;
		// Continent
		string continent = "";
		// Geographic coordinates
		lat_long_t location = { nan(""), nan("") };
		// Timezone (UTC + N hours)
		double timezone = nan("");
		// Deleted - may not necessarily have time vailidity
		bool deleted = false;
		// Time validity is valid
		bool has_validity = false;
		// Validity
		time_period validity = { -1, -1 };
		// the patterns - with time validity
		map< string, patt_matches > patterns = {};
		// sub-entry prefixes
		map <string, patt_matches > sub_patterns = {};

	};

	// Data structure
	struct all_data {
		// All the entities - indexed by dxxc_id
		map < int, ent_entry* > entities;
		// All the entity level prefixes - indexed by starting string
		map < string, patt_matches > patterns;
		// All the sub-entry level prefixes - indexed by starting string
		map < string, patt_matches > sub_patterns;
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

	void type(cty_type_t t);
	cty_type_t type() { return type_; }

	// Return various fields of entity
	virtual string nickname(record* qso);
	virtual string name(record* qso);
	virtual string continent(record* qso);
	virtual int cq_zone(record* qso);
	virtual int itu_zone(record* qso);
	// Get location
	virtual lat_long_t location(record* qso);
	// Update record based on parsing
	virtual bool update_qso(record* qso, bool my_call = false);
	// Get location details
	virtual string get_tip(record* qso);
	// Parsing source
	virtual parse_source_t get_source(record* qso);
	// Return entity 
	virtual int entity(record* qso);

	// Get entity for nickname and vice-versa
	virtual int entity(string nickname);
	virtual string nickname(int adif_id);

	cty_caps_t capabilities() { return capabilities_; }

protected:

	friend class cty1_reader;
	friend class cty2_reader;
	
	// Load the data 
	bool load_data(string filename);
	// Delete data
	void delete_data();
	// Get the filename
	string get_filename();
	// Find the entity, pattern and sub-patterns for the supplied QSO
	void parse(record* qso);
	// Is valid entity
	bool valid_entity(ent_entry* entry, time_t when);
	// Is valid pattern
	bool valid_pattern(patt_entry* entry, time_t when);

	patt_matches* match_pattern(string call, map<string, patt_matches> patterns, time_t when);

	struct {
		ent_entry* entity;
		patt_entry* pattern;
		patt_matches* sub_patterns;
	} parse_result_;

	// Current parse objects
	string current_call_;
	record* current_qso_;
	time_t current_timestamp_;


	// The country data
	all_data* data_;


};



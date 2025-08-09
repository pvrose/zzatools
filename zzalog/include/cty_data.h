#pragma once

#include "utils.h"

#include <map>
#include <list>
#include <cmath>
#include <ostream>
#include <fstream>
#include <string>

using namespace std;

class record;
class cty_element;
class cty_entity;
class cty_exception;
class cty_prefix;

// This class provides a wrapper for all  the callsign exception data in cty.xml
class cty_data
{

public:

	//// Capabailities
	//typedef uchar cty_caps_t;
	//static const cty_caps_t HAS_NONE = 0;
	//// Database has ITU zone data
	//static const cty_caps_t HAS_ITU_ZONE = 1;
	//// Database has timezone data
	//static const cty_caps_t HAS_TIMEZONE = 1 << 1;
	//// Database has historical prefixes and usages
	//static const cty_caps_t HAS_HISTORY = 1 << 2;
	//// Database includes timely exceptions
	//static const cty_caps_t HAS_CURRENCY = 1 << 3;

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

	//// Pattern type
	//typedef uint16_t patt_type_t;
	//static const patt_type_t PTN_NORMAL = 0;
	//// Pattern represents a call that is unauthorised
	//static const patt_type_t INVALID_CALL = 1 << 0;
	//// Pattern represents that the CQ Zone is different from the prefix
	//static const patt_type_t CQZ_EXCEPTION = 1 << 1;
	//// Pattern represents that the ITU zone is different from the prefix
	//static const patt_type_t ITUZ_EXCEPTION = 1 << 2;
	//// Pattern represents that the entity is different from the what the prefix suggest
	//static const patt_type_t DXCC_EXCEPTION = 1 << 3;
	//// Pattern represents a division of an entity
	//static const patt_type_t GEOG_SUBPATTERN = 1 << 4;
	//// Patteren represents a license class or usage for an entoity
	//static const patt_type_t CLASS_SUBPATTERN = 1 << 5;
	//// Pattern represents a continent exception
	//static const patt_type_t CONT_EXCEPTION = 1 << 6;
	//// Pattern has time conditions
	//static const patt_type_t TIME_DEPENDENT = 1 << 7;
	//// Pattern is now deleted
	//static const patt_type_t PTN_DELETED = 1 << 8;
	//// Pattern requires whole call
	//static const patt_type_t WHOLE_CALL = INVALID_CALL | CQZ_EXCEPTION | ITUZ_EXCEPTION | DXCC_EXCEPTION | CONT_EXCEPTION;

	//// Validity time period - where applicable
	//struct time_period {
	//	string start = "19000101";
	//	string end = "29991231";
	//};

	//// Forward declaration for the below typedef
	//struct patt_entry;

	//// List of patterns that may match but differ by time validity
	//typedef list < patt_entry* > patt_matches;

	//// Pattern entry - indexed by string pattern
	//struct patt_entry {
	//	// Pattern type
	//	patt_type_t type = PTN_NORMAL;
	//	// DXCC identifier - defined by ARRL DXCC - used as general entity identifier
	//	int dxcc_id = -1;
	//	// Pattern name
	//	string name = "";
	//	// CQ Zone of different from prefix
	//	int cq_zone = -1;
	//	// ITU Zone is different from prefix
	//	int itu_zone = -1;
	//	// Continent is different from entity
	//	string continent = "";
	//	// Location
	//	lat_long_t location = { nan(""), nan("") };
	//	// pattern validity period
	//	time_period validity;
	//	// Parent
	//	patt_entry* parent = nullptr;
	//	// Children
	//	patt_matches children = { };
	//};

	//// Entity entry - indexed by dxcc_id
	//struct ent_entry {
	//	// Contains ID
	//	int dxcc_id = -1;
	//	// Default prefix
	//	string nickname = "";
	//	// Entity name
	//	string name = "";
	//	// Default CQ ZOne
	//	int cq_zone = -1;
	//	// Default ITU zZone
	//	int itu_zone = -1;
	//	// Continent
	//	string continent = "";
	//	// Geographic coordinates
	//	lat_long_t location = { nan(""), nan("") };
	//	// Timezone (UTC + N hours)
	//	double timezone = nan("");
	//	// Deleted - may not necessarily have time vailidity
	//	bool deleted = false;
	//	// Time validity is valid
	//	bool has_validity = false;
	//	// Validity
	//	time_period validity;
	//	// the patterns - with time validity
	//	map< string, patt_matches > patterns;
	//};

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

	//void type(cty_type_t t);
	//cty_type_t type() { return type_; }

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

	//cty_caps_t capabilities() { return capabilities_; }

	//// Build list
	//// Add the entry to the list
	//void add_entity(int dxcc_id, ent_entry* entry);
	//// Add the pattern to an entity
	//void add_pattern(string pattern, int dxcc_id, patt_entry* entry);
	//// Add the sub-pattern to a pattern
	//void add_subpattern(string pattern, patt_entry* parent, patt_entry* entry);

	// Add an entity
	void add_entity(cty_entity* entry);
	// Add a prefix
	void add_prefix(string pattern, cty_prefix* entry);
	// Add an exception
	void add_exception(string pattern, cty_exception* entry);

	//// Merge patterns - returns true if merged
	//bool merge_pattern(string pattern, patt_type_t type, patt_entry* original, patt_entry* entry);
	//// Merge entry
	//void merge_entry(ent_entry* orig, ent_entry* entry);

protected:

	friend class cty1_reader;
	friend class cty2_reader;
	
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
	// Is a valid element (timeliness)
	bool is_valid(cty_element* elem);
	//// Is valid entity
	//bool valid_entity(ent_entry* entry, string when);
	//// Is valid pattern
	//bool valid_pattern(patt_entry* entry, string when);

	//patt_matches* match_pattern(string call, map<string, patt_matches> patterns, string when);
	// Get all the elements that may match
	list <cty_element*> match_patterns(string call, string when);
	// Get the prefixes that match
	list <cty_element*> match_prefixes(map<string, list< cty_prefix*> > root, string call, string when);

	// Split call into call body and alternate
	void split_call(string call, string& alt, string& body);
	//void print_pattern(string label, patt_entry* entry);
	//void print_entity(string label, ent_entry* entry);

	//bool overlap_validity(patt_entry* p1, patt_entry* p2);
	//bool overlap_validity(ent_entry* e1, ent_entry* e2);

	void dump_database();

	// Parse result is exception - nullptr if not
	cty_exception* exception();

	//struct {
	//	ent_entry* entity;
	//	patt_entry* pattern;
	//	patt_matches* sub_patterns;
	//} parse_result_;

	struct {
		// The entity definition
		cty_entity* entity = nullptr;
		// Either an exception or prefix and its relevent children - 
		// If prefixes, first is entity prefix, then children
		list<cty_element*> patterns;
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

};



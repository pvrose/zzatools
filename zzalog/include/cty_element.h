#pragma once

#include "utils.h"

#include <cstdint>
#include <list>
#include <map>
#include <ostream>
#include <string>

using namespace std;

// This is the base class for all cty_data elements
class cty_element
{
public:

	// Type of element
	enum type_t : uint8_t {
		CTY_UNSPECIFIED = 0,  
		CTY_ENTITY,           // cty_entity
		CTY_PREFIX,           // cty_prefix
		CTY_EXCEPTION         // cty_exception
	};
	// Time range
	struct time_scope {
		string start = "*";
		string finish = "*";
	};
	// Merge error response
	typedef uint8_t error_t; 
	// No issues
	const static error_t CE_OK = 0;
	// CQ Zones clash
	const static error_t CE_CQ_CLASH = 1 << 0;
	// ITU Zones clash
	const static error_t CE_ITU_CLASH = 1 << 1;
	// Continents clash
	const static error_t CE_CONT_CLASH = 1 << 2;
	// Latitude and/or longitude clash
	const static error_t CE_COORD_CLASH = 1 << 3;
	// Name clash
	const static error_t CE_NAME_CLASH = 1 << 4;

	cty_element();
	~cty_element();

	type_t type_;

	// DXCC identifier - -1=invalid, 0=valid, but not an entity
	int dxcc_id_ = -1;
	// Element name
	string name_ = "";
	// Time validity
	time_scope time_validity_;
	// CQ Zone - -1=invalid
	int cq_zone_ = -1;
	// ITU Zone
	int itu_zone_ = -1;
	// Continent
	string continent_ = "";
	// Co-ordinates
	lat_long_t coordinates_ = { nan(""), nan("") };

	// Merge a similar element into this one
	error_t merge(cty_element* elem);

	// Returns true if elem's valiidty overlaps this validity
	bool time_overlap(cty_element* elem);
	// Return true if this validity wholly contains elem's validity
	bool time_contains(cty_element* elem);
	// Return true if supplied time is within this validity
	bool time_contains(string when);

};

ostream& operator<<(ostream& os, const cty_element& elem);

// Version to be used for entities
class cty_entity : public cty_element {

public:

	cty_entity();
	~cty_entity();

	// Additional fields
	// entitiy has been deleted
	bool deleted_ = false;
	// Entitiy nickname - primary prefix
	string nickname_ = "";

};

ostream& operator<<(ostream& os, const cty_entity& rhs);

// Vesrion to be used for prefixes
class cty_prefix : public cty_element {

public:

	cty_prefix();
	// Ddstructor - delete children
	~cty_prefix();

	// Additional fields
	typedef uint8_t pfx_type_t;
	// Prefix specifies the entity
	const static pfx_type_t PFX_ENTITY = 1;
	// Prefix specifies a geographic subdivision
	const static pfx_type_t PFX_GEOGRAPHY = 2;
	// Prefix specifies a usage (eg license class)
	const static pfx_type_t PFX_USAGE = 3;
	// Prefix has been deleted - orred with above
	const static pfx_type_t PFX_DELETED = 4;
	
	pfx_type_t pfx_type_ = 0;
	// State/province
	string province_ = "";
	// Parent prefix
	cty_prefix* parent_ = nullptr;
	// Child prefixes
	map<string, list <cty_prefix*> > children_;

};

ostream& operator<<(ostream& os, const cty_prefix& elem);

// Version to be used for exceptions
class cty_exception : public cty_element {

public:

	cty_exception();
	~cty_exception();

	// Exception type
	enum exc_type_t : uint8_t {
		EXC_INVALID,
		EXC_OVERRIDE
	};
	exc_type_t exc_type_ = EXC_INVALID;

};

ostream& operator<<(ostream& os, const cty_exception& rhs);

#pragma once

#include "cty_data.h"
#include "cty_element.h"

#include <fstream>
#include <list>
#include <string>

using namespace std;

class cty3_reader
{
	enum rec_type_t : uint8_t {
		CTY_UNDEFINED = 0,
		CTY_ENTITY = 1,
		CTY_GEOGRAPHY = 2,
		CTY_SPECIAL = 3,
		CTY_OLD_ENTITY = 4,
		CTY_OLD_PREFIX = 5,
		CTY_UNRECOGNISED = 6,
		CTY_UNASSIGNED = 7,
		CTY_OLD_GEOGRAPHY = 8,
		CTY_CITY = 9
	};

	enum field_type_t : uint8_t {
		FT_TYPE = 0,          /// Record type
		FT_LONGITUDE = 1,     // integer number of 1/180th degree 
		FT_LATITUDE = 2,      // integer number of 1/180th degree
		FT_NAME = 3,          // Entity, geography or special use
		FT_NICKNAME = 4,      // Usual prefix - eg GM for Scotland
		FT_CQZ = 5,           // CQ Zone(s)
		FT_ITUZ = 6,          // ITU Zone(s)
		FT_CONTINENT = 7,     // Continent(s)
		FT_TIMEZONE = 8,      // Timezones - ignored
		FT_DXCC_ID = 9,       // DXCC identifier (eg 279 for Scotland)
		FT_PROVINCE = 10,     // Province (VE), State(W)
		FT_START = 11,        // Start of time validity
		FT_FINISH = 12,       // Finish of time validity
		FT_PFX_MASK = 13,     // Pattersn to generate prefix look-ups
		FT_SOURCE = 14        // Ignored
	};

public:

	cty3_reader();
	~cty3_reader();

	// Load data from specified file into and add each record to the map
	bool load_data(cty_data* data, istream& in, string& version);

protected:

	// Decode entity record
	void load_entity(string line, bool deleted);
	// Decode geography record
	void load_geography(string line, list<cty_prefix*> parents, bool deleted);
	// Decode usage record
	void load_usage(string line, list<cty_prefix*> parents);
	// Load element part of above
	cty_element* load_element(cty_element::type_t type, string line, string& nickname, string& patterns);
	// Convert prefix mask to list of prefixes
	list<string> expand_mask(string patterns);

	cty_data* data_ = nullptr;

	list<cty_prefix*> current_prefixes_;


};


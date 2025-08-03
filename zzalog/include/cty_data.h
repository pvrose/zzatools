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
	enum cty_caps_t {
		HAS_NONE = 0,
		HAS_ITU_ZONE = 1,
		HAS_TIMEZONE = 1 << 1,
		HAS_HISTORY = 1 << 2,
	};

protected:

	cty_caps_t capabilities_;


public:

	// Parse source
	enum parse_source_t {
		INVALID,
		NO_DECODE,
		EXCEPTION,
		ZONE_EXCEPTION,
		DEFAULT
	};

	cty_data() { capabilities_ = HAS_NONE; };
	virtual ~cty_data() {};

	// Return various fields of entity
	virtual string nickname(record* qso) = 0;
	virtual string name(record* qso) = 0;
	virtual string continent(record* qso) = 0;
	virtual int cq_zone(record* qso) = 0;
	virtual int itu_zone(record* qso) = 0;
	// Get location
	virtual lat_long_t location(record* qso) = 0;
	// Update record based on parsing
	virtual bool update_qso(record* qso, bool my_call = false) = 0;
	// Get location details
	virtual string get_tip(record* qso) = 0;
	// Parsing source
	virtual parse_source_t get_source(record* qso) = 0;
	// Return entity 
	virtual int entity(record* qso) = 0;

	// Get entity for nickname and vice-versa
	virtual int entity(string nickname) = 0;
	virtual string nickname(int adif_id) = 0;

	cty_caps_t capabilities() { return capabilities_; }

};



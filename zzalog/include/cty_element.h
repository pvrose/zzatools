#pragma once

#include "utils.h"

#include <cstdint>
#include <list>
#include <map>
#include<ostream>
#include <string>



class cty_filter;

//! This is the base class for all cty_data elements
class cty_element
{
public:

	//! Type of element
	enum type_t : uint8_t {
		CTY_UNSPECIFIED = 0,  
		CTY_ENTITY,           //!< Entity element (cty_entity)
		CTY_PREFIX,           //!< Prefix elemeent (cty_prefix)
		CTY_EXCEPTION,        //!< Exception element (cty_exception)
		CTY_GEOGRAPHY,        //!< Geographic subdivision of entity (cty_geography)
		CTY_FILTER            //!< Usage element (cty_filter)
	};
	//! Time range - in ADIF date format "YYYYMMDD".
	struct time_scope {
		std::string start = "*";     //!< Start of validity of data.
		std::string finish = "*";    //!< End of validity of data.
	};
	//! Merge error response
	typedef uint8_t error_t; 
	//! No issues
	const static error_t CE_OK = 0;
	//! CQ Zones clash
	const static error_t CE_CQ_CLASH = 1 << 0;
	//! ITU Zones clash
	const static error_t CE_ITU_CLASH = 1 << 1;
	//! Continents clash
	const static error_t CE_CONT_CLASH = 1 << 2;
	//! Latitude and/or longitude clash
	const static error_t CE_COORD_CLASH = 1 << 3;
	//! Name clash
	const static error_t CE_NAME_CLASH = 1 << 4;
	//! Deleted clash
	const static error_t CE_DEL_CLASH = 1 << 5;
	//! Non element class
	const static error_t CE_OTHER_CLASH = 1 << 7;

	//! Constructor.
	cty_element();
	//! Destructor.
	~cty_element();

	//! Type of element.
	type_t type_;

	//! DXCC identifier - -1=invalid, 0=valid, but not an entity
	int dxcc_id_ = -1;
	//! Element name
	std::string name_ = "";
	//! Time validity
	time_scope time_validity_;
	//! CQ Zone - -1=invalid
	int cq_zone_ = -1;
	//! ITU Zone
	int itu_zone_ = -1;
	//! Continent
	std::string continent_ = "";
	//! Co-ordinates
	lat_long_t coordinates_ = { nan(""), nan("") };
	//! Item is no longer valid, but has been.
	bool deleted_ = false;
	//! Filters that can be applied to this item.
	std::list<cty_filter*> filters_ = {};

	//! Merge a similar element \p elem into this one, returning error_t showing any clash.
	error_t merge(cty_element* elem);

	//! Returns true if \p elem valiidty overlaps this validity, false if not.
	bool time_overlap(cty_element* elem);
	//! Returns true if this validity wholly contains \p elem validity.
	bool time_contains(cty_element* elem);
	//! Return true if supplied time (\p when) is within this validity.
	bool time_contains(std::string when);

};

//! Output streaming operator "<<" for a cty_element.
std::ostream& operator<<(std::ostream& os, const cty_element& elem);

//! Version of cty_element to be used for entities.
class cty_entity : public cty_element {

public:

	//! Constructor.
	cty_entity();
	//! Destructor.
	~cty_entity();

	//! Merge a similar element \p elem into this one.
	virtual error_t merge(cty_element* elem);

	// Additional fields
	//! Entitiy nickname - usually the primary prefix (eg GM for Scotland).
	std::string nickname_ = "";

};

//! Output streaming operator "<<" for a cty_entity.
std::ostream& operator<<(std::ostream& os, const cty_entity& rhs);

//! Vesrion of cty_element to be used for prefixes
class cty_prefix : public cty_element {

public:

	//! Constructor.
	cty_prefix();
	//! Destructor.
	~cty_prefix();

};

//! Output streaming operator "<<" for a cty_prefix.
std::ostream& operator<<(std::ostream& os, const cty_prefix& elem);

// Version of cty_exception to be used for exceptions
class cty_exception : public cty_element {

public:

	//! Constructor.
	cty_exception();
	//! Destructor.
	~cty_exception();

	//! Exception type
	enum exc_type_t : uint8_t {
		EXC_INVALID,        //!< Callsign used has not been validly assigned to the station.
		EXC_OVERRIDE        //!< The default parsing of callsign is overridden by this exception.
	};
	//! Exception type in this entry.
	exc_type_t exc_type_ = EXC_INVALID;

};

//! Output streaming operator "<<" for a cty_exception.
std::ostream& operator<<(std::ostream& os, const cty_exception& rhs);

// Version of cty_element to be used in geographic or usage filter
class cty_filter : public cty_element {

public:
	//! Constructor.
	cty_filter();
	//! Destructor.
	~cty_filter();

	//! Filter type
	enum filter_t : uint8_t {
		FT_NOT_USED = 0,          
		FT_GEOGRAPHY,             //!< Filter to a geographic dub-division
		FT_USAGE                  //!< Filter to a specific usag
	} filter_type_ = FT_NOT_USED;

	//! Filtering patterns.
	std::string pattern_ = "";
	//! Nickname used for filter. 
	std::string nickname_ = "";
	//! Reason for filter
	std::string reason_ = "";

};

//! Output streaming operator "<<" for a cty_filter.
std::ostream& operator <<(std::ostream& os, const cty_filter& rhs);

//! Version of cty_element used for geographic filters.
class cty_geography : public cty_filter {

public:
	//! Constructor.
	cty_geography();
	//! Destructor.
	~cty_geography();
	//! Primary administrative sub-division where unique to this filter.
	std::string province_ = "";

};

//! Output streaming operator "<<" for a cty_geography.
std::ostream& operator <<(std::ostream& os, const cty_geography& rhs);

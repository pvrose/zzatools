#pragma once

#include "cty_data.h"
#include "cty_element.h"

#include <fstream>
#include <list>
#include <string>



//! Class to read prefix.lst from dxatlas.com into country data database.
class cty3_reader
{
	//! Record type - matches codes in prefix.lst.
	enum rec_type_t : uint8_t {
		CTY_UNDEFINED = 0,        //!< Undefined prefix.
		CTY_ENTITY = 1,           //!< Represents a current DXCC entity.
		CTY_GEOGRAPHY = 2,        //!< A geographic subdivision of an entity.
		CTY_SPECIAL = 3,          //!< A special use prefix.
		CTY_OLD_ENTITY = 4,       //!< A former DXCC entity.
		CTY_OLD_PREFIX = 5,       //!< A prefix formerly used for an entity, but reassigned to another.
		CTY_UNRECOGNISED = 6,     //!< A prefix that is used but has no formal recognition.
		CTY_UNASSIGNED = 7,       //!< A prefix that is unassigned
		CTY_OLD_GEOGRAPHY = 8,    //!< A prefis used for a former geographical subdivision.
		CTY_CITY = 9              //!< Special geographic subdivision.
	};

	//! Each record has these fields in a comma-separated std::list.
	enum field_type_t : uint8_t {
		FT_TYPE = 0,          //!< Record type.
		FT_LONGITUDE = 1,     //!< Longitude in an integer number of 1/180th degree (+ve east).
		FT_LATITUDE = 2,      //!< Latitude in an integer number of 1/180th degree (+ve north).
		FT_NAME = 3,          //!< Entity, geography or special use.
		FT_NICKNAME = 4,      //!< Usual prefix - eg GM for Scotland.
		FT_CQZ = 5,           //!< CQ Zone(s).
		FT_ITUZ = 6,          //!< ITU Zone(s).
		FT_CONTINENT = 7,     //!< Continent(s).
		FT_TIMEZONE = 8,      //!< Timezones - ignored.
		FT_DXCC_ID = 9,       //!< DXCC identifier (eg 279 for Scotland).
		FT_PROVINCE = 10,     //!< Province (VE), State(W).
		FT_START = 11,        //!< Start of time validity.
		FT_FINISH = 12,       //!< Finish of time validity.
		FT_PFX_MASK = 13,     //!< Patterns to generate prefix look-ups.
		FT_SOURCE = 14        //!< Ignored.
	};

public:

	// Constructor.
	cty3_reader();
	// Destructor.
	~cty3_reader();

	//! Load data

	//! \param data Internal database.
	//! \param in input stream.
	//! \param version Returns any version information in the file.
	//! \return true if successful, false if not.
	bool load_data(cty_data* data, std::istream& in, std::string& version);

protected:

	//! Decode entity record from \p line.
	cty_element* load_entity(std::string line, bool deleted);
	//! Decode geography record from \p line.
	cty_element* load_geography(std::string line, bool deleted);
	//! Decode usage record from \p line.
	cty_element* load_usage(std::string line);
	//! Load element part of above
	
	//! \param type Type of element.
	//! \param line Input text.
	//! \param nickname Returns the nickname.
	//! \param patterns Returns the pattern text.
	//! \return cty_element record representing the line.
	cty_element* load_element(cty_element::type_t type, std::string line, std::string& nickname, std::string& patterns);
	//! Convert prefix mask to std::list of prefixes
	std::list<std::string> expand_mask(std::string patterns);

	//! Internal import database.
	cty_data* data_ = nullptr;

	//! The most recent element at each level.
	
	//! level 0 will be an entity, other levels will be filters.
	std::vector<cty_element*> current_elements_;


};


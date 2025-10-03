#pragma once

#include <string>
#include <map>
#include <set>

//! Station type
enum stn_type : uint8_t {
	NOT_USED,                         //!< Default - not valid
	INDIVIDUAL,                       //!< Individual licensee
	CLUB                              //!< Club station
};

//! Default station data
struct stn_default {
	stn_type type{ NOT_USED };        //!< Station type: individual or club
	std::string callsign{ "" };       //!< Station callsign
	std::string location{ "" };       //!< Location identifier
	std::string grid{ "" };           //!< Grid square locator
	std::string name{ "" };           //!< Club or individual name
};

//! Identifiers for ADIF fields indicating user's location
enum qth_value_t : char {
	STREET,                //!< MY_STREET
	CITY,                  //!< MY_CITY
	POSTCODE,              //!< MY_POSTAL_CODE
	LOCATOR,               //!< MY_GRIDSQAURRE
	DXCC_NAME,             //!< MY_COUNTRY
	DXCC_ID,               //!< MY_DXCC
	PRIMARY_SUB,           //!< MY_STATE
	SECONDARY_SUB,         //!< MY_CNTY
	CQ_ZONE,               //!< MY_CQ_ZONE
	ITU_ZONE,              //!< MY_ITU_ZONE
	CONTINENT,             //!< APP_ZZA_MY_CONT
	IOTA,                  //!< MY_IOTA
	WAB,                   //!< APP_ZZA_MY_WAB
	DESCRIPTION            //!< No ADIF equivalent
};

//! Station location data
struct qth_info_t {
	std::map<qth_value_t, std::string> data;
};

//! Mapping identifiers to ADIF field names.
const std::map<qth_value_t, std::string> QTH_ADIF_MAP = {
	{ STREET, "MY_STREET" },
	{ CITY, "MY_CITY" },
	{ POSTCODE, "MY_POSTAL_CODE" },
	{ LOCATOR, "MY_GRIDSQUARE" },
	{ DXCC_NAME, "MY_COUNTRY", },
	{ DXCC_ID, "MY_DXCC", },
	{ PRIMARY_SUB, "MY_STATE" },
	{ SECONDARY_SUB, "MY_CNTY" },
	{ CQ_ZONE, "MY_CQ_ZONE" },
	{ ITU_ZONE, "MY_ITU_ZONE" },
	{ CONTINENT, "APP_ZZA_MY_CONT" },
	{ IOTA, "MY_IOTA" },
	{ WAB, "APP_ZZA_MY_WAB" }
};

// Conversion from enum qth_value_t to string
static std::map<qth_value_t, std::string> QTH_VALUE_T_2_STRING = {
	{ STREET, "Street" },
	{ CITY, "City" },
	{ POSTCODE, "Postcode" },
	{ LOCATOR, "Locator" },
	{ DXCC_NAME, "Country" },
	{ DXCC_ID, "DXCC" },
	{ PRIMARY_SUB, "Primary Subdivision" },
	{ SECONDARY_SUB, "Secondary Subdivision" },
	{ CQ_ZONE, "CQ Zone" },
	{ ITU_ZONE, "ITU Zone" },
	{ CONTINENT, "Continent" },
	{ IOTA, "IOTA" },
	{ WAB, "WAB" },
	{ DESCRIPTION, "Description"}
};

static std::map<std::string, qth_value_t> STRING_2_QTH_INFO_T = {
	{ "Street", STREET },
	{ "City", CITY },
	{ "Postcode", POSTCODE },
	{ "Locator", LOCATOR },
	{ "Country", DXCC_NAME },
	{ "DXCC", DXCC_ID },
	{ "Primary Subdivision", PRIMARY_SUB },
	{ "Secondary Subdivision", SECONDARY_SUB },
	{ "CQ Zone", CQ_ZONE },
	{ "ITU Zone", ITU_ZONE },
	{ "Continent", CONTINENT },
	{ "IOTA", IOTA },
	{ "WAB", WAB },
	{ "Description", DESCRIPTION }
};

//! Identifiers for field indicating station operator
enum oper_value_t : char {
	NAME,           //!< Operator's name
	CALLSIGN,       //!< Operator's callsign
	MAX_OPER        //!< Not used
};

//! Station operator database.
struct oper_info_t {
	std::map< oper_value_t, std::string> data;
};

//! Map of identifiers to ADIF field names
const std::map<oper_value_t, std::string> OPER_ADIF_MAP = {
	{ NAME, "MY_NAME" },
	{ CALLSIGN, "OPERATOR" }
};

//! This calls manages the database for station details (location, operators and callsigns)

class stn_data
{
public:
	//! Constructor.
	stn_data();
	//! Destructor.
	~stn_data();
	//! Load data from station.xml
	void load_data();
	//! LOad data from station.json
	bool load_json();
	//! Store data to station.json
	bool store_json();
	//! Add a specific \p item to location \p id, \p value - returns true if added
	bool add_qth_item(std::string id, qth_value_t item, std::string value);
	//! Remove \p item from location \p id. 
	void remove_qth_item(std::string id, qth_value_t item);
	//! Remove \p id: Retunrs true if successful
	bool delete_qth(std::string id);
	//! Add a new QTH \p id with data \p qth.
	bool add_qth(std::string id, qth_info_t* qth);
	//! Replace data for QTH \p id with data \p qth
	bool replace_qth(std::string id, const qth_info_t& qth);
	//! Add a specific item for operator \p id, \p value - return true if added.
	bool add_oper_item(std::string id, oper_value_t item, std::string value);
	//! Add a new operator \p with data \p oper.
	bool add_oper(std::string id, oper_info_t* oper);
	//! Delete details for opertaor
	bool delete_oper(std::string id);
	//! Add a new station callsign
	bool add_call(std::string call);
	//! Returns the location data for \p id.
	const qth_info_t* get_qth(std::string id);
	//! Returns the operator data for \p id.
	const oper_info_t* get_oper(std::string id);
	//! Returns true if data exists for location \p id.
	bool known_qth(std::string id);
	//! Returns true if data exists for operator \p oper.
	bool known_oper(std::string oper);
	//! Returns true if data exists for callsign \p call.
	bool known_call(std::string call);
	//! Returns all locations
	const std::map<std::string, qth_info_t*>* get_qths();
	//! Returns all operators
	const std::map<std::string, oper_info_t*>* get_opers();
	//! Returns all callsigns
	const std::map<std::string, std::string>* get_calls();
	//! Returns the description for callsign \p id.
	std::string get_call_descr(std::string id);
	//! Add \p description to call \p id
	bool set_call_descr(std::string id, std::string description);
	//! Delete \p call
	bool delete_call(std::string id);

protected:
	//! Station location data
	std::map<std::string, qth_info_t*> qths_;
	//! Station operator data
	std::map<std::string, oper_info_t*> opers_;
	//! Station callsign data
	std::map<std::string, std::string> calls_;
	//! Loaded station defaults
	stn_default loaded_stn_defaults_;
};


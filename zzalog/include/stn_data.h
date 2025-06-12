#pragma once

#include <string>
#include <map>
#include <set>

using namespace std;

enum qth_value_t : char {
	STREET,                // MY_STREET
	CITY,                  // MY_CITY
	POSTCODE,              // MY_POSTAL_CODE
	LOCATOR,               // MY_GRIDSQAURRE
	DXCC_NAME,             // MY_COUNTRY
	DXCC_ID,               // MY_DXCC
	PRIMARY_SUB,           // MY_STATE
	SECONDARY_SUB,         // MY_CNTY
	CQ_ZONE,               // MY_CQ_ZONE
	ITU_ZONE,              // MY_ITU_ZONE
	CONTINENT,             // MY_CONT
	IOTA,                  // MY_IOTA
	WAB                    // APP_ZZA_MY_WAB
};

struct qth_info_t {
	map<qth_value_t, string> data;
	string description;
};

const map<qth_value_t, string> QTH_ADIF_MAP = {
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
	{ CONTINENT, "MY_CONT" },
	{ IOTA, "MY_IOTA" },
	{ WAB, "APP_ZZA_MY_WAB" }
};

enum oper_value_t : char {
	NAME,
	CALLSIGN
};

struct oper_info_t {
	map< oper_value_t, string> data;
	string description;
};

const map<oper_value_t, string> OPER_ADIF_MAP = {
	{ NAME, "MY_NAME" },
	{ CALLSIGN, "OPERATOR" }
};

class stn_data
{
public:
	stn_data();
	~stn_data();

	// Load data from station.xml
	void load_data();
	// Store data to station.xml
	void store_data();
	// Add a specific item - returns true if added
	bool add_qth_item(string id, qth_value_t item, string value);
	// Add description
	bool add_qth_descr(string id, string description);
	// Add a new QTH
	bool add_qth(string id, qth_info_t* qth);
	// Add a specific item
	bool add_oper_item(string id, oper_value_t item, string value);
	// Add description
	bool add_oper_descr(string id, string description);
	// Add a new operator
	bool add_oper(string id, oper_info_t* oper);
	// Add a new station callsign
	bool add_call(string call, string description = "");
	// Fetch the QTH info
	const qth_info_t* get_qth(string id);
	// Fetch the Operatot info
	const oper_info_t* get_oper(string id);
	// Check used before
	bool known_qth(string id);
	bool known_oper(string oper);
	bool known_call(string call);
	// Get all QTHs
	const map<string, qth_info_t*>* get_qths();
	const map<string, oper_info_t*>* get_opers();
	const map<string, string>* get_calls();
	string get_call_descr(string id);

protected:
	// QTH data
	map<string, qth_info_t*> qths_;
	// Operator data
	map<string, oper_info_t*> opers_;
	// Station callsigns
	map<string, string> calls_;
	// Load failed
	bool load_failed_;
};


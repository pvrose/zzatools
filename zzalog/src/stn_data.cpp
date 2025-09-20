#include "stn_data.h"

#include "init_dialog.h"
#include "status.h"

#include "nlohmann/json.hpp"

extern status* status_;
extern std::string default_data_directory_;
extern stn_default station_defaults_;

using json = nlohmann::json;

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
	{ WAB, "WAB" }
};

//! Convert qth_info_t to JSON object
static void to_json(json& j, const qth_info_t& s) {
	for (auto it : s.data) {
		if (it.second.length()) {
			j[QTH_VALUE_T_2_STRING.at(it.first)] = it.second;
		}
	}
}

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
	{ "WAB", WAB }
};

//! Convert JSON object to qth_info_t
static void from_json(const json& j, qth_info_t& s) {
	auto temp = j.get<std::map<std::string, string>>();
	for (auto it : temp) {
		s.data[STRING_2_QTH_INFO_T[it.first]] = it.second;
	}
}

//
static std::map<oper_value_t, std::string> OPER_VALUE_T_2_STRING = {
	{ NAME, "Name" },
	{ CALLSIGN, "Callsign" }
};

//! Convert oper_info_t to JSON object
static void to_json(json& j, const oper_info_t& s) {
	for (auto it : s.data) {
		if (it.second.length()) {
			j[OPER_VALUE_T_2_STRING.at(it.first)] = it.second;
		}
	}
}

static std::map<std::string, oper_value_t> STRING_2_OPER_INFO_T = {
	{ "Name", NAME },
	{ "Callsign", CALLSIGN }
};

//! Convert JSON object to oper_info_t
static void from_json(const json& j, oper_info_t& s) {
	auto temp = j.get<std::map<std::string, string>>();
	for (auto it : temp) {
		s.data[STRING_2_OPER_INFO_T[it.first]] = it.second;
	}
}

// JSON conversion for stn_type
NLOHMANN_JSON_SERIALIZE_ENUM(stn_type, {
	{ NOT_USED, "Not used" },
	{ CLUB, "Club" },
	{ INDIVIDUAL, "Individual "}
})

// Convert stn_default to JSON object
static void to_json(json& j, const stn_default& s) {
	j = json{
		{ "Station type", s.type },
		{ "Callsign", s.callsign},
		{ "Location", s.location },
		{ "Gridsquare", s.grid },
		{ "Name", s.name }
	};
}

// Convert stn_default to JSON object
static void from_json(const json& j, stn_default& s) {
	j.at("Station type").get_to(s.type);
	j.at("Callsign").get_to(s.callsign);
	j.at("Location").get_to(s.location);
	j.at("Gridsquare").get_to(s.grid);
	j.at("Name").get_to(s.name);
}

stn_data::stn_data()
{
	load_data();
}

stn_data::~stn_data() {
    store_json();
}

// Load data from station.xml
void stn_data::load_data() {
	char msg[128];
	status_->misc_status(ST_NOTE, "STN DATA: Loading operation data");
	bool loaded = load_json();
	if (loaded_stn_defaults_.callsign.length() ==0 && (
		station_defaults_.type == NOT_USED || station_defaults_.callsign.length() == 0)) {
		status_->misc_status(ST_ERROR, "STN DATA: We have no default callsign etc.");
		// First use - open dialog to get station defaults.
		init_dialog* dlg = new init_dialog();
		while (dlg->visible()) Fl::check();
		station_defaults_ = dlg->get_default();

	}
	else if (loaded_stn_defaults_.callsign.length() && (
		station_defaults_.type == NOT_USED || station_defaults_.callsign.length() == 0)) {
		stn_type save_type = station_defaults_.type;
		station_defaults_ = loaded_stn_defaults_;
		station_defaults_.type = save_type;
	
		status_->misc_status(ST_NOTE, "STN DATA: Using saved station defaults");
	}
	else {
		status_->misc_status(ST_NOTE, "STN DATA: Using initial station defaults");
	}
	snprintf(msg, sizeof(msg), "STN DATA: Station defaults: Call=%s, Location=%s(%s), Name=%s",
		station_defaults_.callsign.c_str(),
		station_defaults_.location.c_str(),
		station_defaults_.grid.c_str(),
		station_defaults_.name.c_str());
	status_->misc_status(ST_NOTE, msg);
	// If no data loaded
	if (!loaded) {
		// Create a set of initial values from input defaults.
		qth_info_t* qth_info = new qth_info_t;
		(qth_info->data)[LOCATOR] = station_defaults_.grid;
		add_qth(station_defaults_.location, qth_info);
		oper_info_t* oper_info = new oper_info_t;
		add_oper(station_defaults_.name, oper_info);
		add_call(station_defaults_.callsign);
	}
}

// Load data from station.json
bool stn_data::load_json() {
	std::string filename = default_data_directory_ + "station.json";
	ifstream is;
	char msg[128];
	is.open(filename, std::ios_base::in);
	if (!is.good()) {
		char msg[128];
		snprintf(msg, sizeof(msg), "STN DATA: Failed to open %s", filename.c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	json jall;
	try {
		is >> jall;
		
		json  j = jall["Station"];
		auto temp = j.at("Locations").get<std::vector<std::map<std::string, json>>>();
		for (auto& it : temp) {
			for (auto& ita : it) {
				qth_info_t* qi = new qth_info_t(ita.second.template get<qth_info_t>());
				qths_[ita.first] = qi;
			}
		}
		temp = j.at("Operators").get<std::vector<std::map<std::string, json>>>();
		for (auto& it : temp) {
			for (auto& ita : it) {
				oper_info_t* oi = new oper_info_t(ita.second.template get<oper_info_t>());
				opers_[ita.first] = oi;
			}
		}
		auto temp2 = j.at("Station callsigns").get<std::map<std::string, string>>();
		for (auto& it : temp2) {
			calls_[it.first] = it.second;
		}
		if (j.find("Defaults") != j.end()) {
			j.at("Defaults").get_to(loaded_stn_defaults_);
		}
	}
	catch (const json::exception& e) {
		std::snprintf(msg, sizeof(msg), "STN DATA: Failed to load %s: %d (%s)\n",
			filename.c_str(), e.id, e.what());
		status_->misc_status(ST_ERROR, msg);
		is.close();
		return false;
	}
	std::snprintf(msg, sizeof(msg), "STN DATA: File %s Loaded OK", filename.c_str());
	status_->misc_status(ST_OK, msg);
	return true;
}

// Store data to station.json
bool stn_data::store_json() {
	std::string filename = default_data_directory_ + "station.json";
	std::ofstream os;
	os.open(filename, std::ios_base::out);
	if (os.good()) {
		json jall;
		for (auto& it : qths_) {
			if (it.first.length()) {
				json jq;
				jq[it.first] = *it.second;
				jall["Locations"].push_back(jq);
			}
		}
		for (auto& it : opers_) {
			if (it.first.length()) {
				json jo;
				jo[it.first] = *it.second;
				jall["Operators"].push_back(jo);
			}
		}
		json jc;
		for (auto& it : calls_) {
			if (it.first.length()) {
				jc[it.first] = it.second;
			}
		}
		jall["Station callsigns"] = jc;
		jall["Defaults"] = station_defaults_;
		json j;

		j["Station"] = jall;
		os << std::setw(2) << j << endl;
		return true;
	}
	return false;
}


// Add a specific item - returns true if added
bool stn_data::add_qth_item(std::string id, qth_value_t item, std::string value) {
	if (qths_.find(id) == qths_.end()) {
		// New QTH
		qth_info_t* info = new qth_info_t;
		info->data = { { item, value } };
		qths_[id] = info;
		return true;
	} else {
		if (qths_.at(id)->data.find(item) == qths_.at(id)->data.end()) {
			// New item
			qths_.at(id)->data[item] = value;
			return true;
		}
		else {
			qths_.at(id)->data.at(item) = value;
			return true;
		}
	}
}

// remove an item
void stn_data::remove_qth_item(std::string id, qth_value_t item) {
	if (qths_.find(id) != qths_.end()) {
		qths_.at(id)->data.erase(item);
	}
}

// Add a new QTH
bool stn_data::add_qth(std::string id, qth_info_t* qth) {
	if (qths_.find(id) == qths_.end()) {
		// New QTH
		qths_[id] = qth;
		return true;
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "STN DATA: Already have data for QTH %s", id.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

// Add a specific item - returns true if added
bool stn_data::add_oper_item(std::string id, oper_value_t item, std::string value) {
	if (opers_.find(id) == opers_.end()) {
		// New QTH
		oper_info_t* info = new oper_info_t;
		info->data = { { item, value } };
		opers_[id] = info;
		return true;
	}
	else {
		if (opers_.at(id)->data.find(item) == opers_.at(id)->data.end()) {
			// New item
			opers_.at(id)->data[item] = value;
			return true;
		}
		else {
			opers_.at(id)->data.at(item) = value;
			return true;
		}
	}
}

// Add a new operator
bool stn_data::add_oper(std::string id, oper_info_t* oper) {
	if (opers_.find(id) == opers_.end()) {
		// New QTH
		opers_[id] = oper;
		return true;
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "STN DATA: Already have data for Operator \"%s\"", id.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

// Fetch the QTH info
const qth_info_t* stn_data::get_qth(std::string id) {
	if (qths_.find(id) == qths_.end()) {
		//char msg[128];
		//snprintf(msg, sizeof(msg), "STN DATA: No data for QTH \"%s\"", id.c_str());
		//status_->misc_status(ST_ERROR, msg);
		return nullptr;
	}
	else {
		return qths_.at(id);
	}
}
// Fetch the Operatot info
const oper_info_t* stn_data::get_oper(std::string id) {
	if (opers_.find(id) == opers_.end()) {
		//char msg[128];
		//snprintf(msg, sizeof(msg), "STN DATA: No data for Operator \"%s\"", id.c_str());
		//status_->misc_status(ST_ERROR, msg);
		return nullptr;
	}
	else {
		return opers_.at(id);
	}
}

// Get all QTHs
const std::map<std::string, qth_info_t*>* stn_data::get_qths() {
	return &qths_;
}

// Get all operators
const std::map<std::string, oper_info_t*>* stn_data::get_opers() {
	return &opers_;
}

// Add a new callsign
bool stn_data::add_call(std::string call) {
	if (calls_.find(call) != calls_.end()) {
		return false;
	}
	else {
		calls_[call] = "";
		return true;
	}
}

// get existing calls
const std::map<std::string, std::string>* stn_data::get_calls() {
	return &calls_;
}

// Known call
bool stn_data::known_call(std::string call) {
	if (calls_.find(call) == calls_.end()) return false;
	else return true;
}

// Known call
bool stn_data::known_qth(std::string call) {
	if (qths_.find(call) == qths_.end()) return false;
	else return true;
}

// Known call
bool stn_data::known_oper(std::string call) {
	if (opers_.find(call) == opers_.end()) return false;
	else return true;
}

// Get descriptor for a specific call
std::string stn_data::get_call_descr(std::string id) {
	if (calls_.find(id) == calls_.end()) {
		return "";
	}
	else {
		return calls_.at(id);
	}
}


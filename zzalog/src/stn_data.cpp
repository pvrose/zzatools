#include "stn_data.h"

#include "config.h"
#include "file_holder.h"
#include "init_dialog.h"
#include "main.h"
#include "record.h"
#include "status.h"
#include "stn_dialog.h"

#include "nlohmann/json.hpp"

#include <vector>

using json = nlohmann::json;

//! Convert qth_info_t to JSON object
static void to_json(json& j, const qth_info_t& s) {
	for (auto it : s.data) {
		if (it.second.length()) {
			j[QTH_VALUE_T_2_STRING.at(it.first)] = it.second;
		}
	}
}

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
	{ INDIVIDUAL, "Individual"}
})

// Convert stn_default to JSON object
static void to_json(json& j, const stn_default& s) {
	j = json{
		{ "Station type", s.type },
		{ "Callsign", s.callsign},
		{ "Location", s.location },
		{ "Name", s.name }
	};
}

// Convert stn_default to JSON object
static void from_json(const json& j, stn_default& s) {
	j.at("Station type").get_to(s.type);
	j.at("Callsign").get_to(s.callsign);
	j.at("Location").get_to(s.location);
	j.at("Name").get_to(s.name);
}

stn_data::stn_data()
{
	unknown_qth_index_ = 0;
}

stn_data::~stn_data() {
    store_json();
}

// Load data from station.xml
void stn_data::load_data() {
	char msg[128];
	status_->misc_status(ST_NOTE, "STN DATA: Loading operation data");
	bool loaded = load_json();
	// Set current values
	current_ = defaults_;
	if (defaults_.type == CLUB) {
		current_.name = previous_oper_;
	}
	// If no data loaded or default callsign is not set
	if (!loaded || defaults_.callsign.length() == 0) {
		// Create a set of initial values from input defaults.
		stn_window_->set_tab(stn_dialog::DEFAULTS, "", "No station data loaded, set default values.");
		while (stn_window_->visible()) Fl::check();
	}
	else if (qths_.find(defaults_.location) == qths_.end()) {
		// Create a set of initial values from input defaults.
		stn_window_->set_tab(stn_dialog::QTH, defaults_.location, "QTH not known, please enter details.");
		while (stn_window_->visible()) Fl::check();
	}
	else if (defaults_.type == INDIVIDUAL &&
		opers_.find(defaults_.name) == opers_.end()) {
		// Create a set of initial values from input defaults.
		stn_window_->set_tab(stn_dialog::OPERATOR, defaults_.name, "Operator not known, please enter details.");
		while (stn_window_->visible()) Fl::check();
	}
	else if (defaults_.type == CLUB) {
		// Create a set of initial values from input defaults.
		stn_window_->set_tab(stn_dialog::OPERATOR, previous_oper_, "Club station, please specify operator");
		while (stn_window_->visible()) Fl::check();
	}
	else if (calls_.find(defaults_.callsign) == calls_.end()) {
		// Create a set of initial values from input defaults.
		stn_window_->set_tab(stn_dialog::CALLSIGN, defaults_.callsign, "Station callsign not known, please enter details.");
		while (stn_window_->visible()) Fl::check();
	}
	snprintf(msg, sizeof(msg), "STN DATA: Station defaults: Call=%s, Location=%s, Name=%s",
		defaults_.callsign.c_str(),
		defaults_.location.c_str(),
		defaults_.name.c_str());
	status_->misc_status(ST_NOTE, msg);
}

// Load data from station.json
bool stn_data::load_json() {
	std::string filename;
	ifstream is;
	file_holder_->get_file(FILE_STATION, is, filename);
	char msg[128];
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
				if (ita.second.is_null()) {
					qths_[ita.first] = new qth_info_t;;
				}
				else {
					qth_info_t* qi = new qth_info_t(ita.second.template get<qth_info_t>());
					qths_[ita.first] = qi;
				}
			}

		}
		temp = j.at("Operators").get<std::vector<std::map<std::string, json>>>();
		for (auto& it : temp) {
			for (auto& ita : it) {
				if (ita.second.is_null()) {
					opers_[ita.first] = new oper_info_t;
				}
				else {
					oper_info_t* oi = new oper_info_t(ita.second.template get<oper_info_t>());
					opers_[ita.first] = oi;
				}
			}
		}
		auto temp2 = j.at("Station callsigns").get<std::map<std::string, string>>();
		for (auto& it : temp2) {
			calls_[it.first] = it.second;
		}
		if (j.find("Defaults") != j.end()) {
			j.at("Defaults").get_to(defaults_);
		}
		if (j.find("Previous Operator") != j.end()) {
			j.at("Previous Operator").get_to(previous_oper_);
		}
		else {
			previous_oper_ = "";
		}
	}
	catch (const json::exception& e) {
		snprintf(msg, sizeof(msg), "STN DATA: Failed to load %s: %d (%s)\n",
			filename.c_str(), e.id, e.what());
		status_->misc_status(ST_ERROR, msg);
		is.close();
		return false;
	}
	snprintf(msg, sizeof(msg), "STN DATA: File %s Loaded OK", filename.c_str());
	status_->misc_status(ST_OK, msg);
	return true;
}

// Store data to station.json
bool stn_data::store_json() {
	std::string filename;
	std::ofstream os;
	file_holder_->get_file(FILE_STATION, os, filename);
	if (os.good()) {
		json jall;
		for (auto& it : qths_) {
			if (it.first.length()) {
				json jq;
				if (it.second) jq[it.first] = *it.second;
				else jq[it.first];
				jall["Locations"].push_back(jq);
			}
		}
		for (auto& it : opers_) {
			if (it.first.length()) {
				json jo;
				if (it.second) jo[it.first] = *it.second;
				else jo[it.first];
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
		jall["Defaults"] = defaults_;
		if (defaults_.type == CLUB) {
			jall["Previous Operator"] = current_.name;
		}
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

// // Replace a new QTH
bool stn_data::replace_qth(std::string id, const qth_info_t& qth) {
	if (qths_.find(id) != qths_.end()) {
		delete qths_.at(id);
		qth_info_t* new_data = new qth_info_t(qth);
		qths_[id] = new_data;
		return true;
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "STN DATA: Do not have data for QTH %s", id.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

// Delete QTH
bool stn_data::delete_qth(std::string id) {
	if (qths_.find(id) == qths_.end()) {
		return false;
	}
	else {
		delete qths_.at(id);
		qths_.erase(id);
		return true;
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

// Delete QTH
bool stn_data::delete_oper(std::string id) {
	if (opers_.find(id) == opers_.end()) {
		return false;
	}
	else {
		delete opers_.at(id);
		opers_.erase(id);
		return true;
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

// Delete  a callsign
bool stn_data::delete_call(std::string call) {
	if (calls_.find(call) == calls_.end()) {
		return false;
	}
	else {
		calls_.erase(call);
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

// Set decsription for a specific call
bool stn_data::set_call_descr(std::string id, std::string d) {
	if (calls_.find(id) == calls_.end()) {
		return false;
	}
	else {
		calls_[id] = d;
		return true;
	}

}

//! Match \p qso record against QTHs, result matching \p QTH, returns stn_match_t
stn_data::stn_match_t stn_data::match_qso_qths(record* qso, std::string& qth) {
	char msg[128];
	// Lists of QTHs that match one way or another
	std::vector<std::string> cant_matches;
	std::vector<std::string> do_matches;
	std::vector<std::string> extra_matches;
	std::vector<std::string> dont_matches;
	if (qths_.size() == 0) {
		std::string new_qth = "Unknown " + std::to_string(++unknown_qth_index_);
		update_qth_qso(new_qth, qso);
		return EXTRA;
	}
	// For all QTHs - get result
	for (auto qth : qths_) {
		stn_match_t result = match_qso_qth(qso, *qth.second);
		switch (result) {
		case CANT:
			cant_matches.push_back(qth.first);
			break;
		case DO:
			do_matches.push_back(qth.first);
			break;
		case EXTRA:
			extra_matches.push_back(qth.first);
			break;
		case DONT:
			dont_matches.push_back(qth.first);
			break;
		default:
			break;
		}
	}
	if (cant_matches.size() == qths_.size()) {
		snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s has no QTH data",
			qso->item("QSO_DATE").c_str(),
			qso->item("TIME_ON").c_str(),
			qso->item("CALL").c_str());
		status_->misc_status(ST_NOTE, msg);
		qth = "";
		return CANT;
	}
	else if ((do_matches.size() + extra_matches.size()) > 1) {
		snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s matches multiple QTHs",
			qso->item("QSO_DATE").c_str(),
			qso->item("TIME_ON").c_str(),
			qso->item("CALL").c_str());
		status_->misc_status(ST_NOTE, msg);
		if (do_matches.size()) qth = do_matches[0];
		else qth = extra_matches[0];
		return MULTIPLE;
	}
	else if (do_matches.size() == 1) {
		qth = do_matches[0];
		return DO;
	}
	else if (extra_matches.size() == 1) {
		update_qth_qso(extra_matches[0], qso);
		qth = extra_matches[0];
		return EXTRA;
	}
	else {
		snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s matches no QTHs",
			qso->item("QSO_DATE").c_str(),
			qso->item("TIME_ON").c_str(),
			qso->item("CALL").c_str());
		status_->misc_status(ST_WARNING, msg);
		std::string new_qth = "Unknown " + std::to_string(++unknown_qth_index_);
		update_qth_qso(new_qth, qso);
		qth = new_qth;
		return DONT;
	}
}

//! MAtch \p qso against one QTH \p id
stn_data::stn_match_t stn_data::match_qso_qth(record* qso, qth_info_t qth) {
	bool extra = false;
	bool cant = true;
	for (int ix = 0; ix < qth_value_t::DESCRIPTION; ix++) {
		const qth_value_t& qv = (qth_value_t)ix;
		std::string qsov = qso->item(QTH_ADIF_MAP.at(qv));
		if (qsov.length()) {
			//QSO has this field
			cant = false;
			if (qth.data.find(qv) == qth.data.end() || qth.data.at(qv).length() == 0) {
				extra = true;
			}
			else {
				const std::string& qthv = qth.data.at(qv);
				if (qv == LOCATOR) {
					// Only match minimum length
					int len = min(qsov.length(), qthv.length());
					if (qsov.substr(0, len) != qthv.substr(0, len)) {
						return DONT;
					}
					// If QSO has more detailed GRIDSQUARE value
					if (qsov.length() > qthv.length()) {
						extra = true;
					}
				}
				else if (qsov != qthv) {
					return DONT;
				}
			}
		}
	}
	if (cant) return CANT;
	else if (extra) return EXTRA;
	else return DO;
}

//! Match \p qso record against QTHs, result matching \p QTH, returns stn_match_t
stn_data::stn_match_t stn_data::match_qso_opers(record* qso, std::string& oper) {
	char msg[128];
	// Lists of QTHs that match one way or another
	std::vector<std::string> cant_matches;
	std::vector<std::string> do_matches;
	std::vector<std::string> extra_matches;
	std::vector<std::string> dont_matches;
	if (opers_.size() == 0) {
		std::string new_oper = "Operator " + std::to_string(++unknown_oper_index_);
		update_oper_qso(new_oper, qso);
		return EXTRA;
	}
	// For all opers - get result
	for (auto oper : opers_) {
		stn_match_t result = match_qso_oper(qso, *oper.second);
		switch (result) {
		case CANT:
			cant_matches.push_back(oper.first);
			break;
		case DO:
			do_matches.push_back(oper.first);
			break;
		case EXTRA:
			extra_matches.push_back(oper.first);
			break;
		case DONT:
			dont_matches.push_back(oper.first);
			break;
		default:
			break;
		}
	}
	if (cant_matches.size() == opers_.size()) {
		snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s has no operator data",
			qso->item("QSO_DATE").c_str(),
			qso->item("TIME_ON").c_str(),
			qso->item("CALL").c_str());
		status_->misc_status(ST_NOTE, msg);
		oper = "";
		return CANT;
	}
	else if ((do_matches.size() + extra_matches.size()) > 1) {
		snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s matches multiple operatorss",
			qso->item("QSO_DATE").c_str(),
			qso->item("TIME_ON").c_str(),
			qso->item("CALL").c_str());
		status_->misc_status(ST_NOTE, msg);
		if (do_matches.size()) oper = do_matches[0];
		else oper = extra_matches[0];
		return MULTIPLE;
	}
	else if (do_matches.size() == 1) {
		oper = do_matches[0];
		return DO;
	}
	else if (extra_matches.size() == 1) {
		update_oper_qso(extra_matches[0], qso);
		oper = extra_matches[0];
		return EXTRA;
	}
	else {
		snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s matches no operators",
			qso->item("QSO_DATE").c_str(),
			qso->item("TIME_ON").c_str(),
			qso->item("CALL").c_str());
		status_->misc_status(ST_WARNING, msg);
		std::string new_oper = "Unknown " + std::to_string(++unknown_oper_index_);
		update_oper_qso(new_oper, qso);
		oper = new_oper;
		return DONT;
	}
}

//! MAtch \p qso against one oper \p id
stn_data::stn_match_t stn_data::match_qso_oper(record* qso, oper_info_t oper) {
	bool extra = false;
	bool cant = true;
	for (int ix = 0; ix < MAX_OPER; ix++) {
		const oper_value_t& qv = (oper_value_t)ix;
		std::string qsov = qso->item(OPER_ADIF_MAP.at(qv));
		if (qsov.length()) {
			//QSO has this field
			cant = false;
			if (oper.data.find(qv) == oper.data.end() || oper.data.at(qv).length() == 0) {
				extra = true;
			}
			else {
				const std::string& operv = oper.data.at(qv);
				if (qsov != operv) {
					return DONT;
				}
			}
		}
	}
	if (cant) return CANT;
	else if (extra) return EXTRA;
	else return DO;
}

//! Update QTH from QSO
void stn_data::update_qth_qso(std::string qth, record* qso) {
	char msg[128];
	if (qths_.find(qth) == qths_.end()) {
		add_qth(qth, new qth_info_t);
	}
	for (int ix = 0; ix < qth_value_t::DESCRIPTION; ix++) {
		const qth_value_t& qv = (qth_value_t)ix;
		std::string qsov = qso->item(QTH_ADIF_MAP.at(qv));
		std::string qthv = "";
		if (qths_.at(qth)->data.find(qv) != qths_.at(qth)->data.end()) {
			qthv = qths_.at(qth)->data.at(qv);
		}
		if (qsov.length()) {
			if (qv == LOCATOR) {
				if (qsov.length() > qthv.length()) {
					snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s Update QTH \"%s\": %s=\"%s\"",
						qso->item("QSO_DATE").c_str(),
						qso->item("TIME_ON").c_str(),
						qso->item("CALL").c_str(),
						qth.c_str(),
						QTH_ADIF_MAP.at(qv).c_str(),
						qsov.c_str());
					status_->misc_status(ST_WARNING, msg);
					qths_.at(qth)->data[qv] = qsov;
				}
			}
			else {
				if (qthv.length() == 0) {
					snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s Update QTH \"%s\": %s=\"%s\"",
						qso->item("QSO_DATE").c_str(),
						qso->item("TIME_ON").c_str(),
						qso->item("CALL").c_str(),
						qth.c_str(),
						QTH_ADIF_MAP.at(qv).c_str(),
						qsov.c_str());
					status_->misc_status(ST_WARNING, msg);
					qths_.at(qth)->data[qv] = qsov;
				}
			}
		}
	}
}

//! Update oper from QSO
void stn_data::update_oper_qso(std::string oper, record* qso) {
	char msg[128];
	if (opers_.find(oper) == opers_.end()) {
		add_oper(oper, new oper_info_t);
	}
	for (int ix = 0; ix < MAX_OPER; ix++) {
		const oper_value_t& qv = (oper_value_t)ix;
		std::string qsov = qso->item(OPER_ADIF_MAP.at(qv));
		std::string operv = "";
		if (opers_.at(oper)->data.find(qv) != opers_.at(oper)->data.end()) {
			operv = opers_.at(oper)->data.at(qv);
		}
		if (qsov.length()) {
			if (operv.length() == 0) {
				snprintf(msg, sizeof(msg), "STN DATA: Record %s %s %s Update operator \"%s\": %s=\"%s\"",
					qso->item("QSO_DATE").c_str(),
					qso->item("TIME_ON").c_str(),
					qso->item("CALL").c_str(),
					oper.c_str(),
					OPER_ADIF_MAP.at(qv).c_str(),
					qsov.c_str());
				status_->misc_status(ST_WARNING, msg);
				opers_.at(oper)->data[qv] = qsov;
			}
		}
	}
}

// Return station defaults
stn_default stn_data::defaults() {
	return defaults_;
}

// Set station defaults
void stn_data::set_defaults(const stn_default def) {
	defaults_ = def;
	if (defaults_.location.length() && qths_.find(defaults_.location) == qths_.end()) {
		qths_[defaults_.location] = new qth_info_t;
		qths_.at(defaults_.location)->data[DESCRIPTION] = "Main address";
	}
	if (defaults_.name.length() && opers_.find(defaults_.name) == opers_.end()) {
		opers_[defaults_.name] = new oper_info_t;
		opers_.at(defaults_.name)->data[NAME] = defaults_.name;

	}
	if (defaults_.callsign.length() && calls_.find(defaults_.callsign) == calls_.end()) {
		calls_[defaults_.callsign] = "Principal callsign";
	}
}

// Set station type
void stn_data::set_type(const stn_type t) {
	defaults_.type = t;
}

// Get current
stn_default stn_data::current() {
	return current_;
}

// Set current
void stn_data::set_current(stn_default values) {
	current_ = values;
	current_.type = defaults_.type;
}
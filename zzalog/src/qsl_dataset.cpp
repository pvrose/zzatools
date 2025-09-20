#include "qsl_dataset.h"
#include "extract_data.h"
#include "qrz_handler.h"
#include "status.h"
#include "utils.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>
#include <fstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

extern status* status_;
extern std::string VENDOR;
extern std::string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;
extern uint32_t seed_;

using json = nlohmann::json;

std::string qsl_dataset::server_name_ = "";

//! Map dim_unit to string for JSON serialisaton
NLOHMANN_JSON_SERIALIZE_ENUM(qsl_data::dim_unit, {
	{ qsl_data::INCH, "Inch"},
	{ qsl_data::MILLIMETER, "Millimeter"},
	{ qsl_data::POINT, "Point" }
	})

//! Map qsl_type to string for JSON serialisaton
NLOHMANN_JSON_SERIALIZE_ENUM(qsl_data::qsl_type, {
	{ qsl_data::LABEL, "Label"},
	{ qsl_data::FILE, "File"}
	})

//! Map date_format to string for JSON serialisaton
NLOHMANN_JSON_SERIALIZE_ENUM(qsl_data::date_format, {
	{ qsl_data::FMT_Y4MD_ADIF, "YYYYMMDD"},
	{ qsl_data::FMT_Y4MD, "YYYY/MM/DD"},
	{ qsl_data::FMT_Y2MD, "YY/MM/DD"},
	{ qsl_data::FMT_DMY2, "DD/MM/YY"},
	{ qsl_data::FMT_MDY2, "MM/DD/YY"}
	})

//! Map time_format to string for JSON serialisaton
NLOHMANN_JSON_SERIALIZE_ENUM(qsl_data::time_format, {
	{ qsl_data::FMT_HMS_ADIF, "HHMMSS"},
	{ qsl_data::FMT_HMS, "HH:MM:SS"},
	{ qsl_data::FMT_HM_ADIF, "HHMM"},
	{ qsl_data::FMT_HM, "HH:MM"}
	})

//! Map item_type to string for JSON serialisaton
NLOHMANN_JSON_SERIALIZE_ENUM(qsl_data::item_type, {
	{ qsl_data::NONE, "None"},
	{ qsl_data::FIELD, "Field"},
	{ qsl_data::TEXT, "Text"},
	{ qsl_data::IMAGE, "Image"}
	})

const qsl_data LABEL_QSL_DATA =
{ 
	qsl_data::MILLIMETER, // unit
	101.6,                // width
	67.7,                 // height
	4,                    // rows
	2,                    // columns
	101.6,                // col_width
	67.7,                 // row_height
	12.9,                 // row_top
	4.6,                  // col_left
	1,                    // max_qsos
	qsl_data::FMT_Y4MD_ADIF,   // date_format
	qsl_data::FMT_HMS_ADIF,    // time_format
	{}                    // items
};
const qsl_data FILE_QSL_DATA =
{
	qsl_data::POINT,      // unit
	900,                  // width
	600,                  // height
	1,                    // rows
	1,                    // columns
	0,                    // col_width
	0,                    // row_height
	0,                    // row_top
	0,                    // col_left
	1,                    // max_qsos
	qsl_data::FMT_Y4MD_ADIF,   // date_format
	qsl_data::FMT_HMS_ADIF,    // time_format
	{}                    // items
};
const std::map<qsl_data::qsl_type, qsl_data> DEFAULT_QSL_DATA = {
	{ qsl_data::LABEL, LABEL_QSL_DATA },
	{ qsl_data::FILE, FILE_QSL_DATA }
};

static void to_json(json& j, const qsl_data::style_def& s) {
	j = json{
		{ "Font", s.font },
		{ "Size", s.size },
		{ "Colour", (unsigned)s.colour }
	};
}

// Convert qsl_data::item_def to JSON object
static void to_json(json& j, const qsl_data::item_def& s) {
	j["Type"] = s.type;
	switch (s.type) {
	case qsl_data::FIELD: {
		j["Name"] = s.field.field;
		j["Label"] = s.field.label;
		j["Label style"] = s.field.l_style;
		j["Text style"] = s.field.t_style;
		j["X"] = s.field.dx;
		j["Y"] = s.field.dy;
		j["Vertical alignment"] = s.field.vertical;
		j["Boxed"] = s.field.box;
		j["Multiple QSOs"] = s.field.multi_qso;
		j["Display if empty"] = s.field.display_empty;
		break;
	}
	case qsl_data::TEXT: {
		j["Text"] = s.text.text;
		j["Text style"] = s.text.t_style;
		j["X"] = s.text.dx;
		j["Y"] = s.text.dy;
		j["Vertical alignment"] = s.text.vertical;
		break;
	}
	case qsl_data::IMAGE: {
		j["Filename"] = s.image.filename;
		j["X"] = s.image.dx;
		j["Y"] = s.image.dy;
		break;
	}
	default: break;
	}
}

// Convert qsl_data to JSON object
static void to_json(json& j, const qsl_data& s) {
	j = json{
		{ "Unit", s.unit },
		{ "Width", s.width },
		{ "Height", s.height },
		{ "Rows", s.rows},
		{ "Columns", s.columns },
		{ "Column spacing", s.col_width },
		{ "Row spacing", s.row_height },
		{ "Row top", s.row_top },
		{ "Column left", s.col_left},
		{ "QSOs per card", s.max_qsos},
		{ "Date format", s.f_date},
		{ "Time format", s.f_time}
	};
	json jitems;
	for (auto& it : s.items) {
		jitems.push_back(*it);
	}
	j["Items"] = jitems;
}

// Convert qsl_call_data to JSON object
static void to_json(json& j, const qsl_call_data& s) {
	uchar offset = hash8(qsl_dataset::server_name().c_str());
	if (s.used) {
		j["In use"] = true;
		j["Last downloaded"] = s.last_download;
		j["Key"] = string_to_hex(xor_crypt(s.key, seed_, offset));
	}
	else {
		j["In use"] = false;
	}
}

// Convert server_data_t to JSON object
static void to_json(json& j, const server_data_t& s) {
	uchar offset = hash8(qsl_dataset::server_name().c_str());
	// Common items
	j["User"] = s.user;
	j["Password"] = string_to_hex(xor_crypt(s.password, seed_, offset));
	if (qsl_dataset::server_name() != "eMail") {
		j["Enable"] = s.enabled;
		j["Upload per QSO"] = s.upload_per_qso;
		j["Last downloaded"] = s.last_downloaded;
	}
	else {
		j["eMail server"] = s.mail_server;
		j["CC address"] = s.cc_address;
	}
	if (qsl_dataset::server_name() == "eQSL") {
		j["Download confirmed"] = s.download_confirmed;
		j["QSO message"] = s.qso_message;
		j["SWL message"] = s.swl_message;
		json jlog;
		for (auto& it : s.call_data) {
			jlog[it.first] = *it.second;
		}
		j["Logbooks"] = jlog;
	}
	if (qsl_dataset::server_name() == "LotW") {
		j["Export file"] = s.export_file;
	}
	if (qsl_dataset::server_name() == "QRZ") {
		j["Use XML database"] = s.use_xml;
		j["Use API"] = s.use_api;
		json jlog;
		for (auto& it : s.call_data) {
			jlog[it.first] = *it.second;
		}
		j["Logbooks"] = jlog;
	}
}

// Convert JSON object to qsl_data::style_def
static void from_json(const json& j, qsl_data::style_def& s) {
	j.at("Font").get_to(s.font);
	j.at("Size").get_to(s.size);
	j.at("Colour").get_to(s.colour);
}

// Convert JSON object to qsl_data::item_def
static void from_json(const json& j, qsl_data::item_def& s) {
	j.at("Type").get_to(s.type);
	switch (s.type) {
	case qsl_data::FIELD: {
		j.at("Name").get_to(s.field.field);
		j.at("Label").get_to(s.field.label);
		j.at("Label style").get_to(s.field.l_style);
		j.at("Text style").get_to(s.field.t_style);
		j.at("X").get_to(s.field.dx);
		j.at("Y").get_to(s.field.dy);
		j.at("Vertical alignment").get_to(s.field.vertical);
		j.at("Boxed").get_to(s.field.box);
		j.at("Multiple QSOs").get_to(s.field.multi_qso);
		j.at("Display if empty").get_to(s.field.display_empty);
		break;
	}
	case qsl_data::TEXT: {
		j.at("Text").get_to(s.text.text);
		j.at("Text style").get_to(s.text.t_style);
		j.at("X").get_to(s.text.dx);
		j.at("Y").get_to(s.text.dy);
		j.at("Vertical alignment").get_to(s.text.vertical);
		break;
	}
	case qsl_data::IMAGE: {
		j.at("Filename").get_to(s.image.filename);
		j.at("X").get_to(s.image.dx);
		j.at("Y").get_to(s.image.dy);
		break;
	}
	default: break;
	}
}

static void from_json(const json& j, qsl_data& s) {
	j.at("Unit").get_to(s.unit);
	j.at("Width").get_to(s.width);
	j.at("Height").get_to(s.height);
	j.at("Rows").get_to(s.rows);
	j.at("Columns").get_to(s.columns);
	j.at("Column spacing").get_to(s.col_width);
	j.at("Row spacing").get_to(s.row_height);
	j.at("Row top").get_to(s.row_top);
	j.at("Column left").get_to(s.col_left);
	j.at("QSOs per card").get_to(s.max_qsos);
	j.at("Date format").get_to(s.f_date);
	j.at("Time format").get_to(s.f_time);
	if (j.at("Items").is_array()) {
		auto jitems = j.at("Items").get<std::vector<json>>();
		for (auto& it : jitems) {
			qsl_data::item_def* item = new qsl_data::item_def(it);
			s.items.push_back(item);
		}
	}
}

static void from_json(const json& j, qsl_call_data& s) {
	uchar offset = hash8(qsl_dataset::server_name().c_str());
	j.at("In use").get_to(s.used);
	if (s.used) {
		std::string key;
		j.at("Key").get_to(key);
		s.key = xor_crypt(hex_to_string(key), seed_, offset);
		j.at("Last downloaded").get_to(s.last_download);
	}
}

static void from_json(const json& j, server_data_t& s) {
	std::string server = qsl_dataset::server_name();
	uchar offset = hash8(server.c_str());
	std::string password;
	j.at("User").get_to(s.user);
	j.at("Password").get_to(password);
	s.password = xor_crypt(hex_to_string(password), seed_, offset);
	if (server == "eMail") {
		j.at("eMail server").get_to(s.mail_server);
		j.at("CC address").get_to(s.cc_address);
	}
	else {
		j.at("Enable").get_to(s.enabled);
		j.at("Last downloaded").get_to(s.last_downloaded);
		j.at("Upload per QSO").get_to(s.upload_per_qso);
	}
	if (server == "eQSL" || server == "QRZ") {
		auto jlog = j.at("Logbooks").get<std::map<std::string, json>>();
		for (auto it : jlog) {
			qsl_call_data* cd = new qsl_call_data(it.second);
			s.call_data[it.first] = cd;
		}
	}
	if (server == "eQSL") {
		j.at("Download confirmed").get_to(s.download_confirmed);
		j.at("QSO message").get_to(s.qso_message);
		j.at("SWL message").get_to(s.swl_message);
	}
	if (server == "LotW") {
		j.at("Export file").get_to(s.export_file);
	}
	if (server == "QRZ") {
		j.at("Use XML database").get_to(s.use_xml);
		j.at("Use API").get_to(s.use_api);
	}
}

qsl_dataset::qsl_dataset() {
	load_failed_ = false;
	load_data();
}

qsl_dataset::~qsl_dataset() {
	save_data();
}

// Return the QSL design associated with the callsign and QSL type
qsl_data* qsl_dataset::get_card(std::string callsign, qsl_data::qsl_type type, bool create) {
	if (data_.find(type) != data_.end()) {
		auto call_map = data_.at(type);
		if (call_map->find(callsign) != call_map->end()) {
			return call_map->at(callsign);
		}
	}
	qsl_data* data = new qsl_data(DEFAULT_QSL_DATA.at(type));
	if (create) {
		if (data_.find(type) == data_.end()) {
			std::map<std::string, qsl_data*>* call_map = new std::map<std::string, qsl_data* >;
			data_[type] = call_map;
		}
		(*data_[type])[callsign] = data;
		return data;
	}
	else {
		return data;
	}
} 

// Return the server data associated with the extract type
server_data_t* qsl_dataset::get_server_data(std::string server) {
	if (server_data_.find(server) == server_data_.end()) {
		char msg[128];
		snprintf(msg, sizeof(msg), "QSL: Unsupported server %s", server.c_str());
		status_->misc_status(ST_WARNING, msg);
		new_server(server);
	} 
	return server_data_.at(server);
}

qsl_call_data* qsl_dataset::get_qrz_api(std::string callsign) {
	server_data_t* sd = get_server_data("QRZ") ;
	if (sd) {
		if (sd->call_data.find(callsign) != sd->call_data.end()) {
			return sd->call_data.at(callsign);
		} 
	}
	char msg[128];
	snprintf(msg, sizeof(msg), "QSL: Cannot get QRZ.com logbook credentials for %s", callsign.c_str());
	status_->misc_status(ST_ERROR, msg);
	return nullptr;
}

// Read card designs
void qsl_dataset::load_data() {
	data_.clear();
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	if (!load_json(settings)) {
		status_->misc_status(ST_ERROR, "QSL: No QSl data loaded");
	}
}

std::string qsl_dataset::json_file(Fl_Preferences& settings) {
	char* temp;
	Fl_Preferences data_settings(settings, "Datapath");
	data_settings.get("QSLs", temp, "");
	qsl_path_ = temp;
	if (qsl_path_.length() == 0) {
		status_->misc_status(ST_WARNING, "QSL: No directory in settings - please search");
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select QSL Directory");
		if (chooser->show() == 0) {
			qsl_path_ = chooser->filename();
		}
		delete chooser;
		data_settings.set("QSLs", qsl_path_.c_str());
	}
	std::string filename = qsl_path_ + "/config.json";
	free(temp);
	return filename;
}

bool qsl_dataset::load_json(Fl_Preferences& settings) {
	ifstream is;
	string filename = json_file(settings);
	char msg[128];
	status_->misc_status(ST_NOTE, ("QSL: Loading QSL data"));
	is.open(filename, std::ios_base::in);
	if (is.good()) {
		try {
			json jall;
			is >> jall;
			auto qsls = jall.at("QSL Designs").get<std::vector<json>>();
			for (auto it_type : qsls) {
				qsl_data::qsl_type type;
				auto designs = new std::map<std::string, qsl_data*>;
				it_type.at("Design type").get_to(type);
				auto jdesigns = it_type.at("Designs").
					get<std::map<std::string, json>>();
				for (auto it_x : jdesigns) {
					qsl_data* qd = new qsl_data(it_x.second);
					(*designs)[it_x.first] = qd;
				}
				data_[type] = designs;
			}
			auto svrs = jall.at("Servers").get < std::map<std::string, json>>();
			svrs.at("Seed").get_to(seed_);
			for (auto it : svrs) {
				if (it.first != "Seed") {
					// Server_name is used by JSON conversion routines
					server_name_ = it.first;
					server_data_t* sd = new server_data_t(it.second);
					server_data_[it.first] = sd;
				}
			}
			std::snprintf(msg, sizeof(msg), "QSL: File %s loaded OK", filename.c_str());
			status_->misc_status(ST_OK, msg);
			return true;
		}
		catch (const json::exception& e) {
			std::snprintf(msg, sizeof(msg), "QSL: Reading JSON failed %d (%s)\n",
				e.id, e.what());
			status_->misc_status(ST_ERROR, msg);
			is.close();
			return false;
		}
	}
	else {
		status_->misc_status(ST_ERROR, "QSL: Load QSL data failed");
		return false;
	}
}

// Store card designs
void qsl_dataset::save_data() {
	if (!load_failed_) {
		Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
		settings.delete_group("QSL Design");
		save_json(settings);
	}
}

void qsl_dataset::save_json(Fl_Preferences& settings) {
	char msg[128];
	std::string filename = json_file(settings);
	status_->misc_status(ST_NOTE, "QSL: Saving QSL designs and servers");
	std::ofstream os;
	os.open(filename, std::ios_base::out);
	if (os.good()) {
		json jall;
		json jqsls;
		for (auto& it_type : data_) {
			json jt;
			json jcalls;
			for (auto& it_call : *(it_type.second)) {
				jcalls[it_call.first] = *it_call.second;
			}
			jt["Design type"] = it_type.first;
			jt["Designs"] = jcalls;
			jqsls.push_back(jt);
		}
		jall["QSL Designs"] = jqsls;
		json jsvrs;
		for (auto& it_svr : server_data_) {
			server_name_ = it_svr.first;
			jsvrs[it_svr.first] = *it_svr.second;
		}
		jsvrs["Seed"] = seed_;
		jall["Servers"] = jsvrs;
		os << std::setw(2) << jall;
		if (os.fail()) {
			status_->misc_status(ST_ERROR, "QSL: Failed to save data");
		}
		else {
			std::snprintf(msg, sizeof(msg), "QSL: File %s saved OK", filename.c_str());
			status_->misc_status(ST_OK, msg);
		}
		os.close();
	}
	else {
		status_->misc_status(ST_ERROR, "QSL: Failed to open file for saving");
	}
}

// Return serevr name
std::string qsl_dataset::server_name() { return server_name_; }

// Get path to QSL data
std::string qsl_dataset::get_path() {
	return qsl_path_;
}

// Create new server data
bool qsl_dataset::new_server(std::string server) {
	char msg[128];
	if (server_data_.find(server) != server_data_.end()) {
		snprintf(msg, sizeof(msg), "QSL: Already have server data for %s", server.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	} 
	server_data_t* data = new server_data_t;
	server_data_[server] = data;
	return true;
}


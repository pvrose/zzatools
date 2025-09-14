#include "qsl_dataset.h"
#include "qsl_reader.h"
#include "qsl_writer.h"
#include "extract_data.h"
#include "qrz_handler.h"
#include "status.h"
#include "utils.h"

#include <string>
#include <vector>
#include <fstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>

extern status* status_;
extern std::string VENDOR;
extern std::string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;

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
	if (!load_xml(settings)) {
		load_failed_ = true;
		status_->misc_status(ST_ERROR, "QSL: No QSl data loaded");
		//load_prefs(settings);
	}

}

std::string qsl_dataset::xml_file(Fl_Preferences& settings) {
	char* temp;
	Fl_Preferences data_settings(settings, "Datapath");
	data_settings.get("QSLs", temp,"");
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
	std::string filename = qsl_path_ + "/config.xml";
	free(temp);
	return filename;
}

bool qsl_dataset::load_xml(Fl_Preferences& settings) {
	ifstream is;
	is.open(xml_file(settings), std::ios_base::in);
	if (is.good()) {
		qsl_reader* reader = new qsl_reader();
		if (reader->load_data(&data_, &server_data_, is)) {
			status_->misc_status(ST_OK, "QSL: XML loaded OK");
			return true;
		}
	}
	// else
	status_->misc_status(ST_WARNING, "QSL: XML data failed to load - defaulting to prefernces");
	return false;
}

// Store card designs
void qsl_dataset::save_data() {
	if (!load_failed_) {
		Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
		settings.delete_group("QSL Design");
		save_xml(settings);
	}
}

void qsl_dataset::save_xml(Fl_Preferences& settings) {
	std::string filename = xml_file(settings);
	std::ofstream os;
	os.open(filename, std::ios_base::out);
	if (os.good()) {
		qsl_writer* writer = new qsl_writer();
		if (!writer->store_data(&data_, &server_data_, os)) {
			status_->misc_status(ST_OK, "RIG DATA: Saved XML OK");
		}
	}
}

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


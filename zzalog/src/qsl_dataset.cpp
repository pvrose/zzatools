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

extern status* status_;
extern string VENDOR;
extern string PROGRAM_ID;

qsl_dataset::qsl_dataset() {
	load_data();
}

qsl_dataset::~qsl_dataset() {
	save_data();
}

// Return the QSL design associated with the callsign and QSL type
qsl_data* qsl_dataset::get_card(string callsign, qsl_data::qsl_type type, bool create) {
	if (data_.find(type) != data_.end()) {
		auto call_map = data_.at(type);
		if (call_map->find(callsign) != call_map->end()) {
			return call_map->at(callsign);
		}
	}
	qsl_data* data = new qsl_data;
	if (create) {
		if (data_.find(type) == data_.end()) {
			map<string, qsl_data*>* call_map = new map<string, qsl_data* >;
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
server_data_t* qsl_dataset::get_server_data(string server) {
	if (server_data_.find(server) == server_data_.end()) {
		char msg[128];
		snprintf(msg, sizeof(msg), "QSL: Unsupported server %s", server.c_str());
		status_->misc_status(ST_WARNING, msg);
		new_server(server);
	} 
	return server_data_.at(server);
}

qrz_api_data* qsl_dataset::get_qrz_api(string callsign) {
	server_data_t* sd = get_server_data("QRZ") ;
	if (sd) {
		if (sd->api_data.find(callsign) != sd->api_data.end()) {
			return sd->api_data.at(callsign);
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
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	if (!load_xml(settings)) {
		status_->misc_status(ST_ERROR, "QSL DATA; No QSl data loaded");
		//load_prefs(settings);
	}

}

void qsl_dataset::load_items(qsl_data* data) {
	ifstream ip;
	ip.open(data->filename.c_str(), fstream::in);
	string line;
	vector<string> words;
	// For each line in the file.. 
	while (ip.good()) {
		getline(ip, line);
		// Split lines on the 'tab' character
		split_line(line, words, '\t');
		if (words.size() > 1 && (qsl_data::item_type)stoi(words[0]) != qsl_data::NONE) {
			// The line probably contains good data
			qsl_data::item_def* item = new qsl_data::item_def;
			item->type = (qsl_data::item_type)stoi(words[0]);
			switch (item->type) {
			case qsl_data::FIELD: {
				// Line contains a single field item data
				if (words.size() >= 15) {
					item->field.field = words[1];
					item->field.label = words[2];
					item->field.l_style.font = (Fl_Font)stoi(words[3]);
					item->field.l_style.size = (Fl_Fontsize)stoi(words[4]);
					item->field.l_style.colour = (Fl_Color)stoi(words[5]);
					item->field.t_style.font = (Fl_Font)stoi(words[6]);
					item->field.t_style.size = (Fl_Fontsize)stoi(words[7]);
					item->field.t_style.colour = (Fl_Color)stoi(words[8]);
					item->field.dx = stoi(words[9]);
					item->field.dy = stoi(words[10]);
					item->field.vertical = (bool)stoi(words[11]);
					item->field.multi_qso = (bool)stoi(words[12]);
					item->field.box = (bool)stoi(words[13]);
					item->field.display_empty = (bool)stoi(words[14]);
				}
				break;
			}
			case qsl_data::TEXT: {
				// A line contains a single text item data
				if (words.size() >= 7) {
					item->text.text = words[1];
					item->text.t_style.font = (Fl_Font)stoi(words[2]);
					item->text.t_style.size = (Fl_Fontsize)stoi(words[3]);
					item->text.t_style.colour = (Fl_Color)stoi(words[4]);
					item->text.dx = stoi(words[5]);
					item->text.dy = stoi(words[6]);
					if (words.size() >= 8) {
						item->text.vertical = (bool)stoi(words[7]);
					}
				}
				break;
			}
			case qsl_data::IMAGE: {
				// A line contains a single image item data (except actual image data)
				if (words.size() >= 4) {
					if (directory(words[1]) == "")
						item->image.filename = directory(data->filename) + '/' + words[1];
					else
						item->image.filename = words[1];
					item->image.dx = stoi(words[2]);
					item->image.dy = stoi(words[3]);
				}
				break;
			}
			default:
				break;
			}
			// Add the item's data to the data structure
			data->items.push_back(item);
		}
	}
	ip.close();
	char msg[128];
	snprintf(msg, sizeof(msg), "QSL: %zd items read from %s", data->items.size(), data->filename.c_str());
	status_->misc_status(ST_OK, msg);
}

string qsl_dataset::xml_file(Fl_Preferences& settings) {
	char* temp;
	Fl_Preferences data_settings(settings, "Datapath");
	data_settings.get("QSLs", temp,"");
	qsl_path_ = temp;
	string filename = qsl_path_ + "/config.xml";
	free(temp);
	return filename;
}

bool qsl_dataset::load_xml(Fl_Preferences& settings) {
	ifstream is;
	is.open(xml_file(settings), ios_base::in);
	if (is.good()) {
		qsl_reader* reader = new qsl_reader();
		if (reader->load_data(&data_, &server_data_, is)) {
			status_->misc_status(ST_OK, "QSL DATA: XML loaded OK");
			return true;
		}
	}
	// else
	status_->misc_status(ST_WARNING, "QSL DATA: XML data faile to load - defaulting to prefernces");
	return false;
}

// Store card designs
void qsl_dataset::save_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	settings.delete_group("QSL Design");
	save_xml(settings);
}

void qsl_dataset::save_xml(Fl_Preferences& settings) {
	string filename = xml_file(settings);
	ofstream os;
	os.open(filename, ios_base::out);
	if (os.good()) {
		qsl_writer* writer = new qsl_writer();
		if (!writer->store_data(&data_, &server_data_, os)) {
			status_->misc_status(ST_OK, "RIG DATA: Saved XML OK");
		}
	}
}

// Get path to QSL data
string qsl_dataset::get_path() {
	return qsl_path_;
}

// Create new server data
bool qsl_dataset::new_server(string server) {
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


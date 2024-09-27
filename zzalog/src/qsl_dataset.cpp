#include "qsl_dataset.h"
#include "status.h"

#include <string>
#include <vector>
#include <fstream>

#include <FL/Fl_Preferences.H>

extern Fl_Preferences* settings_;
extern status* status_;

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
		(*data_[type])[callsign] = data;
		dirty_[data] = true;
		return data;
	}
	else {
		return data;
	}
} 

// Associatedtext - used in load_data nd save_data
string qsl_types[qsl_data::MAX_TYPE] = { "Label", "PDF" };


// Read card designs
void qsl_dataset::load_data() {
	data_.clear();
	Fl_Preferences qsl_settings(settings_, "QSL Design");

	for (int ix = 0; ix < (int)qsl_data::MAX_TYPE; ix++) {
		map<string, qsl_data*>* type_data = new map<string, qsl_data*>;
		data_[(qsl_data::qsl_type)ix] = type_data;
		Fl_Preferences type_settings(qsl_settings, qsl_types[ix].c_str());
		// Now fetch each callsign that is in the prefs file
		int num_calls = type_settings.groups();
		for (int iy = 0; iy < num_calls; iy++) {
			string call = type_settings.group(iy);
			Fl_Preferences call_settings(type_settings, call.c_str());
			qsl_data* data = new qsl_data;
			(*type_data)[call] = data;
			// Temporary values to convert char* and int to string and enums
			char* temp;
			int itemp;
			// Get the card label data_ for this callsign
			call_settings.get("Width", data->width, 0);
			call_settings.get("Height", data->height, 0);
			call_settings.get("Unit", itemp, (int)qsl_data::MILLIMETER);
			data->unit = (qsl_data::dim_unit)itemp;
			call_settings.get("Number Rows", data->rows, 4);
			call_settings.get("Number Columns", data->columns, 2);
			call_settings.get("Column Width", data->col_width, 101.6);
			call_settings.get("Row Height", data->row_height, 67.7);
			call_settings.get("First Row", data->row_top, 12.9);
			call_settings.get("First Column", data->col_left, 4.6);
			call_settings.get("Max QSOs per Card", data->max_qsos, 1);
			call_settings.get("Date Format", itemp, qsl_data::FMT_Y4MD_ADIF);
			data->f_date = (qsl_data::date_format)itemp;
			call_settings.get("Time Format", itemp, qsl_data::FMT_HMS_ADIF);
			data->f_time = (qsl_data::time_format)itemp;
			call_settings.get("Card Design", temp, "");
			printf("Read data from file %d bytes, value ='%s'\n", strlen(temp), temp);
			data->filename = temp;
			free(temp);
			// Check it's a TSV file
			size_t pos = data->filename.find_last_of('.');
			if (pos == string::npos || data->filename.substr(pos) != ".tsv") {
				// Not a TSV file (or no filename)
				char msg[256];
				snprintf(msg, sizeof(msg), "QSL: Invalid filename %s - setting to no value", data->filename.c_str());
				status_->misc_status(ST_WARNING, msg);
				data->filename = "";
			}
			// Minimum data_ required
			if (data->width == 0 || data->height == 0 || data->filename.length() == 0) {
				// We have either width or height not defined - so load the edfault data_
				char msg[256];
				snprintf(msg, sizeof(msg), "QSL: Incorrect info W=%g, H=%g, File=%s", data->width, data->height, data->filename.c_str());
				status_->misc_status(ST_ERROR, msg);
				data->items.clear();
				// Default width and height if zero
				if (data->width == 0) data->width = data->col_width;
				if (data->height == 0) data->height = data->row_height;
			}
			if (data->filename.length()) {
				// Data looks good so far
				char msg[100];
				snprintf(msg, sizeof(msg), "QSL: Reading card image data %s %s from %s",
					call.c_str(),
					qsl_types[ix].c_str(),
					data->filename.c_str());
				status_->misc_status(ST_LOG, msg);
				load_items(data);
			}
			dirty_[data] = false;
		}
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

// Store card designs
void qsl_dataset::save_data() {
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	qsl_settings.clear();
	for (auto it = data_.begin(); it != data_.end(); it++) {
		qsl_data::qsl_type type = it->first;
		Fl_Preferences type_settings(qsl_settings, qsl_types[(int)type].c_str());
		for (auto ita = it->second->begin(); ita != it->second->end(); ita++) {
			Fl_Preferences call_settings(type_settings, ita->first.c_str());
			qsl_data* data = ita->second;
			// SAve the card label parameters
			call_settings.set("Width", data->width);
			call_settings.set("Height", data->height);
			call_settings.set("Unit", (int)data->unit);
			call_settings.set("Number Rows", data->rows);
			call_settings.set("Number Columns", data->columns);
			call_settings.set("Column Width", data->col_width);
			call_settings.set("Row Height", data->row_height);
			call_settings.set("First Row", data->row_top);
			call_settings.set("First Column", data->col_left);
			call_settings.set("Max QSOs per Card", data->max_qsos);
			call_settings.set("Date Format", (int)data->f_date);
			call_settings.set("Time Format", (int)data->f_time);
			call_settings.set("Card Design", data->filename.c_str());
			// Writing the file with the drawing data
			if (dirty_.at(data)) {
				char msg[100];
				snprintf(msg, sizeof(msg), "QSL: Writing card image data for %s %s to %s", 
					ita->first.c_str(),
					qsl_types[(int)type].c_str(),
					data->filename.c_str());
				status_->misc_status(ST_LOG, msg);
				ofstream op;
				op.open(data->filename.c_str(), fstream::out);
				string line;
				int pos = 0;
				// For all drawing items...
				for (int ix = 0; ix < data->items.size() && op.good(); ix++) {
					qsl_data::item_def* item = data->items[ix];
					// Ignore any item marked with type NONE
					if (item->type != qsl_data::NONE) {
						// Write type the a tab
						op << (int)item->type << '\t';
						switch (item->type) {
						case qsl_data::FIELD: {
							// Write field item data separated by tabs
							op << item->field.field << '\t' <<
								item->field.label << '\t' <<
								(int)item->field.l_style.font << '\t' <<
								(int)item->field.l_style.size << '\t' <<
								(int)item->field.l_style.colour << '\t' <<
								(int)item->field.t_style.font << '\t' <<
								(int)item->field.t_style.size << '\t' <<
								(int)item->field.t_style.colour << '\t' <<
								item->field.dx << '\t' <<
								item->field.dy << '\t' <<
								(int)item->field.vertical << '\t' <<
								(int)item->field.multi_qso << '\t' <<
								(int)item->field.box << '\t' <<
								(int)item->field.display_empty << endl;
							break;
						}
						case qsl_data::TEXT: {
							// Write text item data separated by tabs
							op << item->text.text << '\t' <<
								(int)item->text.t_style.font << '\t' <<
								(int)item->text.t_style.size << '\t' <<
								(int)item->text.t_style.colour << '\t' <<
								item->text.dx << '\t' <<
								item->text.dy << endl;
							break;
						}
						case qsl_data::IMAGE: {
							// Write image item data separted by tabs
							op << terminal(item->image.filename) << '\t' <<
								item->image.dx << '\t' <<
								item->image.dy << endl;
							break;
						}
						}
					}
				}
				op.close();
				snprintf(msg, sizeof(msg), "QSL: %zd items written to %s", data->items.size(), data->filename.c_str());
				status_->misc_status(ST_OK, msg);
				dirty_[data] = false;
			}
		}
	}
}

void qsl_dataset::dirty(qsl_data* data) {
	if (dirty_.find(data) == dirty_.end()) {
		status_->misc_status(ST_FATAL, "QSL: Attempting to mark dirty a non-existant card design");
	}
	else {
		dirty_[data] = true;
	}
}

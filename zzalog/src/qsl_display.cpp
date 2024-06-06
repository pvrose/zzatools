#include "qsl_display.h"
#include "qso_manager.h"
#include "status.h"

#include <set>
#include <string>

#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Preferences.H>

using namespace std;

extern qso_manager* qso_manager_;
extern status* status_;
extern Fl_Preferences* settings_;
// Dynamically drawn QSL card label

qsl_display::qsl_display(int X, int Y, int W, int H, const char* L) : Fl_Group(X, Y, W, H, L) {
    color(FL_WHITE);
	box(FL_FLAT_BOX);
    resizable(nullptr);
    qsos_ = nullptr;
    num_records_ = 0;
	callsign_ = "";
    data_.clear();
    editable_ = false;

}
qsl_display::~qsl_display() {};

void qsl_display::value(string callsign, record** qsos, int num_records) {
    qsos_ = qsos;
    num_records_ = num_records;
    callsign_ = callsign;
    load_data();
    create_form();

    Fl_Group::size(to_points(width_), to_points(height_));
}

void qsl_display::create_form() {
    // Remove all existing widgets
    clear();
	// Because this gets called randomly need to be explicit in current_ handling
	Fl_Group* save = Fl_Group::current();
    begin();

    for (auto it = data_.begin(); it != data_.end(); it++) {
        item_def& item = *(*it);
		Fl_Multiline_Output* op = new Fl_Multiline_Output(x() + item.dx, y() + item.dy, item.dw, item.dh, item.label.c_str()); 
		op->align(item.align);
		op->box(item.box);
		op->labelfont(item.l_style.font);
		op->labelsize(item.l_style.size);
		op->labelcolor(item.l_style.colour);
		op->textfont(item.t_style.font);
		op->textsize(item.t_style.size);
		op->textcolor(item.t_style.colour);
		op->color(FL_WHITE);
		string text = "";
		if (num_records_ == 0) {
			text = item.field;
			if (item.multi_qso) {
				text += "\nditto.";
			}
		} else if (item.multi_qso) {
			for (int i = 0; i < num_records_; i++) {
				if (i > 0) text += '\n';
				text += qsos_[i]->item(item.field);
			}
		} else {
			set<string> values;
			// Uniquify values
			for (int i = 0; i < num_records_; i++) {
				values.insert(qsos_[i]->item(item.field));
			}
			for (auto it = values.begin(); it != values.end(); it++) {
				if (it != values.begin()) text += ' ';
				text += *it;
			}
		}
		if (item.display_empty) {
			op->show();
			op->activate();
		} else if (num_records_ == 0) {
			op->show();
			op->deactivate();
		} else if (text == "") {
			op->hide();
		} else {
			op->show();
			op->activate();
		}
		op->value(text.c_str());
	}
    // Restore original current Fl_Group
	Fl_Group::current(save);
    show();
}

// Set/get the editable flag
bool qsl_display::editable() { return editable_; }
void qsl_display::editable(bool enable) { editable_ = enable; }

// Intercept drag and drop events to move widgets:
// Click - select the widget under the mouse
// On left edge - adjust X and W
// On top edge - adjust Y and H
// On right edge - adjust W
// On borrom edge - adjust H
// Else where - adjust X and Y
int qsl_display::handle(int event) {
    // TODO: handle drag and drop events to move widgets
    return Fl_Group::handle(event);
}

// Convert value from specified unit to points
int qsl_display::to_points(float value) {
	switch (unit_) {
	case INCH:
		// 72 points per inch
		return (int)(value * IN_TO_POINT);
	case MILLIMETER:
		// 72/25.4 points per mm
		return (int)(value * MM_TO_POINT);
	case POINT:
		return (int)value;
	default:
		return 0;
	}
}

// Load data
void qsl_display::load_data() {
    	// Get width and height of label
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	char* temp;
	Fl_Preferences call_settings(qsl_settings, callsign_.c_str());
	call_settings.get("Width", width_, 0);
	call_settings.get("Height", height_, 0);
	call_settings.get("Unit", (int&)unit_, (int)MILLIMETER);
	call_settings.get("Card Design", temp, "");
	filename_ = temp;
	free(temp);
	// Check it's a TSV file
	size_t pos = filename_.find_last_of('.');
	if (pos == string::npos || filename_.substr(pos) != ".tsv") {
		char msg[256];
		snprintf(msg, sizeof(msg), "QSL: Invalid filename %s - setting to no value", filename_.c_str());
		status_->misc_status(ST_WARNING, msg);
		filename_ = "";
	}
	if (width_ == 0 || height_ == 0 || filename_.length() == 0) {
		// We have either width or height not defined - so load the edfault data
		char msg[256];
		snprintf(msg, sizeof(msg), "QSL: Incorrect info W=%g, H=%g, File=%s", width_, height_, filename_.c_str());
		status_->misc_status(ST_ERROR, msg);
		data_.clear();
	} else {
		char msg[100];
		snprintf(msg, sizeof(msg), "QSL: Reading card image data from %s", filename_.c_str());
		data_.clear();
		status_->misc_status(ST_LOG, msg);
		ifstream ip;
		ip.open(filename_.c_str(), fstream::in);
		string line;
		vector<string> words;
		while(ip.good()) {
			getline(ip, line);
			split_line(line, words, ' ');
			if (words.size() == 16) {	
				qsl_display::item_def* item = new qsl_display::item_def;
				item->field = words[0];
				item->label = words[1];
				item->l_style.font = (Fl_Font)stoi(words[2]);
				item->l_style.size = (Fl_Fontsize)stoi(words[3]);
				item->l_style.colour = (Fl_Color)stoi(words[4]);
				item->t_style.font = (Fl_Font)stoi(words[5]);
				item->t_style.size = (Fl_Fontsize)stoi(words[6]);
				item->t_style.colour = (Fl_Color)stoi(words[7]);
				item->dx = stoi(words[8]);
				item->dy = stoi(words[9]);
				item->dw = stoi(words[10]);
				item->dh = stoi(words[11]);
				item->align = (Fl_Align)stoi(words[12]);
				item->multi_qso = (bool)stoi(words[13]);
				item->box = (Fl_Boxtype)stoi(words[14]);
				item->display_empty = (bool)stoi(words[15]);
				data_.push_back(item);
			}
		}
		ip.close();
		snprintf(msg, sizeof(msg), "QSL: %d items read from %s", data_.size(), filename_.c_str());
		status_->misc_status(ST_OK, msg);
	}

}

void qsl_display::save_data() {
    // Get width and height of label
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	Fl_Preferences call_settings(qsl_settings, callsign_.c_str());
	call_settings.set("Width", width_);
	call_settings.set("Height", height_);
	call_settings.set("Unit", (int)unit_);
	call_settings.set("Filename", filename_.c_str());
   	char msg[100];
	snprintf(msg, sizeof(msg), "QSL: Writing card image data to %s", filename_.c_str());
	status_->misc_status(ST_LOG, msg);
	ofstream op;
	op.open(filename_.c_str(), fstream::out);
	string line;
	int pos = 0;
	for (int ix = 0; ix < data_.size() && op.good(); ix++) {
		qsl_display::item_def* item = data_[ix];
		op << item->field << ' ' << 
			item->label << ' ' << 
			(int)item->l_style.font << ' ' << 
			(int)item->l_style.size << ' ' <<
			(int)item->l_style.colour << ' ' <<
			(int)item->t_style.font << ' ' << 
			(int)item->t_style.size <<	' ' << 
			(int)item->t_style.colour << ' ' << 
			item->dx << ' ' << 
			item->dy << ' ' << 
			item->dw << ' ' << 
			item->dh << ' ' << 
			(int)item->align << ' ' << 
			(int)item->multi_qso << ' ' << 
			(int)item->box << ' ' <<
			(int)item->display_empty << endl;
	}
	op.close();
	snprintf(msg, sizeof(msg), "QSL: %d items written to %s", data_.size(), filename_.c_str());
	status_->misc_status(ST_OK, msg);
}

void qsl_display::size(float w, float h, dim_unit unit) {
    width_ = w;
    height_ = h;
    unit_ = unit;

    Fl_Group::size(to_points(width_), to_points(height_));
    redraw();
}

vector<qsl_display::item_def*>* qsl_display::data() {
	return &data_;
}
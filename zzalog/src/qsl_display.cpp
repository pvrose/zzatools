#include "qsl_display.h"
#include "qso_manager.h"
#include "status.h"

#include <set>
#include <string>

#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>

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
    redraw();

    Fl_Group::size(to_points(width_), to_points(height_));
}

void qsl_display::draw() {
	// Colour the whole display
	fl_rectf(x(), y(), w(), h(), FL_WHITE);
	// Save the current font/size

    for (auto it = data_.begin(); it != data_.end(); it++) {
        item_def& item = *(*it);
		switch(item.type) {
			case FIELD: {
				draw_field(item.field);
				break;
			}
			case TEXT: {
				draw_text(item.text);
				break;
			}
			case IMAGE: {
				draw_image(item.image);
				break;
			}
		}
	}
}

void qsl_display::draw_field(field_def& field) {
	Fl_Font savef = fl_font();
	Fl_Fontsize savez = fl_size();
	// Evaluate the text
	string text = "";
	if (num_records_ == 0) {
		text = field.field;
		if (field.multi_qso) {
			text += "\nditto.";
		}
	} else if (field.multi_qso) {
		for (int i = 0; i < num_records_; i++) {
			if (i > 0) text += '\n';
			text += qsos_[i]->item(field.field);
		}
	} else {
		set<string> values;
		// Uniquify values
		for (int i = 0; i < num_records_; i++) {
			values.insert(qsos_[i]->item(field.field));
		}
		for (auto it = values.begin(); it != values.end(); it++) {
			if (it != values.begin()) text += ' ';
			text += *it;
		}
	}
	// Get the size of the text
	int fw = 0;
	int fh = 0;
	fl_font(field.t_style.font, field.t_style.size);
	fl_measure(text.c_str(), fw, fh);
	fw = max(fw, 45);
	fh = max(fh, 15);
	// Get the X and Y positions
	int fx = (field.dx == -1) ? next_x_ : field.dx;
	int fy = (field.dy == -1) ? next_y_ : field.dy;
	// Are we displaying the text?
	if (field.display_empty || text.length()) {
		// Draw the box if necessary
		fl_color(FL_BLACK);
		if (field.box) {
			if (field.display_empty) {
				fl_rect(fx, fy, fw + 4, fh + 4, FL_BLACK);
			} else {
				fl_rect(fx, fy, fw + 4, fh + 4, fl_lighter(FL_BLACK));
			}
		}
		// Draw the text
		if (!field.display_empty) {
			fl_color(fl_lighter(field.t_style.colour));
		} else {
			fl_color(field.t_style.colour);
		}
		fl_draw(text.c_str(), fx + 2, fy + 2 + fl_height() - fl_descent());
		// Now display the label
		int lw = 0;
		int lh = 0;
		fl_font(field.l_style.font, field.l_style.size);
		if (!field.display_empty) {
			fl_color(fl_lighter(field.l_style.colour));
		} else {
			fl_color(field.l_style.colour);
		}
		fl_measure(field.label.c_str(), lw, lh);
		// Work out the label start position
		int lx;
		int ly;
		if (field.vertical) { 
			// Position it above the text centred
			ly = fy - lh + fl_height() - fl_descent() - 1;
			lx = fx + 2 + (fw - lw) / 2;
		} else {
			// Position it to the left of the text (and centred
			lx = fx - lw - 1;
			ly = fy + (fh + fl_height() - fl_descent()) / 2;
		}
		// Drae the label
		fl_draw(field.label.c_str(), lx, ly);
	}

	// set the next X,Y position
	if (field.vertical) {
		// We are actually drawing to the right
		next_x_ = fx + fw + 4;
		next_y_ = fy;
	} else {
		next_x_ = fx;
		next_y_ = fy + fh + 4;
	}
}


void qsl_display::draw_text(text_def& text) {
	fl_font(text.t_style.font, text.t_style.size);
	fl_color(text.t_style.colour);
	// Draw the text
	fl_draw(text.text.c_str(), text.dx, text.dy + fl_height() - fl_descent());
}

void qsl_display::draw_image(image_def& image) {
	if (image.image) {
		image.image->draw(image.dx, image.dy, image.dw, image.dh, 0, 0);
	}
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
			split_line(line, words, '\t');
			if (words.size() > 1 && (item_type)stoi(words[0]) != NONE) {
				qsl_display::item_def* item = new qsl_display::item_def;
				item->type = (item_type)stoi(words[0]);
				switch(item->type) {
				case FIELD: {
					if (words.size() == 15) {
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
				case TEXT: {
					if (words.size() == 7) {
						item->text.text = words[1];
						item->text.t_style.font = (Fl_Font)stoi(words[2]);
						item->text.t_style.size = (Fl_Fontsize)stoi(words[3]);
						item->text.t_style.colour = (Fl_Color)stoi(words[4]);
						item->text.dx = stoi(words[5]);
						item->text.dy = stoi(words[6]);
					}
					break;
				}
				case IMAGE: {
					if (words.size() == 6) {
						item->image.filename = words[1];
						item->image.dx = stoi(words[2]);
						item->image.dy = stoi(words[3]);
						item->image.dw = stoi(words[4]);
						item->image.dh = stoi(words[5]);
						item->image.image = get_image(words[1]);
					}
					break;
				}
				}
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
		if (item->type != NONE) {
			op << (int)item->type << '\t';
			switch (item->type) {
			case FIELD: {
				op << item->field.field << '\t' << 
					item->field.label << '\t' << 
					(int)item->field.l_style.font << '\t' << 
					(int)item->field.l_style.size << '\t' <<
					(int)item->field.l_style.colour << '\t' <<
					(int)item->field.t_style.font << '\t' << 
					(int)item->field.t_style.size <<	'\t' << 
					(int)item->field.t_style.colour << '\t' << 
					item->field.dx << '\t' << 
					item->field.dy << '\t' << 
					(int)item->field.vertical << '\t' << 
					(int)item->field.multi_qso << '\t' << 
					(int)item->field.box << '\t' <<
					(int)item->field.display_empty << endl;
				break;
			}
			case TEXT: {
				op << item->text.text << '\t' <<
					(int)item->text.t_style.font << '\t' <<
					(int)item->text.t_style.size << '\t' <<
					(int)item->text.t_style.colour << '\t' <<
					item->text.dx << '\t' <<
					item->text.dy << endl; 
				break;
			}
			case IMAGE: {
				op << item->image.filename << '\t' <<
					item->image.dx << '\t' <<
					item->image.dy << '\t' <<
					item->image.dw << '\t' <<
					item->image.dh << endl;
				break;
			}
			}			
		}
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
    
Fl_Image* qsl_display::get_image(string filename) {
	size_t pos = filename.find_last_of('.');
	Fl_Image* image = nullptr;
	if (pos == string::npos) {
		char msg[128];
		snprintf(msg, sizeof(msg), "QSL: File %s cannot be identified");
		status_->misc_status(ST_ERROR, msg);
	} else if (filename.substr(pos) == ".jpg") {
		// Open as JPEG
		image = new Fl_JPEG_Image(filename.c_str());
	} else if (filename.substr(pos) == ".png") {
		image = new Fl_PNG_Image(filename.c_str());
	} else if (filename.substr(pos) == ".bmp") {
		image = new Fl_BMP_Image(filename.c_str());
	}
	if (image == nullptr) {
		return nullptr;
	} else if (image->fail()) {
		delete image;
		return nullptr;
	} else {
		return image;
	}
}


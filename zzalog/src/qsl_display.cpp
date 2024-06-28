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

// Static data item containing all card parameters
map<string, qsl_display::card_data> qsl_display::all_data_;

// Constructor - just initialises the data
qsl_display::qsl_display(int X, int Y, int W, int H, const char* L) : Fl_Widget(X, Y, W, H, L) {
    color(FL_WHITE);
	box(FL_FLAT_BOX);
    qsos_ = nullptr;
    num_records_ = 0;
	callsign_ = "";
    editable_ = false;
}

// Destructor - save the data
qsl_display::~qsl_display() {
	save_data();
};

// Set the callsign whose card design to display and 
// Any QSOs to include in it.
void qsl_display::value(string callsign, record** qsos, int num_records) {
    qsos_ = qsos;
    num_records_ = num_records;
    callsign_ = callsign;
	// If we haven't loaded data for the callsign yet, do it now
    if (all_data_.find(callsign) == all_data_.end()) {
		load_data(callsign);
	}
	// Select the data for this callsign
	data_ = &all_data_.at(callsign);
	// Redraw all the items
    redraw();
	// And size the widget to fit.
	resize();
}

// Set the size of the widget to size of the label
void qsl_display::resize() {
    Fl_Widget::size(to_points(data_->width), to_points(data_->height));
}

// Use this QSO to display an example card
void qsl_display::example_qso(record* qso) {
	if (qso) {
		qsos_ = new record*[1];
		qsos_[0] = qso;
		num_records_ = 1;
	} else {
		qsos_ = nullptr;
		num_records_ = 0;
	}
	redraw();
}

// Overload of the Fl_Widget::draw() method
void qsl_display::draw() {
	// Colour the whole display
	fl_rectf(x(), y(), w(), h(), FL_WHITE);
	// For each item...
    for (auto it = data_->items.begin(); it != data_->items.end(); it++) {
        item_def& item = *(*it);
		// Call the draw_<type> method for this item type
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

// Draw an individual field item 
void qsl_display::draw_field(field_def& field) {
	Fl_Font savef = fl_font();
	Fl_Fontsize savez = fl_size();
	// Generate the value text for the field
	string text = "";
	if (num_records_ == 0) {
		// No records - so use the field name as the value
		text = field.field;
		if (field.multi_qso) {
			// Add "ditto" to subsequent lines of the value
			for (int ix = 1; ix < data_->max_qsos; ix++) {
				text +="\nditto";
			}
		}
	} else if (field.multi_qso) {
		// We have records and displaying multiple QSOs on separate lines
		for (int i = 0; i < num_records_; i++) {
			// Separate each QSL's value with new-line
			if (i > 0) text += '\n';
			string value = qsos_[i]->item(field.field);
			// Format date and time items, leave others as is
			if (field.field == "QSO_DATE" ||
				field.field == "QSO_DATE_OFF") {
				text += convert_date(value);
			} else if (field.field == "TIME_ON" ||
				field.field == "TIME_OFF") {
				text += convert_time(value);
			} else {
				text += value;
			}
		}
	} else {
		// We have records and concatenating valeus into a single string
		set<string> values;
		// Only print each values once
		for (int i = 0; i < num_records_; i++) {
			string value = qsos_[i]->item(field.field);
			// Format date and time items, leave others as is
			if (field.field == "QSO_DATE" ||
				field.field == "QSO_DATE_OFF") {
				value = convert_date(value);
			} else if (field.field == "TIME_ON" ||
				field.field == "TIME_OFF") {
				value = convert_time(value);
			}
			values.insert(value);
		}
		for (auto it = values.begin(); it != values.end(); it++) {
			// Separate each value with a space 
			if (it != values.begin()) text += ' ';
			text += *it;
		}
	}
	// Get the size of the text
	int fw = 0;
	int fh = 0;
	int box_gap = 0;
	fl_font(field.t_style.font, field.t_style.size);
	fl_measure(text.c_str(), fw, fh);
	// Have a minimum size 
	fw = max(fw, 45);
	fh = max(fh, 15);
	// Get the X and Y positions - "-1" indicates abut it to previous item
	int fx = (field.dx == -1) ? next_x_ : field.dx + x();
	int fy = (field.dy == -1) ? next_y_ : field.dy + y();
	// Are we displaying the field if its value is the empty string?
	if (field.display_empty || text.length()) {
		// Draw the box if necessary
		fl_color(FL_BLACK);
		if (field.box) {
			// Keep a 2 pixel border around the measured text
			box_gap = 2;
			// If this item is not displayed if the qSo field is empty then loighten the box
			if (field.display_empty) {
				fl_rect(fx, fy, fw + 2 * box_gap, fh + 2 * box_gap, FL_BLACK);
			} else {
				fl_rect(fx, fy, fw + 2 * box_gap, fh + 2 * box_gap, fl_lighter(FL_BLACK));
			}
		} else {
			box_gap = 0;
		}
		// Draw the text - use a lighter shade if empty text is not displayed
		if (!field.display_empty) {
			fl_color(fl_lighter(field.t_style.colour));
		} else {
			fl_color(field.t_style.colour);
		}
		fl_draw(text.c_str(), fx + box_gap, fy + box_gap,
			fw, fh, FL_ALIGN_LEFT, nullptr, false);
		// Now display the label
		int lw = 0;
		int lh = 0;
		fl_font(field.l_style.font, field.l_style.size);
		// Display label ina lighter shade if an empty field is not displayed
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
			// Position it above the text centred horizontally
			ly = fy - lh + fl_height() - fl_descent() - 1;
			lx = fx + box_gap + (fw - lw) / 2;
		} else {
			// Position it to the left of the text centred vertically
			lx = fx - lw - 1;
			ly = fy + box_gap + (fh - lh) / 2 + fl_height() - fl_descent();
		}
		// Draw the label
		if (field.display_empty || text.length()) {
			fl_draw(field.label.c_str(), lx, ly);
		}
	}

	// set the next X,Y position
	if (field.vertical) {
		// The next item will be drawn to the right
		if (field.box)
		next_x_ = fx + fw + 2 * box_gap;
		next_y_ = fy;
	} else {
		// The next item will be drawn below
		next_x_ = fx;
		next_y_ = fy + fh + 2 * box_gap;
	}
}

// Draw a text item
void qsl_display::draw_text(text_def& text) {
	fl_font(text.t_style.font, text.t_style.size);
	fl_color(text.t_style.colour);
	// Draw the text
	fl_draw(text.text.c_str(), text.dx + x(), text.dy + y() + fl_height() - fl_descent());
}

// Draw an image item - note this draws the imageits actual size
void qsl_display::draw_image(image_def& image) {
	if (image.image) {
		image.image->draw(image.dx + x(), image.dy + y());
	}
}

// Set/get the editable flag
bool qsl_display::editable() { return editable_; }
void qsl_display::editable(bool enable) { editable_ = enable; }

// Convert value from specified unit to points
int qsl_display::to_points(float value) {
	switch (data_->unit) {
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
void qsl_display::load_data(string callsign) {
	card_data* data = &(all_data_[callsign]);
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	char* temp;
	Fl_Preferences call_settings(qsl_settings, callsign.c_str());
	// Get the card label data for this callsign
	call_settings.get("Width", data->width, 0);
	call_settings.get("Height", data->height, 0);
	call_settings.get("Unit", (int&)data->unit, (int)MILLIMETER);
	call_settings.get("Number Rows", data->rows, 4);
	call_settings.get("Number Columns", data->columns, 2);
	call_settings.get("Column Width", data->col_width, 101.6);
	call_settings.get("Row Height", data->row_height, 67.7);
	call_settings.get("First Row", data->row_top, 12.9);
	call_settings.get("First Column", data->col_left, 4.6);
	call_settings.get("Max QSOs per Card", data->max_qsos, 1);
	call_settings.get("Date Format", (int&)data->f_date, FMT_Y4MD_ADIF);
	call_settings.get("Time Format", (int&)data->f_time, FMT_HMS_ADIF);
	call_settings.get("Card Design", temp, "");
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
	// Minimum data required
	if (data->width == 0 || data->height == 0 || data->filename.length() == 0) {
		// We have either width or height not defined - so load the edfault data
		char msg[256];
		snprintf(msg, sizeof(msg), "QSL: Incorrect info W=%g, H=%g, File=%s", data->width, data->height, data->filename.c_str());
		status_->misc_status(ST_ERROR, msg);
		data->items.clear();
	} else {
		// Data looks good so far
		char msg[100];
		snprintf(msg, sizeof(msg), "QSL: Reading card image data from %s", data->filename.c_str());
		data->items.clear();
		status_->misc_status(ST_LOG, msg);
		ifstream ip;
		ip.open(data->filename.c_str(), fstream::in);
		string line;
		vector<string> words;
		// For each line in the file.. 
		while(ip.good()) {
			getline(ip, line);
			// Split lines on the 'tab' character
			split_line(line, words, '\t');
			if (words.size() > 1 && (item_type)stoi(words[0]) != NONE) {
				// The line probably contains good data
				qsl_display::item_def* item = new qsl_display::item_def;
				item->type = (item_type)stoi(words[0]);
				switch(item->type) {
				case FIELD: {
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
				case TEXT: {
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
				case IMAGE: {
					// A line contains a single image item data (except actual image data)
					if (words.size() >= 6) {
						item->image.filename = words[1];
						item->image.dx = stoi(words[2]);
						item->image.dy = stoi(words[3]);
						// Load  the image data from the named file into the data structure
						item->image.image = get_image(words[1]);
					}
					break;
				}
				}
				// Add the item's data to the data structure
				data->items.push_back(item);
			}
		}
		ip.close();
		snprintf(msg, sizeof(msg), "QSL: %zd items read from %s", data->items.size(), data->filename.c_str());
		status_->misc_status(ST_OK, msg);
	}
}

// Save the data for the current callsign
void qsl_display::save_data() {
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	Fl_Preferences call_settings(qsl_settings, callsign_.c_str());
	// SAve the card label parameters
	call_settings.set("Width", data_->width);
	call_settings.set("Height", data_->height);
	call_settings.set("Unit", (int)data_->unit);
	call_settings.set("Number Rows", data_->rows);
	call_settings.set("Number Columns", data_->columns);
	call_settings.set("Column Width", data_->col_width);
	call_settings.set("Row Height", data_->row_height);
	call_settings.set("First Row", data_->row_top);
	call_settings.set("First Column", data_->col_left);
	call_settings.set("Max QSOs per Card", data_->max_qsos);
	call_settings.set("Date Format", (int)data_->f_date);
	call_settings.set("Time Format", (int)data_->f_time);
	call_settings.set("Card Design", data_->filename.c_str());
	// Writing the file with the drawing data
   	char msg[100];
	snprintf(msg, sizeof(msg), "QSL: Writing card image data to %s", data_->filename.c_str());
	status_->misc_status(ST_LOG, msg);
	ofstream op;
	op.open(data_->filename.c_str(), fstream::out);
	string line;
	int pos = 0;
	// For all drawing items...
	for (int ix = 0; ix < data_->items.size() && op.good(); ix++) {
		qsl_display::item_def* item = data_->items[ix];
		// Ignore any item marked with type NONE
		if (item->type != NONE) {
			// Write type the a tab
			op << (int)item->type << '\t';
			switch (item->type) {
			case FIELD: {
				// Write field item data separated by tabs
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
				// Write text item data separated by tabs
				op << item->text.text << '\t' <<
					(int)item->text.t_style.font << '\t' <<
					(int)item->text.t_style.size << '\t' <<
					(int)item->text.t_style.colour << '\t' <<
					item->text.dx << '\t' <<
					item->text.dy << endl; 
				break;
			}
			case IMAGE: {
				// Write image item data separted by tabs
				op << item->image.filename << '\t' <<
					item->image.dx << '\t' <<
					item->image.dy << endl;
				break;
			}
			}			
		}
	}
	op.close();
	snprintf(msg, sizeof(msg), "QSL: %zd items written to %s", data_->items.size(), data_->filename.c_str());
	status_->misc_status(ST_OK, msg);
}

// Get a pointer to the card drawing data for the specified callsign
qsl_display::card_data* qsl_display::data(string callsign) {
	// If it hasn't been loaded, do so.
	if (all_data_.find(callsign) == all_data_.end()) {
		load_data(callsign);
	}
	return &all_data_.at(callsign);
}

// Read the image data from the specified filename
Fl_Image* qsl_display::get_image(string filename) {
	// Get the file type (expect one of .jpg, .png, .bmp)
	size_t pos = filename.find_last_of('.');
	Fl_Image* image = nullptr;
	if (pos == string::npos) {
		char msg[128];
		snprintf(msg, sizeof(msg), "QSL: File %s cannot be identified", filename.c_str());
		status_->misc_status(ST_ERROR, msg);
	} else if (filename.substr(pos) == ".jpg") {
		// Open as JPEG
		image = new Fl_JPEG_Image(filename.c_str());
	} else if (filename.substr(pos) == ".png") {
		// Open as PNG
		image = new Fl_PNG_Image(filename.c_str());
	} else if (filename.substr(pos) == ".bmp") {
		// Open as Bitmap
		image = new Fl_BMP_Image(filename.c_str());
	}
	if (image == nullptr) {
		// File not image data
		return nullptr;
	} else if (image->fail()) {
		// File did not load successfully
		delete image;
		return nullptr;
	} else {
		// Supply image
		return image;
	}
}

// Format date
string qsl_display::convert_date(string value) {
	// Input format is YYYYMMDD
	string result;
	switch(data_->f_date) {
		case FMT_Y4MD_ADIF:
		// "20240621"
		result = value;
			return result;
		case FMT_Y4MD:
		// "2024/06/21"
			result = value.substr(0, 4) + '/' + value.substr(4, 2) + '/' + value.substr(6, 2);
			return result;
		case FMT_Y2MD:
		// "24/06/21"
			result = value.substr(0, 4) + '/' + value.substr(4, 2) + '/' + value.substr(6, 2);
			return result;
		case FMT_DMY2:
		// "21/06/24"
			result = value.substr(6,2) + '/' + value.substr(4,2) + '/' + value.substr(2,2);
			return result;
		case FMT_MDY2:
		// ""06/21/24"
			result = value.substr(4,2) + '/' + value.substr(6,2) + '/' + value.substr(2,2);
			return result;
		default:
			return "Invalid";
	}
}

// Format time
string qsl_display::convert_time(string value) {
	// Input format may be HHMMSS or HHMM
	string result;
	if (value.length() == 4) {
		// input is HHMM - do not halucinate seconds
		switch(data_->f_time) {
			case FMT_HMS_ADIF:
			case FMT_HM_ADIF:
				// "1448"
				result = value;
				return result;
			case FMT_HMS:
			case FMT_HM:
				// "14:48"
				result = value.substr(0,2) + ':' + value.substr(2,2);
				return result;
			default:
				return "Invalid";
		}
	} else {
		switch(data_->f_time) {
			case FMT_HMS_ADIF:
				// "144850"
				result = value;
				return result;
			case FMT_HMS:
				// "14:48:50"
				result = value.substr(0,2) + ':' + value.substr(2,2) + ':' + value.substr(4,2);
				return result;
			case FMT_HM_ADIF:
				// "1448"
				result = value.substr(0,4);
				return result;
			case FMT_HM:
				// "14:48"
				result = value.substr(0,2) + ':' + value.substr(2,2);
				return result;
			default:
				return "Invalid";
		}
	}
}

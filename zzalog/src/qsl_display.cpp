#include "qsl_display.h"
#include "qso_manager.h"
#include "status.h"
#include "record.h"
#include "utils.h"

#include <set>
#include <string>

#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Image_Surface.H>

using namespace std;

extern qso_manager* qso_manager_;
extern status* status_;
// Dynamically drawn QSL card label

// Constructor - just initialises the data
qsl_display::qsl_display(int X, int Y, int W, int H)
{
    qsos_ = nullptr;
    num_records_ = 0;
	alt_image_ = nullptr;
	data_ = nullptr;
	alt_text_ = nullptr;
	alt_colour_ = FL_FOREGROUND_COLOR;
	x_ = X;
	y_ = Y;
	w_ = W;
	h_ = H;
	draw_x_ = X;
	draw_y_ = Y;
	draw_w_ = W;
	draw_h_ = H;
	next_x_ = X;
	next_y_ = Y;
	if (W == 0 || H == 0) {
		do_scale_ = false;
		scaling_ = 1.0;
	}
	else {
		do_scale_ = true;
		scaling_ = 1.0;
	}
}

// Destructor - save the data
qsl_display::~qsl_display() {
};

// Overload of the Fl_Widget::draw() method
void qsl_display::draw() {
	// Colour the whole display
	fl_rectf(x_, y_, w_, h_, FL_BACKGROUND_COLOR);
	if (data_) {
		calculate_scale(to_points(data_->width), to_points(data_->height));
		if (data_->items.size() == 0) {
			draw_text("NO IMAGE!", FL_RED);
		}
		else {
			// Now colour the drawing area
			fl_rectf(draw_x_, draw_y_, draw_w_, draw_h_, FL_WHITE);
			// For each item...
			for (auto it = data_->items.begin(); it != data_->items.end(); it++) {
				qsl_data::item_def& item = *(*it);
				// Call the draw_<type> method for this item type
				switch (item.type) {
				case qsl_data::FIELD: {
					draw_field(item.field);
					break;
				}
				case qsl_data::TEXT: {
					draw_text(item.text);
					break;
				}
				case qsl_data::IMAGE: {
					draw_image(item.image);
					break;
				}
				default:
					break;
				}
			}
		}
	}
	else if (alt_image_) {
		calculate_scale(alt_image_->w(), alt_image_->h());
		draw_image(0, 0, alt_image_);
	}
	else if (alt_text_) {
		draw_text(alt_text_, alt_colour_);
	}
	else {
		draw_text("NO IMAGE!", FL_RED);
	}
	
}

// Draw an individual field item 
void qsl_display::draw_field(qsl_data::field_def& field) {
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
			string value = qsos_[i]->item(field.field, false, true);
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
			string value = qsos_[i]->item(field.field, false, true);
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
	int text_gap = 0;
	fl_font(field.t_style.font, scale(field.t_style.size));
	fl_measure(text.c_str(), fw, fh);
	// Have a minimum size 
	int min_size = scale(45);
	// First centre the text horizontally
	int dx = max(min_size - fw, 0) / 2;
	fw = max(fw, min_size);
	fh = max(fh, (min_size / 3));
	// Get the X and Y positions - "-1" indicates abut it to previous item
	int fx = (field.dx == -1) ? next_x_ : draw_x_ + scale(field.dx);
	int fy = (field.dy == -1) ? next_y_ : draw_y_ + scale(field.dy);
	// Are we displaying the field if its value is the empty string?
	if (field.display_empty || text.length()) {
		// Draw the box if necessary
		fl_color(FL_BLACK);
		if (field.box) {
			// Keep a 2 pixel border around the measured text
			box_gap = scale(2);
			// If this item is not displayed if the qSo field is empty then loighten the box
			if (field.display_empty) {
				fl_rect(fx, fy, fw + 2 * box_gap, fh + 2 * box_gap, FL_BLACK);
			} else {
				fl_rect(fx, fy, fw + 2 * box_gap, fh + 2 * box_gap, fl_lighter(FL_BLACK));
			}
			// Now add the offset for text
			text_gap = box_gap + dx;
		} else {
			box_gap = 0;
			text_gap = 0;
		}
		// Draw the text - use a lighter shade if empty text is not displayed
		if (!field.display_empty) {
			fl_color(fl_lighter(field.t_style.colour));
		} else {
			fl_color(field.t_style.colour);
		}
		fl_draw(text.c_str(), fx + text_gap, fy + box_gap,
			fw, fh, FL_ALIGN_LEFT, nullptr, false);
		// Now display the label
		int lw = 0;
		int lh = 0;
		fl_font(field.l_style.font, scale(field.l_style.size));
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
	// Restore font
	fl_font(savef, savez);

	// set the next X,Y position
	if (field.vertical) {
		// The next item will be drawn to the right
		next_x_ = fx + fw + 2 * box_gap;
		next_y_ = fy;
	} else {
		// The next item will be drawn below
		next_x_ = fx;
		next_y_ = fy + fh + 2 * box_gap;
	}
}

// Draw a text item
void qsl_display::draw_text(qsl_data::text_def& text) {
	fl_font(text.t_style.font,scale(text.t_style.size));
	fl_color(text.t_style.colour);
	// Draw the text
	string fulltext;
	if (num_records_ == 0) {
		fulltext = text.text;
	}
	else {
		fulltext = qsos_[0]->item_merge(text.text, true);
	}
	// Get the X and Y positions - "-1" indicates abut it to previous item
	int fx = (text.dx == -1) ? next_x_ : draw_x_ + scale(text.dx);
	int fy = (text.dy == -1) ? next_y_ : draw_y_ + scale(text.dy);
	fl_draw(fulltext.c_str(), fx, fy + fl_height() - fl_descent());
	// The next item will be drawn below
	int fw = 0, fh = 0;
	fl_measure(fulltext.c_str(), fw, fh);
	// set the next X,Y position
	if (text.vertical) {
		// The next item will be drawn to the right
		next_x_ = fx + fw;
		next_y_ = fy;
	}
	else {
		// The next item will be drawn below
		next_x_ = fx;
		next_y_ = fy + fh;
	}
}

// Draw an image item -
void qsl_display::draw_image(qsl_data::image_def& image) {
	Fl_Image* image_data = get_image(image.filename);
	if (image_data) {
		draw_image(image.dx, image.dy, image_data);
	}
}

// Scale and dr5aw the iamge
void qsl_display::draw_image(int x, int y, Fl_Image* image) {
	if (do_scale_) {
		Fl_Image* scale_image = image->copy(scale(image->w()), scale(image->h()));
		scale_image->draw(draw_x_ + scale(x), draw_y_ + scale(y));
	}
	else {
		image->draw(x_ + x, y_ + y);
	}
}

// Scale and draw the text
void qsl_display::draw_text(const char* text, Fl_Color colour) {
	// Colour the whole display
	fl_rectf(x_, y_, w_, h_, FL_BACKGROUND_COLOR);
	Fl_Fontsize sz = fl_size();
	Fl_Font f = fl_font();
	int size = 48;
	fl_font(0, size);
	int w = 0, h = 0;
	fl_measure(text, w, h);
	while (w > w_) {
		size--;
		fl_font(0, size);
		fl_measure(text, w, h);
	}
	fl_color(colour);
	fl_draw(text, x_, y_, w_, h_, FL_ALIGN_CENTER);
	fl_font(f, sz);
}

// Draw al

// Convert value from specified unit to points
int qsl_display::to_points(float value) {
	switch (data_->unit) {
	case qsl_data::INCH:
		// 72 points per inch
		return (int)(value * IN_TO_POINT);
	case qsl_data::MILLIMETER:
		// 72/25.4 points per mm
		return (int)(value * MM_TO_POINT);
	case qsl_data::POINT:
		return (int)value;
	default:
		return 0;
	}
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
	char msg[128];
	if (image == nullptr) {
		snprintf(msg, sizeof(msg), "QSL: File %s is not valid image data", filename.c_str());
		status_->misc_status(ST_ERROR, msg);
		// File not image data
		return nullptr;
	} else {
		switch (image->fail()) {
		case 0:
			return image;
		case Fl_Image::ERR_NO_IMAGE:
		case Fl_Image::ERR_FILE_ACCESS:
			snprintf(msg, sizeof(msg), "QSL: File %s access failed - %s", 
				filename.c_str(),
				strerror(errno));
			status_->misc_status(ST_ERROR, msg);
			break;
		case Fl_Image::ERR_FORMAT:
			snprintf(msg, sizeof(msg), "QSL: File %s cannot decode image", filename.c_str());
			status_->misc_status(ST_ERROR, msg);
			break;
		case Fl_Image::ERR_MEMORY_ACCESS:
			snprintf(msg, sizeof(msg), "QSL: File %s memory violation", filename.c_str());
			status_->misc_status(ST_ERROR, msg);
			break;
		}
		// File did not load successfully
		delete image;
		return nullptr;
	}
}

// Format date
string qsl_display::convert_date(string value) {
	// Input format is YYYYMMDD
	string result;
	switch(data_->f_date) {
	case qsl_data::FMT_Y4MD_ADIF:
		// "20240621"
		result = value;
			return result;
		case qsl_data::FMT_Y4MD:
		// "2024/06/21"
			result = value.substr(0, 4) + '/' + value.substr(4, 2) + '/' + value.substr(6, 2);
			return result;
		case qsl_data::FMT_Y2MD:
		// "24/06/21"
			result = value.substr(0, 4) + '/' + value.substr(4, 2) + '/' + value.substr(6, 2);
			return result;
		// case qsl_data::FMT_DMY2:
		// // "21/06/24"
		// 	result = value.substr(6,2) + '/' + value.substr(4,2) + '/' + value.substr(2,2);
		// 	return result;
		// case qsl_data::FMT_MDY2:
		// // ""06/21/24"
		// 	result = value.substr(4,2) + '/' + value.substr(6,2) + '/' + value.substr(2,2);
		// 	return result;
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
			case qsl_data::FMT_HMS_ADIF:
			case qsl_data::FMT_HM_ADIF:
				// "1448"
				result = value;
				return result;
			case qsl_data::FMT_HMS:
			case qsl_data::FMT_HM:
				// "14:48"
				result = value.substr(0,2) + ':' + value.substr(2,2);
				return result;
			default:
				return "Invalid";
		}
	} else {
		switch(data_->f_time) {
			case qsl_data::FMT_HMS_ADIF:
				// "144850"
				result = value;
				return result;
			case qsl_data::FMT_HMS:
				// "14:48:50"
				result = value.substr(0,2) + ':' + value.substr(2,2) + ':' + value.substr(4,2);
				return result;
			case qsl_data::FMT_HM_ADIF:
				// "1448"
				result = value.substr(0,4);
				return result;
			case qsl_data::FMT_HM:
				// "14:48"
				result = value.substr(0,2) + ':' + value.substr(2,2);
				return result;
			default:
				return "Invalid";
		}
	}
}


// Set new card design data
void qsl_display::set_card(qsl_data* data) {
	data_ = data;
	alt_image_ = nullptr;
	alt_text_ = nullptr;
}

// Set the QSOs to display in the card
void qsl_display::set_qsos(record** qsos, int num_qsos) {
	qsos_ = qsos;
	num_records_ = num_qsos;
}

// Set alternate image
void qsl_display::set_image(Fl_Image* image) {
	alt_image_ = image;
	data_ = nullptr;
	alt_text_ = nullptr;
}

// set alternate text
void qsl_display::set_text(const char* text, Fl_Color colour) {
	delete alt_text_;
	if (text == nullptr) alt_text_ = nullptr;
	else {
		alt_text_ = new char[strlen(text) + 1];
		strcpy(alt_text_, text);
	}
	alt_image_ = nullptr;
	data_ = nullptr;
	alt_colour_ = colour;
}

// Scale 
int qsl_display::scale(int value) {
	if (do_scale_) {
		return (int)((float)value * scaling_);
	}
	else {
		return value;
	}
}

// Calculate scaling factor
void qsl_display::calculate_scale(int w, int h) {
	if (w_ == 0 || h_ == 0 || (w_ == w && h_ == h)) {
		// No scaling use supplied w and h values
		w_ = w;
		h_ = h;
		do_scale_ = false;
		scaling_ = 1.0;
		draw_x_ = x_;
		draw_y_ = y_;
		draw_w_ = w_;
		draw_h_ = h_;
	}
	else {
		// What is the scale factor - minimum of w sca;ling and h scaling
		float scale_w = (float)w_ / (float)w;
		float scale_h = (float)h_ / (float)h;
		do_scale_ = true;
		if (scale_h < scale_w) {
			scaling_ = scale_h;
			// Set drawing coordinates
			draw_x_ = x_ + (w_ - scale(w)) / 2;
			draw_w_ = scale(w);
			draw_y_ = y_;
			draw_h_ = h_;
		}
		else {
			scaling_ = scale_w;
			// Set drawing coordinates
			draw_x_ = x_;
			draw_w_ = w_;
			draw_y_ = y_ + (h_ - scale(h)) / 2;
			draw_h_ = scale(h);
		}
	}
}

// Resize thedesign
void qsl_display::resize(int w, int h) {
	w_ = w;
	h_ = h;
}

// Get the unscaled size
void qsl_display::get_size(int& w, int& h) {
	if (data_) {
		// Unscaled size of the design
		w = to_points(data_->width);
		h = to_points(data_->height);
	}
	else if (alt_image_) {
		// The unscaled size of the image
		w = alt_image_->w();
		h = alt_image_->h();
	}
	else {
		// default to size of the widget
		w = w_;
		h = h_;
	}
}

#include "qsl_form.h"
#include "record.h"
#include "drawing.h"
#include "../zzalib/utils.h"
#include "qsl_design.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern Fl_Preferences* settings_;

// Constructor
qsl_form::qsl_form(int X, int Y, record* record) :
	Fl_Group(X, Y, 0, 0) {
	load_data();
	// Get record
	record_ = record;
	// Draw the widgets
	create_form();
}

// Destructor
qsl_form::~qsl_form() {
}

// Recreate the form
void qsl_form::update() {
	// Delete existing widgets and create the form anew
	clear();
	redraw();
	begin();
	create_form();
}

// Load default data - if settings not yet done.
void qsl_form::load_default() {
	// Card size - 99.1 x 67.7 mm
	width_ = 99.1f;
	height_ = 66.7f;
	unit_ = MILLIMETER;
	// Default design - Contact call un top right
	tr_widgets_ = {
		{ "To <CALL>", FL_RED, 14, FL_HELVETICA }
	};
	// Default design - location info in top left
	tl_widgets_ = {
		{ "<MY_NAME> <STATION_CALLSIGN>", FL_BLACK, 8, FL_HELVETICA },
		{ "<MY_CITY>",  FL_BLACK, 8, FL_HELVETICA },
		{ "<MY_COUNTRY>", FL_BLACK, 8, FL_HELVETICA },
		{ "CQ: <MY_CQ_ZONE> ITU:<MY_ITU_ZONE> IOTA:<MY_IOTA>", FL_BLACK, 8, FL_HELVETICA },
		{ "Grid: <MY_GRIDSQUARE>", FL_BLACK, 8, FL_HELVETICA }

	};
	// CEntral box - QSO details
	tab_widgets_ = {
		{{ "Date", FL_BLACK, 10, FL_HELVETICA },
		 { "Time", FL_BLACK, 10, FL_HELVETICA },
		 { "Band", FL_BLACK, 10, FL_HELVETICA },
		 { "Mode", FL_BLACK, 10, FL_HELVETICA },
		 { "Report", FL_BLACK, 10, FL_HELVETICA }
		 },
		{{ "<QSO_DATE>", FL_BLUE, 10, FL_HELVETICA_ITALIC },
		 { "<TIME_ON>", FL_BLUE, 10, FL_HELVETICA_ITALIC },
		 { "<BAND>", FL_BLUE, 10, FL_HELVETICA_ITALIC },
		 { "<MODE>", FL_BLUE, 10, FL_HELVETICA_ITALIC },
		 { "<RST_SENT>", FL_BLUE, 10, FL_HELVETICA_ITALIC }
		}
	};
	// Bottom right - salutation
	br_widgets_ = {
		{ "Tnx Report, <NAME>, 73", FL_BLACK, 10, FL_HELVETICA_ITALIC }
	};
	// Bottom left - station details
	bl_widgets_ = {
		{ "Rig: <MY_RIG>", FL_BLACK, 8, FL_HELVETICA },
		{ "Ant: <MY_ANTENNA>", FL_BLACK, 8, FL_HELVETICA }
	};
}

void qsl_form::load_data() {
	// Get width and height of label
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	qsl_settings.get("Width", width_, 0);
	qsl_settings.get("Height", height_, 0);
	qsl_settings.get("Unit", (int&)unit_, (int)MILLIMETER);
	if (width_ == 0 || height_ == 0) {
		// We have either width or height not defined
		load_default();
	}
	else {
		// REad in the five sets of widget data
		tl_widgets_.clear();
		Fl_Preferences tl_settings(qsl_settings, "Top Left");
		int count = tl_settings.groups();
		read_settings(tl_settings, tl_widgets_, count);
		Fl_Preferences tr_settings(qsl_settings, "Top Right");
		count = tr_settings.groups();
		read_settings(tr_settings, tr_widgets_, count);
		Fl_Preferences bl_settings(qsl_settings, "Bottom Left");
		count = bl_settings.groups();
		read_settings(bl_settings, bl_widgets_, count);
		Fl_Preferences br_settings(qsl_settings, "Bottom Right");
		count = br_settings.groups();
		read_settings(br_settings, br_widgets_, count);
		// The table settings are more complicated
		Fl_Preferences table_settings(qsl_settings, "Table");
		int num_rows = table_settings.groups();
		tab_widgets_.resize(num_rows);
		for (int i = 0; i < num_rows; i++) {
			Fl_Preferences row_settings(table_settings, table_settings.group(i));
			int num_cols = row_settings.groups();
			read_settings(row_settings, tab_widgets_[i], num_cols);
		}
	}
}

// REad one set of widget data
void qsl_form::read_settings(Fl_Preferences& settings, vector<qsl_form::qsl_widget>& widgets, int count) {
	widgets.resize(count);
	for (int i = 0; i < count; i++) {
		Fl_Preferences line_settings(settings, settings.group(i));
		char* temp;
		line_settings.get("Value", temp, "Default");
		widgets[i].text = temp;
		free(temp);
		line_settings.get("Font", (int&)widgets[i].font, (int)FL_HELVETICA);
		line_settings.get("Size", widgets[i].font_size, 10);
		line_settings.get("Colour", (int&)widgets[i].colour, (int)FL_BLACK);
	}
}

// Create the card design
void qsl_form::create_form() {
	// dimensions in points
	int width = to_points(width_);
	int height = to_points(height_);
	size(width, height);
	// current y value on both sides
	int ly = y() + MARGIN;
	int ry = y() + MARGIN;
	// Draw lines of text both top left and top right
	draw_lines(FL_ALIGN_LEFT, ly, tl_widgets_);
	draw_lines(FL_ALIGN_RIGHT, ry, tr_widgets_);
	// Current y value to below the lower of the two
	int curr_y = max(ly, ry) + GAP;
	// Draw the central box
	draw_table(curr_y);
	curr_y += GAP;
	// Current y below the central box
	ly = curr_y;
	ry = curr_y;
	draw_lines(FL_ALIGN_LEFT, ly, bl_widgets_);
	draw_lines(FL_ALIGN_RIGHT, ry, br_widgets_);
	redraw();
	end();
}

// Draw the lines of text for one of the four corners,
// align indicates left or right, curr_y where to put it, widgets - the lines of text
// returns with the latest y position in curr_y
void qsl_form::draw_lines(Fl_Align align, int& curr_y, vector<qsl_form::qsl_widget>& widgets) {
	// dimensions in points
	int width = to_points(width_);
	for (auto it = widgets.begin(); it != widgets.end(); it++) {
		int curr_h = it->font_size + 1;
		int curr_x = x() + ((align & FL_ALIGN_LEFT) ? MARGIN : (width / 2));
		// Add each line of text as a button with no border. use a button so it responds to being clicked
		Fl_Button* item = new Fl_Button(curr_x, curr_y, (width / 2) - MARGIN, curr_h);
		item->box(FL_NO_BOX);
		item->copy_label(record_ ? record_->item_merge(it->text).c_str() : it->text.c_str());
		item->labelcolor(it->colour);
		item->labelfont(it->font);
		item->labelsize(it->font_size);
		item->align(align | FL_ALIGN_INSIDE);
		item->callback(cb_button, (void*)&(*it));

		curr_y += curr_h;
	}
}

// Draw the central box
void qsl_form::draw_table(int& curr_y) {
	// dimensions in points
	int width = to_points(width_);
	// For each row
	for (auto it = tab_widgets_.begin(); it != tab_widgets_.end(); it++) {
		int curr_x = x() + MARGIN;
		int curr_w = (width - MARGIN - MARGIN) / it->size();
		// Get the largest height required for each cell
		int curr_h = 0;
		for (auto col = it->begin(); col != it->end(); col++) {
			int next_h = col->font_size + 1;
			if (next_h > curr_h) curr_h = next_h;
		}
		// Create a button for each cell - with a border
		for (auto col = it->begin(); col != it->end(); col++) {
			Fl_Button* item = new Fl_Button(curr_x, curr_y, curr_w, curr_h);
			item->box(FL_BORDER_FRAME);
			item->copy_label(record_ ? record_->item_merge(col->text, true).c_str() : col->text.c_str());
			item->labelcolor(col->colour);
			item->labelfont(col->font);
			item->labelsize(col->font_size);
			item->align(FL_ALIGN_CENTER);
			item->callback(cb_button, (void*)&(*col));
			curr_x += curr_w;
		}
		curr_y += curr_h;
	}
}

// Call back for all form widgets - this calls qsl_form's callback which can be set by an instancing class
void qsl_form::cb_button(Fl_Widget* w, void* v) {
	qsl_form* that = ancestor_view<qsl_form>(w);
	((qsl_design*)that->designer_)->display_design_data((Fl_Button*)w, (qsl_widget*)v);
}

// Convert value from specified unit to points
int qsl_form::to_points(float value) {
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

// Resize widget set
void qsl_form::resize_set(qsl_form::widget_set set, int rows) {
	vector<qsl_widget>* widgets = nullptr;
	switch (set) {
	case TOP_LEFT:
		widgets = &tl_widgets_;
		break;
	case TOP_RIGHT:
		widgets = &tr_widgets_;
		break;
	case BOTTOM_LEFT:
		widgets = &bl_widgets_;
		break;
	case BOTTOM_RIGHT:
		widgets = &br_widgets_;
		break;
	}
	if (widgets) {
		size_t old_size = widgets->size();
		widgets->resize((size_t)rows);
		if (old_size) {
			if (old_size < (size_t)rows) {
				// Adding rows to existing ones
				qsl_widget& last_widget = (*widgets)[old_size - 1];
				// Add any required new widgets - using colour, font and size of the last one (if there was one
				for (size_t i = old_size; i < (size_t)rows; i++) {
					(*widgets)[i] = last_widget;
					(*widgets)[i].text = "Type text...";
				}
			}
		} else {
			// Add rows to a new set - use Helvetica 8
			for (size_t i = 0; i < (size_t)rows; i++) {
				(*widgets)[i] = { "Type text...", FL_BLACK, 8, FL_HELVETICA };
			}
		}
		update();
	}
}

// Resize table
void qsl_form::resize_table(int rows, int cols) {
	size_t old_rows = tab_widgets_.size();
	size_t old_cols = old_rows == 0 ? 0 : tab_widgets_[0].size();
	if ( rows != -1 && old_rows != rows) {
		// Resize the number of rows
		tab_widgets_.resize(rows);
		// Add any required new widgets - using colour, font and size of the last one (if there was one
		for (size_t i = old_rows; i < (size_t)rows; i++) {
			tab_widgets_[i].resize(old_cols);
			for (size_t j = 0; j < old_cols; j++) {
				tab_widgets_[i][j] = tab_widgets_[old_rows - 1][j];
				tab_widgets_[i][j].text = "Type...";
			}
		}
	}
	size_t num_cols = cols == -1 ? tab_widgets_[0].size() : cols;
	size_t num_rows = tab_widgets_.size();
	for (size_t i = 0; i < num_rows; i++) {
		// Resize each row
		tab_widgets_[i].resize(num_cols);
		for (size_t j = old_cols; j < num_cols; j++) {
			tab_widgets_[i][j] = tab_widgets_[i][old_cols ];
			tab_widgets_[i][j].text = "Type...";
		}
	}

	update();
}

// Change widget text
void qsl_form::update_text(qsl_form::qsl_widget* widget, string value) {
	if (widget) {
		widget->text = value;
		update();
	}
}

// Change widget font
void qsl_form::update_font(qsl_form::qsl_widget* widget, Fl_Font value) {
	if (widget) {
		widget->font = value;
		update();
	}
}

// Change widget font size
void qsl_form::update_size(qsl_form::qsl_widget* widget, int value) {
	if (widget) {
		widget->font_size = value;
		update();
	}
}

// Change widget colour
void qsl_form::update_colour(qsl_form::qsl_widget* widget, Fl_Color value) {
	if (widget) {
		widget->colour = value;
		update();
	}
}

// Return unit
qsl_form::dim_unit qsl_form::unit() {
	return unit_;
}

// Set unit
void qsl_form::unit(dim_unit unit) {
	unit_ = unit;
	update();
}

// Return width
float qsl_form::width() {
	return width_;
}

// Set width
void qsl_form::width(float width) {
	width_ = width;
	update();
}

// Return height
float qsl_form::height() {
	return height_;
}

// Set height
void qsl_form::height(float height) {
	height_ = height;
	update();
}

// Return size
int qsl_form::set_size(qsl_form::widget_set set) {
	switch (set) {
	case TOP_LEFT:
		return tl_widgets_.size();
	case TOP_RIGHT:
		return tr_widgets_.size();
	case BOTTOM_LEFT:
		return bl_widgets_.size();
	case BOTTOM_RIGHT:
		return br_widgets_.size();
	default:
		return -1;
	}
}

// Return table rows
int qsl_form::table_rows() {
	return tab_widgets_.size();
}

// REturn table columns
int qsl_form::table_cols() {
	if (tab_widgets_.size()) {
		return tab_widgets_[0].size();
	}
	else {
		return 0;
	}
}

// Write one set of widget data
void qsl_form::write_settings(Fl_Preferences& settings, vector<qsl_widget>& widgets) {
	for (unsigned int i = 0; i < widgets.size(); i++) {
		char group[3];
		sprintf(group, "%d", i);
		Fl_Preferences line_settings(settings, group);
		line_settings.set("Value", widgets[i].text.c_str());
		line_settings.set("Font", widgets[i].font);
		line_settings.set("Size", widgets[i].font_size);
		line_settings.set("Colour", (int)widgets[i].colour);
	}
}


// Save all the settings
void qsl_form::save_data() {
	// Remove existing settings
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	qsl_settings.clear();
	qsl_settings.set("Width", width_);
	qsl_settings.set("Height", height_);
	qsl_settings.set("Unit", unit_);
	Fl_Preferences tl_settings(qsl_settings, "Top Left");
	write_settings(tl_settings, tl_widgets_);
	Fl_Preferences tr_settings(qsl_settings, "Top Right");
	write_settings(tr_settings, tr_widgets_);
	Fl_Preferences bl_settings(qsl_settings, "Bottom Left");
	write_settings(bl_settings, bl_widgets_);
	Fl_Preferences br_settings(qsl_settings, "Bottom Right");
	write_settings(br_settings, br_widgets_);
	Fl_Preferences table_settings(qsl_settings, "Table");
	for (unsigned int i = 0; i < tab_widgets_.size(); i++) {
		char row[3];
		sprintf(row, "%d", i);
		Fl_Preferences row_settings(table_settings, row);
		write_settings(row_settings, tab_widgets_[i]);
	}
}

// Set designer
void qsl_form::designer(Fl_Group* designer) {
	designer_ = designer;
}

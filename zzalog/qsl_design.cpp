#include "qsl_design.h"
#include "qsl_form.h"
#include "record.h"
#include "book.h"
#include "drawing.h"
#include "callback.h"
#include "utils.h"
#include "settings.h"
#include "intl_widgets.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Tabs.H>

using namespace zzalog;

extern Fl_Preferences* settings_;
extern book* book_;

// Constructor - standard page dialog
qsl_design::qsl_design(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)
	, current_design_(nullptr)
	, current_data_(nullptr)
	, current_widget_(nullptr)
	, ip_text_(nullptr)
	, br_font_(nullptr)
	, br_size_(nullptr)
	, bn_colour_(nullptr)
	, card_window_(nullptr)
{
	current_data_ = new qsl_form::qsl_widget({ "Text here...", FL_BLACK, 10, FL_HELVETICA });
	do_creation(X, Y);
}

// Delete created items
qsl_design::~qsl_design() {
	delete current_design_;
	delete card_window_;
}

// Load the current design from the settings
void qsl_design::load_values() {
	// Get the settings by creating a qsl_form
	record* record = book_->get_record();
	card_window_ = new Fl_Window(10, 10, "QSL Design");
	current_design_ = new qsl_form(0, 0, record);
	current_design_->position(0, 0);
	current_design_->box(FL_FLAT_BOX);
	current_design_->color(FL_WHITE);
	current_design_->designer(this);
	card_window_->size(current_design_->w(), current_design_->h());
	card_window_->end();
}

//  create the form to edit the design
void qsl_design::create_form(int X, int Y) {
	// Group 1 to define size
	const int XGRP1 = X + EDGE;
	const int YGRP1 = Y + EDGE;
	// Row 1 - 3 radio light buttons
	const int X_111 = XGRP1 + GAP;
	const int X_112 = X_111 + WBUTTON + GAP;
	const int X_113 = X_112 + WBUTTON + GAP;
	const int WGRP11 = X_112 + WBUTTON + GAP - XGRP1;
	const int Y_11 = YGRP1 + GAP + HTEXT;

	// Row 2 to 4 - 3 inputs
	const int X_121 = XGRP1 + WLLABEL + GAP;
	const int X_122 = X_121 + WLLABEL + WBUTTON + GAP;
	const int X_123 = X_122 + WLLABEL + WBUTTON + GAP;
	const int WGRP12 = X_123 + WBUTTON + GAP;
	const int Y_12 = Y_11 + HBUTTON;
	const int Y_13 = Y_12 + HBUTTON;
	const int Y_14 = Y_13 + HBUTTON;
	const int HGRP1 = Y_14 + HBUTTON + GAP - YGRP1;
	const int WGRP1 = max(WGRP11, WGRP12);

	// Group 2 - widget editor
	const int XGRP2 = X + EDGE;
	const int YGRP2 = YGRP1 + HGRP1 + GAP;
	// Row 1 - text input, colour button, font and size browsers (ML input)
	const int X_211 = XGRP2 + GAP;
	const int X_212 = X_211 + WEDIT;
	const int X_213 = X_212 + WRADIO;
	const int X_214 = X_213 + WEDIT;
	const int WGRP2 = X_214+ WBUTTON + GAP - XGRP2;
	const int Y_21 = YGRP2 + GAP + HTEXT;
	const int HGRP2 = Y_21 + max(HBUTTON, HMLIN) + GAP - YGRP2;

	begin();

	// Group 1
	Fl_Group* gp1 = new Fl_Group(XGRP1, YGRP1, WGRP1, HGRP1, "Size parameters");
	gp1->labelfont(FONT);
	gp1->labelsize(FONT_SIZE);
	gp1->box(FL_DOWN_BOX);
	gp1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 - radio buttons
	Fl_Group* radio_gp1 = new Fl_Group(XGRP1, Y_11, WGRP1, HBUTTON);
	radio_gp1->box(FL_NO_BOX);
	Fl_Radio_Light_Button* radio111 = new Fl_Radio_Light_Button(X_111, Y_11, WBUTTON, HBUTTON, "inch");
	radio111->value(current_design_->unit() == qsl_form::INCH);
	radio111->labelfont(FONT);
	radio111->labelsize(FONT_SIZE);
	radio111->selection_color(FL_RED);
	radio111->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio111->callback(cb_radio_dim, (void*)qsl_form::INCH);
	Fl_Radio_Light_Button* radio112 = new Fl_Radio_Light_Button(X_112, Y_11, WBUTTON, HBUTTON, "mm");
	radio112->value(current_design_->unit() == qsl_form::MILLIMETER);
	radio112->labelfont(FONT);
	radio112->labelsize(FONT_SIZE);
	radio112->selection_color(FL_RED);
	radio112->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio112->callback(cb_radio_dim, (void*)qsl_form::MILLIMETER);
	Fl_Radio_Light_Button* radio113 = new Fl_Radio_Light_Button(X_113, Y_11, WBUTTON, HBUTTON, "point");
	radio113->value(current_design_->unit() == qsl_form::POINT);
	radio113->labelfont(FONT);
	radio113->labelsize(FONT_SIZE);
	radio113->selection_color(FL_RED);
	radio113->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio113->callback(cb_radio_dim, (void*)qsl_form::POINT);
	radio_gp1->end();
	// Row 2 - width and height values
	Fl_Value_Input* vip121 = new Fl_Value_Input(X_121, Y_12, WBUTTON, HBUTTON, "Width");
	vip121->value(current_design_->width());
	vip121->labelfont(FONT);
	vip121->labelsize(FONT_SIZE);
	vip121->textfont(FONT);
	vip121->textsize(FONT_SIZE);
	vip121->align(FL_ALIGN_LEFT);
	vip121->callback(cb_vip_sized, (void*)WIDTH);
	vip121->when(FL_WHEN_ENTER_KEY);
	Fl_Value_Input* vip122 = new Fl_Value_Input(X_122, Y_12, WBUTTON, HBUTTON, "Height");
	vip122->value(current_design_->height());
	vip122->labelfont(FONT);
	vip122->labelsize(FONT_SIZE);
	vip122->textfont(FONT);
	vip122->textsize(FONT_SIZE);
	vip122->align(FL_ALIGN_LEFT);
	vip122->callback(cb_vip_sized, (void*)HEIGHT);
	vip122->when(FL_WHEN_ENTER_KEY);
	// Row 3 - TL and TR values
	Fl_Value_Input* vip131 = new Fl_Value_Input(X_121, Y_13, WBUTTON, HBUTTON, "Top Left rows");
	vip131->value(current_design_->set_size(qsl_form::TOP_LEFT));
	vip131->labelfont(FONT);
	vip131->labelsize(FONT_SIZE);
	vip131->textfont(FONT);
	vip131->textsize(FONT_SIZE);
	vip131->align(FL_ALIGN_LEFT);
	vip131->callback(cb_vip_sizeu, (void*)TL_SIZE);
	vip131->when(FL_WHEN_ENTER_KEY);
	Fl_Value_Input* vip132 = new Fl_Value_Input(X_122, Y_13, WBUTTON, HBUTTON, "Top Right rows");
	vip132->value(current_design_->set_size(qsl_form::TOP_RIGHT));
	vip132->labelfont(FONT);
	vip132->labelsize(FONT_SIZE);
	vip132->textfont(FONT);
	vip132->textsize(FONT_SIZE);
	vip132->align(FL_ALIGN_LEFT);
	vip132->callback(cb_vip_sizeu, (void*)TR_SIZE);
	vip132->when(FL_WHEN_ENTER_KEY);
	// Row 4 - Table values
	Fl_Value_Input* vip133 = new Fl_Value_Input(X_123, Y_13, WBUTTON, HBUTTON, "Table rows");
	vip133->value(current_design_->table_rows());
	vip133->labelfont(FONT);
	vip133->labelsize(FONT_SIZE);
	vip133->textfont(FONT);
	vip133->textsize(FONT_SIZE);
	vip133->align(FL_ALIGN_LEFT);
	vip133->callback(cb_vip_sizeu, (void*)TAB_ROWS);
	vip133->when(FL_WHEN_ENTER_KEY);
	Fl_Value_Input* vip141 = new Fl_Value_Input(X_121, Y_14, WBUTTON, HBUTTON, "Table cols");
	vip141->value(current_design_->table_cols());
	vip141->labelfont(FONT);
	vip141->labelsize(FONT_SIZE);
	vip141->textfont(FONT);
	vip141->textsize(FONT_SIZE);
	vip141->align(FL_ALIGN_LEFT);
	vip141->callback(cb_vip_sizeu, (void*)TAB_COLS);
	vip141->when(FL_WHEN_ENTER_KEY);
	// Row 3 - BL and BR values
	Fl_Value_Input* vip142 = new Fl_Value_Input(X_122, Y_14, WBUTTON, HBUTTON, "Bottom Left rows");
	vip142->value(current_design_->set_size(qsl_form::BOTTOM_LEFT));
	vip142->labelfont(FONT);
	vip142->labelsize(FONT_SIZE);
	vip142->textfont(FONT);
	vip142->textsize(FONT_SIZE);
	vip142->align(FL_ALIGN_LEFT);
	vip142->callback(cb_vip_sizeu, (void*)BL_SIZE);
	vip142->when(FL_WHEN_ENTER_KEY);
	Fl_Value_Input* vip143 = new Fl_Value_Input(X_123, Y_14, WBUTTON, HBUTTON, "Bottom Right rows");
	vip143->value(current_design_->set_size(qsl_form::BOTTOM_RIGHT));
	vip143->labelfont(FONT);
	vip143->labelsize(FONT_SIZE);
	vip143->textfont(FONT);
	vip143->textsize(FONT_SIZE);
	vip143->align(FL_ALIGN_LEFT);
	vip143->callback(cb_vip_sizeu, (void*)BR_SIZE);
	vip143->when(FL_WHEN_ENTER_KEY);
	gp1->end();

	// Group 2 - design information
	Fl_Group* gp2 = new Fl_Group(XGRP2, YGRP2, WGRP2, HGRP2, "Design parameters");
	gp2->labelfont(FONT);
	gp2->labelsize(FONT_SIZE);
	gp2->box(FL_DOWN_BOX);
	gp2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Row 1 - text value, colour selector
	ip_text_ = new intl_input(X_211, Y_21, WEDIT, HBUTTON, "Text");
	ip_text_->value(current_data_->text.c_str());
	ip_text_->labelfont(FONT);
	ip_text_->labelsize(FONT_SIZE);
	ip_text_->textfont(current_data_->font);
	ip_text_->textsize(FONT_SIZE);
	ip_text_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip_text_->textcolor(current_data_->colour);
	ip_text_->callback(cb_ip_text);
	ip_text_->when(FL_WHEN_ENTER_KEY);
	bn_colour_ = new Fl_Button(X_212, Y_21, WRADIO, HBUTTON, "Colour");
	bn_colour_->labelfont(FONT);
	bn_colour_->labelsize(FONT_SIZE);
	bn_colour_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn_colour_->callback(cb_bn_colour);
	bn_colour_->color(current_data_->colour);
	// Row 2 - font an size browsers
	br_font_ = new Fl_Hold_Browser(X_213, Y_21, WEDIT, HMLIN, "Font");
	br_font_->labelfont(FONT);
	br_font_->labelsize(FONT_SIZE);
	br_font_->textsize(FONT_SIZE);
	br_font_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br_font_->callback(cb_br_font);
	br_size_ = new Fl_Hold_Browser(X_214, Y_21, WBUTTON, HMLIN, "Size");
	br_size_->textsize(FONT_SIZE);
	br_size_->textfont(FONT);
	br_size_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br_size_->callback(cb_br_size);
	populate_font();
	populate_size();
	gp2->end();


	// Add the design here
	if (((Fl_Tabs*)parent())->value() == this) {
		card_window_->show();
	}
	else {
		card_window_->hide();
	}

	end();
	show();
}

// Save the new design
void qsl_design::save_values() {
	current_design_->save_data();
}

// This is required by page_dialog and intended to enable/dsiable widgets depending on data values
// using it here to change the appearance of the dialog 
void qsl_design::enable_widgets() {
	ip_text_->textcolor(current_data_->colour);
	ip_text_->value(current_data_->text.c_str());
	bn_colour_->color(current_data_->colour);
	br_font_->value(current_data_->font + 1);
	populate_size();
	redraw();
}

// Call back to read a value input into a double
void qsl_design::cb_vip_sized(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	float value = (float)((Fl_Value_Input*)w)->value();
	switch ((size_object)(long)v) {
	case WIDTH:
		that->current_design_->width(value);
		break;
	case HEIGHT:
		that->current_design_->height(value);
		break;
	}
		that->card_window_->size(that->current_design_->w(), that->current_design_->h());
}

// Call back to read a value input into an unsigned
void qsl_design::cb_vip_sizeu(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	unsigned int value = (unsigned int)((Fl_Value_Input*)w)->value();
	switch ((size_object)(long)v) {
	case TL_SIZE:
		that->current_design_->resize_set(qsl_form::TOP_LEFT, value);
		break;
	case TR_SIZE:
		that->current_design_->resize_set(qsl_form::TOP_RIGHT, value);
		break;
	case BL_SIZE:
		that->current_design_->resize_set(qsl_form::BOTTOM_LEFT, value);
		break;
	case BR_SIZE:
		that->current_design_->resize_set(qsl_form::BOTTOM_RIGHT, value);
		break;
	case TAB_ROWS:
		that->current_design_->resize_table(value, -1);
		break;
	case TAB_COLS:
		that->current_design_->resize_table(-1, value);
		break;
	}
}

// Callback when text value changed 
void qsl_design::cb_ip_text(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	string value;
	cb_value<intl_input, string>(w, &value);
	that->current_design_->update_text(that->current_data_, value);
}

// Callback when font value changed
void qsl_design::cb_br_font(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	Fl_Hold_Browser* font_br = (Fl_Hold_Browser*)w;
	Fl_Font value = (Fl_Font)font_br->value() - 1;
	that->current_design_->update_font(that->current_data_, value);
	that->populate_size();
}

// Callback when font size changed
void qsl_design::cb_br_size(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	Fl_Hold_Browser* size_br = (Fl_Hold_Browser*)w;
	int line = size_br->value();
	string value = size_br->text(line);
	that->current_design_->update_size(that->current_data_, stoi(value));
}

// Call back when colour button changed
void qsl_design::cb_bn_colour(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	Fl_Color value = fl_show_colormap(that->current_data_->colour);
	that->current_design_->update_colour(that->current_data_, value);
	that->enable_widgets();
}

// Call back when a radio button is pressed - v indicates which button
void qsl_design::cb_radio_dim(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	that->current_design_->unit((qsl_form::dim_unit)(long)v);
	that->card_window_->size(that->current_design_->w(), that->current_design_->h());
}

// Callback if the design is clicked - w should be the button clicked
void qsl_design::display_design_data(Fl_Button* widget, qsl_form::qsl_widget* data) {
	current_widget_ = widget;
	// Copy the widget data
	current_data_ = data;
	enable_widgets();
}

// Populate the font browser
void qsl_design::populate_font() {
	br_font_->clear();
	// Get only ISO9958-1 fonts
	int num_fonts = Fl::set_fonts(nullptr);
	for (int i = 0; i < num_fonts; i++) {
		// Contains any combination of FL_BOLD and FL_ITALIC
		int attr = 0;
		const char* name = Fl::get_font_name(Fl_Font(i), &attr);
		char buffer[128];
		// display in the named font
		sprintf(buffer, "@F%d@.%s", i, name);
		br_font_->add(buffer);
	}
}

// Populate the size browser
void qsl_design::populate_size() {
	br_size_->clear();
	// To receive the array of sizes
	int* sizes;
	int num_sizes = Fl::get_font_sizes(current_data_->font, sizes);
	if (num_sizes) {
		// We have some sizes
		if (sizes[0] == 0) {
			// Scaleable font - so any size available 
			for (int i = 1; i < max(64, sizes[num_sizes - 1]); i++) {
				char buff[20];
				sprintf(buff, "%d", i);
				br_size_->add(buff);
			}
			br_size_->value(current_data_->font_size);
		}
		else {
			// Only list available sizes
			int select = 0;
			for (int i = 0; i < num_sizes; i++) {
				// while the current value is less than required up the select value
				if (sizes[i] < current_data_->font_size) {
					select = i;
				}
				char buff[20];
				sprintf(buff, "%d", sizes[i]);
				br_size_->add(buff);
			}
			br_size_->value(select);
		}
	}
}

// Handle  the show events - hide or show the window where the design is being displayed
int qsl_design::handle(int event) {
	switch (event) {
	case FL_SHOW:
		card_window_->show();
		break;
	case FL_HIDE:
		card_window_->hide();
		break;
	}
	return page_dialog::handle(event);
}
#include "qsl_design.h"
#include "qsl_form.h"
#include "record.h"
#include "book.h"
#include "drawing.h"
#include "../zzalib/callback.h"
#include "../zzalib/utils.h"
#include "settings.h"
#include "intl_widgets.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Tabs.H>

using namespace zzalog;
using namespace zzalib;

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
	, address_(nullptr)
	, num_lines_(0)
	, include_address_label_(false)
	, show_address_(false)
	, font_add_(FONT)
{
	current_data_ = new qsl_form::qsl_widget({ "Text here...", FL_BLACK, 10, FL_HELVETICA });
	do_creation(X, Y);
}

// Delete created items
qsl_design::~qsl_design() {
	delete current_design_;
	delete card_window_;
	for (unsigned int i = 0; i < num_lines_; i++) {
		delete address_[i];
	}
}

// Load the current design from the settings
void qsl_design::load_values() {
	// Get the settings by creating a qsl_form
	record* record = book_->get_record();
	card_window_ = new Fl_Window(10, 10, "QSL Design");
	current_design_ = new qsl_form(0, 0, &record, 1);
	current_design_->position(0, 0);
	current_design_->box(FL_FLAT_BOX);
	current_design_->color(FL_WHITE);
	current_design_->designer(this);
	bn_address_ = new Fl_Button(current_design_->x(), current_design_->y(), current_design_->w(), current_design_->h());
	bn_address_->box(FL_NO_BOX);
	bn_address_->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	bn_address_->color(FL_WHITE);
	bn_address_->hide();
	card_window_->size(current_design_->w(), current_design_->h());
	card_window_->end();
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	int temp;
	qsl_settings.get("Print Label", temp, (int)false);
	include_address_label_ = temp;
	qsl_settings.get("Number Rows", num_rows_, 4);
	qsl_settings.get("Number Columns", num_cols_, 2);
	qsl_settings.get("Column Width", col_width_, 101.6);
	qsl_settings.get("Row Height", row_height_, 67.7);
	qsl_settings.get("First Row", row_top_, 12.9);
	qsl_settings.get("First Column", col_left_, 4.6);
	Fl_Preferences add_settings(qsl_settings, "Address");
	add_settings.get("Font", (int)font_add_, FONT);
	add_settings.get("Size", size_add_, FONT_SIZE);
	Fl_Preferences text_settings(add_settings, "Text");
	num_lines_ = text_settings.entries();
	address_ = new char* [num_lines_];
	for (unsigned int i = 0; i < num_lines_; i++) {
		char line_num[3];
		snprintf(line_num, 3, "%02d", i);
		text_settings.get(line_num, address_[i], "");
	}
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
	const int WGRP11 = X_113 + WBUTTON + GAP - XGRP1;
	const int Y_11 = YGRP1 + HTEXT;

	// Row 2 to 4 - 3 inputs
	const int X_121 = XGRP1 + WLABEL;
	const int X_122 = X_121 + WLABEL + WBUTTON;
	const int X_123 = X_122 + WLABEL + WBUTTON;
	const int X_124 = X_123 + WLABEL + WBUTTON;
	const int WGRP12 = X_124 + WBUTTON + GAP;
	const int Y_12 = Y_11 + HBUTTON + GAP;
	const int Y_12A = Y_12 + HBUTTON;
	const int Y_13 = Y_12A + HBUTTON;
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

	// Group 3 - address editor
	const int XGRP3 = X + EDGE;
	const int YGRP3 = YGRP2 + HGRP2 + GAP;
	// Row 1 - Enable
	const int X_311 = XGRP3 + GAP;
	const int X_312 = X_311 + WBUTTON + GAP;
	const int Y_31 = YGRP3 + GAP + HTEXT;

	// Row 2 - editor, font and size brows
	const int X_321 = XGRP3 + GAP;
	const int X_322 = X_321 + WEDIT;
	const int X_323 = X_322 + WEDIT;
	const int WGRP3 = X_323 + WBUTTON + GAP - XGRP2;
	const int Y_32 = Y_31 + HBUTTON;
	const int HGRP3 = Y_32 + (2 * HMLIN) + GAP- YGRP3;

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
	radio111->tooltip("Measurements in inches");
	Fl_Radio_Light_Button* radio112 = new Fl_Radio_Light_Button(X_112, Y_11, WBUTTON, HBUTTON, "mm");
	radio112->value(current_design_->unit() == qsl_form::MILLIMETER);
	radio112->labelfont(FONT);
	radio112->labelsize(FONT_SIZE);
	radio112->selection_color(FL_RED);
	radio112->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio112->callback(cb_radio_dim, (void*)qsl_form::MILLIMETER);
	radio112->tooltip("Measurements in millimetres");
	Fl_Radio_Light_Button* radio113 = new Fl_Radio_Light_Button(X_113, Y_11, WBUTTON, HBUTTON, "point");
	radio113->value(current_design_->unit() == qsl_form::POINT);
	radio113->labelfont(FONT);
	radio113->labelsize(FONT_SIZE);
	radio113->selection_color(FL_RED);
	radio113->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio113->callback(cb_radio_dim, (void*)qsl_form::POINT);
	radio113->tooltip("Measurements in printer points");
	radio_gp1->end();
	// Row 2 - width parameters
	Fl_Value_Input* vip121 = new Fl_Value_Input(X_121, Y_12, WBUTTON, HBUTTON, "Columns");
	vip121->value(num_cols_);
	vip121->labelfont(FONT);
	vip121->labelsize(FONT_SIZE);
	vip121->textfont(FONT);
	vip121->textsize(FONT_SIZE);
	vip121->align(FL_ALIGN_LEFT);
	vip121->callback(cb_value<Fl_Value_Input, int>, &num_cols_);
	vip121->when(FL_WHEN_ENTER_KEY);
	vip121->tooltip("Please specify the number of columns when printing");
	Fl_Value_Input* vip122 = new Fl_Value_Input(X_122, Y_12, WBUTTON, HBUTTON, "Width");
	vip122->value(current_design_->width());
	vip122->labelfont(FONT);
	vip122->labelsize(FONT_SIZE);
	vip122->textfont(FONT);
	vip122->textsize(FONT_SIZE);
	vip122->align(FL_ALIGN_LEFT);
	vip122->callback(cb_vip_sized, (void*)WIDTH);
	vip122->when(FL_WHEN_ENTER_KEY);
	vip122->tooltip("Please specify the width of the label in the selected unit");
	Fl_Value_Input* vip123 = new Fl_Value_Input(X_123, Y_12, WBUTTON, HBUTTON, "Position");
	vip123->value(col_left_);
	vip123->labelfont(FONT);
	vip123->labelsize(FONT_SIZE);
	vip123->textfont(FONT);
	vip123->textsize(FONT_SIZE);
	vip123->align(FL_ALIGN_LEFT);
	vip123->callback(cb_value<Fl_Value_Input, double>, &col_left_);
	vip123->when(FL_WHEN_ENTER_KEY);
	vip123->tooltip("Please specify the position of the first column");
	Fl_Value_Input* vip124 = new Fl_Value_Input(X_124, Y_12, WBUTTON, HBUTTON, "Spacing");
	vip124->value(col_width_);
	vip124->labelfont(FONT);
	vip124->labelsize(FONT_SIZE);
	vip124->textfont(FONT);
	vip124->textsize(FONT_SIZE);
	vip124->align(FL_ALIGN_LEFT);
	vip124->callback(cb_value<Fl_Value_Input, double>, &col_width_);
	vip124->when(FL_WHEN_ENTER_KEY);
	vip124->tooltip("Please specify the spacing between columns");
	// Row 2A - height parameters
	Fl_Value_Input* vip12A1 = new Fl_Value_Input(X_121, Y_12A, WBUTTON, HBUTTON, "Rows");
	vip12A1->value(num_cols_);
	vip12A1->labelfont(FONT);
	vip12A1->labelsize(FONT_SIZE);
	vip12A1->textfont(FONT);
	vip12A1->textsize(FONT_SIZE);
	vip12A1->align(FL_ALIGN_LEFT);
	vip12A1->callback(cb_value<Fl_Value_Input, int>, &num_cols_);
	vip12A1->when(FL_WHEN_ENTER_KEY);
	vip12A1->tooltip("Please specify the number of rows when printing");
	Fl_Value_Input* vip12A2 = new Fl_Value_Input(X_122, Y_12A, WBUTTON, HBUTTON, "Height");
	vip12A2->value(current_design_->height());
	vip12A2->labelfont(FONT);
	vip12A2->labelsize(FONT_SIZE);
	vip12A2->textfont(FONT);
	vip12A2->textsize(FONT_SIZE);
	vip12A2->align(FL_ALIGN_LEFT);
	vip12A2->callback(cb_vip_sized, (void*)HEIGHT);
	vip12A2->when(FL_WHEN_ENTER_KEY);
	vip12A2->tooltip("Please specify the width of the label in the selected label");
	Fl_Value_Input* vip12A3 = new Fl_Value_Input(X_123, Y_12A, WBUTTON, HBUTTON, "Position");
	vip12A3->value(row_top_);
	vip12A3->labelfont(FONT);
	vip12A3->labelsize(FONT_SIZE);
	vip12A3->textfont(FONT);
	vip12A3->textsize(FONT_SIZE);
	vip12A3->align(FL_ALIGN_LEFT);
	vip12A3->callback(cb_value<Fl_Value_Input, double>, &row_top_);
	vip12A3->when(FL_WHEN_ENTER_KEY);
	vip12A3->tooltip("Please specify the position of the first row");
	Fl_Value_Input* vip12A4 = new Fl_Value_Input(X_124, Y_12A, WBUTTON, HBUTTON, "Spacing");
	vip12A4->value(row_height_);
	vip12A4->labelfont(FONT);
	vip12A4->labelsize(FONT_SIZE);
	vip12A4->textfont(FONT);
	vip12A4->textsize(FONT_SIZE);
	vip12A4->align(FL_ALIGN_LEFT);
	vip12A4->callback(cb_value<Fl_Value_Input, double>, &row_height_);
	vip12A4->when(FL_WHEN_ENTER_KEY);
	vip12A4->tooltip("Please specify the spacing between rows");
	// Row 3 - TL and TR values
	Fl_Value_Input* vip131 = new Fl_Value_Input(X_121, Y_13, WBUTTON, HBUTTON, "TL Rows");
	vip131->value(current_design_->set_size(qsl_form::TOP_LEFT));
	vip131->labelfont(FONT);
	vip131->labelsize(FONT_SIZE);
	vip131->textfont(FONT);
	vip131->textsize(FONT_SIZE);
	vip131->align(FL_ALIGN_LEFT);
	vip131->callback(cb_vip_sizeu, (void*)TL_SIZE);
	vip131->when(FL_WHEN_ENTER_KEY);
	vip131->tooltip("Please specify the number of lines in the text at the top-left quadrant of the label");
	Fl_Value_Input* vip132 = new Fl_Value_Input(X_122, Y_13, WBUTTON, HBUTTON, "TR Rows");
	vip132->value(current_design_->set_size(qsl_form::TOP_RIGHT));
	vip132->labelfont(FONT);
	vip132->labelsize(FONT_SIZE);
	vip132->textfont(FONT);
	vip132->textsize(FONT_SIZE);
	vip132->align(FL_ALIGN_LEFT);
	vip132->callback(cb_vip_sizeu, (void*)TR_SIZE);
	vip132->when(FL_WHEN_ENTER_KEY);
	vip132->tooltip("Please specify the number of lines in the text at the top-right quadrant of the label");
	// Row 4 - Table values
	Fl_Value_Input* vip133 = new Fl_Value_Input(X_123, Y_13, WBUTTON, HBUTTON, "Tab. rows");
	vip133->value(current_design_->table_rows());
	vip133->labelfont(FONT);
	vip133->labelsize(FONT_SIZE);
	vip133->textfont(FONT);
	vip133->textsize(FONT_SIZE);
	vip133->align(FL_ALIGN_LEFT);
	vip133->callback(cb_vip_sizeu, (void*)TAB_ROWS);
	vip133->when(FL_WHEN_ENTER_KEY);
	vip133->tooltip("Please specify the number of rows in the central table");
	Fl_Value_Input* vip141 = new Fl_Value_Input(X_121, Y_14, WBUTTON, HBUTTON, "Tab. cols");
	vip141->value(current_design_->table_cols());
	vip141->labelfont(FONT);
	vip141->labelsize(FONT_SIZE);
	vip141->textfont(FONT);
	vip141->textsize(FONT_SIZE);
	vip141->align(FL_ALIGN_LEFT);
	vip141->callback(cb_vip_sizeu, (void*)TAB_COLS);
	vip141->when(FL_WHEN_ENTER_KEY);
	vip141->tooltip("Please specify the number of columns in the central table");
	// Row 3 - BL and BR values
	Fl_Value_Input* vip142 = new Fl_Value_Input(X_122, Y_14, WBUTTON, HBUTTON, "BL Rows");
	vip142->value(current_design_->set_size(qsl_form::BOTTOM_LEFT));
	vip142->labelfont(FONT);
	vip142->labelsize(FONT_SIZE);
	vip142->textfont(FONT);
	vip142->textsize(FONT_SIZE);
	vip142->align(FL_ALIGN_LEFT);
	vip142->callback(cb_vip_sizeu, (void*)BL_SIZE);
	vip142->when(FL_WHEN_ENTER_KEY);
	vip142->tooltip("Please specify the number of lines in the text at the bottom-left quadrant of the label");
	Fl_Value_Input* vip143 = new Fl_Value_Input(X_123, Y_14, WBUTTON, HBUTTON, "BR rows");
	vip143->value(current_design_->set_size(qsl_form::BOTTOM_RIGHT));
	vip143->labelfont(FONT);
	vip143->labelsize(FONT_SIZE);
	vip143->textfont(FONT);
	vip143->textsize(FONT_SIZE);
	vip143->align(FL_ALIGN_LEFT);
	vip143->callback(cb_vip_sizeu, (void*)BR_SIZE);
	vip143->when(FL_WHEN_ENTER_KEY);
	vip143->tooltip("Please specify the number of lines in the text at the bottom-right quadrant of the label");
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
	ip_text_->tooltip("Please specify the text to appear in the selected line");
	bn_colour_ = new Fl_Button(X_212, Y_21, WRADIO, HBUTTON, "Colour");
	bn_colour_->labelfont(FONT);
	bn_colour_->labelsize(FONT_SIZE);
	bn_colour_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn_colour_->callback(cb_bn_colour);
	bn_colour_->color(current_data_->colour);
	bn_colour_->tooltip("Please specify the colour for the text in the selected line");
	// Row 2 - font an size browsers
	br_font_ = new Fl_Hold_Browser(X_213, Y_21, WEDIT, HMLIN, "Font");
	br_font_->labelfont(FONT);
	br_font_->labelsize(FONT_SIZE);
	br_font_->textsize(FONT_SIZE);
	br_font_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br_font_->callback(cb_br_font);
	br_font_->tooltip("Please specify the font to use for the selected line");
	br_size_ = new Fl_Hold_Browser(X_214, Y_21, WBUTTON, HMLIN, "Size");
	br_size_->textsize(FONT_SIZE);
	br_size_->textfont(FONT);
	br_size_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br_size_->callback(cb_br_size);
	br_size_->tooltip("Please specify the size of text for the selected line");
	populate_font(br_font_);
	populate_size(br_size_);
	gp2->end();

	Fl_Group* gp3 = new Fl_Group(XGRP3, YGRP3, WGRP3, HGRP3, "Address Label");
	gp3->labelfont(FONT);
	gp3->labelsize(FONT_SIZE);
	gp3->box(FL_DOWN_BOX);
	gp3->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Row 1 position 1 - Enable button
	Fl_Light_Button* bn_use_add = new Fl_Light_Button(X_311, Y_31, WBUTTON, HBUTTON, "Enable");
	bn_use_add->labelfont(FONT);
	bn_use_add->labelsize(FONT_SIZE);
	bn_use_add->align(FL_ALIGN_INSIDE);
	bn_use_add->callback(cb_bn_address, &include_address_label_);
	bn_use_add->value(include_address_label_);
	bn_use_add->tooltip("Print the address label after printing all QSL cards");
	// Row 1 position 2 - Show/Hide button
	bn_show_add_ = new Fl_Light_Button(X_312, Y_31, WBUTTON, HBUTTON, "Show");
	bn_show_add_->labelfont(FONT);
	bn_show_add_->labelsize(FONT_SIZE);
	bn_show_add_->align(FL_ALIGN_INSIDE);
	bn_show_add_->callback(cb_bn_show_add, &show_address_);
	bn_show_add_->value(show_address_);
	bn_show_add_->label(show_address_ ? "Hide" : "Show");
	// Row 2 position 1 - text editor
	editor_ = new intl_editor(X_321, Y_32, WEDIT, HMLIN * 2);
	Fl_Text_Buffer* buffer = new Fl_Text_Buffer(1024);
	for (unsigned int i = 0; i < num_lines_; i++) {
		buffer->append(address_[i]);
		buffer->append("\n");
	}
	editor_->buffer(buffer);
	editor_->textfont(FONT);
	editor_->textsize(FONT_SIZE);
	// Row 2 position 2 - font chooser
	br_font_add_ = new Fl_Hold_Browser(X_213, Y_32, WEDIT, HMLIN, "Font");
	br_font_add_->labelfont(FONT);
	br_font_add_->labelsize(FONT_SIZE);
	br_font_add_->textsize(FONT_SIZE);
	br_font_add_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br_font_add_->callback(cb_br_font_add);
	br_font_add_->tooltip("Please specify the font to use for the text");
	br_size_add_ = new Fl_Hold_Browser(X_214, Y_32, WBUTTON, HMLIN, "Size");
	br_size_add_->textsize(FONT_SIZE);
	br_size_add_->textfont(FONT);
	br_size_add_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br_size_add_->callback(cb_br_size_add);
	br_size_add_->tooltip("Please specify the size of text for the text");
	populate_font(br_font_add_);
	br_font_add_->value(font_add_ + 1);
	populate_size(br_size_add_);

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
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	qsl_settings.set("Print Label", include_address_label_);
	qsl_settings.set("Number Rows", num_rows_);
	qsl_settings.set("Number Columns", num_cols_);
	qsl_settings.set("Column Width", col_width_);
	qsl_settings.set("Row Height", row_height_);
	qsl_settings.set("First Row", row_top_);
	qsl_settings.set("First Column", col_left_);
	// REmove existing address settings
	Fl_Preferences add_settings(qsl_settings, "Address");
	add_settings.set("Font", (int)font_add_);
	add_settings.set("Size", size_add_);
	Fl_Preferences text_settings(add_settings, "Text");
	text_settings.clear();
	Fl_Text_Buffer* buffer = editor_->buffer();
	// Character and line positions
	int pos = 0;
	int line_no = 0;
	// Write each line in the text buffer to the settings
	while (pos < buffer->length()) {
		char line_num[3];
		snprintf(line_num, 3, "%02d", line_no);
		char* line = buffer->line_text(pos);
		text_settings.set(line_num, line);
		// Incrememnt position and line number
		pos = buffer->line_end(pos) + 1;
		line_no++;
	}

}

// This is required by page_dialog and intended to enable/dsiable widgets depending on data values
// using it here to change the appearance of the dialog 
void qsl_design::enable_widgets() {
	ip_text_->textcolor(current_data_->colour);
	ip_text_->value(current_data_->text.c_str());
	bn_colour_->color(current_data_->colour);
	br_font_->value(current_data_->font + 1);
	populate_size(br_size_);
	populate_font(br_font_);
	// Enable/Disable address widgets
	if (include_address_label_) {
		bn_show_add_->activate();
		editor_->activate();
		br_font_add_->activate();
		br_size_add_->activate();
	}
	else {
		bn_show_add_->deactivate();
		editor_->deactivate();
		br_font_add_->deactivate();
		br_size_add_->deactivate();
	}
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
	that->populate_size(that->br_size_);
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

// Change to the font browser
void qsl_design::cb_br_font_add(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	Fl_Hold_Browser* font_br = (Fl_Hold_Browser*)w;
	that->font_add_ = (Fl_Font)font_br->value() - 1;
	if (that->show_address_) that->redraw_address();
}

// change to the font size
void qsl_design::cb_br_size_add(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	Fl_Hold_Browser* size_br = (Fl_Hold_Browser*)w;
	int line = size_br->value();
	string value = size_br->text(line);
	that->size_add_ = stoi(value);
	if (that->show_address_) that->redraw_address();
}

// Set address label parameters
void qsl_design::cb_bn_address(Fl_Widget* w, void* v) {
	cb_value<Fl_Light_Button, bool>(w, v);
	qsl_design* that = ancestor_view<qsl_design>(w);
	that->enable_widgets();
	if (that->include_address_label_ && that->show_address_) that->redraw_address();
}

// Show/Hide address label in window
void qsl_design::cb_bn_show_add(Fl_Widget* w, void* v) {
	cb_value<Fl_Light_Button, bool>(w, v);
	qsl_design* that = ancestor_view<qsl_design>(w);
	// Which do we show?
	if (that->show_address_) {
		w->label("Hide");
		that->current_design_->hide();
		that->bn_address_->show();
		that->redraw_address();
	}
	else {
		w->label("Show");
		that->bn_address_->hide();
		that->current_design_->show();
	}
	that->card_window_->redraw();
}


// Callback if the design is clicked - w should be the button clicked
void qsl_design::display_design_data(Fl_Button* widget, qsl_form::qsl_widget* data) {
	current_widget_ = widget;
	// Copy the widget data
	current_data_ = data;
	enable_widgets();
}

// Populate the font browser
void qsl_design::populate_font(Fl_Hold_Browser* br) {
	br->clear();
	// Get only ISO9958-1 fonts
	int num_fonts = Fl::set_fonts(nullptr);
	for (int i = 0; i < num_fonts; i++) {
		// Contains any combination of FL_BOLD and FL_ITALIC
		int attr = 0;
		const char* name = Fl::get_font_name(Fl_Font(i), &attr);
		char buffer[128];
		// display in the named font
		sprintf(buffer, "@F%d@.%s", i, name);
		br->add(buffer);
	}
}

// Populate the size browser
void qsl_design::populate_size(Fl_Hold_Browser* br) {
	br->clear();
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
				br->add(buff);
			}
			br->value(current_data_->font_size);
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
				br->add(buff);
			}
			br->value(select);
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

// Redraw the address label
void qsl_design::redraw_address() {
	bn_address_->labelfont(font_add_);
	bn_address_->labelsize(size_add_);
	bn_address_->label(editor_->buffer()->text());
	card_window_->redraw();
}
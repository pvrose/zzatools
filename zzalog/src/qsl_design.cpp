#include "qsl_design.h"
#include "qsl_form.h"
#include "callback.h"
#include "utils.h"
#include "settings.h"
#include "qso_manager.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Tabs.H>




extern Fl_Preferences* settings_;
extern qso_manager* qso_manager_;

// Constructor - standard page dialog
qsl_design::qsl_design(int X, int Y, int W, int H, const char* label) :
	Fl_Window(X, Y, W, H, label)
{
	callsign_ = label;
	string new_label = "QSL Design parameters: " + callsign_;
	copy_label(new_label.c_str());

	load_values();
	create_form(0, 0);
}

// Delete created items
qsl_design::~qsl_design() {
}


// Load the current design from the settings
void qsl_design::load_values() {
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	Fl_Preferences call_settings(qsl_settings, callsign_.c_str());
		
	call_settings.get("Unit", (int&)unit_, (int)qsl_form::MILLIMETER);
	call_settings.get("Width", width_, 99.1);
	call_settings.get("Height", height_, 66.7);
	call_settings.get("Number Rows", num_rows_, 4);
	call_settings.get("Number Columns", num_cols_, 2);
	call_settings.get("Column Width", col_width_, 101.6);
	call_settings.get("Row Height", row_height_, 67.7);
	call_settings.get("First Row", row_top_, 12.9);
	call_settings.get("First Column", col_left_, 4.6);
	call_settings.get("Max QSOs per Card", number_qsos_, 1);

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

	// Row 2 to 3 - 4 inputs
	const int X_121 = XGRP1 + WLABEL;
	const int X_122 = X_121 + WLABEL + WBUTTON;
	const int X_123 = X_122 + WLABEL + WBUTTON;
	const int X_124 = X_123 + WLABEL + WBUTTON;
	const int WGRP12 = X_124 + WBUTTON + GAP;
	const int Y_12 = Y_11 + HBUTTON + GAP;
	const int Y_13 = Y_12 + HBUTTON;

	// Row 4 - 1 input
	const int X_141 = XGRP1 + WLABEL;
	const int WGRP13 = X_141 + WBUTTON + GAP;
	const int Y_14 = Y_13 + HBUTTON + GAP;

	// group 1 size
	const int HGRP1 = Y_14 + HBUTTON + GAP - YGRP1;
	const int WGRP1 = max(max(WGRP11, WGRP12), WGRP13);

	// Group 2
	const int XGRP2 = XGRP1;
	const int YGRP2 = YGRP1 + HGRP1 + GAP;

	// Row 1 - 2 buttons
	const int X_211 = XGRP1 + WLABEL;
	const int X_212 = X_211 + GAP + WBUTTON;
	const int WGRP21 = X_212 + WBUTTON + GAP;
	const int Y_21 = YGRP2;

	// Group 2
	const int HGRP2 = Y_21 + HBUTTON + GAP;
	const int WGRP2 = WGRP21;

	// Overall
	const int HALL = HGRP2 + GAP;
	const int WALL = max(WGRP1, WGRP2);

	begin();

	// Group 1
	Fl_Group* gp1 = new Fl_Group(XGRP1, YGRP1, WGRP1, HGRP1, "Size parameters");
	gp1->labelfont(FONT);
	gp1->labelsize(FONT_SIZE);
	gp1->box(FL_DOWN_BOX);
	gp1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 - radio buttons to select size units
	Fl_Group* radio_gp1 = new Fl_Group(XGRP1, Y_11, WGRP1, HBUTTON);
	radio_gp1->box(FL_NO_BOX);
	// Radio to select inches
	Fl_Radio_Light_Button* radio111 = new Fl_Radio_Light_Button(X_111, Y_11, WBUTTON, HBUTTON, "inch");
	radio111->value(unit_ == qsl_form::INCH);
	radio111->labelfont(FONT);
	radio111->labelsize(FONT_SIZE);
	radio111->selection_color(FL_RED);
	radio111->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio111->callback(cb_radio_dim, (void*)qsl_form::INCH);
	radio111->tooltip("Measurements in inches");
	// Radio to select millimetres
	Fl_Radio_Light_Button* radio112 = new Fl_Radio_Light_Button(X_112, Y_11, WBUTTON, HBUTTON, "mm");
	radio112->value(unit_ == qsl_form::MILLIMETER);
	radio112->labelfont(FONT);
	radio112->labelsize(FONT_SIZE);
	radio112->selection_color(FL_RED);
	radio112->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio112->callback(cb_radio_dim, (void*)qsl_form::MILLIMETER);
	radio112->tooltip("Measurements in millimetres");
	// Radio to select points
	Fl_Radio_Light_Button* radio113 = new Fl_Radio_Light_Button(X_113, Y_11, WBUTTON, HBUTTON, "point");
	radio113->value(unit_ == qsl_form::POINT);
	radio113->labelfont(FONT);
	radio113->labelsize(FONT_SIZE);
	radio113->selection_color(FL_RED);
	radio113->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	radio113->callback(cb_radio_dim, (void*)qsl_form::POINT);
	radio113->tooltip("Measurements in printer points");
	radio_gp1->end();
	// Row 2 - width parameters
	// Input to set number of columns to print
	Fl_Value_Input* vip121 = new Fl_Value_Input(X_121, Y_12, WBUTTON, HBUTTON, "Columns");
	vip121->value(num_cols_);
	vip121->labelfont(FONT);
	vip121->labelsize(FONT_SIZE);
	vip121->textfont(FONT);
	vip121->textsize(FONT_SIZE);
	vip121->align(FL_ALIGN_LEFT);
	vip121->callback(cb_value<Fl_Value_Input, int>, &num_cols_);
//	vip121->when(FL_WHEN_ENTER_KEY);
	vip121->tooltip("Please specify the number of columns when printing");
	// Input to specify the width of the label
	Fl_Value_Input* vip122 = new Fl_Value_Input(X_122, Y_12, WBUTTON, HBUTTON, "Width");
	vip122->value(width_);
	vip122->labelfont(FONT);
	vip122->labelsize(FONT_SIZE);
	vip122->textfont(FONT);
	vip122->textsize(FONT_SIZE);
	vip122->align(FL_ALIGN_LEFT);
	vip122->callback(cb_value<Fl_Value_Input, double>, &width_);
//	vip122->when(FL_WHEN_ENTER_KEY);
	vip122->tooltip("Please specify the width of the label in the selected unit");
	// Input to specify the position ofthe left edge of culmn 1
	Fl_Value_Input* vip123 = new Fl_Value_Input(X_123, Y_12, WBUTTON, HBUTTON, "Position");
	vip123->value(col_left_);
	vip123->labelfont(FONT);
	vip123->labelsize(FONT_SIZE);
	vip123->textfont(FONT);
	vip123->textsize(FONT_SIZE);
	vip123->align(FL_ALIGN_LEFT);
	vip123->callback(cb_value<Fl_Value_Input, double>, &col_left_);
//	vip123->when(FL_WHEN_ENTER_KEY);
	vip123->tooltip("Please specify the position of the first column");
	// Input to specify the gap between columns
	Fl_Value_Input* vip124 = new Fl_Value_Input(X_124, Y_12, WBUTTON, HBUTTON, "Spacing");
	vip124->value(col_width_);
	vip124->labelfont(FONT);
	vip124->labelsize(FONT_SIZE);
	vip124->textfont(FONT);
	vip124->textsize(FONT_SIZE);
	vip124->align(FL_ALIGN_LEFT);
	vip124->callback(cb_value<Fl_Value_Input, double>, &col_width_);
//	vip124->when(FL_WHEN_ENTER_KEY);
	vip124->tooltip("Please specify the spacing between columns");
	// Row 2A - height parameters
	// Input to specify the number of rows of labels to print
	Fl_Value_Input* vip12A1 = new Fl_Value_Input(X_121, Y_13, WBUTTON, HBUTTON, "Rows");
	vip12A1->value(num_rows_);
	vip12A1->labelfont(FONT);
	vip12A1->labelsize(FONT_SIZE);
	vip12A1->textfont(FONT);
	vip12A1->textsize(FONT_SIZE);
	vip12A1->align(FL_ALIGN_LEFT);
	vip12A1->callback(cb_value<Fl_Value_Input, int>, &num_rows_);
//	vip12A1->when(FL_WHEN_ENTER_KEY);
	vip12A1->tooltip("Please specify the number of rows when printing");
	// Input to specify the height of each label
	Fl_Value_Input* vip12A2 = new Fl_Value_Input(X_122, Y_13, WBUTTON, HBUTTON, "Height");
	vip12A2->value(height_);
	vip12A2->labelfont(FONT);
	vip12A2->labelsize(FONT_SIZE);
	vip12A2->textfont(FONT);
	vip12A2->textsize(FONT_SIZE);
	vip12A2->align(FL_ALIGN_LEFT);
	vip12A2->callback(cb_value<Fl_Value_Input, double>, &height_);
//	vip12A2->when(FL_WHEN_ENTER_KEY);
	vip12A2->tooltip("Please specify the width of the label in the selected label");
	// Input to specify the top of the first label
	Fl_Value_Input* vip12A3 = new Fl_Value_Input(X_123, Y_13, WBUTTON, HBUTTON, "Position");
	vip12A3->value(row_top_);
	vip12A3->labelfont(FONT);
	vip12A3->labelsize(FONT_SIZE);
	vip12A3->textfont(FONT);
	vip12A3->textsize(FONT_SIZE);
	vip12A3->align(FL_ALIGN_LEFT);
	vip12A3->callback(cb_value<Fl_Value_Input, double>, &row_top_);
//	vip12A3->when(FL_WHEN_ENTER_KEY);
	vip12A3->tooltip("Please specify the position of the first row");
	// Input to specify the gap between each row
	Fl_Value_Input* vip12A4 = new Fl_Value_Input(X_124, Y_13, WBUTTON, HBUTTON, "Spacing");
	vip12A4->value(row_height_);
	vip12A4->labelfont(FONT);
	vip12A4->labelsize(FONT_SIZE);
	vip12A4->textfont(FONT);
	vip12A4->textsize(FONT_SIZE);
	vip12A4->align(FL_ALIGN_LEFT);
	vip12A4->callback(cb_value<Fl_Value_Input, double>, &row_height_);
//	vip12A4->when(FL_WHEN_ENTER_KEY);
	vip12A4->tooltip("Please specify the spacing between rows");
	// Input to specify the (max) number of QSOs per QSL
	Fl_Value_Input* vip141 = new Fl_Value_Input(X_141, Y_14, WBUTTON, HBUTTON, "QSOs/Card");
	vip141->value(number_qsos_);
	vip141->labelfont(FONT);
	vip141->labelsize(FONT_SIZE);
	vip141->textfont(FONT);
	vip141->textsize(FONT_SIZE);
	vip141->align(FL_ALIGN_LEFT);
	vip141->callback(cb_value<Fl_Value_Input, int>, &number_qsos_);
//	vip141->when(FL_WHEN_ENTER_KEY);
	vip141->tooltip("Please specify the number of QSOs per Card");

	gp1->end();

	// Grp2 OK/Cancel
	// Group 1
	Fl_Group* gp2 = new Fl_Group(XGRP2, YGRP2, WGRP2, HGRP2);
	gp2->labelfont(FONT);
	gp2->labelsize(FONT_SIZE);
	gp2->box(FL_NO_BOX);
	gp2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// button - save and close
	Fl_Return_Button* ok_bn = new Fl_Return_Button(X_211, Y_21, WBUTTON, HBUTTON, "OK");
	ok_bn->labelsize(FONT_SIZE);
	ok_bn->color(FL_GREEN);
	ok_bn->callback(cb_bn_ok, nullptr);
	ok_bn->tooltip("Save changes and close dialog");
	add(ok_bn);
	// button - cancel last tab and close
	Fl_Button* cancel_bn = new Fl_Button(X_212, Y_21, WBUTTON, HBUTTON, "Cancel");
	cancel_bn->labelsize(FONT_SIZE);
	cancel_bn->color(FL_RED);
	cancel_bn->callback(cb_bn_can, nullptr);
	cancel_bn->tooltip("Cancel changes and close dialog");
	add(cancel_bn);
	gp2->end();

	this->callback(cb_bn_can, nullptr);
	end();

	resizable(nullptr);
	size(WALL, HALL);

	show();
}

// Save the new design
void qsl_design::save_values() {
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
	Fl_Preferences call_settings(qsl_settings, callsign.c_str());

	call_settings.set("Unit", (int&)unit_);
	call_settings.set("Width", width_);
	call_settings.set("Height", height_);
	call_settings.set("Number Rows", num_rows_);
	call_settings.set("Number Columns", num_cols_);
	call_settings.set("Column Width", col_width_);
	call_settings.set("Row Height", row_height_);
	call_settings.set("First Row", row_top_);
	call_settings.set("First Column", col_left_);
	call_settings.set("Max QSOs per Card", number_qsos_);

}

// Call back when a radio button is pressed - v indicates which button
void qsl_design::cb_radio_dim(Fl_Widget* w, void* v) {
	qsl_design* that = ancestor_view<qsl_design>(w);
	that->unit_ = ((qsl_form::dim_unit)(long)v);
}

// Callback - OK
void qsl_design::cb_bn_ok(Fl_Widget* w, void* arg) {
	// Find the active tab - assume that tabs is child 0
	qsl_design* that = ancestor_view<qsl_design>(w);
	that->save_values();
	Fl::delete_widget(that);
}


// Callback - Cancel
void qsl_design::cb_bn_can(Fl_Widget* w, void* arg) {
	// Find the active tab - assume that tabs is child 0
	qsl_design* that = ancestor_view<qsl_design>(w);
	Fl::delete_widget(that);
}

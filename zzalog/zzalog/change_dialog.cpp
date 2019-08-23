#include "change_dialog.h"
#include "callback.h"
#include "field_choice.h"
//#include "spec_data.h"

#include <set>
#include <string>

#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

using namespace std;
using namespace zzalog;

// extern spec_data* spec_data_;

// Constructor - calls the dialog constructor with placeholders for size
change_dialog::change_dialog(const char* label) :
	win_dialog(10, 10, label)
	, old_field_name_("")
	, new_field_name_("")
	, new_text_("")
	, w_field_name_(nullptr)
	, w_text_(nullptr)
	, action_(RENAME_FIELD)
{
	// Build the dialog
	create_form();
	enable_widgets();
	callback(cb_bn_cancel);
}

// Create the dialog
void change_dialog::create_form() {
	// Position calculations
	const int XG = EDGE;
	const int C1 = XG;
	const int W1 = WSMEDIT;
	const int C2 = C1 + W1 + GAP;
	const int W2 = WSMEDIT;
	const int WG = C2 + W2 - XG;
	const int W4 = WBUTTON;
	const int C4 = WG - W4;
	const int W3 = WBUTTON;
	const int C3 = C4 - W3 - GAP;
	const int WALL = XG + WG + EDGE;
	const int YG = EDGE;
	const int R1 = YG + GAP;
	const int H1 = HTEXT;
	const int R2 = R1 + H1 + GAP;
	const int H2 = HTEXT;
	const int R3 = R2 + H2 + GAP;
	const int H3 = HTEXT;
	const int R4 = R3 + H3 + GAP;
	const int H4 = HTEXT;
	const int R5 = R4 + H4 + GAP;
	const int H5 = HTEXT;
	const int HG = R5 + H5 + GAP - YG;
	const int R6 = R5 + H5 + GAP + GAP;
	const int H6 = HBUTTON;
	const int HALL = R6 + H6 + EDGE;

	// Change the window to fit before adding any widgets
	size(WALL, HALL);

	// Overall dialog surround (excludes OK/Cancel buttons)
	Fl_Group* gp1 = new Fl_Group(XG, YG, WG, HG);
	gp1->box(FL_FLAT_BOX);

	// Free standing label
	Fl_Box* bx11 = new Fl_Box(C1, R1, W1, H1, "Change field");
	bx11->labelsize(FONT_SIZE);
	bx11->box(FL_NO_BOX);
	bx11->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	// Group for radio buttons
	Fl_Group* gp21 = new Fl_Group(C1, R2, W1, HG - R2);
	gp21->box(FL_NO_BOX);

	// Radio button - change the name of the field field
	Fl_Radio_Round_Button* bn21 = new Fl_Radio_Round_Button(C1, R2, W1, H2, "Rename field");
	bn21->box(FL_THIN_UP_BOX);
	bn21->labelsize(FONT_SIZE);
	bn21->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn21->callback(cb_radio_action, (void*)&radio_params_[0]);
	bn21->when(FL_WHEN_RELEASE);
	bn21->value(action_ == RENAME_FIELD);
	bn21->tooltip("Change all fields from the original name to this new name");
	// Radio button - delete all instances of a field
	Fl_Radio_Round_Button* bn31 = new Fl_Radio_Round_Button(C1, R3, W1, H2, "Delete field");
	bn31->box(FL_THIN_UP_BOX);
	bn31->labelsize(FONT_SIZE);
	bn31->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn31->callback(cb_radio_action, (void*)&radio_params_[1]);
	bn31->when(FL_WHEN_RELEASE);
	bn31->value(action_ == DELETE_FIELD);
	bn31->tooltip("Delete all fields with this name");
	// Add a new field to all records - but leave existing fields with this name as is.
	Fl_Radio_Round_Button* bn41 = new Fl_Radio_Round_Button(C1, R4, W1, H2, "Add field");
	bn41->box(FL_THIN_UP_BOX);
	bn41->labelsize(FONT_SIZE);
	bn41->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn41->callback(cb_radio_action, (void*)&radio_params_[2]);
	bn41->when(FL_WHEN_RELEASE);
	bn41->value(action_ == ADD_FIELD);
	bn41->tooltip("Add fields with this data (do not change existing fields)");
	// Add a new field to all records - and change exsiting fields to this new value
	Fl_Radio_Round_Button* bn51 = new Fl_Radio_Round_Button(C1, R5, W1, H2, "Change field");
	bn51->box(FL_THIN_UP_BOX);
	bn51->labelsize(FONT_SIZE);
	bn51->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn51->callback(cb_radio_action, (void*)&radio_params_[3]);
	bn51->when(FL_WHEN_RELEASE);
	bn51->value(action_ == CHANGE_FIELD);
	bn51->tooltip("Add fields with this data (and change any existing fields)");

	gp21->end();

	// Old field name choice
	field_choice* ch12 = new field_choice(C2, R1, W2, H1);
	ch12->textsize(FONT_SIZE);
	int ix = 0;
	ch12->value(0);
	old_field_name_ = ch12->text();
	ch12->callback(cb_text<field_choice, string>, (void*)&old_field_name_);
	ch12->when(FL_WHEN_RELEASE);
	ch12->tooltip("Select the current name of the field");

	// New field name choice
	field_choice* ch22 = new field_choice(C2, R2, W2, H2);
	ch22->textsize(FONT_SIZE);
	ix = 0;
	ch22->value(0);
	new_field_name_ = ch22->text();
	ch22->callback(cb_text<field_choice, string>, (void*)&new_field_name_);
	ch22->when(FL_WHEN_RELEASE);
	ch22->tooltip("Select the name to change it to");
	w_field_name_ = ch22;

	// New text input
	Fl_Input* ip42 = new Fl_Input(C2, R4, W2, H4, "Value");
	ip42->labelsize(FONT_SIZE);
	ip42->textsize(FONT_SIZE);
	ip42->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip42->callback(cb_value<Fl_Input, string>, (void*)&new_text_);
	ip42->when(FL_WHEN_CHANGED);
	ip42->tooltip("Enter the value to apply");
	w_text_ = ip42;

	gp1->end();

	// OK button
	Fl_Button* bn63 = new Fl_Button(C3, R6, W3, H6, "OK");
	bn63->box(FL_UP_BOX);
	bn63->labelsize(FONT_SIZE);
	bn63->callback(cb_bn_ok);
	bn63->when(FL_WHEN_RELEASE);
	bn63->color(fl_lighter(FL_GREEN));
	bn63->tooltip("Make the changes");

	// Cancel button
	Fl_Button* bn64 = new Fl_Button(C4, R6, W4, H6, "Cancel");
	bn64->box(FL_UP_BOX);
	bn64->labelsize(FONT_SIZE);
	bn64->callback(cb_bn_cancel);
	bn64->when(FL_WHEN_RELEASE);
	bn64->color(fl_lighter(FL_RED));
	bn64->tooltip("Cancel changes");

	// 
	end();
}


change_dialog::~change_dialog()
{
}

// Get the data entered by the user.
void change_dialog::get_data(change_action_t& action, string& old_field_name, string& new_field_name, string& new_text) {
	action = action_;
	old_field_name = old_field_name_;
	new_field_name = new_field_name_;
	new_text = new_text_;
}

// callback - OK button
void change_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	change_dialog* that = ancestor_view<change_dialog>(w);
	that->do_button(BN_OK);
}

// callback - cancel button
void change_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	change_dialog* that = ancestor_view<change_dialog>(w);
	that->do_button(BN_CANCEL);
}

// callback - the radio buttons
void change_dialog::cb_radio_action(Fl_Widget* w, void* v) {
	change_dialog* that = ancestor_view<change_dialog>(w);
	cb_radio(w, v);
	that->enable_widgets();
}

// Enable widgets - enables/disables widgets depending on which action is selected
void change_dialog::enable_widgets() {
	switch (action_) {
	case RENAME_FIELD:
		w_field_name_->activate();
		w_text_->deactivate();
		break;
	case DELETE_FIELD:
		w_field_name_->deactivate();
		w_text_->deactivate();
		break;
	case ADD_FIELD:
	case CHANGE_FIELD:
		w_field_name_->deactivate();
		w_text_->activate();
		break;
	}
	redraw();
}

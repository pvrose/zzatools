#include "corr_dialog.h"

#include "../zzalib/callback.h"
//#include "spec_data.h"
#include "../zzalib/utils.h"
#include "field_choice.h"
#include "intl_widgets.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Check_Button.H>

using namespace zzalog;
using namespace zzalib;

//extern spec_data* spec_data_;

// Constuctor - dialog constructor called with place holder size
corr_dialog::corr_dialog(record* record, const string& field, const string& message) :
	win_dialog(10, 10, "Correct invalid ADIF item")
	, change_value_(false)
	, change_field_(false)
	, add_field_(false)
	, change_field_name_("")
	, add_field_name_("")
	, change_value_data_("")
	, add_value_data_("")
	, record_(nullptr)
	, query_field_("")
	, pending_button_(false)
	, button_(BN_OK)
{
	correction_message_ = "";
	field_choices_.clear();

	// position constants - basic 3 x 6 grid of items
	const int COL1 = EDGE;
	const int COL2 = COL1 + WLLABEL + GAP;
	const int COL3 = COL2 + WEDIT + GAP;
	const int WDLG = COL3 + WEDIT + EDGE;
	const int XABANDON = WDLG - EDGE - WBUTTON;
	const int XCANCEL = XABANDON - EDGE - WBUTTON;
	const int XOK = XCANCEL - GAP - WBUTTON;
	const int ROW1 = EDGE;
	const int ROW2 = ROW1 + HTEXT + GAP;
	const int ROW3 = ROW2 + HTEXT + GAP;
	const int ROW4 = ROW3 + HTEXT + GAP;
	const int ROW5 = ROW4 + HTEXT + GAP;
	const int ROW6 = ROW5 + HTEXT + GAP;
	const int HDLG = ROW6 + HBUTTON + EDGE;

	// Resize the window to fit all buttons
	size(WDLG, HDLG);
	// Set query parameters
	record_ = record;
	query_field_ = field;

	// add the items
	// static text
	Fl_Box* box1 = new Fl_Box(COL1, ROW1, WLLABEL, HTEXT, "Current field/data");
	box1->labelsize(FONT_SIZE);
	box1->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	box1->box(FL_FLAT_BOX);
	add(box1);

	// Display for name of field name being queried
	Fl_Output* op1 = new Fl_Output(COL2, ROW1, WEDIT, HTEXT, "Field");
	op1->textsize(FONT_SIZE);
	op1->textcolor(fl_darker(FL_BLUE));
	op1->color(FL_BACKGROUND_COLOR);
	op1->value(field.c_str());
	op1->align(FL_ALIGN_TOP);
	op1->tooltip("Field name being queried");
	// Display of its current value
	Fl_Output* op2 = new Fl_Output(COL3, ROW1, WEDIT, HTEXT, "Current value");
	op2->textsize(FONT_SIZE);
	op2->textcolor(fl_darker(FL_BLUE));
	op2->color(FL_BACKGROUND_COLOR);
	op2->value(record_->item(field).c_str());
	op2->align(FL_ALIGN_TOP);
	op2->tooltip("Current data in field being queried");
	// Action choice - change its value
	Fl_Check_Button* cb1 = new Fl_Check_Button(COL1, ROW2, WLLABEL, HTEXT, "Change value");
	cb1->labelsize(FONT_SIZE);
	cb1->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	cb1->callback(cb_value<Fl_Check_Button, bool>, (void*)&change_value_);
	cb1->when(FL_WHEN_CHANGED);
	cb1->tooltip("Change the data in the field");
	// Input - the new value wanted
	intl_input* ip1 = new intl_input(COL3, ROW2, WEDIT, HTEXT);
	ip1->textsize(FONT_SIZE);
	ip1->callback(cb_value<intl_input, string>, (void*)&change_value_data_);
	ip1->value(op2->value());
	ip1->when(FL_WHEN_CHANGED);
	ip1->tooltip("Value to change data to");
	// Action choice - change the field's name
	Fl_Check_Button* cb2 = new Fl_Check_Button(COL1, ROW3, WLLABEL, HTEXT, "Change field");
	cb2->labelsize(FONT_SIZE);
	cb2->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	cb2->callback(cb_value<Fl_Check_Button, bool>, (void*)&change_field_);
	cb2->when(FL_WHEN_CHANGED);
	cb2->tooltip("Change the field name");
	// Drop-down choice provides all possible field names
	field_choice* ch1 = new field_choice(COL2, ROW3, WEDIT, HTEXT);
	ch1->textsize(FONT_SIZE);
	ch1->callback(cb_choice_text, (void*)&change_field_name_);
	ch1->when(FL_WHEN_CHANGED);
	ch1->tooltip("Name to change field to");
	// Action choice - add another field
	Fl_Check_Button* cb3 = new Fl_Check_Button(COL1, ROW4, WLLABEL, HTEXT, "Add field");
	cb3->labelsize(FONT_SIZE);
	cb3->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	cb3->callback(cb_value<Fl_Check_Button, bool>, (void*)&add_field_);
	cb3->when(FL_WHEN_CHANGED);
	cb3->tooltip("Add another field");
	// Drop-down choice provides all possible field names
	field_choice* ch2 = new field_choice(COL2, ROW4, WEDIT, HTEXT);
	ch2->textsize(FONT_SIZE);
	ch2->callback(cb_choice_text, (void*)&add_field_name_);
	ch2->when(FL_WHEN_CHANGED);
	ch2->tooltip("Field to add");
	// Input - data to use in the additional field
	intl_input* ip2 = new intl_input(COL3, ROW4, WEDIT, HTEXT);
	ip2->textsize(FONT_SIZE);
	ip2->value(op2->value());
	ip2->callback(cb_value<intl_input, string>, (void*)&add_value_data_);
	ip2->when(FL_WHEN_CHANGED);
	ip2->tooltip("Data to use in added field");
	// Output to display the reason for the validation error.
	Fl_Multiline_Output* op3 = new Fl_Multiline_Output(COL1, ROW5, WDLG - EDGE, HTEXT);
	op3->textsize(FONT_SIZE);
	op3->textcolor(FL_RED);
	op3->color(FL_BACKGROUND_COLOR);
	op3->value(message.c_str());
	op3->box(FL_DOWN_BOX);
	op3->align(FL_ALIGN_CENTER);
	// OK Button
	Fl_Button* bn_ok = new Fl_Button(XOK, ROW6, WBUTTON, HBUTTON, "OK");
	bn_ok->labelsize(FONT_SIZE);
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	bn_ok->tooltip("Accept changes");
	// Cancel button
	Fl_Button* bn_cancel = new Fl_Button(XCANCEL, ROW6, WBUTTON, HBUTTON, "Cancel");
	bn_cancel->labelsize(FONT_SIZE);
	bn_cancel->callback(cb_bn_cancel);
	bn_cancel->when(FL_WHEN_RELEASE);
	bn_cancel->tooltip("Cancel changes");
	// Button - quit the validation
	Fl_Button* bn_quit = new Fl_Button(XABANDON, ROW6, WBUTTON, HBUTTON, "Quit all");
	bn_quit->labelsize(FONT_SIZE);
	bn_quit->callback(cb_bn_quit);
	bn_quit->when(FL_WHEN_RELEASE);
	bn_quit->tooltip("Quit validation");


	end();
	// Do not show yet

	// Remember the drop down boxes so that they can be populated with all the field names later
	field_choices_.clear();
	field_choices_.insert(ch1);
	field_choices_.insert(ch2);

	callback(cb_bn_cancel);
}

// Destructor
corr_dialog::~corr_dialog()
{
	// Delete the entries in the field choices
	for (auto it = field_choices_.begin(); it != field_choices_.end(); it++) {
		((Fl_Choice*)(*it))->clear();
	}
	clear();
}

// call backs

// OK button - obey choices made
void corr_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	corr_dialog* that = ancestor_view<corr_dialog>(w);
	string new_value;
	string new_field;
	string old_value = that->record_->item(that->query_field_);
	// Look at requested actions - note more than one can be selected
	if (that->change_value_) {
		// Change the fields value - Set the value to the data entered
		new_value = that->change_value_data_;
	}
	else {
		// Else keep the existing value
		new_value = that->record_->item(that->query_field_);
	}
	if (that->change_field_) {
		// Change the field's name - Set the name to the new one selected, delete the old item
		new_field = that->change_field_name_;
		that->record_->item(that->query_field_, string(""));
	}
	else {
		// Else keep the existing name
		new_field = that->query_field_;
	}
	if (that->change_field_ || that->change_value_) {
		// The item has been changed - update it 
		that->record_->item(new_field, new_value);
		// Create report line
		that->correction_message_ = that->query_field_ + "=" + old_value + " user-corrected to " +
			new_field + "=" + new_value + ".";
	}
	if (that->add_field_) {
		// Create the new item
		that->record_->item(that->add_field_name_, that->add_value_data_);
		// Create report line
		that->correction_message_ = that->add_field_name_ + "=" + that->add_value_data_ + " added by user.";
	}
	if (that->change_field_ || that->change_value_ || that->add_field_) {
		// Some action done - report OK
		that->do_button(BN_OK);
	}
	else {
		// No action done - treat it as CANCEL
		that->do_button(BN_CANCEL);
	}
}

// Cancel button - do nothing except report cancel
void corr_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	corr_dialog* that = ancestor_view<corr_dialog>(w);
	that->do_button(BN_CANCEL);
}

// Quit button
void corr_dialog::cb_bn_quit(Fl_Widget* w, void* v) {
	corr_dialog* that = ancestor_view<corr_dialog>(w);
	that->do_button(BN_SPARE);
}

// Returns correction message - set in OK button
string corr_dialog::correction_message() {
	return correction_message_;
}

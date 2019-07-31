#include "scratchpad.h"
#include "field_choice.h"
#include "callback.h"
#include "utils.h"
#include "book.h"
#include "pfx_data.h"
#include "status.h"
#include "tabbed_forms.h"
#include "menu.h"
#include "intl_dialog.h"

#include <regex>

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern book* book_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_view_;
extern menu* menu_;
extern Fl_Preferences* settings_;
extern intl_dialog* intl_dialog_;
extern void add_scratchpad();


scratchpad::scratchpad() :
	win_dialog(480, 200, "Scratchpad")
	, buffer_(nullptr)
	, record_(nullptr)
	, field_("")
{
	border(true);
	create_form();
	status_->misc_status(ST_OK, "SCRATCHPAD: Created");
}

// Destructor - delete dynamic objects and save position
scratchpad::~scratchpad()
{
	delete record_;
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Top", this->y_root());
	spad_settings.set("Left", this->x_root());
	status_->misc_status(ST_NOTE, "SCRATCHPAD: Deleted");
}

void scratchpad::create_form() {
	const int WEDITOR = w() - EDGE - WBUTTON - WBUTTON - GAP - EDGE;
	const int HEDITOR = h() - EDGE - EDGE;
	const int C2 = EDGE + WEDITOR + GAP;
	const int C3 = C2 + WBUTTON;

	// Create the editor
	buffer_ = new Fl_Text_Buffer(1024);
	editor_ = new Fl_Text_Editor(EDGE, EDGE, WEDITOR, HEDITOR);
	editor_->buffer(buffer_);
	editor_->textsize(FONT_SIZE);
	editor_->textfont(FONT);
	// The callback will be explicitly done 
	editor_->when(FL_WHEN_NEVER);

	// Create the buttons
	int curr_y = EDGE;
	bn_start_ = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "Start");
	bn_start_->labelsize(FONT_SIZE);
	bn_start_->labelfont(FONT);
	bn_start_->tooltip("Create a new record");
	bn_start_->callback(cb_start);

	curr_y += HBUTTON + GAP;
	Fl_Button* bn_call = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "Call");
	bn_call->labelsize(FONT_SIZE);
	bn_call->labelfont(FONT);
	bn_call->tooltip("Copy selected text to callsign field");
	bn_call->callback(cb_action, (void*)WRITE_CALL);

	Fl_Button* bn_name = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "Name");
	bn_name->labelsize(FONT_SIZE);
	bn_name->labelfont(FONT);
	bn_name->tooltip("Copy selected text to name field");
	bn_name->callback(cb_action, (void*)WRITE_NAME);

	curr_y += HBUTTON;
	Fl_Button* bn_qth = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "QTH");
	bn_qth->labelsize(FONT_SIZE);
	bn_qth->labelfont(FONT);
	bn_qth->tooltip("Copy selected text to QTH field");
	bn_qth->callback(cb_action, (void*)WRITE_QTH);

	Fl_Button* bn_grid = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "Gridsquare");
	bn_grid->labelsize(FONT_SIZE);
	bn_grid->labelfont(FONT);
	bn_grid->tooltip("Copy selected text to gridsquare field");
	bn_grid->callback(cb_action, (void*)WRITE_GRID);

	curr_y += HBUTTON;
	Fl_Button* bn_rst_sent = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "RST Sent");
	bn_rst_sent->labelsize(FONT_SIZE);
	bn_rst_sent->labelfont(FONT);
	bn_rst_sent->tooltip("Copy selected text to RST sent field");
	bn_rst_sent->callback(cb_action, (void*)WRITE_RST_SENT);

	Fl_Button* bn_rst_rcvd = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "RST Rcvd");
	bn_rst_rcvd->labelsize(FONT_SIZE);
	bn_rst_rcvd->labelfont(FONT);
	bn_rst_rcvd->tooltip("Copy selected text to RST Received field");
	bn_rst_rcvd->callback(cb_action, (void*)WRITE_RST_RCVD);

	curr_y += HBUTTON;
	Fl_Button* bn_field = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "Field");
	bn_field->labelsize(FONT_SIZE);
	bn_field->labelfont(FONT);
	bn_field->tooltip("Copy selected text to the field specified below");
	bn_field->callback(cb_action, (void*)WRITE_FIELD);

	curr_y += HBUTTON;
	field_choice* ch_field = new field_choice(C2, curr_y, WBUTTON + WBUTTON, HTEXT);
	ch_field->textfont(FONT);
	ch_field->textsize(FONT_SIZE);
	ch_field->tooltip("Select the field you want the selected text to be written to");
	ch_field->callback(cb_text<Fl_Choice, string>, (void*)&field_);

	curr_y += HTEXT + GAP;
	bn_save_ = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "Save");
	bn_save_->labelsize(FONT_SIZE);
	bn_save_->labelfont(FONT);
	bn_save_->tooltip("Save the record");
	bn_save_->callback(cb_save);

	bn_cancel_ = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "Cancel");
	bn_cancel_->labelsize(FONT_SIZE);
	bn_cancel_->labelfont(FONT);
	bn_cancel_->tooltip("Cancel the record");
	bn_cancel_->callback(cb_cancel);

	end();
	show();

	enable_widgets();
	// Add call back
	callback(cb_close);

}

// One of the write field buttons has been activated
void scratchpad::cb_action(Fl_Widget* w, void* v) {
	actions action = (actions)(long)v;
	scratchpad* that = ancestor_view<scratchpad>(w);

	string field;
	// Create a record if we haven't started editing one
	if (that->record_ == nullptr) {
		that->record_ = book_->new_record(menu_->logging());
	}
	// Get the field to write from the button action
	switch (action) {
	case WRITE_CALL:
		field = "CALL";
		break;
	case WRITE_NAME:
		field = "NAME";
		break;
	case WRITE_QTH:
		field = "QTH";
		break;
	case WRITE_GRID:
		field = "GRIDSQUARE";
		break;
	case WRITE_RST_SENT:
		field = "RST_SENT";
		break;
	case WRITE_RST_RCVD:
		field = "RST_RCVD";
		break;
	case WRITE_FIELD:
		field = that->field_;
		break;
	}
	// Get the highlighted text from the editor buffer and write it to the selected field, unhighlight the text
	char* text = that->buffer_->selection_text();
	that->record_->item(field, string(text));
	free(text);
	that->buffer_->unselect();
	// Update views
	tabbed_view_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	// Give the editor the focus
	that->editor_->take_focus();
	// We may have changed the state
	that->enable_widgets();
}

// Save the record and reset the state to no record
void scratchpad::cb_save(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	book_->save_record();
	that->record_ = nullptr;
	that->buffer_->text("");
	that->enable_widgets();
}

// Cancel the edit and reset the state to no record
void scratchpad::cb_cancel(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	book_->delete_record(false);
	that->record_ = nullptr;
	that->buffer_->text("");
	that->enable_widgets();
}

// Close button clicked - check editing or not
void scratchpad::cb_close(Fl_Widget* w, void* v) {
	// It is the window that raised this callback
	scratchpad* that = (scratchpad*)w;
	// If we are editing does the user want to save or cancel?
	if (that->record_ != nullptr) {
		if (fl_choice("Entering a record - save or cancel?", "Cancel", "Save", nullptr) == 1) {
			cb_save(w, v);
		}
		else {
			cb_cancel(w, v);
		}
	}
	// Mark scratchpad disabled, hide it and update menu item
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Enabled", (int)false);
	add_scratchpad();
	menu_->update_items();
}

// Start button - create a new record
void scratchpad::cb_start(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	that->record_ = book_->new_record(menu_->logging());
	that->enable_widgets();
}

// Enable/disable conditional buttons
void scratchpad::enable_widgets() {
	if (record_ != nullptr) {
		// Allow save and cancel as we have a record
		bn_save_->activate();
		bn_cancel_->activate();
		bn_start_->deactivate();
	}
	else {
		bn_save_->deactivate();
		bn_cancel_->deactivate();
		bn_start_->activate();
	}
 }


// Default Text_Editor constructor
scratchpad::editor::editor(int X, int Y, int W, int H) :
	Fl_Text_Editor(X, Y, W, H) {
}

// Handle - special action on left and right click
int scratchpad::editor::handle(int event) {
	switch (event) {
	case FL_FOCUS:
		// Notify that this is the current editor to receive pastes of international chatacters
		if (intl_dialog_) {
			intl_dialog_->editor(this);
		}
	case FL_PUSH:
		// Tell FLTK we want to see FL_RELEASE
		return true;
	case FL_RELEASE:
		if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks() != 0) {
			// SElect the whole word
			int pos = insert_position();
			int word_begin = buffer()->word_start(pos);
			int word_end = buffer()->word_end(pos);
			buffer()->select(word_begin, word_end);
			return true;
		}
	}
	return Fl_Text_Editor::handle(event);
}


#include "scratchpad.h"
#include "field_choice.h"
#include "../zzalib/callback.h"
#include "../zzalib/utils.h"
#include "book.h"
#include "pfx_data.h"
#include "status.h"
#include "tabbed_forms.h"
#include "menu.h"
#include "intl_dialog.h"
#include "spec_data.h"
#include "rig_if.h"
#include "band_view.h"

#include <regex>

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Hold_Browser.H>

using namespace zzalog;

extern book* book_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_view_;
extern menu* menu_;
extern Fl_Preferences* settings_;
extern intl_dialog* intl_dialog_;
extern spec_data* spec_data_;
extern rig_if* rig_if_;
extern band_view* band_view_;
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
	const int C2A = C2 + (WBUTTON / 2);

	// Create the editor
	buffer_ = new Fl_Text_Buffer(1024);
	editor_ = new Fl_Text_Editor(EDGE, EDGE, WEDITOR, HEDITOR);
	editor_->buffer(buffer_);
	editor_->textsize(FONT_SIZE);
	editor_->textfont(FONT);
	// The callback will be explicitly done 
	editor_->when(FL_WHEN_NEVER);

	// Create the buttons - see labels and tooltips for more information
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

	curr_y += HTEXT;
	const int W2A = WBUTTON * 3 / 2;
	ip_freq_ = new Fl_Input(C2A, curr_y, W2A, HTEXT, "Freq");
	ip_freq_->textfont(FONT);
	ip_freq_->textsize(FONT_SIZE);
	ip_freq_->labelfont(FONT);
	ip_freq_->labelsize(FONT_SIZE);
	ip_freq_->align(FL_ALIGN_LEFT);
	ip_freq_->tooltip("Enter frequency of operation, if different");
	ip_freq_->callback(cb_ip_freq);

	curr_y += HTEXT;
	ch_mode_ = new Fl_Choice(C2A, curr_y, W2A, HTEXT, "Mode");
	ch_mode_->textfont(FONT);
	ch_mode_->textsize(FONT_SIZE);
	ch_mode_->labelfont(FONT);
	ch_mode_->labelsize(FONT_SIZE);
	ch_mode_->align(FL_ALIGN_LEFT);
	ch_mode_->tooltip("Select mode of operatio, if different");
	ch_mode_->callback(cb_ch_mode);

	curr_y += HTEXT;
	ip_power_ = new Fl_Input(C2A, curr_y, W2A, HTEXT, "Power");
	ip_power_->textfont(FONT);
	ip_power_->textsize(FONT_SIZE);
	ip_power_->labelfont(FONT);
	ip_power_->labelsize(FONT_SIZE);
	ip_power_->align(FL_ALIGN_LEFT);
	ip_power_->tooltip("Enter transmit power, if different");
	ip_power_->callback(cb_ip_power);

	record* prev_record = book_->get_record(book_->size() - 1, false);
	string frequency;
	string power;
	string mode;
	string submode;
	if (rig_if_) {
		// Get data from rig
		frequency = rig_if_->get_tx_frequency();
		power = rig_if_->get_tx_power();
		rig_if_->get_string_mode(mode, submode);
	}
	else if (record_) {
		frequency = record_->item("FREQ");
		power = record_->item("TX_PWR");
		mode = record_->item("MODE", true);
	}
	else if (prev_record) {
		frequency = prev_record->item("FREQ");
		power = prev_record->item("TX_PWR");
		mode = prev_record->item("MODE", true);
	}
	else {
		frequency = "14.250";
		power = "100";
		mode = "USB";
	}
	ip_freq_->value(frequency.c_str());
	spec_data_->initialise_field_choice(ch_mode_, "Combined", mode);
	ip_power_->value(power.c_str()); 
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

	// Resize if now too big
	curr_y = max(curr_y + HBUTTON + EDGE, h());
	size(w(), curr_y);
	editor_->size(editor_->w(), curr_y - EDGE - EDGE);
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
	hint_t hint = HT_MINOR_CHANGE;
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
		hint = HT_CHANGED;
		break;
	case WRITE_RST_SENT:
		field = "RST_SENT";
		break;
	case WRITE_RST_RCVD:
		field = "RST_RCVD";
		break;
	case WRITE_FIELD:
		field = that->field_;
		if (field == "DXCC" || field == "GRIDSQUARE") {
			hint = HT_MINOR_CHANGE;
		}
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
	// Save frequency mode and TX_PWR if that didn't change
	string freq;
	cb_value<Fl_Input, string>(that->ip_freq_, &freq);
	that->record_->item("FREQ", freq);
	string mode;
	cb_choice_text(that->ch_mode_, &mode);
	that->record_->item("MODE", mode);
	string power;
	cb_value<Fl_Input, string>(that->ip_power_, &power);
	that->record_->item("TX_PWR", power);
	// Save the record - should update viewsg
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
	if (rig_if_) {
		that->record_->item("FREQ", string(that->ip_freq_->value()));
		string mode;
		cb_choice_text(that->ch_mode_, &mode);
		if (spec_data_->is_submode(mode)) {
			that->record_->item("SUBMODE", mode);
			that->record_->item("MODE", spec_data_->mode_for_submode(mode));
		}
		else {
			that->record_->item("MODE", mode);
			that->record_->item("SUBMODE", string(""));
		}
		that->record_->item("TX_PWR", string(that->ip_power_->value()));
		tabbed_view_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
	that->enable_widgets();
}

// Frequency input
void scratchpad::cb_ip_freq(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	if (that->record_) {
		string value;
		cb_value<Fl_Input, string>(w, &value);
		double freq = stod(value) * 1000;
		that->record_->item("FREQ", value);
		if (band_view_ && !band_view_->in_band(freq)) {
			((Fl_Input*)w)->textcolor(FL_RED);
		}
		else {
			((Fl_Input*)w)->textcolor(FL_BLACK);
		}
		// Update views
		band_view_->update(freq);
		tabbed_view_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
}

// Mode choice
void scratchpad::cb_ch_mode(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	if (that->record_) {
		string value;
		cb_choice_text(w, &value);
		if (spec_data_->is_submode(value)) {
			that->record_->item("SUBMODE", value);
			that->record_->item("MODE", spec_data_->mode_for_submode(value));
		}
		else {
			that->record_->item("MODE", value);
			that->record_->item("SUBMODE", string(""));
		}
		// Update views
		tabbed_view_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
}

// Power input
void scratchpad::cb_ip_power(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	if (that->record_) {
		string value;
		cb_value<Fl_Input, string>(w, &value);
		that->record_->item("TX_PWR", value);
		// Update views
		tabbed_view_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
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

// Called when rig is read
void scratchpad::rig_update(string frequency, string mode, string power) {
	ip_freq_->value(frequency.c_str());
	double freq = stod(frequency) * 1000;
	if (band_view_ && !band_view_->in_band(freq)) {
		ip_freq_->textcolor(FL_RED);
	}
	else {
		ip_freq_->textcolor(FL_BLACK);
	}
	ip_power_->value(power.c_str());
	int index = ch_mode_->find_index(mode.c_str());
	ch_mode_->value(index);
	redraw();
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

void scratchpad::update() {
	record* prev_record = book_->get_record();
	ip_freq_->value(prev_record->item("FREQ").c_str());
	ip_power_->value(prev_record->item("TX_PWR").c_str());
	ch_mode_->value(ch_mode_->find_index(prev_record->item("MODE", true).c_str()));
	band_view_->update(stod(prev_record->item("FREQ")));
	redraw();
}
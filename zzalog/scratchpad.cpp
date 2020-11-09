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
#include "../zzalib/rig_if.h"
#include "band_view.h"
#include "extract_data.h"

#include <regex>

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Hold_Browser.H>

using namespace zzalog;
using namespace zzalib;

extern book* book_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern menu* menu_;
extern Fl_Preferences* settings_;
extern intl_dialog* intl_dialog_;
extern spec_data* spec_data_;
extern rig_if* rig_if_;
extern band_view* band_view_;
extern extract_data* extract_records_;
extern void add_scratchpad();

// Constructor for scratchpad editor
spad_editor::spad_editor(int x, int y, int w, int h) :
	intl_editor(x, y, w, h)
{
}

// Destructor for scratchpad editor
spad_editor::~spad_editor() {
}

// Handler of any event coming from the scratchpad editor
int spad_editor::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		// mouse going in and out of focus on this view
		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
		return true;
	case FL_KEYBOARD:
		// Keyboard event - used for keyboard navigation
		switch (Fl::event_key()) {
		case FL_F + 1:
			// F1 - copy selected text to CALL field
			scratchpad::cb_action(this, (void*)scratchpad::WRITE_CALL);
			return true;
		case FL_F + 2:
			// F2 - copy selected text to NAME field
			scratchpad::cb_action(this, (void*)scratchpad::WRITE_NAME);
			return true;
		case FL_F + 3:
			// F3 - copy selected text to QTH field
			scratchpad::cb_action(this, (void*)scratchpad::WRITE_QTH);
			return true;
		case FL_F + 4:
			// F4 - copy selected text to RST_RCVD field
			scratchpad::cb_action(this, (void*)scratchpad::WRITE_RST_RCVD);
			return true;
		case FL_F + 5:
			// F5 - copy selected text to RST_SENT field
			scratchpad::cb_action(this, (void*)scratchpad::WRITE_RST_SENT);
			return true;
		case FL_F + 6:
			// F6 - copy selected text to GRIDSQUARE field
			scratchpad::cb_action(this, (void*)scratchpad::WRITE_GRID);
			return true;
		case FL_F + 7:
			// F7 - save record
			scratchpad::cb_save(this, nullptr);
			return true;
		case FL_F + 8:
			// F8 - discard record
			scratchpad::cb_cancel(this, nullptr);
			return true;
		case FL_F + 9:
			// F9 - Check worked before
			scratchpad::cb_wkb4(this, nullptr);
			return true;
		}
	}
	// Not handled the event - pass up the inheritance
	return intl_editor::handle(event);
}

const int HG = (6 * HBUTTON) + (4 * HTEXT) + (2 * GAP);

// Constructor for scratchpad
scratchpad::scratchpad() :
	win_dialog(WEDITOR + (2 * WBUTTON) + GAP + (2 * EDGE), HG + (2 * GAP), "Scratchpad")
	, buffer_(nullptr)
	, record_(nullptr)
	, field_("")
{
	// These are static, but will get to the same value each time
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences log_settings(user_settings, "Scratchpad");
	log_settings.get("Font Name", (int&)font_, FONT);
	log_settings.get("Font Size", (int&)fontsize_, FONT_SIZE);
	
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

// Create form
void scratchpad::create_form() {
	const int HEDITOR = h() - EDGE - EDGE;
	const int C2 = EDGE + WEDITOR + GAP;
	const int C3 = C2 + WBUTTON;
	const int C2A = C2 + (WBUTTON / 2);

	// Create the editor
	buffer_ = new Fl_Text_Buffer(1024);
	editor_ = new spad_editor(EDGE, EDGE, WEDITOR, HEDITOR);
	editor_->buffer(buffer_);
	editor_->textsize(fontsize_);
	editor_->textfont(font_);
	// The callback will be explicitly done in the handle routine of the editor
	editor_->when(FL_WHEN_NEVER);
	// Allways wrap at a word boundary
	editor_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

	// Create the buttons - see labels and tooltips for more information
	int curr_y = EDGE;
	// First we create an invisible group
	Fl_Group* g = new Fl_Group(C2, curr_y, w() - C2, HG);
	g->box(FL_NO_BOX);

	bn_start_ = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "Start");
	bn_start_->labelsize(FONT_SIZE);
	bn_start_->labelfont(FONT);
	bn_start_->tooltip("Create a new record");
	bn_start_->callback(cb_start);

	Fl_Button* bn_wb4 = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "F9 - B4?");
	bn_wb4->labelsize(FONT_SIZE);
	bn_wb4->labelfont(FONT);
	bn_wb4->tooltip("Display previous QSOs");
	bn_wb4->callback(cb_wkb4);

	curr_y += HBUTTON + GAP;
	Fl_Button* bn_call = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "F1 - Call");
	bn_call->labelsize(FONT_SIZE);
	bn_call->labelfont(FONT);
	bn_call->tooltip("Copy selected text to callsign field");
	bn_call->callback(cb_action, (void*)WRITE_CALL);

	Fl_Button* bn_name = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "F2 - Name");
	bn_name->labelsize(FONT_SIZE);
	bn_name->labelfont(FONT);
	bn_name->tooltip("Copy selected text to name field");
	bn_name->callback(cb_action, (void*)WRITE_NAME);

	curr_y += HBUTTON;
	Fl_Button* bn_qth = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "F3 - QTH");
	bn_qth->labelsize(FONT_SIZE);
	bn_qth->labelfont(FONT);
	bn_qth->tooltip("Copy selected text to QTH field");
	bn_qth->callback(cb_action, (void*)WRITE_QTH);

	Fl_Button* bn_grid = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "F4 - Grid");
	bn_grid->labelsize(FONT_SIZE);
	bn_grid->labelfont(FONT);
	bn_grid->tooltip("Copy selected text to gridsquare field");
	bn_grid->callback(cb_action, (void*)WRITE_GRID);

	curr_y += HBUTTON;
	Fl_Button* bn_rst_sent = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "F5 - Sent");
	bn_rst_sent->labelsize(FONT_SIZE);
	bn_rst_sent->labelfont(FONT);
	bn_rst_sent->tooltip("Copy selected text to RST sent field");
	bn_rst_sent->callback(cb_action, (void*)WRITE_RST_SENT);

	Fl_Button* bn_rst_rcvd = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON, "F6 - Rcvd");
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
		frequency = rig_if_->get_frequency(true);
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

	bn_save_ = new Fl_Button(C2, curr_y, WBUTTON, HBUTTON, "F7 - Save");
	bn_save_->labelsize(FONT_SIZE);
	bn_save_->labelfont(FONT);
	bn_save_->tooltip("Save the record");
	bn_save_->callback(cb_save);

	bn_cancel_ = new Fl_Button(C3, curr_y, WBUTTON, HBUTTON);
	bn_cancel_->labelsize(FONT_SIZE);
	bn_cancel_->labelfont(FONT);
	bn_cancel_->tooltip("Cancel the record");
	bn_cancel_->callback(cb_cancel);

	g->resizable(nullptr);
	g->end();

	size_range(w() - WEDITOR / 2, h());
	resizable(editor_);
	end();
	show();

	enable_widgets();
	// Add call back
	callback(cb_close);

}

// One of the write field buttons has been activated
// v provides the specific action 
void scratchpad::cb_action(Fl_Widget* w, void* v) {
	actions action = (actions)(long)v;
	scratchpad* that = ancestor_view<scratchpad>(w);

	string field;
	// Create a record if we haven't started editing one
	if (that->record_ == nullptr) {
		cb_start(w, nullptr);
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
			hint = HT_CHANGED;
		}
		break;
	}
	// Get the highlighted text from the editor buffer and write it to the selected field, unhighlight the text
	string text = that->buffer_->selection_text();
	// Remove leading and trailing white space
	while (isspace(text[0])) text = text.substr(1);
	while (isspace(text[text.length() - 1])) text = text.substr(0, text.length() - 1);
	that->record_->item(field, text);
	that->buffer_->unselect();
	// Update views
	tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	// Give the editor the focus
	that->editor_->take_focus();
	// We may have changed the state
	that->enable_widgets();
}

void scratchpad::cb_wkb4(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	string text = that->buffer_->selection_text();
	extract_records_->extract_call(text);
}

// Save the record and reset the state to no record
// v is not used
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
// v is not used
void scratchpad::cb_cancel(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	book_->delete_record(false);
	that->record_ = nullptr;
	that->buffer_->text("");
	that->enable_widgets();
}

// Close button clicked - check editing or not
// v is not used
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
// v is not used
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
		tabbed_forms_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
	that->enable_widgets();
}

// Frequency input
// v is not used
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
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
}

// Mode choice
// v is not used
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
		tabbed_forms_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
}

// Power input
// v is not used
void scratchpad::cb_ip_power(Fl_Widget* w, void* v) {
	scratchpad* that = ancestor_view<scratchpad>(w);
	if (that->record_) {
		string value;
		cb_value<Fl_Input, string>(w, &value);
		that->record_->item("TX_PWR", value);
		// Update views
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
}

// Enable/disable conditional buttons
void scratchpad::enable_widgets() {
	if (record_ != nullptr) {
		// Allow save and cancel as we have a record
		bn_save_->activate();
		bn_cancel_->activate();
		bn_cancel_->label("F8 - Cancel");
		bn_start_->deactivate();
	}
	else {
		bn_save_->deactivate();
		bn_cancel_->activate();
		bn_cancel_->label("F8 - Clear");
		bn_start_->activate();
	}
 }

// Called when rig is read to update values here
void scratchpad::rig_update(string frequency, string mode, string power) {
	ip_freq_->value(frequency.c_str());
	double freq = stod(frequency) * 1000;
	// If the frequency is outside a ham-band, display in red else in black
	if (band_view_ && !band_view_->in_band(freq)) {
		ip_freq_->textcolor(FL_RED);
	}
	else {
		ip_freq_->textcolor(FL_BLACK);
	}
	// Power in watts
	ip_power_->value(power.c_str());
	// Mode - index into choice
	int index = ch_mode_->find_index(mode.c_str());
	ch_mode_->value(index);
	redraw();
}

// Get frequency, power and mode from previous record not rig
void scratchpad::update() {
	if (book_->size()) {
		record* prev_record = book_->get_record();
		// Assume as it's a logged record, the frequency is valid
		ip_freq_->textcolor(FL_BLACK);
		ip_freq_->value(prev_record->item("FREQ").c_str());
		ip_power_->value(prev_record->item("TX_PWR").c_str());
		ch_mode_->value(ch_mode_->find_index(prev_record->item("MODE", true).c_str()));
		if (band_view_ && prev_record->item_exists("FREQ")) band_view_->update(stod(prev_record->item("FREQ")) * 1000.0);
	}
	else {
		// No default
		ip_freq_->textcolor(FL_RED);
		ip_freq_->value("0");
		ip_power_->value("0");
		ch_mode_->value(0);
	}
	redraw();
}

// Set font etc.
void scratchpad::set_font(Fl_Font font, Fl_Fontsize size) {
	font_ = font;
	fontsize_ = size;
	// Change the font in the editor
	editor_->textsize(fontsize_);
	editor_->textfont(font_);
	// And ask it to recalculate the wrap positions
	editor_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	redraw();
}
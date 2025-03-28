#include "field_choice.h"
#include "fields.h"
#include "spec_data.h"
#include "intl_dialog.h"
#include "utils.h"
#include "cty_data.h"
#include "qso_entry.h"

#include <cstring>

#include <FL/Fl_Preferences.H>

extern spec_data* spec_data_;
extern cty_data* cty_data_;
extern intl_dialog* intl_dialog_;

// Lists greater than this will be hierarchic - e.g. "A/ADDRESS" else not so "ADDRESS"
const int HIERARCHIC_LIMIT = 12;

// Constructor - add selected fields (most common) from the spec database
field_choice::field_choice(int X, int Y, int W, int H, const char* label) :
	Fl_Choice(X, Y, W, H, label)
	, hierarchic_(true)
	, dataset_(nullptr)
{
}

// Destructor
field_choice::~field_choice()
{
	clear();
}

// Set dataset
void field_choice::set_dataset(string dataset_name, string default_field /* = "" */) {
	// Get the dataset
	dataset_ = spec_data_->dataset(dataset_name);
	// Initialsie
	clear();
	add("", 0, (Fl_Callback*)nullptr);

	// For all dataset
	auto it = dataset_->data.begin();
	if (dataset_->data.size() > HIERARCHIC_LIMIT) {
		hierarchic_ = true;
		for (int i = 1; it != dataset_->data.end(); i++) {
			string text = escape_menu(it->first);
			char curr = text[0];
			// generate tree A/ADDRESS etc
			char entry[128];
			snprintf(entry, 128, "%c/%s", curr, text.c_str());
			add(entry, 0, (Fl_Callback*)nullptr);
			it++;
		}
	}
	else {
		hierarchic_ = false;
		for (int i = 1; it != dataset_->data.end(); i++) {
			add(escape_menu(it->first).c_str(), 0, (Fl_Callback*)nullptr);
			it++;
		}
	}
	value(default_field.c_str());
}

// Get the character string at the selected item
const char* field_choice::value() {
	char* result = new char[128];
	char temp[128];
	item_pathname(temp, 128);
	// If there is a value get its leaf text.
	if (hierarchic_ && strlen(temp) && temp[0] != 0) {
		const char* last_slash = strrchr(temp, '/');
		strcpy(result, last_slash + 1);
	}
	else {
		strcpy(result, temp + 1);
	}
	return result;
}

// Select the item with the leaf name
void field_choice::value(const char* field) {
	if (field == nullptr || strlen(field) == 0) {
		Fl_Choice::value(-1);
	}
	else {
		if (hierarchic_) {
			char actual[128];
			snprintf(actual, 128, "%c/%s", field[0], field);
			Fl_Choice::value(find_index(actual));
		}
		else {
			Fl_Choice::value(find_index(field));
		}
	}
}

// Set hierarchic value
void field_choice::hierarchic(bool h) {
	hierarchic_ = h;
}

Fl_Window* field_input::tip_window_ = nullptr;
const char* comment_marker_ = ":--->";

// field_input constructor
field_input::field_input(int X, int Y, int W, int H, const char* label) :
	Fl_Input_Choice(X, Y, W, H, label)
	, field_name_("")
	, use_menubutton_(false)
	, qso_(nullptr)
{
	type(FL_NORMAL_INPUT);
	menubutton()->callback(cb_menu);
	resize(X, Y, W, H);
}

field_input::~field_input() {
	if (intl_dialog_ && intl_dialog_->visible()) {
		intl_dialog_->editor(nullptr);
	}
}

// Override a few events
int field_input::handle(int event) {
	reason_ = IR_NULL;
	// Tell international character dialog to paste to this widget
	qso_entry* qe = ancestor_view<qso_entry>(this);
	switch (event) {
	case FL_PUSH:
		if (intl_dialog_) {
			intl_dialog_->editor(this);
		}
		if (qe) qe->save_focus(this);
		// Remove any tip window on this or other field_input
		if (tip_window_) {
			// Delete existing tip window
			Fl::delete_widget(tip_window_);
			tip_window_ = nullptr;
		}
		return Fl_Input_Choice::handle(event);
	case FL_FOCUS:
		if (intl_dialog_) {
			intl_dialog_->editor(this);
		}
		if (input()->take_focus()) return true;
		return Fl_Input_Choice::handle(event);
	case FL_UNFOCUS:
		if (tip_window_) {
			Fl::delete_widget(tip_window_);
			tip_window_ = nullptr;
		}
		return Fl_Input_Choice::handle(event);
	case FL_RELEASE:
		switch (Fl::event_button()) {
		case FL_RIGHT_MOUSE: {
			if (qso_ && field_name_.length()) {
				string tip;
				if (field_name_ == "CALL") {
					tip = cty_data_->get_tip(qso_);
				}
				else {
					tip = spec_data_->get_tip(field_name_, qso_);
				}
				if (tip_window_) {
					// Delete existing tip window
					Fl::delete_widget(tip_window_);
					tip_window_ = nullptr;
				}
				// Remember tip position
				tip_window_ = ::tip_window(tip, Fl::event_x_root(), Fl::event_y_root());
				tip_window_->show();
			}
			break;
		}
		}
		return Fl_Input_Choice::handle(event);
	case FL_KEYBOARD:
	// Convert navigation keys to commands
		switch (Fl::event_key()) {
		case FL_Tab:
			// Send to parent to handle
			return 0;
		case FL_Enter:
			reason_ = IR_ENTER;
			do_callback();
			return 1;
		case FL_Left:
			// ALT/Left
			if (Fl::event_state(FL_ALT)) {
				reason_ = IR_LEFT;
				do_callback();
				return 1;
			}
		case FL_Right:
			// ALT/Right
			if (Fl::event_state(FL_ALT)) {
				reason_ = IR_RIGHT;
				do_callback();
				return 1;
			}
		case FL_Up:
			// ALT/Up
			if (Fl::event_state(FL_ALT)) {
				reason_ = IR_UP;
				do_callback();
				return 1;
			}
		case FL_Down:
			// ALT/Down
			if (Fl::event_state(FL_ALT)) {
				reason_ = IR_DOWN;
				do_callback();
				return 1;
			}
		}
		return Fl_Input_Choice::handle(event);
	case FL_PASTE: {
		// After default pasting sction want to retain focus
		int result = input()->handle(event);
		take_focus();
		return result;
	}
	default:
		// Do normal handling
		return Fl_Input_Choice::handle(event);
	}
}

// Intercept the value to re-populate choice
const char* field_input::value() {
	if (is_string(field_name_)) populate_case_choice();

	return Fl_Input_Choice::value();
}

// Intercept the value to re-populate choice
void field_input::value(const char* v) {
	Fl_Input_Choice::value(v);
	if (is_string(field_name_)) populate_case_choice();
}

// 
void field_input::value(int i) {
	Fl_Input_Choice::value(i);
}

// Set QSO
void field_input::qso(record* q) {
	qso_ = q;
}

// Set the fielname - results in choice being populated from
// enumeration values if it is one
// Otherwise widget behaves as an input
void field_input::field_name(const char* field_name, record* qso /*= nullptr*/) {
	field_name_ = field_name;
	qso_ = qso;
	string enum_name;
	// Note MODE is a special case as the enumeration list is that of MODE and SUBMODE combined
	if (field_name_ == "MODE") enum_name = "Combined";
	else enum_name= spec_data_->enumeration_name(field_name_, qso);
	if (enum_name.length()) {
		// Use enumeration values
		populate_choice(enum_name);
		use_menubutton_ = true;
	}
	else if (is_string(field_name_)) {
		// Use supply case variants of the value
		populate_case_choice();
		use_menubutton_ = true;
	} else {
		// No choice - mask menu button
		clear();
		use_menubutton_ = false;
	}
	redraw();
}

// Return the field name 
const char* field_input::field_name() {
	return field_name_.c_str();
}

// Populate the choice with enumeration values
void field_input::populate_choice(string name) {
	// Get the dataset
	spec_dataset* dataset = spec_data_->dataset(name);
	// Initialsie
	clear();
	// Add a blank line for "write-in" values
	add("");

	// For all dataset
	bool hierarchic = false;
	if (dataset->data.size() > 20) hierarchic = true;
	auto it = dataset->data.begin();
	Fl_Menu_Button* menu = menubutton();
	for (int i = 1; it != dataset->data.end(); i++) {
		string menu_entry = escape_menu(it->first);
		string summary = spec_data_->summarise_enumaration(name, menu_entry);
		if (hierarchic) {
			string temp = menu_entry;
			menu_entry = temp.substr(0, 1) + "/" + temp;
		}
		string value = menu_entry;
		if (summary.length()) {
			menu_entry += comment_marker_ + escape_menu(summary);
		}
		menu->add(menu_entry.c_str(), 0, nullptr, (void*)value.c_str());
		add(menu_entry.c_str());
		it++;
	}
}

// Poulate case choice
void field_input::populate_case_choice() {
	clear();
	const char* src = Fl_Input_Choice::value();
	// Generate upper-, lower- and mixed-case versions of the input value
	int len = strlen(src);
	Fl_Menu_Button* menu = menubutton();
	if (len > 0) {
		char* dst = new char[len * 3];
		// upper-case
		memset(dst, 0, len * 3);
		fl_utf_toupper((unsigned char*)src, len, dst);
		add(dst);
		// menu->add(dst, 0, 0, (void*)dst);
		// lower-case
		memset(dst, 0, len * 3);
		fl_utf_tolower((unsigned char*)src, len, dst);
		add(dst);
		// menu->add(dst, 0, 0, (void*)dst);
		// mixed-case
		memset(dst, 0, len * 3);
		bool mixed_upper = true;
		bool prev_upper = true;
		char* temp = dst;
		// TODO move this to utils as replicated from log_table
		for (unsigned int i = 0; i < (unsigned)len; ) {
			int num_utf8_bytes;
			// Get the next UTF-8 character 
			unsigned int ucs = fl_utf8decode(src + i, src + len, &num_utf8_bytes);
			// Step to the next UTF-8 character
			i += num_utf8_bytes;
			// Convert case
			unsigned int new_ucs;
			if (mixed_upper) {
				new_ucs = fl_toupper(ucs);
			}
			else {
				new_ucs = fl_tolower(ucs);
			}
			// Convert UTF-8 character to bytes, store it and step destination pointer
			temp += fl_utf8encode(new_ucs, temp);
			switch (ucs) {
			case ' ':
			case '-':
			case '.':
				// Force upper case after some punctuation
				prev_upper = mixed_upper;
				mixed_upper = true;
				break;
			case '\'':
				// Keep case prior to apostrophe
				mixed_upper = prev_upper;
				break;
			default:
				// Force lower case
				prev_upper = mixed_upper;
				mixed_upper = false;
				break;
			}
		}
		add(dst);
		// menu->add(dst, 0, 0, (void*)dst);
	}
	else {
		// Default place-holders if no text in input
		add("UPPER");
		add("lower");
		add("Mixed");
		// menu->add("UPPER", 0, 0, (void*)"UPPER");
		// menu->add("lower", 0, 0, (void*)"lower");
		// menu->add("MIxed", 0, 0, (void*)"Mixed");
	}
}

// Repopulate choice
void field_input::reload_choice(record* qso /* = nullptr */) {
	field_name(field_name_.c_str(), qso);
	qso_ = qso;
}

// Field is a string type - allow case choice
bool field_input::is_string(string field) {
	// Special case where a string is a suggested enumeation
	if (spec_data_->enumeration_name(field_name_, nullptr).length()) return false;
	char c = spec_data_->datatype_indicator(field);
	switch (c) {
	case 'S':
	case 'M':
	case 'I':
	case 'G':
		return true;
	default:
		return false;
	}
}

// Return reason
field_input::exit_reason_t field_input::reason() {
	exit_reason_t result = reason_;
	reason_ = IR_NULL;
	return result;
}

// Type - allows the input_choice to be used as an Fl_Output
void field_input::type(uchar t) {
	input()->type(t);
}

// Draw the widget
void field_input::draw() {
	if (input()->type() & FL_INPUT_READONLY) {
		menubutton()->deactivate();
	} else if (use_menubutton_) {
		menubutton()->activate();
	} else {
		menubutton()->deactivate();
	}
	Fl_Input_Choice::draw();
}

// Menu button callback
void field_input::cb_menu(Fl_Widget* w, void* v) {
	field_input* that = ancestor_view<field_input>(w);
	const char* val = that->menubutton()->text();
	const char* pos = strstr(val, comment_marker_);
	if (pos == nullptr) {
		that->input()->value(val);
	} else {
		char* nval = new char[pos - val + 1];
		memset(nval, 0, pos - val + 1);
		strncpy(nval, val, pos - val);
		that->input()->value(nval);
	}
	that->do_callback();
}
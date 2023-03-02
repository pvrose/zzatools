#include "field_choice.h"
#include "fields.h"
#include "spec_data.h"
#include "intl_dialog.h"
#include "utils.h"

#include <FL/Fl_Preferences.H>



extern spec_data* spec_data_;
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

field_input::field_input(int X, int Y, int W, int H, const char* label) :
	Fl_Input_Choice(X, Y, W, H, label)
	, field_name_("")

{
}

field_input::~field_input() {
	if (intl_dialog_ && intl_dialog_->visible()) {
		intl_dialog_->editor(nullptr);
	}
}

int field_input::handle(int event) {
	reason_ = IR_NULL;
	// Tell international character dialog to paste to this widget
	switch (event) {
	case FL_FOCUS:
		if (intl_dialog_) {
			intl_dialog_->editor(this);
		}
		return true;
	case FL_KEYBOARD:
		// TODO: Compromise to use ALT/Nav keys 
		switch (Fl::event_key()) {
		case FL_Tab:
			// Send to parent to handle
			return 0;
		case FL_Enter:
			reason_ = IR_NULL;
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
	default:
		// Do normal handling
		return Fl_Input_Choice::handle(event);
	}
}

const char* field_input::value() {
	if (is_string(field_name_)) populate_case_choice();
	return Fl_Input_Choice::value();
}

void field_input::value(const char* v) {
	Fl_Input_Choice::value(v);
	if (is_string(field_name_)) populate_case_choice();
}

void field_input::value(int i) {
	Fl_Input_Choice::value(i);
}

void field_input::field_name(const char* field_name) {
	field_name_ = field_name;
	string enum_name;
	// Note MODE is a special case as the enumeration list is that of MODE and SUBMODE combined
	if (field_name_ == "MODE") enum_name = "Combined";
	else enum_name= spec_data_->enumeration_name(field_name_, nullptr);
	if (enum_name.length()) {
		populate_choice(enum_name);
		menubutton()->show();
	}
	else if (is_string(field_name_)) {
		populate_case_choice();
		menubutton()->show();
	} else {
		clear();
		menubutton()->hide();
	}
}

const char* field_input::field_name() {
	return field_name_.c_str();
}

// Populate the choice with enumeration values
void field_input::populate_choice(string name) {
	// Get the dataset
	spec_dataset* dataset = spec_data_->dataset(name);
	// Initialsie
	clear();
	add("");

	// For all dataset
	bool hierarchic = false;
	if (dataset->data.size() > 20) hierarchic = true;
	auto it = dataset->data.begin();
	for (int i = 1; it != dataset->data.end(); i++) {
		string menu_entry = escape_menu(it->first);
		if (hierarchic) {
			string temp = menu_entry;
			menu_entry = temp.substr(0, 1) + "/" + temp;
		}
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
	if (len > 0) {
		char* dst = new char[len * 3];
		// upper-case
		memset(dst, 0, len * 3);
		fl_utf_toupper((unsigned char*)src, len, dst);
		add(dst);
		// lower-case
		memset(dst, 0, len * 3);
		fl_utf_tolower((unsigned char*)src, len, dst);
		add(dst);
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
	}
	else {
		// Default place-holders if no text in input
		add("UPPER");
		add("lower");
		add("Mixed");
	}
}

// Repopulate choice
void field_input::reload_choice() {
	field_name(field_name_.c_str());
}

// Field is a string type - allow case choice
bool field_input::is_string(string field) {
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
field_input::exit_reason_t field_input::reason() { return reason_; }
#include "field_choice.h"
#include "fields.h"
#include "spec_data.h"
#include "../zzalib/utils.h"

#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern spec_data* spec_data_;

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

field_input::~field_input() {}

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
	else {
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

// Repopulate choice
void field_input::reload_choice() {
	field_name(field_name_.c_str());
}
#include "field_choice.h"
#include "fields.h"
#include "spec_data.h"

#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern Fl_Preferences* settings_;
extern spec_data* spec_data_;

// Constructor - add selected fields (most common) from the spec database
field_choice::field_choice(int X, int Y, int W, int H, const char* label) :
	Fl_Choice(X, Y, W, H, label)
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
	for (int i = 1; it != dataset_->data.end(); i++) {
		char curr = it->first[0];
		// generate tree A/ADDRESS etc
		char entry[128];
		snprintf(entry, 128, "%c/%s", curr, it->first.c_str());
		add(entry, 0, (Fl_Callback*)nullptr);
		it++;
	}
	value(default_field.c_str());
}

// Get the character string at the selected item
const char* field_choice::value() {
	char* result = new char[128];
	char temp[128];
	item_pathname(temp, 128);
	// If there is a value get its leaf text.
	if (strlen(temp) && temp[0] != 0) {
		const char* last_slash = strrchr(temp, '/');
		strcpy(result, last_slash + 1);
	}
	else {
		strcpy(result, temp);
	}
	return result;
}

// Select the item with the leaf name
void field_choice::value(const char* field) {
	if (field == nullptr || strlen(field) == 0) {
		Fl_Choice::value(0);
	}
	else {
		char actual[128];
		snprintf(actual, 128, "%c/%s", field[0], field);
		Fl_Choice::value(find_index(actual));
	}
}


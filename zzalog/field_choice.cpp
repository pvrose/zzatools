#include "field_choice.h"
#include "fields.h"
#include "spec_data.h"

#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern Fl_Preferences* settings_;
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
			char curr = it->first[0];
			// generate tree A/ADDRESS etc
			char entry[128];
			snprintf(entry, 128, "%c/%s", curr, it->first.c_str());
			add(entry, 0, (Fl_Callback*)nullptr);
			it++;
		}
	}
	else {
		hierarchic_ = false;
		for (int i = 1; it != dataset_->data.end(); i++) {
			char curr = it->first[0];
			// generate tree A/ADDRESS etc
			add(it->first.c_str(), 0, (Fl_Callback*)nullptr);
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
	Fl_Group(X, Y, W, H, label)
	, field_name_("")

{
	ip_ = new intl_input(X, Y, W, H, nullptr);
	ip_->callback(cb_ip);
	ch_ = new field_choice(X, Y, W, H, nullptr);
	ch_->callback(cb_ch);
	end();

	show_widget();
}

field_input::~field_input() {}

void field_input::field_name(const char* field_name) {
	field_name_ = field_name;
	show_widget();
}

const char* field_input::field_name() {
	return field_name_.c_str();
}

// Overloaded value 
const char* field_input::value() {
	return value_;
}

// Set the value into both wodgets
void field_input::value(const char* v) {
	value_ = v;
	ip_->value(value_);
	ch_->value(value_);
}

// The two callbacks - as Fl_Input
void field_input::cb_ip(Fl_Widget* w, void* v) {
	Fl_Input* ip = (Fl_Input*)w;
	field_input* that = ancestor_view<field_input>(w);
	that->value_ = ip->value();
	that->do_callback();
}
// and as field_choice
void field_input::cb_ch(Fl_Widget* w, void* v) {
	field_choice* ch = (field_choice*)w;
	field_input* that = ancestor_view<field_input>(w);
	that->value_ = ch->value();
	that->do_callback();
}

// Show the respective widget - use choice if it's an enumertaion, else use input.
Fl_Widget* field_input::show_widget() {
	menu_data_ = spec_data_->enumeration_name(field_name_, nullptr);
	if (menu_data_.length() || field_name_ == "MY_RIG" ||
		field_name_ == "MY_ANTENNA" || field_name_ == "APP_ZZA_QTH" ||
		field_name_ == "STATION_CALLSIGN") {
		if (menu_data_.length() == 0) {
			menu_data_ = field_name_;
		}
		ip_->hide();
		ch_->show();
		populate_choice(menu_data_);
		return ch_;
	}
	else {
		ip_->show();
		ch_->hide();
		return ip_;
	}
}

// Populate the choice with enumeration values
void field_input::populate_choice(string name) {
	ch_->clear();
	if (name == "MY_RIG" || name == "MY_ANTENNA" || name == "APP_ZZA_QTH" || name == "STATION_CALLSIGN") {
		// Add the list of possible values from the appropriate settings
		Fl_Preferences station_settings(settings_, "Stations");
		string setting_path;
		if (name == "MY_RIG") setting_path = "Rigs";
		else if (name == "MY_ANTENNA") setting_path = "Aerials";
		else if (name == "APP_ZZA_QTH") setting_path = "QTHs";
		else if (name == "STATION_CALLSIGN") setting_path = "Callsigns";
		Fl_Preferences kit_settings(station_settings, setting_path.c_str());
		if (name == "STATION_CALLSIGN") {
			int num_items;
			kit_settings.get("Number Callsigns", num_items, 0);
			char temp[10];
			char* value;
			for (int i = 1; i <= num_items; i++) {
				snprintf(temp, 10, "Call%d", i);
				kit_settings.get(temp, value, "");
				ch_->add(value);
				free(value);
			}
		}
		else {
			int num_items = kit_settings.groups();
			for (int i = 0; i < num_items; i++) {
				ch_->add(kit_settings.group(i));
			}
		}
		ch_->hierarchic(false);
	}
	else {
		// Add the list of possible values from the data for the enumeration
		ch_->set_dataset(name);
	}
}

// Repopulate choice
void field_input::reload_choice() {
	// Remember current selection
	const char* value = ch_->value();
	// Add new list
	populate_choice(menu_data_);
	// Restore remebered value
	ch_->value(value);
}
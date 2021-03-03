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
	repopulate(false, "");
}

// Destructor
field_choice::~field_choice()
{
	clear();
}

// Repopulate the choice with all or most popular field names
void field_choice::repopulate(bool all_fields, string default_field) {
	clear();
	if (all_fields) {
		// Use the default method
		spec_data_->initialise_field_choice(this, "Fields", default_field);
	}
	else {
		// Initially add a blank item
		add("");
		value(0);
		Fl_Preferences display_settings(settings_, "Display");
		Fl_Preferences fields_settings(display_settings, "Fields");
		char* field_set;
		// Get the list of field names from settings
		fields_settings.get("App5", field_set, "Default");
		Fl_Preferences set_settings(fields_settings, field_set);
		free(field_set);
		if (set_settings.groups() > 0) {
			// We have defined a list in settings so use that
			for (int i = 0; i < set_settings.groups(); i++) {
				Fl_Preferences field_settings(set_settings, set_settings.group(i));
				char* name;
				field_settings.get("Name", name, "");
				add(name, 0, (Fl_Callback*)nullptr);
				if (string(name) == default_field) {
					value(i + 1);
				}
				free(name);
			}
		}
		else {
			// otherwise use the default list
			for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
				string name = DEFAULT_FIELDS[i].field;
				add(name.c_str(), 0, (Fl_Callback*)nullptr);
			}
		}
	}
}

#include "tsv_writer.h"
#include "fields.h"

#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern Fl_Preferences* settings_;

// Constructor
tsv_writer::tsv_writer()
{
}

// Destructor
tsv_writer::~tsv_writer()
{
}

// write book to output stream - only write specified fields
bool tsv_writer::store_book(book* book, ostream& out, set<string>* fields) {
	set<string>* my_fields;
	// If the fields parameter is not specified - use a default set
	if (fields == nullptr) {
		my_fields = new set<string>;
		default_fields(my_fields);
	}
	else {
		my_fields = fields;
	}
	// Create the header record, for each field - output field names separated by tab character
	for (auto it = my_fields->begin(); it != my_fields->end(); it++) {
		if (it != my_fields->begin()) {
			out << '\t';
		}
		out << *it;
	}
	out << endl;
	// For each record - write it to the output stream
	for (auto it = book->begin(); it != book->end() && out.good(); it++) {
		store_record(*it, out, my_fields);
	}
	return out.good();
}

// write record to output stream
ostream & tsv_writer::store_record(record* record, ostream& out, set<string>* fields) {
	// For each field, output it separated by the tab character
	for (auto it = fields->begin(); it != fields->end(); it++) {
		// Add the tab character before all fields except the first one
		if (it != fields->begin()) {
			out << '\t';
		}
		// Generate specific QSL Message otherwise fetch indirect form of item
		if (*it == "QSLMSG") {
			Fl_Preferences qsl_settings(settings_, "QSL");
			Fl_Preferences card_settings(qsl_settings, "Card");
			char* message;
			if (record->item("SWL") == "Y") {
				card_settings.get("SWL Message", message, "Tnx SWL Report <NAME>, 73");
			}
			else {
				card_settings.get("QSL Message", message, "Tnx QSO <NAME>, 73");
			}
			out << record->item_merge(string(message));
			free(message);
		}
		else {
			out << record->item(*it, false, true);
		}
	}
	out << endl;
	return out;
}

// Default fields
void tsv_writer::default_fields(set<string>* fields) {
	// Read the field data from the settings
	Fl_Preferences display_settings(settings_, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	// Then get the field set for the application
	char app_path[128];
	char* field_set_name;
	sprintf(app_path, "App%d", FO_EXPORTTSV);
	fields_settings.get(app_path, field_set_name, "Default");
	Fl_Preferences field_set_settings(fields_settings, field_set_name);
	int num_fields = field_set_settings.groups();
	// now get the fields in the set
	// For each field
	for (int j = 0; j < num_fields; j++) {
		// Get the field name
		string field_id = field_set_settings.group(j);
		int field_num = stoi(field_id.substr(6)); // "Field n"
		Fl_Preferences field_settings(field_set_settings, field_id.c_str());
		char * temp;
		field_settings.get("Name", temp, "");
		fields->insert(string(temp));
		free(temp);
	}
	free(field_set_name);
}

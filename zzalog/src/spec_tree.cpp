#include "spec_tree.h"

#include "utils.h"
#include "tabbed_forms.h"
#include "callback.h"
#include "status.h"

#include <map>
#include <string>
#include <vector>

#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>




extern spec_data* spec_data_;
extern tabbed_forms* tabbed_forms_;
extern status* status_;
extern Fl_Preferences* settings_;
extern bool DARK;

using namespace std;

// Constructor
spec_tree::spec_tree(int X, int Y, int W, int H, const char* label, field_ordering_t app) :
	Fl_Tree(X, Y, W, H, label),
	view()
	, pas_item_(nullptr)
	, sas_item_(nullptr)
	, sub_item_(nullptr)
{
	// Tree parameters
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences tree_settings(user_settings, "Tree Views");
	tree_settings.get("Font Name", (int&)font_, 0);
	tree_settings.get("Font Size", (int&)fontsize_, FL_NORMAL_SIZE);

	sortorder(FL_TREE_SORT_ASCENDING);
	item_labelfont(font_);
	item_labelsize(fontsize_);
	// item_labelfgcolor(fl_contrast(FL_FOREGROUND_COLOR, item_labelbgcolor()));
	// Call back standard tree callback
	callback(cb_tree);
}

// Destructor
spec_tree::~spec_tree()
{
	delete_tree();
}

// inherited from view
void spec_tree::update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2) {
	switch (hint) {
	case HT_FORMAT:
		// May have changed font - redraw
		item_labelfont(font_);
		item_labelsize(fontsize_);
		populate_tree(false);
		break;
	default:
		// Do nothing
		break;
	}
}

// Delete the tree
void spec_tree::delete_tree() {
	clear();
}

// Populate the tree control with the data from the selected prefixes
void spec_tree::populate_tree(bool activate) {
	// Lengthy operation - so put the timer cursor on
	fl_cursor(FL_CURSOR_WAIT);
	// Add the root
	// now build the tree
	clear();
	// Set root item parameters - font and title
	Fl_Tree_Item* root_item = new Fl_Tree_Item(this);
	root(root_item);
	root_item->labelfont(item_labelfont() | FL_BOLD);
	char spec_label[128];
	snprintf(spec_label, 128, "ADIF Specification version %s", spec_data_->adif_version().c_str());
	root_label(spec_label);
	// Special treatment for PAS, SAS and SUBMODE
	pas_item_ = add("Primary_Administrative_Subdivision");
	pas_item_->labelfont(item_labelfont() | FL_BOLD);
	sas_item_ = add("Secondary_Administrative_Subdivision");
	sas_item_->labelfont(item_labelfont() | FL_BOLD);
	sub_item_ = add("Submode");
	sub_item_->labelfont(item_labelfont() | FL_BOLD);
	status_->misc_status(ST_NOTE, "ADIF SPEC: Display started");
	status_->progress(spec_data_->size(), OT_ADIF, "Converting ADIF specification database into report tree", "datasets");
	// Add all the datasets
	int i = 0;
	for (auto it = spec_data_->begin(); it != spec_data_->end(); it++, i++) {
		// Add it to the tree
		insert_adif_spec(nullptr, *(it->second), it->first);
		status_->progress(i, OT_ADIF);
	}
	status_->progress(spec_data_->size(), OT_ADIF);
	status_->misc_status(ST_OK, "ADIF SPEC: Display done!");
	// Close PAS and SAS items
	pas_item_->close();
	sas_item_->close();
	sub_item_->close();
	// Update display
	show();
	redraw();
	status_->misc_status(ST_OK, "SPEC: Specification displayed");
	// Restore cursor
	fl_cursor(FL_CURSOR_DEFAULT);
	// Tell tabbed view to activate this view
	if (activate) {
		tabbed_forms_->activate_pane(OT_ADIF, true);
	}
}

// Add the ADIF spec dataset to the tree
void spec_tree::insert_adif_spec(Fl_Tree_Item* parent, const spec_dataset& dataset, const string& name) {
	bool subdivision = false;
	bool submode = false;
	// Hang item name.
	Fl_Tree_Item* hang_item;
	// If item needs hanging at PAS, SAS or SUBMODE level
	if (parent == nullptr) {
		if (name.substr(0, 7) == "Primary") {
			hang_item = pas_item_;
			subdivision = true;
		}
		else if (name.substr(0, 9) == "Secondary") {
			hang_item = sas_item_;
			subdivision = true;
		}
		else if (name.substr(0, 7) == "Submode") {
			hang_item = sub_item_;
			submode = true;
		}
		else {
			// Create a top level tree item
			hang_item = add(escape_string(name, "\\/").c_str());
		}
		if (subdivision || submode) {
			// Find the split between the subdivision name and DXCC code and get the DXCC code
			size_t pos_open = name.find('[');
			size_t pos_close = name.find(']');
			string item = name.substr(pos_open + 1, pos_close - pos_open - 1);
			if (subdivision) {
				// Get the DXCC Entity dataset
				spec_dataset* dxcc_dataset = spec_data_->dataset("DXCC_Entity_Code");
				// Get the specific DXCC entry

				map<string, string>* dxcc_data;
				if (dxcc_dataset->data.find(item) == dxcc_dataset->data.end()) {
					item += " Deleted";
					if (dxcc_dataset->data.find(item) != dxcc_dataset->data.end()) {
						dxcc_data = dxcc_dataset->data.at(item);
					}
					else {
						status_->misc_status(ST_FATAL, "ADIF SPEC: Error in specification data");
					}
				}
				else {
					dxcc_data = dxcc_dataset->data.at(item);
				}
				string dxcc_name = (*dxcc_data)["Entity Name"];
				// Make the tree label just the DXCC code in a dark grey
				hang_item = hang_item->add(prefs(), dxcc_name.c_str());
				hang_item->labelcolor(COLOUR_GREY);
			}
			else {
				hang_item = hang_item->add(prefs(), item.c_str());
				hang_item->labelcolor(COLOUR_GREY);
			}
		}
		// They all hang on the parent
		hang_item->labelfont(item_labelfont() | FL_BOLD);
	}
	else {
		hang_item = parent;
	}
	// For all entries in the dataset
	for (auto it = dataset.data.begin(); it != dataset.data.end(); it++) {
		// Entry name and data items for entry
		string entry = it->first;
		if (name == "DXCC_Entity_Code") {
			// Special case for DXCC - numerical order not string order 
			switch (entry.length()) {
			case 1:
				entry = "  " + entry;
				break;
			case 2:
				entry = " " + entry;
				break;
			}
		}
		map<string, string>* entry_data = it->second;
		// Hang the entry name at the hang point
		Fl_Tree_Item* entry_item;
		entry_item = hang_item->add(prefs(), entry.c_str());
		// for each data item (indexed by column name)
		for (size_t column_ix = 0; column_ix < dataset.column_names.size(); column_ix++) {
			// Hang column name and data value as "Name: Value" in blue
			string column = dataset.column_names[column_ix];
			if (entry_data->find(column) != entry_data->end()) {
				string column_text = (*entry_data)[column];
				if (column_text.length() > 0) {
					// The data value is not an empty string
					char line[2048];
					snprintf(line, 2048, "%s: %s", column.c_str(), column_text.c_str());
					Fl_Tree_Item* column_item = entry_item->add(prefs(), line);
					column_item->labelcolor(DARK ? fl_lighter(FL_BLUE) : FL_BLUE);
					if (column == "Enumeration") {
						// Get the enumeration dataset
						spec_dataset* enum_dataset = spec_data_->dataset(column_text);
						if (enum_dataset != nullptr) {
							// If there is a dataset
							string desc_title = enum_dataset->column_names[2];
							// For all enumeration values
							for (auto it = enum_dataset->data.begin(); it != enum_dataset->data.end(); it++) {
								// Hang "Value: Description" under the column name
								string enum_desc;
								if ((*it->second).find(column_text) == (*it->second).end()) {
									enum_desc = (*it->second)[desc_title];
								}
								else {
									enum_desc = (*it->second)[column_text];
								}
								// Special treatment to get entity codes in numeric order
								if (column_text == "DXCC_Entity_Code") {
									switch (it->first.length()) {
									case 1:
										snprintf(line, 2048, "  %s: %s", it->first.c_str(), enum_desc.c_str());
										break;
									case 2:
										snprintf(line, 2048, " %s: %s", it->first.c_str(), enum_desc.c_str());
										break;
									default:
										snprintf(line, 2048, "%s: %s", it->first.c_str(), enum_desc.c_str());
										break;
									}
								}
								else {
									snprintf(line, 2048, "%s: %s", it->first.c_str(), enum_desc.c_str());
								}
								Fl_Tree_Item* enum_item = column_item->add(prefs(), line);
								enum_item->labelcolor(DARK ? FL_RED : fl_darker(FL_RED));
								enum_item->labelfont(item_labelfont() | FL_ITALIC);
							}
						}
					}
					else if (column == "Data Type") {
						// Get the Data Type dataset
						spec_dataset* type_dataset = spec_data_->dataset("Data Types");
						if (type_dataset != nullptr) {
							// If it exists - get the  data type name (may be > 1)
							vector<string> type_names;
							split_line(column_text, type_names, ',');
							// For each data type name
							for (size_t type_ix = 0; type_ix < type_names.size(); type_ix++) {
								// Get the entry for the data type
								map<string, string>* type_data = type_dataset->data[type_names[type_ix]];
								// For each item in the data type entry
								for (size_t type_col_ix = 0; type_col_ix < type_dataset->column_names.size(); type_col_ix++) {
									// Hang "Column: value" for the data type entry item
									string type_col_name = type_dataset->column_names[type_col_ix];
									string type_col_text = (*type_data)[type_col_name];
									if (type_col_text.length() > 0) {
										sprintf(line, "%s: %s", type_col_name.c_str(), type_col_text.c_str());
										Fl_Tree_Item* type_item = column_item->add(prefs(), line);
										type_item->labelcolor(DARK ? FL_GREEN : fl_darker(FL_GREEN));
										type_item->labelfont(item_labelfont() | FL_ITALIC);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	hang_item->close();
}

// Set log font values
void spec_tree::set_font(Fl_Font font, Fl_Fontsize size) {
	font_ = font;
	fontsize_ = size;
}

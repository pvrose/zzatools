#include "report_tree.h"
#include "record.h"
#include "book.h"
#include "extract_data.h"
#include "pfx_data.h"
#include "spec_data.h"
#include "tabbed_forms.h"

#include "status.h"
#include "../zzalib/callback.h"
#include "../zzalib/utils.h"
#include "menu.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>

using namespace zzalog;
using namespace zzalib;

extern book* book_;
extern extract_data* extract_records_;
extern pfx_data* pfx_data_;
extern spec_data* spec_data_;
extern tabbed_forms* tabbed_view_;
extern status* status_;
extern menu* menu_;
extern Fl_Preferences* settings_;


// Constructor
report_tree::report_tree(int X, int Y, int W, int H, const char* label, field_ordering_t app) :
	Fl_Tree(X, Y, W, H, label),
	view()
	, map_(report_map_entry_t())
	, filter_(RF_NONE)
	, selection_(0)
	, add_states_(false)
{
	map_order_.clear();
	// Set tree properties
	sortorder(FL_TREE_SORT_ASCENDING);
	item_labelfont(FONT);
	item_labelsize(FONT_SIZE);
	// Get initial filter
	Fl_Preferences report_settings(settings_, "Report");
	report_settings.get("Filter", (int&)filter_, RF_NONE);
	callback(cb_tree_report);
	delete_tree();
	// Get initial state
	Fl_Preferences level_settings(report_settings, "Levels");
	map_order_.resize(level_settings.entries());
	for (int i = 0; i < level_settings.entries(); i++) {
		level_settings.get(level_settings.entry(i), (int&)map_order_[i], -1);
	}
	// Add state level - copy selected map order adding DXCC before PAS
	adj_order_.clear();
	adj_order_.clear();
	for (size_t i = 0; i < map_order_.size(); i++) {
		if (map_order_[i] == RC_PAS) {
			adj_order_.push_back(RC_DXCC);
		}
		adj_order_.push_back(map_order_[i]);
	}
	// And reflect this is in menu
	menu_->report_mode(map_order_, filter_);

	// Minimum resizing
	min_w_ = w() / 3; // One third the width
	min_h_ = (item_labelsize() + linespacing()) * 7; // 7 lines 

}

// Destructor
report_tree::~report_tree()
{
	delete_tree();
	delete_map(&map_);
}

// Overloaded view update method
void report_tree::update(hint_t hint, unsigned int record_num_1, unsigned int record_num_2) {
	// Only use record_num_1
	selection_ = record_num_1;
	switch (hint) {
	case HT_SELECTED:
		// re-populate_tree if displaying selected only
		if (filter_ == RF_SELECTED) {
			populate_tree(false);
		}
		break;
	case HT_EXTRACTION:
		// re-populate_tree if showing extracted data
		if (filter_ == RF_EXTRACTED) {
			populate_tree(false);
		}
		break;
	case HT_CHANGED:
	case HT_ALL:
	case HT_DELETED:
	case HT_INSERTED:
	case HT_DUPE_DELETED:
	case HT_NEW_DATA:
		// Always re-populate as a substantial change has been made
		populate_tree(false);
		break;
	}
}

// Delete the tree
void report_tree::delete_tree() {
	clear();
	map_order_.clear();
}

// methods
// Add record details to a specific map entry
void report_tree::add_record(record_num_t record_num, report_map_entry_t* entry) {
	string map_key = "";
	string state_code;
	string dxcc_code;
	// Get record
	record* record = get_book()->get_record(record_num, false);
	// Entry type is valid
	bool skip_state = false;
	if (entry->entry_type != -1 && (size_t)entry->entry_type < adj_order_.size()) {
		// The user wants to display this entry type
		report_cat_t category = adj_order_[entry->entry_type];
		switch (category) {
		case RC_DXCC:
			// DXCC map
			if (record->item("SWL") == "Y") {
				// Treat SWL as a separate DXCC
				map_key = "{ SWL }"; // Forces it before alpha
			}
			else {
				// Set key to "GM: Scotland" - get the DXCC code for the record
				int dxcc_code;
				record->item("DXCC", dxcc_code);
				// Get the prefix information
				prefix* prefix = pfx_data_->get_prefix(dxcc_code);
				spec_dataset* dxcc_dataset = spec_data_->dataset("DXCC_Entity_Code");
				map<string, string>* dxcc_data;
				auto it = dxcc_dataset->data.find(record->item("DXCC"));
				if (it != dxcc_dataset->data.end()) {
					// We have an entry for the DXCC s0 build the label
					dxcc_data = it->second;
					map_key = prefix->nickname_ + " " + (*dxcc_data)["Entity Name"];
				}
				else {
					// We cannot find the DXCC entry
					map_key = prefix->nickname_ + " *** Entity Name not available ***";
				}
			}
			break;
		case RC_PAS:
			// Primary Administrative Subdivision - get the PAS and DXCC from the record
			state_code = record->item("STATE");
			dxcc_code = record->item("DXCC");
			// STATE map
			if (state_code == "") {
				// PAS not specified
				if (spec_data_->has_states(dxcc_code)) {
					map_key = "?? *** State unspecified ***";
				}
				else {
					skip_state = true;
					map_key = " *** Entity has no states ***";
				}
			}
			else {
				// Get the PAS dataset
				string pas_name = "Primary_Administrative_Subdivision[" + dxcc_code + "]";
				spec_dataset* state_dataset = spec_data_->dataset(pas_name);
				map<string, string>* state_data;
				if (state_dataset == nullptr) {
					// No PAS dataset available
					map_key = state_code + " *** State name not available ***";
				}
				else {
					// Get the entry for the PAS code
					auto it = state_dataset->data.find(state_code);
					if (it != state_dataset->data.end()) {
						// Generate tree record
						state_data = it->second;
						map_key = state_code + " " + (*state_data)["Primary Administrative Subdivision"];
					}
					else {
						// Default tree record
						map_key = state_code + " *** State name not available ***";
					}
				}
			}
			break;
		case RC_BAND:
			// Get the band from the record
			map_key = record->item("BAND");
			break;
		case RC_MODE:
			// Get the mode from the record
			map_key = record->item("MODE", true);
			break;
		}
	}
	if ((size_t)entry->entry_type < adj_order_.size()) {
		// We have further map entries to navigate
		if (skip_state) {
			// Skip this entry and hang the record at the next level
			entry->entry_type++;
			add_record(record_num, entry);
		}
		else {
			// First find the map at this entry - create a new one if one doesn't exist
			if (entry->next_entry == nullptr) {
				entry->next_entry = new report_map_t;
			}
			// now look in the map for the entry
			report_map_entry_t* next_entry;
			// Find any existing entry with the label
			auto it = entry->next_entry->find(map_key);
			if (it == entry->next_entry->end()) {
				// A map doesn't exist for this key - so create one and add it
				next_entry = new report_map_entry_t;
				next_entry->entry_type = entry->entry_type + 1;
				next_entry->entry_cat = adj_order_[entry->entry_type];
				next_entry->next_entry = nullptr;
				next_entry->record_list = nullptr;
				(*entry->next_entry)[map_key] = next_entry;
			}
			else {
				// Step to the next entry in the map
				next_entry = it->second;
			}
			// Hang the record at the new entry
			add_record(record_num, next_entry);
		}
	}
	else {
		// We are at the final map entry so add the record to the list
		if (entry->record_list == nullptr) {
			// Record list does not yet exist, create it.
			entry->record_list = new record_list_t;
		}
		// Add to the new list
		entry->record_list->push_back(record_num);
	}
}

// Copy the map to the tree control and totalise record counts
void report_tree::copy_map_to_tree(report_map_t* this_map, Fl_Tree_Item* item, int& num_records, int& num_eqsl, int& num_lotw, int& num_card, int &num_any) {
	report_map_entry_t* next_entry;
	string map_key;
	char text[1024];
	// Default format for a branch node on the tree 
	char format[] = "%s %d QSOs - Confirmed %d (%d eQSL, %d LotW, %d Card)";
	size_t count = 1;
	// For all entries at this level of the map
	for (auto it = this_map->begin(); it != this_map->end(); it++, count++) {
		// Initialise totals for this map
		int count_records = 0;
		int count_eqsl = 0;
		int count_lotw = 0;
		int count_card = 0;
		int count_any = 0;
		// Get entry in the map
		map_key = it->first;
		next_entry = it->second;
		Fl_Color label_colour;
		// Set the colour depending on the entry type
		switch (next_entry->entry_cat) {
		case RC_DXCC:
			label_colour = FL_BLACK;
			break;
		case RC_PAS:
			label_colour = fl_darker(FL_DARK3);
			break;
		case RC_BAND:
			label_colour = FL_DARK_GREEN;
			break;
		case RC_MODE:
			label_colour = FL_DARK_RED;
			break;
		}

		if (next_entry->record_list != nullptr) {
			// There is a valid record list in the entry
			// Hang a placeholder text line on the trees - key won't change so unlikely to affect sort order
			count_records = next_entry->record_list->size();
			sprintf(text, format, map_key.c_str(), count_records, 0, 0, 0);

			Fl_Tree_Item* next_item;
			if (item == nullptr) {
				next_item = add(escape_string(text, "\\/").c_str());
			}
			else {
				next_item = item->add(prefs(), text);
			}
			// Copy the record list to the tree - adding the count of records to the totals
			copy_records_to_tree(next_entry->record_list, next_item, count_records, count_eqsl, count_lotw, count_card, count_any);
			// Update the text with actual total record counts
			sprintf(text, format, map_key.c_str(), count_records, count_any, count_eqsl, count_lotw, count_card);
			next_item->label(text);
			// Item data set to say it isn't a record entry
			next_item->user_data((void*)(long)-1);
			next_item->labelcolor(label_colour);
			next_item->close();
		}
		if (next_entry->next_entry != nullptr) {
			// There is a valid map in the entry - hang a placeholder text on the tree 
			sprintf(text, format, map_key.c_str(), 0, 0, 0, 0);
			Fl_Tree_Item* next_item;
			if (item == nullptr) {
				// Top-level - need to escape slash characters
				next_item = add(escape_string(text, "\\/").c_str());
			}
			else {
				// Otherwies can just set label
				next_item = item->add(prefs(), text);
			}
			// Copy the next level down of the map to the tree
			copy_map_to_tree(next_entry->next_entry, next_item, count_records, count_eqsl, count_lotw, count_card, count_any);
			// Update the text with actual total record counts
			sprintf(text, format, map_key.c_str(), count_records, count_any, count_eqsl, count_lotw, count_card);
			next_item->label(text);
			// Item data set to say it isn't a record entry
			next_item->user_data((void*)(long)-1);
			next_item->labelcolor(label_colour);
			next_item->close();
		}
		// Totalise counts to use upwards and report progress
		num_records += count_records;
		num_eqsl += count_eqsl;
		num_lotw += count_lotw;
		num_card += count_card;
		num_any += count_any;
		// Only mark progress if top-level map
		if (item == nullptr) {
			status_->progress(count, OT_REPORT);
		}
	}
	// Only display this on top-level map
	if (item == nullptr) {
		status_->misc_status(ST_OK, "LOG: Display done!");
	}
}

// Copy the list of records in a map entry to the tree control
void report_tree::copy_records_to_tree(record_list_t* record_list, Fl_Tree_Item* item, int& num_records, int& num_eqsl, int& num_lotw, int& num_card, int& num_any) {
	if (record_list != nullptr) {
		// We have records to copy - return the number of recordsf
		num_records = record_list->size();
		char text[1024];
		// For each entry in the record list
		for (auto it = record_list->begin(); it != record_list->end(); it++) {
			// Get the record
			record_num_t record_num = *it;
			record* record = get_book()->get_record(record_num, false);
			// Strings to build up text
			string eqsl_text = "";
			string lotw_text = "";
			string card_text = "";
			string confirmed = "Unconfirmed";
			int is_confirmed = 0;
			// eQSL confirmed
			if (record->item("EQSL_QSL_RCVD") == "Y") {
				eqsl_text = "eQSL";
				confirmed = "Confirmed";
				is_confirmed = 1;
				num_eqsl++;
			}
			// LotW conformed
			if (record->item("LOTW_QSL_RCVD") == "Y") {
				lotw_text = "LotW";
				confirmed = "Confirmed";
				is_confirmed = 1;
				num_lotw++;
			}
			// Card confirmed
			if (record->item("QSL_RCVD") == "Y") {
				card_text = "Card";
				confirmed = "Confirmed";
				is_confirmed = 1;
				num_card++;
			}
			// Totalise the number of confirmed
			num_any += is_confirmed;
			// Display record summary - GM3ZZA: 20170729 16554 - Confirmed eQSL LotW Card
			sprintf(text, "%s: %s %s %s %s - %s %s %s %s",
				record->item("CALL").c_str(),
				record->item("QSO_DATE").c_str(),
				record->item("TIME_ON").c_str(),
				record->item("BAND").c_str(),
				record->item("MODE", true).c_str(),
				confirmed.c_str(), eqsl_text.c_str(), lotw_text.c_str(), card_text.c_str());
			// Hang the text on the tree in sorted order
			Fl_Tree_Item* record_item = item->add(prefs(), text);
			// Item data is the number of the record
			record_item->user_data((void*)(long)record_num);
			record_item->labelcolor(FL_BLUE);
			record_item->labelfont(item_labelfont() | FL_ITALIC);
		}
	}
}

// Create the map top-down
void report_tree::create_map() {
	// Select what records are being analysed
	switch (filter_) {
	case report_filter_t::RF_ALL:
	case report_filter_t::RF_SELECTED:
		// Select records from the main log book
		set_book(book_);
		break;
	case report_filter_t::RF_EXTRACTED:
		// Select records from the extracted records list
		set_book(extract_records_);
		break;
	case report_filter_t::RF_NONE:
		delete_map(&map_);
		return;
	}
	status_->misc_status(ST_NOTE, "LOG: Analysis started");
	status_->progress(get_book()->size(), OT_REPORT, "records");
	// Get current selected record so we can find all records with the same report item
	record* selection = book_->get_record();
	report_cat_t category = adj_order_[0];
	string selector_name;
	string field_name;
	// Get the field we use from the selected record to get our report criterion
	switch (category) {
	case RC_DXCC:
		field_name = "DXCC";
		break;
	case RC_PAS:
		field_name = "STATE";
		break;
	case RC_BAND:
		field_name = "BAND";
		break;
	case RC_MODE:
		field_name = "MODE";
		break;
	}

	if (selection != nullptr) {
		selector_name = selection->item(field_name, true);
	}
	else {
		selector_name = "";
	}
	// For each record in the book
	for (size_t i = 0; i < get_book()->size(); i++) {
		record* record = get_book()->get_record(i, false);
		if (filter_ != RF_SELECTED || record->item(field_name, true) == selector_name) {
			// If it is in the domain of the analysis - add it to the map
			add_record(i, &map_);
		}
		status_->progress(i, OT_REPORT);
	}
	status_->progress(get_book()->size(), OT_REPORT);
	status_->misc_status(ST_OK, "LOG: Analysis done!");
}

// Delete the map in a specific map entry
void report_tree::delete_map(report_map_entry_t* entry) {
	if (entry->next_entry != nullptr) {
		// We have a child map, for each entry in it, 
		for (auto it = entry->next_entry->begin(); it != entry->next_entry->end(); it++) {
			// Delete the entry and release memory 
			report_map_entry_t* next_entry = it->second;
			string map_key = it->first;
			delete_map(next_entry);
			delete next_entry;
		}
		// Tidy up
		entry->next_entry->clear();
		delete entry->next_entry;
		entry->next_entry = nullptr;
	}
	if (entry->record_list != nullptr) {
		// We have a list of records, tidy it up
		entry->record_list->clear();
		delete entry->record_list;
		entry->record_list = nullptr;
	}
}

// redraw the tree control
void report_tree::populate_tree(bool activate) {
	fl_cursor(FL_CURSOR_WAIT);
	// Only if there's a reference table set up.
	// Delete existing data, clear the tree control and recreate the data
	delete_map(&map_);
	clear();
	if (adj_order_.size() > 0) {
		// Generate the map of records
		create_map();
		if (map_.next_entry != nullptr) {
			// If we actually have data copy it to the tree control
			int count_records = 0;
			int num_eqsl = 0;
			int num_lotw = 0;
			int num_card = 0;
			int num_any = 0;
			// Define a custom root item so we can label it later
			Fl_Tree_Item* root_item = new Fl_Tree_Item(this);
			root(root_item);
			root_item->labelfont(item_labelfont() | FL_BOLD);
			root_item->labelcolor(FL_BLACK);
			// Copy the map to the tree - starting by adding top level entries to the root
			// This then iterates down to the record entries
			// Initialise progress bar
			status_->misc_status(ST_NOTE, "LOG: Display started");
			status_->progress(map_.next_entry->size(), OT_REPORT, "entries");
			copy_map_to_tree(map_.next_entry, nullptr, count_records, num_eqsl, num_lotw, num_card, num_any);
			
			// Add the root label
			char text[1028];
			string filter;
			switch (filter_) {
			case report_filter_t::RF_ALL:
				filter = "All";
				break;
			case report_filter_t::RF_EXTRACTED:
				filter = "Extracted";
				break;
			case report_filter_t::RF_SELECTED:
				filter = "Selected";
				break;
			}
			sprintf(text, "Total: %d QSOs (%s) - Confirmed %d (%d eQSL, %d LotW, %d Card)", count_records, filter.c_str(), num_any, num_eqsl, num_lotw, num_card);
			root_item->label(text);

		}
		if (activate) {
			// Switch the view to this one
			tabbed_view_->activate_pane(OT_REPORT, true);
		}
	}
	// Update the status pane if this is the active view
	update_status();
	fl_cursor(FL_CURSOR_DEFAULT);

}

// Update the status pane
void report_tree::update_status() {
	string text = "LOG: ";
	bool error = false;
	// Select on report type - add the description
	switch (filter_) {
	case RF_ALL:
		text += "ALL:";
		break;
	case RF_EXTRACTED:
		text += "Extracted QSOs:";
		break;
	case RF_SELECTED:
		if (get_book()->get_record() == NULL) {
			text += "No selected record";
			error = true;
		}
		else {
			text += "As selected QSO";
		}
		break;
	}
	// Add the ordering criteria
	for (size_t i = 0; i < adj_order_.size(); i++) {
		switch (adj_order_[i]) {
		case RC_DXCC:
			text += " DXCC";
			break;
		case RC_PAS:
			text += " States";
			break;
		case RC_BAND:
			text += " Bands";
			break;
		case RC_MODE:
			text += " Modes";
			break;
		}
	}
	// Get document to update status pane
	status_->misc_status(error ? ST_ERROR : ST_OK, text.c_str());
}

// Add filter - and redraw
void report_tree::add_filter(report_filter_t filter) {
	filter_ = filter;
	Fl_Preferences report_settings(settings_, "Report");
	report_settings.set("Filter", filter_);
	populate_tree(true);
	redraw();
}

// Add category and redraw
void report_tree::add_category(int level, report_cat_t category) {
	// Check validity
	bool valid = true;
	// Depending on level we have different actions
	switch (level) {
	case 1:
		// Changing first level - clear others
		if (map_order_.size() > 1) {
			status_->misc_status(ST_WARNING, "LOG: Changing level 1 removes levels 2 and 3");
		} 
		break;
	case 2:
		// Changing second level - see status messages for action
		if (map_order_.size() == 0) {
			status_->misc_status(ST_ERROR, "LOG: Cannot change level 2 as level 1 has not been defined");
			valid = false;
		}
		else if (map_order_[0] & category) {
			status_->misc_status(ST_ERROR, "LOG: Cannot have the same category at more than 1 level");
			valid = false;
		}
		else if (map_order_.size() > 2) {
			status_->misc_status(ST_WARNING, "LOG: Changing level 2 removes level 3");
		}
		break;
	case 3:
		// Changing third level - see status messages for action
		if (map_order_.size() < 2) {
			status_->misc_status(ST_ERROR, "LOG: Cannot change level 3 as level 2 has not been defined");
			valid = false;
		}
		else if (map_order_[0] & category || map_order_[1] & category) {
			status_->misc_status(ST_ERROR, "LOG: Cannot have the same category at more than 1 level");
			valid = false;
		}
		break;
	}
	if (valid) {
		// The category selection was valid
		// Remove no-longer-wanted levels
		map_order_.resize(level - 1);
		switch (category) {
		case RC_EMPTY:
			// We have already removed it above
			break;
		default:
			// Add the new level
			map_order_.push_back(category);
			break;
		}
		// Add state level - copy selected map order adding DXCC before PAS
		adj_order_.clear();
		for (size_t i = 0; i < map_order_.size(); i++) {
			if (map_order_[i] == RC_PAS) {
				adj_order_.push_back(RC_DXCC);
			}
			adj_order_.push_back(map_order_[i]);
		}
		// Update menu
		menu_->report_mode(map_order_, filter_);
		// Update settings
		Fl_Preferences report_settings(settings_, "Report");
		Fl_Preferences level_settings(report_settings, "Levels");
		level_settings.clear();
		for (size_t i = 0; i < map_order_.size(); i++) {
			level_settings.set(to_string(i).c_str(), map_order_[i]);
		}
		// Create the report
		populate_tree(true);
		redraw();
	}
}

// Click in the report
void report_tree::cb_tree_report(Fl_Widget* w, void* v) {
	report_tree* that = (report_tree*)w;
	Fl_Tree_Item* item = that->callback_item();
	// 
	switch (that->callback_reason()) {
	case FL_TREE_REASON_SELECTED:
		// If we select a record, then select that record in the book
		if ((long)item->user_data() >= 0) {
			that->get_book()->selection((record_num_t)(long)item->user_data());
			tabbed_view_->activate_pane(OT_RECORD, true);
			return;
		}
		cb_tree(w, v);
		break;
	default:
		cb_tree(w, v);
		break;
	}
}
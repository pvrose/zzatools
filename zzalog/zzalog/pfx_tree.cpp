#include "pfx_tree.h"
#include "report_tree.h"
#include "pfx_data.h"
#include "extract_data.h"
#include "book.h"
#include "status.h"
#include "tabbed_forms.h"
#include "callback.h"

#include <FL/fl_draw.H>

using namespace zzalog;

extern pfx_data* pfx_data_;
extern extract_data* extract_records_;
extern book* book_;
extern status* status_;
extern tabbed_forms* tabbed_view_;

// Constructor
pfx_tree::pfx_tree(int X, int Y, int W, int H, const char* label, field_ordering_t app) :
	Fl_Tree(X, Y, W, H, label)
	, view()
{	

	filter_ = RF_NONE;
	items_ = RI_CODE;
	include_fields_ = false;
	record_num_ = 0;
	prefixes_.clear();

	// Set tree parameters
	sortorder(FL_TREE_SORT_ASCENDING);
	item_labelfont(FONT);
	item_labelsize(FONT_SIZE);
	items_ = RI_CODE;
	filter_ = RF_ALL;
	callback(cb_tree);

	// Minimum resizing
	min_w_ = w() / 3; // One third the width
	min_h_ = (item_labelsize() + linespacing()) * 7; // 7 lines 
}

// Destructor
pfx_tree::~pfx_tree()
{
	delete_tree();
	prefixes_.clear();
}

// Delete the tree.
void pfx_tree::delete_tree() {
	clear();
}

// inherited from view - redraw the view if necessary
void pfx_tree::update(hint_t hint, unsigned int record_num_1, unsigned int record_num_2) {
	// Select on hint type
	switch (hint) {
	case HT_SELECTED:
	case HT_EXTRACTION:
	case HT_CHANGED:
	case HT_ALL:
	case HT_DELETED:
	case HT_DUPE_DELETED:
	case HT_INSERTED:
	case HT_FORMAT:
	case HT_IMPORT_QUERY:
	case HT_IMPORT_QUERYNEW:
	case HT_DUPE_QUERY:
		// Something has changed
		record_num_ = record_num_1;
		// Only redraw if we have are only displaying limited number of prefixes
		if (filter_ == RF_EXTRACTED || filter_ == RF_SELECTED) {
			populate_tree(false);
		}
		break;

	default:
		// Do nothing
		break;
	}
}

// Populate the tree control with the data from the selected prefixes
void pfx_tree::populate_tree(bool activate) {
	// Set the "hourglass" cursor
	fl_cursor(FL_CURSOR_WAIT);

	bool bottom_up = false;
	// Delete current contents
	prefixes_.clear();
	delete_tree();

	// Get selected record
	record* record = book_ == nullptr ? nullptr : book_->get_record(record_num_, false);

	// Get the items to display
	switch (items_) {
	case RI_CODE:
	case RI_NAME:
	case RI_NICK:
		// Get the list of prefixes to display
		switch (filter_) {
		case RF_NONE:
			// No prefixes to display
			break;
		case RF_ALL:
			// Display all prefixes - can build the tree top-down
			bottom_up = false;
			// For each prefix record
			for (auto it = pfx_data_->begin(); it != pfx_data_->end(); it++) {
				// Add it to the draw list 
				prefixes_.push_back(it->second);
			}
			break;
		case RF_ALL_CURRENT:
			// do nothing - not supported in pfx_tree
			break;
		case RF_EXTRACTED:
			// Display prefixes in the extracted list - have to build tree bottom-up
			bottom_up = true;
			for (auto it = extract_records_->begin(); it != extract_records_->end(); it++) {
				// Add twice - once geographies, then specials
				add_prefix(*it, false);
				add_prefix(*it, true);
			}
			break;
		case RF_SELECTED:
			// Display prefixes only for the selected call - have to build bottom-yp
			bottom_up = true;
			// Get prefixes for current call
			add_prefix(record, false);
			add_prefix(record, true);
			break;
		default:
			// Do nothing
			break;
		}
		break;
	}

	// Now build the tree
	clear();

	// Set the label on the root - first need a root
	Fl_Tree_Item* root_item = new Fl_Tree_Item(prefs());
	root(root_item);
	root_item->labelfont(item_labelfont() | FL_BOLD);
	// Generate the text to add as root label
	string item_desc;
	string filter_desc;
	string detail_desc;
	// How it is displayed
	switch (items_) {
	case RI_CODE:
		item_desc = "DXCC code (Prefix: Name)";
		break;
	case RI_NAME:
		item_desc = "Name (DXCC code: Prefix)";
		break;
	case RI_NICK:
		item_desc = "Prefix (DXCC code: Name)";
		break;
	}
	// What records are displayed
	switch (filter_) {
	case RF_ALL:
		filter_desc = " ... All records";
		break;
	case RF_EXTRACTED:
		filter_desc = " ... Extracted records";
		break;
	case RF_SELECTED:
		filter_desc = " ... Selected record (" + record->item("CALL") + ")";
		break;
	case RF_NONE:
		filter_desc = " ... No records";
		break;
	}
	// Fields are inluded or not
	if (include_fields_) {
		detail_desc = " ... Full information";
	}
	else {
		detail_desc = " ... Prefixes only";
	}
	// Set it
	root_label(string(item_desc + filter_desc + detail_desc).c_str());

	// Initial status
	status_->misc_status(ST_NOTE, "PFX DATA: Display started");
	status_->progress(prefixes_.size(), OT_PREFIX, "prefixes");
	// For all prefixes that we want to display
	int i = 0;
	for (auto it = prefixes_.begin(); it != prefixes_.end(); it++, i++) {
		if (bottom_up) {
			// Get the prefix and affix to the end of the tree - which will then be built up onto it.
			Fl_Tree_Item* parent;
			insert_parent(parent, *it);
		}
		else {
			// Insert the prefix to the root of the tree (which will then be built down to it).
			insert_child(nullptr, *it);
		}
		status_->progress(i);
	}

	status_->misc_status(ST_OK, "PFX DATA: Display done!");
	// Restore cursor
	fl_cursor(FL_CURSOR_DEFAULT);
	if (filter_ != RF_NONE && activate) {
		tabbed_view_->activate_pane(OT_PREFIX, true);
	}
}

// Add a prefix at the indicated item handle to the tree top-down
void pfx_tree::insert_child(Fl_Tree_Item* parent, prefix* prefix) {
	vector<string> data;
	// Generate lines to add
	get_data(prefix, data);
	Fl_Tree_Item* this_item;
	if (parent == nullptr) {
		// Add to root - escape any \ or /
		this_item = add(escape_string(data[0], "\\/").c_str());
	}
	else {
		// Hang the first line at the supplied hang-point
		this_item = parent->add(prefs(), data[0].c_str());
	}
	if (prefix->depth_ == 0) {
		// For DXCC entities - draw BLACK BOLD
		this_item->labelcolor(FL_BLACK);
		this_item->labelfont(item_labelfont() | FL_BOLD);
	}
	else {
		// For non-DXCC prefixes - draw BLACK
		this_item->labelcolor(FL_BLACK);
	}
	// For each line in the rest of data
	for (size_t i = 1; i < data.size(); i++) {
		// Hang it on the prefix item - BLUE ITALIC
		Fl_Tree_Item* child = this_item->add(prefs(), data[i].c_str());
		child->labelcolor(FL_BLUE);
		child->labelfont(item_labelfont() | FL_ITALIC);
	}
	// For each child of the prefix
	for (auto it = prefix->children_.begin(); it != prefix->children_.end(); it++) {
		// Hang it on the prefix item
		insert_child(this_item, *it);
	}
	// Tidy up
	data.clear();
	this_item->close();

}

// Add a prefix at the indicated item handle in the tree bottom-up
void pfx_tree::insert_parent(Fl_Tree_Item*& child, prefix* prefix) {
	vector<string> data;
	get_data(prefix, data);
	// See if the prefix has already been added
	child = get_item(nullptr, data[0]);
	if (child == NULL) {
		// It hasn't...
		if (prefix->parent_ != NULL) {
			// Insert the parent
			Fl_Tree_Item* parent = nullptr;
			insert_parent(parent, prefix->parent_);
			// Hang the child on the parent
			child = parent->add(prefs(), data[0].c_str());
			child->labelcolor(FL_BLACK);
		}
		else {
			// This is a top level so will hang it at tree root
			child = add(escape_string(data[0], "\\/").c_str());
			child->labelcolor(FL_BLACK);
			child->labelfont(item_labelfont() | FL_BOLD);
		}
		// For the remaining lines in the data
		for (size_t i = 1; i < data.size(); i++) {
			// Hang them on this item
			Fl_Tree_Item* item = child->add(prefs(), data[i].c_str());
			item->labelcolor(FL_BLUE);
			item->labelfont(item_labelfont() | FL_ITALIC);
		}
	}
	// Tidy up
	data.clear();
	child->close();
}

// Scan the tree from the indicated item to get the item below it matches the text 
Fl_Tree_Item* pfx_tree::get_item(Fl_Tree_Item* item, string text) {
	// Look in the child items and return when we find it
	Fl_Tree_Item* child;
	if (item == nullptr) {
		// returns the first item in the tree
		child = first();
	}
	else if (item->children() > 0) {
		// Returns the first child of the current item
		child = item->child(0);
	}
	else {
		child = nullptr;
	}
	// Continue until we run out of children to test or have found the correct item
	while (child != nullptr) {
		// Check the child'd label
		string label = child->label();
		if (text == label) {
			// This is the required item
			return child;
		}
		else {
			// Look down the generations
			Fl_Tree_Item* this_item = get_item(child, text);
			if (this_item != nullptr) {
				// This is the required place to hang
				return this_item;
			}
		}
		child = child->next_sibling();
	}
	// Not found 
	return child;
}

// Convert the prefix data into the text string displayed in the tree
void pfx_tree::get_data(prefix* prefix, vector<string>& data) {
	char text[512];
	string marker;
	data.clear();
	if (prefix->depth_ == 0) {
		// DXCC level entry
		// Add the type of entry to the text
		switch (prefix->type_) {
		case PX_DXCC_ENTITY:
			marker = "";
			break;
		case PX_REMOVED_ENTITY:
			marker = "Deleted: ";
			break;
		case PX_UNASSIGNED:
			marker = "Unassigned: ";
			break;
		case PX_UNDEFINED:
			marker = "Undefined: ";
			break;
		case PX_UNRECOGNISED:
			marker = "Unrecognised: ";
			break;
		default:
			marker = "Unknown: ";
			break;
		}

		// Generate the text to display describing the DXCC entity
		switch (items_) {
		case RI_CODE:
			sprintf(text, "%s%3d (%s: %s)", marker.c_str(), prefix->dxcc_code_, prefix->nickname_.c_str(), prefix->name_.c_str());
			break;
		case RI_NICK:
			sprintf(text, "%s (%s%d: %s)", prefix->nickname_.c_str(), marker.c_str(), prefix->dxcc_code_, prefix->name_.c_str());
			break;
		case RI_NAME:
			sprintf(text, "%s (%s%d: %s)", prefix->name_.c_str(), marker.c_str(), prefix->dxcc_code_, prefix->nickname_.c_str());
			break;
		}
	}
	else {
		// Non-DXCC entry
		switch (prefix->type_) {
		case PX_DXCC_ENTITY:
			marker = "Unexpected DXCC";
			break;
		case PX_REMOVED_ENTITY:
			marker = "Old DXCC";
			break;
		case PX_UNASSIGNED:
			marker = "Unassigned";
			break;
		case PX_UNDEFINED:
			marker = "Undefined";
			break;
		case PX_UNRECOGNISED:
			marker = "Unrecognised";
			break;
		case PX_CITY:
			marker = "City";
			break;
		case PX_GEOGRAPHY:
			marker = "Area";
			break;
		case PX_OLD_GEOGRAPHY:
			marker = "Old Area";
			break;
		case PX_OLD_PREFIX:
			marker = "Old Prefix";
			break;
		case PX_SPECIAL_USE:
			marker = "Class";
			break;
		}
		// Generate the text to describe the prefix
		switch (items_) {
		case RI_CODE:
			sprintf(text, "%s (%s: %s)", marker.c_str(), prefix->nickname_.c_str(), prefix->name_.c_str());
			break;
		case RI_NICK:
			sprintf(text, "%s %s (%s)", marker.c_str(), prefix->nickname_.c_str(), prefix->name_.c_str());
			break;
		case RI_NAME:
			sprintf(text, "%s %s (%s)", marker.c_str(), prefix->name_.c_str(), prefix->nickname_.c_str());
			break;
		}
	}
	// Add to display array
	data.push_back(text);
	// Add the fields:
	if (include_fields_) {
		char addl_text[256];
		// Add any match patterns - space separated list
		if (prefix->patterns_.size() > 0) {
			strcpy(text, " Code:");
			for (size_t i = 0; i < prefix->patterns_.size(); i++) {
				sprintf(addl_text," %s", prefix->patterns_[i].c_str());
				strcat(text, addl_text);
			}
			data.push_back(text);
		}
		// Add longitude, latitude and continent (space separated list)
		if (!isnan(prefix->latitude_) || !isnan(prefix->longitude_) || prefix->continents_.size() > 0) {
			strcpy(text, " Location:");
			if (!isnan(prefix->latitude_)) {
				double temp = abs(prefix->latitude_);
				double degrees = trunc(temp);
				double minutes = trunc((temp - degrees) * 60);
				double seconds = trunc((((temp - degrees) * 60) - minutes) * 60);
				sprintf(addl_text," Lat. %3.0f°%02.0f'%02.0f\" %s;", degrees, minutes, seconds, prefix->latitude_ > 0.0 ? "N" : "S");
				strcat(text, addl_text);
			}
			if (!isnan(prefix->longitude_)) {
				double temp = abs(prefix->longitude_);
				double degrees = trunc(temp);
				double minutes = trunc((temp - degrees) * 60);
				double seconds = trunc((((temp - degrees) * 60) - minutes) * 60);
				sprintf(addl_text," Long. %3.0f°%02.0f'%02.0f\" %s;", degrees, minutes, seconds, prefix->latitude_ > 0.0 ? "E" : "W");
				strcat(text, addl_text);
			}
			if (prefix->continents_.size() > 0) {
				strcat(text, " Cont. ");
				for (size_t i = 0; i < prefix->continents_.size(); i++) {
					sprintf(addl_text," %s", prefix->continents_[i].c_str());
					strcat(text, addl_text);
				}
				strcat(text, ";");
			}
			data.push_back(text);
		}
		// Add zones, space separated lists
		if (prefix->cq_zones_.size() > 0 || prefix->itu_zones_.size() > 0 || prefix->timezone_.length() > 0) {
			strcpy(text, " Zones:");
			if (prefix->cq_zones_.size() > 0) {
				strcat(text, " CQ");
				for (size_t i = 0; i < prefix->cq_zones_.size(); i++) {
					sprintf(addl_text," %d", prefix->cq_zones_[i]);
					strcat(text, addl_text);
				}
				strcat(text, ";");
			}
			if (prefix->itu_zones_.size() > 0) {
				strcat(text, " ITU");
				for (size_t i = 0; i < prefix->itu_zones_.size(); i++) {
					sprintf(addl_text," %d", prefix->itu_zones_[i]);
					strcat(text, addl_text);
				}
				strcat(text, ";");
			}
			if (prefix->timezone_.length() > 0) {
				sprintf(addl_text," TZ %s;", prefix->timezone_.c_str());
				strcat(text, addl_text);
			}
			data.push_back(text);
		}
		// Add validity date range
		if (prefix->valid_from_ != "19000101" || prefix->valid_to_ != "99991231") {
			if (prefix->valid_from_ == "19000101") {
				sprintf(text, " Validity: to %s;", prefix->valid_to_.c_str());
			}
			else if (prefix->valid_to_ == "99991231") {
				sprintf(text, " Validity: from %s;", prefix->valid_from_.c_str());
			}
			else {
				sprintf(text, " Validity: from %s to %s;", prefix->valid_from_.c_str(), prefix->valid_to_.c_str());
			}
			data.push_back(text);
		}
	}
}

// Add the prefix and its children to the list of prefixes to display
void pfx_tree::add_prefix(record* record, bool special) {
	// Get all prefixes for this record
	vector<prefix*> prefixes;
	pfx_data_->all_prefixes(record, &prefixes, special);
	// Add them to the list to display
	prefixes_.insert(prefixes_.end(), prefixes.begin(), prefixes.end());
	prefixes.clear();
}

// Set the filter to display - from menu
void pfx_tree::set_filter(report_filter_t filter) {
	filter_ = filter;
	populate_tree(true);
	redraw();
	// Hide the view if we don't want to display this
	if (filter == RF_NONE) {
		hide();
	}
	else {
		show();
	}
}

// Set the items to display - from menu
void pfx_tree::set_items(report_item_t items) {
	items_ = items;
	populate_tree(true);
	redraw();
}

// Add details
void pfx_tree::add_details(bool enable) {
	include_fields_ = enable;
	populate_tree(true);
	redraw();
}	


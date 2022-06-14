#include "qth_dialog.h"
#include "../zzalib/callback.h"
#include "../zzalib/rig_if.h"

#include "record.h"
#include "pfx_data.h"
#include "prefix.h"
#include "status.h"
#include "tabbed_forms.h"
#include "intl_widgets.h"
#include "qso_manager.h"
#include "dxa_if.h"

#include <set>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/fl_ask.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern rig_if* rig_if_;
extern qso_manager* qso_manager_;
extern dxa_if* dxa_if_;

const char* SETTINGS_NAME = "Stations";
const char* QTH_SETTINGS_NAME = "QTHs";

// Populate the item selector
void qth_dialog::populate_choice() {
	Fl_Input_Choice* w = (Fl_Input_Choice*)choice_;
	w->menubutton()->clear();
	w->input()->value("");
	int index = 0;
	// For each item
	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
		if (it->second || display_all_items_) {
			// If item is currently active or we want to display both active and inactive items
			// Add the item name to the choice
			w->menubutton()->add(it->first.c_str(), 0, cb_ch_stn);
			if (it->first == selected_name_) {
				// If it's the current selection, show it as such
				w->value(index);
				((Fl_Light_Button*)active_)->value(it->second);
				selected_item_ = index;
			}
			index++;
		}
	}
	w->menubutton()->textsize(FONT_SIZE);
	w->menubutton()->textfont(FONT);
	update_item();
}

// button callback - add
// Add the text value of the choice to the list of items - new name is typed into the choice
void qth_dialog::cb_bn_add(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	// Get the value in the choice
	string new_item = ((Fl_Input_Choice*)that->choice_)->value();
	// Set it active (and add it if it's not there)
	that->all_items_[new_item] = true;
	that->selected_name_ = new_item;
	that->add_item();
	that->populate_choice();
	that->redraw();
}

// button callback - delete
void qth_dialog::cb_bn_del(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	// Get the selected item name
	string item = ((Fl_Input_Choice*)that->choice_)->menubutton()->text();
	// get the name to show in its stead - if first name then use second else the previous one.
	auto it = that->all_items_.find(item);
	if (it == that->all_items_.begin()) {
		it++;
	}
	else {
		it--;
	}
	// Select the instead item - unless we are deleting the only one.
	if (it != that->all_items_.end()) {
		that->selected_name_ = it->first;
	}
	else {
		that->selected_name_ = "";
	}
	// Delete the item
	that->all_items_.erase(that->all_items_.find(item));
	that->delete_item(item);
	that->populate_choice();
	that->redraw();
}

// button callback - all/active items
// v is unused
void qth_dialog::cb_bn_all(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	that->display_all_items_ = ((Fl_Light_Button*)w)->value();
	that->populate_choice();
}

// button callback - active/deactive
// v is unused
void qth_dialog::cb_bn_activ8(Fl_Widget* w, void* v) {
	qth_dialog* that = ancestor_view<qth_dialog>(w);
	// Set the item active or inactive dependant on state of light button
	bool activate = ((Fl_Light_Button*)w)->value();
	that->all_items_[that->selected_name_] = activate;
	that->populate_choice();
	that->redraw();
}

// choice callback
// v is unused
void qth_dialog::cb_ch_stn(Fl_Widget* w, void* v) {
	// Input_Choice is a descendant of qth_dialog
	Fl_Input_Choice* ch = ancestor_view<Fl_Input_Choice>(w);
	qth_dialog* that = ancestor_view<qth_dialog>(ch);
	if (ch->menubutton()->changed()) {
		// Save current selection
		that->save_item();
		// Get the new item from the menu
		that->selected_name_ = ch->menubutton()->text();
		that->selected_item_ = ch->menubutton()->value();
		// Update the shared choice value
		ch->value(that->selected_name_.c_str());
		// Update the active button
		((Fl_Light_Button*)that->active_)->value(that->all_items_[that->selected_name_]);
		that->update_item();
	}
	else {
		// A new item has been typed in the input field
		that->selected_name_ = ch->input()->value();
		that->selected_item_ = -1;
	}
	that->redraw();
}

// QTH group constructor
qth_dialog::qth_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label) {
	// Read settings
	load_values();
	// Create the form
	create_form(X, Y);
	// Enable widgets
	enable_widgets();
	// populate the choice an set and pass parameters to callback
	// must be done after all widgets created
	populate_choice();
	// now populate the other items
	add_item();
	update_item();
}

// Destructor
qth_dialog::~qth_dialog() {
	for (auto it = all_qths_.begin(); it != all_qths_.end(); it++) {
		delete it->second;
	}
	all_qths_.clear();
}

// Get initial data from settings - additional ones for the QTH group
void qth_dialog::load_values() {
	// Get the settings for the named group
	Fl_Preferences stations_settings(settings_, SETTINGS_NAME);
	my_settings_ = new Fl_Preferences(stations_settings, QTH_SETTINGS_NAME);
	// Number of items described in settings
	int num_items = my_settings_->groups();
	// Get the current item
	char* current_item;
	int display_all;
	my_settings_->get("Current", current_item, "");
	selected_name_ = current_item;
	free(current_item);
	// Get whether to offer all or just the active ones
	my_settings_->get("Display All", display_all, (int)false);
	display_all_items_ = (bool)display_all;
	// For each item in the settings
	for (int i = 0; i < num_items; i++) {
		// Get that item's settings
		string name = my_settings_->group(i);
		Fl_Preferences item_settings(*my_settings_, name.c_str());
		int active;
		item_settings.get("Active", active, (int)false);
		all_items_[string(name)] = (bool)active;
	}

	// For each item in the settings
	for (int i = 0; i < num_items; i++) {
		// Get that item's settings
		string name = my_settings_->group(i);
		Fl_Preferences item_settings(*my_settings_, name.c_str());
		// Get the QTH details
		qth_info_t* qth = new qth_info_t;
		char* temp;
		item_settings.get("Operator Name", temp, "");
		qth->name = temp;
		free(temp);
		item_settings.get("Street", temp, "");
		qth->street = temp;
		free(temp);
		item_settings.get("Town", temp, "");
		qth->town = temp;
		free(temp);
		item_settings.get("County", temp, "");
		qth->county = temp;
		free(temp);
		item_settings.get("Country", temp, "");
		qth->country = temp;
		free(temp);
		item_settings.get("Postcode", temp, "");
		qth->postcode = temp;
		free(temp);
		item_settings.get("Locator", temp, "");
		qth->locator = temp;
		free(temp);
		item_settings.get("DXCC Id", temp, "");
		qth->dxcc_id = temp;
		free(temp);
		item_settings.get("DXCC Name", temp, "");
		qth->dxcc_name = temp;
		free(temp);
		item_settings.get("State", temp, "");
		qth->state = temp;
		free(temp);
		item_settings.get("CQ Zone", temp, "");
		qth->cq_zone = temp;
		free(temp);
		item_settings.get("ITU Zone", temp, "");
		qth->itu_zone = temp;
		free(temp);
		item_settings.get("Continent", temp, "");
		qth->continent = temp;

		free(temp);
		item_settings.get("IOTA", temp, "");
		qth->iota = temp;
		free(temp);
		all_qths_[string(name)] = qth;
	}
	// Get the current QTH settings
	current_qth_ = *all_qths_[selected_name_];
}

// create the form - additional widgets for QTH settings
void qth_dialog::create_form(int X, int Y) {
	// widget positions - rows
	const int R1 = HTEXT;
	const int H1 = HBUTTON;
	const int R2 = R1 + H1 + GAP;
	const int H2 = HBUTTON;
	const int R3 = R2 + H2 + GAP;
	const int H3 = HBUTTON;
	const int HALL = R3 + 5 * H3 + GAP;

	// widget positions - columns
	const int C1A = GAP;
	const int W1A = WEDIT;
	const int C1B = GAP;
	const int W1B = WBUTTON;
	const int C2B = C1B + W1B + GAP;
	const int W2B = WBUTTON;
	const int C3 = max(C1A + W1A, C2B + W2B) + GAP;
	const int W3 = WBUTTON;
	// Second row of columns
	const int C1C = GAP + WLABEL;
	const int W1C = WSMEDIT;
	const int C2C = C1C + W1C + GAP + WLABEL;
	const int W2C = WSMEDIT;
	const int C3C = C2C + W2C + GAP + WLABEL;
	const int W3C = WSMEDIT;
	const int WALL = max (C3 + W3 + GAP, C3C + W3C + GAP);

	const int COL[3] = { C1C, C2C, C3C };

	// Explicitly call begin to ensure that we haven't had too many ends.
	begin();

	labelsize(FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_THIN_DOWN_BOX);
	// Row 1
	// item selection choice
	Fl_Input_Choice* ch1_1 = new Fl_Input_Choice(X + C1A, Y + R1, W1A, H1);
	ch1_1->textsize(FONT_SIZE);
	ch1_1->callback(cb_value<Fl_Input_Choice, string>, (void*)&selected_name_);
	ch1_1->when(FL_WHEN_RELEASE);
	ch1_1->tooltip("Select item");
	choice_ = ch1_1;
	// Add/Modify button
	Fl_Button* bn1_3 = new Fl_Button(X + C3, Y + R1, W3, H1, "Add/Modify");
	bn1_3->labelsize(FONT_SIZE);
	bn1_3->color(fl_lighter(FL_BLUE));
	bn1_3->callback(cb_bn_add);
	bn1_3->when(FL_WHEN_RELEASE);
	bn1_3->tooltip("Add or modify the selected item");

	// Row 2
	// all items
	Fl_Light_Button* bn2_1 = new Fl_Light_Button(X + C1B, Y + R2, W1B, H2, "Show All");
	bn2_1->value(display_all_items_);
	bn2_1->selection_color(FL_GREEN);
	bn2_1->labelsize(FONT_SIZE);
	bn2_1->callback(cb_bn_all);
	bn2_1->when(FL_WHEN_RELEASE);
	bn2_1->tooltip("Show all items (not just active ones)");
	// item is active
	Fl_Light_Button* bn2_2 = new Fl_Light_Button(X + C2B, Y + R2, W2B, H2, "Active");
	// set it when the selected_item is active
	bn2_2->labelsize(FONT_SIZE);
	bn2_2->selection_color(FL_GREEN);
	bn2_2->callback(cb_bn_activ8);
	bn2_2->when(FL_WHEN_RELEASE);
	bn2_2->tooltip("Set/Clear that the item is active");
	active_ = bn2_2;
	// Delete button
	Fl_Button* bn2_3 = new Fl_Button(X + C3, Y + R2, W3, H2, "Delete");
	bn2_3->labelsize(FONT_SIZE);
	bn2_3->color(fl_lighter(FL_RED));
	bn2_3->callback(cb_bn_del);
	bn2_3->when(FL_WHEN_RELEASE);
	bn2_3->tooltip("Delete the item");

	// Additional widgets for QTHs - for each widget
	for (int i = 0; i < NUM_QTH_PARAMS; i++) {
		// Create an input
		Fl_Input* ip3;
		// X-position

		int x = COL[qth_params_[i].col];
		// Y-position
		int y = Y + R3 + qth_params_[i].row * H3;
		switch (qth_params_[i].type) {
		case INTEGER:
			// Widget is for integer input only
			ip3 = new Fl_Int_Input(x, y, W1C, H3, qth_params_[i].label);
			ip3->callback(cb_value<Fl_Int_Input, string>, qth_params_[i].v);
			break;
		case MIXED:
			// Widget is for text input
			ip3 = new intl_input(x, y, W1C, H3, qth_params_[i].label);
			ip3->callback(cb_value<intl_input, string>, qth_params_[i].v);
			break;
		case UPPER:
			// Widget to only accept upper case
			ip3 = new intl_input(x, y, W1C, H3, qth_params_[i].label);
			ip3->callback(cb_ip_upper, qth_params_[i].v);
			break;
		}
		ip3->labelsize(FONT_SIZE);
		ip3->textsize(FONT_SIZE);
		ip3->when(FL_WHEN_RELEASE);
		char temp[128];
		snprintf(temp, 128, "Enter new value for %s", qth_params_[i].label);
		ip3->copy_tooltip(temp);
		qth_info_[i] = ip3;
	}

	// resize the group accordingly
	resizable(nullptr);
	size(WALL, HALL);
	show();
	end();
}

// Save values in settings
void qth_dialog::save_values() {
	// Clear to remove deleted entries
	my_settings_->clear();
	*all_qths_[selected_name_] = current_qth_;

	my_settings_->set("Current", selected_name_.c_str());
	my_settings_->set("Display All", display_all_items_);
	int index = 0;
	// For each item
	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
		// Get item settings
		if (it->first.length()) {
			Fl_Preferences item_settings(*my_settings_, it->first.c_str());
			item_settings.set("Active", it->second);
		}
		index++;
	}
	// For each item
	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
		if (it->first.length()) {
			// Get item settings
			Fl_Preferences item_settings(*my_settings_, it->first.c_str());
			// For QTH there are additional data to save
			qth_info_t* qth = all_qths_[it->first];
			item_settings.set("Operator Name", qth->name.c_str());
			item_settings.set("Street", qth->street.c_str());
			item_settings.set("Town", qth->town.c_str());
			item_settings.set("County", qth->county.c_str());
			item_settings.set("Post Code", qth->postcode.c_str());
			item_settings.set("Country", qth->country.c_str());
			item_settings.set("Continent", qth->continent.c_str());
			item_settings.set("Locator", qth->locator.c_str());
			item_settings.set("IOTA", qth->iota.c_str());
			item_settings.set("DXCC Id", qth->dxcc_id.c_str());
			item_settings.set("DXCC Name", qth->dxcc_name.c_str());
			item_settings.set("CQ Zone", qth->cq_zone.c_str());
			item_settings.set("ITU Zone", qth->itu_zone.c_str());
			item_settings.set("State", qth->state.c_str());
		}
	}
	// Redraw views that need a location
	tabbed_forms_->update_views(nullptr, HT_LOCATION, -1);

}

// Update QTH related fields
void qth_dialog::update_item() {
	// Get the current selection
	current_qth_ = *all_qths_[selected_name_];
	// For each QTH input widget
	for (int i = 0; i < NUM_QTH_PARAMS; i++) {
		// Set the text in the widget
		((intl_input*)qth_info_[i])->value(((string*)qth_params_[i].v)->c_str());
		// Put a reference to the value location
		((intl_input*)qth_info_[i])->user_data(qth_params_[i].v);
	}
}

// Add an item
void qth_dialog::add_item() {
	// For QTHs - see if it's new
	if (all_qths_.find(selected_name_) == all_qths_.end()) {
		// Create a new QTH
		qth_info_t* qth = new qth_info_t;
		all_qths_[selected_name_] = qth;
	}
	else {
		// Copy current QTH value to this entry
		*all_qths_[selected_name_] = current_qth_;
	}
}

// Deleta an item
void qth_dialog::delete_item(string item) {
	all_qths_.erase(all_qths_.find(item));
}

// Save an item
void qth_dialog::save_item() {
	*all_qths_[selected_name_] = current_qth_;
}

// Callback that converts what is typed to upper-case
// v is pointer to the field in the QTH structure
void qth_dialog::cb_ip_upper(Fl_Widget* w, void* v) {
	cb_value<intl_input, string>(w, v);
	*(string*)v = to_upper(*(string*)v);
	((intl_input*)w)->value(((string*)v)->c_str());
}


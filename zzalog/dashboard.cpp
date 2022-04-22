#include "dashboard.h"
#include "../zzalib/callback.h"
#include "../zzalib/rig_if.h"
#include "../zzalib/serial.h"

#include "record.h"
#include "pfx_data.h"
#include "prefix.h"
#include "status.h"
#include "tabbed_forms.h"
#include "intl_widgets.h"
#include "spec_data.h"
#include "book.h"
#include "extract_data.h"
#include "menu.h"
#include "field_choice.h"
#include "dxa_if.h"
#include "band_view.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "alarm_dial.h"
#include "../zzalib//utils.h"

#include <set>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Output.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Tooltip.H>

using namespace zzalog;
using namespace zzalib;

// External declarations
extern Fl_Preferences* settings_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern rig_if* rig_if_;
extern spec_data* spec_data_;
extern book* book_;
extern extract_data* extract_records_;
extern menu* menu_;
extern dxa_if* dxatlas_;
extern band_view* band_view_;
extern tabbed_forms* tabbed_forms_;
extern import_data* import_data_;
void add_rig_if();
extern double prev_freq_;

// constructor 
dashboard::common_grp::common_grp(int X, int Y, int W, int H, const char* label, equipment_t type)
	: Fl_Group(X, Y, W, H, label)
    , active_(nullptr)
	, choice_(nullptr)
	, band_browser_(nullptr)
	, display_all_items_(false)
	, my_settings_(nullptr)
	, item_no_(0)
	, type_(type)
	, my_name_("")
	, next_name_("")

{
	if (type_ == ANTENNA) {
		settings_name_ = "Aerials";
	}
	else {
		settings_name_ = "Rigs";
	}
	all_items_.clear();
	load_values();
	create_form(X, Y);
	end();
}

// Destructor
dashboard::common_grp::~common_grp() {
	// Release all memory
	all_items_.clear();
}

// Get initial data from settings
void dashboard::common_grp::load_values() {
	// Get the settings for the named group
	Fl_Preferences stations_settings(settings_, "Stations");
	my_settings_ = new Fl_Preferences(stations_settings, settings_name_.c_str());
	// Number of items described in settings
	int num_items = my_settings_->groups();
	// Get the current item
	char * text;
	int display_all;
	my_settings_->get("Current", text, "");
	my_name_ = text;
	free(text);
	my_settings_->get("Next", text, "");
	next_name_ = text;
	free(text);
	// Get whether to offer all or just the active ones
	my_settings_->get("Display All", display_all, (int)false);
	display_all_items_ = (bool)display_all;
	all_items_.clear();
	string bands;
	// For each item in the Antenna or rig settings
	for (int i = 0; i < num_items; i++) {
		// Get that item's settings
		string name = my_settings_->group(i);
		if (name.length()) {
			Fl_Preferences item_settings(*my_settings_, name.c_str());
			char* temp;
			all_items_.push_back(name);
			item_data& info = item_info_[name];
			switch (type_) {
			case RIG:
				// Get the CAT interface parameters
				item_settings.get("Handler", (int&)info.rig_data.handler, RIG_NONE);
				item_settings.get("Polling Interval", info.rig_data.fast_poll_interval, FAST_RIG_DEF);
				item_settings.get("Slow Polling Interval", info.rig_data.slow_poll_interval, SLOW_RIG_DEF);
				{
					// Hamlib settings
					Fl_Preferences hamlib_settings(item_settings, "Hamlib");
					// Get the hamlib settings: Mfr/Model, serial port and baudrate
					hamlib_settings.get("Manufacturer", temp, "Hamlib");
					info.rig_data.hamlib_params.mfr = temp;
					free(temp);
					hamlib_settings.get("Rig Model", temp, "Dummy");
					info.rig_data.hamlib_params.model = temp;
					free(temp);
					hamlib_settings.get("Port", temp, "COM6");
					info.rig_data.hamlib_params.port_name = temp;
					free(temp);
					hamlib_settings.get("Baud Rate", temp, "9600");
					info.rig_data.hamlib_params.baud_rate = temp;
					free(temp);
					hamlib_settings.get("Override Rig Caps", (int&)info.rig_data.hamlib_params.override_caps, false);

					// Flrig settings
					Fl_Preferences flrig_settings(item_settings, "Flrig");
					// Get the Flrig settings: Host IP address, IP port and IP resource
					flrig_settings.get("Host", temp, "127.0.0.1");
					info.rig_data.flrig_params.ip_address = temp;
					free(temp);
					flrig_settings.get("Port", info.rig_data.flrig_params.port, 12345);
					flrig_settings.get("Resource", temp, "/RPC2");
					info.rig_data.flrig_params.resource = temp;
					free(temp);

					// Alarm settings
					Fl_Preferences alarm_settings(item_settings, "Alarms");
					// SWR Settings - warning and error levels
					alarm_settings.get("SWR Warning Level", info.rig_data.alarms.swr_warning, 1.5);
					alarm_settings.get("SWR Error Level", info.rig_data.alarms.swr_error, 2.0);
					// Power settings - warning levels
					// TODO: Add mode specific settings
					alarm_settings.get("Power Warning Level", info.rig_data.alarms.power_warning, 95);
					// Vdd settings - error if above or below 10% of nominal (13.8V)
					alarm_settings.get("Voltage Minimum Level", info.rig_data.alarms.voltage_minimum, (float)(13.8 * 0.85));
					alarm_settings.get("Voltage Maximum Level", info.rig_data.alarms.voltage_maximum, (float)(13.8 * 1.15));
				}

				// Carry onto next - no break

			case ANTENNA:
				// Antenna has just active flag and bands it is intended for
				// Active flag
				int active;
				item_settings.get("Active", active, (int)false);
				info.active = (bool)active;
				// Get the intended bands
				char* temp;
				item_settings.get("Intended Bands", temp, "");
				bands = temp;
				free(temp);
				split_line(bands, info.intended_bands, ';');
				break;
			// 
			}
		}
		else {
			// Default to no handler - hopefully can ignore the rest
			if (type_ == RIG) {
				item_info_[""].rig_data.handler = RIG_NONE;
			}
		}
	}
}

// create the form
// Note this assumes the appropriate common_grp is the active group
void dashboard::common_grp::create_form(int X, int Y) {
	// widget positions - rows
	const int R1 = HTEXT;
	const int H1 = HBUTTON;
	const int R2 = R1 + H1 + GAP;
	const int H2 = HBUTTON;
	const int R3 = R2 + H2 + GAP;
	const int H3 = HBUTTON;
	const int R4 = R3 + H3 + max(GAP, HTEXT);
	const int H4 = HMLIN;
	const int HALL = R4 + H4 + GAP;
	// widget positions - columns
	const int C1 = GAP;
	const int W1B = WBUTTON;
	const int col1 = C1 + W1B + GAP;
	const int W2B = WBUTTON;
	const int W1A = col1 + W2B - C1;
	const int WALL = C1 + W1A + GAP;

	dashboard* dash = ancestor_view<dashboard>(this);

	//// Explicitly call begin to ensure that we haven't had too many ends.
	//begin();
	// resize the group accordingly
	resizable(nullptr);
	resize(X, Y, WALL, HALL);

	labelsize(FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	// Row 1
	// Choice to select or add new antenna or rig
	Fl_Input_Choice* ch1_1 = new Fl_Input_Choice(X + C1, Y + R1, W1A, H1);
	ch1_1->textsize(FONT_SIZE);
	ch1_1->callback(cb_ch_stn, nullptr);
	ch1_1->when(FL_WHEN_CHANGED);
	ch1_1->tooltip("Select item");
	ch1_1->menubutton()->textfont(FONT);
	ch1_1->menubutton()->textsize(FONT_SIZE);
	choice_ = ch1_1;

	// Row 2
	// Button to decide to display all items or only active ones
	Fl_Light_Button* bn2_1 = new Fl_Light_Button(X + C1, Y + R2, W1B, H2, "Show All");
	bn2_1->value(display_all_items_);
	bn2_1->selection_color(FL_GREEN);
	bn2_1->labelsize(FONT_SIZE);
	bn2_1->callback(cb_bn_all);
	bn2_1->when(FL_WHEN_RELEASE);
	bn2_1->tooltip("Show all items (not just active ones)");
	// This antenna or rig is in active use
	Fl_Light_Button* bn2_2 = new Fl_Light_Button(X + col1, Y + R2, W2B, H2, "Active");
	// set it when the selected_item is active
	bn2_2->labelsize(FONT_SIZE);
	bn2_2->selection_color(FL_GREEN);
	bn2_2->callback(cb_bn_activ8);
	bn2_2->when(FL_WHEN_RELEASE);
	bn2_2->tooltip("Set/Clear that the item is active");
	active_ = bn2_2;

	// Row 3
	// Add or modify this antenna or rig
	Fl_Button* bn3_1 = new Fl_Button(X + C1, Y + R3, W1B, H3, (type_ != ANTENNA) ? "Add/Modify" : "Add");
	bn3_1->labelsize(FONT_SIZE);
	bn3_1->color(fl_lighter(FL_BLUE));
	bn3_1->callback(cb_bn_add);
	bn3_1->when(FL_WHEN_RELEASE);
	bn3_1->tooltip(type_ != ANTENNA ? "Add or modify the selected item" : "Add a new item - type in the selector");

	// Remove this antenna or rig
	Fl_Button* bn3_2 = new Fl_Button(X + col1, Y + R3, W2B, H2, "Remove");
	bn3_2->labelsize(FONT_SIZE);
	bn3_2->color(fl_lighter(FL_RED));
	bn3_2->callback(cb_bn_del);
	bn3_2->when(FL_WHEN_RELEASE);
	bn3_2->tooltip("Delete the item");

	// Row 4
	// Bands the antenna or rig was designed for 
	Fl_Multi_Browser* mb4_1 = new Fl_Multi_Browser(X + C1, Y + R4, W1A, H4, "Intended Bands");
	mb4_1->labelsize(FONT_SIZE);
	mb4_1->align(FL_ALIGN_TOP);
	mb4_1->textsize(FONT_SIZE);
	mb4_1->callback(cb_mb_bands);
	mb4_1->tooltip("Select the bands that the antenna is intended to be operated with");
	band_browser_ = mb4_1;

	// Now we have created all the data we can populate the choice widgets
	populate_choice();
	populate_band();

}

// Save values in settings
void dashboard::common_grp::save_values() {
	dashboard* dash = ancestor_view<dashboard>(this);

	// Clear to remove deleted entries
	my_settings_->clear();

	my_settings_->set("Current", my_name_.c_str());
	my_settings_->set("Next", next_name_.c_str());

	my_settings_->set("Display All", display_all_items_);
	int index = 0;
	// For each item
	for (auto it = item_info_.begin(); it != item_info_.end(); it++) {
		string name = (*it).first;
		item_data& info = (*it).second;
		Fl_Preferences item_settings(my_settings_, name.c_str());

		switch(type_) {
		case RIG: {
			item_settings.set("Active", info.active);
			// set the intended bands
			string bands = "";
			// Store all the bands intended to be used with this rig/antenna
			for (auto itb = info.intended_bands.begin(); itb != info.intended_bands.end(); itb++) {
				bands += *itb + ';';
			}
			item_settings.set("Intended Bands", bands.c_str());
			item_settings.set("Handler", info.rig_data.handler);
			item_settings.set("Polling Interval", info.rig_data.fast_poll_interval);
			item_settings.set("Slow Polling Interval", info.rig_data.slow_poll_interval);
			// Read all the groups
			// Hamlib settings
			Fl_Preferences hamlib_settings(item_settings, "Hamlib");
			hamlib_settings.set("Manufacturer", info.rig_data.hamlib_params.mfr.c_str());
			hamlib_settings.set("Rig Model", info.rig_data.hamlib_params.model.c_str());
			hamlib_settings.set("Port", info.rig_data.hamlib_params.port_name.c_str());
			hamlib_settings.set("Baud Rate", info.rig_data.hamlib_params.baud_rate.c_str());
			hamlib_settings.set("Override Rig Caps", info.rig_data.hamlib_params.override_caps);
			// Flrig settings
			Fl_Preferences flrig_settings(item_settings, "Flrig");
			flrig_settings.set("Host", info.rig_data.flrig_params.ip_address.c_str());
			flrig_settings.set("Port", info.rig_data.flrig_params.port);
			flrig_settings.set("Resource", info.rig_data.flrig_params.resource.c_str());
			// Alarm settings
			Fl_Preferences alarm_settings(item_settings, "Alarms");
			// SWR Settings
			alarm_settings.set("SWR Warning Level", info.rig_data.alarms.swr_warning);
			alarm_settings.set("SWR Error Level", info.rig_data.alarms.swr_error);
			// Power settings
			alarm_settings.set("Power Warning Level", info.rig_data.alarms.power_warning);
			// Vdd settings
			alarm_settings.set("Voltage Minimum Level", info.rig_data.alarms.voltage_minimum);
			alarm_settings.set("Voltage Maximum Level", info.rig_data.alarms.voltage_maximum);

			// NO BREAK
		}
		case ANTENNA:
			item_settings.set("Active", info.active);
			// set the intended bands
			string bands = "";
			// Store all the bands intended to be used with this rig/antenna
			for (auto itb = info.intended_bands.begin(); itb != info.intended_bands.end(); itb++) {
				bands += *itb + ';';
			}
			item_settings.set("Intended Bands", bands.c_str());
			break;
		}
	}
}

// Populate the item selector
void dashboard::common_grp::populate_choice() {
	Fl_Input_Choice* w = (Fl_Input_Choice*)choice_;
	w->clear();
	int index = 0;
	bool sel_found = false;
	// For each item
	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
		// Chack if the antenna or rig is in active use
		item_data& info = item_info_[(*it)];
		if (info.active || display_all_items_) {
			// If item is currently active or we want to display both active and inactive items
			// Add the item name to the choice
			if ((*it).length()) {
				w->add((*it).c_str());
				if (*it == my_name_) {
					// If it's the current selection, show it as such
					w->value((*it).c_str());
					((Fl_Light_Button*)active_)->value(info.active);
					item_no_ = index;
					sel_found = true;
				}
				index++;
			}
		}
	}
	if (!sel_found && my_name_.length()) {
		// Selected item not found. Add it to the various lists
		w->add(my_name_.c_str());
		w->value(my_name_.c_str());
		item_info_[my_name_].active = true;
	}
	w->textsize(FONT_SIZE);
	w->textfont(FONT);
}

// Populate the band selection widget
void dashboard::common_grp::populate_band() {
	// Get pointers to the widget to be populated and the top-level dialog
	Fl_Multi_Browser* mb = (Fl_Multi_Browser*)band_browser_;
	dashboard* dash = ancestor_view<dashboard>(mb);
	mb->clear();
	// Add all the possible bands (according to latest ADIF specification)
	for (auto it = dash->ordered_bands_.begin(); it != dash->ordered_bands_.end(); it++) {
		mb->add((*it).c_str());
	}
	item_data& info = item_info_[my_name_];
	auto it = info.intended_bands.begin();
	int i = 0;
	// Select all the bands the antenna is meant to be used for
	while (it != info.intended_bands.end() && i != dash->ordered_bands_.size()) {
		// Note text(i) can be null if it hasn't been set
		if (mb->text(i) && *it == mb->text(i)) {
			mb->select(i);
			it++;
		}
		i++;
	}
}

// button callback - add
// Add the text value of the choice to the list of items - new name is typed into the choice
void dashboard::common_grp::cb_bn_add(Fl_Widget* w, void* v) {
	common_grp* that = ancestor_view<common_grp>(w);
	// Get the value in the choice
	string new_item = ((Fl_Input_Choice*)that->choice_)->value();
	// Set it active (and add it if it's not there)
	that->all_items_.push_back(new_item);

	that->my_name_ = new_item;
	that->item_info_[new_item].active = true;
	that->populate_choice();
	that->redraw();
}

// button callback - delete
void dashboard::common_grp::cb_bn_del(Fl_Widget* w, void* v) {
	common_grp* that = ancestor_view<common_grp>(w);
	// Get the selected item name
	string item = ((Fl_Input_Choice*)that->choice_)->menubutton()->text();
	// Remove the item
	that->all_items_.remove(item);
	// TODO: For now display the first element
	that->my_name_ = *(that->all_items_.begin());
	// Delete the item
	that->populate_choice();
	that->redraw();
}

// button callback - all/active items
// v is unused
void dashboard::common_grp::cb_bn_all(Fl_Widget* w, void* v) {
	common_grp* that = ancestor_view<common_grp>(w);
	that->display_all_items_ = ((Fl_Light_Button*)w)->value();
	that->populate_choice();
}

// button callback - active/deactive
// v is unused
void dashboard::common_grp::cb_bn_activ8(Fl_Widget* w, void* v) {
	common_grp* that = ancestor_view<common_grp>(w);
	// Set the item active or inactive dependant on state of light button
	bool activate = ((Fl_Light_Button*)w)->value();
	that->item_info_[that->my_name_].active = activate;
	that->populate_choice();
	that->redraw();
}

// choice callback
// v is unused
void dashboard::common_grp::cb_ch_stn(Fl_Widget* w, void* v) {
	// Input_Choice is a descendant of common_grp
	Fl_Input_Choice* ch = ancestor_view<Fl_Input_Choice>(w); 
	common_grp* that = ancestor_view<common_grp>(ch);
	dashboard* dash = ancestor_view<dashboard>(that);
	if (ch->menubutton()->changed()) {
		// Get the new item from the menu
		that->item_no_ = ch->menubutton()->value();
		that->my_name_ = ch->menubutton()->text();
		item_data& info = that->item_info_[that->my_name_];
		info.active = true;
		// Update the shared choice value
		ch->value(that->my_name_.c_str());
		// Update the active button
		((Fl_Light_Button*)that->active_)->value(info.active);
		that->populate_band();
		dash->enable_widgets();
	}
	else {
		// A new item is being typed in the input field - use ADD button to process it
	}
}

// Multi-browser callback
// v is usused
void dashboard::common_grp::cb_mb_bands(Fl_Widget* w, void* v) {
	Fl_Multi_Browser* mb = ancestor_view<Fl_Multi_Browser>(w);
	common_grp* that = ancestor_view<common_grp>(mb);
	// Get the list of bands the selected antenna or rig is meant for
	vector<string>& bands = that->item_info_[that->my_name_].intended_bands;
	// Clear the list and add the bands currently selected in the browser
	bands.clear();
	for (int i = 0; i < mb->size(); i++) {
		if (mb->selected(i)) {
			bands.push_back(mb->text(i));
		}
	}
	that->redraw();
	that->save_values();
	((dashboard*)that->parent())->enable_widgets();
}

// Item choice call back
// v is value selected
void dashboard::common_grp::cb_ch_item(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ipch = ancestor_view<Fl_Input_Choice>(w);
	cb_value<Fl_Input_Choice, string>(ipch, v);
	common_grp* that = ancestor_view<common_grp>(w);
	that->save_values();
	that->populate_band();
}

string& dashboard::common_grp::name() {
	return my_name_;
}

string& dashboard::common_grp::next() {
	return next_name_;
}

dashboard::item_data& dashboard::common_grp::info() {
	return item_info_[my_name_];
}

// The main dialog constructor
dashboard::dashboard(int W, int H, const char* label) :
	Fl_Window(W, H, label)
	, rig_grp_(nullptr)
	, antenna_grp_(nullptr)
	, hamlib_grp_(nullptr)
	, flrig_grp_(nullptr)
	, norig_grp_(nullptr)
	, cat_grp_(nullptr)
	, cat_sel_grp_(nullptr)
	, alarms_grp_(nullptr)
	, baud_rate_choice_(nullptr)
	, mfr_choice_(nullptr)
	, override_check_(nullptr)
	, rig_choice_(nullptr)
	, rig_model_choice_(nullptr)
	, port_if_choice_(nullptr)
	, show_all_ports_(nullptr)
	, ch_qth_(nullptr)
	, buffer_(nullptr)
	, editor_(nullptr)
	, bn_save_(nullptr)
	, bn_cancel_(nullptr)
	, logging_mode_(LM_OFF_AIR)
	, wait_connect_(true)
	, created_(false)
	, record_(nullptr)
	, field_("")
	, font_(FONT)
	, fontsize_(FONT_SIZE)
	, enterring_record_(false)
	, all_ports_(false)
	, previous_swr_alarm_(SWR_OK)
	, previous_pwr_alarm_(POWER_OK)
	, previous_vdd_alarm_(VDD_OK)
	, current_swr_alarm_(SWR_OK)
	, current_pwr_alarm_(POWER_OK)
	, current_vdd_alarm_(VDD_OK)
	, selected_qth_("")
	, next_qth_("")
	, last_tx_swr_(1.0)
	, last_tx_pwr_(0.0)
{
	ordered_bands_.clear();
	all_qths_.clear();

	load_values();

	create_form(0,0);

	update();

	end();
	show();
}

// Destructor
dashboard::~dashboard()
{
	Fl::remove_timeout(cb_timer_clock, nullptr);
	save_values();
}

// Handle FL_HIDE and FL_SHOW to get menu to update itself
int dashboard::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		return true;
	}

	return Fl_Window::handle(event);
}

// create the form
void dashboard::create_form(int X, int Y) {

	// Used to evaluate total width and height of the window
	int max_x = X;
	int max_y = Y;
	// Used to maintain the relative positions of the groups
	int curr_x = X + GAP;
	int curr_y = Y + GAP;
	create_spad_widgets(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;
	create_use_widgets(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;
	int save_y = curr_y;
	create_cat_widgets(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x += GAP;
	curr_y = save_y;
	int save_x = curr_x;
	create_alarm_widgets(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = save_x;
	curr_y += GAP;
	create_clock_widgets(curr_x, curr_y);
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	this->resizable(nullptr);
	this->size(max_x + GAP - X, max_y + GAP - Y);
	created_ = true;
}

// Create scratchpad widgets
void dashboard::create_spad_widgets(int& curr_x, int& curr_y) {
	int max_w = 0;
	int max_h = 0;

	// Scratchpad widgets
	Fl_Group* gsp = new Fl_Group(curr_x, curr_y, 10, 10, "QSO Scratchpad");
	gsp->labelfont(FONT);
	gsp->labelsize(FONT_SIZE);
	gsp->box(FL_BORDER_BOX);
	gsp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Choice widget to select the reqiuired logging mode
	ch_logmode_ = new Fl_Choice(gsp->x() + GAP + WLLABEL, gsp->y() + HTEXT, WEDITOR - WLLABEL, HTEXT, "QSO initialisation");
	ch_logmode_->labelfont(FONT);
	ch_logmode_->labelsize(FONT_SIZE);
	ch_logmode_->textfont(FONT);
	ch_logmode_->textsize(FONT_SIZE);
	ch_logmode_->align(FL_ALIGN_LEFT);
	ch_logmode_->add("All fields blank");
	ch_logmode_->add("Current date and time, data from CAT");
	ch_logmode_->add("Current date and time, data from selected QSO");
	ch_logmode_->add("Current date and time, no other data");
	ch_logmode_->add("QSO set by external application");
	ch_logmode_->value(logging_mode_);
	ch_logmode_->callback(cb_value<Fl_Choice, logging_mode_t>, &logging_mode_);

	const int HG = (7 * HBUTTON) + (4 * HTEXT) + (3 * GAP);
	// Editor to capture keyboard entries during phone/cW QSOs
	buffer_ = new Fl_Text_Buffer(1024);
	editor_ = new spad_editor(gsp->x() + GAP, ch_logmode_->y() + ch_logmode_->h(), WEDITOR, HG);
	editor_->labelfont(FONT);
	editor_->labelsize(FONT_SIZE);
	editor_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	editor_->buffer(buffer_);
	editor_->textsize(fontsize_);
	editor_->textfont(font_);
	// The callback will be explicitly done in the handle routine of the editor
	editor_->when(FL_WHEN_NEVER);
	// Allways wrap at a word boundary
	editor_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	max_w = max(max_w, editor_->x() + editor_->w());
	max_h = max(max_h, editor_->y() + editor_->h());

	// Create the buttons - see labels and tooltips for more information
	// First we create an invisible group
	Fl_Group* g = new Fl_Group(editor_->x() + editor_->w() + GAP, editor_->y(), WBUTTON * 2, HG);
	g->box(FL_NO_BOX);

	const int col1 = g->x();
	const int col2 = col1 + WBUTTON;
	int y = g->y();

	// Button - start QSO
	bn_start_ = new Fl_Button(col1, y, WBUTTON, HBUTTON, "Start");
	bn_start_->labelsize(FONT_SIZE);
	bn_start_->labelfont(FONT);
	bn_start_->tooltip("Create a new record");
	bn_start_->callback(cb_start);
	// Button - query worked before?
	Fl_Button* bn_wb4 = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F9 - B4?");
	bn_wb4->labelsize(FONT_SIZE);
	bn_wb4->labelfont(FONT);
	bn_wb4->tooltip("Display previous QSOs");
	bn_wb4->callback(cb_wkb4);

	// Button - parse callsign
	y += bn_wb4->h();
	Fl_Button* bn_parse = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F10 - Parse");
	bn_parse->labelsize(FONT_SIZE);
	bn_parse->labelfont(FONT);
	bn_parse->tooltip("Parse selection as a callsign");
	bn_parse->callback(cb_parse);

	y += bn_parse->h() + GAP;
	// Button - copy call to record (starts QSO if not started)
	Fl_Button* bn_call = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F1 - Call");
	bn_call->labelsize(FONT_SIZE);
	bn_call->labelfont(FONT);
	bn_call->tooltip("Copy selected text to callsign field");
	bn_call->callback(cb_action, (void*)WRITE_CALL);

	// Button - copy name to record
	Fl_Button* bn_name = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F2 - Name");
	bn_name->labelsize(FONT_SIZE);
	bn_name->labelfont(FONT);
	bn_name->tooltip("Copy selected text to name field");
	bn_name->callback(cb_action, (void*)WRITE_NAME);

	y += HBUTTON;
	// Button - copy QTH to record
	Fl_Button* bn_qth = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F3 - QTH");
	bn_qth->labelsize(FONT_SIZE);
	bn_qth->labelfont(FONT);
	bn_qth->tooltip("Copy selected text to QTH field");
	bn_qth->callback(cb_action, (void*)WRITE_QTH);

	// Button - copy grid to record
	Fl_Button* bn_grid = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F4 - Grid");
	bn_grid->labelsize(FONT_SIZE);
	bn_grid->labelfont(FONT);
	bn_grid->tooltip("Copy selected text to gridsquare field");
	bn_grid->callback(cb_action, (void*)WRITE_GRID);

	y += HBUTTON;
	// Button - copy RS(T/Q) sent to record
	Fl_Button* bn_rst_sent = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F5 - Sent");
	bn_rst_sent->labelsize(FONT_SIZE);
	bn_rst_sent->labelfont(FONT);
	bn_rst_sent->tooltip("Copy selected text to RST sent field");
	bn_rst_sent->callback(cb_action, (void*)WRITE_RST_SENT);

	// Button - copy RS(T/Q) received to record
	Fl_Button* bn_rst_rcvd = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F6 - Rcvd");
	bn_rst_rcvd->labelsize(FONT_SIZE);
	bn_rst_rcvd->labelfont(FONT);
	bn_rst_rcvd->tooltip("Copy selected text to RST Received field");
	bn_rst_rcvd->callback(cb_action, (void*)WRITE_RST_RCVD);

	y += HBUTTON;
	// Button - copy selected field to record
	Fl_Button* bn_field = new Fl_Button(col1, y, WBUTTON, HBUTTON, "Field");
	bn_field->labelsize(FONT_SIZE);
	bn_field->labelfont(FONT);
	bn_field->tooltip("Copy selected text to the field specified below");
	bn_field->callback(cb_action, (void*)WRITE_FIELD);

	y += HBUTTON;
	// Choice - allow specified field to be selected
	field_choice* ch_field = new field_choice(col1, y, WBUTTON + WBUTTON, HTEXT);
	ch_field->textfont(FONT);
	ch_field->textsize(FONT_SIZE);
	ch_field->tooltip("Select the field you want the selected text to be written to");
	ch_field->callback(cb_text<Fl_Choice, string>, (void*)&field_);

	y += HTEXT + GAP;
	// Group for frequency, power and mode
	grp_fpm_ = new Fl_Group(col1, y, WBUTTON * 2, HTEXT * 4, "Data to log");
	grp_fpm_->box(FL_NO_BOX);
	grp_fpm_->labelfont(FONT);
	grp_fpm_->labelsize(FONT_SIZE);
	grp_fpm_->align(FL_ALIGN_BOTTOM| FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

	const int W2A = WBUTTON * 3 / 2;
	const int C2A = col1 + WBUTTON / 2;
	// Input - allows user to manually enter frequency
	// Reflects current frequency per logging mde
	ip_freq_ = new Fl_Input(C2A, y, W2A, HTEXT, "Freq");
	ip_freq_->textfont(FONT);
	ip_freq_->textsize(FONT_SIZE);
	ip_freq_->labelfont(FONT);
	ip_freq_->labelsize(FONT_SIZE);
	ip_freq_->align(FL_ALIGN_LEFT);
	ip_freq_->tooltip("Enter frequency of operation, if different");
	ip_freq_->callback(cb_ip_freq);

	y += HTEXT;
	// Choice - allows user to select mode of operation
	// Refelcts current mode per logging mode
	ch_mode_ = new Fl_Choice(C2A, y, W2A, HTEXT, "Mode");
	ch_mode_->textfont(FONT);
	ch_mode_->textsize(FONT_SIZE);
	ch_mode_->labelfont(FONT);
	ch_mode_->labelsize(FONT_SIZE);
	ch_mode_->align(FL_ALIGN_LEFT);
	ch_mode_->tooltip("Select mode of operatio, if different");
	ch_mode_->callback(cb_ch_mode);

	y += HTEXT;
	// Input - allows user to manually enter power
	// Refelcts current power per logging mode
	ip_power_ = new Fl_Input(C2A, y, W2A, HTEXT, "Power");
	ip_power_->textfont(FONT);
	ip_power_->textsize(FONT_SIZE);
	ip_power_->labelfont(FONT);
	ip_power_->labelsize(FONT_SIZE);
	ip_power_->align(FL_ALIGN_LEFT);
	ip_power_->tooltip("Enter transmit power, if different");
	ip_power_->callback(cb_ip_power);

	string frequency = "";
	string power = "";
	string mode = "";
	string submode;
	// Get frequency, power and mode per logging mode
	switch(logging_mode_) {
	case LM_ON_AIR_CAT:
		// Get data from rig
		frequency = rig_if_->get_frequency(true);
		power = rig_if_->get_tx_power();
		rig_if_->get_string_mode(mode, submode);
		break;
	case LM_ON_AIR_COPY:
		if (record_) {
			// Get data from current record (if there is one)
			frequency = record_->item("FREQ");
			power = record_->item("TX_PWR");
			mode = record_->item("MODE", true);
		}
		break;
		mode = "USB";
	}
	ip_freq_->value(frequency.c_str());
	spec_data_->initialise_field_choice(ch_mode_, "Combined", mode);
	ip_power_->value(power.c_str());
	grp_fpm_->end();
	y += HTEXT + GAP;

	// Button - save the record
	bn_save_ = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F7 - Save");
	bn_save_->labelsize(FONT_SIZE);
	bn_save_->labelfont(FONT);
	bn_save_->tooltip("Save the record");
	bn_save_->callback(cb_save);

	// Button - cancel the record
	bn_cancel_ = new Fl_Button(col2, y, WBUTTON, HBUTTON);
	bn_cancel_->labelsize(FONT_SIZE);
	bn_cancel_->labelfont(FONT);
	bn_cancel_->tooltip("Cancel the record");
	bn_cancel_->callback(cb_cancel);

	g->resizable(nullptr);
	g->end();

	gsp->resizable(nullptr);
	gsp->size(g->x() + g->w(), max(editor_->h(), g->h()) + ch_logmode_->h() + HTEXT + GAP);
	gsp->end();

	max_w = max(max_w, gsp->x() + gsp->w() - curr_x);
	max_h = max(max_h, gsp->y() + gsp->h() - curr_y);
	curr_x += max_w;
	curr_y += max_h;

}

// Antenna and Rig ("Use") widgets
// assumes dashboard is the current active group
void dashboard::create_use_widgets(int& curr_x, int& curr_y) {

	int max_w = 0;
	int max_h = 0;

	// Antenna group of widgets
	antenna_grp_ = new common_grp(curr_x, curr_y, 10, 10, "Antennas", ANTENNA);
	antenna_grp_->tooltip("Allows the antennas to be specified");
	antenna_grp_->end();
	antenna_grp_->show();
	max_w = max(max_w, antenna_grp_->x() + antenna_grp_->w() - curr_x);
	max_h = max(max_h, antenna_grp_->y() + antenna_grp_->h() - curr_y);

	// Rig group of widgets
	rig_grp_ = new common_grp(antenna_grp_->x() + antenna_grp_->w() + GAP, antenna_grp_->y(), 10, 10, "Rigs", RIG);
	rig_grp_->tooltip("Allows the rigs to be specified");
	rig_grp_->end();
	rig_grp_->show();
	max_w = max(max_w, rig_grp_->x() + rig_grp_->w() - curr_x);
	max_h = max(max_h, rig_grp_->y() + rig_grp_->h() - curr_y);

	// QTH choice
	Fl_Group* gqth = new Fl_Group(rig_grp_->x() + rig_grp_->w() + GAP, rig_grp_->y(), 10, 10, "QTH");
	gqth->labelfont(FONT);
	gqth->labelsize(FONT_SIZE);
	gqth->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	gqth->box(FL_BORDER_BOX);

	ch_qth_ = new Fl_Choice(gqth->x() + GAP, gqth->y() + HTEXT, WSMEDIT, HTEXT);
	ch_qth_->textfont(FONT);
	ch_qth_->textsize(FONT_SIZE);
	ch_qth_->callback(cb_choice_text, &selected_qth_);
	populate_qth_choice();

	gqth->resizable(nullptr);
	gqth->size(GAP + ch_qth_->w() + GAP, HTEXT + ch_qth_->h() + GAP);
	gqth->end();

	// Box to contain antenna-rig connectivity
	Fl_Help_View* w19 = new Fl_Help_View(rig_grp_->x() + rig_grp_->w() + GAP, gqth->y() + gqth->h() + GAP, WBUTTON * 2, HMLIN);
	w19->box(FL_FLAT_BOX);
	w19->labelfont(FONT);
	w19->labelsize(FONT_SIZE);
	w19->textfont(FONT);
	w19->textsize(FONT_SIZE);
	ant_rig_box_ = w19;

	// Use rig/antenna buttons
	// Only use the selected antenna and rig in the current QSPO
	Fl_Button* w20 = new Fl_Button(w19->x(), w19->y() + w19->h() + GAP, WBUTTON * 2, HBUTTON, "Use in current only");
	w20->labelfont(FONT);
	w20->labelsize(FONT_SIZE);
	w20->callback(cb_bn_use_items, (void*)(long)SELECTED_ONLY);
	w20->tooltip("Use rig and antenna in current QSO");

	// Use in the current QSO and any subsequent ones (until changed)
	Fl_Button* w21 = new Fl_Button(w20->x(), w20->y() + w20->h(), WBUTTON * 2, HBUTTON, "Use in current and next");
	w21->labelfont(FONT);
	w21->labelsize(FONT_SIZE);
	w21->callback(cb_bn_use_items, (void*)(long)SELECTED_ONLY);
	w21->tooltip("Use rig and antenna in current QSO and following");

	// Use in subsequent QSOs
	Fl_Button* w22 = new Fl_Button(w21->x(), w21->y() + w21->h(), WBUTTON * 2, HBUTTON, "Use in next only");
	w22->labelfont(FONT);
	w22->labelsize(FONT_SIZE);
	w22->callback(cb_bn_use_items, (void*)(long)SELECTED_ONLY);
	w22->tooltip("Use rig and antenna in following QSOs");

	max_w = max(max_w, w22->x() + w22->w() - curr_x);
	max_h = max(max_h, w22->y() + w22->h() - curr_y);
	curr_x += max_w;
	curr_y += max_h;

}

// CAT settings widgets
void dashboard::create_cat_widgets(int& curr_x, int& curr_y) {

	int max_w = 0;
	int max_h = 0;

	// CAT control group
	cat_grp_ = new Fl_Group(curr_x, curr_y, 10, 10, "CAT");
	cat_grp_->labelfont(FONT);
	cat_grp_->labelsize(FONT_SIZE);
	cat_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	cat_grp_->box(FL_BORDER_BOX);


	// CAT select group: radio button selecting CAT type
	// o NO
	// o Hamlib
	// o Flrig
	cat_sel_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);

	// Radio - Select No Rig
	bn_nocat_ = new Fl_Radio_Round_Button(cat_sel_grp_->x() + GAP, cat_sel_grp_->y() + HTEXT, WRADIO, HTEXT, "None");
	bn_nocat_->align(FL_ALIGN_RIGHT);
	bn_nocat_->callback(cb_rad_handler, (void*)RIG_NONE);
	bn_nocat_->when(FL_WHEN_RELEASE);
	bn_nocat_->labelsize(FONT_SIZE);
	bn_nocat_->tooltip("Select no rig interface handler");
	bn_nocat_->value(rig_grp_->info().rig_data.handler == RIG_NONE);
	// Radio - Select Hamlib
	bn_hamlib_ = new Fl_Radio_Round_Button(bn_nocat_->x(), bn_nocat_->y() + bn_nocat_->h(), WRADIO, HTEXT, "Hamlib");
	bn_hamlib_->align(FL_ALIGN_RIGHT);
	bn_hamlib_->callback(cb_rad_handler, (void*)RIG_HAMLIB);
	bn_hamlib_->when(FL_WHEN_RELEASE);
	bn_hamlib_->labelsize(FONT_SIZE);
	bn_hamlib_->tooltip("Select Hamlib as the rig interface handler");
	bn_hamlib_->value(rig_grp_->info().rig_data.handler == RIG_HAMLIB);
	// Radio - Select Flrig
	bn_flrig_ = new Fl_Radio_Round_Button(bn_hamlib_->x(), bn_hamlib_->y() + bn_hamlib_->h(), WRADIO, HTEXT, "FlRig");
	bn_flrig_->align(FL_ALIGN_RIGHT);
	bn_flrig_->callback(cb_rad_handler, (void*)RIG_FLRIG);
	bn_flrig_->when(FL_WHEN_RELEASE);
	bn_flrig_->labelsize(FONT_SIZE);
	bn_flrig_->tooltip("Select Flrig as the rig interface handler");
	bn_flrig_->value(rig_grp_->info().rig_data.handler == RIG_FLRIG);

	cat_sel_grp_->resizable(nullptr);
	cat_sel_grp_->size((GAP * 3 / 2) + WRADIO, bn_flrig_->y() + bn_flrig_->h() + GAP - cat_sel_grp_->y());

	cat_sel_grp_->end();

	// Hamlib control grp
	// RIG=====v
	// PORTv  ALL*
	// BAUDv  OVR*
	hamlib_grp_ = new Fl_Group(cat_sel_grp_->x() + cat_sel_grp_->w(), cat_sel_grp_->y(), 10, 10);
	hamlib_grp_->labelsize(FONT_SIZE);
	hamlib_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	hamlib_grp_->box(FL_NO_BOX);

	// Choice - Select the rig model (Manufacturer/Model)
	Fl_Choice* ch_model_ = new Fl_Choice(hamlib_grp_->x() + WLLABEL, hamlib_grp_->y() + HTEXT, WSMEDIT, HTEXT, "Rig model");
	ch_model_->align(FL_ALIGN_LEFT);
	ch_model_->labelsize(FONT_SIZE);
	ch_model_->textsize(FONT_SIZE);
	ch_model_->tooltip("Select the model - for Hamlib");
	ch_model_->callback(cb_ch_model, nullptr);
	rig_model_choice_ = ch_model_;

	// Populate the manufacturere and model choice widgets
	populate_model_choice();

	// Choice port name
	Fl_Choice* ch_mfr = new Fl_Choice(ch_model_->x(), ch_model_->y() + ch_model_->h(), WBUTTON, HTEXT, "Port");
	ch_mfr->align(FL_ALIGN_LEFT);
	ch_mfr->labelsize(FONT_SIZE);
	ch_mfr->textsize(FONT_SIZE);
	ch_mfr->callback(cb_ch_port, nullptr);
	ch_mfr->tooltip("Select the comms port to use");
	port_if_choice_ = ch_mfr;

	// Use all ports
	Fl_Check_Button* bn_useall = new Fl_Check_Button(ch_mfr->x() + ch_mfr->w(), ch_mfr->y(), HBUTTON, HBUTTON, "All");
	bn_useall->align(FL_ALIGN_RIGHT);
	bn_useall->labelfont(FONT);
	bn_useall->labelsize(FONT_SIZE);
	bn_useall->tooltip("Select all existing ports, not just those available");
	bn_useall->callback(cb_bn_all, &all_ports_);
	show_all_ports_ = bn_useall;
	populate_port_choice();

	// Baud rate input 
	Fl_Choice* ch_baudrate = new Fl_Choice(ch_model_->x(), ch_mfr->y() + ch_mfr->h(), WBUTTON, HTEXT, "Baud rate");
	ch_baudrate->align(FL_ALIGN_LEFT);
	ch_baudrate->labelsize(FONT_SIZE);
	ch_baudrate->textsize(FONT_SIZE);
	ch_baudrate->tooltip("Enter baud rate");
	ch_baudrate->callback(cb_ch_baud, nullptr);
	baud_rate_choice_ = ch_baudrate;

	// Override capabilities (as coded in hamlib)
	Fl_Check_Button* bn_override = new Fl_Check_Button(ch_baudrate->x() + ch_baudrate->w(), ch_baudrate->y(), HBUTTON, HBUTTON, "Override\ncapability");
	bn_override->align(FL_ALIGN_RIGHT);
	bn_override->labelsize(FONT_SIZE);
	bn_override->tooltip("Allow full baud rate selection");
	bn_override->callback(cb_ch_over, nullptr);
	override_check_ = bn_override;

	populate_baud_choice();

	hamlib_grp_->resizable(nullptr);
	hamlib_grp_->size(max(ch_model_->x() + ch_model_->w(),
		max(bn_useall->x() + bn_useall->w(), bn_override->x() + bn_override->w())) + GAP - hamlib_grp_->x(),
		bn_override->y() + bn_override->h() + GAP - hamlib_grp_->y());

	hamlib_grp_->end();

	// Flrig group
	// [IPA        ]
	// [PORT#      ]
	// [RES        ]
	flrig_grp_ = new Fl_Group(hamlib_grp_->x(), hamlib_grp_->y(), 10, 10);
	flrig_grp_->labelsize(FONT_SIZE);
	flrig_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	flrig_grp_->box(FL_NO_BOX);

	// IP v4 address
	Fl_Input* ip_ipaddress = new Fl_Input(flrig_grp_->x() + WLLABEL, flrig_grp_->y() + HTEXT, WSMEDIT, HBUTTON, "IP Address");
	ip_ipaddress->align(FL_ALIGN_LEFT);
	ip_ipaddress->labelsize(FONT_SIZE);
	ip_ipaddress->textsize(FONT_SIZE);
	ip_ipaddress->tooltip("The IP address of the Flrig server");
	ip_ipaddress->value(rig_grp_->info().rig_data.flrig_params.ip_address.c_str());
	ip_ipaddress->callback(cb_ip_ipa, nullptr);
	ip_ipaddress->when(FL_WHEN_ENTER_KEY_ALWAYS);

	// IPv4 port number
	Fl_Input* ip_portnum = new Fl_Int_Input(ip_ipaddress->x(), ip_ipaddress->y() + ip_ipaddress->h(), WBUTTON, HTEXT, "Port");
	ip_portnum->align(FL_ALIGN_LEFT);
	ip_portnum->labelsize(FONT_SIZE);
	ip_portnum->textsize(FONT_SIZE);
	ip_portnum->tooltip("The IP port number of the Flrig server");
	ip_portnum->value(to_string(rig_grp_->info().rig_data.flrig_params.port).c_str());
	ip_portnum->callback(cb_ip_portn);
	ip_portnum->when(FL_WHEN_ENTER_KEY_ALWAYS);

	// XML-RPC resource name
	intl_input* ip_resource = new intl_input(ip_portnum->x(), ip_portnum->y() + ip_portnum->h(), WBUTTON, HTEXT, "Resource");
	ip_resource->align(FL_ALIGN_LEFT);
	ip_resource->labelsize(FONT_SIZE);
	ip_resource->textsize(FONT_SIZE);
	ip_resource->tooltip("The resource ID of the Flrig server");
	ip_resource->value(rig_grp_->info().rig_data.flrig_params.resource.c_str());
	ip_resource->callback(cb_ip_resource, nullptr);
	ip_resource->when(FL_WHEN_ENTER_KEY_ALWAYS);

	flrig_grp_->resizable(nullptr);
	flrig_grp_->size(max(ip_ipaddress->x() + ip_ipaddress->w(), max(ip_portnum->x() + ip_portnum->w(), ip_resource->x() + ip_resource->w())) + GAP - flrig_grp_->x(), ip_resource->y() + ip_resource->h() + GAP - flrig_grp_->y());
	flrig_grp_->end();

	// No rig group
	// TEXT
	norig_grp_ = new Fl_Group(hamlib_grp_->x(), hamlib_grp_->y(), WBUTTON, HTEXT);
	norig_grp_->labelsize(FONT_SIZE);
	norig_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	norig_grp_->box(FL_NO_BOX);
	norig_grp_->end();

	// Connected status
	connect_bn_ = new Fl_Button(bn_nocat_->x(), cat_sel_grp_->y() + cat_sel_grp_->h(), WBUTTON * 2, HBUTTON, "Connect...");
	connect_bn_->labelfont(FONT);
	connect_bn_->labelsize(FONT_SIZE);
	connect_bn_->color(FL_YELLOW);
	connect_bn_->tooltip("Select to attempt to connect rig");
	connect_bn_->callback(cb_bn_connect, nullptr);

	// Poll period group;
// <>FAST
// <>SLOW
	Fl_Group* poll_grp = new Fl_Group(connect_bn_->x(), connect_bn_->y() + connect_bn_->h() + GAP, 10, 10, "Polling interval (s)");
	poll_grp->labelsize(FONT_SIZE);
	poll_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	poll_grp->box(FL_NO_BOX);

	// Spinner to select fast polling rate (i.e. when still connected)
	ctr_pollfast_ = new Fl_Spinner(poll_grp->x() + WLLABEL, poll_grp->y() + HTEXT, WSMEDIT, HTEXT, "Connected");
	ctr_pollfast_->align(FL_ALIGN_LEFT);
	ctr_pollfast_->labelsize(FONT_SIZE);
	ctr_pollfast_->textsize(FONT_SIZE);
	ctr_pollfast_->tooltip("Select the polling period for fast polling");
	ctr_pollfast_->type(FL_FLOAT_INPUT);
	ctr_pollfast_->minimum(FAST_RIG_MIN);
	ctr_pollfast_->maximum(FAST_RIG_MAX);
	ctr_pollfast_->step(0.01);
	ctr_pollfast_->value(rig_grp_->info().rig_data.fast_poll_interval);
	ctr_pollfast_->callback(cb_ctr_pollfast);
	ctr_pollfast_->when(FL_WHEN_CHANGED);

	// Spinner to select slow polling rate (i.e. after disconnection to avoid excessive errors)
	Fl_Spinner* ctr_pollslow_ = new Fl_Spinner(ctr_pollfast_->x(), ctr_pollfast_->y() + ctr_pollfast_->h(), WSMEDIT, HTEXT, "Disconnected");
	ctr_pollslow_->align(FL_ALIGN_LEFT);
	ctr_pollslow_->labelsize(FONT_SIZE);
	ctr_pollslow_->textsize(FONT_SIZE);
	ctr_pollslow_->tooltip("Select the polling period for slow polling");
	ctr_pollslow_->type(FL_FLOAT_INPUT);
	ctr_pollslow_->minimum(SLOW_RIG_MIN);
	ctr_pollslow_->maximum(SLOW_RIG_MAX);
	ctr_pollslow_->step(0.5);
	ctr_pollslow_->value(rig_grp_->info().rig_data.slow_poll_interval);
	ctr_pollslow_->callback(cb_ctr_pollslow);
	ctr_pollslow_->when(FL_WHEN_CHANGED);

	poll_grp->resizable(nullptr);
	poll_grp->size(max(ctr_pollfast_->w(), ctr_pollslow_->w()) + WLLABEL + GAP, ctr_pollslow_->y() + ctr_pollslow_->h() + GAP - poll_grp->y());
	poll_grp->end();

	// Display hamlib ot flrig settings as selected
	switch (rig_grp_->info().rig_data.handler) {
	case RIG_NONE:
		norig_grp_->show();
		hamlib_grp_->hide();
		flrig_grp_->hide();
		break;
	case RIG_HAMLIB:
		norig_grp_->hide();
		hamlib_grp_->show();
		flrig_grp_->hide();
		break;
	case RIG_FLRIG:
		norig_grp_->hide();
		hamlib_grp_->hide();
		flrig_grp_->show();
		break;
	}

	cat_grp_->resizable(nullptr);
	cat_grp_->size(max(poll_grp->w(),
		cat_sel_grp_->w() +
		max(hamlib_grp_->w(),
			max(flrig_grp_->w(), norig_grp_->w()))) + GAP,
		poll_grp->y() + poll_grp->h() + GAP - cat_grp_->y());
	max_w = max(max_w, cat_grp_->x() + cat_grp_->w() - curr_x);
	max_h = max(max_w, cat_grp_->y() + cat_grp_->h() - curr_y);
	cat_grp_->end();

	curr_x += max_w;
	curr_y += max_h;

}

// Alarm widgets
void dashboard::create_alarm_widgets(int& curr_x, int& curr_y) {

	int max_w = 0;
	int max_h = 0;

	alarms_grp_ = new Fl_Group(curr_x, curr_y, 10, 10, "Alarms");
	alarms_grp_->labelfont(FONT);
	alarms_grp_->labelsize(FONT_SIZE);
	alarms_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	alarms_grp_->box(FL_BORDER_BOX);

	// dial to select SWR warning and error levels
	dial_swr_ = new alarm_dial(alarms_grp_->x() + GAP, alarms_grp_->y() + GAP, 80, 80, "SWR");
	dial_swr_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER);
	dial_swr_->labelsize(FONT_SIZE);
	dial_swr_->labelfont(FONT);
	dial_swr_->tooltip("Modify the SWR warning/error levels");
	dial_swr_->minimum(1.0);
	dial_swr_->maximum(5.0);
	dial_swr_->step(0.1);
	dial_swr_->alarm_color(FL_RED);
	dial_swr_->color2(FL_BLACK);
	dial_swr_->alarms(rig_grp_->info().rig_data.alarms.swr_warning,
		rig_grp_->info().rig_data.alarms.swr_error);
	dial_swr_->value(rig_if_ ? rig_if_->swr_meter() : 1.0);
	dial_swr_->callback(cb_alarm_swr, nullptr);

	// dial to select power warning levels
	dial_pwr_ = new alarm_dial(dial_swr_->x() + dial_swr_->w() + GAP, alarms_grp_->y() + GAP, 80, 80, "Power");
	dial_pwr_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER);
	dial_pwr_->labelsize(FONT_SIZE);
	dial_pwr_->labelfont(FONT);
	dial_pwr_->tooltip("Modify the Power warning levels");
	dial_pwr_->minimum(0.0);
	dial_pwr_->maximum(100.0);
	dial_pwr_->step(1);
	dial_pwr_->alarms(rig_grp_->info().rig_data.alarms.power_warning, nan(""));
	dial_pwr_->value(rig_if_ ? rig_if_->pwr_meter() : 0.0);
	dial_pwr_->callback(cb_alarm_pwr, nullptr);

	// dial to display Vdd minimum and maximum error levels
	dial_vdd_ = new alarm_dial(dial_pwr_->x() + dial_pwr_->w() + GAP, alarms_grp_->y() + GAP, 80, 80, "Vdd");
	dial_vdd_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER);
	dial_vdd_->labelsize(FONT_SIZE);
	dial_vdd_->labelfont(FONT);
	dial_vdd_->tooltip("Modify the Vdd warning levels");
	dial_vdd_->minimum(10.0);
	dial_vdd_->maximum(20.0);
	dial_vdd_->step(0.1);
	dial_vdd_->alarms(rig_grp_->info().rig_data.alarms.voltage_minimum,
		rig_grp_->info().rig_data.alarms.voltage_maximum);
	dial_vdd_->value(rig_if_ ? rig_if_->vdd_meter() : 10.0);
	dial_vdd_->callback(cb_alarm_vdd, nullptr);

	alarms_grp_->resizable(nullptr);

	alarms_grp_->size(dial_swr_->w() + dial_pwr_->w() + dial_vdd_->w() + (GAP * 4), dial_swr_->h() + GAP + HTEXT);
	max_w = max(max_w, alarms_grp_->x() + alarms_grp_->w() - curr_x);
	max_h = max(max_h, alarms_grp_->y() + alarms_grp_->h() - curr_y);
	alarms_grp_->end();

	curr_x += max_w;
	curr_y += max_h;

}

// Create widgets to hold current tima and date (UTC)
void dashboard::create_clock_widgets(int& curr_x, int& curr_y) {
	int max_w = 0;
	int max_h = 0;

	clock_grp_ = new Fl_Group(curr_x, curr_y, 10, 10, "Clock - UTC");
	clock_grp_->labelfont(FONT);
	clock_grp_->labelsize(FONT_SIZE);
	clock_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	clock_grp_->box(FL_BORDER_BOX);

	bn_time_ = new Fl_Button(clock_grp_->x() + GAP, clock_grp_->y() + HTEXT, 250, 100);
	bn_time_->color(FL_BLACK);
	bn_time_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_time_->labelfont(FONT | FL_BOLD);
	bn_time_->labelsize(5 * FONT_SIZE);
	bn_time_->labelcolor(FL_YELLOW);
	bn_time_->box(FL_FLAT_BOX);

	bn_date_ = new Fl_Button(bn_time_->x(), bn_time_->y() + bn_time_->h(), bn_time_->w(), HTEXT * 3 / 2);
	bn_date_->color(FL_BLACK);
	bn_date_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_date_->labelfont(FONT);
	bn_date_->labelsize(FONT_SIZE * 3 / 2);
	bn_date_->labelcolor(FL_YELLOW);
	bn_date_->box(FL_FLAT_BOX);

	clock_grp_->resizable(nullptr);
	clock_grp_->size(2 * GAP + bn_time_->w(), GAP + HTEXT + bn_time_->h() + bn_date_->h());
	max_w = clock_grp_->w();
	max_h = clock_grp_->h();
	clock_grp_->end();

	curr_x += max_w;
	curr_y += max_h;

	// Start clock timer
	Fl::add_timeout(0, cb_timer_clock, this);

}

// Load values
void dashboard::load_values() {
	order_bands();

	// These are static, but will get to the same value each time
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences log_settings(user_settings, "Scratchpad");
	log_settings.get("Font Name", (int&)font_, FONT);
	log_settings.get("Font Size", (int&)fontsize_, FONT_SIZE);

	Fl_Preferences stations_settings(settings_, "Stations");

	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	// Set logging mode -default is On-air with or without rig connection
	int new_lm;
	logging_mode_t default_lm = rig_if_ ? LM_ON_AIR_CAT : LM_ON_AIR_COPY;
	dash_settings.get("Logging Mode", new_lm, default_lm);

	// If we are set to "On-air with CAT connection" check connecton
	if (!rig_if_ && new_lm == LM_ON_AIR_CAT) new_lm = LM_ON_AIR_COPY;
	logging_mode((logging_mode_t)new_lm);

	// Get the settings for the list of QTHs
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	// Number of items described in settings
	int num_items = qths_settings.groups();
	// Get the current item
	char* current_item;
	qths_settings.get("Current", current_item, "");
	selected_qth_ = current_item;
	free(current_item);
	all_qths_.clear();
	// For each item in the settings
	for (int i = 0; i < num_items; i++) {
		// Get that item's settings
		string name = qths_settings.group(i);
		all_qths_.insert(name);
	}
}

// Write values back to settings - write the three groups back separately
void dashboard::save_values() {

	// Save window position
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.set("Left", x_root());
	dash_settings.set("Top", y_root());
	dash_settings.set("Enabled", (signed int)shown());
	dash_settings.set("Logging Mode", logging_mode());

	Fl_Preferences stations_settings(settings_, "Stations");
	rig_grp_->save_values();
	antenna_grp_->save_values();

	// Set the selected QTH
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	qths_settings.set("Current", selected_qth_.c_str());
}

// Enable the widgets - activate the group associated with each rig handler when that
// handler is enabled
void dashboard::enable_widgets() {

	// Not all widgets may exist yet!
	if (!created_) return;

	enable_spad_widgets();
	enable_cat_widgets();
	enable_use_widgets();
	enable_alarm_widgets();
	enable_clock_widgets();

}

// Enable scratchpad widgets
void dashboard::enable_spad_widgets() {

	// Scratchpad widgets
	ch_logmode_->value(logging_mode_);

	if (enterring_record_) {
		// Allow save and cancel as we have a record
		bn_save_->activate();
		bn_cancel_->activate();
		bn_cancel_->label("F8 - Cancel");
		bn_start_->deactivate();
	}
	else {
		// Allow start and clear as we do not have a record
		bn_save_->deactivate();
		bn_cancel_->activate();
		bn_cancel_->label("F8 - Clear");
		bn_start_->activate();
	}

}

// Enable Antenna and rig setting widgets
void dashboard::enable_use_widgets() {

	// Antenna/rig compatibility
	Fl_Help_View* mlo = (Fl_Help_View*)ant_rig_box_;
	if (rig_if_ || record_) {
		// We have either a CAT connection or a previous record to cop
		double frequency;
		// Get logging mode frequency
		if (rig_if_) {
			frequency = rig_if_->tx_frequency();
		}
		else {
			record_->item("FREQ", frequency);
		}
		string rig_band = spec_data_->band_for_freq(frequency / 1000000.0);
		// Check if band supported by rig
		vector<string> bands = rig_grp_->info().rig_data.intended_bands;
		bool found = false;
		for (auto it = bands.begin(); it != bands.end() && !found; it++) {
			if (*it == rig_band) {
				found = true;
			}
		}
		if (!found) {
			// Rig does not have band
			mlo->color(FL_RED);
			mlo->value("Rig does not support the band read from it");

		}
		else {
			found = false;
			// Check if band supported by antenna
			vector<string> bands = antenna_grp_->info().intended_bands;
			for (auto it = bands.begin(); it != bands.end() && !found; it++) {
				if (*it == rig_band) {
					found = true;
				}
			}
			if (!found) {
				// Antenna not intended for band
				mlo->color(FL_YELLOW);
				mlo->value("Antenna not intended for this band. It may be what you want");
			}
			else {
				// Antenna AND rig are intended for band
				mlo->color(FL_GREEN);
				mlo->value("Antenna is intended for this band");
			}
		}
	}
	else {
		// No band selected - check that antenna and rig have at least one #
		// band they are both meant for.
		vector<string> r_bands = rig_grp_->info().rig_data.intended_bands;
		vector<string> a_bands = antenna_grp_->info().intended_bands;
		bool found = false;
		for (auto itr = r_bands.begin(); itr != r_bands.end() && !found; itr++) {
			for (auto ita = a_bands.begin(); ita != a_bands.end() && !found; ita++) {
				if (*itr == *ita) found = true;
			}
		}
		if (!found) {
			// Antenna not intended for same bands as rig
			mlo->color(FL_RED);
			mlo->value("Antenna is not intended for any band rig is capable of");
		}
		else {
			// Antenna intended for a band the rig is capabl of
			mlo->color(FL_BLUE);
			mlo->value("Antenna is intended for a band the rig is capable of");
		}
	}
	mlo->textcolor(fl_contrast(FL_BLACK, mlo->color()));

}

// Enable CAT Connection widgets
void dashboard::enable_cat_widgets() {

	// CAT control widgets
	// TODO update CAT widgets
	if (rig_grp_->info().rig_data.handler == RIG_HAMLIB) {
		hamlib_grp_->activate();
		hamlib_grp_->show();
		bn_hamlib_->value(true);
	}
	else {
		hamlib_grp_->deactivate();
		hamlib_grp_->hide();
		bn_hamlib_->value(false);
	}
	if (rig_grp_->info().rig_data.handler == RIG_FLRIG) {
		flrig_grp_->activate();
		flrig_grp_->show();
		bn_flrig_->value(true);
	}
	else {
		flrig_grp_->deactivate();
		flrig_grp_->hide();
		bn_flrig_->value(false);
	}
	if (rig_grp_->info().rig_data.handler == RIG_NONE) {
		norig_grp_->activate();
		norig_grp_->show();
		bn_nocat_->value(true);
	}
	else {
		norig_grp_->deactivate();
		norig_grp_->hide();
		bn_nocat_->value(false);
	}

	// Connect button
	if (rig_if_) {
		connect_bn_->color(FL_GREEN);
		connect_bn_->label("Connected");
	}
	else {
		switch (rig_grp_->info().rig_data.handler) {
		case RIG_HAMLIB:
		case RIG_FLRIG:
			if (wait_connect_) {
				connect_bn_->color(FL_YELLOW);
				connect_bn_->label("... Connect");
			}
			else {
				connect_bn_->color(FL_RED);
				connect_bn_->label("Disconnected");
			}
			break;
		case RIG_NONE:
			connect_bn_->color(FL_BACKGROUND_COLOR);
			connect_bn_->label("No CAT connection");
			break;
		}
	}

}

// Enable the alarm widgets
void dashboard::enable_alarm_widgets() {

	if (rig_if_) {
		alarms_grp_->activate();

		// SWR widget - set colour and raise alarm
		if (previous_swr_alarm_ != current_swr_alarm_) {
			char message[200];
			switch (current_swr_alarm_) {
			case SWR_ERROR:
				dial_swr_->color2(FL_RED);
				snprintf(message, 200, "DASH: SWR %g > %g", dial_swr_->Fl_Line_Dial::value(), dial_swr_->alarm2());
				status_->misc_status(ST_ERROR, message);
				break;
			case SWR_WARNING:
				dial_swr_->color2(fl_color_average(FL_RED, FL_YELLOW, 0.33f));
				snprintf(message, 200, "DASH: SWR %g > %g", dial_swr_->Fl_Line_Dial::value(), dial_swr_->alarm1());
				status_->misc_status(ST_WARNING, message);
				break;
			case SWR_OK:
				dial_swr_->color2(FL_BLACK);
				snprintf(message, 200, "DASH: SWR %g OK", dial_swr_->Fl_Line_Dial::value());
				status_->misc_status(ST_OK, message);
				break;
			}
		}
		if (rig_if_->get_tx()) {
			dial_swr_->activate();
		}
		else {
			dial_swr_->deactivate();
		}

		// Power widget - set colour and raise alarm
		if (previous_pwr_alarm_ != current_pwr_alarm_) {
			char message[200];
			switch (current_pwr_alarm_) {
			case POWER_OK:
				dial_pwr_->color2(FL_BLACK);
				snprintf(message, 200, "DASH: Power %g OK", dial_pwr_->Fl_Line_Dial::value());
				status_->misc_status(ST_OK, message);
				break;
			case POWER_WARNING:
				dial_pwr_->color2(fl_color_average(FL_RED, FL_YELLOW, 0.33f));
				snprintf(message, 200, "DASH: Power %g > %g", dial_pwr_->Fl_Line_Dial::value(), dial_pwr_->alarm1());
				status_->misc_status(ST_WARNING, message);
				break;
			}
		}
		if (rig_if_->get_tx()) {
			dial_pwr_->activate();
		}
		else {
			dial_pwr_->deactivate();
		}

		// Vdd (PA drain voltage) widget - set colour and raise alarm
		if (previous_vdd_alarm_ != current_vdd_alarm_) {
			char message[200];
			switch (current_vdd_alarm_) {
			case VDD_UNDER:
				dial_vdd_->color2(FL_RED);
				snprintf(message, 200, "DASH: Vdd %g < %g", dial_vdd_->Fl_Line_Dial::value(), dial_vdd_->alarm1());
				status_->misc_status(ST_ERROR, message);
				break;
			case VDD_OK:
				dial_vdd_->color2(FL_BLACK);
				snprintf(message, 200, "DASH: Vdd %g OK", dial_vdd_->Fl_Line_Dial::value());
				status_->misc_status(ST_OK, message);
				break;
			case VDD_OVER:
				dial_vdd_->color2(FL_RED);
				snprintf(message, 200, "DASH: Vdd %g > %g", dial_vdd_->Fl_Line_Dial::value(), dial_vdd_->alarm2());
				status_->misc_status(ST_ERROR, message);
				break;

			}
		}
	}
	else {
		alarms_grp_->deactivate();
	}
}

// Place holder for noe
void dashboard::enable_clock_widgets() {}

// Read the list of bands from the ADIF specification and put them in frequency order
void dashboard::order_bands() {
	// List of bands - in string order of name of the bands (e.g. 10M or 3CM)
	spec_dataset* band_dataset = spec_data_->dataset("Band");
	ordered_bands_.clear();
	for (auto its = band_dataset->data.begin(); its != band_dataset->data.end(); its++) {
		bool found = false;
		string band = (*its).first;
		// Iterator to the generated list - restart each loop at the beginning of this list
		auto itd = ordered_bands_.begin();
		// Get the frequency of the band we are adding
		string freq = (*its).second->at("Lower Freq (MHz)");
		double s_frequency = stod(freq, nullptr);
		// Until we have found where to put it or reached the end of the new list
		while (!found && itd != ordered_bands_.end()) {
			map<string, string>* band_data = band_dataset->data.at(*itd);
			// Get the frqruency of the current band in the output list
			freq = band_data->at("Lower Freq (MHz)");
			double d_frequency = stod(freq, nullptr);
			// If this is where we add it (first entry in output listthat is higher than it
			if (s_frequency < d_frequency) {
				found = true;
				ordered_bands_.insert(itd, band);
			}
			itd++;
		}
		if (!found) {
			ordered_bands_.push_back(band);
		}
	}
}

// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
void dashboard::populate_model_choice() {
	Fl_Choice* ch = (Fl_Choice*)rig_model_choice_;
	// Get hamlib Model number and populate control with all model names
	ch->clear();
	char* target_pathname = nullptr;
	// For each possible rig ids in hamlib
	// TODO: Check maximum rig number
	for (rig_model_t i = 1; i < 4000; i += 1) {
		// Get the capabilities of this rig ID (at ID #xx01)
		const rig_caps* capabilities = rig_get_caps(i);
		if (capabilities != nullptr) {
			// There is a rig - add the model name to that choice
			// What is the status of the handler for this particular rig?
			char status[16];
			switch (capabilities->status) {
			case RIG_STATUS_ALPHA:
				strcpy(status, " (Alpha)");
				break;
			case RIG_STATUS_UNTESTED:
				strcpy(status, " (Untested)");
				break;
			case RIG_STATUS_BETA:
				strcpy(status, " (Beta)");
				break;
			case RIG_STATUS_STABLE:
				strcpy(status, "");
				break;
			case RIG_STATUS_BUGGY:
				strcpy(status, " (Buggy)");
				break;
			}
			// Generate the item pathname - e.g. "Icom/IC-736 (untested)"
			char* temp = new char[strlen(status) + 10 + strlen(capabilities->model_name) + strlen(capabilities->mfg_name)];
			// The '/' ensures all rigs from same manufacturer are in a sub-menu to Icom
			sprintf(temp, "%s/%s%s", capabilities->mfg_name, capabilities->model_name, status);
			ch->add(temp);
			hamlib_data* hlinfo = &rig_grp_->info().rig_data.hamlib_params;
			if (strcmp(hlinfo->model.c_str(), (capabilities->model_name)) == 0 &&
				strcmp(hlinfo->mfr.c_str(), (capabilities->mfg_name)) == 0) {
				// We are adding the current selected rig, remember it's menu item value and hamlib reference number
				target_pathname = new char[strlen(temp) + 1];
				strcpy(target_pathname, temp);
				hlinfo->model_id = i;
			}
		}
	}
	bool found = false;
	// Go through all the menu items until we find our remembered pathname, and set the choice value to that item number
	// We have to do it like this as the choice value when we added it may have changed.
	for (int i = 0; i < ch->size() && !found && target_pathname; i++) {
		char item_pathname[128];
		ch->item_pathname(item_pathname, 127, &ch->menu()[i]);
		if (strcmp(item_pathname, target_pathname) == 0) {
			found = true;
			ch->value(i);
		}
	}
	delete[] target_pathname;
}

// Rig handler radio button clicked
// v contains the radio button value 
void dashboard::cb_rad_handler(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	// Get the selected radio button
	rig_handler_t handler = (rig_handler_t)(long)v;

	that->rig_grp_->info().rig_data.handler = handler;

	// Disconnect radio as we are changing handler
	if (rig_if_) {
		delete rig_if_;
		rig_if_ = nullptr;
	}
	// We have a handler - set flag that we are waiting to connect
	if (handler != RIG_NONE) that->wait_connect_ = true;
	// Enable the appropriate widget group for the selected handler
	that->enable_widgets();
	// Save selected interface
	that->save_values();
}

// Model input choice selected
// v is not used
void dashboard::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	dashboard* that = ancestor_view<dashboard>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	// Get the full item name - e.g. Icom/IC-736 (untested)
	char path[128];
	ch->item_pathname(path, sizeof(path) - 1);
	// Get the manufacturer - i.e. upto the / character
	char* pos_stroke = strchr(path, '/');
	info->mfr = string(path, pos_stroke - path);
	// Get the mode - upto the " (" - returns nullptr if it's not there
	char* pos_bracket = strstr(pos_stroke + 1, " (");
	if (pos_bracket == nullptr) {
		info->model = string(pos_stroke + 1);
	}
	else {
		info->model = string(pos_stroke + 1, pos_bracket - pos_stroke - 1);
	}
	// For each possible rig ids in hamlib 
	bool found = false;
	for (rig_model_t i = 1; i < 4000 && !found; i += 1) {
		// Get the capabilities of this rig ID (at ID #xx01)
		const rig_caps* capabilities = rig_get_caps(i);
		if (capabilities != nullptr) {
			if (strcmp(info->model.c_str(), (capabilities->model_name)) == 0 &&
				strcmp(info->mfr.c_str(), (capabilities->mfg_name)) == 0) {
				info->model_id = i;
				found = true;
			}
		}
	}
	that->populate_baud_choice();
}

// Callback selecting port
// v is unused
void dashboard::cb_ch_port(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->port_name);
}

// Callback selecting baud-rate
// v is unused
void dashboard::cb_ch_baud(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->baud_rate);
}

// Override rig capabilities selected - repopulate the baud choice
// v is uused
void dashboard::cb_ch_over(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	cb_value<Fl_Check_Button, bool>(w, (void*)&info->override_caps);
	that->populate_baud_choice();
}

// Select "display all ports" in port choice
// v is a pointer to the all ports flag
void dashboard::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	dashboard* that = ancestor_view<dashboard>(w);
	that->populate_port_choice();
}

// Select Flrig server IP Address
// v is unused
void dashboard::cb_ip_ipa(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	flrig_data* info = &that->rig_grp_->info().rig_data.flrig_params;
	cb_value<Fl_Input, string>(w, (void*)&info->ip_address);
}

// Select Flrig server port number
// v is unused
void dashboard::cb_ip_portn(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	flrig_data* info = &that->rig_grp_->info().rig_data.flrig_params;
	cb_value_int<Fl_Int_Input>(w, (void*)&info->port);
}

// Selecte Flrig resource ID
// v is unused
void dashboard::cb_ip_resource(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	flrig_data* info = &that->rig_grp_->info().rig_data.flrig_params;
	cb_value<intl_input, string>(w, (void*)&info->resource);
}

// Changed the SWR wrning level
// v is unused
void dashboard::cb_alarm_swr(Fl_Widget* w, void* v) {
	alarm_dial* dial = ancestor_view<alarm_dial>(w);
	dashboard* that = ancestor_view<dashboard>(dial);
	double val = dial->Fl_Line_Dial::value();
	double error = dial->alarm2();
	double warn = dial->alarm1();
	// If in receive mode, use last value read in TX mode
	if (!rig_if_->get_tx()) {
		val = that->last_tx_swr_;
		dial->Fl_Line_Dial::value(val);
	}
	else {
		that->last_tx_swr_ = val;
	}
	// Check against error or warning levels
	that->previous_swr_alarm_ = that->current_swr_alarm_;
	if (val > error) {
		that->current_swr_alarm_ = SWR_ERROR;
	}
	else if (val > warn) {
		that->current_swr_alarm_ = SWR_WARNING;
	}
	else {
		that->current_swr_alarm_ = SWR_OK;
	}
	that->enable_widgets();
}

// Changed the power level
// v is unused
void dashboard::cb_alarm_pwr(Fl_Widget* w, void* v) {
	alarm_dial* dial = ancestor_view<alarm_dial>(w);
	dashboard* that = ancestor_view<dashboard>(dial);
	double val = dial->Fl_Line_Dial::value();
	double warn = dial->alarm1();
	// If in receive mode, use last value read in TX mode
	if (!rig_if_->get_tx()) {
		val = that->last_tx_pwr_;
		dial->Fl_Line_Dial::value(val);
	}
	else {
		that->last_tx_pwr_ = val;
	}
	// Check against warning level
	that->previous_pwr_alarm_ = that->current_pwr_alarm_;
	if (val > warn) {
		that->current_pwr_alarm_ = POWER_WARNING;
	}
	else {
		that->current_pwr_alarm_ = POWER_OK;
	}
	that->enable_widgets();
}

// Changed the drain voltage
void dashboard::cb_alarm_vdd(Fl_Widget* w, void* v) {
	alarm_dial* dial = ancestor_view<alarm_dial>(w);
	dashboard* that = ancestor_view<dashboard>(dial);
	double val = dial->Fl_Line_Dial::value();
	double max = dial->alarm2();
	double min = dial->alarm1();
	// Check against error levels
	that->previous_vdd_alarm_ = that->current_vdd_alarm_;
	if (val > max) {
		that->current_vdd_alarm_ = VDD_OVER;
	}
	else if (val < min) {
		that->current_vdd_alarm_ = VDD_UNDER;
	}
	else {
		that->current_vdd_alarm_ = VDD_OK;
	}
	that->enable_widgets();
}

// Changed the fast polling interval
// v is not used
void dashboard::cb_ctr_pollfast(Fl_Widget* w, void* v) {
	// Get the warning level
	dashboard* that = ancestor_view<dashboard>(w);
	cb_value<Fl_Spinner, double>(w, &that->rig_grp_->info().rig_data.fast_poll_interval);
}

// Changed the fast polling interval
// v is not used
void dashboard::cb_ctr_pollslow(Fl_Widget* w, void* v) {
	// Get the warning level
	dashboard* that = ancestor_view<dashboard>(w);
	cb_value<Fl_Spinner, double>(w, &that->rig_grp_->info().rig_data.slow_poll_interval);
}

// Pressed the connect button
// v is not used
void dashboard::cb_bn_connect(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	that->save_values();
	switch (that->rig_grp_->info().rig_data.handler) {
	case RIG_HAMLIB:
	case RIG_FLRIG:
		if (rig_if_) {
		// We are connected - set disconnected
			delete rig_if_;
			rig_if_ = nullptr;
			that->wait_connect_ = true;
		}
		else {
			// Wer are discooencted, so connect
			add_rig_if();
			that->wait_connect_ = false;
		}
		break;
	case RIG_NONE:
		if (rig_if_) {
			delete rig_if_;;
			rig_if_ = nullptr;
			that->wait_connect_ = false;
		}
		break;
	}
	that->enable_widgets();
}

// Callback for use items button
// v is use_item_t
void dashboard::cb_bn_use_items(Fl_Widget* w, void* v) {
	use_item_t use = (use_item_t)(long)v;
	dashboard* that = ancestor_view<dashboard>(w);
	record* sel_record = book_->get_record();
	switch (use) {
	case SELECTED_ONLY:
		// Update selected record only with antenna and rig
		sel_record->item("MY_RIG", that->rig_grp_->name());
		sel_record->item("MY_ANTENNA", that->antenna_grp_->name());
		sel_record->item("APP_ZZA_QTH", that->selected_qth_);
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->selection());
		break;
	case SELECTED_NEW:
		// Update selected and subsequent records with antenna and rig
		sel_record->item("MY_RIG", that->rig_grp_->name());
		sel_record->item("MY_ANTENNA", that->antenna_grp_->name());
		sel_record->item("APP_ZZA_QTH", that->selected_qth_);
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->selection());
		that->rig_grp_->next() = that->rig_grp_->name();
		that->antenna_grp_->next() = that->antenna_grp_->name();
		that->next_qth_ = that->selected_qth_;
		break;
	case NEW_ONLY:
		// Set subsequent records with antenna and rig
		sel_record->user_details();
		that->rig_grp_->next() = that->rig_grp_->name();
		that->antenna_grp_->next() = that->antenna_grp_->name();
		that->next_qth_ = that->selected_qth_;
		break;
	}
}

// One of the write field buttons has been activated
// v provides the specific action 
void dashboard::cb_action(Fl_Widget* w, void* v) {
	actions action = (actions)(long)v;
	dashboard* that = ancestor_view<dashboard>(w);

	string field;
	// Create a record if we haven't started editing one
	if (!that->enterring_record_) {
		cb_start(w, nullptr);
	}
	// Get the field to write from the button action. Default update is a minor change
	hint_t hint = HT_MINOR_CHANGE;
	switch (action) {
	case WRITE_CALL:
		field = "CALL";
		break;
	case WRITE_NAME:
		field = "NAME";
		break;
	case WRITE_QTH:
		field = "QTH";
		break;
	case WRITE_GRID:
		field = "GRIDSQUARE";
		hint = HT_CHANGED;
		break;
	case WRITE_RST_SENT:
		field = "RST_SENT";
		break;
	case WRITE_RST_RCVD:
		field = "RST_RCVD";
		break;
	case WRITE_FIELD:
		field = that->field_;
		if (field == "DXCC" || field == "GRIDSQUARE") {
			hint = HT_CHANGED;
		}
		break;
	}
	// Get the highlighted text from the editor buffer and write it to the selected field, unhighlight the text
	char* raw_text = nullptr;
	string text;
	if (that->buffer_->selected()) {
		raw_text = that->buffer_->selection_text();
		text = raw_text;
	}
	else {
		int end_pos = that->editor_->insert_position();
		int start_pos = that->buffer_->word_start(end_pos - 1);
		if (start_pos < end_pos) {
			that->buffer_->select(start_pos, end_pos);
			that->redraw();
			raw_text = that->buffer_->selection_text();
			text = raw_text;
		}
	}
	if (raw_text) {
		free(raw_text);
	}
	// Remove leading and trailing white space
	while (isspace(text[0])) text = text.substr(1);
	while (isspace(text[text.length() - 1])) text = text.substr(0, text.length() - 1);
	that->record_->item(field, text);
	that->buffer_->unselect();
	// Set DX location on DxAtlas
	if (action == WRITE_GRID) {
		dxatlas_->set_dx_loc(to_upper(text), that->record_->item("CALL"));
	}
	// Update views
	tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	// Give the editor the focus
	that->editor_->take_focus();
	// We may have changed the state
	that->enable_widgets();
}

// Have we worked before? Loads previous contacts in extract view
// v is unused
void dashboard::cb_wkb4(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	string text = that->buffer_->selection_text();
	extract_records_->extract_call(text);
}

// Parse selected text as callsign
// // v is unused
void dashboard::cb_parse(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	string text = that->buffer_->selection_text();
	// Create a temporary record to parse the callsign
	record* tip_record = new record(that->logging_mode_, nullptr);
	string message = "";
	// Set the callsign in the temporary record
	tip_record->item("CALL", to_upper(text));
	// Parse the temporary record
	message = pfx_data_->get_tip(tip_record);
	// Create a tooltip window at the parse button (in w) X and Y
	Fl_Window* tw = ::tip_window(message, that->x_root() + w->x() + w->w() / 2, that->y_root() + w->y() + w->h() / 2);
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
	delete tip_record;
}

// Save the record and reset the state to no record
// v is not used
void dashboard::cb_save(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	// Save frequency mode and TX_PWR if that didn't change
	string freq;
	cb_value<Fl_Input, string>(that->ip_freq_, &freq);
	that->record_->item("FREQ", freq);
	string mode;
	cb_choice_text(that->ch_mode_, &mode);
	that->record_->item("MODE", mode);
	string power;
	cb_value<Fl_Input, string>(that->ip_power_, &power);
	that->record_->item("TX_PWR", power);
	// Save the record - should update views
	book_->save_record();
	that->enterring_record_ = false;
	that->buffer_->text("");
	that->enable_widgets();
	dxatlas_->clear_dx_loc();
}

// Cancel the edit and reset the state to no record
// v is not used
void dashboard::cb_cancel(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	if (that->enterring_record_) {
		book_->delete_record(false);
		that->record_ = book_->get_record();
		dxatlas_->clear_dx_loc();
	}
	that->buffer_->text("");
	that->enable_widgets();
}

// Close button clicked - check editing or not
// v is not used
void dashboard::cb_close(Fl_Widget* w, void* v) {
	// It is the window that raised this callback
	dashboard* that = (dashboard*)w;
	// If we are editing does the user want to save or cancel?
	if (that->record_ != nullptr) {
		if (fl_choice("Entering a record - save or cancel?", "Cancel", "Save", nullptr) == 1) {
			cb_save(w, v);
		}
		else {
			cb_cancel(w, v);
		}
	}
	// Mark dashboard disabled, hide it and update menu item
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Enabled", (int)false);
	menu_->update_items();
}

// Start button - create a new record
// v is not used
void dashboard::cb_start(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	that->record_ = book_->new_record(that->logging_mode());
	that->enterring_record_ = true;
	if (that->logging_mode_) {
		that->record_->item("FREQ", string(that->ip_freq_->value()));
		string mode;
		cb_choice_text(that->ch_mode_, &mode);
		if (spec_data_->is_submode(mode)) {
			that->record_->item("SUBMODE", mode);
			that->record_->item("MODE", spec_data_->mode_for_submode(mode));
		}
		else {
			that->record_->item("MODE", mode);
			that->record_->item("SUBMODE", string(""));
		}
		that->record_->item("TX_PWR", string(that->ip_power_->value()));
		tabbed_forms_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
	// Get the highlighted text from the editor buffer and write it to the field "CALL", unhighlight the text
	if (that->buffer_->selected()) {
		string text = that->buffer_->selection_text();
		// Remove leading and trailing white space
		while (isspace(text[0])) text = text.substr(1);
		while (isspace(text[text.length() - 1])) text = text.substr(0, text.length() - 1);
		that->record_->item("CALL", text);
		that->buffer_->unselect();
	}
	that->enable_widgets();
}

// Frequency input
// v is not used
void dashboard::cb_ip_freq(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	if (that->record_) {
		// Input is in MHz - keep as that for record, convert to kHz for band check
		string value;
		cb_value<Fl_Input, string>(w, &value);
		double freq = stod(value) * 1000;
		that->record_->item("FREQ", value);
		if (band_view_ && !band_view_->in_band(freq)) {
			((Fl_Input*)w)->textcolor(FL_RED);
		}
		else {
			((Fl_Input*)w)->textcolor(FL_BLACK);
		}
		// Update views
		band_view_->update(freq);
		prev_freq_ = freq;
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
}

// Mode choice
// v is not used
void dashboard::cb_ch_mode(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	if (that->record_) {
		// Value in choice is a sub-mode
		string value;
		cb_choice_text(w, &value);
		if (spec_data_->is_submode(value)) {
			that->record_->item("SUBMODE", value);
			that->record_->item("MODE", spec_data_->mode_for_submode(value));
		}
		else {
			that->record_->item("MODE", value);
			that->record_->item("SUBMODE", string(""));
		}
		// Update views
		tabbed_forms_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
}

// Power input
// v is not used
void dashboard::cb_ip_power(Fl_Widget* w, void* v) {
	dashboard* that = ancestor_view<dashboard>(w);
	if (that->record_) {
		string value;
		cb_value<Fl_Input, string>(w, &value);
		that->record_->item("TX_PWR", value);
		// Update views
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
}

// Populate the choice with the available ports
void dashboard::populate_port_choice() {
	Fl_Choice* ch = (Fl_Choice*)port_if_choice_;
	ch->clear();
	ch->add("NONE");
	ch->value(0);
	int num_ports = 1;
	string* existing_ports = new string[1];
	serial serial;
	// Get the list of all ports or available (not in use) ports
	while (!serial.available_ports(num_ports, existing_ports, all_ports_, num_ports)) {
		delete[] existing_ports;
		existing_ports = new string[num_ports];
	}
	// now for the returned ports
	for (int i = 0; i < num_ports; i++) {
		// Add the name onto the choice drop-down list
		char message[100];
		const char* port = existing_ports[i].c_str();
		snprintf(message, sizeof(message), "RIG: Found port %s", port);
		status_->misc_status(ST_LOG, message);
		ch->add(port);
		// Set the value to the list of ports
		if (strcmp(port, rig_grp_->info().rig_data.hamlib_params.port_name.c_str()) == 0) {
			ch->value(i);
		}
	}
}

// Clock timer callback - received every UTC_TIMER seconds
void dashboard::cb_timer_clock(void* v) {
	// Update the label in the clock button which is passed as the parameter
	dashboard* dash = (dashboard*)v;
	time_t now = time(nullptr);
	tm* value = gmtime(&now);
	char result[100];
	// convert to C string, then C++ string
	strftime(result, 99, "%H:%M:%S", value);
	dash->bn_time_->copy_label(result);
	// Convert date
	strftime(result, 99, "%A %d %B %Y", value);
	dash->bn_date_->copy_label(result);

	dash->clock_grp_->redraw();

	Fl::repeat_timeout(UTC_TIMER, cb_timer_clock, v);
}


// Populate the baud rate choice menu
void dashboard::populate_baud_choice() {
	Fl_Choice* ch = (Fl_Choice*)baud_rate_choice_;
	ch->clear();
	// Override rig's capabilities?
	bool override_caps = rig_grp_->info().rig_data.hamlib_params.override_caps;
	Fl_Button* bn = (Fl_Button*)override_check_;
	bn->value(override_caps);

	// Get the baud-rates supported by the rig
	const rig_caps* caps = rig_get_caps(rig_grp_->info().rig_data.hamlib_params.model_id);
	int min_baud_rate = 300;
	int max_baud_rate = 460800;
	if (caps) {
		min_baud_rate = caps->serial_rate_min;
		max_baud_rate = caps->serial_rate_max;
	}
	// Default baud-rates
	const int baud_rates[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800 };
	int num_rates = sizeof(baud_rates) / sizeof(int);
	int index = 0;
	ch->value(0);
	// If no values add an empty value
	if (num_rates == 0)	ch->add("");
	// For all possible rates
	for (int i = 0; i < num_rates; i++) {
		int rate = baud_rates[i];
		if (override_caps || (rate >= min_baud_rate && rate <= max_baud_rate)) {
			// capabilities overridden or within the range supported by capabilities
			ch->add(to_string(rate).c_str());
			if (to_string(rate) == rig_grp_->info().rig_data.hamlib_params.baud_rate) {
				ch->value(index);
				index++;
			}
		}
	}
}

// Populate QTH choice
void dashboard::populate_qth_choice() {
	ch_qth_->clear();
	int index = 0;
	for (auto it = all_qths_.begin(); it != all_qths_.end(); it++, index++) {
		ch_qth_->add((*it).c_str());
		if (*it == selected_qth_) {
			ch_qth_->value(index);
		}
	}
}

// Return the logging mode
logging_mode_t dashboard::logging_mode() {
	return logging_mode_;
}

// Set the logging mode
void dashboard::logging_mode(logging_mode_t mode) {
	logging_mode_ = mode;
	enable_widgets();
}

// Called when rig is read to update values here
void dashboard::rig_update(string frequency, string mode, string power) {
	ip_freq_->value(frequency.c_str());
	double freq = stod(frequency) * 1000;
	// If the frequency is outside a ham-band, display in red else in black
	if (band_view_ && !band_view_->in_band(freq)) {
		ip_freq_->textcolor(FL_RED);
	}
	else {
		ip_freq_->textcolor(FL_BLACK);
	}
	// Power in watts
	ip_power_->value(power.c_str());
	// Mode - index into choice
	int index = ch_mode_->find_index(mode.c_str());
	ch_mode_->value(index);
	redraw();
}

// Get QSO information from previous record not rig
void dashboard::update() {
	// Get freq etc from QSO or rig
	// Get present values data from rig
	if (rig_if_) {
		dial_swr_->value(rig_if_->swr_meter());
		dial_pwr_->value(rig_if_->pwr_meter());
		dial_vdd_->value(rig_if_->vdd_meter());
		ip_freq_->value(rig_if_->get_frequency(true).c_str());
		ip_power_->value(rig_if_->get_tx_power().c_str());
		string mode;
		string submode;
		rig_if_->get_string_mode(mode, submode);
		ch_mode_->value(ch_mode_->find_index(submode.c_str()));
		if (band_view_) {
			double freq = stod(rig_if_->get_frequency(true)) * 1000.0;
			if (band_view_->in_band(freq)) {
				ip_freq_->textcolor(FL_BLACK);
			}
			else {
				ip_freq_->textcolor(FL_RED);
			}
			band_view_->update(freq);
			prev_freq_ = freq;
		}
		string rig_name = rig_if_->rig_name();
		if (rig_name != rig_grp_->name()) {
			rig_grp_->name() = rig_name;
			rig_grp_->populate_choice();
		}
	}
	else if (book_->size()) {
		record* prev_record = book_->get_record();
		// Assume as it's a logged record, the frequency is valid
		ip_freq_->textcolor(FL_BLACK);
		ip_freq_->value(prev_record->item("FREQ").c_str());
		ip_power_->value(prev_record->item("TX_PWR").c_str());
		ch_mode_->value(ch_mode_->find_index(prev_record->item("MODE", true).c_str()));
		if (band_view_ && prev_record->item_exists("FREQ")) {
			double freq = stod(prev_record->item("FREQ")) * 1000.0;
			band_view_->update(freq);
			prev_freq_ = freq;
		}

		antenna_grp_->name() = prev_record->item("MY_ANTENNA");
		antenna_grp_->populate_choice();
		rig_grp_->name() = prev_record->item("MY_RIG");
		rig_grp_->populate_choice();
	}
	else {
		// No default
		ip_freq_->textcolor(FL_RED);
		ip_freq_->value("0");
		ip_power_->value("0");
		ch_mode_->value(0);
		antenna_grp_->name() = "";
		rig_grp_->name() = "";
	}
	enable_widgets();
}

// Set font etc.
void dashboard::set_font(Fl_Font font, Fl_Fontsize size) {
	font_ = font;
	fontsize_ = size;
	// Change the font in the editor
	editor_->textsize(fontsize_);
	editor_->textfont(font_);
	// And ask it to recalculate the wrap positions
	editor_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	redraw();
}

// Constructor for dashboard editor
spad_editor::spad_editor(int x, int y, int w, int h) :
	intl_editor(x, y, w, h)
{
}

// Destructor for dashboard editor
spad_editor::~spad_editor() {
}

// Handler of any event coming from the dashboard editor
int spad_editor::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		// mouse going in and out of focus on this view
		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
		return true;
	case FL_KEYBOARD:
		// Keyboard event - used for keyboard navigation
		switch (Fl::event_key()) {
		case FL_F + 1:
			// F1 - copy selected text to CALL field
			dashboard::cb_action(this, (void*)dashboard::WRITE_CALL);
			return true;
		case FL_F + 2:
			// F2 - copy selected text to NAME field
			dashboard::cb_action(this, (void*)dashboard::WRITE_NAME);
			return true;
		case FL_F + 3:
			// F3 - copy selected text to QTH field
			dashboard::cb_action(this, (void*)dashboard::WRITE_QTH);
			return true;
		case FL_F + 4:
			// F4 - copy selected text to RST_RCVD field
			dashboard::cb_action(this, (void*)dashboard::WRITE_RST_RCVD);
			return true;
		case FL_F + 5:
			// F5 - copy selected text to RST_SENT field
			dashboard::cb_action(this, (void*)dashboard::WRITE_RST_SENT);
			return true;
		case FL_F + 6:
			// F6 - copy selected text to GRIDSQUARE field
			dashboard::cb_action(this, (void*)dashboard::WRITE_GRID);
			return true;
		case FL_F + 7:
			// F7 - save record
			dashboard::cb_save(this, nullptr);
			return true;
		case FL_F + 8:
			// F8 - discard record
			dashboard::cb_cancel(this, nullptr);
			return true;
		case FL_F + 9:
			// F9 - Check worked before
			dashboard::cb_wkb4(this, nullptr);
			return true;
		case FL_F + 10:
			// F10 - Parse callsign
			dashboard::cb_parse(this, nullptr);
			return true;
		}
	}
	// Not handled the event - pass up the inheritance
	return intl_editor::handle(event);
}

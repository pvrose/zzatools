#include "qso_manager.h"
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
#include "qth_dialog.h"
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
extern dxa_if* dxa_if_;
extern band_view* band_view_;
extern tabbed_forms* tabbed_forms_;
extern import_data* import_data_;
void add_rig_if();
extern double prev_freq_;
extern bool read_only_;

// constructor 
qso_manager::common_grp::common_grp(int X, int Y, int W, int H, const char* label, item_t type)
	: Fl_Group(X, Y, W, H, label)
	, choice_(nullptr)
	, band_browser_(nullptr)
	, settings_name_("")
	, my_settings_(nullptr)
	, item_no_(0)
	, type_(type)
	, my_name_("")
{
	switch (type_) {
	case ANTENNA: {
		settings_name_ = "Aerials";
		break;
	}
	case RIG: {
		settings_name_ = "Rigs";
		break;
	}
	case CALLSIGN: {
		settings_name_ = "Callsigns";
		break;
	}
	case QTH: {
		settings_name_ = "QTHs";
		break;
	}
	}
	all_items_.clear();
	load_values();
	create_form(X, Y);
	end();
}

// Destructor
qso_manager::common_grp::~common_grp() {
	// Release all memory
	all_items_.clear();
}

// Get initial data from settings
void qso_manager::common_grp::load_values() {
	// Get the settings for the named group
	Fl_Preferences stations_settings(settings_, "Stations");
	my_settings_ = new Fl_Preferences(stations_settings, settings_name_.c_str());
	// Number of items described in settings
	int num_items = my_settings_->groups();
	// Get the current item
	char * text;
	my_settings_->get("Current", text, "");
	my_name_ = text;
	free(text);
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
			{
				// Antenna has just active flag and bands it is intended for
				// Get the intended bands
				char* temp;
				item_settings.get("Intended Bands", temp, "");
				bands = temp;
				free(temp);
				split_line(bands, info.intended_bands, ';');
				break;
				//
			}
			case CALLSIGN:
			{
				info.intended_bands.clear();
				break;
			}
			case QTH:
			{
				info.intended_bands.clear();
			}
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
void qso_manager::common_grp::create_form(int X, int Y) {
	// widget positions - rows
	const int R1 = HTEXT;
	const int H1 = HBUTTON;
	const int R2 = R1 + H1 + GAP;
	const int H2 = HBUTTON;
	const int R3 = R2 + H2 + GAP;
	const int H3 = HBUTTON;
	const int R4 = R3 + H3 + max(GAP, HTEXT);
	const int H4 = HMLIN;
	const int HALL = (type_ == CALLSIGN || type_ == QTH) ? R3 + H3 + GAP : R4 + H4 + GAP;
	// widget positions - columns
	const int C1 = GAP;
	const int W1B = WBUTTON;
	const int col1 = C1 + W1B + GAP;
	const int W2B = WBUTTON;
	const int W1A = col1 + W2B - C1;
	const int WALL = C1 + W1A + GAP;

	qso_manager* dash = ancestor_view<qso_manager>(this);

	//// Explicitly call begin to ensure that we haven't had too many ends.
	//begin();
	// resize the group accordingly
	resizable(nullptr);
	resize(X, Y, WALL, HALL);

	labelsize(FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	// Row 1
	// Output to display name feom settings
	Fl_Output* op2_1 = new Fl_Output(X + C1, Y + R1, W1A, H1);
	op2_1->box(FL_FLAT_BOX);
	op2_1->color(FL_BACKGROUND_COLOR);
	op2_1->labelsize(FONT_SIZE);
	op2_1->labelfont(FONT);
	//op2_1->textfont(FONT | FL_BOLD); // Font, size and colour are set in enable_widgets()
	op2_1->tooltip("Value in settings");
	op_settings_ = op2_1;

	// Row 2
	// Choice to select or add new antenna or rig
	Fl_Input_Choice* ch1_1 = new Fl_Input_Choice(X + C1, Y + R2, W1A, H2);
	ch1_1->textsize(FONT_SIZE);
	ch1_1->callback(cb_ch_stn, nullptr);
	ch1_1->when(FL_WHEN_CHANGED);
	ch1_1->tooltip("Select item");
	ch1_1->menubutton()->textfont(FONT);
	ch1_1->menubutton()->textsize(FONT_SIZE);
	choice_ = ch1_1;

	// Row 3
	// Add or modify this antenna or rig
	Fl_Button* bn3_1 = new Fl_Button(X + C1, Y + R3, W1B, H3, "Add/Modify");
	bn3_1->labelsize(FONT_SIZE);
	bn3_1->color(fl_lighter(FL_BLUE));
	bn3_1->labelcolor(FL_YELLOW);
	bn3_1->callback(cb_bn_add);
	bn3_1->when(FL_WHEN_RELEASE);
	bn3_1->tooltip("Add or modify the selected item - opens new dialog");

	// Remove this antenna or rig
	Fl_Button* bn3_2 = new Fl_Button(X + col1, Y + R3, W2B, H2, "Remove");
	bn3_2->labelsize(FONT_SIZE);
	bn3_2->color(fl_lighter(FL_RED));
	bn3_2->labelcolor(FL_YELLOW);
	bn3_2->callback(cb_bn_del);
	bn3_2->when(FL_WHEN_RELEASE);
	bn3_2->tooltip("Delete the item");

	switch(type_) {
	case RIG:
	case ANTENNA:
	{
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
		break;
	}
	case CALLSIGN:
	case QTH:
	{
		populate_choice();
		break;
	}
	}
	enable_widgets();

}

// Save values in settings
void qso_manager::common_grp::save_values() {
	qso_manager* dash = ancestor_view<qso_manager>(this);

	// Clear to remove deleted entries
	switch (type_) {
	case QTH:
		break;
	default:
		my_settings_->clear();
		break;
	}

	my_settings_->set("Current", my_name_.c_str());

	int index = 0;
	// For each item
	for (auto it = item_info_.begin(); it != item_info_.end(); it++) {
		string name = (*it).first;
		item_data& info = (*it).second;
		Fl_Preferences item_settings(my_settings_, name.c_str());

		switch (type_) {
		case RIG: {
			// set the intended bands
			string bands = "";
			// Store all the bands intended to be used with this rig/antenna
			for (auto itb = info.intended_bands.begin(); itb != info.intended_bands.end(); itb++) {
				if ((*itb).length()) {
					bands += *itb + ';';
				}
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

			break;
		}
		case ANTENNA: {
			// set the intended bands
			string bands = "";
			// Store all the bands intended to be used with this rig/antenna
			for (auto itb = info.intended_bands.begin(); itb != info.intended_bands.end(); itb++) {
				if ((*itb).length()) {
					bands += *itb + ';';
				}
			}
			item_settings.set("Intended Bands", bands.c_str());
			break;
		}
		case CALLSIGN:
		case QTH:
		{
			// No additional data - in QTH case save by separate dialog
			break;
		}
		}
	}
}

// Populate the item selector
void qso_manager::common_grp::populate_choice() {
	Fl_Input_Choice* w = (Fl_Input_Choice*)choice_;
	w->clear();
	int index = 0;
	bool sel_found = false;
	// For each item
	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
		// Chack if the antenna or rig is in active use
		item_data& info = item_info_[(*it)];
		// If item is currently active or we want to display both active and inactive items
		// Add the item name to the choice
		if ((*it).length()) {
			// Escape any slash character 
			const char* v = escape_string(*it, "/\\&_").c_str();
			w->add(v);
			if (*it == my_name_) {
				// If it's the current selection, show it as such
				w->value(v);
				item_no_ = index;
				sel_found = true;
			}
			index++;
		}
	}
	if (!sel_found && my_name_.length()) {
		// Selected item not found. Add it to the various lists
		w->add(my_name_.c_str());
		w->value(my_name_.c_str());
	}
	w->textsize(FONT_SIZE);
	w->textfont(FONT);
}

// Populate the band selection widget
void qso_manager::common_grp::populate_band() {
	// Get pointers to the widget to be populated and the top-level dialog
	Fl_Multi_Browser* mb = (Fl_Multi_Browser*)band_browser_;
	qso_manager* dash = ancestor_view<qso_manager>(mb);
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

// Enable widgets
void qso_manager::common_grp::enable_widgets() {
	// Set value and style of settings ouput widget
	char* next_value;
	Fl_Output* op = (Fl_Output*)op_settings_;
	qso_manager* dash = ancestor_view<qso_manager>(this);
	my_settings_->get("Current", next_value, "");
	if (strcmp(next_value, my_name_.c_str())) {
		op->textcolor(FL_RED);
		op->textfont(FONT | FL_BOLD | FL_ITALIC);
		op->textsize(FONT_SIZE + 2);
	}
	else {
		op->textcolor(FL_BLACK);
		op->textfont(FONT | FL_BOLD);
		op->textsize(FONT_SIZE + 2);
	}
	op->value(next_value);
	op->redraw();
	free(next_value);
	// Set values into choice
	if (!dash->items_changed_) {
		Fl_Input_Choice* ch = (Fl_Input_Choice*)choice_;
		ch->value(my_name_.c_str());
	}
}

// button callback - add
// Add the text value of the choice to the list of items - new name is typed into the choice
void qso_manager::common_grp::cb_bn_add(Fl_Widget* w, void* v) {
	common_grp* that = ancestor_view<common_grp>(w);
	qso_manager* dash = ancestor_view<qso_manager>(w);
	// Get the value in the choice
	string new_item = ((Fl_Input_Choice*)that->choice_)->value();
	dash->items_changed_ = true;
	// Open the QTH dialog
	button_t result = BN_OK;
	switch (that->type_) {
	case QTH:
	{
		qth_dialog* dialog = new qth_dialog(new_item);
		result = dialog->display();
	}
	}
	switch (result) {
	case BN_OK:
	{
		// Set it active (and add it if it's not there)
		that->all_items_.push_back(new_item);

		that->my_name_ = new_item;
		that->enable_widgets();
		that->populate_choice();
		tabbed_forms_->update_views(nullptr, HT_LOCATION, -1);

		that->redraw();
		break;
	}
	case BN_CANCEL:
	{
		break;
	}
	}

}

// button callback - delete
void qso_manager::common_grp::cb_bn_del(Fl_Widget* w, void* v) {
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

// choice callback
// v is unused
void qso_manager::common_grp::cb_ch_stn(Fl_Widget* w, void* v) {
	// Input_Choice is a descendant of common_grp
	Fl_Input_Choice* ch = ancestor_view<Fl_Input_Choice>(w); 
	common_grp* that = ancestor_view<common_grp>(ch);
	qso_manager* dash = ancestor_view<qso_manager>(that);
	if (ch->menubutton()->changed()) {
		// Get the new item from the menu
		that->item_no_ = ch->menubutton()->value();
		that->my_name_ = ch->menubutton()->text();
		item_data& info = that->item_info_[that->my_name_];
		// Update the shared choice value
		ch->value(that->my_name_.c_str());
		switch (that->type_) {
		case RIG:
		case ANTENNA:
			that->populate_band();
			break;
		}
		dash->enable_widgets();
	}
	else {
		// A new item is being typed in the input field - use ADD button to process it
	}
	dash->items_changed_ = true;
}

// Multi-browser callback
// v is usused
void qso_manager::common_grp::cb_mb_bands(Fl_Widget* w, void* v) {
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
	((qso_manager*)that->parent())->enable_widgets();
}

// Item choice call back
// v is value selected
void qso_manager::common_grp::cb_ch_item(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ipch = ancestor_view<Fl_Input_Choice>(w);
	cb_value<Fl_Input_Choice, string>(ipch, v);
	common_grp* that = ancestor_view<common_grp>(w);
	that->save_values();
	that->populate_band();
}

string& qso_manager::common_grp::name() {
	return my_name_;
}

qso_manager::item_data& qso_manager::common_grp::info() {
	return item_info_[my_name_];
}

void qso_manager::common_grp::update_settings_name() {
	my_settings_->set("Current", my_name_.c_str());
}

// qso_group_
qso_manager::qso_group::qso_group(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
	, current_qso_(nullptr)
	, logging_mode_(LM_OFF_AIR)
	, contest_id_("")
	, exch_fmt_ix_(0)
	, exch_fmt_id_("")
	, max_ef_index_(0)
	, field_mode_(NO_CONTEST)
{
	load_values();
}

// Destructor
qso_manager::qso_group::~qso_group() {
}

// Load values
void qso_manager::qso_group::load_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	// Set logging mode -default is On-air with or without rig connection
	int new_lm;
	logging_mode_t default_lm = rig_if_ ? LM_ON_AIR_CAT : LM_ON_AIR_COPY;
	dash_settings.get("Logging Mode", new_lm, default_lm);

	// If we are set to "On-air with CAT connection" check connecton
	if (!rig_if_ && new_lm == LM_ON_AIR_CAT) new_lm = LM_ON_AIR_COPY;
	
	logging_mode_ = (logging_mode_t)new_lm;

	// Contest definitions
	Fl_Preferences contest_settings(dash_settings, "Contests");
	contest_settings.get("Next Serial Number", serial_num_, 0);
	contest_settings.get("Contest Mode", (int&)field_mode_, false);
	char* temp;
	contest_settings.get("Contest ID", temp, "");
	contest_id_ = temp;
	free(temp);
	contest_settings.get("Exchange Format", exch_fmt_ix_, 0);
	// Contest exchnage formats
	Fl_Preferences format_settings(contest_settings, "Formats");
	unsigned int num_formats = format_settings.groups();
	max_ef_index_ = num_formats;
	// Already have contests set, read them
	//	Format ID (nickname)
	//		Index: n
	//		TX: Transmit fields
	//		RX: Receive fields
	for (unsigned int i = 0; i < num_formats; i++) {
		Fl_Preferences one_setting(format_settings, format_settings.group(i));
		int index;
		one_setting.get("Index", index, 0);
		ef_ids_[index] = format_settings.group(i);
		one_setting.get("TX", temp, "RST_SENT,STX");
		ef_txs_[index] = string(temp);
		free(temp);
		one_setting.get("RX", temp, "RST_RCVD,SRX");
		ef_rxs_[index] = string(temp);
		free(temp);
	}
	// Set active contest format ID
	exch_fmt_id_ = ef_ids_[exch_fmt_ix_];
}

// Create qso_group
void qso_manager::qso_group::create_form(int X, int Y) {
	int max_w = 0;
	int max_h = 0;

	begin();

	Fl_Group* g = new Fl_Group(X, Y, 0, 0, "QSO Data Entry");
	g->labelfont(FONT);
	g->labelsize(FONT_SIZE);
	g->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g->box(FL_BORDER_BOX);

	// Choice widget to select the reqiuired logging mode
	ch_logmode_ = new Fl_Choice(x() + GAP + WLLABEL, y() + HTEXT, 8 * WBUTTON, HTEXT, "QSO initialisation");
	ch_logmode_->labelfont(FONT);
	ch_logmode_->labelsize(FONT_SIZE);
	ch_logmode_->textfont(FONT);
	ch_logmode_->textsize(FONT_SIZE);
	ch_logmode_->align(FL_ALIGN_LEFT);
	ch_logmode_->add("Current date and time - used for parsing only");
	ch_logmode_->add("All fields blank");
	ch_logmode_->add("Current date and time, data from CAT");
	ch_logmode_->add("Current date and time, data from selected QSO");
	ch_logmode_->add("Current date and time, no other data");
	ch_logmode_->add("QSO set by external application");
	ch_logmode_->value(logging_mode_);
	ch_logmode_->callback(cb_logging_mode, &logging_mode_);
	ch_logmode_->tooltip("Select the logging mode - i.e. how to initialise a new QSO record");

	int curr_y = ch_logmode_->y() + ch_logmode_->h() + GAP;;
	int top = ch_logmode_->y() + ch_logmode_->h();
	int left = x() + GAP;
	int curr_x = left;

	Fl_Group* g_contest = new Fl_Group(curr_x, curr_y, 0, 0, "Contest");
	g_contest->labelfont(FONT);
	g_contest->labelsize(FONT_SIZE);
	g_contest->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_contest->box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;

	bn_enable_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Enable");
	bn_enable_->labelfont(FONT);
	bn_enable_->labelsize(FONT_SIZE);
	bn_enable_->value(field_mode_ != NO_CONTEST);
	bn_enable_->selection_color(FL_GREEN);
	bn_enable_->callback(cb_ena_contest, nullptr);
	bn_enable_->tooltip("Enable contest operation - resets contest parameters");

	curr_x += bn_enable_->w() + GAP;

	// Pause contest button
	bn_pause_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Pause");
	bn_pause_->labelfont(FONT);
	bn_pause_->labelsize(FONT_SIZE);
	bn_pause_->value(field_mode_ == PAUSED);
	bn_pause_->selection_color(FL_RED);
	bn_pause_->callback(cb_pause_contest, nullptr);
	bn_pause_->tooltip("Pause contest, i.e. keep parameters when resume");

	curr_x += bn_pause_->w() + GAP + (WLABEL * 3 / 2);

	// Contest ID - used for logging
	ch_contest_id_ = new field_choice(curr_x, curr_y, WBUTTON * 2, HBUTTON, "CONTEST_ID");
	ch_contest_id_->labelfont(FONT);
	ch_contest_id_->labelsize(FONT_SIZE);
	ch_contest_id_->textfont(FONT);
	ch_contest_id_->textsize(FONT_SIZE);
	ch_contest_id_->set_dataset("Contest_ID");
	ch_contest_id_->value(contest_id_.c_str());
	ch_contest_id_->callback(cb_value<field_choice, string>, &contest_id_);
	ch_contest_id_->tooltip("Select the ID for the contest (for logging)");

	max_w = max(max_w, curr_x + ch_contest_id_->w() + GAP - x());
	curr_x = g_contest->x() + GAP + WLABEL;
	curr_y += ch_contest_id_->h();

	// Choice widget to select the required exchanges
	ch_format_ = new Fl_Input_Choice(curr_x, curr_y, WBUTTON + WBUTTON, HBUTTON, "Exch.");
	ch_format_->labelfont(FONT);
	ch_format_->labelsize(FONT_SIZE);
	ch_format_->textfont(FONT);
	ch_format_->textsize(FONT_SIZE);
	ch_format_->menubutton()->textfont(FONT);
	ch_format_->menubutton()->textsize(FONT_SIZE);
	ch_format_->value(exch_fmt_id_.c_str());
	ch_format_->align(FL_ALIGN_LEFT);
	ch_format_->callback(cb_format, nullptr);
	ch_format_->when(FL_WHEN_RELEASE);
	ch_format_->tooltip("Select existing exchange format or type in new (click \"Add\" to add)");
	populate_exch_fmt();

	curr_x += ch_format_->w();

	// Add exchange button
	bn_add_exch_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Add");
	bn_add_exch_->labelfont(FONT);
	bn_add_exch_->labelsize(FONT_SIZE);
	bn_add_exch_->value(field_mode_ == EDIT);
	bn_add_exch_->callback(cb_add_exch, nullptr);
	bn_add_exch_->selection_color(FL_RED);
	bn_add_exch_->tooltip("Add new exchange format - choose fields and click \"TX\" or \"RX\" to create them");
	
	curr_x += bn_add_exch_->w() + GAP;

	// Define contest exchanges
	bn_define_tx_ = new Fl_Button(curr_x, curr_y, WBUTTON/2, HBUTTON, "TX");
	bn_define_tx_->labelfont(FONT);
	bn_define_tx_->labelsize(FONT_SIZE);
	bn_define_tx_->callback(cb_def_format, (void*)true);
	bn_define_tx_->tooltip("Use the specified fields as contest exchange on transmit");

	curr_x += bn_define_tx_->w();

	// Define contest
	bn_define_rx_ = new Fl_Button(curr_x, curr_y, WBUTTON/2, HBUTTON, "RX");
	bn_define_rx_->labelfont(FONT);
	bn_define_rx_->labelsize(FONT_SIZE);
	bn_define_rx_->callback(cb_def_format, (void*)false);
	bn_define_rx_->tooltip("Use the specified fields as contest exchange on receive");

	curr_x += bn_define_rx_->w() +GAP;

	// Serial number control buttons - Initialise to 001
	bn_init_serno_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@|<");
	bn_init_serno_->labelfont(FONT);
	bn_init_serno_->labelsize(FONT_SIZE);
	bn_init_serno_->callback(cb_init_serno, nullptr);
	bn_init_serno_->tooltip("Reset the contest serial number counter to \"001\"");

	curr_x += bn_init_serno_->w();

	// Serial number control buttons - Decrement
	bn_dec_serno_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@<");
	bn_dec_serno_->labelfont(FONT);
	bn_dec_serno_->labelsize(FONT_SIZE + 2);
	bn_dec_serno_->callback(cb_dec_serno, nullptr);
	bn_dec_serno_->tooltip("Decrement the contest serial number counter by 1");

	curr_x += bn_dec_serno_->w();

	// Serial number control buttons - Decrement
	bn_inc_serno_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@>");
	bn_inc_serno_->labelfont(FONT);
	bn_inc_serno_->labelsize(FONT_SIZE + 2);
	bn_inc_serno_->callback(cb_inc_serno, nullptr);
	bn_inc_serno_->tooltip("Increment the contest serial number counter by 1");

	curr_x += bn_inc_serno_->w() + GAP + WLABEL;

	// Transmitted contest exchange
	op_serno_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Serial");
	op_serno_->align(FL_ALIGN_LEFT);
	op_serno_->labelfont(FONT);
	op_serno_->labelsize(FONT_SIZE);
	op_serno_->textfont(FONT);
	op_serno_->textsize(FONT_SIZE);
	op_serno_->tooltip("This is the serial number you should be sending");

	curr_x += op_serno_->w() + GAP;
	curr_y += op_serno_->h() + GAP;

	g_contest->resizable(nullptr);
	g_contest->size(curr_x - g_contest->x(), curr_y - g_contest->y());
	g_contest->end();

	curr_x += GAP;

	max_w = max(max_w, curr_x - x());
	curr_y += GAP;
	int col2_y = curr_y;
	const int WNAME = WBUTTON * 9 / 5;
	const int WVALUE = WBUTTON * 6 / 5;
	const int WFIELD = WNAME + WVALUE + GAP;
	const int HFIELD = HBUTTON;
	curr_x = left + WNAME;

	// Input - allows user to manually enter frequency
	// Reflects current frequency per logging mde
	ip_freq_ = new Fl_Input(curr_x, curr_y, WVALUE, HFIELD, "FREQ");
	ip_freq_->textfont(FONT);
	ip_freq_->textsize(FONT_SIZE);
	ip_freq_->labelfont(FONT);
	ip_freq_->labelsize(FONT_SIZE);
	ip_freq_->align(FL_ALIGN_LEFT);
	ip_freq_->tooltip("Enter frequency of operation, if different");
	ip_freq_->callback(cb_ip_freq);

	curr_x += WFIELD;

	// Choice - allows user to select mode of operation
	// Refelcts current mode per logging mode
	ch_mode_ = new field_choice(curr_x, curr_y, WVALUE, HFIELD, "MODE");
	ch_mode_->textfont(FONT);
	ch_mode_->textsize(FONT_SIZE);
	ch_mode_->labelfont(FONT);
	ch_mode_->labelsize(FONT_SIZE);
	ch_mode_->align(FL_ALIGN_LEFT);
	ch_mode_->tooltip("Select mode of operatio, if different");
	ch_mode_->callback(cb_ch_mode);

	curr_x += WFIELD; 

	// Input - allows user to manually enter power
	// Refelcts current power per logging mode
	ip_power_ = new Fl_Input(curr_x, curr_y, WVALUE, HFIELD, "TX_PWR");
	ip_power_->textfont(FONT);
	ip_power_->textsize(FONT_SIZE);
	ip_power_->labelfont(FONT);
	ip_power_->labelsize(FONT_SIZE);
	ip_power_->align(FL_ALIGN_LEFT);
	ip_power_->tooltip("Enter transmit power, if different");
	ip_power_->callback(cb_ip_power);

	curr_x += WVALUE;
	max_w = max(max_w, curr_x);
	curr_y += HFIELD;

	string frequency = "";
	string power = "";
	string mode = "";
	string submode;
	// Get frequency, power and mode per logging mode
	switch (logging_mode_) {
	case LM_ON_AIR_CAT:
		// Get data from rig
		frequency = rig_if_->get_frequency(true);
		power = rig_if_->get_tx_power();
		rig_if_->get_string_mode(mode, submode);
		break;
	case LM_ON_AIR_COPY:
	{
		record* selected_qso = book_->get_record();
		if (selected_qso) {
			// Get data from current record (if there is one)
			frequency = selected_qso->item("FREQ");
			power = selected_qso->item("TX_PWR");
			mode = selected_qso->item("MODE", true);
		}
		break;
	}
	}
	update_frequency(frequency);
	ch_mode_->set_dataset("Combined", mode);
	ip_power_->value(power.c_str());

	curr_x = left + WNAME;

	//Callsign entry field
	ip_call_ = new Fl_Input(curr_x, curr_y, WVALUE, HBUTTON, "CALL");
	ip_call_->labelfont(FONT);
	ip_call_->labelsize(FONT_SIZE);
	ip_call_->textfont(FONT);
	ip_call_->textsize(FONT_SIZE);
	//ip_call_->callback(cb_value<Fl_Input, string>, &callsign_);
	//ip_call_->value(callsign_.c_str());
	ip_call_->tooltip("Input the call of the contact");

	curr_x = left + WFIELD;
		
	// Field1 choice and entry - array of 11 items
	const int NUMBER_COLS = 3;
	for (int ia = 0; ia < NUMBER_FIELDS; ia++) {
		ch_field_[ia] = new field_choice(curr_x, curr_y, WNAME, HFIELD);
		ch_field_[ia]->textfont(FONT);
		ch_field_[ia]->textsize(FONT_SIZE);
		ch_field_[ia]->align(FL_ALIGN_RIGHT);
		ch_field_[ia]->tooltip("Specify the field to provide");
		//ch_field_[ia]->callback(cb_field, (void*)ia);
		ch_field_[ia]->set_dataset("Fields");

		curr_x += WNAME;

		ip_field_[ia] = new Fl_Input(curr_x, curr_y, WVALUE, HFIELD);
		ip_field_[ia]->textfont(FONT);
		ip_field_[ia]->textsize(FONT_SIZE);
		ip_field_[ia]->tooltip("Specify the value of field");
		//ip_field_[ia]->callback(cb_inp_field, (void*)ia);
		if (ia % NUMBER_COLS == NUMBER_COLS - 2) {
			// Start new line of widgets
			curr_x = left;
			curr_y += HFIELD;
		}
		else {
			curr_x += WVALUE + GAP;
		}
	}

	curr_x = left + (NUMBER_COLS * WFIELD);
	// Adjust logging mode choice
	ch_logmode_->size(curr_x - ch_logmode_->x(), ch_logmode_->h());

	max_w = max(max_w, curr_x);

	// nOtes input
	curr_x = left + WNAME;
	
	ip_notes_ = new Fl_Input(curr_x, curr_y, WFIELD * NUMBER_COLS - WNAME - GAP, HBUTTON, "NOTES");
	ip_notes_->labelfont(FONT);
	ip_notes_->labelsize(FONT_SIZE);
	ip_notes_->textfont(FONT);
	ip_notes_->textsize(FONT_SIZE);
	//ip_notes_->callback(cb_value<Fl_Input, string>, &notes_);
	//ip_notes_->value(notes_.c_str());
	ip_notes_->tooltip("Add any notes for the QSO");

	curr_x = left + WLABEL;
	curr_y += HBUTTON;


	// QSO start/stop
	curr_x = left;
	curr_y += GAP;

	bn_start_qso_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Start");
	bn_start_qso_->labelfont(FONT);
	bn_start_qso_->labelsize(FONT_SIZE);
	bn_start_qso_->callback(cb_start_qso);
	bn_start_qso_->color(fl_lighter(FL_YELLOW));
	bn_start_qso_->tooltip("Start a QSO (or update current one)");

	curr_x += bn_start_qso_->w();

	bn_log_qso_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Log");
	bn_log_qso_->labelfont(FONT);
	bn_log_qso_->labelsize(FONT_SIZE);
	bn_log_qso_->callback(cb_log_qso);
	bn_log_qso_->color(fl_lighter(FL_GREEN));
	bn_log_qso_->tooltip("Save current QSO to log (or start new and save it)");

	curr_x += bn_log_qso_->w();

	bn_cancel_qso_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Cancel");
	bn_cancel_qso_->labelfont(FONT);
	bn_cancel_qso_->labelsize(FONT_SIZE);
	bn_cancel_qso_->callback(cb_cancel_qso);
	bn_cancel_qso_->color(fl_lighter(FL_RED));
	bn_cancel_qso_->tooltip("Save current QSO to log (or start new and save it)");

	curr_x += bn_cancel_qso_->w() + GAP;

	bn_wkd_b4_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "B4?");
	bn_wkd_b4_->labelfont(FONT);
	bn_wkd_b4_->labelsize(FONT_SIZE);
	bn_wkd_b4_->callback(cb_wkb4);
	bn_wkd_b4_->tooltip("Display all previous QSOs with this callsign");

	curr_x += bn_wkd_b4_->w();

	bn_parse_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "DX?");
	bn_parse_->labelfont(FONT);
	bn_parse_->labelsize(FONT_SIZE);
	bn_parse_->callback(cb_parse);
	bn_parse_->tooltip("Display the DX details for this callsign");

	max_w = max(max_w, bn_parse_->x() + bn_parse_->w());
	curr_x = left;
	curr_y += HBUTTON;

	// end of column 1 widgets
	max_h = curr_y + GAP;

	g->resizable(nullptr);
	g->size(max_w, max_h);
	g->end();

	resizable(nullptr);
	size(max_w, max_h);
	end();

	initialise_fields();
}

// Enable QSO widgets
void qso_manager::qso_group::enable_widgets() {
	// Disable log mode menu item from CAT if no CAT
	if (rig_if_ == nullptr) {
		ch_logmode_->mode(LM_ON_AIR_CAT, ch_logmode_->mode(LM_ON_AIR_CAT) | FL_MENU_INACTIVE);
	}
	else {
		ch_logmode_->mode(LM_ON_AIR_CAT, ch_logmode_->mode(LM_ON_AIR_CAT) & ~FL_MENU_INACTIVE);
	}
	ch_logmode_->redraw();
	// Get exchange data
	if (exch_fmt_ix_ < MAX_CONTEST_TYPES && field_mode_ == CONTEST) {
		char text[10];
		snprintf(text, 10, "%03d", serial_num_);
		op_serno_->value(text);
	}
	else {
		op_serno_->value("");
	}
	// Basic contest on/off widgets 
	switch (field_mode_) {
	case NO_CONTEST:
		op_serno_->deactivate();
		bn_pause_->deactivate();
		ch_contest_id_->deactivate();
		ch_format_->deactivate();
		bn_init_serno_->deactivate();
		bn_inc_serno_->deactivate();
		bn_dec_serno_->deactivate();
		break;
	default:
		op_serno_->activate();
		bn_pause_->activate();
		ch_contest_id_->activate();
		ch_format_->activate();
		bn_init_serno_->activate();
		bn_inc_serno_->activate();
		bn_dec_serno_->activate();
	}
	op_serno_->redraw();
	bn_pause_->redraw();
	ch_contest_id_->redraw();
	ch_format_->redraw();
	bn_init_serno_->redraw();
	bn_inc_serno_->redraw();
	bn_dec_serno_->redraw();
	// Mode dependent
	switch(field_mode_) {
	case NEW:
		bn_add_exch_->activate();
		bn_add_exch_->label("Add");
		bn_define_rx_->deactivate();
		bn_define_tx_->deactivate();
		break;
	case DEFINE:
	case EDIT:
		bn_add_exch_->activate();
		bn_add_exch_->label("Save");
		bn_define_rx_->activate();
		bn_define_tx_->activate();
		break;
	case CONTEST:
		bn_add_exch_->activate();
		bn_add_exch_->label("Edit");
		bn_define_rx_->deactivate();
		bn_define_tx_->deactivate();
		break;
	default:
		bn_add_exch_->deactivate();
		bn_add_exch_->label(nullptr);
		bn_define_rx_->deactivate();
		bn_define_tx_->deactivate();
		break;
	}
	bn_add_exch_->redraw();
	bn_add_exch_->redraw();
	bn_define_rx_->redraw();
	bn_define_tx_->redraw();
	// Start/Log/cancel buttons
	if (current_qso_) {
		bn_start_qso_->label("Update");
		bn_log_qso_->activate();
		bn_log_qso_->label("Log");
		bn_cancel_qso_->activate();
	}
	else {
		bn_start_qso_->label("Start");
		bn_log_qso_->activate();
		bn_log_qso_->label("Start/Log");
		bn_cancel_qso_->deactivate();
	}
	bn_start_qso_->redraw();
	bn_log_qso_->redraw();
	bn_log_qso_->redraw();
	bn_cancel_qso_->redraw();
}

// Update the fields in the record
void qso_manager::qso_group::update_record() {
	if (current_qso_) {
		// WRite (and read back) field values
		current_qso_->item("CALL", string(ip_call_->value()));
		ip_call_->value(current_qso_->item("CALL").c_str());
		for (int i = 0; i < NUMBER_FIELDS; i++) {
			string field = ch_field_[i]->value();
			string value = ip_field_[i]->value();
			if (field.length()) {
				current_qso_->item(field, value);
				ip_field_[i]->value(current_qso_->item(field).c_str());
			}
		}

		current_qso_->item("NOTES", string(ip_notes_->value()));
		ip_notes_->value(current_qso_->item("NOTES").c_str());
		enable_widgets();
		// Notify other views
		tabbed_forms_->update_views(nullptr, HT_INSERTED, book_->size() - 1);
	}
}

// Copy record to the fields - reverse of above
void qso_manager::qso_group::update_fields() {
	if (current_qso_) {
		ip_call_->value(current_qso_->item("CALL").c_str());
		for (int i = 0; i < NUMBER_FIELDS; i++) {
			string field = ch_field_[i]->value();
			if (field.length()) {
				ip_field_[i]->value(current_qso_->item(field).c_str());
			}
		}
		ip_notes_->value(current_qso_->item("NOTES").c_str());
	}
}

// Update frequency input
void qso_manager::qso_group::update_frequency(string frequency) {
	ip_freq_->value(frequency.c_str());
	double freq;
	try {
		freq = stod(frequency) * 1000;
	}
	catch (exception e) {
		freq = 0.0;
	}
	// If the frequency is outside a ham-band, display in red else in black
	if (band_view_ && !band_view_->in_band(freq)) {
		ip_freq_->textcolor(FL_RED);
	}
	else {
		ip_freq_->textcolor(FL_BLACK);
	}

}

// Copy from an existing record (or 
void qso_manager::qso_group::copy_record(record* old_record) {
	if (current_qso_) {
		for (auto f = old_record->begin(); f != old_record->end(); f++) {
			// Copy non-null field to current record
			if (current_qso_->item((*f).first).length() == 0) {
				current_qso_->item((*f).first, (*f).second, false, true);
			}
		}
		update_fields();
	}
	else {
		// Copy FREQ, MODE, TX_PWR, CALL 
		update_frequency(old_record->item("FREQ"));
		ch_mode_->value(old_record->item("MODE", true).c_str());
		ip_power_->value(old_record->item("TX_PWR").c_str());
	}
}

// Copy fields from CAT
void qso_manager::qso_group::copy_cat() {
	// Get frequency, mode and transmit power from rig
	update_frequency(rig_if_->get_frequency(true));
	// Get mode - NB USB/LSB need further processing
	string mode;
	string submode;
	rig_if_->get_string_mode(mode, submode);
	if (mode == "DATA L" || mode == "DATA U") ch_mode_->value("");
	else if (submode == "") ch_mode_->value(mode.c_str());
	else ch_mode_->value(submode.c_str());
	ip_power_->value(rig_if_->get_tx_power().c_str());
}

// Clear fields
void qso_manager::qso_group::clear_qso() {
	update_frequency("0");
	ch_mode_->value(0);
	ip_power_->value("0");
	ip_call_->value("");
	for (int i = 0; i < NUMBER_FIELDS; i++) {
		ip_field_[i]->value("");
	}
	ip_notes_->value("");
}

// Select logging mode
void qso_manager::qso_group::cb_logging_mode(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, logging_mode_t>(w, v);
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_mode_) {
	case LM_ON_AIR_CAT:
		that->copy_cat();
		break;
	case LM_ON_AIR_COPY:
		that->copy_record(book_->get_record());
		break;
	default:
		that->clear_qso();
		break;
	}
}

// Set contest enable/disable
// v - not used
void qso_manager::qso_group::cb_ena_contest(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	bool enable = false;
	cb_value<Fl_Light_Button, bool>(w, &enable);

	if (enable) {
		that->serial_num_ = 1;
		that->field_mode_ = CONTEST;
	}
	else {
		that->field_mode_ = NO_CONTEST;
	}
	that->initialise_fields();
	that->enable_widgets();
}

// Pause contest mode
void qso_manager::qso_group::cb_pause_contest(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	bool pause = false;
	cb_value<Fl_Light_Button, bool>(w, &pause);

	if (pause) {
		that->field_mode_ = PAUSED;
	}
	else {
		that->field_mode_ = CONTEST;
	}
	that->initialise_fields();
	that->enable_widgets();
}

// Set exchange format
// v is exchange
void qso_manager::qso_group::cb_format(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	// Get format ID
	that->exch_fmt_id_ = ch->value();
	// Get source
	if (ch->menubutton()->changed()) {
		// Selected menu item
		that->exch_fmt_ix_ = ch->menubutton()->value();
		that->field_mode_ = CONTEST; // Should already be so
		that->initialise_fields();
	}
	else {
		that->field_mode_ = NEW;
	}
	that->enable_widgets();
}

// Add exchange format
// v is unused
void qso_manager::qso_group::cb_add_exch(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	bool update = false;
	cb_value<Fl_Light_Button, bool>(w, &update);
	if (update) {
		// Switch to update format definitions
		switch (that->field_mode_) {
		case NEW:
			// Adding a new format - add it to the list
			that->exch_fmt_ix_ = that->add_format_id(that->exch_fmt_id_);
			that->field_mode_ = DEFINE;
			that->initialise_fields();
			break;
		case CONTEST:
			// Editing an existing format
			that->field_mode_ = EDIT;
			that->initialise_fields();
			break;
		}
		that->enable_widgets();
	} else {
		// Start/Resume contest
		that->field_mode_ = CONTEST;
		that->populate_exch_fmt();
		that->initialise_fields();
	}
	that->enable_widgets();
}

// Define contest exchange
// v - bool TX or RX
void qso_manager::qso_group::cb_def_format(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	bool tx = (bool)(long)v;
	that->add_format_def(that->exch_fmt_ix_, tx);
	that->initialise_fields();
	that->enable_widgets();
}

// Start QSO - If not started: set start time/date; copy per logging mode; add any set fields. If started: add any set fields
void qso_manager::qso_group::cb_start_qso(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	qso_manager* mgr = (qso_manager*)that->parent();
	if (that->current_qso_ == nullptr) {
		that->current_qso_ = mgr->start_qso();
	}
	that->update_record();
	that->enable_widgets();
}

// Log QSO - If not started: as cb_start_qso. Also: save QSO
void qso_manager::qso_group::cb_log_qso(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	qso_manager* mgr = (qso_manager*)that->parent();
	that->update_record();
	mgr->end_qso();
	that->serial_num_++;
	that->initialise_fields();
	that->enable_widgets();
}

// Cancel QSO - delete QSO; clear fields
void qso_manager::qso_group::cb_cancel_qso(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	if (that->current_qso_) {
		book_->delete_record(true);
		dxa_if_->clear_dx_loc();
	}
	that->initialise_fields();
	that->current_qso_ = nullptr;
	that->ip_call_->value("");
	that->ip_notes_->value("");
	for (int i = 0; i < NUMBER_FIELDS; i++) {
		that->ip_field_[i]->value("");
	}
	that->enable_widgets();
}

// Callback - Worked B4? button
void qso_manager::qso_group::cb_wkb4(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	extract_records_->extract_call(string(that->ip_call_->value()));
}

// Callback - Parse callsign
void qso_manager::qso_group::cb_parse(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	qso_manager* mgr = (qso_manager*)that->parent();
	// Create a temporary record to parse the callsign
	record* tip_record = mgr->dummy_qso();
	string message = "";
	// Set the callsign in the temporary record
	tip_record->item("CALL", string(that->ip_call_->value()));
	// Parse the temporary record
	message = pfx_data_->get_tip(tip_record);
	// Create a tooltip window at the parse button (in w) X and Y
	Fl_Window* qw = ancestor_view<qso_manager>(w);
	Fl_Window* tw = ::tip_window(message, qw->x_root() + w->x() + w->w() / 2, qw->y_root() + w->y() + w->h() / 2);
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
	delete tip_record;
}

// Reset contest serial number
void qso_manager::qso_group::cb_init_serno(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	that->serial_num_ = 1;
	that->enable_widgets();
}

// Increment serial number
void qso_manager::qso_group::cb_inc_serno(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	that->serial_num_++;
	that->enable_widgets();
}

// Decrement serial number
void qso_manager::qso_group::cb_dec_serno(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	that->serial_num_--;
	that->enable_widgets();
}

// Callback - frequency input
// v - not used
void qso_manager::qso_group::cb_ip_freq(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	if (that->current_qso_) {
		// Input is in MHz - keep as that for record, convert to kHz for band check
		string value;
		cb_value<Fl_Input, string>(w, &value);
		double freq = stod(value) * 1000;
		that->current_qso_->item("FREQ", value);
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

// Callback - band choice
void qso_manager::qso_group::cb_ch_mode(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	if (that->current_qso_) {
		// Value in choice is a sub-mode
		string value;
		cb_choice_text(w, &value);
		if (spec_data_->is_submode(value)) {
			that->current_qso_->item("SUBMODE", value);
			that->current_qso_->item("MODE", spec_data_->mode_for_submode(value));
		}
		else {
			that->current_qso_->item("MODE", value);
			that->current_qso_->item("SUBMODE", string(""));
		}
		// Update views
		tabbed_forms_->update_views(nullptr, HT_CHANGED, book_->size() - 1);
	}
}

// Callback - power input
void qso_manager::qso_group::cb_ip_power(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	if (that->current_qso_) {
		string value;
		cb_value<Fl_Input, string>(w, &value);
		that->current_qso_->item("TX_PWR", value);
		// Update views
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
	}
}

// Save the settings
void qso_manager::qso_group::save_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.set("Logging Mode", (int)logging_mode_);

	// Contest definitions
	Fl_Preferences contest_settings(dash_settings, "Contests");
	contest_settings.clear();
	contest_settings.set("Next Serial Number", serial_num_);
	contest_settings.set("Contest Mode", (int)field_mode_);
	contest_settings.set("Contest ID", contest_id_.c_str());
	contest_settings.set("Exchange Format", exch_fmt_ix_);
	// Exchanges
	Fl_Preferences format_settings(contest_settings, "Formats");
	for (int ix = 0; ix < max_ef_index_; ix++) {
		Fl_Preferences one_settings(format_settings, ef_ids_[ix].c_str());
		one_settings.set("Index", (signed)ix);
		one_settings.set("TX", ef_txs_[ix].c_str());
		one_settings.set("RX", ef_rxs_[ix].c_str());
	}
}

// Add contests
void qso_manager::qso_group::populate_exch_fmt() {
	for (int ix = 0; ix < max_ef_index_; ix++) {
		ch_format_->add(ef_ids_[ix].c_str());
	}
}

// Initialise fields from format definitions
void qso_manager::qso_group::initialise_fields() {
	string preset_fields;
	string tx_fields;
	bool lock_preset;
	bool new_fields;
	switch (field_mode_) {
	case NO_CONTEST:
	case PAUSED:
		// Non -contest mode
		preset_fields = "RST_SENT,RST_RCVD,NAME,QTH";

		new_fields = true;
		lock_preset = false;
		break;
	case CONTEST:
		// Contest mode
		preset_fields = ef_txs_[exch_fmt_ix_] + "," + ef_rxs_[exch_fmt_ix_];
		new_fields = true;
		lock_preset = true;
		break;
	case NEW:
		// Do not change existing
		preset_fields = "";
		new_fields = false;
		lock_preset = true;
		break;
	case DEFINE:
		// Define new exchange - provide base RS/Serno
		preset_fields = "RST_RCVD,SRX";
		new_fields = true;
		lock_preset = false;
		break;
	case EDIT:
		// Unlock existing definition 
		preset_fields = "";
		new_fields = false;
		lock_preset = false;
		break;
	}
	// Now set fields
	vector<string> field_names;
	split_line(preset_fields, field_names, ',');
	size_t ix = 0;
	for (ix = 0; ix < field_names.size(); ix++) {
		if (new_fields) {
			ch_field_[ix]->value(field_names[ix].c_str());
		}
		if (lock_preset) {
			ch_field_[ix]->deactivate();
		}
		else {
			ch_field_[ix]->activate();
		}
	}
	for (; ix < NUMBER_FIELDS; ix++) {
		ch_field_[ix]->value("");
		ch_field_[ix]->activate();
	}
	// Set contest format
	ch_format_->value(exch_fmt_id_.c_str());
	// Default Contest TX values
	if (field_mode_ == CONTEST) {
		// Create a dummy QSO to get "stuff"
		qso_manager* boss = ancestor_view<qso_manager>(parent());
		record* dummy_qso;
		// Start QSO refers to other widgets within qso_manager
		if (boss->created_) {
			dummy_qso = boss->start_qso(false);
		}
		else {
			dummy_qso = boss->dummy_qso();
		}
		dummy_qso->user_details();

		vector<string> tx_fields;
		split_line(ef_txs_[exch_fmt_ix_], tx_fields, ',');
		for (size_t i = 0; i < tx_fields.size(); i++) {
			if (tx_fields[i] == "RST_SENT") {
				string contest_mode = spec_data_->dxcc_mode(dummy_qso->item("MODE"));
				if (contest_mode == "CW" || contest_mode == "DATA") {
					ip_field_[i]->value("599");
				}
				else {
					ip_field_[i]->value("59");
				}
			}
			else if (tx_fields[i] == "STX") {
				char text[10];
				snprintf(text, 10, "%03d", serial_num_);
				ip_field_[i]->value(text);
			}
			else {
				ip_field_[i]->value(dummy_qso->item(tx_fields[i]).c_str());
			}
		}
		ip_call_->value("");
		ip_notes_->value("");
		for (size_t i = tx_fields.size(); i < NUMBER_FIELDS; i++) {
			ip_field_[i]->value("");
		}
	}
	else {
		ip_call_->value("");
		ip_notes_->value("");
		for (int i = 0; i < NUMBER_FIELDS; i++) {
			ip_field_[i]->value("");
		}
	}
}

// Add new format - return format index
int qso_manager::qso_group::add_format_id(string id) {
	// Add the string to the choice
	ch_format_->add(id.c_str());
	int index = ch_format_->menubutton()->find_index(id.c_str());
	// Add the format id to the array
	ef_ids_[index] = id;
	max_ef_index_ = index + 1;
	// Return the index
	return index;
}

// Add new format definition 
void qso_manager::qso_group::add_format_def(int ix, bool tx) {
	// Get the string from the field choices
	string defn = "";
	for (int i = 0; i < NUMBER_FIELDS; i++) {
		const char* field = ch_field_[i]->value();
		if (strlen(field)) {
			if (i > 0) {
				defn += ",";
			}
			defn += field;
		}
	}
	// Add the format definition to the array
	if (tx) {
		ef_txs_[ix] = defn;
	}
	else {
		ef_rxs_[ix] = defn;
	}
}

// Clock group - constructor
qso_manager::clock_group::clock_group
	(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
{
	load_values();
}

// Clock group destructor
qso_manager::clock_group::~clock_group() {
	Fl::remove_timeout(cb_timer_clock, nullptr);
	save_values();
}

// get settings
void qso_manager::clock_group::load_values() {
	// No code
}

// Create form
void qso_manager::clock_group::create_form(int X, int Y) {

	Fl_Group* g = new Fl_Group(X, Y, 10, 10, "Clock - UTC");
	g->labelfont(FONT);
	g->labelsize(FONT_SIZE);
	g->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g->box(FL_BORDER_BOX);

	int curr_x = g->x() + GAP;
	int curr_y = g->y() + HTEXT;

	const int WCLOCKS = 250;

	bn_time_ = new Fl_Button(curr_x, curr_y, WCLOCKS, 3 * HTEXT);
	bn_time_->color(FL_BLACK);
	bn_time_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_time_->labelfont(FONT | FL_BOLD);
	bn_time_->labelsize(5 * FONT_SIZE);
	bn_time_->labelcolor(FL_YELLOW);
	bn_time_->box(FL_FLAT_BOX);

	curr_y += bn_time_->h();

	bn_date_ = new Fl_Button(curr_x, curr_y, WCLOCKS, HTEXT * 3 / 2);
	bn_date_->color(FL_BLACK);
	bn_date_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_date_->labelfont(FONT);
	bn_date_->labelsize(FONT_SIZE * 3 / 2);
	bn_date_->labelcolor(FL_YELLOW);
	bn_date_->box(FL_FLAT_BOX);

	curr_y += bn_date_->h();

	bn_local_ = new Fl_Button(curr_x, curr_y, WCLOCKS, HTEXT * 3 / 2);
	bn_local_->color(FL_BLACK);
	bn_local_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_local_->labelfont(FONT);
	bn_local_->labelsize(FONT_SIZE * 3 / 2);
	bn_local_->labelcolor(FL_RED);
	bn_local_->box(FL_FLAT_BOX);

	curr_x += GAP + WCLOCKS;
	curr_y += bn_local_->h() + GAP;

	g->resizable(nullptr);
	g->size(curr_x - g->x(), curr_y - g->y());
	g->end();

	resizable(nullptr);
	size(g->w(), g->h());
	show();
	end();

	// Start clock timer
	Fl::add_timeout(0, cb_timer_clock, this);
}

// Enable/disab;e widgets
void qso_manager::clock_group::enable_widgets() {
	// NO CODE
}

// save value
void qso_manager::clock_group::save_values() {
	// No code
}

// Callback - 1s timer
void qso_manager::clock_group::cb_timer_clock(void* v) {
	// Update the label in the clock button which is passed as the parameter
	clock_group* that = (clock_group*)v;
	time_t now = time(nullptr);
	tm* value = gmtime(&now);
	char result[100];
	// convert to C string, then C++ string
	strftime(result, 99, "%H:%M:%S", value);
	that->bn_time_->copy_label(result);
	// Convert date
	strftime(result, 99, "%A %d %B %Y", value);
	that->bn_date_->copy_label(result);
	// Convert local time
	value = localtime(&now);
	strftime(result, 99, "%T %Z", value);
	that->bn_local_->copy_label(result);

	Fl::repeat_timeout(UTC_TIMER, cb_timer_clock, v);
}


// The main dialog constructor
qso_manager::qso_manager(int W, int H, const char* label) :
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
	, wait_connect_(true)
	, created_(false)
	, font_(FONT)
	, fontsize_(FONT_SIZE)
	, all_ports_(false)
	, previous_swr_alarm_(SWR_OK)
	, previous_pwr_alarm_(POWER_OK)
	, previous_vdd_alarm_(VDD_OK)
	, current_swr_alarm_(SWR_OK)
	, current_pwr_alarm_(POWER_OK)
	, current_vdd_alarm_(VDD_OK)
	, last_tx_swr_(1.0)
	, last_tx_pwr_(0.0)
{
	ordered_bands_.clear();

	load_values();

	create_form(0,0);

	update_rig();
	update_qso();

	end();
	show();
}

// Destructor
qso_manager::~qso_manager()
{
	save_values();
}

// Handle FL_HIDE and FL_SHOW to get menu to update itself
int qso_manager::handle(int event) {

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
void qso_manager::create_form(int X, int Y) {

	// Used to evaluate total width and height of the window
	int max_x = X;
	int max_y = Y;
	// Used to maintain the relative positions of the groups
	int curr_x = X + GAP;
	int curr_y = Y + GAP;

	qso_group_ = new qso_group(curr_x, curr_y, 0, 0, nullptr);
	qso_group_->create_form(curr_x, curr_y);
	curr_x += qso_group_->w();
	curr_y += qso_group_->h();
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
	clock_group_ = new clock_group(curr_x, curr_y, 0, 0, nullptr);
	clock_group_->create_form(curr_x, curr_y);
	curr_x += clock_group_->w();
	curr_y += clock_group_->h();
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	this->resizable(nullptr);
	this->size(max_x + GAP - X, max_y + GAP - Y);
	created_ = true;

	enable_widgets();
}

//// Create scratchpad widgets
//void qso_manager::create_spad_widgets(int& curr_x, int& curr_y) {
//	int max_w = 0;
//	int max_h = 0;
//
//	// Scratchpad widgets
//	Fl_Group* gsp = new Fl_Group(curr_x, curr_y, 10, 10, "QSO Scratchpad");
//	gsp->labelfont(FONT);
//	gsp->labelsize(FONT_SIZE);
//	gsp->box(FL_BORDER_BOX);
//	gsp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//
//
//	const int HG = (7 * HBUTTON) + (4 * HTEXT) + (3 * GAP);
//	// Editor to capture keyboard entries during phone/cW QSOs
//	buffer_ = new Fl_Text_Buffer(1024);
//	editor_ = new spad_editor(gsp->x() + GAP, ch_logmode_->y() + ch_logmode_->h(), WEDITOR, HG);
//	editor_->labelfont(FONT);
//	editor_->labelsize(FONT_SIZE);
//	editor_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
//	editor_->buffer(buffer_);
//	editor_->textsize(fontsize_);
//	editor_->textfont(font_);
//	// The callback will be explicitly done in the handle routine of the editor
//	editor_->when(FL_WHEN_NEVER);
//	// Allways wrap at a word boundary
//	editor_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
//	max_w = max(max_w, editor_->x() + editor_->w());
//	max_h = max(max_h, editor_->y() + editor_->h());
//
//	// Create the buttons - see labels and tooltips for more information
//	// First we create an invisible group
//	Fl_Group* g = new Fl_Group(editor_->x() + editor_->w() + GAP, editor_->y(), WBUTTON * 2, HG);
//	g->box(FL_NO_BOX);
//
//	const int col1 = g->x();
//	const int col2 = col1 + WBUTTON;
//	int y = g->y();
//
//	// Button - start QSO
//	bn_start_ = new Fl_Button(col1, y, WBUTTON, HBUTTON, "Start");
//	bn_start_->labelsize(FONT_SIZE);
//	bn_start_->labelfont(FONT);
//	bn_start_->tooltip("Create a new record");
//	bn_start_->callback(cb_start);
//	// Button - query worked before?
//	Fl_Button* bn_wb4 = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F9 - B4?");
//	bn_wb4->labelsize(FONT_SIZE);
//	bn_wb4->labelfont(FONT);
//	bn_wb4->tooltip("Display previous QSOs");
//	bn_wb4->callback(cb_wkb4);
//
//	// Button - parse callsign
//	y += bn_wb4->h();
//	Fl_Button* bn_parse = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F10 - Parse");
//	bn_parse->labelsize(FONT_SIZE);
//	bn_parse->labelfont(FONT);
//	bn_parse->tooltip("Parse selection as a callsign");
//	bn_parse->callback(cb_parse);
//
//	y += bn_parse->h() + GAP;
//	// Button - copy call to record (starts QSO if not started)
//	Fl_Button* bn_call = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F1 - Call");
//	bn_call->labelsize(FONT_SIZE);
//	bn_call->labelfont(FONT);
//	bn_call->tooltip("Copy selected text to callsign field");
//	bn_call->callback(cb_action, (void*)WRITE_CALL);
//
//	// Button - copy name to record
//	Fl_Button* bn_name = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F2 - Name");
//	bn_name->labelsize(FONT_SIZE);
//	bn_name->labelfont(FONT);
//	bn_name->tooltip("Copy selected text to name field");
//	bn_name->callback(cb_action, (void*)WRITE_NAME);
//
//	y += HBUTTON;
//	// Button - copy QTH to record
//	Fl_Button* bn_qth = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F3 - QTH");
//	bn_qth->labelsize(FONT_SIZE);
//	bn_qth->labelfont(FONT);
//	bn_qth->tooltip("Copy selected text to QTH field");
//	bn_qth->callback(cb_action, (void*)WRITE_QTH);
//
//	// Button - copy grid to record
//	Fl_Button* bn_grid = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F4 - Grid");
//	bn_grid->labelsize(FONT_SIZE);
//	bn_grid->labelfont(FONT);
//	bn_grid->tooltip("Copy selected text to gridsquare field");
//	bn_grid->callback(cb_action, (void*)WRITE_GRID);
//
//	y += HBUTTON;
//	// Button - copy RS(T/Q) sent to record
//	Fl_Button* bn_rst_sent = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F5 - Sent");
//	bn_rst_sent->labelsize(FONT_SIZE);
//	bn_rst_sent->labelfont(FONT);
//	bn_rst_sent->tooltip("Copy selected text to RST sent field");
//	bn_rst_sent->callback(cb_action, (void*)WRITE_RST_SENT);
//
//	// Button - copy RS(T/Q) received to record
//	Fl_Button* bn_rst_rcvd = new Fl_Button(col2, y, WBUTTON, HBUTTON, "F6 - Rcvd");
//	bn_rst_rcvd->labelsize(FONT_SIZE);
//	bn_rst_rcvd->labelfont(FONT);
//	bn_rst_rcvd->tooltip("Copy selected text to RST Received field");
//	bn_rst_rcvd->callback(cb_action, (void*)WRITE_RST_RCVD);
//
//	y += HBUTTON;
//	// Button - copy selected field to record
//	Fl_Button* bn_field = new Fl_Button(col1, y, WBUTTON, HBUTTON, "Field");
//	bn_field->labelsize(FONT_SIZE);
//	bn_field->labelfont(FONT);
//	bn_field->tooltip("Copy selected text to the field specified below");
//	bn_field->callback(cb_action, (void*)WRITE_FIELD);
//
//	y += HBUTTON;
//	// Choice - allow specified field to be selected
//	field_choice* ch_field = new field_choice(col1, y, WBUTTON + WBUTTON, HTEXT);
//	ch_field->textfont(FONT);
//	ch_field->textsize(FONT_SIZE);
//	ch_field->tooltip("Select the field you want the selected text to be written to");
//	ch_field->callback(cb_text<Fl_Choice, string>, (void*)&field_);
//	ch_field->repopulate(true, "");
//
//	y += HTEXT + GAP;
//
//	// Button - save the record
//	bn_save_ = new Fl_Button(col1, y, WBUTTON, HBUTTON, "F7 - Save");
//	bn_save_->labelsize(FONT_SIZE);
//	bn_save_->labelfont(FONT);
//	bn_save_->tooltip("Save the record");
//	bn_save_->callback(cb_save);
//
//	// Button - cancel the record
//	bn_cancel_ = new Fl_Button(col2, y, WBUTTON, HBUTTON);
//	bn_cancel_->labelsize(FONT_SIZE);
//	bn_cancel_->labelfont(FONT);
//	bn_cancel_->tooltip("Cancel the record");
//	bn_cancel_->callback(cb_cancel);
//
//	g->resizable(nullptr);
//	g->end();
//
//	gsp->resizable(nullptr);
//	gsp->size(g->x() + g->w(), max(editor_->h(), g->h()) + ch_logmode_->h() + HTEXT + GAP);
//	gsp->end();
//
//	max_w = max(max_w, gsp->x() + gsp->w() - curr_x);
//	max_h = max(max_h, gsp->y() + gsp->h() - curr_y);
//	curr_x += max_w;
//	curr_y += max_h;
//
//}

// Antenna and Rig ("Use") widgets
// assumes qso_manager is the current active group
void qso_manager::create_use_widgets(int& curr_x, int& curr_y) {

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

	// Box to contain antenna-rig connectivity
	Fl_Help_View* w19 = new Fl_Help_View(antenna_grp_->x(), curr_y + max_h + GAP, max_w, HBUTTON);
	w19->box(FL_FLAT_BOX);
	w19->labelfont(FONT);
	w19->labelsize(FONT_SIZE);
	w19->textfont(FONT);
	w19->textsize(FONT_SIZE);
	ant_rig_box_ = w19;
	int max_h1 = max_h + w19->h();

	// Callsign group
	callsign_grp_ = new common_grp(rig_grp_->x() + rig_grp_->w() + GAP, antenna_grp_->y(), 10, 10, "Callsigns", CALLSIGN);
	callsign_grp_->tooltip("Select the station callsign for this QSO");
	callsign_grp_->end(); 
	callsign_grp_->show();
	max_w = max(max_w, callsign_grp_->x() + callsign_grp_->w() - curr_x);
	int max_h2 = callsign_grp_->y() + callsign_grp_->h() - curr_y;

	// QTH group
	qth_grp_ = new common_grp(callsign_grp_->x(), callsign_grp_->y() + callsign_grp_->h() + GAP, 10, 10, "QTHs", QTH);
	qth_grp_->tooltip("Select the location for this QSO");
	qth_grp_->end();
	qth_grp_->show();

	max_w = max(max_w, qth_grp_->x() + qth_grp_->w() - curr_x);
	max_h2 += qth_grp_->h();

	int h19 = w19->h() + max_h2 - max_h1;
	max_h1 = min(max_h1, max_h2);
	w19->size(w19->w(), h19);
	max_h = max(max_h1, max_h2);

	Fl_Group* guse = new Fl_Group(callsign_grp_->x() + callsign_grp_->w() + GAP, antenna_grp_->y(), 10, 10, "Use in QSOs");
	guse->labelfont(FONT);
	guse->labelsize(FONT_SIZE);
	guse->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	guse->box(FL_BORDER_BOX);

	// Use rig/antenna buttons
	// Only use the selected antenna and rig in the current QSPO
	Fl_Button* w20 = new Fl_Button(guse->x() + GAP, guse->y() + HTEXT, WBUTTON, HBUTTON, "Now");
	w20->labelfont(FONT);
	w20->labelsize(FONT_SIZE);
	w20->callback(cb_bn_use_items, (void*)(long)SELECTED_ONLY);
	w20->tooltip("Use items in current QSO");

	// Use in the current QSO and any subsequent ones (until changed)
	Fl_Button* w21 = new Fl_Button(w20->x(), w20->y() + w20->h(), WBUTTON, HBUTTON, "Now/Next");
	w21->labelfont(FONT);
	w21->labelsize(FONT_SIZE);
	w21->callback(cb_bn_use_items, (void*)(long)SELECTED_NEW);
	w21->tooltip("Use items in current QSO and future QSOs");

	// Use in subsequent QSOs
	Fl_Button* w22 = new Fl_Button(w21->x(), w21->y() + w21->h(), WBUTTON, HBUTTON, "Next");
	w22->labelfont(FONT);
	w22->labelsize(FONT_SIZE);
	w22->callback(cb_bn_use_items, (void*)(long)NEW_ONLY);
	w22->tooltip("Use items in future QSOs");

	// Use in subsequent QSOs
	Fl_Button* w23 = new Fl_Button(w22->x(), w22->y() + w22->h(), WBUTTON, HBUTTON, "Cancel");
	w23->labelfont(FONT);
	w23->labelsize(FONT_SIZE);
	w23->callback(cb_bn_use_items, (void*)(long)CANCEL_USE);
	w23->tooltip("Cancel selection");

	guse->resizable(nullptr);
	guse->size(GAP + w20->w() + GAP, HTEXT + w20->h() + w21->h() + w21->h() + + w22->h() + GAP);
	guse->end();

	max_w = max(max_w, guse->x() + guse->w() - curr_x);
	max_h = max(max_h, guse->y() + guse->h() - curr_y);

	curr_x += max_w;
	curr_y += max_h + GAP;

}

// CAT settings widgets
void qso_manager::create_cat_widgets(int& curr_x, int& curr_y) {

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
void qso_manager::create_alarm_widgets(int& curr_x, int& curr_y) {

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
	dial_swr_->selection_color(FL_BLACK);
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

// Load values
void qso_manager::load_values() {
	order_bands();

	// These are static, but will get to the same value each time
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences log_settings(user_settings, "Scratchpad");
	log_settings.get("Font Name", (int&)font_, FONT);
	log_settings.get("Font Size", (int&)fontsize_, FONT_SIZE);

	Fl_Preferences stations_settings(settings_, "Stations");


	load_locations();

}

void qso_manager::load_locations() {
	Fl_Preferences stations_settings(settings_, "Stations");

	// Get the settings for the list of QTHs
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	// Number of items described in settings
	int num_items = qths_settings.groups();
	// For each item in the settings
	for (int i = 0; i < num_items; i++) {
		// Get that item's settings
		string name = qths_settings.group(i);
	}

}

// Write values back to settings - write the three groups back separately
void qso_manager::save_values() {

	// Save window position
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.set("Left", x_root());
	dash_settings.set("Top", y_root());
	dash_settings.set("Enabled", (signed int)shown());
	dash_settings.set("Logging Mode", logging_mode());

	Fl_Preferences stations_settings(settings_, "Stations");
	qso_group_->save_values();
	rig_grp_->save_values();
	antenna_grp_->save_values();
	callsign_grp_->save_values();
	qth_grp_->save_values();
	clock_group_->save_values();
}

// Enable the widgets - activate the group associated with each rig handler when that
// handler is enabled
void qso_manager::enable_widgets() {

	// Not all widgets may exist yet!
	if (!created_) return;

	qso_group_->enable_widgets();
	enable_cat_widgets();
	enable_use_widgets();
	enable_alarm_widgets();

}

// Enable Antenna and rig setting widgets
void qso_manager::enable_use_widgets() {

	antenna_grp_->enable_widgets();
	rig_grp_->enable_widgets();
	callsign_grp_->enable_widgets();
	qth_grp_->enable_widgets();

	// Antenna/rig compatibility
	Fl_Help_View* mlo = (Fl_Help_View*)ant_rig_box_;
	if (rig_if_ || qso_group_->current_qso_) {
		// We have either a CAT connection or a previous record to cop
		double frequency;
		// Get logging mode frequency
		if (rig_if_) {
			frequency = rig_if_->tx_frequency();
		}
		else {
			qso_group_->current_qso_->item("FREQ", frequency);
		}
		string rig_band = spec_data_->band_for_freq(frequency / 1000000.0);
		// Check if band supported by rig
		vector<string> bands = rig_grp_->info().intended_bands;
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
		vector<string> r_bands = rig_grp_->info().intended_bands;
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
			mlo->color(FL_DARK_GREEN);
			mlo->value("Antenna is intended for a band the rig is capable of");
		}
	}
	mlo->textcolor(fl_contrast(FL_BLACK, mlo->color()));
}

// Update just the Locations widget from settings
void qso_manager::update_locations() {
	load_locations();
}

// Enable CAT Connection widgets
void qso_manager::enable_cat_widgets() {

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
void qso_manager::enable_alarm_widgets() {

	if (rig_if_) {
		alarms_grp_->activate();

		// SWR widget - set colour and raise alarm
		if (previous_swr_alarm_ != current_swr_alarm_) {
			char message[200];
			switch (current_swr_alarm_) {
			case SWR_ERROR:
				dial_swr_->selection_color(FL_RED);
				snprintf(message, 200, "DASH: SWR %g > %g", dial_swr_->Fl_Line_Dial::value(), dial_swr_->alarm2());
				status_->misc_status(ST_ERROR, message);
				break;
			case SWR_WARNING:
				dial_swr_->selection_color(fl_color_average(FL_RED, FL_YELLOW, 0.33f));
				snprintf(message, 200, "DASH: SWR %g > %g", dial_swr_->Fl_Line_Dial::value(), dial_swr_->alarm1());
				status_->misc_status(ST_WARNING, message);
				break;
			case SWR_OK:
				dial_swr_->selection_color(FL_BLACK);
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
				dial_pwr_->selection_color(FL_BLACK);
				snprintf(message, 200, "DASH: Power %g OK", dial_pwr_->Fl_Line_Dial::value());
				status_->misc_status(ST_OK, message);
				break;
			case POWER_WARNING:
				dial_pwr_->selection_color(fl_color_average(FL_RED, FL_YELLOW, 0.33f));
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
				dial_vdd_->selection_color(FL_RED);
				snprintf(message, 200, "DASH: Vdd %g < %g", dial_vdd_->Fl_Line_Dial::value(), dial_vdd_->alarm1());
				status_->misc_status(ST_ERROR, message);
				break;
			case VDD_OK:
				dial_vdd_->selection_color(FL_BLACK);
				snprintf(message, 200, "DASH: Vdd %g OK", dial_vdd_->Fl_Line_Dial::value());
				status_->misc_status(ST_OK, message);
				break;
			case VDD_OVER:
				dial_vdd_->selection_color(FL_RED);
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

// Read the list of bands from the ADIF specification and put them in frequency order
void qso_manager::order_bands() {
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
void qso_manager::populate_model_choice() {
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
void qso_manager::cb_rad_handler(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
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
void qso_manager::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	qso_manager* that = ancestor_view<qso_manager>(w);
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
void qso_manager::cb_ch_port(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_manager::cb_ch_baud(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->baud_rate);
}

// Override rig capabilities selected - repopulate the baud choice
// v is uused
void qso_manager::cb_ch_over(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
	cb_value<Fl_Check_Button, bool>(w, (void*)&info->override_caps);
	that->populate_baud_choice();
}

// Select "display all ports" in port choice
// v is a pointer to the all ports flag
void qso_manager::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_manager* that = ancestor_view<qso_manager>(w);
	that->populate_port_choice();
}

// Select Flrig server IP Address
// v is unused
void qso_manager::cb_ip_ipa(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
	flrig_data* info = &that->rig_grp_->info().rig_data.flrig_params;
	cb_value<Fl_Input, string>(w, (void*)&info->ip_address);
}

// Select Flrig server port number
// v is unused
void qso_manager::cb_ip_portn(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
	flrig_data* info = &that->rig_grp_->info().rig_data.flrig_params;
	cb_value_int<Fl_Int_Input>(w, (void*)&info->port);
}

// Selecte Flrig resource ID
// v is unused
void qso_manager::cb_ip_resource(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
	flrig_data* info = &that->rig_grp_->info().rig_data.flrig_params;
	cb_value<intl_input, string>(w, (void*)&info->resource);
}

// Changed the SWR wrning level
// v is unused
void qso_manager::cb_alarm_swr(Fl_Widget* w, void* v) {
	alarm_dial* dial = ancestor_view<alarm_dial>(w);
	qso_manager* that = ancestor_view<qso_manager>(dial);
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
void qso_manager::cb_alarm_pwr(Fl_Widget* w, void* v) {
	alarm_dial* dial = ancestor_view<alarm_dial>(w);
	qso_manager* that = ancestor_view<qso_manager>(dial);
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
void qso_manager::cb_alarm_vdd(Fl_Widget* w, void* v) {
	alarm_dial* dial = ancestor_view<alarm_dial>(w);
	qso_manager* that = ancestor_view<qso_manager>(dial);
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
void qso_manager::cb_ctr_pollfast(Fl_Widget* w, void* v) {
	// Get the warning level
	qso_manager* that = ancestor_view<qso_manager>(w);
	cb_value<Fl_Spinner, double>(w, &that->rig_grp_->info().rig_data.fast_poll_interval);
}

// Changed the fast polling interval
// v is not used
void qso_manager::cb_ctr_pollslow(Fl_Widget* w, void* v) {
	// Get the warning level
	qso_manager* that = ancestor_view<qso_manager>(w);
	cb_value<Fl_Spinner, double>(w, &that->rig_grp_->info().rig_data.slow_poll_interval);
}

// Pressed the connect button
// v is not used
void qso_manager::cb_bn_connect(Fl_Widget* w, void* v) {
	qso_manager* that = ancestor_view<qso_manager>(w);
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
void qso_manager::cb_bn_use_items(Fl_Widget* w, void* v) {
	use_item_t use = (use_item_t)(long)v;
	qso_manager* that = ancestor_view<qso_manager>(w);
	record* sel_record = book_->get_record();
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	switch (use) {
	case SELECTED_ONLY:
		// Update selected record only with antenna and rig
		sel_record->item("MY_RIG", that->rig_grp_->name());
		sel_record->item("MY_ANTENNA", that->antenna_grp_->name());
		sel_record->item("STATION_CALLSIGN", that->callsign_grp_->name());
		sel_record->item("APP_ZZA_QTH", that->qth_grp_->name());
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->selection());
		break;
	case SELECTED_NEW:
		// Update selected and subsequent records with antenna and rig
		sel_record->item("MY_RIG", that->rig_grp_->name());
		sel_record->item("MY_ANTENNA", that->antenna_grp_->name());
		sel_record->item("APP_ZZA_QTH", that->qth_grp_->name());
		sel_record->item("STATION_CALLSIGN", that->callsign_grp_->name());
		that->rig_grp_->update_settings_name();
		that->antenna_grp_->update_settings_name();
		that->callsign_grp_->update_settings_name();
		that->qth_grp_->update_settings_name();
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->selection());
		break;
	case NEW_ONLY:
		// Set subsequent records with antenna and rig
		sel_record->user_details();
		that->rig_grp_->update_settings_name();
		that->antenna_grp_->update_settings_name();
		that->callsign_grp_->update_settings_name();
		that->qth_grp_->update_settings_name();
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->selection());
		break;
	case CANCEL_USE:
		// Do nothing except clear in use flag
		break;
	}
	that->items_changed_ = false;
}


// Close button clicked - check editing or not
// v is not used
void qso_manager::cb_close(Fl_Widget* w, void* v) {
	// It is the window that raised this callback
	qso_manager* that = (qso_manager*)w;
	// If we are editing does the user want to save or cancel?
	if (that->qso_group_->current_qso_ != nullptr) {
		if (fl_choice("Entering a record - save or cancel?", "Cancel", "Save", nullptr) == 1) {
			qso_group::cb_log_qso(w, v);
		}
		else {
			qso_group::cb_cancel_qso(w, v);
		}
	}
	// Mark qso_manager disabled, hide it and update menu item
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Enabled", (int)false);
	menu_->update_items();
}

// Populate the choice with the available ports
void qso_manager::populate_port_choice() {
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

// Populate the baud rate choice menu
void qso_manager::populate_baud_choice() {
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

// Return the logging mode
qso_manager::logging_mode_t qso_manager::logging_mode() {
	return qso_group_->logging_mode_;
}

// Set the logging mode
void qso_manager::logging_mode(logging_mode_t mode) {
	qso_group_->logging_mode_ = mode;
	enable_widgets();
}




// Return true if we have a current QSO
bool qso_manager::qso_in_progress() {
	return qso_group_->current_qso_ != nullptr;
}

// Called when rig is read to update values here
void qso_manager::rig_update(string frequency, string mode, string power) {
	qso_group_->update_frequency(frequency);
	// Power in watts
	qso_group_->ip_power_->value(power.c_str());
	// Mode - index into choice
	qso_group_->ch_mode_->value(mode.c_str());
}

// Get QSO information from previous record not rig
void qso_manager::update_rig() {
	// Get freq etc from QSO or rig
	// Get present values data from rig
	switch (qso_group_->logging_mode_) {
	case LM_IMPORTED:
	case LM_OFF_AIR:
	case LM_ON_AIR_TIME:
		// Do nothing
		break;
	case LM_ON_AIR_CAT: {
		if (rig_if_) {
			dial_swr_->value(rig_if_->swr_meter());
			dial_pwr_->value(rig_if_->pwr_meter());
			dial_vdd_->value(rig_if_->vdd_meter());
			string rig_name = rig_if_->rig_name();
			if (rig_name != rig_grp_->name()) {
				rig_grp_->name() = rig_name;
				rig_grp_->populate_choice();
			}
			qso_group_->update_frequency(rig_if_->get_frequency(true));
			qso_group_->ip_power_->value(rig_if_->get_tx_power().c_str());
			string mode;
			string submode;
			rig_if_->get_string_mode(mode, submode);
			// Set to SUBMODE if exists else MODE
			if (submode.length()) {
				qso_group_->ch_mode_->value(submode.c_str());
			}
			else {
				qso_group_->ch_mode_->value(mode.c_str());
			}
			if (band_view_) {
				double freq = stod(rig_if_->get_frequency(true)) * 1000.0;
				band_view_->update(freq);
				prev_freq_ = freq;
			}
		}
		else {
			// We have now disconnected rig - disable selecting this logging mode
			qso_group_->logging_mode_ = LM_ON_AIR_TIME;
			enable_widgets();
		}
		break;
	}
	case LM_ON_AIR_COPY: {
		record* prev_record = book_->get_record();
		if (qso_in_progress() && prev_record == qso_group_->current_qso_) {
			qso_group_->update_fields();
		}
		// Assume as it's a logged record, the frequency is valid - NOT!
		qso_group_->update_frequency(prev_record->item("FREQ"));
		qso_group_->ip_power_->value(prev_record->item("TX_PWR").c_str());
		qso_group_->ch_mode_->value(prev_record->item("MODE", true).c_str());
		if (band_view_ && prev_record->item_exists("FREQ")) {
			double freq = stod(prev_record->item("FREQ")) * 1000.0;
			band_view_->update(freq);
			prev_freq_ = freq;
		}

		antenna_grp_->name() = prev_record->item("MY_ANTENNA");
		antenna_grp_->populate_choice();
		rig_grp_->name() = prev_record->item("MY_RIG");
		rig_grp_->populate_choice();
		break;
	}
	default:
		// No default
		qso_group_->update_frequency("0");
		qso_group_->ip_power_->value("0");
		qso_group_->ch_mode_->value(0);
		antenna_grp_->name() = "";
		rig_grp_->name() = "";
	}
	enable_use_widgets();
}

// Called whenever another view updates a record (or selects a new one)
void qso_manager::update_qso() {
	record* prev_record = book_->get_record();
	if (qso_in_progress() && prev_record == qso_group_->current_qso_) {
		// Update the view if another view changes the record
		qso_group_->update_fields();
		antenna_grp_->name() = prev_record->item("MY_ANTENNA");
		antenna_grp_->populate_choice();
		rig_grp_->name() = prev_record->item("MY_RIG");
		rig_grp_->populate_choice();
		callsign_grp_->name() = prev_record->item("STATION_CALLSIGN");
	}
	else if (qso_group_->current_qso_ == nullptr) {
		// Not actively recording a QSO, update displayed fields
		qso_group_->copy_record(prev_record);
	}
}

// Start QSO
record* qso_manager::start_qso(bool add_to_book) {

	record* new_record = new record;
	string mode;
	string submode;
	string timestamp = now(false, "%Y%m%d%H%M%S");
	record* copy_from = book_->get_record();

	switch (qso_group_->logging_mode_) {
	case LM_IMPORTED:
		// No pre-population
		break;
	case LM_OFF_AIR:
		// Prepopulate rig, antenna, QTH and callsign
		new_record->item("STATION_CALLSIGN", callsign_grp_->name());
		new_record->item("MY_RIG", rig_grp_->name());
		new_record->item("MY_ANTENNA", antenna_grp_->name());
		new_record->item("APP_ZZA_QTH", qth_grp_->name());
		break;
	case LM_ON_AIR_CAT:
		// Interactive mode - start QSO - update fields from radio 
		// Get current date and time in UTC
		new_record->item("QSO_DATE", timestamp.substr(0, 8));
		// Time as HHMMSS - always log seconds.
		new_record->item("TIME_ON", timestamp.substr(8));
		new_record->item("QSO_DATE_OFF", string(""));
		new_record->item("TIME_OFF", string(""));
		new_record->item("CALL", string(""));
		// Get frequency, mode and transmit power from rig
		new_record->item("FREQ", rig_if_->get_frequency(true));
		if (rig_if_->is_split()) {
			new_record->item("FREQ_RX", rig_if_->get_frequency(false));
		}
		// Get mode - NB USB/LSB need further processing
		rig_if_->get_string_mode(mode, submode);
		new_record->item("MODE", mode);
		new_record->item("SUBMODE", submode);
		new_record->item("TX_PWR", rig_if_->get_tx_power());
		// initialise fields
		new_record->item("RX_PWR", string(""));
		new_record->item("RST_SENT", string(""));
		new_record->item("RST_RCVD", string(""));
		new_record->item("NAME", string(""));
		new_record->item("QTH", string(""));
		new_record->item("GRIDSQUARE", string(""));
		// Prepopulate rig, antenna, QTH and callsign
		new_record->item("STATION_CALLSIGN", callsign_grp_->name());
		new_record->item("MY_RIG", rig_grp_->name());
		new_record->item("MY_ANTENNA", antenna_grp_->name());
		new_record->item("APP_ZZA_QTH", qth_grp_->name());
		break;
	case LM_ON_AIR_COPY:
		// Interactive mode - start QSO - date/time only
		// Get current date and time in UTC
		new_record->item("QSO_DATE", timestamp.substr(0, 8));
		// Time as HHMMSS - always log seconds.
		new_record->item("TIME_ON", timestamp.substr(8));
		new_record->item("QSO_DATE_OFF", string(""));
		new_record->item("TIME_OFF", string(""));
		new_record->item("CALL", string(""));
		// otherwise leave blank so that we enter it manually later.
		new_record->item("FREQ", copy_from->item("FREQ"));
		new_record->item("FREQ_RX", copy_from->item("FREQ_RX"));
		new_record->item("MODE", copy_from->item("MODE"));
		new_record->item("SUBMODE", copy_from->item("SUBMODE"));
		new_record->item("TX_PWR", copy_from->item("TX_PWR"));
		// initialise fields
		new_record->item("RX_PWR", string(""));
		new_record->item("RST_SENT", string(""));
		new_record->item("RST_RCVD", string(""));
		new_record->item("NAME", string(""));
		new_record->item("QTH", string(""));
		new_record->item("GRIDSQUARE", string(""));
		// Prepopulate rig, antenna, QTH and callsign
		new_record->item("STATION_CALLSIGN", callsign_grp_->name());
		new_record->item("MY_RIG", rig_grp_->name());
		new_record->item("MY_ANTENNA", antenna_grp_->name());
		new_record->item("APP_ZZA_QTH", qth_grp_->name());
		break;
	case LM_ON_AIR_TIME:
		// Interactive mode - start QSO - date/time only
		// Get current date and time in UTC
		new_record->item("QSO_DATE", timestamp.substr(0, 8));
		// Time as HHMMSS - always log seconds.
		new_record->item("TIME_ON", timestamp.substr(8));
		new_record->item("QSO_DATE_OFF", string(""));
		new_record->item("TIME_OFF", string(""));
		new_record->item("CALL", string(""));
		// otherwise leave blank so that we enter it manually later.
		new_record->item("FREQ", string(""));
		new_record->item("FREQ_RX", string(""));
		new_record->item("MODE", string(""));
		new_record->item("SUBMODE", string(""));
		new_record->item("TX_PWR", string(""));
		// initialise fields
		new_record->item("RX_PWR", string(""));
		new_record->item("RST_SENT", string(""));
		new_record->item("RST_RCVD", string(""));
		new_record->item("NAME", string(""));
		new_record->item("QTH", string(""));
		new_record->item("GRIDSQUARE", string(""));
		// Prepopulate rig, antenna, QTH and callsign
		new_record->item("STATION_CALLSIGN", callsign_grp_->name());
		new_record->item("MY_RIG", rig_grp_->name());
		new_record->item("MY_ANTENNA", antenna_grp_->name());
		new_record->item("APP_ZZA_QTH", qth_grp_->name());
		break;
	}

	// Add to book
	if (add_to_book) {
		record_num_t new_record_num = book_->append_record(new_record);
		book_->selection(book_->item_number(new_record_num));
		// Notify other views
		tabbed_forms_->update_views(nullptr, HT_INSERTED, new_record_num);
	}

	// Return created record
	return new_record;
}

// End QSO - add time off
// TODO: Can be called without current_qso_ - needs to be set by something.
void qso_manager::end_qso() {
	// get current book item number
	record_num_t item_number = book_->selection();
	record_num_t record_number = book_->record_number(item_number);
	record* qso = book_->get_record();
	// Modified by parsing and validation
	bool record_modified = false;
	// On-air logging add date/time off
	switch (qso_group_->logging_mode_) {
	case LM_ON_AIR_CAT:
	case LM_ON_AIR_COPY:
	case LM_ON_AIR_TIME:
		if (qso->item("TIME_OFF") == "") {
			// Add end date/time - current time of interactive entering
			// Get current date and time in UTC
			string timestamp = now(false, "%Y%m%d%H%M%S");
			qso->item("QSO_DATE_OFF", timestamp.substr(0, 8));
			// Time as HHMMSS - always log seconds.
			qso->item("TIME_OFF", timestamp.substr(8));
			record_modified = true;
		}
		break;
	case LM_OFF_AIR:
		book_->correct_record_position(item_number);
		record_modified = true;
		break;
	}

	// check whether record has changed - when parsed
	if (pfx_data_->parse(qso)) {
		record_modified = true;
	}

	// check whether record has changed - when validated
	if (spec_data_->validate(qso, item_number)) {
		record_modified = true;
	}

	// Upload QSO to QSL servers
	book_->upload_qso(record_number);
	// Update this view
	qso_group_->current_qso_ = nullptr;
	enable_widgets();

	// If new or changed then update the fact and let every one know
	if (record_modified) {
		book_->modified(true);
		book_->selection(item_number, HT_INSERTED);
	}

	// Update session end - if we crash before we close down, we may fail to remember session properly
	time_t today = time(nullptr);
	void* p_today = &today;
	settings_->set("Session End", p_today, sizeof(time_t));
	settings_->flush();

	// Do not automatically save when in debug mode as there may be a bug in the application corrupting the log
#ifndef _DEBUG
	if (!read_only_) {
		book_->store_data();
	}
#endif // _DEBUG
}

// Dummy QSO - only current date and time
record* qso_manager::dummy_qso() {
	record* dummy = new record;
	string timestamp = now(false, "%Y%m%d%H%M%S");
	// Get current date and time in UTC
	dummy->item("QSO_DATE", timestamp.substr(0, 8));
	// Time as HHMMSS - always log seconds.
	dummy->item("TIME_ON", timestamp.substr(8));
	dummy->item("QSO_DATE_OFF", string(""));
	dummy->item("TIME_OFF", string(""));
	dummy->item("CALL", string(""));
	// otherwise leave blank so that we enter it manually later.
	dummy->item("FREQ", string(""));
	dummy->item("FREQ_RX", string(""));
	dummy->item("MODE", string(""));
	dummy->item("SUBMODE", string(""));
	dummy->item("TX_PWR", string(""));
	// initialise fields
	dummy->item("RX_PWR", string(""));
	dummy->item("RST_SENT", string(""));
	dummy->item("RST_RCVD", string(""));
	dummy->item("NAME", string(""));
	dummy->item("QTH", string(""));
	dummy->item("GRIDSQUARE", string(""));

	return dummy;
}


//// Set font etc.
//void qso_manager::set_font(Fl_Font font, Fl_Fontsize size) {
//	font_ = font;
//	fontsize_ = size;
//	// Change the font in the editor
//	editor_->textsize(fontsize_);
//	editor_->textfont(font_);
//	// And ask it to recalculate the wrap positions
//	editor_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
//	redraw();
//}

//// Constructor for qso_manager editor
//spad_editor::spad_editor(int x, int y, int w, int h) :
//	intl_editor(x, y, w, h)
//{
//}
//
//// Destructor for qso_manager editor
//spad_editor::~spad_editor() {
//}
//
//// Handler of any event coming from the qso_manager editor
//int spad_editor::handle(int event) {
//	switch (event) {
//	case FL_FOCUS:
//	case FL_UNFOCUS:
//		// mouse going in and out of focus on this view
//		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
//		return true;
//	case FL_KEYBOARD:
//		// Keyboard event - used for keyboard navigation
//		switch (Fl::event_key()) {
//		case FL_F + 1:
//			// F1 - copy selected text to CALL field
//			qso_manager::cb_action(this, (void*)qso_manager::WRITE_CALL);
//			return true;
//		case FL_F + 2:
//			// F2 - copy selected text to NAME field
//			qso_manager::cb_action(this, (void*)qso_manager::WRITE_NAME);
//			return true;
//		case FL_F + 3:
//			// F3 - copy selected text to QTH field
//			qso_manager::cb_action(this, (void*)qso_manager::WRITE_QTH);
//			return true;
//		case FL_F + 4:
//			// F4 - copy selected text to RST_RCVD field
//			qso_manager::cb_action(this, (void*)qso_manager::WRITE_RST_RCVD);
//			return true;
//		case FL_F + 5:
//			// F5 - copy selected text to RST_SENT field
//			qso_manager::cb_action(this, (void*)qso_manager::WRITE_RST_SENT);
//			return true;
//		case FL_F + 6:
//			// F6 - copy selected text to GRIDSQUARE field
//			qso_manager::cb_action(this, (void*)qso_manager::WRITE_GRID);
//			return true;
//		case FL_F + 7:
//			// F7 - save record
//			qso_manager::cb_save(this, nullptr);
//			return true;
//		case FL_F + 8:
//			// F8 - discard record
//			qso_manager::cb_cancel(this, nullptr);
//			return true;
//		case FL_F + 9:
//			// F9 - Check worked before
//			qso_manager::cb_wkb4(this, nullptr);
//			return true;
//		case FL_F + 10:
//			// F10 - Parse callsign
//			qso_manager::cb_parse(this, nullptr);
//			return true;
//		}
//	}
//	// Not handled the event - pass up the inheritance
//	return intl_editor::handle(event);
//}

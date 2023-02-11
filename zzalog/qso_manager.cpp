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

//// item_choice - converts "/" to "\/" and "&" to "&&"
//void qso_manager::common_grp::item_choice::add(const char* text) {
//	string escaped = escape_menu(text);
//	Fl_Input_Choice::add(escaped.c_str());
//}
//
//// Strips out '\' and first '&' characters
//const char* qso_manager::common_grp::item_choice::text() {
//	const char* src = menubutton()->text();
//	int len = strlen(src) + 1;
//	char* unescaped = new char[len];
//	memset(unescaped, 0, len);
//	char* dest = unescaped;
//	while (*src) {
//		switch (*src) {
//		case '\\':
//		case '&':
//			src++;
//			break;
//		}
//		*dest++ = *src++;
//	}
//	return unescaped;
//}
//
//// constructor 
//qso_manager::common_grp::common_grp(int X, int Y, int W, int H, const char* label, stn_item_t station_item)
//	: Fl_Group(X, Y, W, H, label)
//	, choice_(nullptr)
//	, band_browser_(nullptr)
//	, item_no_(0)
//	, station_item_(station_item)
//	, my_name_("")
//{
//	switch (station_item_) {
//	case RIG:
//		my_field_ = "MY_RIG";
//		break;
//	case ANTENNA:
//		my_field_ = "MY_ANTENNA";
//		break;
//		case QTH
//	}
//	create_form(X, Y);
//	end();
//}
//
//// Destructor
//qso_manager::common_grp::~common_grp() {
//}
//
//// create the form
//void qso_manager::common_grp::create_form(int X, int Y) {
//
//	qso_manager* dash = ancestor_view<qso_manager>(this);
//
//	begin();
//
//	labelsize(FONT_SIZE);
//	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//	box(FL_BORDER_BOX);
//
//	// Row 1
//	// removed
//	// Row 2
//	// Choice to select or add new antenna or rig
//	item_choice* ch1_1 = new item_choice(C1A, Y + R2, WSMEDIT, H2);
//	ch1_1->textsize(FONT_SIZE);
//	ch1_1->callback(cb_ch_stn, nullptr);
//	ch1_1->when(FL_WHEN_CHANGED);
//	ch1_1->tooltip("Select item");
//	ch1_1->menubutton()->textfont(FONT);
//	ch1_1->menubutton()->textsize(FONT_SIZE);
//	choice_ = ch1_1;
//
//	// Row 3
//	// Add or modify this antenna or rig
//	Fl_Button* bn3_1 = new Fl_Button(C1A, Y + R3, W1A, HBUTTON, "Add");
//	bn3_1->labelsize(FONT_SIZE);
//	bn3_1->color(fl_lighter(FL_BLUE));
//	bn3_1->labelcolor(FL_YELLOW);
//	bn3_1->callback(cb_bn_add);
//	bn3_1->when(FL_WHEN_RELEASE);
//	bn3_1->tooltip("Add or modify the selected item - opens new dialog");
//
//	// Remove this antenna or rig
//	Fl_Button* bn3_2 = new Fl_Button(C1B, Y + R3, W1B, HBUTTON, "Remove");
//	bn3_2->labelsize(FONT_SIZE);
//	bn3_2->color(fl_lighter(FL_RED));
//	bn3_2->labelcolor(FL_YELLOW);
//	bn3_2->callback(cb_bn_del);
//	bn3_2->when(FL_WHEN_RELEASE);
//	bn3_2->tooltip("Delete the item");
//
//	switch (station_item_) {
//	case RIG:
//	case ANTENNA:
//	{
//		// Row 4
//		// Bands the antenna or rig was designed for 
//		Fl_Multi_Browser* mb4_1 = new Fl_Multi_Browser(C1A, Y + R4, WSMEDIT, H4, "Intended Bands");
//		mb4_1->labelsize(FONT_SIZE);
//		mb4_1->align(FL_ALIGN_TOP);
//		mb4_1->textsize(FONT_SIZE);
//		mb4_1->callback(cb_mb_bands);
//		mb4_1->tooltip("Select the bands that the antenna is intended to be operated with");
//		band_browser_ = mb4_1;
//
//		// Now we have created all the data we can populate the choice widgets
//		populate_choice();
//		populate_band();
//		break;
//	}
//	case CALLSIGN:
//	case QTH:
//	{
//		populate_choice();
//		break;
//	}
//	}
//	enable_widgets();
//
//}

// Constructor
qso_manager::cat_group::cat_group(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	cat_data_ = new cat_data;
	load_values();
	create_form(X, Y);
	enable_widgets();
}

// DEstructor
qso_manager::cat_group::~cat_group() {}

// Get initial data from settings
void qso_manager::cat_group::load_values() {
	char* temp;
	Fl_Preferences cat_settings(settings_, "CAT");
	// Get the CAT interface parameters
	cat_settings.get("Polling Interval", cat_data_->fast_poll_interval, FAST_RIG_DEF);
	cat_settings.get("Slow Polling Interval", cat_data_->slow_poll_interval, SLOW_RIG_DEF);

	// Hamlib settings
	Fl_Preferences hamlib_settings(cat_settings, "Hamlib");
	// Get the hamlib settings: Mfr/Model, serial port and baudrate
	hamlib_settings.get("Manufacturer", temp, "Hamlib");
	cat_data_->hamlib_params.mfr = temp;
	free(temp);
	hamlib_settings.get("Rig Model", temp, "Dummy");
	cat_data_->hamlib_params.model = temp;
	free(temp);
	hamlib_settings.get("Port", temp, "COM6");
	cat_data_->hamlib_params.port_name = temp;
	free(temp);
	// Cannot always use a bool directly in settings.get()
	int all = false;
	hamlib_settings.get("All Ports", all, false);
	cat_data_->all_ports = all;
	hamlib_settings.get("Baud Rate", temp, "9600");
	cat_data_->hamlib_params.baud_rate = temp;
	free(temp);
	hamlib_settings.get("Override Rig Caps", (int&)cat_data_->hamlib_params.override_caps, false);
}

// Create CAT control widgets
void qso_manager::cat_group::create_form(int X, int Y) {

	begin();

	int max_w = 0;
	int max_h = 0;

	// CAT control group
	label("CAT Configuration");
	labelfont(FONT);
	labelsize(FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	int curr_x = X + GAP + WLABEL;
	int curr_y = Y + HTEXT;

	// Choice - Select the rig model (Manufacturer/Model)
	Fl_Choice* ch_model_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Rig");
	ch_model_->align(FL_ALIGN_LEFT);
	ch_model_->labelsize(FONT_SIZE);
	ch_model_->textsize(FONT_SIZE);
	ch_model_->tooltip("Select the model - for Hamlib");
	ch_model_->callback(cb_ch_model, nullptr);
	rig_model_choice_ = ch_model_;

	// Populate the manufacturere and model choice widgets
	populate_model_choice();

	// Hamlib control grp
	// RIG=====v
	// PORTv  ALL*
	// BAUDv  OVR*
	curr_y += ch_model_->h() + GAP;
	serial_grp_ = new Fl_Group(X + GAP, curr_y, 10, 10);
	serial_grp_->labelsize(FONT_SIZE);
	serial_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	serial_grp_->box(FL_NO_BOX);

	// Choice port name - serial
	curr_x = serial_grp_->x() + WLABEL;
	curr_y = serial_grp_->y();
	Fl_Choice* ch_port = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Port");
	ch_port->align(FL_ALIGN_LEFT);
	ch_port->labelsize(FONT_SIZE);
	ch_port->textsize(FONT_SIZE);
	ch_port->callback(cb_ch_port, nullptr);
	ch_port->tooltip("Select the comms port to use");
	port_if_choice_ = ch_port;

	// Use all ports
	int save_x = curr_x;
	curr_x += ch_port->w() + GAP;
	Fl_Check_Button* bn_useall = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All ports");
	bn_useall->align(FL_ALIGN_RIGHT);
	bn_useall->labelfont(FONT);
	bn_useall->labelsize(FONT_SIZE);
	bn_useall->tooltip("Select all existing ports, not just those available");
	bn_useall->callback(cb_bn_all, &cat_data_->all_ports);
	show_all_ports_ = bn_useall;
	populate_port_choice();

	// Baud rate input 
	curr_x += bn_useall->w() + WLABEL + GAP;
	int max_x = curr_x;
	curr_x = save_x;
	curr_y += ch_port->h();
	int max_y = curr_y;
	Fl_Choice* ch_baudrate = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Baud rate");
	ch_baudrate->align(FL_ALIGN_LEFT);
	ch_baudrate->labelsize(FONT_SIZE);
	ch_baudrate->textsize(FONT_SIZE);
	ch_baudrate->tooltip("Enter baud rate");
	ch_baudrate->callback(cb_ch_baud, nullptr);
	baud_rate_choice_ = ch_baudrate;

	// Override capabilities (as coded in hamlib)
	curr_x += ch_baudrate->w() + GAP;
	Fl_Check_Button* bn_override = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Override\ncapability");
	bn_override->align(FL_ALIGN_RIGHT);
	bn_override->labelsize(FONT_SIZE);
	bn_override->tooltip("Allow full baud rate selection");
	bn_override->callback(cb_ch_over, nullptr);
	override_check_ = bn_override;

	populate_baud_choice();

	curr_x += bn_override->w() + WLABEL + GAP;
	max_x = max(max_x, curr_x);
	curr_y += ch_baudrate->h() + GAP;
	max_y = max(max_y, curr_y);
	serial_grp_->resizable(nullptr);
	serial_grp_->size(max_x - serial_grp_->x(), max_y - serial_grp_->y());

	serial_grp_->end();

	network_grp_ = new Fl_Group(serial_grp_->x(), serial_grp_->y(), 10, 10);
	network_grp_->labelsize(FONT_SIZE);
	network_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	network_grp_->box(FL_NO_BOX);

	// Input port name - network
	curr_x = network_grp_->x() + WLABEL;
	curr_y = network_grp_->y();
	Fl_Input* ip_port = new Fl_Input(curr_x, curr_y, WSMEDIT, HTEXT, "Port");
	ip_port->align(FL_ALIGN_LEFT);
	ip_port->labelsize(FONT_SIZE);
	ip_port->textsize(FONT_SIZE);
	ip_port->callback(cb_ip_port, nullptr);
	ip_port->tooltip("Enter the network/USB port to use");
	ip_port->value(cat_data_->hamlib_params.port_name.c_str());
	port_if_input_ = ip_port;

	curr_x += ip_port->w() + GAP;
	curr_y += ip_port->h() + GAP;
	network_grp_->resizable(nullptr);
	network_grp_->size(curr_x - network_grp_->x(), curr_y - network_grp_->y());

	network_grp_->end();

	max_y = max(serial_grp_->y() + serial_grp_->h(), network_grp_->y() + network_grp_->h());
	max_x = max(serial_grp_->x() + serial_grp_->w(), network_grp_->x() + network_grp_->w());
	curr_y = max_y + GAP;
	curr_x = save_x;

	// Connected status
	bn_connect_ = new Fl_Button(curr_x, curr_y, WBUTTON * 2, HBUTTON, "Connect...");
	bn_connect_->labelfont(FONT);
	bn_connect_->labelsize(FONT_SIZE);
	bn_connect_->color(FL_YELLOW);
	bn_connect_->tooltip("Select to attempt to connect rig");
	bn_connect_->callback(cb_bn_connect, nullptr);

	// Poll period group;
// <>FAST
// <>SLOW
	curr_x += bn_connect_->w() + GAP;
	max_x = max(max_x, curr_x);
	curr_x = X + GAP;
	curr_y += bn_connect_->h() + GAP;
	Fl_Group* poll_grp = new Fl_Group(curr_x, curr_y, 10, 10, "Polling interval (s)");
	poll_grp->labelsize(FONT_SIZE);
	poll_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	poll_grp->box(FL_NO_BOX);

	curr_x = save_x;
	curr_y += HTEXT;
	// Spinner to select fast polling rate (i.e. when still connected)
	ctr_pollfast_ = new Fl_Spinner(curr_x, curr_y, WSMEDIT, HTEXT, "Conn'd");
	ctr_pollfast_->align(FL_ALIGN_LEFT);
	ctr_pollfast_->labelsize(FONT_SIZE);
	ctr_pollfast_->textsize(FONT_SIZE);
	ctr_pollfast_->tooltip("Select the polling period for fast polling");
	ctr_pollfast_->type(FL_FLOAT_INPUT);
	ctr_pollfast_->minimum(FAST_RIG_MIN);
	ctr_pollfast_->maximum(FAST_RIG_MAX);
	ctr_pollfast_->step(0.01);
	ctr_pollfast_->value(cat_data_->fast_poll_interval);
	ctr_pollfast_->callback(cb_ctr_pollfast);
	ctr_pollfast_->when(FL_WHEN_CHANGED);
	max_x = max(max_x, curr_x + ctr_pollfast_->w() + GAP);
	curr_y += ctr_pollfast_->h();

	// Spinner to select slow polling rate (i.e. after disconnection to avoid excessive errors)
	Fl_Spinner* ctr_pollslow_ = new Fl_Spinner(curr_x, curr_y, WSMEDIT, HTEXT, "Disconn'd");
	ctr_pollslow_->align(FL_ALIGN_LEFT);
	ctr_pollslow_->labelsize(FONT_SIZE);
	ctr_pollslow_->textsize(FONT_SIZE);
	ctr_pollslow_->tooltip("Select the polling period for slow polling");
	ctr_pollslow_->type(FL_FLOAT_INPUT);
	ctr_pollslow_->minimum(SLOW_RIG_MIN);
	ctr_pollslow_->maximum(SLOW_RIG_MAX);
	ctr_pollslow_->step(0.5);
	ctr_pollslow_->value(cat_data_->slow_poll_interval);
	ctr_pollslow_->callback(cb_ctr_pollslow);
	ctr_pollslow_->when(FL_WHEN_CHANGED);
	max_x = max(max_x, curr_x + ctr_pollslow_->w() + GAP);
	curr_y += ctr_pollslow_->h() + GAP;

	poll_grp->resizable(nullptr);
	poll_grp->size(max_x - poll_grp->x(), curr_y - poll_grp->y());
	poll_grp->end();

	max_y = poll_grp->y() + poll_grp->h();

	// Display hamlib ot flrig settings as selected
	enable_widgets();

	resizable(nullptr);
	size(max_x - x(), max_y - y());
	end();
}

// Save values in settings
void qso_manager::cat_group::save_values() {
	qso_manager* dash = ancestor_view<qso_manager>(this);

	Fl_Preferences cat_settings(settings_, "CAT");

	// Polling interval
	cat_settings.set("Polling Interval", cat_data_->fast_poll_interval);
	cat_settings.set("Slow Polling Interval", cat_data_->slow_poll_interval);
	// Read all the groups
	// Hamlib settings
	Fl_Preferences hamlib_settings(cat_settings, "Hamlib");
	hamlib_settings.set("Manufacturer", cat_data_->hamlib_params.mfr.c_str());
	hamlib_settings.set("Rig Model", cat_data_->hamlib_params.model.c_str());
	hamlib_settings.set("Port", cat_data_->hamlib_params.port_name.c_str());
	hamlib_settings.set("Baud Rate", cat_data_->hamlib_params.baud_rate.c_str());
	hamlib_settings.set("Override Rig Caps", cat_data_->hamlib_params.override_caps);
	hamlib_settings.set("All Ports", cat_data_->all_ports);
}

// Enable CAT Connection widgets
void qso_manager::cat_group::enable_widgets() {

	// CAT control widgets
	switch (cat_data_->hamlib_params.port_type) {
	case RIG_PORT_SERIAL:
		serial_grp_->activate();
		serial_grp_->show();
		network_grp_->deactivate();
		network_grp_->hide();
		break;
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		serial_grp_->deactivate();
		serial_grp_->hide();
		network_grp_->activate();
		network_grp_->show();
		break;
	default:
		serial_grp_->deactivate();
		serial_grp_->hide();
		network_grp_->deactivate();
		network_grp_->hide();
		break;
	}

	// Connect button
	if (rig_if_) {
		bn_connect_->color(FL_GREEN);
		bn_connect_->label("Connected");
	}
	else {
		switch (cat_data_->hamlib_params.port_type) {
		case RIG_PORT_NONE:
			bn_connect_->color(FL_BACKGROUND_COLOR);
			bn_connect_->label("No CAT connection");
			break;
		default:
			if (wait_connect_) {
				bn_connect_->color(FL_YELLOW);
				bn_connect_->label("... Connect");
			}
			else {
				bn_connect_->color(FL_RED);
				bn_connect_->label("Disconnected");
			}
			break;
		}
	}
	//// Set values in CAT widgets
	//// Sledge hammer approach
	//populate_baud_choice();
	//populate_model_choice();
	//populate_port_choice();
}

// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
void qso_manager::cat_group::populate_model_choice() {
	Fl_Choice* ch = (Fl_Choice*)rig_model_choice_;
	// Get hamlib Model number and populate control with all model names
	ch->clear();
	set<string> rig_list;
	map<string, rig_model_t> rig_ids;
	rig_list.clear();
	rig_ids.clear();
	// For each possible rig ids in hamlib
	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
	for (rig_model_t i = 1; i < max_rig_num; i += 1) {
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
			char* temp = new char[256];
			// The '/' ensures all rigs from same manufacturer are in a sub-menu to Icom
			string mfg = escape_menu(capabilities->mfg_name);
			string model = escape_menu(capabilities->model_name);
			snprintf(temp, 256, "%s/%s%s", mfg.c_str(), model.c_str(), status);
			rig_list.insert(temp);
			rig_ids[temp] = capabilities->rig_model;
			if (string(capabilities->mfg_name) == cat_data_->hamlib_params.mfr &&
				string(capabilities->model_name) == cat_data_->hamlib_params.model) {
				cat_data_->hamlib_params.model_id = capabilities->rig_model;
				cat_data_->hamlib_params.port_type = capabilities->port_type;
			}
		}
	}
	// Add the rigs in alphabetical order to the choice widget, set widget's value to intended
	for (auto ix = rig_list.begin(); ix != rig_list.end(); ix++) {
		string name = *ix;
		rig_model_t id = rig_ids.at(name);
		int pos = ch->add(name.c_str(), 0, nullptr, (void*)id);
		if (id == cat_data_->hamlib_params.model_id) {
			ch->value(pos);
		}
	}
	bool found = false;
}

// Populate the choice with the available ports
void qso_manager::cat_group::populate_port_choice() {
	if (cat_data_->hamlib_params.port_type == RIG_PORT_SERIAL) {
		Fl_Choice* ch = (Fl_Choice*)port_if_choice_;
		ch->clear();
		ch->add("NONE");
		ch->value(0);
		int num_ports = 1;
		string* existing_ports = new string[1];
		serial serial;
		// Get the list of all ports or available (not in use) ports
		while (!serial.available_ports(num_ports, existing_ports, cat_data_->all_ports, num_ports)) {
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
			if (strcmp(port, cat_data_->hamlib_params.port_name.c_str()) == 0) {
				ch->value(i);
			}
		}
	}
}

// Populate the baud rate choice menu
void qso_manager::cat_group::populate_baud_choice() {
	if (cat_data_->hamlib_params.port_type == RIG_PORT_SERIAL) {
		Fl_Choice* ch = (Fl_Choice*)baud_rate_choice_;
		ch->clear();
		// Override rig's capabilities?
		bool override_caps = cat_data_->hamlib_params.override_caps;
		Fl_Button* bn = (Fl_Button*)override_check_;
		bn->value(override_caps);

		// Get the baud-rates supported by the rig
		const rig_caps* caps = rig_get_caps(cat_data_->hamlib_params.model_id);
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
				if (to_string(rate) == cat_data_->hamlib_params.baud_rate) {
					ch->value(index);
					index++;
				}
			}
		}
	}
}


// Model input choice selected
// v is not used
void qso_manager::cat_group::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	cat_group* that = ancestor_view<cat_group>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	const Fl_Menu_Item* item = ch->mvalue();
	rig_model_t id = (long)item->user_data();
	const char* label = ch->text();
	const rig_caps* capabilities = rig_get_caps(id);
	if (capabilities != nullptr) {
		info->model = capabilities->model_name;
		info->mfr = capabilities->mfg_name;
		info->model_id = id;
		info->port_type = capabilities->port_type;
	}
	else {
		char message[128];
		snprintf(message, 128, "DASH: Error reading hamlib details selecting %s", label);
		status_->misc_status(ST_ERROR, message);
	}
	that->populate_baud_choice();
	that->enable_widgets();
}

// Callback selecting port
// v is unused
void qso_manager::cat_group::cb_ch_port(Fl_Widget* w, void* v) {
	cat_group* that = ancestor_view<cat_group>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->port_name);
}

// Callback entering port
// v is unused
void qso_manager::cat_group::cb_ip_port(Fl_Widget* w, void* v) {
	cat_group* that = ancestor_view<cat_group>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_value<Fl_Input, string>(w, (void*)&info->port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_manager::cat_group::cb_ch_baud(Fl_Widget* w, void* v) {
	cat_group* that = ancestor_view<cat_group>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->baud_rate);
}

// Override rig capabilities selected - repopulate the baud choice
// v is uused
void qso_manager::cat_group::cb_ch_over(Fl_Widget* w, void* v) {
	cat_group* that = ancestor_view<cat_group>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_value<Fl_Check_Button, bool>(w, (void*)&info->override_caps);
	that->populate_baud_choice();
}

// Select "display all ports" in port choice
// v is a pointer to the all ports flag
void qso_manager::cat_group::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	cat_group* that = ancestor_view<cat_group>(w);
	that->populate_port_choice();
}

// Changed the fast polling interval
// v is not used
void qso_manager::cat_group::cb_ctr_pollfast(Fl_Widget* w, void* v) {
	// Get the warning level
	cat_group* that = ancestor_view<cat_group>(w);
	cb_value<Fl_Spinner, double>(w, &that->cat_data_->fast_poll_interval);
}

// Changed the fast polling interval
// v is not used
void qso_manager::cat_group::cb_ctr_pollslow(Fl_Widget* w, void* v) {
	// Get the warning level
	cat_group* that = ancestor_view<cat_group>(w);
	cb_value<Fl_Spinner, double>(w, &that->cat_data_->slow_poll_interval);
}

// Pressed the connect button - this is also called from toolbar
// v is not used - allow for w to be qso_manager_
void qso_manager::cat_group::cb_bn_connect(Fl_Widget* w, void* v) {
	cat_group* that = ancestor_view<cat_group>(w);
	qso_manager* mgr;
	if (that == nullptr) {
		mgr = ancestor_view<qso_manager>(w);
		that = mgr->cat_group_;
	}
	else {
		mgr = (qso_manager*)that->parent();
	}
	that->save_values();
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
	mgr->update_rig();
}

//switch (station_item_) {
//case CALLSIGN: {
//	int num_entries;
//	my_settings_->get("Number Callsigns", num_entries, 0);
//	for (int i = 1; i <= num_entries; i++) {
//		char name[10];
//		snprintf(name, 10, "Call%d", i);
//		my_settings_->get(name, text, "");
//		all_items_.push_back(string(text));
//		free(text);
//	}
//}
//}
//// Get default value
//my_settings_->get("Default", text, "");
//my_name_ = text;
//string bands;
//int num_items = my_settings_->groups();
//// For each item in the Antenna or rig settings
//for (int i = 0; i < num_items; i++) {
//	// Get that item's settings
//	string name = my_settings_->group(i);
//	if (name.length()) {
//		Fl_Preferences item_settings(*my_settings_, name.c_str());
//		char* temp;
//		string true_name = unescape_hex(name);
//		all_items_.push_back(true_name);
//		item_data& info = item_info_[true_name];
//		switch (station_item_) {
//		case RIG:
//			// Get the CAT interface parameters
//			item_settings.get("Polling Interval", info.rig_data.fast_poll_interval, FAST_RIG_DEF);
//			item_settings.get("Slow Polling Interval", info.rig_data.slow_poll_interval, SLOW_RIG_DEF);
//			{
//				// Hamlib settings
//				Fl_Preferences hamlib_settings(item_settings, "Hamlib");
//				// Get the hamlib settings: Mfr/Model, serial port and baudrate
//				hamlib_settings.get("Manufacturer", temp, "Hamlib");
//				info.rig_data.hamlib_params.mfr = temp;
//				free(temp);
//				hamlib_settings.get("Rig Model", temp, "Dummy");
//				info.rig_data.hamlib_params.model = temp;
//				free(temp);
//				hamlib_settings.get("Port", temp, "COM6");
//				info.rig_data.hamlib_params.port_name = temp;
//				free(temp);
//				hamlib_settings.get("Baud Rate", temp, "9600");
//				info.rig_data.hamlib_params.baud_rate = temp;
//				free(temp);
//				hamlib_settings.get("Override Rig Caps", (int&)info.rig_data.hamlib_params.override_caps, false);
//
//				// Alarm settings
//				Fl_Preferences alarm_settings(item_settings, "Alarms");
//				// SWR Settings - warning and error levels
//				alarm_settings.get("SWR Warning Level", info.rig_data.alarms.swr_warning, 1.5);
//				alarm_settings.get("SWR Error Level", info.rig_data.alarms.swr_error, 2.0);
//				// Power settings - warning levels
//				// TODO: Add mode specific settings
//				alarm_settings.get("Power Warning Level", info.rig_data.alarms.power_warning, 95);
//				// Vdd settings - error if above or below 10% of nominal (13.8V)
//				alarm_settings.get("Voltage Minimum Level", info.rig_data.alarms.voltage_minimum, (float)(13.8 * 0.85));
//				alarm_settings.get("Voltage Maximum Level", info.rig_data.alarms.voltage_maximum, (float)(13.8 * 1.15));
//			}
//
//			// Carry onto next - no break
//
//		case ANTENNA:
//		{
//			// Antenna has just active flag and bands it is intended for
//			// Get the intended bands
//			char* temp;
//			item_settings.get("Intended Bands", temp, "");
//			bands = temp;
//			free(temp);
//			split_line(bands, info.intended_bands, ';');
//			break;
//			//
//		}
//		case CALLSIGN:
//		{
//			info.intended_bands.clear();
//			break;
//		}
//		case QTH:
//		{
//			info.intended_bands.clear();
//		}
//		}
//	}
//	else {
//		// Default to no handler - hopefully can ignore the rest
//	}
//}


//// Save values in settings
//void qso_manager::common_grp::save_values() {
//	qso_manager* dash = ancestor_view<qso_manager>(this);
//
//	// Clear to remove deleted entries
//	switch (station_item_) {
//	case QTH:
//		break;
//	default:
//		my_settings_->clear();
//		break;
//	}
//
//	// Set default value
//	my_settings_->set("Default", my_name_.c_str());
//	int index = 0;
//	// For each item
//	for (auto it = item_info_.begin(); it != item_info_.end(); it++) {
//		string name = (*it).first;
//		item_data& info = (*it).second;
//
//		switch (station_item_) {
//		case RIG: {
//			Fl_Preferences item_settings(my_settings_, name.c_str());
//			// set the intended bands
//			string bands = "";
//			// Store all the bands intended to be used with this rig/antenna
//			for (auto itb = info.intended_bands.begin(); itb != info.intended_bands.end(); itb++) {
//				if ((*itb).length()) {
//					bands += *itb + ';';
//				}
//			}
//			item_settings.set("Intended Bands", bands.c_str());
//			item_settings.set("Polling Interval", info.rig_data.fast_poll_interval);
//			item_settings.set("Slow Polling Interval", info.rig_data.slow_poll_interval);
//			// Read all the groups
//			// Hamlib settings
//			Fl_Preferences hamlib_settings(item_settings, "Hamlib");
//			hamlib_settings.set("Manufacturer", info.rig_data.hamlib_params.mfr.c_str());
//			hamlib_settings.set("Rig Model", info.rig_data.hamlib_params.model.c_str());
//			hamlib_settings.set("Port", info.rig_data.hamlib_params.port_name.c_str());
//			hamlib_settings.set("Baud Rate", info.rig_data.hamlib_params.baud_rate.c_str());
//			hamlib_settings.set("Override Rig Caps", info.rig_data.hamlib_params.override_caps);
//			// Alarm settings
//			Fl_Preferences alarm_settings(item_settings, "Alarms");
//			// SWR Settings
//			alarm_settings.set("SWR Warning Level", info.rig_data.alarms.swr_warning);
//			alarm_settings.set("SWR Error Level", info.rig_data.alarms.swr_error);
//			// Power settings
//			alarm_settings.set("Power Warning Level", info.rig_data.alarms.power_warning);
//			// Vdd settings
//			alarm_settings.set("Voltage Minimum Level", info.rig_data.alarms.voltage_minimum);
//			alarm_settings.set("Voltage Maximum Level", info.rig_data.alarms.voltage_maximum);
//
//			break;
//		}
//		case ANTENNA: {
//			Fl_Preferences item_settings(my_settings_, name.c_str());
//			string bands = "";
//			// Store all the bands intended to be used with this rig/antenna
//			for (auto itb = info.intended_bands.begin(); itb != info.intended_bands.end(); itb++) {
//				if ((*itb).length()) {
//					bands += *itb + ';';
//				}
//			}
//			item_settings.set("Intended Bands", bands.c_str());
//			break;
//		}
//
//		case QTH:
//		{
//			// No additional data - in QTH case save by separate dialog
//			break;
//		}
//		}
//	}
//	switch (station_item_) {
//	case CALLSIGN: {
//		int index = 0;
//		my_settings_->set("Number Callsigns", (int)all_items_.size());
//		for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
//			index++;
//			char id[10];
//			snprintf(id, 10, "Call%d", index);
//			my_settings_->set(id, (*it).c_str());
//		}
//		break;
//	}
//	}
//}

//// Populate the item selector
//void qso_manager::common_grp::populate_choice() {
//	item_choice* w = (item_choice*)choice_;
//	w->clear();
//	int index = 0;
//	bool sel_found = false;
//	// For each item
//	for (auto it = all_items_.begin(); it != all_items_.end(); it++) {
//		// Chack if the antenna or rig is in active use
//		item_data& info = item_info_[(*it)];
//		// If item is currently active or we want to display both active and inactive items
//		// Add the item name to the choice
//		if ((*it).length()) {
//			w->add((*it).c_str());
//			if (*it == my_name_) {
//				// If it's the current selection, show it as such
//				w->value((*it).c_str());
//				item_no_ = index;
//				sel_found = true;
//			}
//			index++;
//		}
//	}
//	if (!sel_found && my_name_.length()) {
//		// Selected item not found. Add it to the various lists
//		w->add(my_name_.c_str());
//		w->value(my_name_.c_str());
//	}
//	w->textsize(FONT_SIZE);
//	w->textfont(FONT);
//}
//
//// Populate the band selection widget
//void qso_manager::common_grp::populate_band() {
//	// Get pointers to the widget to be populated and the top-level dialog
//	Fl_Multi_Browser* mb = (Fl_Multi_Browser*)band_browser_;
//	qso_manager* dash = ancestor_view<qso_manager>(mb);
//	mb->clear();
//	// Add all the possible bands (according to latest ADIF specification)
//	for (auto it = dash->ordered_bands_.begin(); it != dash->ordered_bands_.end(); it++) {
//		mb->add((*it).c_str());
//	}
//	item_data& info = item_info_[my_name_];
//	auto it = info.intended_bands.begin();
//	int i = 0;
//	// Select all the bands the antenna is meant to be used for
//	while (it != info.intended_bands.end() && i != dash->ordered_bands_.size()) {
//		// Note text(i) can be null if it hasn't been set
//		if (mb->text(i) && *it == mb->text(i)) {
//			mb->select(i);
//			it++;
//		}
//		i++;
//	}
//}
//
//// Enable widgets
//void qso_manager::common_grp::enable_widgets() {
//	// Set value and style of settings ouput widget
//	qso_manager* dash = ancestor_view<qso_manager>(this);
//	// Set values into choice
//	if (!dash->items_changed_) {
//		item_choice* ch = (item_choice*)choice_;
//		ch->value(my_name_.c_str());
//	}
//}

//// button callback - add
//// Add the text value of the choice to the list of items - new name is typed into the choice
//void qso_manager::common_grp::cb_bn_add(Fl_Widget* w, void* v) {
//	common_grp* that = ancestor_view<common_grp>(w);
//	qso_manager* dash = ancestor_view<qso_manager>(w);
//	dash->items_changed_ = true;
//	// Open the QTH dialog
//	button_t result = BN_OK;
//	switch (that->station_item_) {
//	case QTH:
//	{
//		qth_dialog* dialog = new qth_dialog(that->my_name_);
//		result = dialog->display();
//	}
//	}
//	switch (result) {
//	case BN_OK:
//	{
//		// Set it active (and add it if it's not there)
//		that->all_items_.push_back(that->my_name_);
//
//		that->enable_widgets();
//		that->populate_choice();
//		that->save_values();
//		dash->qso_group_->update_station_choices(that->station_item_);
//		tabbed_forms_->update_views(nullptr, HT_LOCATION, -1);
//
//		that->redraw();
//		break;
//	}
//	case BN_CANCEL:
//	{
//		break;
//	}
//	}
//
//}
//
//// button callback - delete
//void qso_manager::common_grp::cb_bn_del(Fl_Widget* w, void* v) {
//	common_grp* that = ancestor_view<common_grp>(w);
//	qso_manager* dash = ancestor_view<qso_manager>(w);
//	// Get the selected item name
//	string item = ((item_choice*)that->choice_)->text();
//	// Remove the item
//	that->all_items_.remove(item);
//	// TODO: For now display the first element
//	that->my_name_ = *(that->all_items_.begin());
//	// Delete the item
//	that->populate_choice();
//	that->save_values();
//	dash->qso_group_->update_station_choices(that->station_item_);
//	that->redraw();
//}
//
//// choice callback
//// v is unused
//void qso_manager::common_grp::cb_ch_stn(Fl_Widget* w, void* v) {
//	// Input_Choice is a descendant of common_grp
//	item_choice* ch = ancestor_view<item_choice>(w); 
//	common_grp* that = ancestor_view<common_grp>(ch);
//	qso_manager* dash = ancestor_view<qso_manager>(that);
//	if (ch->menubutton()->changed()) {
//		// Get the new item from the menu - note menu has escaped '/' and '&'
//		that->item_no_ = ch->menubutton()->value();
//		that->my_name_ = ch->text();
//		item_data& info = that->item_info_[that->my_name_];
//		// Update the shared choice value to the unescaped version
//		ch->value(that->my_name_.c_str());
//		switch (that->station_item_) {
//		case RIG:
//		case ANTENNA:
//			that->populate_band();
//			break;
//		}
//		that->save_values();
//		dash->qso_group_->update_station_choices(that->station_item_);
//	}
//	else {
//		// A new item is being typed in the input field - use ADD button to process it
//		// New item has unescaped '/' and '&' characters
//		that->item_no_ = -1;
//		that->my_name_ = ch->value();
//	}
//	dash->items_changed_ = true;
//}
//
//// Multi-browser callback
//// v is usused
//void qso_manager::common_grp::cb_mb_bands(Fl_Widget* w, void* v) {
//	Fl_Multi_Browser* mb = ancestor_view<Fl_Multi_Browser>(w);
//	common_grp* that = ancestor_view<common_grp>(mb);
//	// Get the list of bands the selected antenna or rig is meant for
//	vector<string>& bands = that->item_info_[that->my_name_].intended_bands;
//	// Clear the list and add the bands currently selected in the browser
//	bands.clear();
//	for (int i = 0; i < mb->size(); i++) {
//		if (mb->selected(i)) {
//			bands.push_back(mb->text(i));
//		}
//	}
//	that->redraw();
//	that->save_values();
//	((qso_manager*)that->parent())->enable_widgets();
//}
//
//// Item choice call back
//// v is value selected
//void qso_manager::common_grp::cb_ch_item(Fl_Widget* w, void* v) {
//	item_choice* ipch = ancestor_view<item_choice>(w);
//	cb_value<item_choice, string>(ipch, v);
//	common_grp* that = ancestor_view<common_grp>(w);
//	that->save_values();
//	that->populate_band();
//}
//
//string& qso_manager::common_grp::name() {
//	return my_name_;
//}
//
//qso_manager::item_data& qso_manager::common_grp::info() {
//	return item_info_[my_name_];
//}
//
//
//void qso_manager::common_grp::update_settings_name() {
//	my_settings_->set("Default", my_name_.c_str());
//}
//
//// Update name and reset choice values
//void qso_manager::common_grp::update_choice(string name) {
//	if (name.length()) {
//		my_name_ = name;
//		populate_choice();
//	}
//}

// qso_group_
qso_manager::qso_group::qso_group(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
	, current_qso_(nullptr)
	, original_qso_(nullptr)
	, logging_mode_(LM_OFF_AIR)
	, contest_id_("")
	, exch_fmt_ix_(0)
	, exch_fmt_id_("")
	, max_ef_index_(0)
	, field_mode_(NO_CONTEST)
	, logging_state_(QSO_INACTIVE)
	, previous_locator_("")
	, previous_qth_("")
{
	for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
		ip_field_[ix] = nullptr;
		ch_field_[ix] = nullptr;
	}
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

	// Read contest details from most recent QSO
	if (book_->size()) {
		record* prev_record = book_->get_record(book_->size() - 1, false);
		string prev_contest = prev_record->item("CONTEST_ID");
		char message[100];
		if (field_mode_ != NO_CONTEST) {
			if (prev_contest != contest_id_) {
				snprintf(message, 100, "DASH: Contest ID in log (%s) differs from settings (%s)", prev_contest.c_str(), contest_id_.c_str());
				status_->misc_status(ST_WARNING, message);
			}
			else {
				// Get serial number from log
				string serno = prev_record->item("STX");
				if (serno.length() > 0) {
					int sernum = stoi(serno);
					if (sernum > serial_num_) {
						snprintf(message, 100, "DASH: Contest serial in log (%d) greater than settings (%d) - using log", sernum, serial_num_);
						status_->misc_status(ST_WARNING, message);
						serial_num_ = sernum;
					}
					else {
						snprintf(message, 100, "DASH: Contest serial in log (%d) less than settings (%d) - using settings", sernum, serial_num_);
						status_->misc_status(ST_NOTE, message);
					}
				}
			}
		}
	}
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

	curr_x += bn_inc_serno_->w();

	// Transmitted contest exchange
	op_serno_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Serial");
	op_serno_->align(FL_ALIGN_TOP);
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
	curr_x = left;

	// Fixed fields
	// N rows of NUMBER_PER_ROW
	const int NUMBER_PER_ROW = 3;
	const int WCHOICE = WBUTTON * 3 / 2;
	const int WINPUT = WBUTTON * 7 / 4;
	for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
		if (ix >= NUMBER_FIXED) {
			ch_field_[ix] = new field_choice(curr_x, curr_y, WCHOICE, HBUTTON);
			ch_field_[ix]->textfont(FONT);
			ch_field_[ix]->textsize(FONT_SIZE);
			ch_field_[ix]->align(FL_ALIGN_RIGHT);
			ch_field_[ix]->tooltip("Specify the field to provide");
			ch_field_[ix]->callback(cb_ch_field, (void*)ix);
			ch_field_[ix]->set_dataset("Fields");
		}
		curr_x += WCHOICE;
		ip_field_[ix] = new field_input(curr_x, curr_y, WINPUT, HBUTTON);
		ip_field_[ix]->align(FL_ALIGN_LEFT);
		ip_field_[ix]->tooltip("Enter required value to log");
		ip_field_[ix]->callback(cb_ip_field, (void*)ix);
		ip_field_[ix]->input()->when(FL_WHEN_ENTER_KEY);
		if (ix < NUMBER_FIXED) {
			ip_field_[ix]->field_name(fixed_names_[ix].c_str());
			field_ips_[fixed_names_[ix]] = ip_field_[ix];
			ip_field_[ix]->label(fixed_names_[ix].c_str());
		}
		curr_x += WINPUT + GAP;
		if (ix % NUMBER_PER_ROW == (NUMBER_PER_ROW - 1)) {
			max_w = max(max_w, curr_x - x());
			curr_x = left;
			curr_y += HBUTTON;
		}
	}


	max_w = max(max_w, curr_x);

	// nOtes input
	curr_x = left + WCHOICE;
	curr_y += HBUTTON;
	
	ip_notes_ = new intl_input(curr_x, curr_y, max_w - curr_x, HBUTTON, "NOTES");
	ip_notes_->labelfont(FONT);
	ip_notes_->labelsize(FONT_SIZE);
	ip_notes_->textfont(FONT);
	ip_notes_->textsize(FONT_SIZE);
	ip_notes_->callback(cb_ip_notes, nullptr);
	ip_notes_->tooltip("Add any notes for the QSO");

	curr_x = left + WLABEL;
	curr_y += HBUTTON;


	// QSO start/stop
	curr_x = left;
	curr_y += GAP;

	bn_activate_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Activate");
	bn_activate_->callback(cb_activate);
	bn_activate_->color(FL_CYAN);
	bn_activate_->tooltip("Pre-load QSO fields based on logging mode");
	curr_x += bn_activate_->w();

	bn_start_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Start");
	bn_start_->callback(cb_start);
	bn_start_->color(FL_YELLOW);
	bn_start_->tooltip("Start the QSO, after saving, and/or activating");
	curr_x += bn_start_->w();

	bn_save_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Save");
	bn_save_->callback(cb_save);
	bn_save_->color(FL_GREEN);
	bn_save_->tooltip("Log the QSO, activate a new one");
	curr_x += bn_save_->w();

	bn_edit_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Edit");
	bn_edit_->callback(cb_edit);
	bn_edit_->color(FL_MAGENTA);
	bn_edit_->tooltip("Edit the selected QSO");
	curr_x += bn_edit_->w();

	bn_cancel_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Cancel");
	bn_cancel_->callback(cb_cancel);
	bn_cancel_->color(fl_lighter(FL_RED));
	bn_cancel_->tooltip("Cancel the current QSO entry or edit");
	curr_x += bn_edit_->w() + GAP;

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

	curr_x += bn_parse_->w() + GAP;

	bn_edit_qth_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Edit QTH");
	bn_edit_qth_->callback(cb_bn_edit_qth);
	bn_edit_qth_->tooltip("Edit the details of the QTH macro");

	curr_x += bn_edit_qth_->w();

	max_w = max(max_w, curr_x);
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
	switch (logging_state_) {
	case QSO_INACTIVE:
		bn_activate_->activate();
		bn_start_->activate();
		bn_save_->activate();
		bn_cancel_->deactivate();
		bn_edit_->activate();
		bn_edit_qth_->deactivate();
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->deactivate();
		break;
	case QSO_EDIT:
		bn_activate_->deactivate();
		bn_start_->deactivate();
		bn_save_->activate();
		bn_cancel_->activate();
		bn_edit_->deactivate();
		bn_edit_qth_->activate();
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->activate();
			ip_field_[ix]->activate();
		}
		ip_notes_->activate();
		break;
	case QSO_PENDING:
		bn_activate_->deactivate();
		bn_start_->activate();
		bn_save_->activate();
		bn_cancel_->activate();
		bn_edit_->deactivate();
		bn_edit_qth_->deactivate();
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->activate();
			ip_field_[ix]->activate();
		}
		ip_notes_->activate();
		break;
	case QSO_STARTED:
		bn_activate_->deactivate();
		bn_start_->deactivate();
		bn_save_->activate();
		bn_cancel_->activate();
		bn_edit_->deactivate();
		bn_edit_qth_->activate();
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->activate();
			ip_field_[ix]->activate();
		}
		ip_notes_->activate();
		break;
	}
	bn_activate_->redraw();
	bn_start_->redraw();
	bn_save_->redraw();
	bn_cancel_->redraw();
	bn_edit_->redraw();
	bn_edit_qth_->redraw();
}

// Update specific station item input
void qso_manager::qso_group::update_station_choices(stn_item_t station_item) {
	// Note can update multiple items as the flags are bit wise
	if (station_item & RIG) {
		field_ips_["MY_RIG"]->reload_choice();
	}
	if (station_item & ANTENNA) {
		field_ips_["MY_ANTENNA"]->reload_choice();
	}
	if (station_item & QTH) {
		field_ips_["APP_ZZA_QTH"]->reload_choice();
	}
	if (station_item & CALLSIGN) {
		field_ips_["STATION_CALLSIGN"]->reload_choice();
	}
	redraw();
}

// Copy record to the fields - reverse of above
void qso_manager::qso_group::copy_qso_to_display(int flags) {
	if (current_qso_) {
		for (int i = 0; i < NUMBER_TOTAL; i++) {
			string field;
			if (i < NUMBER_FIXED) field = fixed_names_[i];
			else field = ch_field_[i]->value();
			bool copy = false;
			if (flags == CF_ALL) copy = true;
			else {
				// Copy operating parameters
				if (flags & CF_RIG_ETC) {
					if (field == "MY_RIG" || field == "MY_ANTENNA" ||
						field == "STATION_CALLSIGN" || field == "APP_ZZA_QTH") copy = true;
				}
				// Copy Rig based parameters
				if (flags & CF_CAT) {
					if (field == "FREQ" || field == "BAND" || field == "TX_PWR" ||
						field == "MODE" || field == "SUBMODE") copy = true;
				}
				// Copy timestamp parameters
				if (flags & CF_CLOCK) {
					if (field == "QSO_DATE" || field == "QSO_DATE_OFF" ||
						field == "TIME_ON" || field == "TIME_OFF") copy = true;
				}
				// TODO - check any other feilds that may need copying
				// We could use a sledge-hammer approach and always copy evry field
				// The reason for a filter is to avoid unnecessary updates (and hence flicker)
			}
			if (copy && field.length()) {
				ip_field_[i]->value(current_qso_->item(field, true).c_str());
			}
		}
		ip_notes_->value(current_qso_->item("NOTES").c_str());
		// If QTH changes tell DXA-IF to update home_location
		check_qth_changed();
	}
}

// Copy from an existing record: fields depend on flags set
void qso_manager::qso_group::copy_qso_to_qso(record* old_record, int flags) {
	if (current_qso_) {
		if (flags == CF_ALL) {
			*current_qso_ = *old_record;
		}
		else {
			if (flags & CF_RIG_ETC) {
				current_qso_->item("MY_RIG", old_record->item("MY_RIG"));
				current_qso_->item("MY_ANTENNA", old_record->item("MY_ANTENNA"));
				current_qso_->item("APP_ZZA_QTH", old_record->item("APP_ZZA_QTH"));
				current_qso_->item("STATION_CALLSIGN", old_record->item("STATION_CALLSIGN"));
			}
			if (flags & CF_CAT) {
				// Copy all the fields relevant to operating conditions
				current_qso_->item("FREQ", old_record->item("FREQ"));
				current_qso_->item("MODE", old_record->item("MODE"));
				current_qso_->item("SUBMODE", old_record->item("SUBMODE"));
				current_qso_->item("TX_PWR", old_record->item("TX_PWR"));
			}
			if (flags & CF_CLOCK) {
				// Copy timestamps
				current_qso_->item("QSO_DATE", old_record->item("QSO_DATE"));
				current_qso_->item("TIME_ON", old_record->item("TIME_ON"));
				current_qso_->item("QSO_DATE_OFF", old_record->item("QSO_DATE_OFF"));
				current_qso_->item("TIME_OFF", old_record->item("TIME_OFF"));
			}
		}
		copy_qso_to_display(flags);
	}
}

// Copy fields from CAT and default rig etc.
void qso_manager::qso_group::copy_cat_to_qso() {
	current_qso_->item("FREQ", rig_if_->get_frequency(true));
	// Get mode - NB USB/LSB need further processing
	string mode;
	string submode;
	rig_if_->get_string_mode(mode, submode);
	if (mode != "DATA L" && mode != "DATA U") {
		current_qso_->item("MODE", mode);
		current_qso_->item("SUBMODE", submode);
	}
	else {
		current_qso_->item("MODE", string(""));
		current_qso_->item("SUBMODE", string(""));
	}
	current_qso_->item("TX_PWR", rig_if_->get_tx_power());

	copy_qso_to_display(CF_CAT);
}

// Copy current timestamp to QSO
void qso_manager::qso_group::copy_clock_to_qso(time_t clock) {
	// Only allow this if in activate - will be called every second 
	if (current_qso_) {
		tm* value = gmtime(&clock);
		char result[100];
		// convert date
		strftime(result, 99, "%Y%m%d", value);
		current_qso_->item("QSO_DATE", string(result));
		// convert time
		strftime(result, 99, "%H%M%S", value);
		current_qso_->item("TIME_ON", string(result));
		current_qso_->item("QSO_DATE_OFF", string(""));
		current_qso_->item("TIME_OFF", string(""));

		copy_qso_to_display(CF_CLOCK);
	}
}

// Clear fields
void qso_manager::qso_group::clear_display() {
	for (int i = 0; i < NUMBER_TOTAL; i++) {
		ip_field_[i]->value("");
	}
	ip_notes_->value("");
}

// Clear fields in current QSO
void qso_manager::qso_group::clear_qso() {
	current_qso_->delete_contents();
	copy_qso_to_display(CF_ALL);
}

// Select logging mode
void qso_manager::qso_group::cb_logging_mode(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, logging_mode_t>(w, v);
	qso_group* that = ancestor_view<qso_group>(w);
	// Deactivate and reactivate to pick up logging mode changes
	if (that->logging_state_ == QSO_PENDING) {
		that->action_deactivate();
		that->action_activate();
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

// Activate- Go from QSO_INACTIVE to QSO_PENDING
void qso_manager::qso_group::cb_activate(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	if (that->logging_state_ == QSO_INACTIVE) {
		that->action_activate();
	}
}

// Start QSO - transition from QSO_INACTIVE->QSO_PENDING->QSO_STARTED
void qso_manager::qso_group::cb_start(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	qso_manager* mgr = (qso_manager*)that->parent();
	switch (that->logging_state_) {
	case QSO_INACTIVE:
		that->action_activate();
		// Fall into next state
	case QSO_PENDING:
		that->action_start();
		break;
	}
}

// Save QSO - transition through QSO_PENDING->QSO_STARTED->QSO_INACTIVE saving QSO
void qso_manager::qso_group::cb_save(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	// Two routes - QSO entry
	case QSO_PENDING:
		that->action_start();
	case QSO_STARTED:
		that->action_save();
		that->action_activate();
		break;
	// QSO editing
	case QSO_EDIT:
		that->action_save_edit();
		break;
	}
}

// Cancel QSO - delete QSO; clear fields
void qso_manager::qso_group::cb_cancel(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QSO_PENDING:
		that->action_deactivate();
		break;
	case QSO_STARTED:
		that->action_cancel();
		break;
	case QSO_EDIT:
		that->action_cancel_edit();
		break;
	}
}

// Edit QSO - transition to QSO_EDIT
void qso_manager::qso_group::cb_edit(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QSO_INACTIVE:
		that->action_edit();
		break;
	}
}

// Callback - Worked B4? button
void qso_manager::qso_group::cb_wkb4(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	extract_records_->extract_call(string(that->field_ips_["CALL"]->value()));
	book_->selection(that->current_rec_num_, HT_SELECTED);
}

// Callback - Parse callsign
void qso_manager::qso_group::cb_parse(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	qso_manager* mgr = (qso_manager*)that->parent();
	// Create a temporary record to parse the callsign
	record* tip_record = mgr->dummy_qso();
	string message = "";
	// Set the callsign in the temporary record
	tip_record->item("CALL", string(that->field_ips_["CALL"]->value()));
	// Parse the temporary record
	message = pfx_data_->get_tip(tip_record);
	// Create a tooltip window at the parse button (in w) X and Y
	Fl_Window* qw = ancestor_view<qso_manager>(w);
	Fl_Window* tw = ::tip_window(message, qw->x_root() + w->x() + w->w() / 2, qw->y_root() + w->y() + w->h() / 2);
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
	delete tip_record;
}

// Callback - Edit QTH
void qso_manager::qso_group::cb_bn_edit_qth(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	qso_manager* mgr = (qso_manager*)that->parent();
	string qth = mgr->get_default(QTH);
	// Open QTH dialog
	qth_dialog* dlg = new qth_dialog(qth);
	set<string> changed_fields;
	record* macro;
	record* current = that->get_default_record();
	switch (dlg->display()) {
	case BN_OK:
		changed_fields = spec_data_->get_macro_changes();
		macro = spec_data_->expand_macro("APP_ZZA_QTH", qth);
		for (auto fx = changed_fields.begin(); fx != changed_fields.end(); fx++) {
			current->item(*fx, macro->item(*fx));
		}
		that->check_qth_changed();
		that->enable_widgets();
		break;
	case BN_CANCEL:
		break;
	}
	delete dlg;
}

// Reset contest serial number
void qso_manager::qso_group::cb_init_serno(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	that->serial_num_ = 1;
	that->initialise_fields();
	that->enable_widgets();
}

// Increment serial number
void qso_manager::qso_group::cb_inc_serno(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	that->serial_num_++;
	that->initialise_fields();
	that->enable_widgets();
}

// Decrement serial number
void qso_manager::qso_group::cb_dec_serno(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	that->serial_num_--;
	that->initialise_fields();
	that->enable_widgets();
}

// Callback change field selected
void qso_manager::qso_group::cb_ch_field(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	field_choice* ch = (field_choice*)w;
	int ix = (int)v;
	const char* field = ch->value();
	that->ip_field_[ix]->field_name(field);	
	that->ip_field_[ix]->value(that->current_qso_->item(field).c_str());

}

// Callback - general input
// v - index of input widget
void qso_manager::qso_group::cb_ip_field(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	field_input* ip = (field_input*)w;
	string field = ip->field_name();
	string value = ip->value();
	that->current_qso_->item(field, value);

	if (field == "FREQ") {
		double freq = atof(value.c_str()) * 1000;
		if (band_view_) {
			band_view_->update(freq);
		}
		prev_freq_ = freq;
	}
	else if (field == "MODE") {
		if (spec_data_->is_submode(value)) {
			that->current_qso_->item("SUBMODE", value);
			that->current_qso_->item("MODE", spec_data_->mode_for_submode(value));
		}
		else {
			that->current_qso_->item("MODE", value);
			that->current_qso_->item("SUBMODE", string(""));
		}
	}
	else if (field == "APP_ZZA_QTH") {
		// Send new value to spec_data to create an empty entry if it's a new one
		if (!ip->menubutton()->changed()) {
			macro_defn entry = { nullptr, "" };
			spec_data_->add_user_macro(field, value, entry);
		}
		that->check_qth_changed();
	}
	tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, that->current_rec_num_);
}

// Callback -notes input
// v - not used
void qso_manager::qso_group::cb_ip_notes(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	string notes;
	cb_value<intl_input, string>(w, &notes);
	that->current_qso_->item("NOTES", notes);
	tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, book_->size() - 1);
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
	int iy;
	for (ix = 0; ix < field_names.size(); ix++) {
		iy = ix + NUMBER_FIXED;
		if (new_fields) {
			ch_field_[iy]->value(field_names[ix].c_str());
			ip_field_[iy]->field_name(field_names[ix].c_str());
		}
		if (lock_preset) {
			ch_field_[iy]->deactivate();
		}
		else {
			ch_field_[iy]->activate();
		}
	}
	for (; iy < NUMBER_TOTAL; iy++) {
		ch_field_[iy]->value("");
		ip_field_[iy]->value("");
		ip_field_[iy]->field_name("");
	}
	// Set contest format
	ch_format_->value(exch_fmt_id_.c_str());
	// Default Contest TX values
	if (field_mode_ == CONTEST) {
		// Automatically create a pending QSO
		if (logging_state_ == QSO_INACTIVE) {
			action_activate();
		}

		vector<string> tx_fields;
		split_line(ef_txs_[exch_fmt_ix_], tx_fields, ',');
		for (size_t i = 0; i < tx_fields.size(); i++) {
			int ix = NUMBER_FIXED + i;
			if (tx_fields[i] == "RST_SENT") {
				string contest_mode = spec_data_->dxcc_mode(current_qso_->item("MODE"));
				if (contest_mode == "CW" || contest_mode == "DATA") {
					ip_field_[ix]->value("599");
				}
				else {
					ip_field_[ix]->value("59");
				}
			}
			else if (tx_fields[i] == "STX") {
				char text[10];
				snprintf(text, 10, "%03d", serial_num_);
				ip_field_[ix]->value(text);
			}
			else {
				ip_field_[ix]->value(current_qso_->item(tx_fields[i]).c_str());
			}
		}
		field_ips_["CALL"]->value("");
		ip_notes_->value("");
		for (size_t i = NUMBER_FIXED + tx_fields.size(); i < NUMBER_TOTAL; i++) {
			ip_field_[i]->value("");
		}
	}
	else {
		field_ips_["CALL"]->value("");
		ip_notes_->value("");
		for (int i = NUMBER_FIXED; i < NUMBER_TOTAL; i++) {
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
	for (int i = NUMBER_FIXED; i < NUMBER_TOTAL; i++) {
		const char* field = ch_field_[i]->value();
		if (strlen(field)) {
			if (i > NUMBER_FIXED) {
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

// Get default record to copy
record* qso_manager::qso_group::get_default_record() {
	switch (logging_state_) {
	case QSO_INACTIVE:
		switch (logging_mode_) {
		case LM_OFF_AIR:
		case LM_ON_AIR_CAT:
		case LM_ON_AIR_TIME:
			return book_->get_latest();
		case LM_ON_AIR_COPY:
			return book_->get_record();
		default: 
			return nullptr;
		}
	default:
		return current_qso_;
	}
}

// Check if QTH has changed and action change (redraw DxAtlas
void qso_manager::qso_group::check_qth_changed() {
	record* current = get_default_record();
	if (current) {
		if (current->item("MY_GRIDSQUARE", true) != previous_locator_ ||
			current->item("APP_ZZA_QTH") != previous_qth_) {
			previous_locator_ = current->item("MY_GRIDSQUARE", true);
			previous_qth_ = current->item("APP_ZZA_QTH");
			dxa_if_->update(HT_LOCATION);
		}
	}
}

// Action ACTIVATE: transition from QSO_INACTIVE to QSO_PENDING
void qso_manager::qso_group::action_activate() {
	if (logging_state_ != QSO_INACTIVE || current_qso_ != nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to activate a QSO when one is already active");
		return;
	}
	current_qso_ = new record;
	time_t now = time(nullptr);
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	record* source_record = get_default_record();
	switch (logging_mode_) {
	case LM_OFF_AIR:
		copy_qso_to_qso(source_record, CF_RIG_ETC);
		break;
	case LM_ON_AIR_CAT:
		copy_qso_to_qso(source_record, CF_RIG_ETC);
		copy_cat_to_qso();
		copy_clock_to_qso(now);
		break;
	case LM_ON_AIR_COPY:
		copy_qso_to_qso(source_record, CF_RIG_ETC | CF_CAT);
		copy_clock_to_qso(now);
		break;
	case LM_ON_AIR_TIME:
		copy_qso_to_qso(source_record, CF_RIG_ETC);
		copy_clock_to_qso(now);
		break;
	}
	copy_qso_to_display(CF_ALL);
	logging_state_ = QSO_PENDING;
	enable_widgets();
}

// Action START - transition from QSO_PENDING to QSO_STARTED
void qso_manager::qso_group::action_start() {
	if (logging_state_ != QSO_PENDING || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to start a QSO while not pending");
		return;
	}
	// Add to book
	current_rec_num_ = book_->append_record(current_qso_);
	book_->selection(book_->item_number(current_rec_num_), HT_INSERTED);
	logging_state_ = QSO_STARTED;
	enable_widgets();
}

// Action SAVE - transition from QSO_STARTED to QSO_INACTIVE while saving record
void qso_manager::qso_group::action_save() {
	if (logging_state_ != QSO_STARTED || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to save a QSO when not inputing one");
		return;
	}
	if (current_qso_ != book_->get_record()) {
		status_->misc_status(ST_SEVERE, "Mismatch between selected QSO in book and QSO manager");
		return;
	}
	bool old_save_enabled = book_->save_enabled();
	record_num_t item_number = book_->item_number(current_rec_num_);
	book_->enable_save(false);
	// On-air logging add date/time off
	switch (logging_mode_) {
	case LM_ON_AIR_CAT:
	case LM_ON_AIR_COPY:
	case LM_ON_AIR_TIME:
		if (current_qso_->item("TIME_OFF") == "") {
			// Add end date/time - current time of interactive entering
			// Get current date and time in UTC
			string timestamp = now(false, "%Y%m%d%H%M%S");
			current_qso_->item("QSO_DATE_OFF", timestamp.substr(0, 8));
			// Time as HHMMSS - always log seconds.
			current_qso_->item("TIME_OFF", timestamp.substr(8));
		}
		break;
	case LM_OFF_AIR:
		book_->correct_record_position(item_number);
		break;
	}

	// check whether record has changed - when parsed
	if (pfx_data_->parse(current_qso_)) {
	}

	// check whether record has changed - when validated
	if (spec_data_->validate(current_qso_, item_number)) {
	}

	book_->add_use_data(current_qso_);

	// Upload QSO to QSL servers
	book_->upload_qso(current_rec_num_);
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();

	// If new or changed then update the fact and let every one know
	book_->modified(true);
	book_->selection(item_number, HT_INSERTED);
	book_->enable_save(old_save_enabled);
}

// Action CANCEL - Transition from QSO_STARTED to QSO_INACTIVE without saving record
void qso_manager::qso_group::action_cancel() {
	if (logging_state_ != QSO_STARTED || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to save a QSO when not inputing one");
		return;
	}
	if (current_qso_ != book_->get_record()) {
		status_->misc_status(ST_SEVERE, "Mismatch between selected QSO in book and QSO manager");
		return;
	}
	book_->delete_record(true);
	delete current_qso_;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	check_qth_changed();
	enable_widgets();
}

// Action DEACTIVATE - Transition from QSO_PENDING to QSO_INACTIVE
void qso_manager::qso_group::action_deactivate() {
	if (logging_state_ != QSO_PENDING || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to deactivate when not pending");
	}
	delete current_qso_;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action EDIT - Transition from QSO_INACTIVE to QSO_EDIT
void qso_manager::qso_group::action_edit() {
	if (logging_state_ != QSO_INACTIVE || original_qso_ != nullptr ) {
		status_->misc_status(ST_SEVERE, "Attempting to edit a QSO while inputing a QSO");
		return;
	}
	// Copy current selection
	current_rec_num_ = book_->selection();
	// Edit currently selected QSO
	current_qso_ = book_->get_record();
	// And save a copy of it
	original_qso_ = new record(*current_qso_);
	copy_qso_to_display(CF_ALL);
	logging_state_ = QSO_EDIT;
	enable_widgets();
}

// Action SAVE EDIT - Transition from QSO_EDIT to QSO_INACTIVE while saving changes
void qso_manager::qso_group::action_save_edit() {
	if (logging_state_ != QSO_EDIT || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to save a QSO when not editing one");
		return;
	}
	if (current_rec_num_ != book_->selection()) {
		status_->misc_status(ST_SEVERE, "Mismatch between selected QSO in book and QSO manager");
		return;
	}
	// We no longer need to maintain the copy of the original QSO
	book_->add_use_data(current_qso_);
	book_->modified(true);
	delete original_qso_;
	original_qso_ = nullptr;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// ACtion CANCEL EDIT - Transition from QSO_EDIT to QSO_INACTIVE scrapping changes
void qso_manager::qso_group::action_cancel_edit() {
	if (logging_state_ != QSO_EDIT || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "Attempting to save a QSO when not editing one");
		return;
	}
	if (current_rec_num_ != book_->selection()) {
		status_->misc_status(ST_SEVERE, "Mismatch between selected QSO in book and QSO manager");
		return;
	}
	// Copy original back to the book
	*book_->get_record() = *original_qso_;
	delete original_qso_;
	original_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	copy_qso_to_display(CF_ALL);
	enable_widgets();
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

	qso_manager* mgr = ancestor_view<qso_manager>(that);
	if (mgr->qso_group_->logging_state_ == QSO_PENDING) {
		mgr->qso_group_->copy_clock_to_qso(now);
	}

	Fl::repeat_timeout(UTC_TIMER, cb_timer_clock, v);
}


// The main dialog constructor
qso_manager::qso_manager(int W, int H, const char* label) :
	Fl_Window(W, H, label)
	, cat_group_(nullptr)
	, created_(false)
	, font_(FONT)
	, fontsize_(FONT_SIZE)
{
	load_values();
	create_form(0,0);
	update_rig();
	update_qso();

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

	begin();

	qso_group_ = new qso_group(curr_x, curr_y, 0, 0, nullptr);
	qso_group_->create_form(curr_x, curr_y);
	curr_x += qso_group_->w();
	curr_y += qso_group_->h();
	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	curr_x = X + GAP;
	curr_y += GAP;
	int save_y = curr_y;

	cat_group_ = new cat_group(curr_x, curr_y, 0, 0, nullptr);
	cat_group_->create_form(curr_x, curr_y);

	curr_x += cat_group_->w() + GAP;

	clock_group_ = new clock_group(curr_x, curr_y, 0, 0, nullptr);
	clock_group_->create_form(curr_x, curr_y);
	curr_x += clock_group_->w();
	curr_y += max(clock_group_->h(), cat_group_->h());

	max_x = max(max_x, curr_x);
	max_y = max(max_y, curr_y);

	this->resizable(nullptr);
	this->size(max_x + GAP - X, max_y + GAP - Y);
	created_ = true;

	end();
	show();

	enable_widgets();
}

//
//// Antenna and Rig ("Use") widgets
//// assumes qso_manager is the current active group
//void qso_manager::create_use_widgets(int& curr_x, int& curr_y) {
//
//	int max_w = 0;
//	int max_h = 0;
//
//	Fl_Group* station_grp = new Fl_Group(curr_x, curr_y, 500, 500,
//		"Station Configuration - use QSO group to set station details for QSO");
//	station_grp->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
//	station_grp->box(FL_BORDER_BOX);
//	curr_x += GAP;
//	curr_y += HTEXT;
//	int save_x = curr_x;
//	int save_y = curr_y;
//
//	// common_grp resizes itself
//	// Antenna group of widgets
//	antenna_grp_ = new common_grp(curr_x, curr_y, 10, 10, "Antenna", ANTENNA);
//	antenna_grp_->tooltip("Allows the antennas to be managed");
//	antenna_grp_->end();
//	antenna_grp_->show();
//	max_w = max(max_w, antenna_grp_->x() + antenna_grp_->w() - curr_x);
//	max_h = max(max_h, antenna_grp_->y() + antenna_grp_->h() - curr_y);
//	curr_x += antenna_grp_->w() + GAP;
//
//	// Rig group of widgets
//	rig_grp_ = new common_grp(curr_x, curr_y, 10, 10, "Rig", RIG);
//	rig_grp_->tooltip("Allows the rigs to be managed");
//	rig_grp_->end();
//	rig_grp_->show();
//	curr_x += rig_grp_->w();
//	curr_y += max(antenna_grp_->h(), rig_grp_->h()) + GAP;
//
//	max_w = max(max_w, curr_x - save_x);
//	max_h = max(max_h, curr_y - save_y);
//
//	// Box to contain antenna-rig connectivity
//	Fl_Help_View* w19 = new Fl_Help_View(save_x, curr_y, max_w, HBUTTON * 2);
//	w19->box(FL_FLAT_BOX);
//	w19->labelfont(FONT);
//	w19->labelsize(FONT_SIZE);
//	w19->textfont(FONT);
//	w19->textsize(FONT_SIZE);
//	ant_rig_box_ = w19;
//
//	curr_x += GAP;
//	curr_y += w19->h() + GAP;
//
//	int max_h1 = max(max_h, curr_y - station_grp->y());
//
//	// Callsign group
//	curr_y = save_y;
//	callsign_grp_ = new common_grp(curr_x, curr_y, 10, 10, "Callsign", CALLSIGN);
//	callsign_grp_->tooltip("Manage the callsigns available");
//	callsign_grp_->end(); 
//	callsign_grp_->show();
//
//	// QTH group
//	curr_y += callsign_grp_->h() + GAP;
//	qth_grp_ = new common_grp(curr_x, curr_y, 10, 10, "QTH", QTH);
//	qth_grp_->tooltip("Manage the locations available");
//	qth_grp_->end();
//	qth_grp_->show();
//
//	curr_x += max(callsign_grp_->w(), qth_grp_->w()) + GAP;
//
//	max_w = max(max_w, curr_x - station_grp->x());
//	curr_y += qth_grp_->h() + GAP;
//	int max_h2 = curr_y - save_y;
//
//	// Adjust w19 to be level with qth_grp is the latter is lower
//	if (max_h2 > max_h1) {
//		w19->size(w19->w(), w19->h() + max_h2 - max_h1);
//		max_h = max_h2;
//	}
//	else {
//		max_h = max_h1;
//	};
//
//	station_grp->end();
//	station_grp->resizable(nullptr);
//	station_grp->size(max_w, max_h);
//
//	curr_x = station_grp->x() + station_grp->w();
//	curr_y = station_grp->y() + station_grp->h();
//
//}

//// CAT settings widgets
//void qso_manager::create_cat_widgets(int& curr_x, int& curr_y) {
//
//	int max_w = 0;
//	int max_h = 0;
//
//	// CAT control group
//	cat_grp_ = new Fl_Group(curr_x, curr_y, 10, 10, "CAT");
//	cat_grp_->labelfont(FONT);
//	cat_grp_->labelsize(FONT_SIZE);
//	cat_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//	cat_grp_->box(FL_BORDER_BOX);
//
//	// Choice - Select the rig model (Manufacturer/Model)
//	Fl_Choice* ch_model_ = new Fl_Choice(cat_grp_->x() + WLABEL, cat_grp_->y() + HTEXT, WSMEDIT, HTEXT, "Rig");
//	ch_model_->align(FL_ALIGN_LEFT);
//	ch_model_->labelsize(FONT_SIZE);
//	ch_model_->textsize(FONT_SIZE);
//	ch_model_->tooltip("Select the model - for Hamlib");
//	ch_model_->callback(cb_ch_model, nullptr);
//	rig_model_choice_ = ch_model_;
//
//	// Populate the manufacturere and model choice widgets
//	populate_model_choice();
//
//	// Hamlib control grp
//	// RIG=====v
//	// PORTv  ALL*
//	// BAUDv  OVR*
//	serial_grp_ = new Fl_Group(cat_grp_->x() + GAP, ch_model_->y() + ch_model_->h() + GAP, 10, 10);
//	serial_grp_->labelsize(FONT_SIZE);
//	serial_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//	serial_grp_->box(FL_NO_BOX);
//
//
//	// Choice port name - serial
//	Fl_Choice* ch_port = new Fl_Choice(ch_model_->x(), ch_model_->y() + ch_model_->h(), WBUTTON, HTEXT, "Port");
//	ch_port->align(FL_ALIGN_LEFT);
//	ch_port->labelsize(FONT_SIZE);
//	ch_port->textsize(FONT_SIZE);
//	ch_port->callback(cb_ch_port, nullptr);
//	ch_port->tooltip("Select the comms port to use");
//	port_if_choice_ = ch_port;
//
//	// Use all ports
//	Fl_Check_Button* bn_useall = new Fl_Check_Button(ch_port->x() + ch_port->w(), ch_port->y(), HBUTTON, HBUTTON, "All");
//	bn_useall->align(FL_ALIGN_RIGHT);
//	bn_useall->labelfont(FONT);
//	bn_useall->labelsize(FONT_SIZE);
//	bn_useall->tooltip("Select all existing ports, not just those available");
//	bn_useall->callback(cb_bn_all, &all_ports_);
//	show_all_ports_ = bn_useall;
//	populate_port_choice();
//
//	// Baud rate input 
//	Fl_Choice* ch_baudrate = new Fl_Choice(ch_model_->x(), ch_port->y() + ch_port->h(), WBUTTON, HTEXT, "Baud rate");
//	ch_baudrate->align(FL_ALIGN_LEFT);
//	ch_baudrate->labelsize(FONT_SIZE);
//	ch_baudrate->textsize(FONT_SIZE);
//	ch_baudrate->tooltip("Enter baud rate");
//	ch_baudrate->callback(cb_ch_baud, nullptr);
//	baud_rate_choice_ = ch_baudrate;
//
//	// Override capabilities (as coded in hamlib)
//	Fl_Check_Button* bn_override = new Fl_Check_Button(ch_baudrate->x() + ch_baudrate->w(), ch_baudrate->y(), HBUTTON, HBUTTON, "Override\ncapability");
//	bn_override->align(FL_ALIGN_RIGHT);
//	bn_override->labelsize(FONT_SIZE);
//	bn_override->tooltip("Allow full baud rate selection");
//	bn_override->callback(cb_ch_over, nullptr);
//	override_check_ = bn_override;
//
//	populate_baud_choice();
//
//	serial_grp_->resizable(nullptr);
//	serial_grp_->size(max(ch_model_->x() + ch_model_->w(),
//		max(bn_useall->x() + bn_useall->w(), bn_override->x() + bn_override->w())) + GAP - serial_grp_->x(),
//		bn_override->y() + bn_override->h() + GAP - serial_grp_->y());
//
//	serial_grp_->end();
//
//	network_grp_ = new Fl_Group(serial_grp_->x(), serial_grp_->y(), 10, 10);
//	network_grp_->labelsize(FONT_SIZE);
//	network_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//	network_grp_->box(FL_NO_BOX);
//
//	// Input port name - network
//	Fl_Input* ip_port = new Fl_Input(ch_model_->x(), ch_model_->y() + ch_model_->h(), WSMEDIT, HTEXT, "Port");
//	ip_port->align(FL_ALIGN_LEFT);
//	ip_port->labelsize(FONT_SIZE);
//	ip_port->textsize(FONT_SIZE);
//	ip_port->callback(cb_ip_port, nullptr);
//	ip_port->tooltip("Enter the network/USB port to use");
//	ip_port->value(rig_grp_->info().rig_data.hamlib_params.port_name.c_str());
//	port_if_input_ = ip_port;
//
//	network_grp_->resizable(nullptr);
//	network_grp_->size(ip_port->x() + ip_port->w() + GAP - network_grp_->x(),
//		ip_port->y() + ip_port->h() + GAP - network_grp_->y());
//
//	network_grp_->end();
//
//	int max_y = max(serial_grp_->y() + serial_grp_->h(), network_grp_->y() + network_grp_->h());
//
//	// Connected status
//	connect_bn_ = new Fl_Button(serial_grp_->x(), max_y + GAP, WBUTTON * 2, HBUTTON, "Connect...");
//	connect_bn_->labelfont(FONT);
//	connect_bn_->labelsize(FONT_SIZE);
//	connect_bn_->color(FL_YELLOW);
//	connect_bn_->tooltip("Select to attempt to connect rig");
//	connect_bn_->callback(cb_bn_connect, nullptr);
//
//	// Poll period group;
//// <>FAST
//// <>SLOW
//	Fl_Group* poll_grp = new Fl_Group(cat_grp_->x(), connect_bn_->y() + connect_bn_->h() + GAP, 10, 10, "Polling interval (s)");
//	poll_grp->labelsize(FONT_SIZE);
//	poll_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//	poll_grp->box(FL_NO_BOX);
//
//	// Spinner to select fast polling rate (i.e. when still connected)
//	ctr_pollfast_ = new Fl_Spinner(poll_grp->x() + WLABEL, poll_grp->y() + HTEXT, WSMEDIT, HTEXT, "Conn'd");
//	ctr_pollfast_->align(FL_ALIGN_LEFT);
//	ctr_pollfast_->labelsize(FONT_SIZE);
//	ctr_pollfast_->textsize(FONT_SIZE);
//	ctr_pollfast_->tooltip("Select the polling period for fast polling");
//	ctr_pollfast_->type(FL_FLOAT_INPUT);
//	ctr_pollfast_->minimum(FAST_RIG_MIN);
//	ctr_pollfast_->maximum(FAST_RIG_MAX);
//	ctr_pollfast_->step(0.01);
//	ctr_pollfast_->value(rig_grp_->info().rig_data.fast_poll_interval);
//	ctr_pollfast_->callback(cb_ctr_pollfast);
//	ctr_pollfast_->when(FL_WHEN_CHANGED);
//
//	// Spinner to select slow polling rate (i.e. after disconnection to avoid excessive errors)
//	Fl_Spinner* ctr_pollslow_ = new Fl_Spinner(ctr_pollfast_->x(), ctr_pollfast_->y() + ctr_pollfast_->h(), WSMEDIT, HTEXT, "Disconn'd");
//	ctr_pollslow_->align(FL_ALIGN_LEFT);
//	ctr_pollslow_->labelsize(FONT_SIZE);
//	ctr_pollslow_->textsize(FONT_SIZE);
//	ctr_pollslow_->tooltip("Select the polling period for slow polling");
//	ctr_pollslow_->type(FL_FLOAT_INPUT);
//	ctr_pollslow_->minimum(SLOW_RIG_MIN);
//	ctr_pollslow_->maximum(SLOW_RIG_MAX);
//	ctr_pollslow_->step(0.5);
//	ctr_pollslow_->value(rig_grp_->info().rig_data.slow_poll_interval);
//	ctr_pollslow_->callback(cb_ctr_pollslow);
//	ctr_pollslow_->when(FL_WHEN_CHANGED);
//
//	poll_grp->resizable(nullptr);
//	poll_grp->size(max(ctr_pollfast_->w(), ctr_pollslow_->w()) + WLABEL + GAP, ctr_pollslow_->y() + ctr_pollslow_->h() + GAP - poll_grp->y());
//	poll_grp->end();
//
//	// Display hamlib ot flrig settings as selected
//	enable_cat_widgets();
//
//	cat_grp_->resizable(nullptr);
//	cat_grp_->size(max(poll_grp->w(),
//		max(serial_grp_->w(), network_grp_->w()) + GAP),
//		poll_grp->y() + poll_grp->h() - cat_grp_->y());
//	max_w = max(max_w, cat_grp_->x() + cat_grp_->w() - curr_x);
//	max_h = max(max_w, cat_grp_->y() + cat_grp_->h() - curr_y);
//	cat_grp_->end();
//
//	curr_x += max_w;
//	curr_y += max_h;
//
//}
//
//// Alarm widgets
//void qso_manager::create_alarm_widgets(int& curr_x, int& curr_y) {
//
//	int max_w = 0;
//	int max_h = 0;
//
//	alarms_grp_ = new Fl_Group(curr_x, curr_y, 10, 10, "Alarms");
//	alarms_grp_->labelfont(FONT);
//	alarms_grp_->labelsize(FONT_SIZE);
//	alarms_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
//	alarms_grp_->box(FL_BORDER_BOX);
//
//	// dial to select SWR warning and error levels
//	dial_swr_ = new alarm_dial(alarms_grp_->x() + GAP, alarms_grp_->y() + GAP, 80, 80, "SWR");
//	dial_swr_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER);
//	dial_swr_->labelsize(FONT_SIZE);
//	dial_swr_->labelfont(FONT);
//	dial_swr_->tooltip("Modify the SWR warning/error levels");
//	dial_swr_->minimum(1.0);
//	dial_swr_->maximum(5.0);
//	dial_swr_->step(0.1);
//	dial_swr_->alarm_color(FL_RED);
//	dial_swr_->selection_color(FL_BLACK);
//	dial_swr_->alarms(rig_grp_->info().rig_data.alarms.swr_warning,
//		rig_grp_->info().rig_data.alarms.swr_error);
//	dial_swr_->value(rig_if_ ? rig_if_->swr_meter() : 1.0);
//	dial_swr_->callback(cb_alarm_swr, nullptr);
//
//	// dial to select power warning levels
//	dial_pwr_ = new alarm_dial(dial_swr_->x() + dial_swr_->w() + GAP, alarms_grp_->y() + GAP, 80, 80, "Power");
//	dial_pwr_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER);
//	dial_pwr_->labelsize(FONT_SIZE);
//	dial_pwr_->labelfont(FONT);
//	dial_pwr_->tooltip("Modify the Power warning levels");
//	dial_pwr_->minimum(0.0);
//	dial_pwr_->maximum(100.0);
//	dial_pwr_->step(1);
//	dial_pwr_->alarms(rig_grp_->info().rig_data.alarms.power_warning, nan(""));
//	dial_pwr_->value(rig_if_ ? rig_if_->pwr_meter() : 0.0);
//	dial_pwr_->callback(cb_alarm_pwr, nullptr);
//
//	// dial to display Vdd minimum and maximum error levels
//	dial_vdd_ = new alarm_dial(dial_pwr_->x() + dial_pwr_->w() + GAP, alarms_grp_->y() + GAP, 80, 80, "Vdd");
//	dial_vdd_->align(FL_ALIGN_BOTTOM | FL_ALIGN_CENTER);
//	dial_vdd_->labelsize(FONT_SIZE);
//	dial_vdd_->labelfont(FONT);
//	dial_vdd_->tooltip("Modify the Vdd warning levels");
//	dial_vdd_->minimum(10.0);
//	dial_vdd_->maximum(20.0);
//	dial_vdd_->step(0.1);
//	dial_vdd_->alarms(rig_grp_->info().rig_data.alarms.voltage_minimum,
//		rig_grp_->info().rig_data.alarms.voltage_maximum);
//	dial_vdd_->value(rig_if_ ? rig_if_->vdd_meter() : 10.0);
//	dial_vdd_->callback(cb_alarm_vdd, nullptr);
//
//	alarms_grp_->resizable(nullptr);
//
//	alarms_grp_->size(dial_swr_->w() + dial_pwr_->w() + dial_vdd_->w() + (GAP * 4), dial_swr_->h() + GAP + HTEXT);
//	max_w = max(max_w, alarms_grp_->x() + alarms_grp_->w() - curr_x);
//	max_h = max(max_h, alarms_grp_->y() + alarms_grp_->h() - curr_y);
//	alarms_grp_->end();
//
//	curr_x += max_w;
//	curr_y += max_h;
//
//}

// Load values
void qso_manager::load_values() {

	// These are static, but will get to the same value each time
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences log_settings(user_settings, "Scratchpad");
	log_settings.get("Font Name", (int&)font_, FONT);
	log_settings.get("Font Size", (int&)fontsize_, FONT_SIZE);

}

//void qso_manager::load_locations() {
//	Fl_Preferences stations_settings(settings_, "Stations");
//
//	// Get the settings for the list of QTHs
//	Fl_Preferences qths_settings(stations_settings, "QTHs");
//	// Number of items described in settings
//	int num_items = qths_settings.groups();
//	// For each item in the settings
//	for (int i = 0; i < num_items; i++) {
//		// Get that item's settings
//		string name = qths_settings.group(i);
//	}
//
//}

// Write values back to settings - write the three groups back separately
void qso_manager::save_values() {

	// Save window position
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.set("Left", x_root());
	dash_settings.set("Top", y_root());
	dash_settings.set("Enabled", (signed int)shown());
	dash_settings.set("Logging Mode", logging_mode());

//	Fl_Preferences stations_settings(settings_, "Stations");
	qso_group_->save_values();
	cat_group_->save_values();
	clock_group_->save_values();
}

// Enable the widgets - activate the group associated with each rig handler when that
// handler is enabled
void qso_manager::enable_widgets() {

	// Not all widgets may exist yet!
	if (!created_) return;

	qso_group_->enable_widgets();
	cat_group_->enable_widgets();
}

//// Enable Antenna and rig setting widgets
//void qso_manager::enable_use_widgets() {
//
//	antenna_grp_->enable_widgets();
//	rig_grp_->enable_widgets();
//	callsign_grp_->enable_widgets();
//	qth_grp_->enable_widgets();
//
//	// Antenna/rig compatibility
//	Fl_Help_View* mlo = (Fl_Help_View*)ant_rig_box_;
//	if (rig_if_ || qso_group_->current_qso_) {
//		// We have either a CAT connection or a previous record to cop
//		double frequency;
//		// Get logging mode frequency
//		if (rig_if_) {
//			frequency = rig_if_->tx_frequency();
//		}
//		else {
//			qso_group_->current_qso_->item("FREQ", frequency);
//		}
//		string rig_band = spec_data_->band_for_freq(frequency / 1000000.0);
//		// Check if band supported by rig
//		vector<string> bands = rig_grp_->info().intended_bands;
//		bool found = false;
//		for (auto it = bands.begin(); it != bands.end() && !found; it++) {
//			if (*it == rig_band) {
//				found = true;
//			}
//		}
//		if (!found) {
//			// Rig does not have band
//			mlo->color(FL_RED);
//			mlo->value("Rig does not support the band read from it");
//
//		}
//		else {
//			found = false;
//			// Check if band supported by antenna
//			vector<string> bands = antenna_grp_->info().intended_bands;
//			for (auto it = bands.begin(); it != bands.end() && !found; it++) {
//				if (*it == rig_band) {
//					found = true;
//				}
//			}
//			if (!found) {
//				// Antenna not intended for band
//				mlo->color(FL_YELLOW);
//				mlo->value("Antenna not intended for this band. It may be what you want");
//			}
//			else {
//				// Antenna AND rig are intended for band
//				mlo->color(FL_GREEN);
//				mlo->value("Antenna is intended for this band");
//			}
//		}
//	}
//	else {
//		// No band selected - check that antenna and rig have at least one #
//		// band they are both meant for.
//		vector<string> r_bands = rig_grp_->info().intended_bands;
//		vector<string> a_bands = antenna_grp_->info().intended_bands;
//		bool found = false;
//		for (auto itr = r_bands.begin(); itr != r_bands.end() && !found; itr++) {
//			for (auto ita = a_bands.begin(); ita != a_bands.end() && !found; ita++) {
//				if (*itr == *ita) found = true;
//			}
//		}
//		if (!found) {
//			// Antenna not intended for same bands as rig
//			mlo->color(FL_RED);
//			mlo->value("Antenna is not intended for any band rig is capable of");
//		}
//		else {
//			// Antenna intended for a band the rig is capabl of
//			mlo->color(FL_DARK_GREEN);
//			mlo->value("Antenna is intended for a band the rig is capable of");
//		}
//	}
//	mlo->textcolor(fl_contrast(FL_BLACK, mlo->color()));
//}

//// Update just the Locations widget from settings
//void qso_manager::update_locations() {
//	load_locations();
//}

//// Enable CAT Connection widgets
//void qso_manager::enable_cat_widgets() {
//
//	// CAT control widgets
//	switch (rig_grp_->info().rig_data.hamlib_params.port_type) {
//	case RIG_PORT_SERIAL:
//		serial_grp_->activate();
//		serial_grp_->show();
//		network_grp_->deactivate();
//		network_grp_->hide();
//		break;
//	case RIG_PORT_NETWORK:
//	case RIG_PORT_USB:
//		serial_grp_->deactivate();
//		serial_grp_->hide();
//		network_grp_->activate();
//		network_grp_->show();
//		break;
//	default:
//		serial_grp_->deactivate();
//		serial_grp_->hide();
//		network_grp_->deactivate();
//		network_grp_->hide();
//		break;
//	}
//
//	// Connect button
//	if (rig_if_) {
//		connect_bn_->color(FL_GREEN);
//		connect_bn_->label("Connected");
//	}
//	else {
//		switch (rig_grp_->info().rig_data.hamlib_params.port_type) {
//		case RIG_PORT_NONE:
//			connect_bn_->color(FL_BACKGROUND_COLOR);
//			connect_bn_->label("No CAT connection");
//			break;
//		default:
//			if (wait_connect_) {
//				connect_bn_->color(FL_YELLOW);
//				connect_bn_->label("... Connect");
//			}
//			else {
//				connect_bn_->color(FL_RED);
//				connect_bn_->label("Disconnected");
//			}
//			break;
//		}
//	}
//}
//
//// Enable the alarm widgets
//void qso_manager::enable_alarm_widgets() {
//
//	if (rig_if_) {
//		alarms_grp_->activate();
//
//		// SWR widget - set colour and raise alarm
//		if (previous_swr_alarm_ != current_swr_alarm_) {
//			char message[200];
//			switch (current_swr_alarm_) {
//			case SWR_ERROR:
//				dial_swr_->selection_color(FL_RED);
//				snprintf(message, 200, "DASH: SWR %g > %g", dial_swr_->Fl_Line_Dial::value(), dial_swr_->alarm2());
//				status_->misc_status(ST_ERROR, message);
//				break;
//			case SWR_WARNING:
//				dial_swr_->selection_color(fl_color_average(FL_RED, FL_YELLOW, 0.33f));
//				snprintf(message, 200, "DASH: SWR %g > %g", dial_swr_->Fl_Line_Dial::value(), dial_swr_->alarm1());
//				status_->misc_status(ST_WARNING, message);
//				break;
//			case SWR_OK:
//				dial_swr_->selection_color(FL_BLACK);
//				snprintf(message, 200, "DASH: SWR %g OK", dial_swr_->Fl_Line_Dial::value());
//				status_->misc_status(ST_OK, message);
//				break;
//			}
//		}
//		if (rig_if_->get_tx()) {
//			dial_swr_->activate();
//		}
//		else {
//			dial_swr_->deactivate();
//		}
//
//		// Power widget - set colour and raise alarm
//		if (previous_pwr_alarm_ != current_pwr_alarm_) {
//			char message[200];
//			switch (current_pwr_alarm_) {
//			case POWER_OK:
//				dial_pwr_->selection_color(FL_BLACK);
//				snprintf(message, 200, "DASH: Power %g OK", dial_pwr_->Fl_Line_Dial::value());
//				status_->misc_status(ST_OK, message);
//				break;
//			case POWER_WARNING:
//				dial_pwr_->selection_color(fl_color_average(FL_RED, FL_YELLOW, 0.33f));
//				snprintf(message, 200, "DASH: Power %g > %g", dial_pwr_->Fl_Line_Dial::value(), dial_pwr_->alarm1());
//				status_->misc_status(ST_WARNING, message);
//				break;
//			}
//		}
//		if (rig_if_->get_tx()) {
//			dial_pwr_->activate();
//		}
//		else {
//			dial_pwr_->deactivate();
//		}
//
//		// Vdd (PA drain voltage) widget - set colour and raise alarm
//		if (previous_vdd_alarm_ != current_vdd_alarm_) {
//			char message[200];
//			switch (current_vdd_alarm_) {
//			case VDD_UNDER:
//				dial_vdd_->selection_color(FL_RED);
//				snprintf(message, 200, "DASH: Vdd %g < %g", dial_vdd_->Fl_Line_Dial::value(), dial_vdd_->alarm1());
//				status_->misc_status(ST_ERROR, message);
//				break;
//			case VDD_OK:
//				dial_vdd_->selection_color(FL_BLACK);
//				snprintf(message, 200, "DASH: Vdd %g OK", dial_vdd_->Fl_Line_Dial::value());
//				status_->misc_status(ST_OK, message);
//				break;
//			case VDD_OVER:
//				dial_vdd_->selection_color(FL_RED);
//				snprintf(message, 200, "DASH: Vdd %g > %g", dial_vdd_->Fl_Line_Dial::value(), dial_vdd_->alarm2());
//				status_->misc_status(ST_ERROR, message);
//				break;
//			}
//		}
//	}
//	else {
//		alarms_grp_->deactivate();
//	}
//}
//
//// Read the list of bands from the ADIF specification and put them in frequency order
//void qso_manager::order_bands() {
//	// List of bands - in string order of name of the bands (e.g. 10M or 3CM)
//	spec_dataset* band_dataset = spec_data_->dataset("Band");
//	ordered_bands_.clear();
//	for (auto its = band_dataset->data.begin(); its != band_dataset->data.end(); its++) {
//		bool found = false;
//		string band = (*its).first;
//		// Iterator to the generated list - restart each loop at the beginning of this list
//		auto itd = ordered_bands_.begin();
//		// Get the frequency of the band we are adding
//		string freq = (*its).second->at("Lower Freq (MHz)");
//		double s_frequency = stod(freq, nullptr);
//		// Until we have found where to put it or reached the end of the new list
//		while (!found && itd != ordered_bands_.end()) {
//			map<string, string>* band_data = band_dataset->data.at(*itd);
//			// Get the frqruency of the current band in the output list
//			freq = band_data->at("Lower Freq (MHz)");
//			double d_frequency = stod(freq, nullptr);
//			// If this is where we add it (first entry in output listthat is higher than it
//			if (s_frequency < d_frequency) {
//				found = true;
//				ordered_bands_.insert(itd, band);
//			}
//			itd++;
//		}
//		if (!found) {
//			ordered_bands_.push_back(band);
//		}
//	}
//}

//// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
//void qso_manager::populate_model_choice() {
//	Fl_Choice* ch = (Fl_Choice*)rig_model_choice_;
//	// Get hamlib Model number and populate control with all model names
//	ch->clear();
//	// Get set to order the rigs
//	set<string> rig_list;
//	rig_list.clear();
//	char* target_pathname = nullptr;
//	// For each possible rig ids in hamlib
//	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
//	for (rig_model_t i = 1; i < max_rig_num; i += 1) {
//		// Get the capabilities of this rig ID (at ID #xx01)
//		const rig_caps* capabilities = rig_get_caps(i);
//		if (capabilities != nullptr) {
//			// There is a rig - add the model name to that choice
//			// What is the status of the handler for this particular rig?
//			char status[16];
//			switch (capabilities->status) {
//			case RIG_STATUS_ALPHA:
//				strcpy(status, " (Alpha)");
//				break;
//			case RIG_STATUS_UNTESTED:
//				strcpy(status, " (Untested)");
//				break;
//			case RIG_STATUS_BETA:
//				strcpy(status, " (Beta)");
//				break;
//			case RIG_STATUS_STABLE:
//				strcpy(status, "");
//				break;
//			case RIG_STATUS_BUGGY:
//				strcpy(status, " (Buggy)");
//				break;
//			}
//			// Generate the item pathname - e.g. "Icom/IC-736 (untested)"
//			char* temp = new char[strlen(status) + 10 + strlen(capabilities->model_name) + strlen(capabilities->mfg_name)];
//			// The '/' ensures all rigs from same manufacturer are in a sub-menu to Icom
//			string mfg = escape_menu(capabilities->mfg_name);
//			string model = escape_menu(capabilities->model_name);
//			sprintf(temp, "%s/%s%s", mfg.c_str(), model.c_str(), status);
//			rig_list.insert(temp);
//			hamlib_data* hlinfo = &rig_grp_->info().rig_data.hamlib_params;
//			if (strcmp(hlinfo->model.c_str(), (capabilities->model_name)) == 0 &&
//				strcmp(hlinfo->mfr.c_str(), (capabilities->mfg_name)) == 0) {
//				// We are adding the current selected rig, remember it's menu item value and hamlib reference number
//				target_pathname = new char[strlen(temp) + 1];
//				strcpy(target_pathname, temp);
//				hlinfo->model_id = i;
//				hlinfo->port_type = capabilities->port_type;
//			}
//		}
//	}
//	for (auto ix = rig_list.begin(); ix != rig_list.end(); ix++) {
//		ch->add((*ix).c_str());
//	}
//	bool found = false;
//	// Go through all the menu items until we find our remembered pathname, and set the choice value to that item number
//	// We have to do it like this as the choice value when we added it may have changed.
//	for (int i = 0; i < ch->size() && !found && target_pathname; i++) {
//		char item_pathname[128];
//		ch->item_pathname(item_pathname, 127, &ch->menu()[i]);
//		if (strcmp(item_pathname, target_pathname) == 0) {
//			found = true;
//			ch->value(i);
//		}
//	}
//	delete[] target_pathname;
//}
//
//// Model input choice selected
//// v is not used
//void qso_manager::cb_ch_model(Fl_Widget* w, void* v) {
//	Fl_Choice* ch = (Fl_Choice*)w;
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
//	// Get the full item name - e.g. Icom/IC-736 (untested)
//	char path[128];
//	ch->item_pathname(path, sizeof(path) - 1);
//	// Get the manufacturer - i.e. upto the / character
//	char* pos_stroke = strchr(path, '/');
//	info->mfr = string(path, pos_stroke - path);
//	// Get the mode - upto the " (" - returns nullptr if it's not there
//	char* pos_bracket = strstr(pos_stroke + 1, " (");
//	if (pos_bracket == nullptr) {
//		info->model = string(pos_stroke + 1);
//	}
//	else {
//		info->model = string(pos_stroke + 1, pos_bracket - pos_stroke - 1);
//	}
//	// For each possible rig ids in hamlib 
//	bool found = false;
//	for (rig_model_t i = 1; i < 4000 && !found; i += 1) {
//		// Get the capabilities of this rig ID (at ID #xx01)
//		const rig_caps* capabilities = rig_get_caps(i);
//		if (capabilities != nullptr) {
//			if (strcmp(info->model.c_str(), (capabilities->model_name)) == 0 &&
//				strcmp(info->mfr.c_str(), (capabilities->mfg_name)) == 0) {
//				info->model_id = i;
//				info->port_type = capabilities->port_type;
//				found = true;
//			}
//		}
//	}
//	that->populate_baud_choice();
//	that->enable_cat_widgets();
//}
//
//// Callback selecting port
//// v is unused
//void qso_manager::cb_ch_port(Fl_Widget* w, void* v) {
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
//	cb_text<Fl_Choice, string>(w, (void*)&info->port_name);
//}
//
//// Callback entering port
//// v is unused
//void qso_manager::cb_ip_port(Fl_Widget* w, void* v) {
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
//	cb_value<Fl_Input, string>(w, (void*)&info->port_name);
//}
//
//// Callback selecting baud-rate
//// v is unused
//void qso_manager::cb_ch_baud(Fl_Widget* w, void* v) {
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
//	cb_text<Fl_Choice, string>(w, (void*)&info->baud_rate);
//}
//
//// Override rig capabilities selected - repopulate the baud choice
//// v is uused
//void qso_manager::cb_ch_over(Fl_Widget* w, void* v) {
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	hamlib_data* info = &that->rig_grp_->info().rig_data.hamlib_params;
//	cb_value<Fl_Check_Button, bool>(w, (void*)&info->override_caps);
//	that->populate_baud_choice();
//}
//
//// Select "display all ports" in port choice
//// v is a pointer to the all ports flag
//void qso_manager::cb_bn_all(Fl_Widget* w, void* v) {
//	cb_value<Fl_Check_Button, bool>(w, v);
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	that->populate_port_choice();
//}
//
//// Changed the SWR wrning level
//// v is unused
//void qso_manager::cb_alarm_swr(Fl_Widget* w, void* v) {
//	alarm_dial* dial = ancestor_view<alarm_dial>(w);
//	qso_manager* that = ancestor_view<qso_manager>(dial);
//	double val = dial->Fl_Line_Dial::value();
//	double error = dial->alarm2();
//	double warn = dial->alarm1();
//	// If in receive mode, use last value read in TX mode
//	if (!rig_if_->get_tx()) {
//		val = that->last_tx_swr_;
//		dial->Fl_Line_Dial::value(val);
//	}
//	else {
//		that->last_tx_swr_ = val;
//	}
//	// Check against error or warning levels
//	that->previous_swr_alarm_ = that->current_swr_alarm_;
//	if (val > error) {
//		that->current_swr_alarm_ = SWR_ERROR;
//	}
//	else if (val > warn) {
//		that->current_swr_alarm_ = SWR_WARNING;
//	}
//	else {
//		that->current_swr_alarm_ = SWR_OK;
//	}
//	that->enable_widgets();
//}
//
//// Changed the power level
//// v is unused
//void qso_manager::cb_alarm_pwr(Fl_Widget* w, void* v) {
//	alarm_dial* dial = ancestor_view<alarm_dial>(w);
//	qso_manager* that = ancestor_view<qso_manager>(dial);
//	double val = dial->Fl_Line_Dial::value();
//	double warn = dial->alarm1();
//	// If in receive mode, use last value read in TX mode
//	if (!rig_if_->get_tx()) {
//		val = that->last_tx_pwr_;
//		dial->Fl_Line_Dial::value(val);
//	}
//	else {
//		that->last_tx_pwr_ = val;
//	}
//	// Check against warning level
//	that->previous_pwr_alarm_ = that->current_pwr_alarm_;
//	if (val > warn) {
//		that->current_pwr_alarm_ = POWER_WARNING;
//	}
//	else {
//		that->current_pwr_alarm_ = POWER_OK;
//	}
//	that->enable_widgets();
//}
//
//// Changed the drain voltage
//void qso_manager::cb_alarm_vdd(Fl_Widget* w, void* v) {
//	alarm_dial* dial = ancestor_view<alarm_dial>(w);
//	qso_manager* that = ancestor_view<qso_manager>(dial);
//	double val = dial->Fl_Line_Dial::value();
//	double max = dial->alarm2();
//	double min = dial->alarm1();
//	// Check against error levels
//	that->previous_vdd_alarm_ = that->current_vdd_alarm_;
//	if (val > max) {
//		that->current_vdd_alarm_ = VDD_OVER;
//	}
//	else if (val < min) {
//		that->current_vdd_alarm_ = VDD_UNDER;
//	}
//	else {
//		that->current_vdd_alarm_ = VDD_OK;
//	}
//	that->enable_widgets();
//}

//// Changed the fast polling interval
//// v is not used
//void qso_manager::cb_ctr_pollfast(Fl_Widget* w, void* v) {
//	// Get the warning level
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	cb_value<Fl_Spinner, double>(w, &that->rig_grp_->info().rig_data.fast_poll_interval);
//}
//
//// Changed the fast polling interval
//// v is not used
//void qso_manager::cb_ctr_pollslow(Fl_Widget* w, void* v) {
//	// Get the warning level
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	cb_value<Fl_Spinner, double>(w, &that->rig_grp_->info().rig_data.slow_poll_interval);
//}

//// Pressed the connect button
//// v is not used
//void qso_manager::cb_bn_connect(Fl_Widget* w, void* v) {
//	qso_manager* that = ancestor_view<qso_manager>(w);
//	that->save_values();
//	if (rig_if_) {
//	// We are connected - set disconnected
//		delete rig_if_;
//		rig_if_ = nullptr;
//		that->wait_connect_ = true;
//	}
//	else {
//		// Wer are discooencted, so connect
//		add_rig_if();
//		that->wait_connect_ = false;
//	}
//	that->update_rig();
//}

// Close button clicked - check editing or not
// v is not used
void qso_manager::cb_close(Fl_Widget* w, void* v) {
	// It is the window that raised this callback
	qso_manager* that = (qso_manager*)w;
	// If we are editing does the user want to save or cancel?
	if (that->qso_group_->logging_state_ == QSO_EDIT ||
		that->qso_group_->logging_state_ == QSO_STARTED) {
		if (fl_choice("Entering a record - save or cancel?", "Cancel", "Save", nullptr) == 1) {
			qso_group::cb_save(w, v);
		}
		else {
			qso_group::cb_cancel(w, v);
		}
	}
	// Mark qso_manager disabled, hide it and update menu item
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Enabled", (int)false);
	menu_->update_items();
}

//// Populate the choice with the available ports
//void qso_manager::populate_port_choice() {
//	if (rig_grp_->info().rig_data.hamlib_params.port_type == RIG_PORT_SERIAL) {
//		Fl_Choice* ch = (Fl_Choice*)port_if_choice_;
//		ch->clear();
//		ch->add("NONE");
//		ch->value(0);
//		int num_ports = 1;
//		string* existing_ports = new string[1];
//		serial serial;
//		// Get the list of all ports or available (not in use) ports
//		while (!serial.available_ports(num_ports, existing_ports, all_ports_, num_ports)) {
//			delete[] existing_ports;
//			existing_ports = new string[num_ports];
//		}
//		// now for the returned ports
//		for (int i = 0; i < num_ports; i++) {
//			// Add the name onto the choice drop-down list
//			char message[100];
//			const char* port = existing_ports[i].c_str();
//			snprintf(message, sizeof(message), "RIG: Found port %s", port);
//			status_->misc_status(ST_LOG, message);
//			ch->add(port);
//			// Set the value to the list of ports
//			if (strcmp(port, rig_grp_->info().rig_data.hamlib_params.port_name.c_str()) == 0) {
//				ch->value(i);
//			}
//		}
//	}
//}
//
//// Populate the baud rate choice menu
//void qso_manager::populate_baud_choice() {
//	if (rig_grp_->info().rig_data.hamlib_params.port_type == RIG_PORT_SERIAL) {
//		Fl_Choice* ch = (Fl_Choice*)baud_rate_choice_;
//		ch->clear();
//		// Override rig's capabilities?
//		bool override_caps = rig_grp_->info().rig_data.hamlib_params.override_caps;
//		Fl_Button* bn = (Fl_Button*)override_check_;
//		bn->value(override_caps);
//
//		// Get the baud-rates supported by the rig
//		const rig_caps* caps = rig_get_caps(rig_grp_->info().rig_data.hamlib_params.model_id);
//		int min_baud_rate = 300;
//		int max_baud_rate = 460800;
//		if (caps) {
//			min_baud_rate = caps->serial_rate_min;
//			max_baud_rate = caps->serial_rate_max;
//		}
//		// Default baud-rates
//		const int baud_rates[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800 };
//		int num_rates = sizeof(baud_rates) / sizeof(int);
//		int index = 0;
//		ch->value(0);
//		// If no values add an empty value
//		if (num_rates == 0)	ch->add("");
//		// For all possible rates
//		for (int i = 0; i < num_rates; i++) {
//			int rate = baud_rates[i];
//			if (override_caps || (rate >= min_baud_rate && rate <= max_baud_rate)) {
//				// capabilities overridden or within the range supported by capabilities
//				ch->add(to_string(rate).c_str());
//				if (to_string(rate) == rig_grp_->info().rig_data.hamlib_params.baud_rate) {
//					ch->value(index);
//					index++;
//				}
//			}
//		}
//	}
//}

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
	switch (qso_group_->logging_state_) {
	case QSO_PENDING:
	case QSO_INACTIVE:
		return false;
	case QSO_EDIT:
	case QSO_STARTED:
		return true;
	}
	return false;
}

// Called when rig is read to update values here
void qso_manager::rig_update(string frequency, string mode, string power) {
	if (qso_group_->logging_state_ == QSO_PENDING && qso_group_->logging_mode_ == LM_ON_AIR_CAT) {
		qso_group_->copy_cat_to_qso();
	}
}

// Get QSO information from previous record not rig
void qso_manager::update_rig() {
	// Get freq etc from QSO or rig
	// Get present values data from rig
	if (qso_group_->logging_state_ == QSO_PENDING) {
		switch (qso_group_->logging_mode_) {
		case LM_IMPORTED:
		case LM_OFF_AIR:
		case LM_ON_AIR_TIME:
			// Do nothing
			break;
		case LM_ON_AIR_CAT: {
			if (rig_if_) {
				//dial_swr_->value(rig_if_->swr_meter());
				//dial_pwr_->value(rig_if_->pwr_meter());
				//dial_vdd_->value(rig_if_->vdd_meter());
				qso_group_->copy_cat_to_qso();
				if (band_view_) {
					double freq;
					qso_group_->current_qso_->item("FREQ", freq);;
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
			break;
		}
		}
	}
	//enable_use_widgets();
}

// Called whenever another view updates a record (or selects a new one)
void qso_manager::update_qso() {
	record* prev_record = book_->get_record();
	if (qso_in_progress() && prev_record == qso_group_->current_qso_) {
		// Update the view if another view changes the record
		qso_group_->copy_qso_to_display(qso_group::CF_ALL);
	}
	else if (qso_group_->logging_state_ == QSO_PENDING) {
		// Switch to selected record if in QSO_PENDING state
		qso_group_->copy_qso_to_qso(book_->get_record(), qso_group::CF_NOTCLOCK);
	}
}

// Start QSO
void qso_manager::start_qso() {

	switch (qso_group_->logging_state_) {
	case QSO_INACTIVE:
		qso_group_->action_activate();
		// drop through
	case QSO_PENDING:
		qso_group_->action_start();
		break;
	case QSO_STARTED:
		status_->misc_status(ST_ERROR, "Cannot start a QSO when one already started");
		break;
	case QSO_EDIT:
		status_->misc_status(ST_ERROR, "Cannot start a QSO while editing an existing one");
		break;
	}
	//record* new_record = new record;
	//string mode;
	//string submode;
	//string timestamp = now(false, "%Y%m%d%H%M%S");
	//record* copy_from = book_->get_record();

	//switch (qso_group_->logging_mode_) {
	//case LM_IMPORTED:
	//	// No pre-population
	//	break;
	//case LM_OFF_AIR:
	//	// Prepopulate rig, antenna, QTH and callsign
	//	new_record->item("STATION_CALLSIGN", callsign_grp_->name());
	//	new_record->item("MY_RIG", rig_grp_->name());
	//	new_record->item("MY_ANTENNA", antenna_grp_->name());
	//	new_record->item("APP_ZZA_QTH", qth_grp_->name());
	//	break;
	//case LM_ON_AIR_CAT:
	//	// Interactive mode - start QSO - update fields from radio 
	//	// Get current date and time in UTC
	//	new_record->item("QSO_DATE", timestamp.substr(0, 8));
	//	// Time as HHMMSS - always log seconds.
	//	new_record->item("TIME_ON", timestamp.substr(8));
	//	new_record->item("QSO_DATE_OFF", string(""));
	//	new_record->item("TIME_OFF", string(""));
	//	new_record->item("CALL", string(""));
	//	// Get frequency, mode and transmit power from rig
	//	new_record->item("FREQ", rig_if_->get_frequency(true));
	//	if (rig_if_->is_split()) {
	//		new_record->item("FREQ_RX", rig_if_->get_frequency(false));
	//	}
	//	// Get mode - NB USB/LSB need further processing
	//	rig_if_->get_string_mode(mode, submode);
	//	new_record->item("MODE", mode);
	//	new_record->item("SUBMODE", submode);
	//	new_record->item("TX_PWR", rig_if_->get_tx_power());
	//	// initialise fields
	//	new_record->item("RX_PWR", string(""));
	//	new_record->item("RST_SENT", string(""));
	//	new_record->item("RST_RCVD", string(""));
	//	new_record->item("NAME", string(""));
	//	new_record->item("QTH", string(""));
	//	new_record->item("GRIDSQUARE", string(""));
	//	// Prepopulate rig, antenna, QTH and callsign
	//	new_record->item("STATION_CALLSIGN", callsign_grp_->name());
	//	new_record->item("MY_RIG", rig_grp_->name());
	//	new_record->item("MY_ANTENNA", antenna_grp_->name());
	//	new_record->item("APP_ZZA_QTH", qth_grp_->name());
	//	break;
	//case LM_ON_AIR_COPY:
	//	// Interactive mode - start QSO - date/time only
	//	// Get current date and time in UTC
	//	new_record->item("QSO_DATE", timestamp.substr(0, 8));
	//	// Time as HHMMSS - always log seconds.
	//	new_record->item("TIME_ON", timestamp.substr(8));
	//	new_record->item("QSO_DATE_OFF", string(""));
	//	new_record->item("TIME_OFF", string(""));
	//	new_record->item("CALL", string(""));
	//	// otherwise leave blank so that we enter it manually later.
	//	new_record->item("FREQ", copy_from->item("FREQ"));
	//	new_record->item("FREQ_RX", copy_from->item("FREQ_RX"));
	//	new_record->item("MODE", copy_from->item("MODE"));
	//	new_record->item("SUBMODE", copy_from->item("SUBMODE"));
	//	new_record->item("TX_PWR", copy_from->item("TX_PWR"));
	//	// initialise fields
	//	new_record->item("RX_PWR", string(""));
	//	new_record->item("RST_SENT", string(""));
	//	new_record->item("RST_RCVD", string(""));
	//	new_record->item("NAME", string(""));
	//	new_record->item("QTH", string(""));
	//	new_record->item("GRIDSQUARE", string(""));
	//	// Prepopulate rig, antenna, QTH and callsign
	//	new_record->item("STATION_CALLSIGN", callsign_grp_->name());
	//	new_record->item("MY_RIG", rig_grp_->name());
	//	new_record->item("MY_ANTENNA", antenna_grp_->name());
	//	new_record->item("APP_ZZA_QTH", qth_grp_->name());
	//	break;
	//case LM_ON_AIR_TIME:
	//	// Interactive mode - start QSO - date/time only
	//	// Get current date and time in UTC
	//	new_record->item("QSO_DATE", timestamp.substr(0, 8));
	//	// Time as HHMMSS - always log seconds.
	//	new_record->item("TIME_ON", timestamp.substr(8));
	//	new_record->item("QSO_DATE_OFF", string(""));
	//	new_record->item("TIME_OFF", string(""));
	//	new_record->item("CALL", string(""));
	//	// otherwise leave blank so that we enter it manually later.
	//	new_record->item("FREQ", string(""));
	//	new_record->item("FREQ_RX", string(""));
	//	new_record->item("MODE", string(""));
	//	new_record->item("SUBMODE", string(""));
	//	new_record->item("TX_PWR", string(""));
	//	// initialise fields
	//	new_record->item("RX_PWR", string(""));
	//	new_record->item("RST_SENT", string(""));
	//	new_record->item("RST_RCVD", string(""));
	//	new_record->item("NAME", string(""));
	//	new_record->item("QTH", string(""));
	//	new_record->item("GRIDSQUARE", string(""));
	//	// Prepopulate rig, antenna, QTH and callsign
	//	new_record->item("STATION_CALLSIGN", callsign_grp_->name());
	//	new_record->item("MY_RIG", rig_grp_->name());
	//	new_record->item("MY_ANTENNA", antenna_grp_->name());
	//	new_record->item("APP_ZZA_QTH", qth_grp_->name());
	//	break;
	//}

}

// End QSO - add time off
// TODO: Can be called without current_qso_ - needs to be set by something.
void qso_manager::end_qso() {
	switch (qso_group_->logging_state_) {
	case QSO_INACTIVE:
		status_->misc_status(ST_ERROR, "Cannot end a QSO that hasn't been started");
		break;
	case QSO_PENDING:
		qso_group_->action_start();
		// drop through
	case QSO_STARTED:
		qso_group_->action_save();
		break;
	case QSO_EDIT:
		status_->misc_status(ST_ERROR, "CAnnot end a QSO while editing an existing one");
		break;
	}

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

// Copy the sleected QSOs MY_RIG etc to the supplied qso record
void qso_manager::update_import_qso(record* import_qso) {
	record* use_qso;
	string mode = import_qso->item("MODE");
	string submode = import_qso->item("SUBMODE");
	// If we have activated the QSO manager, then use active QSO as template
	if (qso_group_->current_qso_) use_qso = qso_group_->current_qso_;
	else {
		// Otherwise look for the most recent QSO using the same mode
		record_num_t record_num = book_->size() - 1;
		use_qso = book_->get_record(record_num, false);
		bool found = false;
		while (use_qso && !found && record_num > 0) {
			if (use_qso->item("MODE") == mode && use_qso->item("SUBMODE") == submode) {
				found = true;
			}
			record_num--;
		}
		if (!found) {
			// Fallback on selected QSO
			use_qso = book_->get_record();
		}
	}
	// If we have a QSO to copy from and one to copy to, copy these fields.
	if (use_qso && import_qso) {
		char message[128];
		snprintf(message, 128, "IMPORT: Copying station data from %s %s %s %s",
			use_qso->item("QSO_DATE").c_str(),
			use_qso->item("TIME_ON").c_str(),
			use_qso->item("CALL").c_str(),
			use_qso->item("MODE").c_str());
		status_->misc_status(ST_LOG, message);
		import_qso->item("MY_RIG", use_qso->item("MY_RIG"));
		import_qso->item("MY_ANTENNA", use_qso->item("MY_ANTENNA"));
		import_qso->item("STATION_CALLSIGN", use_qso->item("STATION_CALLSIGN"));
		import_qso->item("APP_ZZA_QTH", use_qso->item("APP_ZZA_QTH"));
	}
}

// Get the default value of the station item
string qso_manager::get_default(stn_item_t item) {
	record* source = qso_group_->get_default_record();
	if (source) {
		switch (item) {
		case RIG:
			return source->item("MY_RIG");
		case ANTENNA:
			return source->item("MY_ANTENNA");
		case CALLSIGN:
			return source->item("STATION_CALLSIGN");
		case QTH:
			return source->item("APP_ZZA_QTH");
		default:
			return "";
		}
	}
	else {
		return"";
	}
}
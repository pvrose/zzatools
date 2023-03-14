#include "qso_manager.h"
#include "callback.h"
#include "rig_if.h"
#include "serial.h"

#include "record.h"
#include "pfx_data.h"
#include "prefix.h"
#include "status.h"
#include "tabbed_forms.h"
#include "intl_widgets.h"
#include "spec_data.h"
#include "book.h"
#include "extract_data.h"
#include "import_data.h"
#include "menu.h"
#include "field_choice.h"
#ifdef _WIN32
#include "dxa_if.h"
#endif
#include "band_view.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "alarm_dial.h"
#include "qth_dialog.h"
#include "utils.h"
#include "qsl_viewer.h"

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




// External declarations
extern Fl_Preferences* settings_;
extern pfx_data* pfx_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern rig_if* rig_if_;
extern spec_data* spec_data_;
extern book* book_;
extern extract_data* extract_records_;
extern import_data* import_data_;
extern menu* menu_;
#ifdef _WIN32
extern dxa_if* dxa_if_;
#endif
extern band_view* band_view_;
extern tabbed_forms* tabbed_forms_;
extern import_data* import_data_;
void add_rig_if();
extern double prev_freq_;
extern bool read_only_;

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
			snprintf(message, sizeof(message), "DASH: Found port %s", port);
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
	qsl_viewer_ = new qsl_viewer(10, 10);
	qsl_viewer_->callback(cb_qsl_viewer);
	qsl_viewer_->hide();
	// Initialise field input map
	field_ip_map_.clear();

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

// Create contest group
Fl_Group* qso_manager::qso_group::create_contest_group(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

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

	int max_w = curr_x + ch_contest_id_->w() + GAP - x();
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
	bn_define_tx_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "TX");
	bn_define_tx_->labelfont(FONT);
	bn_define_tx_->labelsize(FONT_SIZE);
	bn_define_tx_->callback(cb_def_format, (void*)true);
	bn_define_tx_->tooltip("Use the specified fields as contest exchange on transmit");

	curr_x += bn_define_tx_->w();

	// Define contest
	bn_define_rx_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "RX");
	bn_define_rx_->labelfont(FONT);
	bn_define_rx_->labelsize(FONT_SIZE);
	bn_define_rx_->callback(cb_def_format, (void*)false);
	bn_define_rx_->tooltip("Use the specified fields as contest exchange on receive");

	curr_x += bn_define_rx_->w() + GAP;

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

	return g_contest;
}

Fl_Group* qso_manager::qso_group::create_entry_group(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	int col2_y = curr_y;
	int max_w = 0;

	Fl_Group* g = new Fl_Group(curr_x, curr_y, 0, 0, "Entry");
	g->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g->box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += GAP;
	// Fixed fields
	// N rows of NUMBER_PER_ROW
	const int NUMBER_PER_ROW = 2;
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
		ip_field_[ix]->input()->when(FL_WHEN_RELEASE_ALWAYS);
		if (ix < NUMBER_FIXED) {
			ip_field_[ix]->field_name(fixed_names_[ix].c_str(), current_qso_);
			field_ip_map_[fixed_names_[ix]] = ix;
			field_names_[ix] = fixed_names_[ix];
			ip_field_[ix]->label(fixed_names_[ix].c_str());
		}
		else {
			field_names_[ix] = "";
		}
		number_fields_in_use_ = NUMBER_FIXED;
		curr_x += WINPUT + GAP;
		if (ix % NUMBER_PER_ROW == (NUMBER_PER_ROW - 1)) {
			max_w = max(max_w, curr_x - x());
			curr_x = X + GAP;
			curr_y += HBUTTON;
		}
	}


	max_w = max(max_w, curr_x);

	// nOtes input
	curr_x = X + WCHOICE;
	curr_y += HBUTTON;

	ip_notes_ = new intl_input(curr_x, curr_y, max_w - curr_x, HBUTTON, "NOTES");
	ip_notes_->labelfont(FONT);
	ip_notes_->labelsize(FONT_SIZE);
	ip_notes_->textfont(FONT);
	ip_notes_->textsize(FONT_SIZE);
	ip_notes_->callback(cb_ip_notes, nullptr);
	ip_notes_->tooltip("Add any notes for the QSO");

	curr_y += HBUTTON + GAP;
	g->resizable(nullptr);
	g->size(max_w, curr_y - Y);
	g->end();

	return g;
}

// Query table
Fl_Group* qso_manager::qso_group::create_query_group(int X, int Y) {

	int curr_x = X;
	int curr_y = Y;

	Fl_Group* g = new Fl_Group(curr_x, curr_y, 0, 0, "Query");
	g->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g->box(FL_BORDER_BOX);

	const int WTABLE = 420;
	const int HTABLE = 250;

	curr_x += GAP;
	curr_y += HTEXT;

	tab_query_ = new record_table(curr_x, curr_y, WTABLE, HTABLE, "Query Message goes here");
	tab_query_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	tab_query_->callback(cb_tab_qso, nullptr);

	curr_x += WTABLE + GAP;
	curr_y += HTABLE + GAP;

	g->resizable(nullptr);
	g->size(curr_x - X, curr_y - Y);
	g->end();

	return g;
}

// QSO control group
Fl_Group* qso_manager::qso_group::create_button_group(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	Fl_Group* g = new Fl_Group(curr_x, curr_y, 0, 0, "Controls");
	g->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g->box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;
	int max_x = curr_x;

	const int NUMBER_PER_ROW = 8;
	for (int ix = 0; ix < MAX_ACTIONS; ix++) {
		bn_action_[ix] = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "");
		if ((ix + 1) % NUMBER_PER_ROW == 0 && ix < MAX_ACTIONS) {
			curr_x += WBUTTON;
			max_x = max(max_x, curr_x);
			curr_x = X + GAP;
			curr_y += HBUTTON;
		}
		else {
			curr_x += WBUTTON;
			max_x = max(max_x, curr_x);
		}
	}

	max_x += GAP;
	curr_y += GAP;
	g->resizable(nullptr);
	g->size(max_x - X, curr_y - Y);
	g->end();

	return g;
}

// Create qso_group
void qso_manager::qso_group::create_form(int X, int Y) {
	int max_w = 0;

	begin();
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	label("QSO Data");

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
	ch_logmode_->add("Current date and time, data from selected QSO - including call");
	ch_logmode_->add("Current date and time, data from selected QSO - excluding call");
	ch_logmode_->add("Current date and time, no other data");
	ch_logmode_->value(logging_mode_);
	ch_logmode_->callback(cb_logging_mode, &logging_mode_);
	ch_logmode_->tooltip("Select the logging mode - i.e. how to initialise a new QSO record");

	int curr_y = ch_logmode_->y() + ch_logmode_->h() + GAP;;
	int top = ch_logmode_->y() + ch_logmode_->h();
	int curr_x = X + GAP;

	g_contest_ = create_contest_group(curr_x, curr_y);

	max_w = max(max_w, g_contest_->x() + g_contest_->w() + GAP);
	curr_y = g_contest_->y() + g_contest_->h() + GAP;

	// One or the other of the two groups below will be shown at a time
	g_entry_ = create_entry_group(curr_x, curr_y);
	max_w = max(max_w, g_entry_->x() + g_entry_->w() + GAP);
	g_query_ = create_query_group(curr_x, curr_y);
	max_w = max(max_w, g_query_->x() + g_query_->w() + GAP);
	curr_y = max(g_entry_->y() + g_entry_->h(), g_query_->y() + g_query_->h()) + GAP;

	g_buttons_ = create_button_group(curr_x, curr_y);
	max_w = max(max_w, g_buttons_->x() + g_buttons_->w() + GAP);
	curr_y += g_buttons_->h() + GAP;

	ch_logmode_->size(max_w - ch_logmode_->x() - GAP, ch_logmode_->h());

	resizable(nullptr);
	size(max_w, curr_y - Y);
	end();

	initialise_fields();
}

void qso_manager::qso_group::enable_contest_widgets() {
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
	switch (field_mode_) {
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

}

// Enable field widgets
void qso_manager::qso_group::enable_entry_widgets() {
	// Now enable disan=
	switch (logging_state_) {
	case QSO_INACTIVE:
		g_entry_->show();
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->deactivate();
		break;
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_EDIT:
		g_entry_->show();
		for (int ix = 0; ix <= number_fields_in_use_ && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->activate();
			ip_field_[ix]->activate();
		}
		for (int ix = number_fields_in_use_ + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->activate();
		break;
	default:
		// Reserver=d for Query states
		g_entry_->hide();
		break;
	}
}

// Enable query widgets
void qso_manager::qso_group::enable_query_widgets() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_EDIT:
		g_query_->hide();
		break;
	case QSO_BROWSE:
	default:
		g_query_->show();
		tab_query_->activate();
		break;
	}
}

// Enable action buttons
void qso_manager::qso_group::enable_button_widgets() {
	const list<button_type>& buttons = button_map_.at(logging_state_);
	int ix = 0;
	for (auto bn = buttons.begin(); bn != buttons.end() && ix < MAX_ACTIONS; bn++, ix++) {
		const button_action& action = action_map_.at(*bn);
		bn_action_[ix]->label(action.label);
		bn_action_[ix]->tooltip(action.label);
		bn_action_[ix]->color(action.colour);
		bn_action_[ix]->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, action.colour));
		bn_action_[ix]->callback(action.callback, action.userdata);
		bn_action_[ix]->activate();
	}
	for (; ix < MAX_ACTIONS; ix++) {
		bn_action_[ix]->label("");
		bn_action_[ix]->tooltip("");
		bn_action_[ix]->color(FL_BACKGROUND_COLOR);
		bn_action_[ix]->callback((Fl_Callback*)nullptr);
		bn_action_[ix]->deactivate();
	}
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

	enable_contest_widgets();
	enable_entry_widgets();
	enable_query_widgets();
	enable_button_widgets();

}

// Update specific station item input
void qso_manager::qso_group::update_station_choices(stn_item_t station_item) {
	// Note can update multiple items as the flags are bit wise
	if (station_item & RIG) {
		ip_field_[field_ip_map_["MY_RIG"]]->reload_choice();
	}
	if (station_item & ANTENNA) {
		ip_field_[field_ip_map_["MY_ANTENNA"]]->reload_choice();
	}
	if (station_item & QTH) {
		ip_field_[field_ip_map_["APP_ZZA_QTH"]]->reload_choice();
	}
	if (station_item & CALLSIGN) {
		ip_field_[field_ip_map_["STATION_CALLSIGN"]]->reload_choice();
	}
	redraw();
}

// Update QSO
void qso_manager::qso_group::update_qso(record_num_t log_qso) {
	if (log_qso == current_rec_num_) {
		if (logging_state_ == QSO_STARTED ||
			logging_state_ == QSO_EDIT) {
			// Update the view if another view changes the record
			copy_qso_to_display(qso_group::CF_ALL_FLAGS);
		}
	}
	else if (logging_state_ == QSO_PENDING) {
		// Switch to selected record if in QSO_PENDING state
		current_rec_num_ = log_qso;
		copy_qso_to_qso(book_->get_record(current_rec_num_, false), qso_group::CF_RIG_ETC | qso_group::CF_CAT);
		action_view_qsl();
	}
	else if (logging_state_ == QSO_EDIT) {
		// Selecting a new record - save the old one
		action_save_edit();
		action_edit();
		action_view_qsl();
	}
}

// Update query
void qso_manager::qso_group::update_query(logging_state_t query, record_num_t match_num, record_num_t query_num) {
	if (logging_state_ == QSO_PENDING || logging_state_ == QSO_INACTIVE) {
		current_rec_num_ = match_num;
		query_rec_num_ = query_num;
		action_query(query);
	}
	else {
		// TODO - tidy up befor actioning
	}
}

// Copy record to the fields - reverse of above
void qso_manager::qso_group::copy_qso_to_display(int flags) {
	record* source = get_default_record();
	if (source) {
		// For each field input
		for (int i = 0; i < NUMBER_TOTAL; i++) {
			string field;
			if (i < NUMBER_FIXED) field = fixed_names_[i];
			else field = ch_field_[i]->value();
			if (field.length()) {
				if (flags == CF_ALL_FLAGS) {
					// Copy all fields that have edit fields defined
					ip_field_[i]->value(source->item(field, false, true).c_str());
				}
				else {
					// Copy per flag bits
					for (auto sf = COPY_SET.begin(); sf != COPY_SET.end(); sf++) {
						copy_flags f = (*sf);
						if (flags & f) {
							for (auto fx = COPY_FIELDS.at(f).begin(); fx != COPY_FIELDS.at(f).end(); fx++) {
								if ((*fx) == field)	ip_field_[i]->value(source->item(field, false, true).c_str());
							}
						}
					}
				}
			}
		}
		ip_notes_->value(source->item("NOTES").c_str());
		// If QTH changes tell DXA-IF to update home_location
		check_qth_changed();
	}
}

// Copy from an existing record: fields depend on flags set
void qso_manager::qso_group::copy_qso_to_qso(record* old_record, int flags) {
	if (current_qso_ && old_record) {
		// For all flag bits
		for (auto sf = COPY_SET.begin(); sf != COPY_SET.end(); sf++) {
			copy_flags f = (*sf);
			for (auto fx = COPY_FIELDS.at(f).begin(); fx != COPY_FIELDS.at(f).end(); fx++) {
				if (flags & f) {
					// If it's set copy it
					current_qso_->item((*fx), old_record->item((*fx)));
				}
				else {
					// else clear it
					current_qso_->item(string(""));
				}
			}
		}
		copy_qso_to_display(flags);
	}
}

// Copy fields from CAT and default rig etc.
void qso_manager::qso_group::copy_cat_to_qso() {
	string freqy = rig_if_->get_frequency(true);
	string mode;
	string submode;
	rig_if_->get_string_mode(mode, submode);
	string tx_power = rig_if_->get_tx_power();
	switch (logging_state_) {
	case QSO_PENDING: {
		// Load values from rig
		current_qso_->item("FREQ", freqy);
		// Get mode - NB USB/LSB need further processing
		if (mode != "DATA L" && mode != "DATA U") {
			current_qso_->item("MODE", mode);
			current_qso_->item("SUBMODE", submode);
		}
		else {
			current_qso_->item("MODE", string(""));
			current_qso_->item("SUBMODE", string(""));
		}
		current_qso_->item("TX_PWR", tx_power);
		break;
	}
	case QSO_STARTED: {
		// Ignore values except TX_PWR which accumulates maximum value
		char message[128];
		if (current_qso_->item("FREQ") != freqy) {
			snprintf(message, 128, "DASH: Rig frequency changed during QSO, New value %s ignored", freqy.c_str());
			status_->misc_status(ST_WARNING, message);
		}
		if (current_qso_->item("MODE") != mode) {
			snprintf(message, 128, "DASH: Rig mode changed during QSO, New value %s ignored", mode.c_str());
			status_->misc_status(ST_WARNING, message);
		}
		if (current_qso_->item("SUBMODE") != submode) {
			snprintf(message, 128, "DASH: Rig submode changed during QSO, New value %s ignored", submode.c_str());
			status_->misc_status(ST_WARNING, message);
		}
		double current_power = 0.0;
		current_qso_->item("TX_PWR", current_power);
		double rig_power = atof(tx_power.c_str());
		current_qso_->item("TX_PWR", to_string(max(current_power, rig_power)));
		break;
	}
	}
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

		copy_qso_to_display(CF_TIME);
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
	copy_qso_to_display(CF_QSO);
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
	case QSO_BROWSE:
		that->action_cancel_browse();
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
	case QSO_BROWSE:
		that->action_cancel_browse();
		that->action_edit();
		break;
	}
}

// Callback - Worked B4? button
void qso_manager::qso_group::cb_wkb4(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	extract_records_->extract_call(string(that->ip_field_[that->field_ip_map_["CALL"]]->value()));
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
	tip_record->item("CALL", string(that->ip_field_[that->field_ip_map_["CALL"]]->value()));
	// Parse the temporary record
	message = pfx_data_->get_tip(tip_record);
	// Create a tooltip window at the parse button (in w) X and Y
	Fl_Window* qw = ancestor_view<qso_manager>(w);
	Fl_Window* tw = ::tip_window(message, qw->x_root() + w->x() + w->w() / 2, qw->y_root() + w->y() + w->h() / 2);
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
	delete tip_record;
}

// Callback - view QSL button
void qso_manager::qso_group::cb_bn_view_qsl(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	record* qso = that->current_qso_;
	qsl_viewer* qsl = that->qsl_viewer_;
	that->action_view_qsl();
	if (qso) qsl->show();
}

// Callback - QSL viewer "closing" - make it hide instead
void qso_manager::qso_group::cb_qsl_viewer(Fl_Widget* w, void* v) {
	qsl_viewer* qsl = (qsl_viewer*)w;
	if (qsl->visible()) qsl->hide();
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

// CAllback - navigate buttons
// v - direction
void qso_manager::qso_group::cb_bn_navigate(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	navigate_t target = (navigate_t)(long)v;
	that->action_navigate(target);
}

// Callback - browse
void qso_manager::qso_group::cb_bn_browse(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QSO_INACTIVE:
		that->action_browse();
		break;
	default:
		break;
	}
}

// Callback - add query record
void qso_manager::qso_group::cb_bn_add_query(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QUERY_MATCH:
	case QUERY_NEW:
		that->action_add_query();
		break;
	default:
		break;
	}
}

// Callback - add query record
void qso_manager::qso_group::cb_bn_reject_query(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QUERY_MATCH:
	case QUERY_NEW:
		that->action_reject_query();
		break;
	default:
		break;
	}
}

// Callback - add query record
void qso_manager::qso_group::cb_bn_merge_query(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QUERY_MATCH:
	case QUERY_NEW:
		that->action_merge_query();
		break;
	default:
		break;
	}
}

// Callback - add query record
void qso_manager::qso_group::cb_bn_find_match(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	switch (that->logging_state_) {
	case QUERY_NEW:
		that->action_find_match();
		break;
	default:
		break;
	}
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
	const char* field = ch->value();
	int ix = (int)(long)v;
	if (strlen(field)) {
		that->action_add_field(ix, field);
	}
	else {
		that->action_del_field(ix);
	}
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

// Callback - table
void qso_manager::qso_group::cb_tab_qso(Fl_Widget* w, void* v) {
	qso_group* that = ancestor_view<qso_group>(w);
	record_table* table = (record_table*)w;
	int row = table->callback_row();
	int col = table->callback_col();
	int button = Fl::event_button();
	bool double_click = Fl::event_clicks();
	string field = table->field(row);
	switch (table->callback_context()) {
	case Fl_Table::CONTEXT_ROW_HEADER:
		if (button == FL_LEFT_MOUSE && double_click) {
			that->action_add_field(that->number_fields_in_use_, field);
		}
		break;
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
	int iy;
	for (ix = 0, iy = NUMBER_FIXED; ix < field_names.size(); ix++, iy++) {
		if (new_fields) {
			ch_field_[iy]->value(field_names[ix].c_str());
			ip_field_[iy]->field_name(field_names[ix].c_str(), current_qso_);
			field_names_[iy] = field_names[ix];
		}
		if (lock_preset) {
			ch_field_[iy]->deactivate();
		}
		else {
			ch_field_[iy]->activate();
		}
	}
	number_fields_in_use_ = NUMBER_FIXED + field_names.size();
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
		ip_field_[field_ip_map_["CALL"]]->value("");
		ip_notes_->value("");
		for (size_t i = NUMBER_FIXED + tx_fields.size(); i < NUMBER_TOTAL; i++) {
			ip_field_[i]->value("");
		}
	}
	else {
		ip_field_[field_ip_map_["CALL"]]->value("");
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
			current_rec_num_ = book_->size() - 1;
			return book_->get_latest();
		case LM_ON_AIR_COPY:
		case LM_ON_AIR_CLONE:
			current_rec_num_ = book_->selection();
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
#ifdef _WIN32
			dxa_if_->update(HT_LOCATION);
#endif
		}
	}
}

// Action ACTIVATE: transition from QSO_INACTIVE to QSO_PENDING
void qso_manager::qso_group::action_activate() {
	if (logging_state_ != QSO_INACTIVE || current_qso_ != nullptr) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to activate a QSO when one is already active");
		return;
	}
	current_qso_ = new record;
	time_t now = time(nullptr);
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	record* source_record = get_default_record();
	switch (logging_mode_) {
	case LM_OFF_AIR:
		// Just copy the station details
		copy_qso_to_qso(source_record, CF_RIG_ETC);
		break;
	case LM_ON_AIR_CAT:
		// Copy station details and get read rig details for band etc.
		copy_qso_to_qso(source_record, CF_RIG_ETC);
		copy_cat_to_qso();
		copy_clock_to_qso(now);
		break;
	case LM_ON_AIR_CLONE:
		// Clone the QSO - get station and band from original QSO
		copy_qso_to_qso(source_record, CF_RIG_ETC | CF_CAT);
		copy_clock_to_qso(now);
		break;
	case LM_ON_AIR_COPY:
		// Copy the QSO - as abobe but also same callsign and details
		copy_qso_to_qso(source_record, CF_RIG_ETC | CF_CAT | CF_CONTACT);
		copy_clock_to_qso(now);
		break;
	case LM_ON_AIR_TIME:
		// Copy the station details and set the current date/time.
		copy_qso_to_qso(source_record, CF_RIG_ETC);
		copy_clock_to_qso(now);
		break;
	}
	logging_state_ = QSO_PENDING;
	enable_widgets();
}

// Action START - transition from QSO_PENDING to QSO_STARTED
void qso_manager::qso_group::action_start() {
	if (logging_state_ != QSO_PENDING || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to start a QSO while not pending");
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
		status_->misc_status(ST_SEVERE, "DASH: Attempting to save a QSO when not inputing one");
		return;
	}
	if (current_qso_ != book_->get_record()) {
		status_->misc_status(ST_SEVERE, "DASH: Mismatch between selected QSO in book and QSO manager");
		return;
	}
	bool old_save_enabled = book_->save_enabled();
	record_num_t item_number = book_->item_number(current_rec_num_);
	book_->enable_save(false);
	// On-air logging add date/time off
	switch (logging_mode_) {
	case LM_ON_AIR_CAT:
	case LM_ON_AIR_COPY:
	case LM_ON_AIR_CLONE:
	case LM_ON_AIR_TIME:
		// All on-air modes - set cime-off to now
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
		status_->misc_status(ST_SEVERE, "DASH: Attempting to save a QSO when not inputing one");
		return;
	}
	if (current_qso_ != book_->get_record()) {
		status_->misc_status(ST_SEVERE, "DASH: Mismatch between selected QSO in book and QSO manager");
		return;
	}
	book_->delete_record(true);
	delete current_qso_;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	check_qth_changed();
	enable_widgets();
	qsl_viewer_->hide();
}

// Action DEACTIVATE - Transition from QSO_PENDING to QSO_INACTIVE
void qso_manager::qso_group::action_deactivate() {
	if (logging_state_ != QSO_PENDING && logging_state_ != QSO_BROWSE || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to deactivate when not pending");
	}
	delete current_qso_;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action EDIT - Transition from QSO_INACTIVE to QSO_EDIT
void qso_manager::qso_group::action_edit() {
	if (logging_state_ != QSO_INACTIVE || original_qso_ != nullptr ) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to edit a QSO while inputing a QSO");
		return;
	}
	// Edit currently selected QSO
	current_qso_ = book_->get_record(current_rec_num_, false);
	// And save a copy of it
	original_qso_ = new record(*current_qso_);
	logging_state_ = QSO_EDIT;
	copy_qso_to_display(CF_ALL_FLAGS);
	enable_widgets();
}

// Action SAVE EDIT - Transition from QSO_EDIT to QSO_INACTIVE while saving changes
void qso_manager::qso_group::action_save_edit() {
	if (logging_state_ != QSO_EDIT || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to save a QSO when not editing one");
		return;
	}
	if (current_rec_num_ != book_->selection()) {
		status_->misc_status(ST_WARNING, "DASH: Mismatch between selected QSO in book and QSO manager");
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
		status_->misc_status(ST_SEVERE, "DASH: Attempting to save a QSO when not editing one");
		return;
	}
	if (current_rec_num_ != book_->selection()) {
		status_->misc_status(ST_SEVERE, "DASH: Mismatch between selected QSO in book and QSO manager");
		return;
	}
	// Copy original back to the book
	*book_->get_record() = *original_qso_;
	delete original_qso_;
	original_qso_ = nullptr;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	copy_qso_to_display(CF_ALL_FLAGS);
	enable_widgets();
	qsl_viewer_->hide();
}

// Action CANCEL in BROWSE 
void qso_manager::qso_group::action_cancel_browse() {
	if (logging_state_ != QSO_BROWSE || current_qso_ == nullptr) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to save a QSO when not editing one");
		return;
	}
	if (current_rec_num_ != book_->selection()) {
		status_->misc_status(ST_SEVERE, "DASH: Mismatch between selected QSO in book and QSO manager");
		return;
	}
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	copy_qso_to_display(CF_ALL_FLAGS);
	enable_widgets();
	qsl_viewer_->hide();
}

// Action navigate button
void qso_manager::qso_group::action_navigate(int target) {
	logging_state_t saved_state = logging_state_;
	switch (logging_state_) {
	case QSO_EDIT:
		action_save_edit();
		break;
	case QSO_PENDING:
		action_deactivate();
		break;
	case QSO_BROWSE:
		action_cancel_browse();
		break;
	}
	// We should now be inactive - navigate to new QSO
	book_->navigate((navigate_t)target);
	// And restore state
	switch (saved_state) {
	case QSO_EDIT:
		action_edit();
		action_view_qsl();
		break;
	case QSO_PENDING:
		action_activate();
		action_view_qsl();
		break;
	case QSO_BROWSE:
		action_browse();
		action_view_qsl();
		break;
	}
}

// Action view qsl
void qso_manager::qso_group::action_view_qsl() {
	switch(logging_state_) {
	case QSO_EDIT:
	case QSO_BROWSE:
		if (current_qso_) {
			char title[128];
			snprintf(title, 128, "QSL Status: %s %s %s %s %s",
				current_qso_->item("CALL").c_str(),
				current_qso_->item("QSO_DATE").c_str(),
				current_qso_->item("TIME_ON").c_str(),
				current_qso_->item("BAND").c_str(),
				current_qso_->item("MODE", true, true).c_str());
			qsl_viewer_->copy_label(title);
			qsl_viewer_->set_qso(current_qso_, current_rec_num_);
			char msg[128];
			snprintf(msg, 128, "DASH: %s", title);
			status_->misc_status(ST_LOG, msg);
		}
		break;
	default:
		status_->misc_status(ST_SEVERE, "DASH: No QSO selected - cannot view QSL status");
		break;
	}
}

// Action add field - add the selected field to the set of entries
void qso_manager::qso_group::action_add_field(int ix, string field) {
	if (ix == NUMBER_TOTAL) {
		char msg[128];
		snprintf(msg, 128, "DASH: Cannot add any more fields to edit - %s ignored", field.c_str());
		status_->misc_status(ST_ERROR, msg);
		return;
	}
	if (ix < number_fields_in_use_) {
		const char* old_field = field_names_[ix].c_str();
		ip_field_[ix]->field_name(field.c_str(), current_qso_);
		ip_field_[ix]->value(current_qso_->item(field).c_str());
		// Change mapping
		if (strlen(old_field)) {
			field_ip_map_.erase(old_field);
		}
		field_ip_map_[field] = ix;
		field_names_[ix] = field;
	}
	else if (ix == number_fields_in_use_) {
		if (field_ip_map_.find(field) == field_ip_map_.end()) {
			number_fields_in_use_++;
			ch_field_[ix]->value(field.c_str());
			ip_field_[ix]->field_name(field.c_str(), current_qso_);
			ip_field_[ix]->value(current_qso_->item(field).c_str());
			field_ip_map_[field] = ix;
			field_names_[ix] = field;
		}
	}
	else {
		status_->misc_status(ST_SEVERE, "DASH: Trying to select a deactivated widget");
	}
	enable_entry_widgets();
}

// Delete a field
void qso_manager::qso_group::action_del_field(int ix) {
	string& old_field = field_names_[ix];
	field_ip_map_.erase(old_field);
	int pos = ix;
	for (; pos < number_fields_in_use_ - 1; pos++) {
		string& field = field_names_[pos + 1];
		field_names_[pos] = field;
		ch_field_[pos]->value(field.c_str());
		ip_field_[pos]->field_name(field.c_str(), current_qso_);
		ip_field_[pos]->value(current_qso_->item(field).c_str());
	}
	ch_field_[pos]->value("");
	ip_field_[pos]->field_name("");
	ip_field_[pos]->value("");
	number_fields_in_use_--;

	enable_entry_widgets();
}

// Action browse
void qso_manager::qso_group::action_browse() {
	if (logging_state_ != QSO_INACTIVE || original_qso_ != nullptr) {
		status_->misc_status(ST_SEVERE, "DASH: Attempting to browse a QSO while inputing a QSO");
		return;
	}
	// Copy current selection
	current_rec_num_ = book_->selection();
	// Edit currently selected QSO
	current_qso_ = book_->get_record();
	logging_state_ = QSO_BROWSE;
	tab_query_->set_records(current_qso_, nullptr, nullptr);
	tab_query_->label("Browsing record...");
	enable_widgets();
}

// Action query
void qso_manager::qso_group::action_query(logging_state_t query) {
	switch (query) {
	case QUERY_MATCH:
		current_qso_ = book_->get_record(current_rec_num_, false);
		// And save a copy of it
		original_qso_ = new record(*current_qso_);
		query_qso_ = import_data_->get_record(query_rec_num_, false);
		break;
	case QUERY_NEW:
		current_qso_ = book_->get_record(current_rec_num_, false);
		original_qso_ = nullptr;
		query_qso_ = nullptr;
		break;
	case QUERY_DUPE:
		current_qso_ = book_->get_record(current_rec_num_, false);
		original_qso_ = new record(*current_qso_);
		query_qso_ = book_->get_record(query_rec_num_, false);
		break;
	}
	tab_query_->set_records(current_qso_, query_qso_, original_qso_);
	tab_query_->copy_label(import_data_->match_question().c_str());
	enable_widgets();
}

// Action add query - add query QSO to book
void qso_manager::qso_group::action_add_query() {
	import_data_->save_update();
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
}

// Action reject query - do nothing
void qso_manager::qso_group::action_reject_query() {
	import_data_->discard_update(true);
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
}

// Action merge query
void qso_manager::qso_group::action_merge_query() {
	import_data_->merge_update();
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
}

// ACtion find match
void qso_manager::qso_group::action_find_match() {
	update_query(QUERY_MATCH, current_rec_num_, query_rec_num_);
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
	update_qso(HT_SELECTED, book_->selection(), -1);

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

// Load values
void qso_manager::load_values() {

	// These are static, but will get to the same value each time
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences log_settings(user_settings, "Scratchpad");
	log_settings.get("Font Name", (int&)font_, FONT);
	log_settings.get("Font Size", (int&)fontsize_, FONT_SIZE);

}

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
	case QSO_EDIT:
		return false;
	case QSO_STARTED:
		return true;
	}
	return false;
}

// Called when rig is read to update values here
void qso_manager::rig_update(string frequency, string mode, string power) {
	if ((qso_group_->logging_state_ == QSO_PENDING ||
		qso_group_->logging_state_ == QSO_STARTED) &&
		qso_group_->logging_mode_ == LM_ON_AIR_CAT) {
		qso_group_->copy_cat_to_qso();
	}
}

// Get QSO information from previous record not rig
void qso_manager::update_rig() {
	// Get freq etc from QSO or rig
	// Get present values data from rig
	if (qso_group_->logging_state_ == QSO_PENDING) {
		switch (qso_group_->logging_mode_) {
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
		case LM_ON_AIR_COPY:
		case LM_ON_AIR_CLONE:
		{
			break;
		}
		}
	}
	//enable_use_widgets();
}

// Called whenever another view updates a record (or selects a new one)
void qso_manager::update_qso(hint_t hint, record_num_t match_num, record_num_t query_num) {
	switch (hint) {
	case HT_SELECTED:
	case HT_ALL:
	case HT_CHANGED:
	case HT_DELETED:
	case HT_INSERTED:
	case HT_MINOR_CHANGE:
	case HT_NEW_DATA:
	case HT_RESET_ORDER:
		qso_group_->update_qso(match_num);
		break;
	case HT_IMPORT_QUERY:
		qso_group_->update_query(QUERY_MATCH, match_num, query_num);
		break;
	case HT_IMPORT_QUERYNEW:
		qso_group_->update_query(QUERY_NEW, match_num, query_num);
		break;
	case HT_DUPE_QUERY:
		qso_group_->update_query(QUERY_DUPE, match_num, query_num);
		break;
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
		status_->misc_status(ST_ERROR, "DASH: Cannot start a QSO when one already started");
		break;
	case QSO_EDIT:
		status_->misc_status(ST_ERROR, "DASH: Cannot start a QSO while editing an existing one");
		break;
	}
}

// End QSO - add time off
// TODO: Can be called without current_qso_ - needs to be set by something.
void qso_manager::end_qso() {
	switch (qso_group_->logging_state_) {
	case QSO_INACTIVE:
		status_->misc_status(ST_ERROR, "DASH: Cannot end a QSO that hasn't been started");
		break;
	case QSO_PENDING:
		qso_group_->action_start();
		// drop through
	case QSO_STARTED:
		qso_group_->action_save();
		break;
	case QSO_EDIT:
		qso_group_->action_save_edit();
		break;
	}

}

// Edit QSO
void qso_manager::edit_qso() {
	switch (qso_group_->logging_state_) {
	case QSO_INACTIVE:
		qso_group_->action_edit();
		break;
	case QSO_PENDING:
	case QSO_STARTED:
		status_->misc_status(ST_ERROR, "DASH: Cannot edit a QSO when in on-air");
		break;
	case QSO_EDIT:
		status_->misc_status(ST_ERROR, "DASH: Cannot edit another QSO while editing an existing one");
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

// Copy the sleected QSOs MY_RIG etc to the supplied qso record
void qso_manager::update_import_qso(record* import_qso) {
	record* use_qso;
	string mode = import_qso->item("MODE");
	string submode = import_qso->item("SUBMODE");
	// If we have activated the QSO manager, then use active QSO as template
	use_qso = qso_group_->get_default_record();
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

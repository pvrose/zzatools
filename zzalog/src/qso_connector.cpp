#include "qso_connector.h"
#include "rig_if.h"
#include "serial.h"
#include "status.h"
#include "qso_manager.h"

#include <set>
#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;

extern Fl_Preferences* settings_;
extern rig_if* rig_if_;
extern status* status_;
extern void add_rig_if();


// Constructor
qso_connector::qso_connector(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	cat_data_ = new cat_data;
	load_values();
	create_form(X, Y);
	enable_widgets();
}

// DEstructor
qso_connector::~qso_connector() {}

// Get initial data from settings
void qso_connector::load_values() {
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
void qso_connector::create_form(int X, int Y) {

	begin();

	int max_w = 0;
	int max_h = 0;

	// CAT control group
	label("CAT Configuration");
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	int curr_x = X + GAP + WLABEL;
	int curr_y = Y + HTEXT;

	// Choice - Select the rig model (Manufacturer/Model)
	Fl_Choice* ch_model_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Rig");
	ch_model_->align(FL_ALIGN_LEFT);
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
	serial_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	serial_grp_->box(FL_NO_BOX);

	// Choice port name - serial
	curr_x = serial_grp_->x() + WLABEL;
	curr_y = serial_grp_->y();
	Fl_Choice* ch_port = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Port");
	ch_port->align(FL_ALIGN_LEFT);
	ch_port->callback(cb_ch_port, nullptr);
	ch_port->tooltip("Select the comms port to use");
	port_if_choice_ = ch_port;

	// Use all ports
	int save_x = curr_x;
	curr_x += ch_port->w() + GAP;
	Fl_Check_Button* bn_useall = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All ports");
	bn_useall->align(FL_ALIGN_RIGHT);
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
	ch_baudrate->tooltip("Enter baud rate");
	ch_baudrate->callback(cb_ch_baud, nullptr);
	baud_rate_choice_ = ch_baudrate;

	// Override capabilities (as coded in hamlib)
	curr_x += ch_baudrate->w() + GAP;
	Fl_Check_Button* bn_override = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Override\ncapability");
	bn_override->align(FL_ALIGN_RIGHT);
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
	network_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	network_grp_->box(FL_NO_BOX);

	// Input port name - network
	curr_x = network_grp_->x() + WLABEL;
	curr_y = network_grp_->y();
	Fl_Input* ip_port = new Fl_Input(curr_x, curr_y, WSMEDIT, HTEXT, "Port");
	ip_port->align(FL_ALIGN_LEFT);
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
	poll_grp->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	poll_grp->box(FL_NO_BOX);

	curr_x = save_x;
	curr_y += HTEXT;
	// Spinner to select fast polling rate (i.e. when still connected)
	ctr_pollfast_ = new Fl_Spinner(curr_x, curr_y, WSMEDIT, HTEXT, "Conn'd");
	ctr_pollfast_->align(FL_ALIGN_LEFT);
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
void qso_connector::save_values() {
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
void qso_connector::enable_widgets() {

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
	redraw();
}

// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
void qso_connector::populate_model_choice() {
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
void qso_connector::populate_port_choice() {
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
void qso_connector::populate_baud_choice() {
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
void qso_connector::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	qso_connector* that = ancestor_view<qso_connector>(w);
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
	that->populate_port_choice();
	that->enable_widgets();
}

// Callback selecting port
// v is unused
void qso_connector::cb_ch_port(Fl_Widget* w, void* v) {
	qso_connector* that = ancestor_view<qso_connector>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->port_name);
}

// Callback entering port
// v is unused
void qso_connector::cb_ip_port(Fl_Widget* w, void* v) {
	qso_connector* that = ancestor_view<qso_connector>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_value<Fl_Input, string>(w, (void*)&info->port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_connector::cb_ch_baud(Fl_Widget* w, void* v) {
	qso_connector* that = ancestor_view<qso_connector>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_text<Fl_Choice, string>(w, (void*)&info->baud_rate);
}

// Override rig capabilities selected - repopulate the baud choice
// v is uused
void qso_connector::cb_ch_over(Fl_Widget* w, void* v) {
	qso_connector* that = ancestor_view<qso_connector>(w);
	hamlib_data* info = &that->cat_data_->hamlib_params;
	cb_value<Fl_Check_Button, bool>(w, (void*)&info->override_caps);
	that->populate_baud_choice();
}

// Select "display all ports" in port choice
// v is a pointer to the all ports flag
void qso_connector::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_connector* that = ancestor_view<qso_connector>(w);
	that->populate_port_choice();
}

// Changed the fast polling interval
// v is not used
void qso_connector::cb_ctr_pollfast(Fl_Widget* w, void* v) {
	// Get the warning level
	qso_connector* that = ancestor_view<qso_connector>(w);
	cb_value<Fl_Spinner, double>(w, &that->cat_data_->fast_poll_interval);
}

// Changed the fast polling interval
// v is not used
void qso_connector::cb_ctr_pollslow(Fl_Widget* w, void* v) {
	// Get the warning level
	qso_connector* that = ancestor_view<qso_connector>(w);
	cb_value<Fl_Spinner, double>(w, &that->cat_data_->slow_poll_interval);
}

// Pressed the connect button - this is also called from toolbar
// v is not used - allow for w to be qso_manager_
void qso_connector::cb_bn_connect(Fl_Widget* w, void* v) {
	qso_connector* that = ancestor_view<qso_connector>(w);
	that->switch_rig();
}

// Connect rig if disconnected and vice-versa
void qso_connector::switch_rig() {
	if (rig_if_) {
		// We are connected - set disconnected
		delete rig_if_;
		rig_if_ = nullptr;
		wait_connect_ = true;
	}
	else {
		// Wer are discooencted, so connect
		add_rig_if();
		wait_connect_ = false;
	}
	((qso_manager*)parent())->update_rig();
	save_values();
}



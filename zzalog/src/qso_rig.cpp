#include "qso_rig.h"
#include "rig_if.h"
#include "serial.h"
#include "status.h"
#include "qso_manager.h"

#include <set>
#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;

extern Fl_Preferences* settings_;
extern status* status_;


// Constructor
qso_rig::qso_rig(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, nullptr)
{
	// If no name is provided then get from qso_manager
	if (L == nullptr || strlen(L) == 0) copy_label(ancestor_view<qso_manager>(this)->get_default(qso_manager::RIG).c_str());
	// Otherwise copy that supplied as it is probably a transient string
	else copy_label(L);
	load_values();
	rig_ = new rig_if(label(), hamlib_data_);
	create_form(X, Y);
	enable_widgets();
}

// DEstructor
qso_rig::~qso_rig() {
	save_values();
	delete rig_;
}

// Get initial data from settings
void qso_rig::load_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	Fl_Preferences cat_settings(settings_, "CAT");
	if (cat_settings.groupExists(label())) {
		Fl_Preferences rig_settings(cat_settings, label());
		Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
		char* temp;
		hamlib_settings.get("Rig Model", temp, "dummy");
		hamlib_data_.model = temp;
		free(temp);
		hamlib_settings.get("Manufacturer", temp, "Hamlib");
		hamlib_data_.mfr = temp;
		free(temp);
		hamlib_settings.get("Port", temp, "COM6");
		hamlib_data_.port_name = temp;
		free(temp);
		hamlib_settings.get("Baud Rate", hamlib_data_.baud_rate, 9600);
		hamlib_settings.get("Model ID", (int&)hamlib_data_.model_id, -1);

		// Check that hamlib is currently OK
		const rig_caps* capabilities = rig_get_caps(hamlib_data_.model_id);
		if (capabilities->model_name != hamlib_data_.model ||
			capabilities->mfg_name != hamlib_data_.mfr) {
			char msg[128];
			snprintf(msg, 128, "RIG: Saved model id %d does not match supplied rig model %s/%s",
				hamlib_data_.model_id,
				hamlib_data_.mfr.c_str(),
				hamlib_data_.model.c_str());
			status_->misc_status(ST_WARNING, msg);
			find_hamlib_data();
		}
		else {
			hamlib_data_.port_type = capabilities->port_type;
		};
	}
	else {
		// New hamlib data
		hamlib_data_ = rig_if::hamlib_data_t();
	}
	mode_ = hamlib_data_.port_type;
}

void qso_rig::find_hamlib_data() {
	// Now search hamlib for the model_id
	// Search through the rig database until we find the required rig.
	const rig_caps* capabilities = nullptr;
	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
	for (rig_model_t i = 0; i < max_rig_num && hamlib_data_.model_id == -1; i++) {
		// Read each rig's capabilities
		capabilities = rig_get_caps(i);
		try {
			if (capabilities != nullptr) {
				// Not all numbers represent a rig as the mapping is sparse
				// Check the model name
				if (capabilities->model_name == hamlib_data_.model &&
					capabilities->mfg_name == hamlib_data_.mfr) hamlib_data_.model_id = i;
			}
		}
		catch (exception*) {}
	}
	if (hamlib_data_.model_id == -1) {
		char msg[128];
		snprintf(msg, 128, "RIG: %s/%s not found in hamlib capabilities",
			hamlib_data_.mfr.c_str(),
			hamlib_data_.model.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Create CAT control widgets
void qso_rig::create_form(int X, int Y) {

	begin();

	int max_w = 0;
	int max_h = 0;

	// CAT control group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	int curr_x = X + GAP + WLABEL;
	int curr_y = Y + GAP;

	// First button - connect/disconnect or add
	bn_connect_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
	bn_connect_->color(FL_YELLOW);
	bn_connect_->tooltip("Select to attempt to connect/disconnect rig");
	bn_connect_->callback(cb_bn_connect, nullptr);
	
	curr_y += HBUTTON + GAP;

	// Choice - Select the rig model (Manufacturer/Model)
	ch_rig_model_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Rig");
	ch_rig_model_->align(FL_ALIGN_LEFT);
	ch_rig_model_->tooltip("Select the model - for Hamlib");
	ch_rig_model_->callback(cb_ch_model, nullptr);

	// Populate the manufacturere and model choice widgets
	populate_model_choice();

	// Hamlib control grp
	// RIG=====v
	// PORTv  ALL*
	// BAUDv  OVR*
	curr_y += HTEXT + GAP;
	serial_grp_ = new Fl_Group(X + GAP, curr_y, 10, 10);
	serial_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	serial_grp_->box(FL_NO_BOX);

	// Choice port name - serial
	curr_x = serial_grp_->x() + WLABEL;
	curr_y = serial_grp_->y();
	ch_port_name_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Port");
	ch_port_name_->align(FL_ALIGN_LEFT);
	ch_port_name_->callback(cb_ch_port, nullptr);
	ch_port_name_->tooltip("Select the comms port to use");

	// Use all ports
	int save_x = curr_x;
	curr_x += WBUTTON + GAP;
	bn_all_ports_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All ports");
	bn_all_ports_->align(FL_ALIGN_RIGHT);
	bn_all_ports_->tooltip("Select all existing ports, not just those available");
	bn_all_ports_->callback(cb_bn_all, &use_all_ports_);
	populate_port_choice();
	curr_x += HBUTTON + WLABEL + GAP;
	int max_x = curr_x;

	// Baud rate input 
	curr_x = save_x;
	curr_y += HBUTTON;
	int max_y = curr_y;
	ch_baud_rate_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Baud rate");
	ch_baud_rate_->align(FL_ALIGN_LEFT);
	ch_baud_rate_->tooltip("Enter baud rate");
	ch_baud_rate_->callback(cb_ch_baud, nullptr);

	// Override capabilities (as coded in hamlib)
	curr_x += WBUTTON + GAP;
	bn_all_rates_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All rates");
	bn_all_rates_->align(FL_ALIGN_RIGHT);
	bn_all_rates_->tooltip("Allow full baud rate selection");
	bn_all_rates_->callback(cb_ch_over, nullptr);
	populate_baud_choice();
	curr_x += HBUTTON + WLABEL + GAP;
	max_x = max(max_x, curr_x);

	curr_y += HBUTTON + GAP;
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
	ip_port_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HTEXT, "Port");
	ip_port_->align(FL_ALIGN_LEFT);
	ip_port_->callback(cb_ip_port, nullptr);
	ip_port_->tooltip("Enter the network/USB port to use");
	ip_port_->value(hamlib_data_.port_name.c_str());

	curr_x += WSMEDIT + GAP;
	curr_y += HTEXT + GAP;
	network_grp_->resizable(nullptr);
	network_grp_->size(curr_x - network_grp_->x(), curr_y - network_grp_->y());

	network_grp_->end();

	max_y = max(serial_grp_->y() + serial_grp_->h(), network_grp_->y() + network_grp_->h());
	max_x = max(serial_grp_->x() + serial_grp_->w(), network_grp_->x() + network_grp_->w());
	curr_y = max_y + GAP;
	curr_x = save_x;

	// Connected status

	// Display hamlib ot flrig settings as selected
	enable_widgets();

	resizable(nullptr);
	size(max_x - x(), curr_y - y());
	end();
}

// Save values in settings
void qso_rig::save_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	if (hamlib_data_.port_type != RIG_PORT_NONE) {
		Fl_Preferences cat_settings(settings_, "CAT");
		Fl_Preferences rig_settings(cat_settings, label());
		Fl_Preferences hamlib_settings(rig_settings, "Hamlib");
		hamlib_settings.set("Rig Model", hamlib_data_.model.c_str());
		hamlib_settings.set("Manufacturer", hamlib_data_.mfr.c_str());
		hamlib_settings.set("Port", hamlib_data_.port_name.c_str());
		hamlib_settings.set("Baud Rate", hamlib_data_.baud_rate);
		hamlib_settings.set("Model ID", (int)hamlib_data_.model_id);
	}
}

// Enable CAT Connection widgets
void qso_rig::enable_widgets() {

	// CAT control widgets
	switch (mode_) {
	case RIG_PORT_SERIAL:
		// If we are connected do not allow it to be changed
		if (rig_->is_open()) serial_grp_->deactivate();
		else serial_grp_->activate();
		serial_grp_->show();
		network_grp_->deactivate();
		network_grp_->hide();
		if (rig_->is_open()) {
			bn_connect_->color(FL_GREEN);
			bn_connect_->label("Disconnect");
		}
		else {
			bn_connect_->color(FL_RED);
			bn_connect_->label("Connect");
		}
		break;
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		serial_grp_->deactivate();
		serial_grp_->hide();
		// If we are connected do not allow it to be changed
		if (rig_->is_open()) network_grp_->deactivate();
		else network_grp_->activate();
		network_grp_->show();
		ip_port_->value(hamlib_data_.port_name.c_str());
		if (rig_->is_open()) {
			bn_connect_->color(FL_GREEN);
			bn_connect_->label("Disconnect");
		}
		else {
			bn_connect_->color(FL_RED);
			bn_connect_->label("Connect");
		}
		break;
	case RIG_PORT_NONE:
		serial_grp_->deactivate();
		serial_grp_->hide();
		network_grp_->deactivate();
		network_grp_->hide();
		bn_connect_->color(FL_YELLOW);
		bn_connect_->label("Add Rig");
		break;
	}
	redraw();
}

// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
void qso_rig::populate_model_choice() {
	// Get hamlib Model number and populate control with all model names
	ch_rig_model_->clear();
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
			if (string(capabilities->mfg_name) == hamlib_data_.mfr &&
				string(capabilities->model_name) == hamlib_data_.model) {
				hamlib_data_.model_id = capabilities->rig_model;
				hamlib_data_.port_type = capabilities->port_type;
			}
		}
	}
	// Add the rigs in alphabetical order to the choice widget, set widget's value to intended
	ch_rig_model_->add("");
	for (auto ix = rig_list.begin(); ix != rig_list.end(); ix++) {
		string name = *ix;
		rig_model_t id = rig_ids.at(name);
		int pos = ch_rig_model_->add(name.c_str(), 0, nullptr, (void*)id);
		if (id == hamlib_data_.model_id) {
			ch_rig_model_->value(pos);
		}
	}
	bool found = false;
}

// Populate the choice with the available ports
void qso_rig::populate_port_choice() {
	if (hamlib_data_.port_type == RIG_PORT_SERIAL) {
		ch_port_name_->clear();
		ch_port_name_->add("NONE");
		ch_port_name_->value(0);
		int num_ports = 1;
		string* existing_ports = new string[1];
		serial serial;
		// Get the list of all ports or available (not in use) ports
		while (!serial.available_ports(num_ports, existing_ports, use_all_ports_, num_ports)) {
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
			ch_port_name_->add(port);
			// Set the value to the list of ports
			if (strcmp(port, hamlib_data_.port_name.c_str()) == 0) {
				ch_port_name_->value(i);
			}
		}
	}
}

// Populate the baud rate choice menu
void qso_rig::populate_baud_choice() {
	if (hamlib_data_.port_type == RIG_PORT_SERIAL) {
		ch_baud_rate_->clear();
		// Override rig's capabilities?
		bn_all_rates_->value(use_all_rates_);

		// Get the baud-rates supported by the rig
		const rig_caps* caps = rig_get_caps(hamlib_data_.model_id);
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
		ch_baud_rate_->value(0);
		// If no values add an empty value
		if (num_rates == 0)	ch_baud_rate_->add("");
		// For all possible rates
		for (int i = 0; i < num_rates; i++) {
			int rate = baud_rates[i];
			if (use_all_rates_ || (rate >= min_baud_rate && rate <= max_baud_rate)) {
				// capabilities overridden or within the range supported by capabilities
				ch_baud_rate_->add(to_string(rate).c_str());
				if (rate == hamlib_data_.baud_rate) {
					ch_baud_rate_->value(index);
					index++;
				}
			}
		}
	}
}


// Model input choice selected
// v is not used
void qso_rig::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	qso_rig* that = ancestor_view<qso_rig>(w);
	const Fl_Menu_Item* item = ch->mvalue();
	rig_model_t id = (long)item->user_data();
	const char* label = ch->text();
	const rig_caps* capabilities = rig_get_caps(id);
	if (capabilities != nullptr) {
		that->hamlib_data_.model = capabilities->model_name;
		that->hamlib_data_.mfr = capabilities->mfg_name;
		that->hamlib_data_.model_id = id;
		that->hamlib_data_.port_type = capabilities->port_type;
	}
	else {
		char message[128];
		snprintf(message, 128, "DASH: Error reading hamlib details selecting %s", label);
		status_->misc_status(ST_ERROR, message);
	}
	switch (that->mode_) {
	case RIG_PORT_SERIAL:
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		that->mode_ = that->hamlib_data_.port_type;
		break;
	case RIG_PORT_NONE:
		// Retain mode = NONE until the add button is clicked
		break;
	}
	that->populate_baud_choice();
	that->populate_port_choice();
	that->enable_widgets();
}

// Callback selecting port
// v is unused
void qso_rig::cb_ch_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_text<Fl_Choice, string>(w, (void*)&that->hamlib_data_.port_name);
}

// Callback entering port
// v is unused
void qso_rig::cb_ip_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Input, string>(w, (void*)&that->hamlib_data_.port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_rig::cb_ch_baud(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_text<Fl_Choice, string>(w, (void*)&that->hamlib_data_.baud_rate);
}

// Override rig capabilities selected - repopulate the baud choice
// v is uused
void qso_rig::cb_ch_over(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->populate_baud_choice();
}

// Select "display all ports" in port choice
// v is a pointer to the all ports flag
void qso_rig::cb_bn_all(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->populate_port_choice();
}

// Pressed the connect button - this is also called from toolbar
// v is not used 
void qso_rig::cb_bn_connect(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	switch (that->mode_) {
	case RIG_PORT_SERIAL:
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		that->switch_rig();
		break;
	case RIG_PORT_NONE:
		that->new_rig();
		break;
	}
}

// Connect rig if disconnected and vice-versa
void qso_rig::switch_rig() {
	delete rig_;
	rig_ = new rig_if(label(), hamlib_data_);
	ancestor_view<qso_manager>(this)->update_rig();
	enable_widgets();
}

// 1 s clock interface - read rig and update status
//RS_OFF,              // The rig is off or disconnected
//RS_ERROR,            // An error with the rig occured
//RS_RX,               // The rig is connected and in RX mode
//RS_TX,               // The rig is connected and in TX mode
//RS_HIGH              // The rig is in TX mode but SWR is too high
void qso_rig::ticker() {
	rig_->ticker();
	string msg = rig_->rig_info();
	rig_status_t st;
	if (!rig_->is_open()) st = RS_OFF;
	else if (!rig_->is_good()) st = RS_ERROR;
	else if (!rig_->get_data().ptt) st = RS_RX;
	else if (rig_->get_data().swr >= 2.0) st = RS_TX;
	else st = RS_HIGH;
	status_->rig_status(st, msg.c_str());
}

// New rig 
void qso_rig::new_rig() {
	mode_ = hamlib_data_.port_type;
	enable_widgets();
}

// Return rig pointer
rig_if* qso_rig::rig() {
	return rig_;
}

// Return the colour used in the connect button as its alert
Fl_Color qso_rig::alert_colour() {
	return bn_connect_->color();
}
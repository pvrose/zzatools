#include "qso_rig.h"
#include "rig_if.h"
#include "serial.h"
#include "status.h"
#include "qso_manager.h"
#include "band_data.h"
#include "spec_data.h"

#include <set>
#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;

extern Fl_Preferences* settings_;
extern status* status_;
extern band_data* band_data_;
extern spec_data* spec_data_;


// Constructor
qso_rig::qso_rig(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, nullptr),
	modify_freq_(false),
	freq_offset_(0.0),
	modify_gain_(false),
	gain_(0),
	modify_power_(false),
	power_(0.0)
{
	// If no name is provided then get from qso_manager
	if (L == nullptr || strlen(L) == 0) copy_label(ancestor_view<qso_manager>(this)->get_default(qso_manager::RIG).c_str());
	// Otherwise copy that supplied as it is probably a transient string
	else copy_label(L);
	// CAT control group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	load_values();
	rig_ = new rig_if(label(), &hamlib_data_);
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

		Fl_Preferences modifier_settings(rig_settings, "Modifiers");
		if (modifier_settings.get("Frequency", freq_offset_, 0.0)) {
			modify_freq_ = true;
		} else {
			modify_freq_ = false;
		}
		if (modifier_settings.get("Gain", gain_, 0)) {
			modify_gain_ = true;
		} else {
			modify_gain_ = false;
		}
		if (modifier_settings.get("Power", power_, 0.0)) {
			modify_power_ = true;
		} else {
			modify_power_ = false;
		}
		// If we cannot read power from hamlib - force modify_power_
		if (!(capabilities->has_get_level & RIG_LEVEL_RFPOWER_METER_WATTS)) {
			char msg[128];
			snprintf(msg, sizeof(msg), "DASH: Rig %s does not supply power - set it", 
				hamlib_data_.model.c_str());
			status_->misc_status(ST_WARNING, msg);
			modify_power_ = true;
			modify_gain_ = false;
		}
	}
	else {
		// New hamlib data
		hamlib_data_ = rig_if::hamlib_data_t();
		modify_freq_ = false;
		modify_power_ = false;
		modify_gain_ = false;
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

	int curr_x = X + GAP;
	int curr_y = Y + 1;
	
	// "Label" - rig status
	op_status_ = new Fl_Output(curr_x, curr_y, WEDIT, HBUTTON);
	op_status_->color(FL_BACKGROUND_COLOR);
	op_status_->textfont(FL_BOLD);
	op_status_->textsize(FL_NORMAL_SIZE + 2);
	op_status_->box(FL_FLAT_BOX);
	
	curr_y += op_status_->h();

	// First button - connect/disconnect or add
	bn_connect_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
	bn_connect_->tooltip("Select to attempt to connect/disconnect rig");
	bn_connect_->callback(cb_bn_connect, nullptr);
	
	curr_x += bn_connect_->w();
	bn_select_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Select");
	bn_select_->color(FL_YELLOW, FL_BLACK);
	bn_select_->tooltip("Select the rig to connect");
	bn_select_->value(false);
	bn_select_->callback(cb_bn_select, nullptr);
	
	curr_x = X + GAP + WLABEL;
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
	bn_all_rates_->callback(cb_ch_over, &use_all_rates_);
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

	curr_x = x() + GAP;
	curr_y = max(serial_grp_->y() + serial_grp_->h(), network_grp_->y() + network_grp_->h());
	
	modifier_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);
	modifier_grp_->box(FL_FLAT_BOX);

	curr_x = modifier_grp_->x() + WLABEL;

	bn_mod_freq_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "\316\224 F (MHz)");
	bn_mod_freq_->align(FL_ALIGN_LEFT);
	bn_mod_freq_->callback(cb_bn_mod_freq, (void*)&modify_freq_);
	bn_mod_freq_->tooltip("Allow frequency to be offset - eg transverter");
	bn_mod_freq_->value(modify_freq_);
	
	curr_x += WRADIO;
	ip_freq_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_freq_->callback(cb_ip_freq, (void*)&freq_offset_);
	ip_freq_->tooltip("Enter frequency offset");
	ip_freq_->when(FL_WHEN_CHANGED);
	char text[20];
	snprintf(text, sizeof(text), "%g", freq_offset_);
	ip_freq_->value(text);

	curr_y += HBUTTON;
	curr_x = modifier_grp_->x() + WLABEL;

	bn_gain_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Gain (dB)");
	bn_gain_->align(FL_ALIGN_LEFT);
	bn_gain_->callback(cb_bn_power, (void*)(intptr_t)false);
	bn_gain_->tooltip("Allow amplifier");
	bn_gain_->value(modify_gain_);

	curr_x += WRADIO;
	ip_gain_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_gain_->callback(cb_ip_gain, (void*)&gain_);
	ip_gain_->when(FL_WHEN_CHANGED);
	ip_gain_->tooltip("Specify the amplifier gain (dB)");
	snprintf(text, sizeof(text), "%d", gain_);
	ip_gain_->value(text);

	curr_y += HBUTTON;
	curr_x = modifier_grp_->x() + WLABEL;

	bn_power_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Pwr (W)");
	bn_power_->align(FL_ALIGN_LEFT);
	bn_power_->callback(cb_bn_power, (void*)(intptr_t)true);
	bn_power_->tooltip("Specify absolute power");
	bn_power_->value(modify_power_);

	curr_x += WRADIO;
	ip_power_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_power_->callback(cb_ip_power, (void*)&power_);
	ip_power_->when(FL_WHEN_CHANGED);
	ip_power_->tooltip("Specify power out ");
	snprintf(text, sizeof(text), "%g", power_);
	ip_power_->value(text);

	curr_x += WBUTTON;
	curr_y += HBUTTON + GAP;
	modifier_grp_->resizable(nullptr);
	modifier_grp_->size(curr_x - modifier_grp_->x(), curr_y - modifier_grp_->y());

	curr_x = x() + GAP;

	display_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);
	display_grp_->box(FL_NO_BOX);

	op_summary_ = new Fl_Box(curr_x, curr_y, WSMEDIT + WBUTTON, HTEXT);
	op_summary_->tooltip("Frequency summary");
	op_summary_->box(FL_FLAT_BOX);
	op_summary_->color(FL_BLACK);
	op_summary_->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	op_summary_->labelsize(FL_NORMAL_SIZE + 2);

	curr_y += op_summary_->h();

	curr_x = op_summary_->x();
	op_freq_mode_ = new Fl_Box(curr_x, curr_y, op_summary_->w(), op_summary_->h());
	op_freq_mode_->tooltip("Current displayed mode");
	op_freq_mode_->box(FL_FLAT_BOX);
	op_freq_mode_->color(FL_BLACK);
	op_freq_mode_->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	op_freq_mode_->labelcolor(FL_YELLOW);
	op_freq_mode_->labelfont(FL_BOLD);
	op_freq_mode_->labelsize(FL_NORMAL_SIZE + 4);

	curr_y += op_freq_mode_->h();
	curr_x += op_freq_mode_->w() + GAP;

	display_grp_->resizable(nullptr);
	display_grp_->size(curr_x - display_grp_->x(), curr_y - display_grp_->y());

	display_grp_->end();


	max_x = max(
		max(serial_grp_->x() + serial_grp_->w(), network_grp_->x() + network_grp_->w()),
		display_grp_->x() + display_grp_->w());

	curr_y = curr_y + GAP;
	curr_x = save_x;

	// Connected status

	// Display hamlib ot flrig settings as selected
	modify_rig();
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
		// Modifier settings
		Fl_Preferences modr_settings(rig_settings, "Modifiers");
		modr_settings.clear();
		if (modify_freq_) {
			modr_settings.set("Frequency", freq_offset_);
		}
		if (modify_gain_) {
			modr_settings.set("Gain", gain_);
		}
		if (modify_power_) {
			modr_settings.set("Power", power_);
		}
	}
}

// Enable CAT Connection widgets
void qso_rig::enable_widgets() {
	// CAT access buttons
	if (!rig_) {
		bn_connect_->deactivate();
		bn_connect_->color(FL_BACKGROUND_COLOR);
		bn_connect_->label("");
		bn_select_->activate();
		if (bn_select_->value()) {
			bn_select_->label("Use");
		} else {
			bn_select_->label("Select");
		}
	} else if (rig_->is_open()) {
		bn_connect_->activate();
		bn_connect_->color(COLOUR_APPLE);
		bn_connect_->label("Disconnect");
		bn_select_->deactivate();
		bn_select_->label("");
		bn_select_->value(false);
	} else {
		bn_connect_->activate();
		bn_connect_->color(COLOUR_MAUVE);
		bn_connect_->label("Connect");
		bn_select_->activate();
		if (bn_select_->value()) {
			bn_select_->label("Use");
		} else {
			bn_select_->label("Select");
		}
	}
	// Status
	if (!rig_) {
		op_status_->value("No rig specified");
	} else if (rig_->is_opening()) {
		op_status_->value("Opening rig");
	} else if (rig_->is_open()) {
		op_status_->value("Connected");
	} else {
		op_status_->value("Disconnected");
	}
	// CAT control widgets - allow only when select button active
	if (bn_select_->value()) {
		serial_grp_->activate();
		network_grp_->activate();
	} else {
		serial_grp_->deactivate();
		network_grp_->deactivate();
	}
	switch (mode_) {
	case RIG_PORT_SERIAL:
		serial_grp_->show();
		network_grp_->hide();
		break;
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		serial_grp_->hide();
		network_grp_->show();
		if (!bn_select_->value()) {
			ip_port_->value(hamlib_data_.port_name.c_str());
		}
		break;
	case RIG_PORT_NONE:
		serial_grp_->hide();
		network_grp_->hide();
		break;
	}
	// Update modifier values
	if (modify_freq_) {
		ip_freq_->activate();
	} else {
		ip_freq_->deactivate();
	}
	// We have modified the values so reload them
	bn_gain_->value(modify_gain_);
	bn_power_->value(modify_power_);
	if (modify_gain_) {
		ip_gain_->activate();
	} else {
		ip_gain_->deactivate();
	}
	if (modify_power_) {
		ip_power_->activate();
	} else {
		ip_power_->deactivate();
	}
	// Update display values
	if (rig_ && rig_->is_open()) {
		op_summary_->activate();
		op_summary_->color(FL_BLACK);
		op_freq_mode_->activate();
		if (rig_->get_ptt()) {
			op_freq_mode_->color(FL_RED);
		} else {
			op_freq_mode_->color(FL_BLACK);
		}
		double freq = rig_->get_dfrequency(true);
		band_data::band_entry_t* entry = band_data_->get_entry(freq * 1000);
		if (entry) {
			char l[50];
			snprintf(l, sizeof(l), "%s %s", spec_data_->band_for_freq(freq).c_str(), entry->mode.c_str());
			op_summary_->copy_label(l);
			op_summary_->labelcolor(FL_YELLOW);
		}
		else {
			op_summary_->label("Out of band!");
			op_summary_->labelcolor(FL_RED);
		}
		char msg[100];
		string rig_mode;
		string submode;
		rig_->get_string_mode(rig_mode, submode);
		snprintf(msg, sizeof(msg), "  %0.3fMHz %s %sW" , 
			freq, 
			submode.length() ? submode.c_str() : rig_mode.c_str(),
			rig_->get_tx_power(true).c_str()
		);
		op_freq_mode_->copy_label(msg);
	}
	else {
		op_summary_->deactivate();
		op_summary_->label("");
		op_summary_->color(FL_BACKGROUND_COLOR);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
		op_freq_mode_->color(FL_BACKGROUND_COLOR);
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
		int pos = ch_rig_model_->add(name.c_str(), 0, nullptr, (void*)(intptr_t)id);
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
	rig_model_t id = (intptr_t)item->user_data();
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
	switch (that->hamlib_data_.port_type) {
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
	const char* text = ((Fl_Choice*)w)->text();
	that->hamlib_data_.baud_rate = atoi(text);
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
	if (that->rig_->is_open()) {
		that->rig_->close();
	} else {
		that->rig_->open();
	}
	that->enable_widgets();
}

// Pressed the select button 
// v is not used 
void qso_rig::cb_bn_select(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	Fl_Light_Button* bn = (Fl_Light_Button*)w;
	if (bn->value()) {
		// This is called in switch_rig
		that->enable_widgets();
	} else {
		that->switch_rig();
	}
}

// Pressed the modify frequency
// v is the bool
void qso_rig::cb_bn_mod_freq(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Check_Button, bool>(w, v);
	that->modify_rig();
	that->enable_widgets();
}

// Typed in freaquency value
// v is the input
void qso_rig::cb_ip_freq(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	float f;
	cb_value_float<Fl_Float_Input>(w, &f);
	*(double*)v = (double)f;
	that->modify_rig();
	that->enable_widgets();
}

// Pressed either power or gain check button
// v: false = gain, true = power
void qso_rig::cb_bn_power(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	bool bn_p = (bool)(intptr_t)v;
	if (bn_p) {
		bool v = that->bn_power_->value();
		if (v) {
			that->modify_power_ = true;
			that->modify_gain_ = false;
		} else {
			that->modify_power_ = false;
			that->modify_gain_ = false;
		}
	} else {
		bool v = that->bn_gain_->value();
		if (v) {
			that->modify_power_ = false;
			that->modify_gain_ = true;
		} else {
			that->modify_power_ = false;
			that->modify_gain_ = false;
		}
	}
	that->modify_rig();
	that->enable_widgets();
}

// Entered gain value
void qso_rig::cb_ip_gain(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value_int<Fl_Int_Input>(w, v);
	that->modify_rig();
	that->enable_widgets();
}

// Entered power value
void qso_rig::cb_ip_power(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	float f;
	cb_value_float<Fl_Float_Input>(w, &f);
	*(double*)v = (double)f;
	that->modify_rig();
	that->enable_widgets();
}

// Connect rig if disconnected and vice-versa
void qso_rig::switch_rig() {
	if (rig_) {
		delete rig_;
		rig_ = nullptr;
		rig_ = new rig_if(label(), &hamlib_data_);
	}
	ancestor_view<qso_manager>(this)->update_rig();
	modify_rig();
	enable_widgets();
}

// 1 s clock interface - read rig and update status
//RS_OFF,              // The rig is off or disconnected
//RS_ERROR,            // An error with the rig occured
//RS_RX,               // The rig is connected and in RX mode
//RS_TX,               // The rig is connected and in TX mode
//RS_HIGH              // The rig is in TX mode but SWR is too high
void qso_rig::ticker() {
	if (rig_) {
		rig_->ticker();
		// The rig may have disconnected - update connect/select buttons
		enable_widgets();
		// Update status with latest values from rig
		string msg = rig_->rig_info();
		rig_status_t st;
		if (!rig_->is_open()) st = RS_OFF;
		else if (!rig_->is_good()) st = RS_ERROR;
		else if (!rig_->get_ptt()) st = RS_RX;
		else st = RS_TX;
		status_->rig_status(st, msg.c_str());
	}
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

// Enable rig values
void qso_rig::modify_rig() {
	if (rig_) {
		if (modify_freq_) {
			rig_->set_freq_modifier(freq_offset_);
		} else {
			rig_->clear_freq_modifier();
		}
		if (modify_gain_ && modify_power_) {
			status_->misc_status(ST_SEVERE, "DASH: Trying to set both gain and absolute power");
		} else if (modify_gain_) {
			rig_->set_power_modifier(gain_);
		} else if (modify_power_) {
			rig_->set_power_modifier(power_);
		} else {
			rig_->clear_power_modifier();
		}
	}
}
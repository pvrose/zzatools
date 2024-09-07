#include "qso_rig.h"
#include "rig_if.h"
#include "serial.h"
#include "status.h"
#include "qso_manager.h"
#include "band_data.h"
#include "spec_data.h"
#include "ticker.h"
#include "field_choice.h"

#include <set>
#include <string>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Value_Slider.H>

using namespace std;

extern Fl_Preferences* settings_;
extern status* status_;
extern band_data* band_data_;
extern spec_data* spec_data_;
extern bool DARK;
extern ticker* ticker_;


// Constructor
qso_rig::qso_rig(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, nullptr),
	modify_freq_(false),
	freq_offset_(0.0),
	modify_gain_(false),
	gain_(0),
	modify_power_(false),
	power_(0.0),
	rig_ok_(false)
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
	rig_ = new rig_if(label(), cat_data_[cat_index_]->hamlib);
	if (rig_->is_good()) rig_ok_ = true;
	const rig_caps* capabilities = rig_get_caps(cat_data_[cat_index_]->hamlib->model_id);
	if (capabilities && !(capabilities->has_get_level & RIG_LEVEL_RFPOWER_METER_WATTS)) {
		char msg[128];
		snprintf(msg, sizeof(msg), "DASH: Rig %s does not supply power - set it",
			cat_data_[cat_index_]->hamlib->model.c_str());
		status_->misc_status(ST_WARNING, msg);
		modify_power_ = true;
		modify_gain_ = false;
	}
	modify_rig();
	create_form(X, Y);
	enable_widgets();

	ticker_->add_ticker(this, cb_ticker, 10);
}

// DEstructor
qso_rig::~qso_rig() {
	save_values();
	delete rig_;
	ticker_->remove_ticker(this);
}

void qso_rig::load_cat_data(qso_rig::cat_data_t* cat_data, Fl_Preferences settings) {
	char* temp;
	settings.get("Rig Model", temp, "dummy");
	cat_data->hamlib->model = temp;
	free(temp);
	settings.get("Manufacturer", temp, "hamlib");
	cat_data->hamlib->mfr = temp;
	free(temp);
	settings.get("Port", temp, "COM6");
	cat_data->hamlib->port_name = temp;
	free(temp);
	settings.get("Baud Rate", cat_data->hamlib->baud_rate, 9600);
	settings.get("Model ID", (int&)cat_data->hamlib->model_id, -1);
	// Timeout value
	settings.get("Timeout", cat_data->hamlib->timeout, 1.0);

	// Check that hamlib is currently OK
	const rig_caps* capabilities = rig_get_caps(cat_data->hamlib->model_id);
	if (capabilities == nullptr) {
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG: No CAT details for %s", label());
		status_->misc_status(ST_WARNING, msg);
	}
	else {
		if (capabilities->model_name != cat_data->hamlib->model ||
			capabilities->mfg_name != cat_data->hamlib->mfr) {
			char msg[128];
			snprintf(msg, 128, "RIG: Saved model id %d (%s/%s) does not match supplied rig model %s/%s using hamlib values",
				cat_data->hamlib->model_id,
				capabilities->mfg_name,
				capabilities->model_name,
				cat_data->hamlib->mfr.c_str(),
				cat_data->hamlib->model.c_str());
			status_->misc_status(ST_WARNING, msg);
			cat_data->hamlib->model = capabilities->model_name;
			cat_data->hamlib->mfr = capabilities->mfg_name;
		}
		cat_data->hamlib->port_type = capabilities->port_type;
	}
	if (settings.get("Command", temp, "")) {
		cat_data->use_cat_app = true;
	}
	cat_data->app = temp;
	
	free(temp);

}

// Get initial data from settings
void qso_rig::load_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	Fl_Preferences cat_settings(settings_, "CAT");
	if (cat_settings.groupExists(label())) {
		Fl_Preferences rig_settings(cat_settings, label());
		int max_cat = rig_settings.groups();
		if (max_cat > 0) cat_data_.resize(max_cat);
		else cat_data_.resize(1);
		if (max_cat != 0) {
			for (int ix = 0; ix < max_cat; ix++) {
				cat_data_t* data = new cat_data_t;
				data->hamlib = new hamlib_data_t;
				char g[5];
				snprintf(g, sizeof(g), "%d", ix);
				Fl_Preferences cat_settings(rig_settings, g);
				load_cat_data(data, cat_settings);
				cat_data_[ix] = data;
			}
		}
		else {
			cat_data_t* data = new cat_data_t;
			data->hamlib = new hamlib_data_t;
			load_cat_data(data, rig_settings);
			cat_data_[0] = data;
		}
		char* temp;
		// Get default cat type
		rig_settings.get("Default CAT", cat_index_, 0);
		// Preferred antenna
		rig_settings.get("Antenna", temp, "");
		antenna_ = temp;
		free(temp);

		// Get the CAT value modifiers
		// Transverter offset
		if (rig_settings.get("Frequency", freq_offset_, 0.0)) {
			modify_freq_ = true;
		} else {
			modify_freq_ = false;
		}
		// Amplifier gain
		if (rig_settings.get("Gain", gain_, 0)) {
			modify_gain_ = true;
		} else {
			modify_gain_ = false;
		}
		// Fixed power device
		if (rig_settings.get("Power", power_, 0.0)) {
			modify_power_ = true;
		} else {
			modify_power_ = false;
		}
		mode_ = (uint16_t)cat_data_[cat_index_]->hamlib->port_type;
	}
	else {
		// New hamlib data
		cat_data_.clear();
		modify_freq_ = false;
		modify_power_ = true;
		modify_gain_ = false;
		mode_ = RIG_PORT_NONE;
	}
}

// Get the hamlib data for the rig
void qso_rig::find_hamlib_data() {
	// Now search hamlib for the model_id
	// Search through the rig database until we find the required rig.
	const rig_caps* capabilities = nullptr;
	rig_model_t max_rig_num = 40 * MAX_MODELS_PER_BACKEND;
	// Serach the list for the rig. Stop when it's found
	for (rig_model_t i = 0; i < max_rig_num && cat_data_[cat_index_]->hamlib->model_id == -1; i++) {
		// Read each rig's capabilities
		capabilities = rig_get_caps(i);
		try {
			if (capabilities != nullptr) {
				// Not all numbers represent a rig as the mapping is sparse
				// Check the model name
				if (capabilities->model_name == cat_data_[cat_index_]->hamlib->model &&
					capabilities->mfg_name == cat_data_[cat_index_]->hamlib->mfr)
					cat_data_[cat_index_]->hamlib->model_id = i;
			}
		}
		catch (exception*) {}
	}
	if (cat_data_[cat_index_]->hamlib->model_id == -1) {
		char msg[128];
		snprintf(msg, 128, "RIG: %s/%s not found in hamlib capabilities",
			cat_data_[cat_index_]->hamlib->mfr.c_str(),
			cat_data_[cat_index_]->hamlib->model.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Keep the display 3 buttons wide
const int WDISPLAY = 3 * WBUTTON;
// Create the status part of the display
void qso_rig::create_status(int curr_x, int curr_y) {
	int save_x = curr_x;

	status_grp_ = new Fl_Group(curr_x, curr_y, WDISPLAY, HBUTTON + HTEXT * 3);
	status_grp_->box(FL_NO_BOX);

	bn_tx_rx_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	bn_tx_rx_->box(FL_OVAL_BOX);

	curr_x += HBUTTON;

	// "Label" - rig status
	op_status_ = new Fl_Output(curr_x, curr_y, WDISPLAY - (2 * HBUTTON), HBUTTON);
	op_status_->color(FL_BACKGROUND_COLOR);
	op_status_->textfont(FL_BOLD);
	op_status_->textsize(FL_NORMAL_SIZE + 2);
	op_status_->box(FL_FLAT_BOX);

	curr_x += op_status_->w();
	bn_index_ = new Fl_Menu_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	bn_index_->box(FL_FLAT_BOX);
	bn_index_->align(FL_ALIGN_LEFT);
	bn_index_->labelsize(FL_NORMAL_SIZE - 2);
	populate_index_menu();

	curr_x = save_x;
	curr_y += op_status_->h();

	// Display frequency and mode information
	op_freq_mode_ = new Fl_Box(curr_x, curr_y, WDISPLAY, HTEXT * 3);
	op_freq_mode_->tooltip("Current displayed mode");
	op_freq_mode_->box(FL_FLAT_BOX);
	op_freq_mode_->color(FL_BLACK);
	op_freq_mode_->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	op_freq_mode_->labelcolor(FL_YELLOW);
	op_freq_mode_->labelfont(FL_BOLD);
	op_freq_mode_->labelsize(FL_NORMAL_SIZE + 10);

	status_grp_->end();

}

// Create the control buttons
void qso_rig::create_buttons(int curr_x, int curr_y) {
	buttons_grp_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HBUTTON);
	buttons_grp_->box(FL_NO_BOX);
	// First button - connect/disconnect or add
	bn_connect_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Connect");
	bn_connect_->tooltip("Select to attempt to connect/disconnect rig");
	bn_connect_->callback(cb_bn_connect, nullptr);
	
	curr_x += bn_connect_->w();
	// Selects the rig or allows a rig to be selected
	bn_select_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Select");
	bn_select_->tooltip("Select the rig to connect");
	bn_select_->value(false);
	bn_select_->callback(cb_bn_select, nullptr);

	curr_x += bn_select_->w();
	// Start rig app
	bn_start_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Start");
	bn_start_->tooltip("Start rig app for this connection");
	bn_start_->callback(cb_bn_start, nullptr);

	buttons_grp_->end();

}

// Create rig and antenna choosers
void qso_rig::create_rig_ant(int curr_x, int curr_y) {
	rig_ant_grp_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HTEXT + HBUTTON * 3);
	rig_ant_grp_->box(FL_NO_BOX);

	curr_x += WBUTTON;
	// Choice - Select the rig model (Manufacturer/Model)
	ch_rig_model_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HTEXT, "Rig");
	ch_rig_model_->align(FL_ALIGN_LEFT);
	ch_rig_model_->tooltip("Select the model - for Hamlib");
	ch_rig_model_->callback(cb_ch_model, nullptr);

	// Populate the manufacturere and model choice widgets
	populate_model_choice();

	curr_y += HBUTTON;

	// field input to select an antenna from the ones previously logged or type in new
	ip_antenna_ = new field_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Antenna");
	ip_antenna_->align(FL_ALIGN_LEFT);
	ip_antenna_->callback(cb_value<field_input, string>, &antenna_);
	ip_antenna_->tooltip("Select the preferred antenna for this rig");
	ip_antenna_->field_name("MY_ANTENNA");
	ip_antenna_->value(antenna_.c_str());

	curr_y += HBUTTON;
	
	v_timeout_ = new Fl_Value_Slider(curr_x, curr_y, WSMEDIT, HBUTTON, "Timeout sec.");
	v_timeout_->align(FL_ALIGN_LEFT);
	v_timeout_->type(FL_HOR_SLIDER);
	v_timeout_->callback(cb_timeout);
	v_timeout_->tooltip("Set the value for timeing out the rig (0.1 to 5 s)");
	v_timeout_->range(0.1, 5.0);
	v_timeout_->step(0.1);
	v_timeout_->value(cat_data_[cat_index_]->hamlib ? cat_data_[cat_index_]->hamlib->timeout : 1.0);

	curr_y += HBUTTON;

	v_smeters_ = new Fl_Value_Slider(curr_x, curr_y, WSMEDIT, HBUTTON, "S-meter stack");
	v_smeters_->align(FL_ALIGN_LEFT);
	v_smeters_->type(FL_HOR_SLIDER);
	v_smeters_->callback(cb_smeters);
	v_smeters_->tooltip("Set the number of s-meter samples to provide peak");
	v_smeters_->range(1, 20);
	v_smeters_->step(1);
	v_smeters_->value(cat_data_[cat_index_]->hamlib ? cat_data_[cat_index_]->hamlib->num_smeters : 5);

	rig_ant_grp_->end();


}

// Create tabbed form for configuration data
void qso_rig::create_config(int curr_x, int curr_y) {
	// Tabbed form
	config_tabs_ = new Fl_Tabs(curr_x, curr_y, 10, 10);
	config_tabs_->box(FL_BORDER_BOX);
	config_tabs_->callback(cb_config);
	config_tabs_->when(FL_WHEN_CHANGED);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	config_tabs_->client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;
	curr_x = rx;
	curr_y = ry;
	// Create connection tab
	create_connex(curr_x, curr_y);
	rw = max(rw, connect_tab_->x() + connect_tab_->w() - rx);
	rh = max(rh, connect_tab_->y() + connect_tab_->h() - ry);
	// Create mofifier tab
	create_modifier(curr_x, curr_y);
	rw = max(rw, modifier_tab_->x() + modifier_tab_->w() - rx);
	rh = max(rh, modifier_tab_->y() + modifier_tab_->h() - ry);
	config_tabs_->resizable(nullptr);
	config_tabs_->size(config_tabs_->w() + rw - saved_rw, config_tabs_->h() + rh - saved_rh);
	config_tabs_->end();
	curr_x = config_tabs_->x() + config_tabs_->w();
	curr_y = config_tabs_->y() + config_tabs_->h();

}

// Create form to configure the hamlib port connection
// Create two versions: 1 for serial ports and one for networked ports
void qso_rig::create_connex(int curr_x, int curr_y) {
	connect_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Connection");
	connect_tab_->labelsize(FL_NORMAL_SIZE + 2);
	curr_x += GAP;
	curr_y += GAP;
	int max_x = curr_x;
	int max_y = curr_y;
	// Create serial port
	create_serial(curr_x, curr_y);
	max_x = max(max_x, serial_grp_->x() + serial_grp_->w());
	max_y = max(max_y, serial_grp_->y() + serial_grp_->h());
	// Create network port
	create_network(curr_x, curr_y);
	max_x = max(max_x, network_grp_->x() + network_grp_->w());
	max_y = max(max_y, network_grp_->y() + network_grp_->h());

	connect_tab_->resizable(nullptr);
	connect_tab_->size(max_x - connect_tab_->x(), max_y - connect_tab_->y());

	connect_tab_->end();

}

// Create form for defining a serial port connection
void qso_rig::create_serial(int curr_x, int curr_y) {
	serial_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);
	serial_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	serial_grp_->box(FL_NO_BOX);

	curr_x = serial_grp_->x() + WLABEL;
	curr_y = serial_grp_->y();
	// Choice to select from available ports
	ch_port_name_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Port");
	ch_port_name_->align(FL_ALIGN_LEFT);
	ch_port_name_->callback(cb_ch_port, nullptr);
	ch_port_name_->tooltip("Select the comms port to use");

	// Calculate  width necessary for "All"
	int tw = 0;
	int th = 0;
	fl_measure("All", tw, th);
	int save_x = curr_x;
	curr_x += WBUTTON + GAP;
	// Check flag to populate above choice with all ports not just unused ones
	bn_all_ports_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All");
	bn_all_ports_->align(FL_ALIGN_RIGHT);
	bn_all_ports_->tooltip("Select all existing ports, not just those available");
	bn_all_ports_->callback(cb_bn_all, &use_all_ports_);
	populate_port_choice();
	curr_x += HBUTTON + tw + GAP;
	int max_x = curr_x;

	curr_x = save_x;
	curr_y += HBUTTON;
	int max_y = curr_y;
	// Choice to select from available baud rates
	ch_baud_rate_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HTEXT, "Baud rate");
	ch_baud_rate_->align(FL_ALIGN_LEFT);
	ch_baud_rate_->tooltip("Enter baud rate");
	ch_baud_rate_->callback(cb_ch_baud, nullptr);

	curr_x += WBUTTON + GAP;
	// Check flag to populate above choice with all baud rates not just 
	// those defined in hamlibs capabilities structure for the rig
	bn_all_rates_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "All");
	bn_all_rates_->align(FL_ALIGN_RIGHT);
	bn_all_rates_->tooltip("Allow full baud rate selection");
	bn_all_rates_->callback(cb_ch_over, &use_all_rates_);
	populate_baud_choice();
	curr_x += HBUTTON + tw;
	curr_x = max(max_x, curr_x);

	curr_y += HBUTTON + GAP;
	curr_y = max(max_y, curr_y);
	serial_grp_->resizable(nullptr);
	serial_grp_->size(curr_x - serial_grp_->x(), curr_y - serial_grp_->y());

	serial_grp_->end();

}

// Create for to configure a network connection
void qso_rig::create_network(int curr_x, int curr_y) {
	network_grp_ = new Fl_Group(curr_x, curr_y, 10, 10);
	network_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	network_grp_->box(FL_NO_BOX);

	curr_x = network_grp_->x();
	curr_y = network_grp_->y();

	bn_use_app_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Use app");
	bn_use_app_->callback(cb_bn_use_app, &cat_data_[cat_index_]->use_cat_app);
	bn_use_app_->tooltip("Set to allow a CAT app to be used");
	bn_use_app_->value(cat_data_[cat_index_]->use_cat_app);

	curr_x += WBUTTON;

	// App name (flrig or wfview to connect to rig
	ip_app_name_ = new Fl_Input(curr_x, curr_y, w() - curr_x - (2 * GAP), HTEXT);
	ip_app_name_->callback(cb_value<Fl_Input, string>, &cat_data_[cat_index_]->app);
	ip_app_name_->tooltip("Please provide the command to use to connect");
	ip_app_name_->value("");

	curr_y += HTEXT;

	// Input to type in network address and port number
	ip_port_ = new Fl_Input(curr_x, curr_y, ip_app_name_->w(), HTEXT, "Host:Port");
	ip_port_->align(FL_ALIGN_LEFT);
	ip_port_->callback(cb_ip_port, nullptr);
	ip_port_->tooltip("Enter the network/USB port to use");
	ip_port_->value(cat_data_[cat_index_]->hamlib->port_name.c_str());

	curr_y += HTEXT + GAP;

	curr_x += ip_app_name_->w();
	network_grp_->resizable(nullptr);
	network_grp_->size(curr_x - network_grp_->x(), curr_y - network_grp_->y());

	network_grp_->end();

}

// Create the form to configure any rig add-ons (transverter or amplifier)
void qso_rig::create_modifier(int curr_x, int curr_y) {
	modifier_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Transverter/Amp");
	modifier_tab_->labelsize(FL_NORMAL_SIZE + 2);

	curr_x = modifier_tab_->x() + WBUTTON;
	curr_y += (HTEXT + GAP)/2;

	// Check flag to enable frequency to be offset (for a transverter)
	bn_mod_freq_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "\316\224F (MHz)");
	bn_mod_freq_->align(FL_ALIGN_LEFT);
	bn_mod_freq_->callback(cb_bn_mod_freq, (void*)&modify_freq_);
	bn_mod_freq_->tooltip("Allow frequency to be offset - eg transverter");
	bn_mod_freq_->value(modify_freq_);
	
	curr_x += WRADIO;
	// Input for the frequency offset (in MHz)
	ip_freq_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_freq_->callback(cb_ip_freq, (void*)&freq_offset_);
	ip_freq_->tooltip("Enter frequency offset");
	ip_freq_->when(FL_WHEN_CHANGED);
	char text[20];
	snprintf(text, sizeof(text), "%g", freq_offset_);
	ip_freq_->value(text);

	curr_y += HBUTTON;
	curr_x = modifier_tab_->x() + WBUTTON;

	// Check flag to enable the power to be adjusted 
	bn_gain_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Gain (dB)");
	bn_gain_->align(FL_ALIGN_LEFT);
	bn_gain_->callback(cb_bn_power, (void*)(intptr_t)false);
	bn_gain_->tooltip("Allow amplifier");
	bn_gain_->value(modify_gain_);

	curr_x += WRADIO;
	// Input for the amplifier gain
	ip_gain_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_gain_->callback(cb_ip_gain, (void*)&gain_);
	ip_gain_->when(FL_WHEN_CHANGED);
	ip_gain_->tooltip("Specify the amplifier gain (dB)");
	snprintf(text, sizeof(text), "%d", gain_);
	ip_gain_->value(text);

	curr_y += HBUTTON;
	curr_x = modifier_tab_->x() + WBUTTON;

	// Check flag to define a fixed power output (for rigs that do not supply this)
	bn_power_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Pwr (W)");
	bn_power_->align(FL_ALIGN_LEFT);
	bn_power_->callback(cb_bn_power, (void*)(intptr_t)true);
	bn_power_->tooltip("Specify absolute power");
	bn_power_->value(modify_power_);

	curr_x += WRADIO;
	// Input for the rig power output
	ip_power_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	ip_power_->callback(cb_ip_power, (void*)&power_);
	ip_power_->when(FL_WHEN_CHANGED);
	ip_power_->tooltip("Specify power out ");
	snprintf(text, sizeof(text), "%g", power_);
	ip_power_->value(text);

	curr_x += WBUTTON;
	curr_y += HBUTTON + GAP;
	modifier_tab_->resizable(nullptr);
	modifier_tab_->size(curr_x - modifier_tab_->x(), curr_y - modifier_tab_->y());

	modifier_tab_->end();
}

// Create the overall form
void qso_rig::create_form(int X, int Y) {

	begin();

	size(WBUTTON * 3 + GAP * 2, h());

	int curr_x = X + GAP;
	int curr_y = Y + 1;

	// Create the frqeuency and mode display
	create_status(curr_x, curr_y);

	curr_y = status_grp_->y() + status_grp_->h();
	
	// Create the control buttons
	create_buttons(curr_x, curr_y);

	curr_y = buttons_grp_->y() + buttons_grp_->h() + GAP;

	// Create the rig and antenna choices
	create_rig_ant(curr_x, curr_y);
	curr_y = rig_ant_grp_->y() + rig_ant_grp_->h() + GAP;

	// Create the configuration controls
	create_config(curr_x, curr_y);

	curr_y = config_tabs_->y() + config_tabs_->h();

	// Connected status

	// // Display hamlib ot flrig settings as selected
	// modify_rig();
	// enable_widgets();

	resizable(nullptr);
	size(w(), curr_y + GAP - y());
	end();
}

void qso_rig::save_cat_data(qso_rig::cat_data_t* cat_data, Fl_Preferences settings) {
	settings.set("Rig Model", cat_data->hamlib->model.c_str());
	settings.set("Manufacturer", cat_data->hamlib->mfr.c_str());
	settings.set("Port", cat_data->hamlib->port_name.c_str());
	settings.set("Baud Rate", cat_data->hamlib->baud_rate);
	settings.set("Model ID", (int)cat_data->hamlib->model_id);
	settings.set("Timeout", cat_data->hamlib->timeout);
	if (cat_data->use_cat_app) {
		settings.set("Command", cat_data->app.c_str());
	}
}

// Save values in settings
void qso_rig::save_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	if (cat_data_[cat_index_]->hamlib->port_type != RIG_PORT_NONE) {
		Fl_Preferences cat_settings(settings_, "CAT");
		Fl_Preferences rig_settings(cat_settings, label());
		rig_settings.clear();
		if (cat_data_.size() > 1) {
			for (int ix = 0; ix < cat_data_.size(); ix++) {
				char g[5];
				snprintf(g, sizeof(g), "%d", ix);
				Fl_Preferences cat_settings(rig_settings, g);
				save_cat_data(cat_data_[ix], cat_settings);
			}
		}
		else {
			save_cat_data(cat_data_[0], rig_settings);
		}
		rig_settings.set("Default CAT", cat_index_);
		// Preferred antenna
		rig_settings.set("Antenna", antenna_.c_str());
		// Modifier settings
		if (modify_freq_) {
			rig_settings.set("Frequency", freq_offset_);
		}
		if (modify_gain_) {
			rig_settings.set("Gain", gain_);
		}
		if (modify_power_) {
			rig_settings.set("Power", power_);
		}
		settings_->flush();
	}
}

// Enable CAT Connection widgets
void qso_rig::enable_widgets(bool tick) {
	// CAT access buttons
	if (!rig_) {
		// No rig
		bn_connect_->deactivate();
		// bn_connect_->color(FL_BACKGROUND_COLOR);
		bn_connect_->label("");
		bn_select_->activate();
		if (bn_select_->value()) {
			bn_select_->label("Use");
		} else {
			bn_select_->label("Select");
		}
		if (cat_data_[cat_index_]->use_cat_app) bn_start_->activate();
		else bn_start_->deactivate();
	} else if (rig_->is_open()) {
		// Rig is connected
		bn_connect_->activate();
		// bn_connect_->color(fl_lighter(FL_RED));
		bn_connect_->label("Disconnect");
		bn_select_->deactivate();
		bn_select_->label("");
		bn_select_->value(false);
		bn_start_->deactivate();
	} else {
		// Rig is not connected
		bn_connect_->activate();
		// bn_connect_->color(fl_lighter(FL_GREEN));
		bn_connect_->label("Connect");
		bn_select_->activate();
		if (bn_select_->value()) {
			bn_select_->label("Use");
		} else {
			bn_select_->label("Select");
		}
		if (cat_data_[cat_index_]->use_cat_app) bn_start_->activate();
		else bn_start_->deactivate();
	}
	bn_connect_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_connect_->color()));
	bn_select_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_select_->color()));
	double freq;
	// Status and frequency/mode
	if (!rig_) {
		// No rig - decativate freq/mode
		op_status_->value("No rig specified");
		bn_tx_rx_->label("");
		bn_tx_rx_->color(FL_BLACK);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	} else if (rig_->is_opening()) {
		// Rig is connecting - deactivate freq/mode
		op_status_->value("Opening rig");
		bn_tx_rx_->label("");
		bn_tx_rx_->color(FL_YELLOW);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	} else if (rig_->is_open()) {
		// Rig is connected
		if (!rig_->get_powered()) {
			op_status_->value("Powered down");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(COLOUR_ORANGE);
		}
		else if (rig_->get_slow()) {
			// Do no change display
			op_status_->value("Unresponsive");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(COLOUR_ORANGE);
		}
		else {
			freq = rig_->get_dfrequency(true);
			int freq_Hz = (int)(freq * 1000000) % 1000;
			int freq_kHz = (int)(freq * 1000) % 1000;
			int freq_MHz = (int)freq;
			band_data::band_entry_t* entry = band_data_->get_entry(freq * 1000);
			if (entry) {
				char l[50];
				snprintf(l, sizeof(l), "%s (%s)", 
					spec_data_->band_for_freq(freq).c_str(),
					entry->mode.c_str());
				op_status_->value(l);
				if (rig_->get_ptt()) {
					bn_tx_rx_->label("TX");
					bn_tx_rx_->color(FL_RED);
				} else {
					bn_tx_rx_->label("RX");
					bn_tx_rx_->color(FL_GREEN);
				}
			}
			else {
				op_status_->value("Out of band!");
				if (rig_->get_ptt()) {
					bn_tx_rx_->label("TX");
					bn_tx_rx_->color(FL_DARK_RED);
				} else {
					bn_tx_rx_->label("RX");
					bn_tx_rx_->color(FL_DARK_GREEN);
				}
			}
			op_freq_mode_->activate();
			op_freq_mode_->color(FL_BLACK);
			op_freq_mode_->labelcolor(FL_YELLOW);

			char msg[200];
			string rig_mode;
			string submode;
			rig_->get_string_mode(rig_mode, submode);
			// Set Freq/Mode to Frequency (MHz with kHz seperator), mode, power (W)
			snprintf(msg, sizeof(msg), "%d.%03d.%03d MHz\n%s %sW %s" , 
				freq_MHz, freq_kHz, freq_Hz,
				submode.length() ? submode.c_str() : rig_mode.c_str(),
				rig_->get_tx_power(true).c_str(),
				rig_->get_smeter(true).c_str()
			);
			op_freq_mode_->copy_label(msg);
			int size = FL_NORMAL_SIZE + 10;
			fl_font(0, size);
			int w, h;
			fl_measure(msg, w, h);
			while (w > op_freq_mode_->w()) {
				size--;
				fl_font(0, size);
				fl_measure(msg, w, h);
			}
			op_freq_mode_->labelsize(size);
		}
	} else if (rig_->has_no_cat()) {
		// No rig available - deactivate freq/mode
		op_status_->value("No CAT Available");
		bn_tx_rx_->label("");
		bn_tx_rx_->color(COLOUR_GREY);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	} else {
		if (rig_ok_) {
			// Rig has disconnected since last update
			char msg[128];
			if (rig_->is_network_error()) {
				snprintf(msg, sizeof(msg), "RIG: Failed to access network app for %s",
				label());
			} else if (rig_->is_rig_error()) {
				snprintf(msg, sizeof(msg), "Failed to access rig %s", label());
			} else {
				strcpy(msg, rig_->error_message(label()).c_str());
			}
			status_->misc_status(ST_WARNING, msg);
			rig_ok_ = false;
		}
		// Rig is not connected - deactivate freq/mode
		op_status_->value("Disconnected");
		bn_tx_rx_->label("");
		bn_tx_rx_->color(FL_BLACK);
		op_freq_mode_->deactivate();
		op_freq_mode_->label("");
	}
	bn_tx_rx_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_tx_rx_->color()));
	// CAT control widgets - allow only when select button active
	if (rig_->is_open()) {
		// Rig is open - disable connection configuration controls
		ch_rig_model_->deactivate();
		serial_grp_->deactivate();
		network_grp_->deactivate();
	} else {
		// Rig is not open - allow it to be configured
		ch_rig_model_->activate();
		serial_grp_->activate();
		network_grp_->activate();
		if (bn_select_->value()) {
			// Set configuration widgets to accept input
			serial_grp_->clear_output();
			network_grp_->clear_output();
		} else {
			// Set configuration widgets to inhibit input
			serial_grp_->set_output();
			network_grp_->set_output();
		}
	}
	switch ((rig_mode_t)mode_) {
	case RIG_PORT_SERIAL:
		// Serial port - show serial configuration, hide network
		serial_grp_->show();
		network_grp_->hide();
		if (!bn_select_->value()) {
			serial_grp_->deactivate();
 		} else {
			serial_grp_->activate();
		}
		break;
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		// Network or USB port - show network, hide serial
		serial_grp_->hide();
		network_grp_->show();
		if (!bn_select_->value()) {
			ip_port_->value(cat_data_[cat_index_]->hamlib->port_name.c_str());
			ip_port_->user_data(&cat_data_[cat_index_]->hamlib->port_name);
			network_grp_->deactivate();
 		} else {
			network_grp_->activate();
		}
		bn_use_app_->value(cat_data_[cat_index_]->use_cat_app);
		// Chaneg the user data for "use app" button
		bn_use_app_->user_data(&cat_data_[cat_index_]->use_cat_app);
		if (!tick) ip_app_name_->value(cat_data_[cat_index_]->app.c_str());
		// The hamlib model has an associated application (eg flrig or wfview)
		if (cat_data_[cat_index_]->use_cat_app) {
			ip_app_name_->activate();
			ip_app_name_->user_data(&cat_data_[cat_index_]->app);
		} else {
			ip_app_name_->deactivate();
		}
		break;
	case RIG_PORT_NONE:
		// Hide both sets of configuration
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
	// Now use standard TAB highlighting
	for (int ix = 0; ix < config_tabs_->children(); ix++) {
		Fl_Widget* wx = config_tabs_->child(ix);
		if (wx == config_tabs_->value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}
	// Label the CAT method selection with the CAT method
	char l[5];
	snprintf(l, sizeof(l), "CAT%d", cat_index_ + 1);
	bn_index_->copy_label(l);

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
			// The '/' ensures all rigs from same manufacturer are in a 
			// a sub-menu to the manufacturer
			string mfg = escape_menu(capabilities->mfg_name);
			string model = escape_menu(capabilities->model_name);
			if (model.length() == 0) {
				model = mfg;
				mfg = "Other";
			} 
			if (mfg.length() == 0) {
				mfg = "Other";
			}
			snprintf(temp, 256, "%s/%s%s", mfg.c_str(), model.c_str(), status);
			rig_list.insert(temp);
			rig_ids[temp] = capabilities->rig_model;
			if (string(capabilities->mfg_name) == cat_data_[cat_index_]->hamlib->mfr &&
				string(capabilities->model_name) == cat_data_[cat_index_]->hamlib->model) {
				cat_data_[cat_index_]->hamlib->model_id = capabilities->rig_model;
				cat_data_[cat_index_]->hamlib->port_type = capabilities->port_type;
			}
		}
	}
	// Add the rigs in alphabetical order to the choice widget,
	// set widget's value to intended
	ch_rig_model_->add("");
	for (auto ix = rig_list.begin(); ix != rig_list.end(); ix++) {
		string name = *ix;
		rig_model_t id = rig_ids.at(name);
		// Add the id as user data for the menu item
		int pos = ch_rig_model_->add(name.c_str(), 0, nullptr, (void*)(intptr_t)id);
		if (id == cat_data_[cat_index_]->hamlib->model_id) {
			ch_rig_model_->value(pos);
		}
	}
	bool found = false;
}

// Populate the choice with the available ports
void qso_rig::populate_port_choice() {
	if (cat_data_[cat_index_]->hamlib->port_type == RIG_PORT_SERIAL) {
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
			if (strcmp(port, cat_data_[cat_index_]->hamlib->port_name.c_str()) == 0) {
				ch_port_name_->value(i);
			}
		}
	}
}

// Populate the baud rate choice menu
void qso_rig::populate_baud_choice() {
	if (cat_data_[cat_index_]->hamlib->port_type == RIG_PORT_SERIAL) {
		ch_baud_rate_->clear();
		// Override rig's capabilities?
		bn_all_rates_->value(use_all_rates_);

		// Get the baud-rates supported by the rig
		const rig_caps* caps = rig_get_caps(cat_data_[cat_index_]->hamlib->model_id);
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
				if (rate == cat_data_[cat_index_]->hamlib->baud_rate) {
					ch_baud_rate_->value(index);
					index++;
				}
			}
		}
	}
}

// Populate the cat select drop-down menu
void qso_rig::populate_index_menu() {
	bn_index_->clear();
	// Add a menu item for each CAT if more than 1
	if (cat_data_.size() > 1) {
		for (int ix = 0; ix < cat_data_.size(); ix++) {
			char l[5];
			snprintf(l, sizeof(l), "CAT%d", ix + 1);
			bn_index_->add(l, 0, cb_select_cat, (void*)(intptr_t)ix);
		}
	}
	// Add a menu item to add a new CAT
	bn_index_->add("New", 0, cb_new_cat);
}

// Model input choice selected
// v is not used
void qso_rig::cb_ch_model(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	qso_rig* that = ancestor_view<qso_rig>(w);
	const Fl_Menu_Item* item = ch->mvalue();
	// The rig id was passed as user data to the menu item when it was added
	rig_model_t id = (intptr_t)item->user_data();
	const char* label = ch->text();
	const rig_caps* capabilities = rig_get_caps(id);
	hamlib_data_t* hamlib = that->cat_data_[that->cat_index_]->hamlib;
	if (capabilities != nullptr) {
		hamlib->model = capabilities->model_name;
		hamlib->mfr = capabilities->mfg_name;
		hamlib->model_id = id;
		hamlib->port_type = capabilities->port_type;
	}
	else {
		char message[128];
		snprintf(message, 128, "DASH: Error reading hamlib details selecting %s", label);
		status_->misc_status(ST_ERROR, message);
	}
	switch (hamlib->port_type) {
	case RIG_PORT_SERIAL:
	case RIG_PORT_NETWORK:
	case RIG_PORT_USB:
		that->mode_ = hamlib->port_type;
		break;
	case RIG_PORT_NONE:
		// Retain mode = NONE until the add button is clicked
		break;
	}
	that->populate_baud_choice();
	that->populate_port_choice();
	that->enable_widgets();
}

// Callback selecting serial port
// v is unused
void qso_rig::cb_ch_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_text<Fl_Choice, string>(w, (void*)&that->cat_data_[that->cat_index_]->hamlib->port_name);
}

// Callback entering named port
// v is unused
void qso_rig::cb_ip_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Input, string>(w, (void*)&that->cat_data_[that->cat_index_]->hamlib->port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_rig::cb_ch_baud(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	const char* text = ((Fl_Choice*)w)->text();
	that->cat_data_[that->cat_index_]->hamlib->baud_rate = atoi(text);
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
	if (!that->rig_) {
		that->rig_ = new rig_if(that->label(), that->cat_data_[that->cat_index_]->hamlib);
		if (that->rig_->is_good()) that->rig_ok_ = true;
	}
	else if (that->rig_->is_open()) {
		that->rig_->close();
		that->rig_ok_ = false;

	} else {
		that->rig_->open();
		if (that->rig_->is_good()) that->rig_ok_ = true;
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
		}
		else {
			that->modify_power_ = false;
			that->modify_gain_ = false;
		}
	}
	else {
		bool v = that->bn_gain_->value();
		if (v) {
			that->modify_power_ = false;
			that->modify_gain_ = true;
		}
		else {
			that->modify_power_ = false;
			that->modify_gain_ = false;
		}
	}
	that->modify_rig();
	that->enable_widgets();
}

// Entered gain value
// v points the the gain attribute
void qso_rig::cb_ip_gain(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value_int<Fl_Int_Input>(w, v);
	that->modify_rig();
	that->enable_widgets();
}

// Entered power value
// v points to the absolute power attribute
void qso_rig::cb_ip_power(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	// A frig to convert the float value of the widget to the double attribute
	float f;
	cb_value_float<Fl_Float_Input>(w, &f);
	*(double*)v = (double)f;
	that->modify_rig();
	that->enable_widgets();
}

// Clicked start button
// v points to the string containing the command to  invoke flrig
void qso_rig::cb_bn_start(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	string command = that->cat_data_[that->cat_index_]->app + "&";
	int result = system(command.c_str());
	char msg[100];
	if (result == 0) {
		snprintf(msg, sizeof(msg), "RIG: Started %s OK", command.c_str());
		status_->misc_status(ST_OK, msg);
	}
	else {
		snprintf(msg, sizeof(msg), "RIG: Failed to start %s", command.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Selecting config tab
// v is not used
void qso_rig::cb_config(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets();
}

// Use application button
void qso_rig::cb_bn_use_app(Fl_Widget* w, void* v) {
	cb_value<Fl_Light_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets();
}

// Timeout callback
void qso_rig::cb_timeout(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Value_Slider, double>(w, &that->cat_data_[that->cat_index_]->hamlib->timeout);
}

// Smeter stack length
void qso_rig::cb_smeters(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	double value;
	cb_value<Fl_Value_Slider, double>(w, &value);
	that->cat_data_[that->cat_index_]->hamlib->num_smeters = (int)value;;
}

// Use selected CAT
void qso_rig::cb_select_cat(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->cat_index_ = (int)(intptr_t)v;
	that->switch_rig();
}

// Generate a new CAT
void qso_rig::cb_new_cat(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cat_data_t* data = new cat_data_t;
	data->hamlib = new hamlib_data_t;
	that->cat_data_.push_back(data);
	that->populate_index_menu();
	that->switch_rig();
}

// Connect rig if disconnected and vice-versa
void qso_rig::switch_rig() {
	if (rig_) {
		delete rig_;
		rig_ = nullptr;
		rig_ = new rig_if(label(), cat_data_[cat_index_]->hamlib);
		const rig_caps* capabilities = rig_get_caps(cat_data_[cat_index_]->hamlib->model_id);
		if (capabilities && !(capabilities->has_get_level & RIG_LEVEL_RFPOWER_METER_WATTS)) {
			char msg[128];
			snprintf(msg, sizeof(msg), "DASH: Rig %s does not supply power - set it",
				cat_data_[cat_index_]->hamlib->model.c_str());
			status_->misc_status(ST_WARNING, msg);
			modify_power_ = true;
			modify_gain_ = false;
		}
	}
	ancestor_view<qso_manager>(this)->update_rig();
	modify_rig();
	enable_widgets();
}

// 1 s clock interface - read rig and update status
void qso_rig::ticker() {
	if (rig_) {
		// The rig may have disconnected - update connect/select buttons
		enable_widgets(true);
	}
}

// Static 1s ticker
void qso_rig::cb_ticker(void* v) {
	// temporarily remove onself from the ticker list as this may take for than 1 s
	ticker_->activate_ticker(v, false);
	((qso_rig*)v)->ticker();
	ticker_->activate_ticker(v, true);
}

// New rig 
void qso_rig::new_rig() {
	mode_ = (uint16_t)cat_data_[cat_index_]->hamlib->port_type;
	enable_widgets();
}

// Return pointer to rig_if object
rig_if* qso_rig::rig() {
	return rig_;
}

// Return the colour used in the connect button as its alert
Fl_Color qso_rig::alert_colour() {
	return bn_connect_->color();
}

// Modify the values returned from the rig according to the modifier attributes
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

// Return the preferred antenna
string qso_rig::antenna() {
	return antenna_;
}

// Force disconnect
void qso_rig::disconnect() {
	if (rig_ && rig_->is_open()) {
		rig_->close();
	}
}

// Return selected CAT
const char* qso_rig::cat() {
	char* result = new char[5];
	if (cat_data_.size() <= 1) {
		strcpy(result, "");
	}
	else {
		snprintf(result, 5, "CAT%d", cat_index_ + 1);
	}
	return result;
}

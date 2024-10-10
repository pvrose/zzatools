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
	rig_ok_(false),
	rig_state_(NO_RIG),
	cat_index_(-1)
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
	if (cat_data_.size()) {
		rig_ = new rig_if(label(), cat_data_[cat_index_]->hamlib);
		if (rig_->is_good()) rig_ok_ = true;
		if (rig_ok_) modify_hamlib_data();
	}
	else {
		rig_ = nullptr;
	}
	create_form(X, Y);
	enable_widgets(DAMAGE_ALL);

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

	// Read non-hamlib CAT data
	int itemp;
	settings.get("Power Mode", itemp, (int)power_mode_t::RF_METER);
	cat_data->hamlib->power_mode = (power_mode_t)itemp;
	settings.get("Maximum Power", cat_data->hamlib->max_power, 0.0);
	settings.get("Frequency Mode", itemp, (int)freq_mode_t::VFO);
	cat_data->hamlib->freq_mode = (freq_mode_t)itemp;
	settings.get("Crystal Frequency", cat_data->hamlib->frequency, 0.0);
	settings.get("Amplifier Gain", cat_data->hamlib->gain, 0);
	settings.get("Transverter Offset", cat_data->hamlib->freq_offset, 0.0);
	settings.get("Transverter Power", cat_data->hamlib->tvtr_power, 0.0);
	settings.get("Accessories", itemp, (int)BAREBACK);
	cat_data->hamlib->accessory = (accessory_t)itemp;
	
	free(temp);

}

// Get initial data from settings
void qso_rig::load_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	Fl_Preferences cat_settings(settings_, "CAT");
	if (cat_settings.groupExists(label())) {
		Fl_Preferences rig_settings(cat_settings, label());
		for (int ix = 0; ix < rig_settings.groups(); ix++) {
			cat_data_t* data = new cat_data_t;
			data->hamlib = new hamlib_data_t;
			char g[5];
			snprintf(g, sizeof(g), "%d", ix);
			Fl_Preferences cat_settings(rig_settings, g);
			load_cat_data(data, cat_settings);
			cat_data_.push_back(data);
		}
		char* temp;
		// Get default cat type
		rig_settings.get("Default CAT", cat_index_, cat_index_);
		if (cat_index_ >= cat_data_.size()) cat_index_ = cat_data_.size() - 1;
		// Preferred antenna
		rig_settings.get("Antenna", temp, "");
		antenna_ = temp;
		free(temp);

		mode_ = cat_data_.size() ? 
			(uint16_t)cat_data_[cat_index_]->hamlib->port_type :
			(uint16_t)RIG_PORT_NONE;
	}
	else {
		// New hamlib data
		cat_data_.clear();
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
	rig_ant_grp_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HTEXT + HBUTTON);
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

	rig_ant_grp_->end();


}

// Create tabbed form for configuration data
void qso_rig::create_config(int curr_x, int curr_y) {
	// Tabbed form
	config_tabs_ = new Fl_Tabs(curr_x, curr_y, 10, 10);
	config_tabs_->box(FL_BORDER_BOX);
	config_tabs_->callback(cb_config);
	config_tabs_->when(FL_WHEN_CHANGED);
	config_tabs_->handle_overflow(Fl_Tabs::OVERFLOW_PULLDOWN);

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
	// Create defaults tab
	create_defaults(curr_x, curr_y);
	rw = max(rw, defaults_tab_->x() + defaults_tab_->w() - rx);
	rh = max(rh, defaults_tab_->y() + defaults_tab_->h() - ry);
	// Create accessory tab
	create_accessory(curr_x, curr_y);
	rw = max(rw, accessory_tab_->x() + accessory_tab_->w() - rx);
	rh = max(rh, accessory_tab_->y() + accessory_tab_->h() - ry);
	// Create timeout &c tab
	create_timeout(curr_x, curr_y);
	rw = max(rw, timeout_tab_->x() + timeout_tab_->w() - rx);
	rh = max(rh, timeout_tab_->y() + timeout_tab_->h() - ry);

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
	bn_use_app_->callback(cb_bn_use_app, nullptr);
	bn_use_app_->tooltip("Set to allow a CAT app to be used");
	bn_use_app_->value(false);

	curr_x += WBUTTON;

	// App name (flrig or wfview to connect to rig
	ip_app_name_ = new Fl_Input(curr_x, curr_y, w() - curr_x - (2 * GAP), HTEXT);
	ip_app_name_->callback(cb_value<Fl_Input, string>, nullptr);
	ip_app_name_->tooltip("Please provide the command to use to connect");
	ip_app_name_->value("");

	curr_y += HTEXT;

	// Input to type in network address and port number
	ip_port_ = new Fl_Input(curr_x, curr_y, ip_app_name_->w(), HTEXT, "Host:Port");
	ip_port_->align(FL_ALIGN_LEFT);
	ip_port_->callback(cb_ip_port, nullptr);
	ip_port_->tooltip("Enter the network/USB port to use");
	ip_port_->value("");

	curr_y += HTEXT + GAP;

	curr_x += ip_app_name_->w();
	network_grp_->resizable(nullptr);
	network_grp_->size(curr_x - network_grp_->x(), curr_y - network_grp_->y());

	network_grp_->end();

}

// Create the defaults tab to specify power and frequency if rig cannot supply
void qso_rig::create_defaults(int curr_x, int curr_y) {
	defaults_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Defaults");
	defaults_tab_->labelsize(FL_NORMAL_SIZE + 2);

	curr_x += GAP;
	curr_y += 15;

	int save_y = curr_y;

	op_pwr_type_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Power");
	op_pwr_type_->box(FL_FLAT_BOX);
	op_pwr_type_->align(FL_ALIGN_TOP);
	op_pwr_type_->tooltip("Shows how the power is calculated.");

	curr_y += HBUTTON + 15;
	ip_max_pwr_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Max. - W");
	ip_max_pwr_->align(FL_ALIGN_TOP);
	ip_max_pwr_->callback(cb_value_double<Fl_Float_Input>, nullptr);
	ip_max_pwr_->tooltip("Specify the maximum power out from the rig");

	int max_y = curr_y + HBUTTON + GAP;

	curr_y = save_y;
	curr_x += WBUTTON + GAP;

	op_freq_type_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Frequency");
	op_freq_type_->box(FL_FLAT_BOX);
	op_freq_type_->align(FL_ALIGN_TOP);
	op_freq_type_->tooltip("Shows hoe the frequency is generated");

	curr_y += HBUTTON + 15;
	ip_xtal_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Fixed - MHz");
	ip_xtal_->align(FL_ALIGN_TOP);
	ip_xtal_->callback(cb_value_double<Fl_Float_Input>, nullptr);
	ip_xtal_->tooltip("Provide a fixed frequency - eg crystal");
	max_y = max(max_y, curr_y + HBUTTON + GAP);
	int max_x = curr_x + WBUTTON + GAP;

	defaults_tab_->resizable(nullptr);
	defaults_tab_->size(max_x - defaults_tab_->x(), max_y - defaults_tab_->y());
	defaults_tab_->end();

}

// Create the tab for the accessories
void qso_rig::create_accessory(int curr_x, int curr_y) {
	accessory_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Accessories");
	accessory_tab_->labelsize(FL_NORMAL_SIZE + 2);

	curr_x += GAP;
	curr_y += GAP;
	int save_y = curr_y;

	bn_amplifier_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Amplifier");
	bn_amplifier_->box(FL_FLAT_BOX);
	bn_amplifier_->callback(cb_bn_amplifier, nullptr);
	bn_amplifier_->tooltip("Select whether an ammplifier is fitted");

	curr_y += HBUTTON + 15;
	ip_gain_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Gain - dB");
	ip_gain_->align(FL_ALIGN_TOP);
	ip_gain_->callback(cb_value_int<Fl_Int_Input>, nullptr);
	ip_gain_->tooltip("Specify the amplifier gain in decibels");

	int max_y = curr_y = HBUTTON + GAP;

	curr_y = save_y;
	curr_x += WBUTTON + GAP;

	bn_transverter_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Transverter");
	bn_transverter_->box(FL_FLAT_BOX);
	bn_transverter_->callback(cb_bn_transverter, nullptr);
	bn_transverter_->tooltip("Select to add a transverter");

	curr_y += HBUTTON + 15;
	ip_offset_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Offset - MHz");
	ip_offset_->align(FL_ALIGN_TOP);
	ip_offset_->callback(cb_value_double<Fl_Float_Input>, nullptr);
	ip_offset_->tooltip("Specify the Transverter frequency offset to apply");

	curr_y += HBUTTON + 15;
	ip_tvtr_pwr_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Power - W");
	ip_tvtr_pwr_->align(FL_ALIGN_TOP);
	ip_tvtr_pwr_->callback(cb_value_double<Fl_Float_Input>, nullptr);
	ip_tvtr_pwr_->tooltip("Specify the transverter power output");

	max_y = max(max_y, curr_y + HBUTTON + GAP);
	int max_x = curr_x + WBUTTON + GAP;

	accessory_tab_->resizable(nullptr);
	accessory_tab_->size(max_x - accessory_tab_->x(), max_y - accessory_tab_->y());
	accessory_tab_->end();
}

// Cretae a tab to specify the timeout when the rig disappears.
void qso_rig::create_timeout(int curr_x, int curr_y) {
	timeout_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Timeout etc.");
	timeout_tab_->labelsize(FL_NORMAL_SIZE + 2);

	curr_x = timeout_tab_->x() + WBUTTON + GAP;
	curr_y += (HTEXT + GAP) / 2;

	v_timeout_ = new Fl_Value_Slider(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON, "Timeout sec.");
	v_timeout_->align(FL_ALIGN_LEFT);
	v_timeout_->type(FL_HOR_SLIDER);
	v_timeout_->callback(cb_timeout);
	v_timeout_->tooltip("Set the value for timeing out the rig (0.1 to 5 s)");
	v_timeout_->range(0.1, 5.0);
	v_timeout_->step(0.1);
	v_timeout_->value(1.0);

	curr_y += HBUTTON;

	v_smeters_ = new Fl_Value_Slider(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON, "S-meter stack");
	v_smeters_->align(FL_ALIGN_LEFT);
	v_smeters_->type(FL_HOR_SLIDER);
	v_smeters_->callback(cb_smeters);
	v_smeters_->tooltip("Set the number of s-meter samples to provide peak");
	v_smeters_->range(1, 20);
	v_smeters_->step(1);
	v_smeters_->value(5);

	curr_x += v_smeters_->w();
	curr_y += HBUTTON + GAP;
	timeout_tab_->resizable(nullptr);
	timeout_tab_->size(curr_x - timeout_tab_->x(), curr_y - timeout_tab_->y());

	timeout_tab_->end();

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
	// Read non-hamlib CAT data
	settings.set("Power Mode", cat_data->hamlib->power_mode);
	settings.set("Maximum Power", cat_data->hamlib->max_power, 0.0);
	settings.set("Frequency Mode", cat_data->hamlib->freq_mode);
	settings.set("Crystal Frequency", cat_data->hamlib->frequency);
	settings.set("Amplifier Gain", cat_data->hamlib->gain);
	settings.set("Transverter Offset", cat_data->hamlib->freq_offset);
	settings.set("Transverter Power", cat_data->hamlib->tvtr_power);
	settings.set("Accessories", cat_data->hamlib->accessory);

}

// Save values in settings
void qso_rig::save_values() {
	// Read hamlib configuration - manufacturer,  model, port and baud-rate
	Fl_Preferences cat_settings(settings_, "CAT");
	if (cat_data_.size()) {
		Fl_Preferences rig_settings(cat_settings, label());
		rig_settings.clear();
		for (int ix = 0; ix < cat_data_.size(); ix++) {
			char g[5];
			snprintf(g, sizeof(g), "%d", ix);
			Fl_Preferences cat_type_settings(rig_settings, g);
			save_cat_data(cat_data_[ix], cat_type_settings);
		}
		rig_settings.set("Default CAT", cat_index_);
		// Preferred antenna
		rig_settings.set("Antenna", antenna_.c_str());
		settings_->flush();
	} else {
		// Remove this rig from the settings file
		cat_settings.delete_group(label());
	}
}

// get rig sttaus
qso_rig::rig_state_t qso_rig::rig_state() {
	if (!rig_) 
		if (rig_starting_) return STARTING;
		else return NO_RIG;
	else if (rig_->is_opening()) return OPENING;
	else if (rig_->is_open())
		if (!rig_->get_powered()) return POWERED_DOWN;
		else if (rig_->get_slow()) return UNRESPONSIVE;
		else return OPEN;
	else if (rig_->has_no_cat()) return NO_CAT;
	else if (rig_starting_) return STARTING;
	else return DISCONNECTED;
}

// Enable CAT Connection widgets
void qso_rig::enable_widgets(uchar damage) {
	cat_data_t* cat_data = cat_data_.size() ? cat_data_[cat_index_] : nullptr;
	hamlib_data_t* hamlib = cat_data ? cat_data->hamlib : nullptr;
	rig_state_ = rig_state();
	if (damage & DAMAGE_STATUS) {
		// CAT access buttons
		switch (rig_state_) {
		case NO_RIG: {
			// No rig
			bn_connect_->deactivate();
			// bn_connect_->color(FL_BACKGROUND_COLOR);
			bn_connect_->label("");
			bn_select_->activate();
			if (bn_select_->value()) {
				bn_select_->label("Use");
			}
			else {
				bn_select_->label("Select");
			}
			if (cat_data && cat_data->use_cat_app) bn_start_->activate();
			else bn_start_->deactivate();
			break;
		}
		case OPEN:
		case POWERED_DOWN:
		case UNRESPONSIVE:
		{
			// Rig is connected
			bn_connect_->activate();
			// bn_connect_->color(fl_lighter(FL_RED));
			bn_connect_->label("Disconnect");
			bn_select_->deactivate();
			bn_select_->label("");
			bn_select_->value(false);
			bn_start_->deactivate();
			break;
		}
		default:
		{
			// Rig is not connected
			if (hamlib && hamlib->port_type != RIG_PORT_NONE) {
				bn_connect_->activate();
			}
			else {
				bn_connect_->deactivate();
			}
			// bn_connect_->color(fl_lighter(FL_GREEN));
			bn_connect_->label("Connect");
			bn_select_->activate();
			if (bn_select_->value()) {
				bn_select_->label("Use");
			}
			else {
				bn_select_->label("Select");
			}
			if (cat_data && cat_data->use_cat_app) bn_start_->activate();
			else bn_start_->deactivate();
			break;
		}
		}
		bn_connect_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_connect_->color()));
		bn_select_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_select_->color()));
		// Status and frequency/mode
		switch (rig_state_) {
		case NO_RIG:
		{
			// No rig - decativate freq/mode
			op_status_->value("No rig specified");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(FL_BLACK);
			op_freq_mode_->deactivate();
			op_freq_mode_->label("");
			break;
		}
		case OPENING:
		{
			// Rig is connecting - deactivate freq/mode
			op_status_->value("Opening rig");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(FL_YELLOW);
			op_freq_mode_->deactivate();
			op_freq_mode_->label("");
			break;
		}
		case POWERED_DOWN:
		{
			op_status_->value("Powered down");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(COLOUR_ORANGE);
			break;
		}
		case UNRESPONSIVE:
		{
			// Do no change display
			op_status_->value("Unresponsive");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(COLOUR_ORANGE);
			break;
		}
		case OPEN:
		{
			break;
		}
		case NO_CAT:
		{
			// No rig available - deactivate freq/mode
			op_status_->value("No CAT Available");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(COLOUR_GREY);
			op_freq_mode_->deactivate();
			op_freq_mode_->label("");
			break;
		}
		case DISCONNECTED:
		default:
		{
			if (rig_ok_) {
				// Rig has disconnected since last update
				char msg[128];
				if (rig_->is_network_error()) {
					snprintf(msg, sizeof(msg), "RIG: Failed to access network app for %s",
						label());
				}
				else if (rig_->is_rig_error()) {
					snprintf(msg, sizeof(msg), "Failed to access rig %s", label());
				}
				else {
					strcpy(msg, rig_->error_message(label()).c_str());
				}
				status_->misc_status(ST_WARNING, msg);
				// Tidy up if rig has disconnected
				rig_ok_ = false;
				delete rig_;
				rig_ = nullptr;

			}
			// Rig is not connected - deactivate freq/mode
			op_status_->value("Disconnected");
			bn_tx_rx_->label("");
			bn_tx_rx_->color(FL_BLACK);
			op_freq_mode_->deactivate();
			op_freq_mode_->label("");
			break;
		}
		}
		// CAT control widgets - allow only when select button active
		if (rig_ && rig_->is_open()) {
			// Rig is open - disable connection configuration controls
			ch_rig_model_->deactivate();
			serial_grp_->deactivate();
			network_grp_->deactivate();
		}
		else {
			// Rig is not open - allow it to be configured
			ch_rig_model_->activate();
			serial_grp_->activate();
			network_grp_->activate();
			if (bn_select_->value()) {
				// Set configuration widgets to accept input
				serial_grp_->clear_output();
				network_grp_->clear_output();
			}
			else {
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
			}
			else {
				serial_grp_->activate();
			}
			break;
		case RIG_PORT_NETWORK:
		case RIG_PORT_USB:
			// Network or USB port - show network, hide serial
			serial_grp_->hide();
			network_grp_->show();
			if (!bn_select_->value()) {
				ip_port_->value(hamlib->port_name.c_str());
				ip_port_->user_data(&hamlib->port_name);
				network_grp_->deactivate();
			}
			else {
				network_grp_->activate();
			}
			bn_use_app_->value(cat_data->use_cat_app);
			// Chaneg the user data for "use app" button
			bn_use_app_->user_data(&cat_data->use_cat_app);
			ip_app_name_->value(cat_data->app.c_str());
			// The hamlib model has an associated application (eg flrig or wfview)
			if (cat_data_[cat_index_]->use_cat_app) {
				ip_app_name_->activate();
				ip_app_name_->user_data(&cat_data->app);
			}
			else {
				ip_app_name_->deactivate();
			}
			break;
		case RIG_PORT_NONE:
			// Hide both sets of configuration
			serial_grp_->hide();
			network_grp_->hide();
			break;
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


	}
	if (damage & DAMAGE_ADDONS) {

		if (hamlib) {
			// Only update these when not called by ticker
			// Power defaults
			op_pwr_type_->activate();
			switch (hamlib->power_mode) {
			case RF_METER:
				op_pwr_type_->value("RF Meter");
				break;
			case DRIVE_LEVEL:
				op_pwr_type_->value("Drive");
				break;
			case MAX_POWER:
				op_pwr_type_->value("Specify");
				break;
			default:
				op_pwr_type_->value("");
				break;
			}
			ip_max_pwr_->activate();
			char text[25];
			snprintf(text, sizeof(text), "%g", hamlib->max_power);
			ip_max_pwr_->value(text);
			ip_max_pwr_->user_data(&hamlib->max_power);

			// Fequency defaults
			op_freq_type_->activate();
			switch (hamlib->freq_mode) {
			case NO_FREQ:
				op_freq_type_->value("Enter in QSO");
				break;
			case VFO:
				op_freq_type_->value("VFO");
				break;
			case XTAL:
				op_freq_type_->value("Fixed");
				break;
			default:
				op_freq_type_->value("");
				break;
			}
			ip_xtal_->activate();
			snprintf(text, sizeof(text), "%0.6f", hamlib->frequency);
			ip_xtal_->value(text);
			ip_xtal_->user_data(&hamlib->frequency);

			bn_amplifier_->activate();
			bn_amplifier_->value((hamlib->accessory & AMPLIFIER) == AMPLIFIER);
			bn_amplifier_->user_data(&hamlib->accessory);

			ip_gain_->activate();
			if (hamlib->accessory & AMPLIFIER) {
				ip_gain_->activate();
				snprintf(text, sizeof(text), "%d", hamlib->gain);
				ip_gain_->value(text);
				ip_gain_->user_data(&hamlib->gain);
			}
			else {
				ip_gain_->deactivate();
				ip_gain_->value("");
			}

			bn_transverter_->activate();
			bn_transverter_->value((hamlib->accessory & TRANSVERTER) == TRANSVERTER);
			bn_transverter_->user_data(&hamlib->accessory);
			if (hamlib->accessory & TRANSVERTER) {
				ip_offset_->activate();
				snprintf(text, sizeof(text), "%0.6f", hamlib->freq_offset);
				ip_offset_->value(text);
				ip_offset_->user_data(&hamlib->freq_offset);
				ip_tvtr_pwr_->activate();
				snprintf(text, sizeof(text), "%g", hamlib->tvtr_power);
				ip_tvtr_pwr_->value(text);
				ip_tvtr_pwr_->user_data(&hamlib->tvtr_power);
			}
			else {
				ip_offset_->deactivate();
				ip_offset_->value("");
				ip_tvtr_pwr_->deactivate();
				ip_tvtr_pwr_->value("");
			}

			// Timeout and S-meter tracking values
			v_timeout_->activate();
			v_timeout_->value(hamlib->timeout);
			v_smeters_->activate();
			v_smeters_->value(hamlib->num_smeters);
		}
		else {
			op_pwr_type_->deactivate();
			ip_max_pwr_->deactivate();
			op_freq_type_->deactivate();
			ip_xtal_->deactivate();
			bn_amplifier_->deactivate();
			ip_gain_->deactivate();
			bn_transverter_->deactivate();
			ip_offset_->deactivate();
			ip_tvtr_pwr_->deactivate();
			v_timeout_->deactivate();
			v_smeters_->deactivate();
		}

	}

	if ((damage & DAMAGE_VALUES) && rig_state_ == OPEN) {
		double freq;
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
			}
			else {
				bn_tx_rx_->label("RX");
				bn_tx_rx_->color(FL_GREEN);
			}
		}
		else {
			op_status_->value("Out of band!");
			if (rig_->get_ptt()) {
				bn_tx_rx_->label("TX");
				bn_tx_rx_->color(FL_DARK_RED);
			}
			else {
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
		snprintf(msg, sizeof(msg), "%d.%03d.%03d MHz\n%s %sW %s",
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
		bn_tx_rx_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_tx_rx_->color()));

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
			if (cat_data_.size()) {
				if (string(capabilities->mfg_name) == cat_data_[cat_index_]->hamlib->mfr &&
					string(capabilities->model_name) == cat_data_[cat_index_]->hamlib->model) {
					cat_data_[cat_index_]->hamlib->model_id = capabilities->rig_model;
					cat_data_[cat_index_]->hamlib->port_type = capabilities->port_type;
				}
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
		if (cat_data_.size() && id == cat_data_[cat_index_]->hamlib->model_id) {
			ch_rig_model_->value(pos);
		}
	}
	bool found = false;
}

// Populate the choice with the available ports
void qso_rig::populate_port_choice() {
	if (cat_data_.size() && cat_data_[cat_index_]->hamlib->port_type == RIG_PORT_SERIAL) {
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
	if (cat_data_.size() && cat_data_[cat_index_]->hamlib->port_type == RIG_PORT_SERIAL) {
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
	// Add a menu item for each cat item
	if (cat_data_.size()) {
		for (int ix = 0; ix < cat_data_.size(); ix++) {
			char l[5];
			snprintf(l, sizeof(l), "CAT%d", ix + 1);
			bn_index_->add(l, 0, cb_select_cat, (void*)(intptr_t)ix);
		}
	}
	// Add a menu item to add a new CAT
	bn_index_->add("New", 0, cb_new_cat);
	// Add a menu item to remove a CAT method
	if (cat_data_.size()) {
		bn_index_->add("Delete", 0, cb_del_cat);
	}
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
	hamlib_data_t* hamlib;
	if (that->cat_data_.size()) {
		// Use existing CAT entry
		hamlib = that->cat_data_[that->cat_index_]->hamlib;
	}
	else {
		// Create new CAT entry
		cat_data_t* cat_item = new cat_data_t;
		cat_item->hamlib = new hamlib_data_t;
		that->cat_index_ = that->cat_data_.size();
		that->cat_data_.push_back(cat_item);
		hamlib = cat_item->hamlib;
	}
	if (capabilities != nullptr) {
		hamlib->model = capabilities->model_name;
		hamlib->mfr = capabilities->mfg_name;
		hamlib->model_id = id;
		hamlib->port_type = capabilities->port_type;
		that->modify_hamlib_data();
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
	that->enable_widgets(DAMAGE_ALL);
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
	that->rig_starting_ = false;
	that->enable_widgets(DAMAGE_ALL);
}

// Pressed the select button 
// v is not used 
void qso_rig::cb_bn_select(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	Fl_Light_Button* bn = (Fl_Light_Button*)w;
	if (bn->value()) {
		// This is called in switch_rig
		that->enable_widgets(DAMAGE_ALL);
	} else {
		that->switch_rig();
	}
}

// Clicked start button
// v points to the string containing the command to  invoke flrig
void qso_rig::cb_bn_start(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	string command = that->cat_data_[that->cat_index_]->app + "&";
	int result = system(command.c_str());
	char msg[100];
	if (result == 0) {
		that->rig_starting_ = true;
		snprintf(msg, sizeof(msg), "RIG: Started %s OK", command.c_str());
		status_->misc_status(ST_OK, msg);
	}
	else {
		that->rig_starting_ = false;
		snprintf(msg, sizeof(msg), "RIG: Failed to start %s", command.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Selecting config tab
// v is not used
void qso_rig::cb_config(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets(DAMAGE_ALL);
}

// Use application button
void qso_rig::cb_bn_use_app(Fl_Widget* w, void* v) {
	cb_value<Fl_Light_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets(DAMAGE_ALL);
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
	that->cat_index_ = that->cat_data_.size() - 1;
	that->populate_index_menu();
	that->switch_rig();
}

// Remove a CAT
void qso_rig::cb_del_cat(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->cat_data_.erase(that->cat_data_.begin() + that->cat_index_);
	if (that->cat_index_ >= that->cat_data_.size()) {
		that->cat_index_ = that->cat_data_.size() - 1;
	}
	that->populate_index_menu();
	that->switch_rig();
}

// ACcessories
void qso_rig::cb_bn_amplifier(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	bool value = ((Fl_Check_Button*)w)->value();
	accessory_t* t = (accessory_t*)v;
	if (value) {
		*t = (accessory_t)(*t | AMPLIFIER);
	}
	else {
		*t = (accessory_t)(*t & ~AMPLIFIER);
	}
	that->enable_widgets(DAMAGE_ADDONS);
}

void qso_rig::cb_bn_transverter(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	bool value = ((Fl_Check_Button*)w)->value();
	accessory_t* t = (accessory_t*)v;
	if (value) {
		*t = (accessory_t)(*t | TRANSVERTER);
	}
	else {
		*t = (accessory_t)(*t & ~TRANSVERTER);
	}
	that->enable_widgets(DAMAGE_ADDONS);
}

// Connect rig if disconnected and vice-versa
void qso_rig::switch_rig() {
	if (rig_) {
		delete rig_;
		rig_ = nullptr;
		rig_ = new rig_if(label(), cat_data_.size() ?  cat_data_[cat_index_]->hamlib : nullptr);
		modify_hamlib_data();
	}
	ancestor_view<qso_manager>(this)->update_rig();
	enable_widgets(DAMAGE_ALL);
}

// 1 s clock interface - read rig and update status
void qso_rig::ticker() {
	rig_state_t current = rig_state();
		printf("DEBUG: Rig state %d\n", (int)current);
	if (current != rig_state_) {
		rig_state_ = current;
		// The rig may have disconnected - update connect/select buttons
		enable_widgets(DAMAGE_ALL);
	} else {
		enable_widgets(DAMAGE_VALUES);
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
	if (cat_data_.size()) mode_ = (uint16_t)cat_data_[cat_index_]->hamlib->port_type;
	else mode_ = RIG_PORT_NONE;
	enable_widgets(DAMAGE_ALL);
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
void qso_rig::modify_hamlib_data() {
	hamlib_data_t* hamlib = cat_data_[cat_index_]->hamlib;
	const rig_caps* capabilities = rig_get_caps(hamlib->model_id);
	if (capabilities->port_type == RIG_PORT_NONE) {
		hamlib->power_mode = MAX_POWER;
	}
	else if (capabilities && (capabilities->has_get_level & RIG_LEVEL_RFPOWER_METER_WATTS)) {
		hamlib->power_mode = RF_METER;
	}
	else if (capabilities && (capabilities->has_get_level & RIG_LEVEL_RFPOWER)) {
		hamlib->power_mode = DRIVE_LEVEL;
	}
	else {
		hamlib->power_mode = MAX_POWER;
	}
	if (capabilities->port_type == RIG_PORT_NONE) {
		hamlib->freq_mode = NO_FREQ;
	}
	else {
		// TODO - how do I know if it's fixed frequency?
		hamlib->freq_mode = VFO;
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

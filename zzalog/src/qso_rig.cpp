#include "qso_rig.h"
#include "rig_if.h"
#include "serial.h"
#include "status.h"
#include "qso_manager.h"
#include "band_data.h"
#include "spec_data.h"
#include "ticker.h"
#include "field_choice.h"
#include "filename_input.h"
#include "file_viewer.h"
#include "rig_data.h"

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

extern status* status_;
extern band_data* band_data_;
extern spec_data* spec_data_;
extern rig_data* rig_data_;
extern bool DARK;
extern ticker* ticker_;
extern string VENDOR;
extern string PROGRAM_ID;
extern void open_html(const char*);


// Constructor
qso_rig::qso_rig(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, nullptr),
	rig_ok_(false),
	rig_state_(NO_RIG)
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
	tooltip("Allows the onfiguration of an individual rig connection and displays current status");
	load_values();
	if (cat_data_) {
		rig_ = new rig_if(label(), cat_data_->hamlib);
		if (rig_->is_good()) rig_ok_ = true;
		if (rig_ok_) modify_hamlib_data();
	}
	else {
		rig_ = nullptr;
	}
	create_form(X, Y);
	enable_widgets(DAMAGE_ALL);

	if (!rig_ok_) {
		if (cat_data_ && cat_data_->auto_start) {
			cb_bn_start(bn_start_, nullptr);
		}
	}
	ticker_->add_ticker(this, cb_ticker, 10);
}

// DEstructor
qso_rig::~qso_rig() {
	save_values();
	delete rig_;
	ticker_->remove_ticker(this);
}

// Handle
int qso_rig::handle(int event) {
	int result = Fl_Group::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("qso_rig.html");
			return true;
		}
		break;
	}
	return result;
}


// Get initial data from settings
void qso_rig::load_values() {
	rig_info_ = rig_data_->get_rig(label());
	cat_data_ = rig_data_->cat_data(label());
}

// Keep the display 3 buttons wide
const int WDISPLAY = 3 * WBUTTON;
// Create the status part of the display
void qso_rig::create_status(int curr_x, int curr_y) {
	int save_x = curr_x;

	status_grp_ = new Fl_Group(curr_x, curr_y, WDISPLAY, HBUTTON * 2 + HTEXT * 3);
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
	op_freq_mode_->labelfont(0);
	op_freq_mode_->labelsize(FL_NORMAL_SIZE + 10);

	curr_y += op_freq_mode_->h();

	// Display instantaneous mode
	bn_instant_ = new Fl_Check_Button(curr_x, curr_y, WDISPLAY, HBUTTON, "Display instantaneous");
	bn_instant_->tooltip("Display shows instananoues power and S-meter values or maxuimum over recent samples");
	bn_instant_->box(FL_FLAT_BOX);
	bn_instant_->callback(cb_value<Fl_Button, bool>, &rig_info_->use_instant_values);
	bn_instant_->value(rig_info_->use_instant_values);
	bn_instant_->type(FL_TOGGLE_BUTTON);

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
	ip_antenna_->callback(cb_value<field_input, string>, &rig_info_->antenna);
	ip_antenna_->tooltip("Select the preferred antenna for this rig");
	ip_antenna_->field_name("MY_ANTENNA");
	ip_antenna_->value(rig_info_->antenna.c_str());

	rig_ant_grp_->end();


}

// Create tabbed form for configuration data
void qso_rig::create_config(int curr_x, int curr_y) {
	// Tabbed form
	config_tabs_ = new Fl_Tabs(curr_x, curr_y, w() - 2 * GAP, 10);
	config_tabs_->box(FL_BORDER_BOX);
	config_tabs_->callback(cb_config);
	config_tabs_->when(FL_WHEN_CHANGED);
	config_tabs_->handle_overflow(Fl_Tabs::OVERFLOW_PULLDOWN);

	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	config_tabs_->client_area(rx, ry, rw, rh, 0);
	int saved_rh = rh;
	curr_x = rx;
	curr_y = ry;
	// Create connection tab
	create_connex(curr_x, curr_y);
	rh = max(rh, connect_tab_->y() + connect_tab_->h() - ry);
	// Create auto tab
	create_auto(curr_x, curr_y);
	rh = max(rh, auto_tab_->y() + auto_tab_->h() - ry);
	// Create defaults tab
	create_defaults(curr_x, curr_y);
	rh = max(rh, defaults_tab_->y() + defaults_tab_->h() - ry);
	// Create accessory tab
	create_accessory(curr_x, curr_y);
	rh = max(rh, accessory_tab_->y() + accessory_tab_->h() - ry);
	// Create timeout &c tab
	create_timeout(curr_x, curr_y);
	rh = max(rh, timeout_tab_->y() + timeout_tab_->h() - ry);


	config_tabs_->resizable(nullptr);
	config_tabs_->size(config_tabs_->w(), config_tabs_->h() + rh - saved_rh);
	config_tabs_->end();
	curr_x = config_tabs_->x() + config_tabs_->w();
	curr_y = config_tabs_->y() + config_tabs_->h();

}

// Create form to configure the hamlib port connection
// Create two versions: 1 for serial ports and one for networked ports
void qso_rig::create_connex(int curr_x, int curr_y) {
	connect_tab_ = new Fl_Group(curr_x, curr_y, w() - 2 * GAP, 10, "Connection");
	connect_tab_->labelsize(FL_NORMAL_SIZE + 2);
	curr_x += GAP;
	curr_y += GAP;
	int max_y = curr_y;
	// Create serial port
	create_serial(curr_x, curr_y);
	max_y = max(max_y, serial_grp_->y() + serial_grp_->h());
	// Create network port
	create_network(curr_x, curr_y);
	max_y = max(max_y, network_grp_->y() + network_grp_->h());


	connect_tab_->resizable(nullptr);
	connect_tab_->size(connect_tab_->w(), max_y - connect_tab_->y());

	connect_tab_->end();

}

// Create form for defining a serial port connection
void qso_rig::create_serial(int curr_x, int curr_y) {
	serial_grp_ = new Fl_Group(curr_x, curr_y, w() - 2 * GAP, 10);
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
	serial_grp_->size(serial_grp_->w(), curr_y - serial_grp_->y());

	serial_grp_->end();

}

// Create for to configure a network connection
void qso_rig::create_network(int curr_x, int curr_y) {
	network_grp_ = new Fl_Group(curr_x, curr_y, w() - 2 * GAP, 10);
	network_grp_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	network_grp_->box(FL_NO_BOX);

	curr_x = network_grp_->x();
	curr_y = network_grp_->y();

	bn_use_app_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Use");
	bn_use_app_->callback(cb_bn_use_app, nullptr);
	bn_use_app_->tooltip("Set to allow a CAT app to be used");
	bn_use_app_->value(false);

	curr_x += WBUTTON / 2;

	bn_show_app_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Show");
	bn_show_app_->callback(cb_show_app, &ip_app_name_);
	bn_show_app_->tooltip("Show the app script");

	curr_x += WBUTTON / 2;
	int this_w = network_grp_->w() + network_grp_->x() - curr_x - GAP - GAP;

	// App name (flrig or wfview to connect to rig
	ip_app_name_ = new filename_input(curr_x, curr_y, this_w, HTEXT);
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

// Create the automatic action tab
void qso_rig::create_auto(int curr_x, int curr_y) {
	auto_tab_ = new Fl_Group(curr_x, curr_y, 10, 10, "Auto");
	auto_tab_->labelsize(FL_NORMAL_SIZE + 1);

	curr_x += GAP;
	curr_y += GAP;

	bn_autostart_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Start");
	bn_autostart_->align(FL_ALIGN_RIGHT);
	bn_autostart_->callback(cb_bn_autostart, nullptr);
	bn_autostart_->tooltip("Automatically start the CAT interface application");

	curr_y += HBUTTON;
	bn_autoconn_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Connect");
	bn_autoconn_->align(FL_ALIGN_RIGHT);
	bn_autoconn_->callback(cb_bn_autoconn, nullptr);
	bn_autoconn_->tooltip("Automatically start the CAT interface application");

	curr_x += WBUTTON;
	v_connect_delay_ = new Fl_Value_Slider(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON, "Delay");
	v_connect_delay_->align(FL_ALIGN_TOP);
	v_connect_delay_->type(FL_HOR_SLIDER);
	v_connect_delay_->callback(cb_connect_delay, nullptr);
	v_connect_delay_->bounds(0.0, 10.0);
	v_connect_delay_->step(0.5);
	v_connect_delay_->tooltip("Specify the delay between starting the app and trying to connect");

	curr_x += v_connect_delay_->w();
	curr_y += HBUTTON + GAP;

	auto_tab_->resizable(nullptr);
	auto_tab_->size(curr_x - auto_tab_->x(), curr_y - auto_tab_->y());

	auto_tab_->end();

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

	ch_pwr_type_ = new Fl_Choice(curr_x, curr_y, WBUTTON, HBUTTON, "Power");
	ch_pwr_type_->align(FL_ALIGN_TOP);
	ch_pwr_type_->tooltip("Select power type to override hamlib default");
	ch_pwr_type_->callback(cb_ch_power, nullptr);
	populate_power_choice();

	curr_y += HBUTTON;
	ip_max_pwr_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "W");
	ip_max_pwr_->align(FL_ALIGN_RIGHT);
	ip_max_pwr_->callback(cb_value_double<Fl_Float_Input>, nullptr);
	ip_max_pwr_->tooltip("Specify the maximum power out from the rig");

	int max_y = curr_y + HBUTTON + GAP;

	curr_y = save_y;
	curr_x += WBUTTON;

	bn_override_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "O/R");
	bn_override_->align(FL_ALIGN_TOP);
	bn_override_->callback(cb_bn_override, nullptr);
	bn_override_->tooltip("Check this to override hamlib defaults - eg rigctld");

	curr_x += HBUTTON;

	op_freq_type_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Frequency");
	op_freq_type_->box(FL_FLAT_BOX);
	op_freq_type_->align(FL_ALIGN_TOP);
	op_freq_type_->tooltip("Shows hoe the frequency is generated");

	curr_y += HBUTTON;
	ip_xtal_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "MHz");
	ip_xtal_->align(FL_ALIGN_RIGHT);
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
	int save_y = curr_y;

	bn_amplifier_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Amplifier");
	bn_amplifier_->box(FL_FLAT_BOX);
	bn_amplifier_->callback(cb_bn_amplifier, nullptr);
	bn_amplifier_->tooltip("Select whether an ammplifier is fitted");

	curr_y += HBUTTON;
	ip_gain_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON - 5, HBUTTON, "dB");
	ip_gain_->align(FL_ALIGN_RIGHT);
	ip_gain_->callback(cb_value_int<Fl_Int_Input>, nullptr);
	ip_gain_->tooltip("Specify the amplifier gain in decibels");

	int max_y = curr_y = HBUTTON + GAP;

	curr_y = save_y;
	curr_x += WBUTTON + GAP;

	bn_transverter_ = new Fl_Check_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Transverter");
	bn_transverter_->box(FL_FLAT_BOX);
	bn_transverter_->callback(cb_bn_transverter, nullptr);
	bn_transverter_->tooltip("Select to add a transverter");

	curr_y += HBUTTON;
	ip_offset_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "\316\224MHz");
	ip_offset_->align(FL_ALIGN_RIGHT);
	ip_offset_->callback(cb_value_double<Fl_Float_Input>, nullptr);
	ip_offset_->tooltip("Specify the Transverter frequency offset to apply");

	curr_y += HBUTTON ;
	ip_tvtr_pwr_ = new Fl_Float_Input(curr_x, curr_y, WBUTTON, HBUTTON, "W");
	ip_tvtr_pwr_->align(FL_ALIGN_RIGHT);
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
	curr_y += GAP;

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

	curr_y += HBUTTON;

	v_to_count_ = new Fl_Value_Slider(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON, "TO Count");
	v_to_count_->align(FL_ALIGN_LEFT);
	v_to_count_->type(FL_HOR_SLIDER);
	v_to_count_->callback(cb_to_count);
	v_to_count_->tooltip("Set the number of timeouts an access to a rig value before abandoning that value");
	v_to_count_->range(1, 10);
	v_to_count_->step(1);
	v_to_count_->value(5);

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

// Save values in settings
void qso_rig::save_values() {
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
	cat_data_ = rig_data_->cat_data(label());
	hamlib_data_t* hamlib = cat_data_ ? cat_data_->hamlib : nullptr;
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
			if (cat_data_ && cat_data_->use_cat_app) bn_start_->activate();
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
			if (cat_data_ && cat_data_->use_cat_app) bn_start_->activate();
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
		if (cat_data_) {
			switch (cat_data_->hamlib->port_type) {
			case RIG_PORT_SERIAL:
				// Serial port - show serial configuration, hide network
				if (visible_r()) serial_grp_->show();
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
				if (visible_r()) network_grp_->show();
				if (!bn_select_->value()) {
					ip_port_->value(hamlib->port_name.c_str());
					ip_port_->user_data(&hamlib->port_name);
					network_grp_->deactivate();
				}
				else {
					network_grp_->activate();
				}
				bn_use_app_->value(cat_data_->use_cat_app);
				// Chaneg the user data for "use app" button
				bn_use_app_->user_data(&cat_data_->use_cat_app);
				ip_app_name_->value(cat_data_->app.c_str());
				// The hamlib model has an associated application (eg flrig or wfview)
				if (cat_data_->use_cat_app) {
					ip_app_name_->activate();
					ip_app_name_->user_data(&cat_data_->app);
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
			default:
				break;
			}
		} else {
			// Hide both sets of configuration
			serial_grp_->hide();
			network_grp_->hide();
		}
		// Set the rig name into the choice
		if (hamlib && hamlib->model_id != -1) {
			int pos = rig_choice_pos_.at(hamlib->model_id);
			ch_rig_model_->value(pos);
		} else {
			ch_rig_model_->value(0);
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
		//char l[5];
		//snprintf(l, sizeof(l), "CAT%d", cat_index_ + 1);
		if (rig_info_->default_app >= 0 && rig_info_->default_app < rig_info_->cat_data.size()) {
			bn_index_->copy_label(cat_data_->nickname.c_str());
		}
		else {
			bn_index_->label("CAT");
		}


	}
	if (damage & DAMAGE_ADDONS) {

		if (hamlib) {
			// Only update these when not called by ticker
			// Power defaults
			bn_override_->value(cat_data_->override_hamlib);
			bn_override_->user_data(&cat_data_->override_hamlib);
			if (cat_data_->override_hamlib) {
				op_pwr_type_->hide();
				ch_pwr_type_->show();
				ch_pwr_type_->value(hamlib->power_mode);
				ch_pwr_type_->user_data(&hamlib->power_mode);
				switch(hamlib->power_mode) {
					case DRIVE_LEVEL:
					case MAX_POWER:
						ip_max_pwr_->activate();
						break;
					default:
						ip_max_pwr_->deactivate();
						break;
				}
			} else {
				op_pwr_type_->show();
				ch_pwr_type_->hide();
				switch (hamlib->power_mode) {
				case RF_METER:
					op_pwr_type_->value("RF Meter");
					ip_max_pwr_->deactivate();
					break;
				case DRIVE_LEVEL:
					op_pwr_type_->value("Drive");
					ip_max_pwr_->activate();
					break;
				case MAX_POWER:
					op_pwr_type_->value("Specify");
					ip_max_pwr_->activate();
					break;
				default:
					op_pwr_type_->value("None");
					ip_max_pwr_->deactivate();
					break;
				}
			}
			char text[25];
			snprintf(text, sizeof(text), "%g", hamlib->max_power);
			ip_max_pwr_->value(text);
			ip_max_pwr_->user_data(&hamlib->max_power);

			// Fequency defaults
			op_freq_type_->activate();
			switch (hamlib->freq_mode) {
			case NO_FREQ:
				op_freq_type_->value("Enter in QSO");
				ip_xtal_->deactivate();
				break;
			case VFO:
				op_freq_type_->value("VFO");
				ip_xtal_->deactivate();
				break;
			case XTAL:
				op_freq_type_->value("Fixed");
				ip_xtal_->activate();
				break;
			default:
				op_freq_type_->value("");
				ip_xtal_->deactivate();
				break;
			}
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
			v_to_count_->activate();
			v_to_count_->value(hamlib->max_to_count);
		}
		else {
			op_pwr_type_->activate();
			ip_max_pwr_->activate();
			op_pwr_type_->value("Specify");
			op_freq_type_->activate();
			op_freq_type_->value("Enter in QSO");
			ip_xtal_->deactivate();
			bn_amplifier_->deactivate();
			ip_gain_->deactivate();
			bn_transverter_->deactivate();
			ip_offset_->deactivate();
			ip_tvtr_pwr_->deactivate();
			v_timeout_->deactivate();
			v_smeters_->deactivate();
			v_to_count_->deactivate();
		}

	}

	if ((damage & DAMAGE_VALUES) && (rig_state_ == OPEN || rig_state_ == UNRESPONSIVE)) {
		double tx_freq, rx_freq, freq;
		tx_freq = rig_->get_dfrequency(true);
		rx_freq = rig_->get_dfrequency(false);
		freq = rig_->get_ptt() ? tx_freq : rx_freq;
		if (rig_state_ == OPEN) {
			band_data::band_entry_t* entry = band_data_->get_entry(freq);
			if (entry) {
				char l[50];
				if (rig_->get_split()) {
					snprintf(l, sizeof(l), "SPLIT %s/%s",
					spec_data_->band_for_freq(tx_freq).c_str(),
					spec_data_->band_for_freq(rx_freq).c_str());
				} else {
					strcpy(l, spec_data_->band_for_freq(freq).c_str());
					for (auto it = entry->modes.begin(); it != entry->modes.end(); it++) {
						strcat(l, " ");
						strcat(l, (*it).c_str());
					}
				}
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
			bn_tx_rx_->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, bn_tx_rx_->color()));
		}
		op_freq_mode_->activate();
		op_freq_mode_->color(FL_BLACK, FL_BLACK);
		op_freq_mode_->labelcolor(FL_YELLOW);

		char msg[200];
		string rig_mode;
		string submode;
		rig_->get_string_mode(rig_mode, submode);
		bool ptt = rig_->get_ptt();
		char bullet[] = "\342\200\243";
		if (tx_freq == rx_freq) {
			if (rig_info_->use_instant_values) {
				// Set Freq/Mode to Frequency (MHz with kHz seperator), mode, power (W)
				snprintf(msg, sizeof(msg), "%0.6f MHz\n%s %sW %s",
					tx_freq,
					submode.length() ? submode.c_str() : rig_mode.c_str(),
					rig_->get_tx_power(false).c_str(),
					rig_->get_smeter(false).c_str()
				);
			} else {
				// Set Freq/Mode to Frequency (MHz with kHz seperator), mode, power (W)
				snprintf(msg, sizeof(msg), "%0.6f MHz\n%s %sW %s",
					tx_freq,
					submode.length() ? submode.c_str() : rig_mode.c_str(),
					rig_->get_tx_power(true).c_str(),
					rig_->get_smeter(true).c_str()
				);
			}
		}
		else if (rig_info_->use_instant_values) {
			// Set Freq/Mode to Frequency (MHz with kHz seperator), mode, power (W)
			snprintf(msg, sizeof(msg), "%s %0.6f MHz\n%s %0.6f MHz\n%s %sW %s",
				ptt ? bullet : " ", tx_freq, ptt ? " " : bullet, rx_freq,
				submode.length() ? submode.c_str() : rig_mode.c_str(),
				rig_->get_tx_power(false).c_str(),
				rig_->get_smeter(false).c_str()
			);
		} else {
			// Set Freq/Mode to Frequency (MHz with kHz seperator), mode, power (W)
			snprintf(msg, sizeof(msg), "%s %0.6f MHz\n%s %0.6f MHz\n%s %sW %s",
				ptt ? bullet : " ", tx_freq, ptt ? " " : bullet, rx_freq,
				submode.length() ? submode.c_str() : rig_mode.c_str(),
				rig_->get_tx_power(true).c_str(),
				rig_->get_smeter(true).c_str()
			);
		}
		op_freq_mode_->copy_label(msg);
		int size = FL_NORMAL_SIZE + 10;
		fl_font(0, size);
		int w = 0, h;
		fl_measure(msg, w, h);
		while (w > op_freq_mode_->w() || h > op_freq_mode_->h()) {
			size--;
			fl_font(0, size);
			fl_measure(msg, w, h);
		}
		op_freq_mode_->labelsize(size);

	}

	if (damage & DAMAGE_AUTOS) {
		if (cat_data_) {
			bn_autostart_->value(cat_data_->auto_start);
			bn_autostart_->activate();
			bn_autoconn_->value(cat_data_->auto_connect);
			bn_autoconn_->activate();
			v_connect_delay_->value(cat_data_->connect_delay);
			if (cat_data_->auto_connect) {
				v_connect_delay_->activate();
			} else {
				v_connect_delay_->deactivate();
			}
		} else {
			bn_autostart_->deactivate();
			bn_autoconn_->deactivate();
			v_connect_delay_->deactivate();
		}
	}

	redraw();
}

// Populate manufacturer and model choices - hierarchical menu: first manufacturer, then model
void qso_rig::populate_model_choice() {
	// Get hamlib Model number and populate control with all model names
	ch_rig_model_->clear();
	rig_choice_pos_.clear();
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
			if (cat_data_) {
				if (string(capabilities->mfg_name) == cat_data_->hamlib->mfr &&
					string(capabilities->model_name) == cat_data_->hamlib->model) {
					cat_data_->hamlib->model_id = capabilities->rig_model;
					cat_data_->hamlib->port_type = capabilities->port_type;
				}
			}
		}
	}
	// Add the rigs in alphabetical order to the choice widget,
	// set widget's value to intended
	ch_rig_model_->add("", 0, nullptr, nullptr);
	for (auto ix = rig_list.begin(); ix != rig_list.end(); ix++) {
		string name = *ix;
		rig_model_t id = rig_ids.at(name);
		// Add the id as user data for the menu item
		int pos = ch_rig_model_->add(name.c_str(), 0, nullptr, (void*)(intptr_t)id);
		if (cat_data_ && id == cat_data_->hamlib->model_id) {
			ch_rig_model_->value(pos);
		}
		rig_choice_pos_[id] = pos;
	}
}

// Populate the choice with the available ports
void qso_rig::populate_port_choice() {
	if (cat_data_ && cat_data_->hamlib->port_type == RIG_PORT_SERIAL) {
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
			string sport = *(existing_ports + i);
			const char* port = sport.c_str();
			snprintf(message, sizeof(message), "DASH: Found port %s", port);
			status_->misc_status(ST_LOG, message);
			ch_port_name_->add(port);
			// Set the value to the list of ports
			if (strcmp(port, cat_data_->hamlib->port_name.c_str()) == 0) {
				ch_port_name_->value(i);
			}
		}
	}
}

// Populate the baud rate choice menu
void qso_rig::populate_baud_choice() {
	if (cat_data_ && cat_data_->hamlib->port_type == RIG_PORT_SERIAL) {
		ch_baud_rate_->clear();
		// Override rig's capabilities?
		bn_all_rates_->value(use_all_rates_);

		// Get the baud-rates supported by the rig
		const rig_caps* caps = rig_get_caps(cat_data_->hamlib->model_id);
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
				if (rate == cat_data_->hamlib->baud_rate) {
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
	if (rig_info_ && rig_info_->cat_data.size()) {
		for (int ix = 0; ix < rig_info_->cat_data.size(); ix++) {
			bn_index_->add(rig_info_->cat_data[ix]->nickname.c_str(), 0, cb_select_cat, (void*)(intptr_t)ix);
		}
	}
	// Add a menu item to add a new CAT
	bn_index_->add("New", 0, cb_new_cat);
	// Add a menu item to remove a CAT method
	if (rig_info_ && rig_info_->cat_data.size()) {
		bn_index_->add("Delete", 0, cb_del_cat);
	}
}

// Populate power type choice
void qso_rig::populate_power_choice() {
	ch_pwr_type_->clear();
	ch_pwr_type_->add("None");
	ch_pwr_type_->add("RF Power");
	ch_pwr_type_->add("Drive");
	ch_pwr_type_->add("Specify");
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
	if (that->cat_data_) {
		// Use existing CAT entry
		hamlib = that->cat_data_->hamlib;
	}
	else {
		// Create new CAT entry
		cat_data_t* cat_item = new cat_data_t;
		cat_item->hamlib = new hamlib_data_t;
		that->rig_info_->default_app = that->rig_info_->cat_data.size();
		that->rig_info_->cat_data.push_back(cat_item);
		that->cat_data_ = rig_data_->cat_data(that->label());
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
	that->populate_baud_choice();
	that->populate_port_choice();
	that->enable_widgets(DAMAGE_ALL);
}

// Callback selecting serial port
// v is unused
void qso_rig::cb_ch_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_text<Fl_Choice, string>(w, (void*)&that->cat_data_->hamlib->port_name);
}

// Callback entering named port
// v is unused
void qso_rig::cb_ip_port(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cb_value<Fl_Input, string>(w, (void*)&that->cat_data_->hamlib->port_name);
}

// Callback selecting baud-rate
// v is unused
void qso_rig::cb_ch_baud(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	const char* text = ((Fl_Choice*)w)->text();
	that->cat_data_->hamlib->baud_rate = atoi(text);
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
		that->rig_ = new rig_if(that->label(), that->cat_data_->hamlib);
		if (that->rig_->is_good()) that->rig_ok_ = true;
	}
	else if (that->rig_->is_open()) {
		that->rig_->close();
		that->rig_ok_ = false;

	} else {
		that->rig_->open();
		if (that->rig_->is_good()) {
			that->rig_ok_ = true;
			that->modify_hamlib_data();
		}
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
	cat_data_t* cat_data = that->cat_data_;
#ifdef _WIN32
	string command = "start /min " + cat_data->app;
#else
	string command = cat_data->app + "&";
#endif
	int result = system(command.c_str());
	char msg[100];
	if (result == 0) {
		that->rig_starting_ = true;
		snprintf(msg, sizeof(msg), "RIG: Started %s OK", command.c_str());
		status_->misc_status(ST_OK, msg);
		// Start the connect delay timer
		if (cat_data->auto_connect) {
			Fl::add_timeout(cat_data->connect_delay, cb_start_timer, (Fl_Widget*)that->bn_autoconn_);
		}
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
	cb_value<Fl_Value_Slider, double>(w, &that->cat_data_->hamlib->timeout);
}

// Smeter stack length
void qso_rig::cb_smeters(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	double value;
	cb_value<Fl_Value_Slider, double>(w, &value);
	that->cat_data_->hamlib->num_smeters = (int)value;;
}

// Timeout count
void qso_rig::cb_to_count(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	double value;
	cb_value<Fl_Value_Slider, double>(w, &value);
	that->cat_data_->hamlib->max_to_count = (int)value;;
}

// Use selected CAT
void qso_rig::cb_select_cat(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->rig_info_->default_app = (int)(intptr_t)v;
	that->switch_rig();
}

// Generate a new CAT
void qso_rig::cb_new_cat(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	cat_data_t* data = new cat_data_t;
	const char* nickname = fl_input("Please enter the nickname for the CAT connection", "None");
	if (nickname) {
		data->nickname = nickname;
		data->hamlib = new hamlib_data_t;
		that->rig_info_->cat_data.push_back(data);
		that->rig_info_->default_app = that->rig_info_->cat_data.size() - 1;
		that->cat_data_ = rig_data_->cat_data(that->label());
		that->populate_index_menu();
		that->switch_rig();
	}
}

// Remove a CAT
void qso_rig::cb_del_cat(Fl_Widget* w, void* v) {
	qso_rig* that = ancestor_view<qso_rig>(w);
	auto erase_it = that->rig_info_->cat_data.begin() + that->rig_info_->default_app;
	that->rig_info_->cat_data.erase(erase_it);
	if (that->rig_info_->default_app >= that->rig_info_->cat_data.size()) {
		that->rig_info_->default_app = that->rig_info_->cat_data.size() - 1;
		that->cat_data_ = rig_data_->cat_data(that->label());
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

// Show scripts
void qso_rig::cb_show_app(Fl_Widget* w, void* v) {
	filename_input* ip = *(filename_input**)v;
	if (strlen(ip->value()) > 0) {
		file_viewer* fwin = new file_viewer(640, 480);
		fwin->load_file(ip->value());
	}
}

// Override hamlib
void qso_rig::cb_bn_override(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets(DAMAGE_ADDONS);
}

// Select hamlib override power type
void qso_rig::cb_ch_power(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, power_mode_t>(w, v);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->enable_widgets(DAMAGE_ADDONS);
}

// Select autostart
void qso_rig::cb_bn_autostart(Fl_Widget* w, void* v) {
	bool value;
	cb_value<Fl_Check_Button, bool>(w, &value);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->cat_data_->auto_start = value;
}

// Select autoconnect
void qso_rig::cb_bn_autoconn(Fl_Widget* w, void* v) {
	bool value;
	cb_value<Fl_Check_Button, bool>(w, &value);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->cat_data_->auto_connect = value;
	that->enable_widgets(DAMAGE_AUTOS);
}

// Connect delay
void qso_rig::cb_connect_delay(Fl_Widget* w, void* v) {
	double value;
	cb_value<Fl_Value_Slider, double>(w, &value);
	qso_rig* that = ancestor_view<qso_rig>(w);
	that->cat_data_->connect_delay = value;
}

// Connect timer delay
void qso_rig::cb_start_timer(void* v) {
	// Start connect
	cb_bn_connect((Fl_Widget*)v, nullptr);
}

// Connect rig if disconnected and vice-versa
void qso_rig::switch_rig() {
	if (rig_) {
		delete rig_;
		rig_ = nullptr;
	}
	cat_data_ = rig_data_->cat_data(label());
	if (cat_data_) {
		rig_ = new rig_if(label(), cat_data_->hamlib);
		modify_hamlib_data();
		ancestor_view<qso_manager>(this)->update_rig();
	}
	enable_widgets(DAMAGE_ALL);
}

// 1 s clock interface - read rig and update status
void qso_rig::ticker() {
	rig_state_t current = rig_state();
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
	int cat_index = rig_info_->default_app;
	if (cat_index >= 0 && cat_index < rig_info_->cat_data.size()) {
		if (!cat_data_->override_hamlib) {
			hamlib_data_t* hamlib = cat_data_->hamlib;
			const rig_caps* capabilities = rig_get_caps(hamlib->model_id);
			if (capabilities) {
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
		}
	}
}

// Return the preferred antenna
string qso_rig::antenna() {
	return rig_info_->antenna;
}

// Force disconnect
void qso_rig::disconnect() {
	if (rig_ && rig_->is_open()) {
		rig_->close();
	}
}

// Return selected CAT
const char* qso_rig::cat() {
	char* result = new char[32];
	if (!cat_data_) {
		return "";
	}
	else {
		return cat_data_->nickname.c_str();
	}
	return result;
}

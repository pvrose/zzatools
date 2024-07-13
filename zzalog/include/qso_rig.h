#pragma once

#include "rig_if.h"
#include "hamlib/rig.h"
#include "field_choice.h"
#include "modems.h"

#include <string>

#include <FL/Fl_Group.H>
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

using namespace std;

// Displays and controls the status of a single rig connection
// Controls c
class qso_rig :
    public Fl_Group
{


public:

	qso_rig(int X, int Y, int W, int H, const char* l = nullptr);
	~qso_rig();

	// get settings 
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable widgets
	void enable_widgets();
	// Save changes
	void save_values();
	// Switch the rig on or off
	void switch_rig();
	// 1s clock interface
	void ticker();
	// Static version
	static void cb_ticker(void* v);
	// New rig
	void new_rig();
	// Get the rig
	rig_if* rig();
	// Notification colour
	Fl_Color alert_colour();
	// Preferred antenna
	string antenna();
	// Suffix to apply to app invocations
	string app(modem_t m);
	// Disconnect rig
	void disconnect();

protected:
	// Callback - model choice
	static void cb_ch_model(Fl_Widget* w, void* v);
	// Callback - hamlib serial ports
	static void cb_ch_port(Fl_Widget* w, void* v);
	// Callback - hamlib baudrate
	static void cb_ch_baud(Fl_Widget* w, void* v);
	// Callback override caps
	static void cb_ch_over(Fl_Widget* w, void* v);
	// Callback all ports
	static void cb_bn_all(Fl_Widget* w, void* v);
	// Callback network port
	static void cb_ip_port(Fl_Widget* w, void* v);
	// Callback - Connect button
	static void cb_bn_connect(Fl_Widget* w, void* v);
	// Callback - Select rig
	static void cb_bn_select(Fl_Widget* w, void* v);

	// Callback - modify frequency
	static void cb_bn_mod_freq(Fl_Widget* w, void* v);
	// Callback - input frequency
	static void cb_ip_freq(Fl_Widget* w, void* v);
	// Callback - use gain/power
	static void cb_bn_power(Fl_Widget* w, void* v);
	// Callback - input gain
	static void cb_ip_gain(Fl_Widget* w, void* v);
	// Callback - input power
	static void cb_ip_power(Fl_Widget* w, void* v);
	// Call back start flrig
	static void cb_bn_start(Fl_Widget* w, void* v);
	// Tabs clicked
	static void cb_config(Fl_Widget* w, void* v);

	// Get hamlib data
	void find_hamlib_data();

	//populate port choice
	void populate_port_choice();
	// Populate model choice
	void populate_model_choice();
	//Populate baud rate choice
	void populate_baud_choice();
	// Create the various parts of the form
	void create_status(int& x, int& y);
	void create_buttons(int& x, int& y);
	void create_rig_ant(int& x, int& y);
	void create_config(int& x, int& y);
	void create_connex(int& x, int& y);
	void create_serial(int& x, int& y);
	void create_network(int& x, int& y);
	void create_apps(int& x, int& y);
	void create_modifier(int& x, int& y);

	// Update rig with modifiers
	void modify_rig();

	// Rig status
	Fl_Output* op_status_;
	// Freq/Mode display
	Fl_Box* op_freq_mode_;

	// Control buttons
	Fl_Button* bn_connect_;
	Fl_Light_Button* bn_select_;
	Fl_Button* bn_start_;

	// Rig and antenna selection
	Fl_Choice* ch_rig_model_;
	field_input* ip_antenna_;

	// Configuartion - 3 tabs
	Fl_Tabs* config_tabs_;
	// Connection tab - either serial or network
	Fl_Group* connect_tab_;
	Fl_Group* serial_grp_;

	Fl_Choice* ch_port_name_;
	Fl_Check_Button* bn_all_ports_;
	Fl_Choice* ch_baud_rate_;
	Fl_Check_Button* bn_all_rates_;

	Fl_Group* network_grp_;
	// Hamlib widgets to revalue when rig selected changes
	Fl_Input* ip_port_;
	// Application tab
	Fl_Group* app_tab_;
	Fl_Input* ip_rig_app_;
	Fl_Input* ip_app_[NUMBER_APPS];

	// Modifier widgets
	Fl_Group* modifier_tab_;
	Fl_Check_Button* bn_mod_freq_;
	Fl_Float_Input* ip_freq_;
	Fl_Check_Button* bn_gain_;
	Fl_Int_Input* ip_gain_;
	Fl_Check_Button* bn_power_;
	Fl_Float_Input* ip_power_;

	// Add all ports to port choice
	bool use_all_ports_;
	// Use all baud rates
	bool use_all_rates_;

	rig_if* rig_;
	// hamlib data
	rig_if::hamlib_data_t hamlib_data_;
	// Current mode
	rig_port_e mode_;
	// Current antenna
	string antenna_;
	// flrig config paramaters
	string app_flrig_;
	string apps_[NUMBER_APPS];
	

	// Modifier attributes
	// Add a fixed offset - eg for transverters
	bool modify_freq_;
	// The fixed frequency offset
	double freq_offset_;
	// Change the power by a gain ration
	bool modify_gain_;
	// The gain of an amplifier
	int gain_;
	// Use fixed power - eg non-CAT rigs
	bool modify_power_;
	// The fixed power to use
	double power_;


};


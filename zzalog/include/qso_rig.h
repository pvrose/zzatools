#pragma once

// #include "rig_if.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Group.H>

using namespace std;

class field_input;
class rig_if;
struct hamlib_data_t;
class filename_input;
class Fl_Output;
class Fl_Button;
class Fl_Box;
class Fl_Light_Button;
class Fl_Choice;
class Fl_Tabs;
class Fl_Check_Button;
class Fl_Input;
class Fl_Int_Input;
class Fl_Float_Input;
class Fl_Value_Slider;
class Fl_Menu_Button;
class Fl_Preferences;

const int NUMBER_RIG_APPS = 2;
const string RIG_APP_NAMES[NUMBER_RIG_APPS] = { "FLRig", "WFView" };
// Bit-wise flags for redarwing qso_rig
const uchar DAMAGE_STATUS = 1;
const uchar DAMAGE_VALUES = 2;
const uchar DAMAGE_ADDONS = 4;
const uchar DAMAGE_ALL = 0xFF;


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
	void enable_widgets(uchar damage);
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
	// Disconnect rig
	void disconnect();
	// Get the selected CAT - return null byte if none else return ASCII digit
	const char* cat();

protected:

	struct cat_data_t {
		hamlib_data_t* hamlib = nullptr;
		bool use_cat_app = false;
		string app = "";
		string nickname = "";
	};

	enum rig_state_t : uchar {
		NO_RIG,            // No rig specified
		NO_CAT,            // No CAT available
		DISCONNECTED,      // Rig not connected
		OPENING,           // Opeining rig
		OPEN,              // rig has been opened and is connected
		POWERED_DOWN,      // Rig has powered down
		UNRESPONSIVE,      // Rig has not responded for a while
		STARTING,          // The app has been invoked but may not be ready to connect
	};

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
	// Callback use app button
	static void cb_bn_use_app(Fl_Widget* w, void* v);

	// Callback to add amplifier 
	static void cb_bn_amplifier(Fl_Widget* w, void* v);
	// Callback to add transverter
	static void cb_bn_transverter(Fl_Widget* w, void* v);

	// Call back start flrig
	static void cb_bn_start(Fl_Widget* w, void* v);
	// Tabs clicked
	static void cb_config(Fl_Widget* w, void* v);
	// Timeout callback
	static void cb_timeout(Fl_Widget* w, void* v);
	// CAT index menu items
	static void cb_select_cat(Fl_Widget* w, void* v);
	// Add a new CAT method
	static void cb_new_cat(Fl_Widget* w, void* v);
	// Delete current CAT method
	static void cb_del_cat(Fl_Widget* w, void* v);
	// S-meter peak smapling
	static void cb_smeters(Fl_Widget* w, void* v);
	// Show app
	static void cb_show_app(Fl_Widget* w, void* v);

	// Get hamlib data
	void find_hamlib_data();

	//populate port choice
	void populate_port_choice();
	// Populate model choice
	void populate_model_choice();
	//Populate baud rate choice
	void populate_baud_choice();
	// Populate index menu button
	void populate_index_menu();
	// Create the various parts of the form
	void create_status(int x, int y);
	void create_buttons(int x, int y);
	void create_rig_ant(int x, int y);
	void create_config(int x, int y);
	void create_connex(int x, int y);
	void create_serial(int x, int y);
	void create_network(int x, int y);
	void create_defaults(int X, int Y);
	void create_accessory(int X, int Y);
	void create_timeout(int X, int Y);

	void load_cat_data(cat_data_t* data, Fl_Preferences settings);
	void save_cat_data(cat_data_t* data, Fl_Preferences settings);

	void modify_hamlib_data();

	rig_state_t rig_state();

	Fl_Group* status_grp_;
	// Rig status
	Fl_Output* op_status_;
	// Rig TX/RX
	Fl_Button* bn_tx_rx_;
	// CAT index
	Fl_Menu_Button* bn_index_;
	// Freq/Mode display
	Fl_Box* op_freq_mode_;
	// Instantaneous mode
	Fl_Check_Button* bn_instant_;

	// Control buttons
	Fl_Group* buttons_grp_;
	Fl_Button* bn_connect_;
	Fl_Light_Button* bn_select_;
	Fl_Button* bn_start_;

	// Rig and antenna selection
	Fl_Group* rig_ant_grp_;
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
	Fl_Light_Button* bn_use_app_;
	Fl_Button* bn_show_app_;
	filename_input* ip_app_name_;
	// Hamlib widgets to revalue when rig selected changes
	Fl_Input* ip_port_;

	Fl_Group* defaults_tab_;
	Fl_Output* op_pwr_type_;
	Fl_Float_Input* ip_max_pwr_;
	Fl_Output* op_freq_type_;
	Fl_Float_Input* ip_xtal_;

	Fl_Group* accessory_tab_;
	Fl_Check_Button* bn_amplifier_;
	Fl_Int_Input* ip_gain_;
	Fl_Check_Button* bn_transverter_;
	Fl_Float_Input* ip_offset_;
	Fl_Float_Input* ip_tvtr_pwr_;

	// Operational settings
	Fl_Group* timeout_tab_;
	Fl_Value_Slider* v_timeout_;
	Fl_Value_Slider* v_smeters_;


	// Add all ports to port choice
	bool use_all_ports_;
	// Use all baud rates
	bool use_all_rates_;

	rig_if* rig_;
	// hamlib data
	vector<cat_data_t*> cat_data_;
	// Current mode - note C enum in hamlib/rig.h
	uint16_t mode_;
	// Current antenna
	string antenna_;
	// Rig was good
	bool rig_ok_;
	// CAT index
	int cat_index_;
	// Previous rig state
	rig_state_t rig_state_;
	// Flag set when start button pressed and cleared on connect button
	bool rig_starting_;

	// Map the hamlig model_id to position in rig_model choice
	map<int, int> rig_choice_pos_;
	//Display instantaneous values
	bool instant_meters_;

};


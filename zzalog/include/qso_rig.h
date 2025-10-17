#pragma once

// #include "rig_if.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Group.H>



class field_input;
class rig_if;
struct hamlib_data_t;
struct cat_data_t;
struct rig_data_t;
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

//! Number of supported rig access apps.
const int NUMBER_RIG_APPS = 2;
//! The supported rig access apps.
const std::string RIG_APP_NAMES[NUMBER_RIG_APPS] = { "FLRig", "WFView" };
//! Bit-wise flags for redrawing qso_rig
const uchar DAMAGE_STATUS = 1;     //!< Update status
const uchar DAMAGE_VALUES = 2;     //!< Update values read from the rig.
const uchar DAMAGE_ADDONS = 4;     //!< Update the "Accessories" tab.
const uchar DAMAGE_AUTOS = 8;      //!< Update the "Auto" tab.
const uchar DAMAGE_ALL = 0xFF;     //!< Update everything.


//! This class displays and controls the status of a single rig connection.
class qso_rig :
    public Fl_Group
{

public:

	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_rig(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
	~qso_rig();

	//! Inherited from Fl_Group::handle(): Allows keyboard F1 to open userguide.
	virtual int handle(int event);

	//! Load configuration data from rig_data.
	void load_values();
	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets after data change.
	void enable_widgets(uchar damage);
	//! Save changes back to rig_data.
	void save_values();
	//! Connect or disconnect the rig: depends on initial comnection state.
	void switch_rig();
	//! Called from ticker callback to poll rig values every 1 second.
	void ticker();
	//! Callback every 1 second.
	static void cb_ticker(void* v);
	//! A new rig connection has been made, redraw everything.
	void new_rig();
	//! Retunrs the rig connection interface rig_if.
	rig_if* rig();
	//! Returns preferred antenna for this rig.
	std::string antenna();
	//! Disconnect rig
	void disconnect();
	//! Returns the nickname of the CAT interface used.
	const char* cat();

protected:

	//! State of the interface. 
	enum rig_state_t : uchar {
		NO_RIG,            //!< No rig specified
		NO_CAT,            //!< No CAT available
		DISCONNECTED,      //!< Rig not connected
		OPENING,           //!< Opeining rig
		OPEN,              //!< rig has been opened and is connected
		POWERED_DOWN,      //!< Rig has powered down
		UNRESPONSIVE,      //!< Rig has not responded for a while
		STARTING,          //!< The app has been invoked but may not be ready to connect
	};

	//! Callback from "Rig" choice. 
	static void cb_ch_model(Fl_Widget* w, void* v);
	//! Callback from "Port" choice.
	static void cb_ch_port(Fl_Widget* w, void* v);
	//! Callback from "Baud rate" choice.
	static void cb_ch_baud(Fl_Widget* w, void* v);
	//! Callback feom "Override" check.
	static void cb_ch_over(Fl_Widget* w, void* v);
	//! Callback from "All" check.
	static void cb_bn_all(Fl_Widget* w, void* v);
	//! Callback from "Host Port" input.
	static void cb_ip_port(Fl_Widget* w, void* v);
	//! Callback from "Connect" button.
	static void cb_bn_connect(Fl_Widget* w, void* v);
	//! Callback from "Select" button.
	static void cb_bn_select(Fl_Widget* w, void* v);
	//! Callback from "Use" button.
	static void cb_bn_use_app(Fl_Widget* w, void* v);

	//! Callback from "Amplifer" chack.
	static void cb_bn_amplifier(Fl_Widget* w, void* v);
	//! Callback from "Transverter" check.
	static void cb_bn_transverter(Fl_Widget* w, void* v);

	//! Callback from "Start" button.
	static void cb_bn_start(Fl_Widget* w, void* v);
	//! Callback when tabs selected.
	static void cb_config(Fl_Widget* w, void* v);
	//! Callback from "Timeout sec." slider.
	static void cb_timeout(Fl_Widget* w, void* v);
	//! Callback from CAT menu.
	static void cb_select_cat(Fl_Widget* w, void* v);
	//! Callback when "New" is selected in CAT menu. 
	static void cb_new_cat(Fl_Widget* w, void* v);
	//! Callback when "Delete" is selected in CAT menu.
	static void cb_del_cat(Fl_Widget* w, void* v);
	//! Callback from "S-meter Stack" slider.
	static void cb_smeters(Fl_Widget* w, void* v);
	//! Callback from "TO Count" slider.
	static void cb_to_count(Fl_Widget* w, void* v);
	//! Callback from "Show" button.
	static void cb_show_app(Fl_Widget* w, void* v);
	//! Callback from "Defaults/Power" choice
	static void cb_ch_power(Fl_Widget* w, void* v);
	//! Callback from "Defaults/O/R" check.
	static void cb_bn_override(Fl_Widget* w, void* v);
	//! Callbach from "Auto/Start" check.
	static void cb_bn_autostart(Fl_Widget* w, void* v);
	//! Callback from "Auto/Connect" check.
	static void cb_bn_autoconn(Fl_Widget* w, void* v);
	//! Callback from "Auto/Delay" slider.
	static void cb_connect_delay(Fl_Widget* w, void* v);
	//! Callback from timer started by "Start" button.
	static void cb_start_timer(void* v);

	//! populate port choice
	void populate_port_choice();
	//! Populate model choice
	void populate_model_choice();
	//! Populate baud rate choice
	void populate_baud_choice();
	//! Populate index menu button
	void populate_index_menu();
	//! Populate power type choice
	void populate_power_choice();
	// Create the various parts of the form
	void create_status(int x, int y);       //!< Create the status widgets
	void create_buttons(int x, int y);      //!< Create the "Connect" etc buttons
	void create_rig_ant(int x, int y);      //!< Createthe "Rig" and "Antenna" widgets.
	void create_config(int x, int y);       //!< Create the tabs 
	void create_connex(int x, int y);       //!< Craete the "Connection" tab.
	void create_serial(int x, int y);       //!< Create serial port connection widgets.
	void create_network(int x, int y);      //!< Create network port connection widgets.
	void create_defaults(int X, int Y);     //!< Create "Defaults" tab.
	void create_accessory(int X, int Y);    //!< Create "Accessories" tab.
	void create_timeout(int X, int Y);      //!< Create "Timeout etc" tab.
	void create_auto(int X, int Y);         //!< Create "Auto" tab.

	//! Modify the values returned from the rig according to the modifier attributes.
	void modify_hamlib_data();

	//! The rig state.
	rig_state_t rig_state();

	//! Status group
	Fl_Group* status_grp_;
	//! Output: Rig status text
	Fl_Output* op_status_;
	//! Button: Rig status icon
	Fl_Button* bn_tx_rx_;
	//! Menu: CAT methods
	Fl_Menu_Button* bn_index_;
	//! Box: Freq/Mode display
	Fl_Box* op_freq_mode_;
	//! Check: Instantaneous mode
	Fl_Check_Button* bn_instant_;

	//! Control buttons
	Fl_Group* buttons_grp_;
	Fl_Button* bn_connect_;        //!< Button: "Connect"|"Disconnect" 
	Fl_Light_Button* bn_select_;   //!< Light: "Select"|"Use"
	Fl_Button* bn_start_;          //!< Button: "Start"

	//! Rig and antenna selection
	Fl_Group* rig_ant_grp_;    
	Fl_Choice* ch_rig_model_;      //!< Menu: "Rig" shows available rigs
	field_input* ip_antenna_;      //!< Menu: "Antenna" shows available antennas

	//! Configuartion - 5 tabs
	Fl_Tabs* config_tabs_;
	//! Connection tab - either serial or network
	Fl_Group* connect_tab_;
	Fl_Group* serial_grp_;           //!< Serial port configuration

	Fl_Choice* ch_port_name_;        //!< Menu: "Port" shows available serial ports
	Fl_Check_Button* bn_all_ports_;  //!< Check: "All" if std::set all ports available or not.
	Fl_Choice* ch_baud_rate_;        //!< Menu: "Baud rate" shows available data speeds.
	Fl_Check_Button* bn_all_rates_;  //!< Check: "All" if std::set all data speeds.

	Fl_Group* network_grp_;          //!< Network port configuration
	Fl_Light_Button* bn_use_app_;    //!< Light: "Use" use an application to connect.
	Fl_Button* bn_show_app_;         //!< Button: "Show" open a file_editor to show script used to connect.
	filename_input* ip_app_name_;    //!< Input: Command or script namme to connect.
	// Hamlib widgets to revalue when rig selected changes
	Fl_Input* ip_port_;              //!< Input: "Host:Port" network address eg localhost:12345.

	Fl_Group* defaults_tab_;         //!< Tab: "Defaults"
	Fl_Output* op_pwr_type_;         //!< Output: "Power" displays how power is calculated.
	Fl_Choice* ch_pwr_type_;         //!< Menu: "Power" selects how power is calculated
	Fl_Float_Input* ip_max_pwr_;     //!< Input: "Power" maximum power (in watts) for calculation.
	Fl_Check_Button* bn_override_;   //!< Check: "O/R" allows default power and frequency calculation to be overridden.
	Fl_Output* op_freq_type_;        //!< Output: "Frequency" displays how frequency is calculated.
	Fl_Float_Input* ip_xtal_;        //!< Input: "Frequency" allows fixed frequency (in megahertz) to be specified.

	Fl_Group* accessory_tab_;        //!< Tab: "Accessories"
	Fl_Check_Button* bn_amplifier_;  //!< Check: "Amplifier" indicates an amplifier is attached.
	Fl_Int_Input* ip_gain_;          //!< Input: Amplifier gain (in dB)
	Fl_Check_Button* bn_transverter_;//!< Check: "Transverter" is fitted.
	Fl_Float_Input* ip_offset_;      //!< Input: Transverter local oscillator frequency (in megahertz).
	Fl_Float_Input* ip_tvtr_pwr_;    //!< Input: Transverter output power (in watts).

	// Operational settings
	Fl_Group* timeout_tab_;          //!< Tab: "Timeouts etc"
	Fl_Value_Slider* v_timeout_;     //!< Slider: "Timeout sec." connection timeout (in seconds)
	Fl_Value_Slider* v_smeters_;     //!< Slider: "S-meter stack" the number of readings used to smooth S-meter reading. 
	Fl_Value_Slider* v_to_count_;    //!< Slider: "TO Count" the number of timeouts allowed before ZZALOG stops trying to read the value thattimed out.

	// Auto start and connect settings
	Fl_Group* auto_tab_;             //!< Tab: "Auto"
	Fl_Check_Button* bn_autostart_;  //!< Check: "Start" Rig access command is called automatically on ZZALOG start-up. 
	Fl_Check_Button* bn_autoconn_;   //!< Check: "Connect" ZZALOG attempts to connect to the rig shortly after the access script is launched.
	Fl_Value_Slider* v_connect_delay_;//!< Slider: "Delay" the delay (in seconds) between starting the access script and connecting to the rig.


	//! Add all ports to port choice
	bool use_all_ports_;
	//! Use all baud rates
	bool use_all_rates_;

	//! The rig interface.
	rig_if* rig_;
	//! CAT data for particular RIG and cat_index_
	cat_data_t* cat_data_;
	//! Rig data
	rig_data_t* rig_info_;
	//! Rig has connected and is responding.
	bool rig_ok_;
	//! Current rig state.
	rig_state_t rig_state_;
	//! Flag std::set when start button pressed and cleared on connect button
	bool rig_starting_;

	//! Map the hamlib model_id to position in rig choice menu.
	std::map<int, int> rig_choice_pos_;
};


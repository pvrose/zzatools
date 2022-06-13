#ifndef __STN_DIALOG__
#define __STN_DIALOG__


#include "../zzalib/callback.h"
#include "../zzalib/rig_if.h"
#include "record.h"
#include "intl_widgets.h"
#include "alarm_dial.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Button.H>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// Version of intl_editor that handles events such as clicking on words
	class spad_editor :
		public intl_editor
	{
	public:
		spad_editor(int x, int y, int w, int h);
		~spad_editor();

		int handle(int event);

	protected:

	};

	// This class provides the dialog to chage the current station settings: rig, antenna and QTH
	class qso_manager :
		public Fl_Window
	{
		// Logging mode - used when initialising a record
	public:
		enum logging_mode_t {
			LM_FOR_PARSING, // Use current time
			LM_OFF_AIR,     // Off-line logging (w/o radio)
			LM_ON_AIR_CAT,  // Real -time logging - data from radio
			LM_ON_AIR_COPY, // Real-time logging - data from selected QSO
			LM_ON_AIR_TIME, // Real-time logging - date/time only
			LM_IMPORTED,    // Import from modem software (w/ or w/o radio)
		};

	protected:
		// A bit of a misnomer - but the category of settings
		enum item_t {
			RIG,
			ANTENNA,
			CALLSIGN,
			NONE
		};

		// Use item destination
		enum use_item_t {
			SELECTED_ONLY,
			SELECTED_NEW,
			NEW_ONLY
		};

		// Hamlib parameters 
		struct hamlib_data {
			// Manufacturer
			string mfr = "";
			// Model
			string model = "";
			// Portname
			string port_name = "";
			// Baud rate
			string baud_rate = "9600";
			// Model ID - as knoen by hamlib
			int model_id = -1;
			// Override caps
			bool override_caps = false;
		};
		// Flrig parameters
		struct flrig_data {
			// IP address
			string ip_address = "127.0.0.1";
			// Port name
			int port = 12345;
			// Resurce
			string resource = "/RPC2";
		};
		// Alarm parameters
		struct alarm_data {
			double swr_warning = 1.5;
			double swr_error = 2.5;
			double power_warning = 95;
			double voltage_minimum = 13.8 * 0.85;
			double voltage_maximum = 13.8 * 1.15;
		};
		// Rig parameters (from handler onwards - rig only)
		struct rig_xdata {
			// Rig currently in use
			bool active = false;
			// Bands supported by rig
			vector<string> intended_bands;
			// CAT handler
			rig_handler_t handler = RIG_NONE;
			// Polling intervals
			double fast_poll_interval = 1.0;
			double slow_poll_interval = 60.0;
			// Hamlib data
			hamlib_data hamlib_params;
			// Flrig data
			flrig_data flrig_params;
			// Alarm data
			alarm_data alarms;
		};
		// Antenna parameters
		struct item_data {
			// Antenna currently in use
			bool active = false;
			//Bands supported
			vector<string> intended_bands;
			// Rig-only data
			rig_xdata rig_data;

			item_data() {
				intended_bands.clear();
			}
		};


		// This class provides individual grouping for rig, antenna.
		class common_grp :
			public Fl_Group
		{


		public:

			common_grp(int X, int Y, int W, int H, const char* label, item_t type);
			virtual ~common_grp();

			// get settings
			virtual void load_values();
			// create the form
			virtual void create_form(int X, int Y);
			// save values
			virtual void save_values();
			// button callback - add
			static void cb_bn_add(Fl_Widget* w, void* v);
			// button callback - delete
			static void cb_bn_del(Fl_Widget* w, void* v);
			// button callback - all/active items
			static void cb_bn_all(Fl_Widget* w, void* v);
			// button callback - active/deactive
			static void cb_bn_activ8(Fl_Widget* w, void* v);
			// choice callback
			static void cb_ch_stn(Fl_Widget* w, void* v);
			// Multi-browser callback
			static void cb_mb_bands(Fl_Widget* w, void* v);
			// Item
			static void cb_ch_item(Fl_Widget* w, void* v);

		protected:
			// Choice widget
			Fl_Widget* choice_;
			// Active widget
			Fl_Widget* active_;
			// Band select widget
			Fl_Widget* band_browser_;
			// selected item
			int item_no_;
			// all items 
			list<string> all_items_;
			// display all items
			bool display_all_items_;
			// Item name
			string my_name_;
			string next_name_;
			// The settings
			Fl_Preferences* my_settings_;
			// Map per item name to item data
			map<string, item_data> item_info_;

		public:
			// (re)populate the choice widget
			void populate_choice();
			// Populate band widget
			void populate_band();
			// return rig_info
			item_data& info();
			// return current name
			string& name();
			// return naext name
			string& next();
			// SAve next value
			void save_next_value();
			// type
			item_t type_;
			// in text for Fl_Preference
			string settings_name_;
		};


	// qso_manager
	public:
		qso_manager(int W, int H, const char* label);
		virtual ~qso_manager();

		// Override to tell menu when it opens and closes
		virtual int handle(int event);

		// get settings - rely on individual groups to do it
		virtual void load_values();
		// create the form
		virtual void create_form(int X, int Y);
		// enable/disable widgets - rely on individual groups to do it
		virtual void enable_widgets();
		// save values
		virtual void save_values();
		// Get bands and order them
		void order_bands();

		// Callback for rig handler selection (radio)
		static void cb_rad_handler(Fl_Widget* w, void* v);
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
		// Callback - flrig IP address
		static void cb_ip_ipa(Fl_Widget* w, void* v);
		// Callback - flrig port number
		static void cb_ip_portn(Fl_Widget* w, void* v);
		// Callback  flrig resource indicator
		static void cb_ip_resource(Fl_Widget* w, void* v);
		// Callback - SWR alarm
		static void cb_alarm_swr(Fl_Widget* w, void* v);
		// Callback - Power alarm
		static void cb_alarm_pwr(Fl_Widget* w, void* v);
		// Callback - Vdd alarm
		static void cb_alarm_vdd(Fl_Widget* w, void* v);
		// Callback - Polling intervals
		static void cb_ctr_pollfast(Fl_Widget* w, void* v);
		static void cb_ctr_pollslow(Fl_Widget* w, void* v);
		// Callback - Connect button
		static void cb_bn_connect(Fl_Widget* w, void* v);
		// Callback - use rig/ant buttons
		static void cb_bn_use_items(Fl_Widget* w, void* v);
		// Callback - close button
		static void cb_close(Fl_Widget* w, void* v);
		// Call back - general button action
		static void cb_action(Fl_Widget* w, void* v);
		// Callback - start button
		static void cb_start(Fl_Widget* w, void* v);
		// Callback - save button
		static void cb_save(Fl_Widget* w, void* v);
		// Callback - cancel button
		static void cb_cancel(Fl_Widget* w, void* v);
		// Callback - Worked B4? button
		static void cb_wkb4(Fl_Widget* w, void* v);
		// Callback - Parse callsign
		static void cb_parse(Fl_Widget* w, void* v);
		// Callback - frequency input
		static void cb_ip_freq(Fl_Widget* w, void* v);
		// Callback - band choice
		static void cb_ch_mode(Fl_Widget* w, void* v);
		// Callback - power input
		static void cb_ip_power(Fl_Widget* w, void* v);
		// Callback - 1s timer
		static void cb_timer_clock(void* v);

		// Set font
		void set_font(Fl_Font font, Fl_Fontsize size);

		// Actions attached to the various buttons and keyboard shortcuts
		enum actions {
			WRITE_CALL,
			WRITE_NAME,
			WRITE_QTH,
			WRITE_GRID,
			WRITE_RST_SENT,
			WRITE_RST_RCVD,
			WRITE_FIELD
		};

		//populate port choice
		void populate_port_choice();
		// Populate model choice
		void populate_model_choice();
		//Populate baud rate choice
		void populate_baud_choice();
		// Populate QTH choice
		void populate_qth_choice();
		// Get logging mode
		logging_mode_t logging_mode();
		// Set logging mode
		void logging_mode(logging_mode_t mode);
		// Called when rig is read
		void rig_update(string frequency, string mode, string power);
		// Called when rig is closed
		void update();
		// Save next QSO values
		void save_next_values();
		// Update locations
		void update_locations();
		// Load locations
		void load_locations();
		// Start QSO
		record* start_qso();
		// Create a dummy qso - eg for parsing a callsign
		record* dummy_qso();
		// End QSO
		void end_qso();
		// QSO i n progress
		bool qso_in_progress();


	protected:
		// Create the various widgets sets
		void create_use_widgets(int &curr_x, int &curr_y);
		void create_spad_widgets(int& curr_x, int& curr_y);
		void create_cat_widgets(int& curr_x, int& curr_y);
		void create_alarm_widgets(int& curr_x, int& curr_y);
		void create_clock_widgets(int& curr_x, int& curr_y);

		// Enable the various sets of widgets
		void enable_use_widgets();
		void enable_spad_widgets();
		void enable_cat_widgets();
		void enable_alarm_widgets();
		void enable_clock_widgets();

		// Set of bands in frequency order
		list<string> ordered_bands_;
		// Display all ports
		bool all_ports_;
		// Logging mode
		logging_mode_t logging_mode_;
		// Selecetd CAT but not connecyed
		bool wait_connect_;
		// Widgets have been xcreated
		bool created_;
		// Selected equipment
		string selected_qth_;
		// Next name
		string next_qth_;
		// Avaiable QTHs
		set<string> all_qths_;
		// Keep track of alarm activation to avoid too many alarms for the same violation
		enum { SWR_OK, SWR_WARNING, SWR_ERROR } previous_swr_alarm_, current_swr_alarm_;
		enum { POWER_OK, POWER_WARNING } previous_pwr_alarm_, current_pwr_alarm_;
		enum { VDD_UNDER, VDD_OK, VDD_OVER } previous_vdd_alarm_, current_vdd_alarm_;
		// Reemeber last tx SWR and Power readings to display during receive
		double last_tx_swr_;
		double last_tx_pwr_;

		// Groups
		common_grp* rig_grp_;
		common_grp* antenna_grp_;
		common_grp* callsign_grp_;
		Fl_Group* hamlib_grp_;
		Fl_Group* flrig_grp_;
		Fl_Group* norig_grp_;
		Fl_Group* cat_grp_;
		Fl_Group* cat_sel_grp_;
		Fl_Group* alarms_grp_;
		Fl_Group* clock_grp_;
		// widgets
		// Hamlib widgets to revalue when rig selected changes
		Fl_Widget* mfr_choice_;
		Fl_Widget* rig_model_choice_;
		Fl_Widget* port_if_choice_;
		Fl_Widget* baud_rate_choice_;
		Fl_Widget* rig_choice_;
		Fl_Widget* override_check_;
		Fl_Widget* show_all_ports_;

		Fl_Choice* ch_qth_;

		// The text items
		Fl_Text_Buffer* buffer_;
		// The editor
		spad_editor* editor_;
		// Save button
		Fl_Button* bn_save_;
		// Cancel button
		Fl_Button* bn_cancel_;
		// Start button
		Fl_Button* bn_start_;
		// Frequency input
		Fl_Input* ip_freq_;
		// Mode choice
		Fl_Choice* ch_mode_;
		// Power input
		Fl_Input* ip_power_;
		// Group for freq/power/mode
		Fl_Group* grp_fpm_;
		// Logging mode
		Fl_Choice* ch_logmode_;
		Fl_Radio_Round_Button* bn_nocat_;
		Fl_Radio_Round_Button* bn_hamlib_;
		Fl_Radio_Round_Button* bn_flrig_;
		Fl_Spinner* ctr_pollfast_;
		Fl_Spinner* ctr_pollslow_;
		alarm_dial* dial_swr_;
		alarm_dial* dial_pwr_;
		alarm_dial* dial_vdd_;
		Fl_Button* connect_bn_;
		Fl_Widget* ant_rig_box_;
		Fl_Button* bn_time_;
		Fl_Button* bn_date_;
		Fl_Button* bn_local_;

		// The record being entered or used
		record* current_qso_;
		// The fieldname
		string field_;

		// Scratchpad editor font
		Fl_Font font_;
		Fl_Fontsize fontsize_;

		const static int WEDITOR = 400;


	};

}
#endif


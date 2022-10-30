#ifndef __STN_DIALOG__
#define __STN_DIALOG__


#include "../zzalib/callback.h"
#include "../zzalib/rig_if.h"
#include "record.h"
#include "intl_widgets.h"
#include "alarm_dial.h"
#include "field_choice.h"

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

	//// Version of intl_editor that handles events such as clicking on words
	//class spad_editor :
	//	public intl_editor
	//{
	//public:
	//	spad_editor(int x, int y, int w, int h);
	//	~spad_editor();

	//	int handle(int event);

	//protected:

	//};

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
			QTH,
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
			// Enable widgets
			virtual void enable_widgets();
			// button callback - add
			static void cb_bn_add(Fl_Widget* w, void* v);
			// button callback - delete
			static void cb_bn_del(Fl_Widget* w, void* v);
			// choice callback
			static void cb_ch_stn(Fl_Widget* w, void* v);
			// Multi-browser callback
			static void cb_mb_bands(Fl_Widget* w, void* v);
			// Item
			static void cb_ch_item(Fl_Widget* w, void* v);

		protected:
			// Choice widget
			Fl_Widget* choice_;
			// Band select widget
			Fl_Widget* band_browser_;
			// Current item value
			Fl_Widget* op_settings_;
			// selected item
			int item_no_;
			// all items 
			list<string> all_items_;
			// Item name
			string my_name_;
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
			// type
			item_t type_;
			// in text for Fl_Preference
			string settings_name_;
			// Set current settings to name
			void update_settings_name();
		};

		// Class for QSO group of widgets
		class qso_group :
			public Fl_Group
		{
			friend class qso_manager;

		public:
			qso_group(int X, int Y, int W, int H, const char* l);
			~qso_group();

			// get settings
			void load_values();
			// Create form
			void create_form(int X, int Y);
			// Enable/disab;e widgets
			void enable_widgets();
			// save value
			void save_values();

			// Callbacks
		protected:
			// Start QSO - If not started: set start time/date; copy per logging mode; add any set fields. If started: add any set fields
			static void cb_start_qso(Fl_Widget* w, void* v);
			// Log QSO - If not started: as cb_start_qso. Also: save QSO
			static void cb_log_qso(Fl_Widget* w, void* v);
			// Cancel QSO - delete QSO; clear fields
			static void cb_cancel_qso(Fl_Widget* w, void* v);
			// Callback - Worked B4? button
			static void cb_wkb4(Fl_Widget* w, void* v);
			// Callback - Parse callsign
			static void cb_parse(Fl_Widget* w, void* v);
			//// Field input - v: field name
			//static void cb_inp_field(Fl_Widget* w, void* v);
			//// Field choice - v: widget no. X1 to X7
			//static void cb_field(Fl_Widget* w, void* v);
			// Callback - frequency input
			static void cb_ip_freq(Fl_Widget* w, void* v);
			// Callback - band choice
			static void cb_ch_mode(Fl_Widget* w, void* v);
			// Callback - power input
			static void cb_ip_power(Fl_Widget* w, void* v);
			// Initialise setial number
			static void cb_init_serno(Fl_Widget* w, void* v);
			// Increment serial number
			static void cb_inc_serno(Fl_Widget* w, void* v);
			// Decrement serial number
			static void cb_dec_serno(Fl_Widget* w, void* v);
			// Define contest exchange format
			static void cb_def_format(Fl_Widget* w, void* v);
			// Start/Stop contest mode
			static void cb_ena_contest(Fl_Widget* w, void* v);
			// Pause contest mode
			static void cb_pause_contest(Fl_Widget* w, void* v);
			// Exchange format choice
			static void cb_format(Fl_Widget* w, void* v);
			// Add exchange button
			static void cb_add_exch(Fl_Widget* w, void* v);
			// Logging mode
			static void cb_logging_mode(Fl_Widget* w, void* v);

			// Add contest exchanges
			void populate_exch_fmt();
			// Update fields in record
			void update_record();
			// Copy fields from record
			void update_fields();
			// Initialise fields
			void initialise_fields();
			// Copy from an existing record
			void copy_record(record* old_record);
			// Copy fields from CAT
			void copy_cat();
			// Clear QSO fields
			void clear_qso();
			// Add new format - return format index
			int add_format_id(string id);
			// Add new format definition 
			void add_format_def(int ix, bool tx);

			// Logging mode
			logging_mode_t logging_mode_;
			// Current record
			record* current_qso_;
			// Contest ID
			string contest_id_;
			// Contest exchange format index
			int exch_fmt_ix_;
			// And its id
			string exch_fmt_id_;
			// Exchanges: format ID, TX and RX fields
			static const int MAX_CONTEST_TYPES = 100;
			string ef_ids_[MAX_CONTEST_TYPES];
			string ef_txs_[MAX_CONTEST_TYPES];
			string ef_rxs_[MAX_CONTEST_TYPES];
			int max_ef_index_;
			// Adding an exchange
			enum field_mode_t {
				NO_CONTEST = 0,  // Normal non-contest logging behaviour
				CONTEST,         // Normal contest logging behaviour
				PAUSED,          // Log non-contest within contest logging
				NEW,             // A new format id is selected
				DEFINE,          // Define new contest exchange definition
				EDIT             // Edit contest exchange definition
			} field_mode_;
			
			// Serial number
			int serial_num_;
			// Loggable field names
			static const int NUMBER_FIELDS = 11;


			// Widgets

			// Log button
			Fl_Button* bn_log_qso_;
			// Cancel button
			Fl_Button* bn_cancel_qso_;
			// Start button
			Fl_Button* bn_start_qso_;
			// Worked before button
			Fl_Button* bn_wkd_b4_;
			// Parse button
			Fl_Button* bn_parse_;
			// Frequency input
			Fl_Input* ip_freq_;
			// Mode choice
			field_choice* ch_mode_;
			// Power input
			Fl_Input* ip_power_;
			// Group for freq/power/mode
			Fl_Group* grp_fpm_;
			// Logging mode
			Fl_Choice* ch_logmode_;
			// Contest ID
			field_choice* ch_contest_id_;
			// Contest exchange
			Fl_Input_Choice* ch_format_;
			// Add exchange
			Fl_Button* bn_add_exch_;
			// Define exchanges
			Fl_Button* bn_define_tx_;
			Fl_Button* bn_define_rx_;
			// TX Serial number
			Fl_Output* op_serno_;
			// Initialise serial number
			Fl_Button* bn_init_serno_;
			// Increment serial number
			Fl_Button* bn_inc_serno_;
			// Decrement serial number
			Fl_Button* bn_dec_serno_;
			// Pause contest
			Fl_Light_Button* bn_pause_;
			// Enable contest
			Fl_Light_Button* bn_enable_;
			// Callsign
			Fl_Input* ip_call_;
			// Notes
			Fl_Input* ip_notes_;
			// Field choices
			field_choice* ch_field_[NUMBER_FIELDS];
			// field inputs
			Fl_Input* ip_field_[NUMBER_FIELDS];

		};

		class clock_group :
			public Fl_Group

		{
			friend class qso_manager;

			clock_group(int X, int Y, int W, int H, const char* l);
			~clock_group();

			// get settings
			void load_values();
			// Create form
			void create_form(int X, int Y);
			// Enable/disab;e widgets
			void enable_widgets();
			// save value
			void save_values();

			// Callback - 1s timer
			static void cb_timer_clock(void* v);

			Fl_Button* bn_time_;
			Fl_Button* bn_date_;
			Fl_Button* bn_local_;
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
		// Get logging mode
		logging_mode_t logging_mode();
		// Set logging mode
		void logging_mode(logging_mode_t mode);
		// Called when rig is read
		void rig_update(string frequency, string mode, string power);
		// Called when rig is changed
		void update_rig();
		// Called when QSO is changed
		void update_qso();
		// Update locations
		void update_locations();
		// Load locations
		void load_locations();
		// QSO i n progress
		bool qso_in_progress();
		// Start QSO
		record* start_qso(bool add_to_book = true);
		// Create a dummy qso - eg for parsing a callsign
		record* dummy_qso();
		// End QSO
		void end_qso();


	protected:
		// Create the various widgets sets
		void create_use_widgets(int &curr_x, int &curr_y);
		void create_cat_widgets(int& curr_x, int& curr_y);
		void create_alarm_widgets(int& curr_x, int& curr_y);

		// Enable the various sets of widgets
		void enable_use_widgets();
		void enable_cat_widgets();
		void enable_alarm_widgets();

		// Set of bands in frequency order
		list<string> ordered_bands_;
		// Display all ports
		bool all_ports_;
		// Selecetd CAT but not connecyed
		bool wait_connect_;
		// Widgets have been xcreated
		bool created_;
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
		common_grp* qth_grp_;
		qso_group* qso_group_;
		Fl_Group* hamlib_grp_;
		Fl_Group* flrig_grp_;
		Fl_Group* norig_grp_;
		Fl_Group* cat_grp_;
		Fl_Group* cat_sel_grp_;
		Fl_Group* alarms_grp_;
		clock_group* clock_group_;
		// widgets
		// Hamlib widgets to revalue when rig selected changes
		Fl_Widget* mfr_choice_;
		Fl_Widget* rig_model_choice_;
		Fl_Widget* port_if_choice_;
		Fl_Widget* baud_rate_choice_;
		Fl_Widget* rig_choice_;
		Fl_Widget* override_check_;
		Fl_Widget* show_all_ports_;

		//// The text items
		//Fl_Text_Buffer* buffer_;
		//// The editor
		//spad_editor* editor_;
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

		// Scratchpad editor font
		Fl_Font font_;
		Fl_Fontsize fontsize_;

		const static int WEDITOR = 400;

	};
}

#endif


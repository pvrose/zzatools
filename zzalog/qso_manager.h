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
		enum stn_item_t {
			RIG = 1,
			ANTENNA = 2,
			CALLSIGN = 4,
			QTH = 8,
			NONE = 0
		};

		// Loggng state
		enum logging_state_t {
			QSO_INACTIVE,    // No QSO being edited
			QSO_PENDING,     // Collecting data for QSO - not qctive
			QSO_STARTED,     // QSO started
			QSO_EDIT,        // Editing existing QSO
		};

		//// Use item destination
		//enum use_item_t {
		//	SELECTED_ONLY,
		//	SELECTED_NEW,
		//	NEW_ONLY,
		//	CANCEL_USE
		//};

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
			// Port type
			rig_port_t port_type = RIG_PORT_NONE;
		};

		// Rig parameters (from handler onwards - rig only)
		struct cat_data {
			// Polling intervals
			double fast_poll_interval = 1.0;
			double slow_poll_interval = 60.0;
			//
			bool all_ports = false;
			// Hamlib data
			hamlib_data hamlib_params;
		};

		//// This class provides individual grouping for rig, antenna.
		//class common_grp :
		//	public Fl_Group
		//{
		//	class item_choice :
		//		public Fl_Input_Choice
		//	{
		//	public:
		//		item_choice(int X, int Y, int W, int H, const char* L = nullptr) :
		//			Fl_Input_Choice(X, Y, W, H, L) {}
		//		// Add escaping '/' and '&' characters
		//		void add(const char* label);
		//		// Remove above escapes from menubutton::text()
		//		const char* text();
		//	};

		//public:

		//	common_grp(int X, int Y, int W, int H, const char* label, stn_item_t type);
		//	virtual ~common_grp();

		//	// create the form
		//	virtual void create_form(int X, int Y);
		//	// Enable widgets
		//	virtual void enable_widgets();
		//	// button callback - add
		//	static void cb_bn_add(Fl_Widget* w, void* v);
		//	// button callback - delete
		//	static void cb_bn_del(Fl_Widget* w, void* v);
		//	// choice callback
		//	static void cb_ch_stn(Fl_Widget* w, void* v);
		//	// Multi-browser callback
		//	static void cb_mb_bands(Fl_Widget* w, void* v);
		//	// Item
		//	static void cb_ch_item(Fl_Widget* w, void* v);

		//protected:
		//	// Choice widget
		//	Fl_Widget* choice_;
		//	// Band select widget
		//	Fl_Widget* band_browser_;
		//	// selected item
		//	int item_no_;
		//	// Item name
		//	string my_name_;
		//	// Field name
		//	string my_field_;

		//public:
		//	// (re)populate the choice widget
		//	void populate_choice();
		//	// Populate band widget
		//	void populate_band();
		//	// return current name
		//	string& name();
		//	// type
		//	stn_item_t station_item_;
		//	// Update name and selecton
		//	void update_choice(string name);

		//};

		// Class for QSO group of widgets
		class qso_group :
			public Fl_Group
		{
			friend class qso_manager;
			friend class clock_group;

			enum copy_flags : int {
				CF_NONE = 0,
				CF_RIG_ETC = 1,
				CF_CAT = 2,
				CF_CLOCK = 4,
				CF_REST = 8,
				CF_ALL = CF_RIG_ETC | CF_CAT | CF_CLOCK | CF_REST,
				CF_NOTCLOCK = CF_RIG_ETC | CF_CAT | CF_REST
			};

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
			// Update MY_RIG etc
			void update_station_choices(stn_item_t item);

			// Callbacks
		protected:
			// Activate QSO logging 
			static void cb_activate(Fl_Widget* w, void* v);
			// Start QSO (log first if already in QSO_STARTED, Activate in QSO_INACTIVE))
			static void cb_start(Fl_Widget* w, void* v);
			// Log QSO (start first if in QSO_PENDING)
			static void cb_save(Fl_Widget* w, void* v);
			// Quit QSO logging - delete QSO; clear fields
			static void cb_cancel(Fl_Widget* w, void* v);
			// Go edit mode
			static void cb_edit(Fl_Widget* w, void* v);
			// Callback - Worked B4? button
			static void cb_wkb4(Fl_Widget* w, void* v);
			// Callback - Parse callsign
			static void cb_parse(Fl_Widget* w, void* v);
			// Field input - v: field name
			static void cb_ip_field(Fl_Widget* w, void* v);
			// Field choice - v: widget no. X1 to X7
			static void cb_ch_field(Fl_Widget* w, void* v);
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
			// Notes input field
			static void cb_ip_notes(Fl_Widget* w, void* v);

			// Add contest exchanges
			void populate_exch_fmt();
			// Copy fields from record
			void copy_qso_to_display(int flags);
			// Initialise fields
			void initialise_fields();
			// Copy from an existing record
			void copy_qso_to_qso(record* old_record, int flags);
			// Copy fields from CAT
			void copy_cat_to_qso();
			// Copy clock to QSO
			void copy_clock_to_qso(time_t clock);
			// Clear display fields
			void clear_display();
			// Clear QSO fields
			void clear_qso();
			// Add new format - return format index
			int add_format_id(string id);
			// Add new format definition 
			void add_format_def(int ix, bool tx);
			// State transition actions:-
			// Create a new record per loging mode
			void action_activate();
			// Clear all QSO infp
			void action_deactivate();
			// Add new record to book, add time on
			void action_start();
			// Stop QSO - 
			void action_save();
			// Cancel QSO - remove new record from book
			void action_cancel();
			// Copy selected QSO and allow it to be edited
			void action_edit();
			// Copy edited QSO back to selected QSO
			void action_save_edit();
			// Cancel editing
			void action_cancel_edit();

			// Logging mode
			logging_mode_t logging_mode_;
			// Logging state
			logging_state_t logging_state_;
			// Current record - being entered or edited
			record* current_qso_;
			// Selected record
			record* selected_qso_;
			// Saved current record
			record* original_qso_;
			// Current record number
			record_num_t current_rec_num_;
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
			// Map name back to index
			map<string, field_input*> field_ips_;
			// Loggable field names
			static const int NUMBER_FIXED = 10;
			static const int NUMBER_TOTAL = NUMBER_FIXED + 12;
			const string fixed_names_[NUMBER_FIXED] = {
				"MY_RIG", "MY_ANTENNA", "APP_ZZA_QTH", "STATION_CALLSIGN",
				"QSO_DATE", "TIME_ON", "CALL", "FREQ", 
				"MODE", "TX_PWR" };

			// Widgets
			// Activate - go to QSO_PENDING state
			Fl_Button* bn_activate_;
			// Start - go to QSO_STARTED state
			Fl_Button* bn_start_;
			// Save - log
			Fl_Button* bn_save_;
			// Cancel 
			Fl_Button* bn_cancel_;
			// Edit
			Fl_Button* bn_edit_;
			// Worked before button
			Fl_Button* bn_wkd_b4_;
			// Parse button
			Fl_Button* bn_parse_;
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
			// Notes
			Fl_Input* ip_notes_;
			// Field choices
			field_choice* ch_field_[NUMBER_TOTAL];
			// field inputs
			field_input* ip_field_[NUMBER_TOTAL];

		};

		public:
		class cat_group :
			public Fl_Group
		{
		public:

			cat_group(int X, int Y, int W, int H, const char* l);
			~cat_group();

			// get settings 
			void load_values();
			// Create form
			void create_form(int X, int Y);
			// Enable widgets
			void enable_widgets();
			// Save changes
			void save_values();

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
		public:
			// Callback - Connect button
			static void cb_bn_connect(Fl_Widget* w, void* v);
		protected:
			// Spinner fast polling rate
			static void cb_ctr_pollfast(Fl_Widget* w, void* v);
			// slow polling rate
			static void cb_ctr_pollslow(Fl_Widget* w, void* v);

			//populate port choice
			void populate_port_choice();
			// Populate model choice
			void populate_model_choice();
			//Populate baud rate choice
			void populate_baud_choice();

			Fl_Group* serial_grp_;
			Fl_Group* network_grp_;
			// Hamlib widgets to revalue when rig selected changes
			Fl_Widget* mfr_choice_;
			Fl_Widget* rig_model_choice_;
			Fl_Widget* port_if_choice_;
			Fl_Widget* port_if_input_;
			Fl_Widget* baud_rate_choice_;
			Fl_Widget* rig_choice_;
			Fl_Widget* override_check_;
			Fl_Widget* show_all_ports_;
			Fl_Button* bn_connect_;

			// CAT connection data
			cat_data* cat_data_;
			// Waiting connect
			bool wait_connect_;

			//// The text items
			//Fl_Text_Buffer* buffer_;
			//// The editor
			//spad_editor* editor_;
			Fl_Spinner* ctr_pollfast_;
			Fl_Spinner* ctr_pollslow_;

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
		// Update record with MY_RIG etc. For use when importing 
		void update_import_qso(record* import);

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
		// QSO i n progress
		bool qso_in_progress();
		// Start QSO
		void start_qso();
		// Create a dummy qso - eg for parsing a callsign
		record* dummy_qso();
		// End QSO
		void end_qso();


	protected:

		// Set of bands in frequency order
		list<string> ordered_bands_;
		// Widgets have been xcreated
		bool created_;
		// Typing into choice - prevent it being overwritten
		bool items_changed_;

		// Groups
		qso_group* qso_group_;
		cat_group* cat_group_;
		clock_group* clock_group_;
		// widgets

		// Scratchpad editor font
		Fl_Font font_;
		Fl_Fontsize fontsize_;

		const static int WEDITOR = 400;

	};
}

#endif


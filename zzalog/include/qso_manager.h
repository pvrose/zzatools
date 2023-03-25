#ifndef __STN_DIALOG__
#define __STN_DIALOG__


#include "callback.h"
#include "rig_if.h"
#include "record.h"
#include "intl_widgets.h"
#include "alarm_dial.h"
#include "field_choice.h"
#include "qsl_viewer.h"
#include "record_table.h"
#include "book.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <array>

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
			LM_ON_AIR_CLONE,// Real-time logging - data from selected QSO (except CALL)
			LM_ON_AIR_TIME, // Real-time logging - date/time only
			//LM_IMPORTED,    // Import from modem software (w/ or w/o radio)
		};

		// A bit of a misnomer - but the category of settings
		enum stn_item_t {
			RIG = 1,
			ANTENNA = 2,
			CALLSIGN = 4,
			QTH = 8,
			NONE = 0
		};

	protected:

		// Loggng state
		enum logging_state_t {
			QSO_INACTIVE,    // No QSO being edited
			QSO_PENDING,     // Collecting data for QSO - not qctive
			QSO_STARTED,     // QSO started
			QSO_EDIT,        // Editing existing QSO
			QSO_BROWSE,      // View selected view in table view
			QUERY_MATCH,     // Query: Compare new QSO with nearest match in log - add, reject or selectively merge
			QUERY_NEW,       // Query: Not able to find QSO - allow manual matching
			QUERY_DUPE,      // Query: Two records in log found to be possible duplicates - select either, both or a merge
			QRZ_MERGE,       // Merge details downloaded from QRZ.com
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
				CF_TIME = 4,
				CF_CONTACT = 8,
				CF_REPORTS = 16,
				CF_QSO = CF_RIG_ETC | CF_CAT | CF_TIME | CF_CONTACT,
				CF_ALL_FLAGS = -1
			};

			const map < copy_flags, set<string> > COPY_FIELDS =
			{
				{ CF_RIG_ETC, { "MY_RIG", "MY_ANTENNA", "STATION_CALLSIGN", "APP_ZZA_QTH" } },
				{ CF_CAT, { "MODE", "FREQ", "SUBMODE", "TX_PWR", "BAND"} },
				{ CF_TIME, { "QSO_DATE", "QSO_DATE_OFF", "TIME_ON", "TIME_OFF" } },
				{ CF_CONTACT, { "CALL", "NAME", "QTH", "DXCC", "STATE", "CNTY", "GRIDSQUARE", "CQZ", "ITUZ" } },
				{ CF_REPORTS, { "RST_SENT", "RST_RCVD", "SRX", "STX" }}
			};

			const set < copy_flags > COPY_SET = { CF_RIG_ETC, CF_CAT, CF_TIME, CF_CONTACT, CF_REPORTS };

			enum dupe_flags : int {
				DF_1,
				DF_2,
				DF_MERGE,
				DF_BOTH
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
			// Edit QTH details
			static void cb_bn_edit_qth(Fl_Widget* w, void* v);
			// Navigate buttons
			static void cb_bn_navigate(Fl_Widget*, void* v);
			// QSL Viewer closed
			static void cb_qsl_viewer(Fl_Widget* w, void* v);
			// View QSL button
			static void cb_bn_view_qsl(Fl_Widget* w, void* v);
			// Browse call back
			static void cb_bn_browse(Fl_Widget* w, void* v);
			// Record table
			static void cb_tab_qso(Fl_Widget* w, void* v);
			// Add query
			static void cb_bn_add_query(Fl_Widget* w, void* v);
			// Reject query
			static void cb_bn_reject_query(Fl_Widget* w, void* v);
			// Merge query
			static void cb_bn_merge_query(Fl_Widget* w, void* v);
			// Find QSO
			static void cb_bn_find_match(Fl_Widget* w, void* v);
			// Dupe
			static void cb_bn_dupe(Fl_Widget* w, void* v);
			// Merge yes/no
			static void cb_bn_save_merge(Fl_Widget* w, void* v);
			// Fetch data from QRZ.com
			static void cb_bn_fetch_qrz(Fl_Widget* w, void* v);
			// Parse WSJT-X for call
			static void cb_bn_all_txt(Fl_Widget* w, void* v);

			// Individual group creates
			Fl_Group* create_contest_group(int X, int Y);
			Fl_Group* create_entry_group(int X, int Y);
			Fl_Group* create_query_group(int X, int Y);
			Fl_Group* create_button_group(int X, int Y);
			// Individual enable
			void enable_contest_widgets();
			void enable_entry_widgets();
			void enable_query_widgets();
			void enable_button_widgets();

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
			// Navigate
			void action_navigate(int target);
			// Update QSL viewer
			void action_view_qsl();
			// Browse record
			void action_browse();
			// Action add field to widgets
			void action_add_field(int ix, string field);
			// Action delete field
			void action_del_field(int ix);
			// Action cancel browse
			void action_cancel_browse();
			// Action add query
			void action_add_query();
			// Action reject query
			void action_reject_query();
			// Action merge query
			void action_merge_query();
			// Action find match
			void action_find_match();
			// Action query
			void action_query(logging_state_t query);
			// Action handle dupe
			void action_handle_dupe(dupe_flags action);
			// Action handle d-click
			void action_handle_dclick(int col, string field);
			// Action merge from QRZ.com
			void action_save_merge();
			// Look in all.txt
			void action_look_all_txt();
			// Copy found text
			void action_copy_all_text(string text);
			// Set current QSO from selected record
			void action_set_current();

			// Get default copy record
			record* get_default_record();
			// QTH has changed
			void check_qth_changed();

			// Update QSO
			void update_qso(qso_num_t log_num);
			// Update query
			void update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);

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
			// Presented query 
			record* query_qso_;
			// Current record number
			qso_num_t current_rec_num_;
			// Query record number
			qso_num_t query_rec_num_;
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
			static const int NUMBER_FIXED = 10;
			static const int NUMBER_TOTAL = NUMBER_FIXED + 12;
			const string fixed_names_[NUMBER_FIXED] = {
				"MY_RIG", "MY_ANTENNA", "APP_ZZA_QTH", "STATION_CALLSIGN",
				"QSO_DATE", "TIME_ON", "CALL", "FREQ", 
				"MODE", "TX_PWR" };
			map<string, int> field_ip_map_;
			string field_names_[NUMBER_TOTAL];
			int number_fields_in_use_;

			enum button_type {
				ACTIVATE,
				START_QSO,
				EDIT_QSO,
				CANCEL_EDIT,
				CANCEL_BROWSE,
				CANCEL_QSO,
				QUIT_QSO,
				SAVE_QSO,
				SAVE_EDIT,
				WORKED_B4,
				PARSE,
				EDIT_QTH,
				NAV_FIRST,
				NAV_PREV,
				NAV_NEXT,
				NAV_LAST,
				ADD_QUERY,
				REJECT_QUERY,
				MERGE_QUERY,
				FIND_QSO,
				BROWSE,
				KEEP_DUPE_1,
				KEEP_DUPE_2,
				MERGE_DUPE,
				KEEP_BOTH_DUPES,
				MERGE_DONE,
				VIEW_QSL,
				LOOK_ALL_TXT,
			};

			const map<logging_state_t, list<button_type> > button_map_ =
			{
				{ QSO_INACTIVE, {ACTIVATE, START_QSO, EDIT_QSO, BROWSE } },
				{ QSO_PENDING, { START_QSO, QUIT_QSO, EDIT_QTH } },
				{ QSO_STARTED, { SAVE_QSO, CANCEL_QSO, EDIT_QTH, WORKED_B4, PARSE } },
				{ QSO_EDIT, { SAVE_EDIT, CANCEL_EDIT, NAV_FIRST, NAV_PREV, NAV_NEXT, NAV_LAST, WORKED_B4, VIEW_QSL } },
				{ QSO_BROWSE, { EDIT_QSO, CANCEL_BROWSE, NAV_FIRST, NAV_PREV, NAV_NEXT, NAV_LAST, WORKED_B4, VIEW_QSL } },
				{ QUERY_MATCH, { ADD_QUERY, REJECT_QUERY, MERGE_QUERY, NAV_PREV, NAV_NEXT }},
				{ QUERY_NEW, { ADD_QUERY, REJECT_QUERY, FIND_QSO, LOOK_ALL_TXT }},
				{ QUERY_DUPE, { KEEP_DUPE_1, MERGE_DUPE, KEEP_DUPE_2, KEEP_BOTH_DUPES }},
				{ QRZ_MERGE, { MERGE_DONE }}
				// TODO add the query button maps
			};

			// Size of longest list in the above
			static const int MAX_ACTIONS = 8;

			struct button_action {
				const char* label;
				const char* tooltip;
				Fl_Color colour;
				Fl_Callback* callback;
				void* userdata;
			};

			const Fl_Color COLOUR_ORANGE = 93; /* R=4/4, B=0/4, G=5/7 */	
			const Fl_Color COLOUR_APPLE = 87;  /* R=3/4, B=0/4, G=7/7 */
			const Fl_Color COLOUR_PINK = 170;  /* R=4/4, B=2/4, G=2/7 */
			const Fl_Color COLOUR_NAVY = 136;  /* R=0/4, B=2/4, G=0/7 */

			const map<button_type, button_action> action_map_ = 
			{
				{ ACTIVATE, { "Activate", "Pre-load QSO fields based on logging mode", FL_CYAN, cb_activate, 0 } },
				{ START_QSO, { "Start QSO", "Start the QSO, after saving, and/or activating", FL_YELLOW, cb_start, 0 } },
				{ EDIT_QSO, { "Edit QSO", "Edit the selected QSO", FL_MAGENTA, cb_edit, 0 } },
				{ BROWSE, { "Browse Log", "Browse the log without editing", FL_BLUE, cb_bn_browse, 0}},
				{ QUIT_QSO, { "Quit", "Quit entry mode", COLOUR_PINK, cb_cancel, 0 } },
				{ EDIT_QTH, { "Edit QTH", "Edit the details of the QTH macro", FL_BACKGROUND_COLOR, cb_bn_edit_qth, 0 } },
				{ SAVE_QSO, { "Save QSO", "Log the QSO, activate a new one", FL_GREEN, cb_save, 0 } },
				{ CANCEL_QSO, { "Quit QSO", "Cancel the current QSO entry", COLOUR_PINK, cb_cancel, 0 } },
				{ WORKED_B4, { "B4?", "Display all previous QSOs with this callsign", FL_BACKGROUND_COLOR, cb_wkb4, 0 } },
				{ SAVE_EDIT, { "Save", "Copy changed record back to book", FL_GREEN, cb_save, 0}},
				{ CANCEL_EDIT, { "Cancel Edit", "Cancel the current QSO edit", COLOUR_PINK, cb_cancel, 0 } },
				{ NAV_FIRST, { "@$->|", "Select first record in book", FL_YELLOW, cb_bn_navigate, (void*)NV_FIRST } },
				{ NAV_PREV, { "@<-", "Select previous record in book", FL_YELLOW, cb_bn_navigate, (void*)NV_PREV } },
				{ NAV_NEXT, { "@->", "Select next record in book", FL_YELLOW, cb_bn_navigate, (void*)NV_NEXT } },
				{ NAV_LAST, { "@->|", "Select last record in book", FL_YELLOW, cb_bn_navigate, (void*)NV_LAST } },
				{ VIEW_QSL, { "View QSL", "Display QSL status", FL_BACKGROUND_COLOR, cb_bn_view_qsl, 0 } },
				{ PARSE, { "DX?", "Display the DX details for this callsign", FL_BACKGROUND_COLOR, cb_parse, 0 } },
				{ CANCEL_BROWSE, { "Quit Browse", "Quit browse mode", COLOUR_PINK, cb_cancel, 0 } },
				{ ADD_QUERY, { "Add QSO", "Add queried QSO to log", FL_GREEN, cb_bn_add_query, 0 }},
				{ REJECT_QUERY, {"Reject QSO", "Do not add queried QSO to log", COLOUR_PINK, cb_bn_reject_query, 0} },
				{ MERGE_QUERY, {"Merge QSO", "Merge query with logged QSO", COLOUR_ORANGE, cb_bn_merge_query, 0 } },
				{ FIND_QSO, { "@search", "Display possible match", FL_YELLOW, cb_bn_find_match, 0}},
				{ KEEP_DUPE_1, { "Keep 1", "Keep first QSO and delete second", COLOUR_APPLE, cb_bn_dupe, (void*)DF_1}},
				{ KEEP_DUPE_2, { "Keep 2", "Keep second QSO and delete first", COLOUR_APPLE, cb_bn_dupe, (void*)DF_2}},
				{ MERGE_DUPE, { "Merge", "Merge the two records", COLOUR_ORANGE, cb_bn_dupe, (void*)DF_MERGE}},
				{ KEEP_BOTH_DUPES, { "Keep 1 && 2", "Keep both records", FL_GREEN, cb_bn_dupe, (void*)DF_BOTH}},
				{ MERGE_DONE, { "Done", "Save changes", COLOUR_APPLE, cb_bn_save_merge, 0}},
				{ LOOK_ALL_TXT, { "all.txt?", "Look in WSJT-X all.txt file for possible contact", COLOUR_NAVY, cb_bn_all_txt, 0 } },
			};

			// Previous value
			string previous_qth_;
			string previous_locator_;
			// Query message
			string query_message_;

			// Widgets
			// Contest group
			Fl_Group* g_contest_;
			// Entry group
			Fl_Group* g_entry_;
			// Query group
			Fl_Group* g_query_;
			// Button group
			Fl_Group* g_buttons_;
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
			// QSL Viewer window
			qsl_viewer* qsl_viewer_;
			// Record table
			record_table* tab_query_;
			// Action buttos
			Fl_Button* bn_action_[MAX_ACTIONS];

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
			// Callback - click on group
			static void cb_click(Fl_Widget* w, void* v);

			// Display local time rather than UTC
			bool display_local_;

			Fl_Group* g_clock_;
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
		// Query/update
		void update(hint_t hint, qso_num_t log_num, qso_num_t query_num);

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
		// Present query (uses view update mechanism)
		void update_qso(hint_t hint, qso_num_t match_num, qso_num_t query_num);
		// QSO i n progress
		bool qso_in_progress();
		// Start QSO
		void start_qso();
		// Create a dummy qso - eg for parsing a callsign
		record* dummy_qso();
		// End QSO
		void end_qso();
		// Edit QSO
		void edit_qso();
		// Get default value
		string get_default(stn_item_t item);


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

		const static int WEDITOR = 400;

	};

#endif


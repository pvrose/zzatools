#pragma once

#include "qso_contest.h"
#include "qso_entry.h"
#include "qsl_viewer.h"
#include "field_choice.h"
#include "record_table.h"
#include "book.h"

#include <string>
#include <set>
#include <list>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Output.H>

using namespace std;

class qso_data :
    public Fl_Group
{
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


	enum dupe_flags : int {
		DF_1,
		DF_2,
		DF_MERGE,
		DF_BOTH
	};

public:
	qso_data(int X, int Y, int W, int H, const char* l);
	~qso_data();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

	// Create a dummy QSO
	record* dummy_qso();
	// Start a new QSO
	void start_qso();
	// End QSO
	void end_qso();
	// Edit QSO
	void edit_qso();

	// Get logging mode
	qso_data::logging_mode_t logging_mode();
	// Set logging mode
	void logging_mode(qso_data::logging_mode_t mode);
	// Get logging state
	qso_data::logging_state_t logging_state();
	// Update QSO
	void update_qso(qso_num_t log_num);
	// Update query
	void update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	// Get default copy record
	record* get_default_record();
	// Initialise fields
	void initialise_fields();
	// Get defined fields
	string get_defined_fields();
	// Current QSO
	record* current_qso();
	// Curernt QSO number
	qso_num_t current_qso_num();
	// Update view and 
	void update_rig();
	// Update clock
	void update_time(time_t when);

	// Callbacks
public:
	// Log QSO (start first if in QSO_PENDING)
	static void cb_save(Fl_Widget* w, void* v);
	// Quit QSO logging - delete QSO; clear fields
	static void cb_cancel(Fl_Widget* w, void* v);
protected:
	// Activate QSO logging 
	static void cb_activate(Fl_Widget* w, void* v);
	// Start QSO (log first if already in QSO_STARTED, Activate in QSO_INACTIVE))
	static void cb_start(Fl_Widget* w, void* v);
	// Go edit mode
	static void cb_edit(Fl_Widget* w, void* v);
	// Callback - Worked B4? button
	static void cb_wkb4(Fl_Widget* w, void* v);
	// Callback - Parse callsign
	static void cb_parse(Fl_Widget* w, void* v);
	// Logging mode
	static void cb_logging_mode(Fl_Widget* w, void* v);
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
	Fl_Group* create_query_group(int X, int Y);
	Fl_Group* create_button_group(int X, int Y);
	// Individual enable
	void enable_query_widgets();
	void enable_button_widgets();

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

	// Query message
	string query_message_;

	// Widgets
	// Contest group
	qso_contest* g_contest_;
	// Entry group
	qso_entry* g_entry_;
	// Query group
	Fl_Group* g_query_;
	// Button group
	Fl_Group* g_buttons_;
	// Group for freq/power/mode
	Fl_Group* grp_fpm_;
	// Logging mode
	Fl_Choice* ch_logmode_;
	// QSL Viewer window
	qsl_viewer* qsl_viewer_;
	// Record table
	record_table* tab_query_;
	// Action buttos
	Fl_Button* bn_action_[MAX_ACTIONS];


};


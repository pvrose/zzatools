#pragma once

#include "qso_contest.h"
#include "qso_entry.h"
#include "qso_query.h"
#include "qso_buttons.h"
#include "qso_net_entry.h"
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

	// Loggng state
	enum logging_state_t {
		QSO_INACTIVE,    // No QSO being edited
		QSO_PENDING,     // Collecting data for QSO - not qctive
		QSO_STARTED,     // QSO started
		QSO_EDIT,        // Editing existing QSO
		QSO_BROWSE,      // View selected view in table view
		QSO_PEEK,        // View alternate QSO in entry view
		QSO_PEEK_ED,     // View alternate QSO in entry view and allow it to be edited
		QUERY_MATCH,     // Query: Compare new QSO with nearest match in log - add, reject or selectively merge
		QUERY_NEW,       // Query: Not able to find QSO - allow manual matching
		QUERY_WSJTX,     // Query found in WSJT-X ALL.TXT file
		QUERY_DUPE,      // Query: Two records in log found to be possible duplicates - select either, both or a merge
		QRZ_MERGE,       // Merge details downloaded from QRZ.com
		NET_STARTED,     // Started several QSOs in a net.
		NET_EDIT,        // Converting sngle QSO into a net by adding callsigns
		NET_ADDING,      // Adding a new QSO to the started net.
		SWITCHING,       // Switching state - used to ignore stuff
		QSO_MODEM,       // Recording QSOs from a modem
		MANUAL_ENTRY,    // Allow the definition of a record to query
	};

	enum dupe_flags : int {
		DF_1,
		DF_2,
		DF_MERGE,
		DF_BOTH
	};

	// Source of editing
	enum qso_init_t {
		QSO_ON_AIR,         // Start a QSO using current time and CAT if connected
		QSO_NONE,           // Start a QSO with no initial values
		QSO_COPY_CALL,      // Start a QSO copying callsign, station details and CAT conditions
		QSO_COPY_CONDX,     // Start a QSO copying station details and CAT conditions
		QSO_COPY_FOR_NET,   // Start a QSO copyin station details, CAT conditions and start time
		QSO_AS_WAS,         // Used with action_activate() to maintain the existing one
		QSO_COPY_MODEM,     // Copy QSO from modem
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
	void start_qso(qso_init_t mode);
	// End QSO
	void end_qso();
	// Edit QSO
	void edit_qso();

	// Get logging state
	qso_data::logging_state_t logging_state();
	// Get contest mode
	qso_contest::contest_mode_t contest_mode();
	// Update QSO
	void update_qso(qso_num_t log_num);
	// Update query
	void update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	// Update modem QSO
	void update_modem_qso(record* qso);
	// Get default copy record
	qso_num_t get_default_number();
	// Initialise fields
	void initialise_fields(qso_entry* entry);
	// Get defined fields
	string get_defined_fields();
	// Current QSO
	record* current_qso();
	// Current QSO number
	qso_num_t current_number();
	//// Query QSO
	//record* query_qso();
	//// Original QSO
	//record* original_qso();
	// Update view and 
	void update_rig();
	// Gt call
	string get_call();
	// Ticker clock
	void ticker();
	// The QSO is being edited
	bool qso_editing(qso_num_t number);


	// State transition actions:-
	// Create a new record per loging mode
	void action_activate(qso_init_t mode);
	// Clear all QSO infp
	void action_deactivate();
	// Add new record to book, add time on
	void action_start(qso_init_t mode);
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
	void action_view_qsl(bool force = false);
	// Browse record
	void action_browse();
	// Action cancel browse
	void action_cancel_browse();
	// Action add query
	void action_add_query();
	// Action reject query
	void action_reject_query();
	// Action reject manual query
	void action_reject_manual();
	// Action merge query
	void action_merge_query();
	// Action find match
	void action_find_match();
	// Action query
	void action_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	// Action handle dupe
	void action_handle_dupe(dupe_flags action);
	// Action merge from QRZ.com
	void action_save_merge();
	// Look in all.txt
	void action_look_all_txt();
	// // Copy found text
	// void action_copy_all_text(string text);
	// Create a net from current QSO
	void action_create_net();
	// Add a QSO to the net - copy existing qso start times or not
	void action_add_net_qso();
	// Save the whole net
	void action_save_net_all();
	// Save a net edit
	void action_save_net_edit();
	// Cancel the whole net
	void action_cancel_net_all();
	// Cancel an individual net edit
	void action_cancel_net_edit();
	// Create a new QSO 
	void action_new_qso(record* qso, qso_init_t mode);
	// Delete the selected QSO
	void action_delete_qso();
	// Use the QSO received from the modem
	void action_add_modem(record* qso);
	// Update modem
	void action_update_modem(record* qso);
	// Cancel modem
	void action_cancel_modem();
	// Start peeking behaviour
	void action_peek(qso_num_t number);
	// Cancel peeking behaviour
	void action_cancel_peek();
	// Edit peeked QSO
	void action_edit_peek();
	// Create a query entry
	void action_query_entry();
	// Execute the query
	void action_exec_query();
	// Cancel the query
	void action_cancel_query();
	// Import the query
	void action_import_query();

	// Edit - saved logging state
	logging_state_t edit_return_state_;

protected:
	// Callbacks
	// QSL Viewer closed
	static void cb_qsl_viewer(Fl_Widget* w, void* v);

	// Logging state
	logging_state_t logging_state_;
	//// Current record - being entered or edited
	//record* current_qso_;
	// Selected record
	record* selected_qso_;
	//// Saved current record
	//record* original_qso_;
	//// Presented query 
	//record* query_qso_;
	//// Current record number
	//qso_num_t current_rec_num_;
	//// Query record number
	//qso_num_t query_rec_num_;
	// Disable drawing update
	bool inhibit_drawing_;
	// Current starting mode
	qso_init_t previous_mode_;
	// Potential match
	qso_num_t potential_match_;
	qso_num_t query_number_;

	// Peek - save logging state
	logging_state_t interrupted_state_;
	// QSO being peeked
	record* peeked_qso_;

	// Widgets
	// Contest group
	qso_contest* g_contest_;
	// Entry group
	qso_entry* g_entry_;
	// Query group
	qso_query* g_query_;
	// Net Entry group
	qso_net_entry* g_net_entry_;
	// Peek entry group
	qso_entry* g_peek_;
	// Query entry group
	qso_entry* g_qy_entry_;
	// Button group
	qso_buttons* g_buttons_;
	// Group for freq/power/mode
	Fl_Group* grp_fpm_;
	// QSL Viewer window
	qsl_viewer* qsl_viewer_;


};


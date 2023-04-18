#pragma once

#include "qso_contest.h"
#include "qso_entry.h"
#include "qso_query.h"
#include "qso_buttons.h"
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
		NET_STARTED,     // Started several QSOs in a net.
		NET_EDIT,        // Converting sngle QSO into a net by adding callsigns
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
	// Get contest mode
	qso_contest::contest_mode_t contest_mode();
	// Update QSO
	void update_qso(qso_num_t log_num);
	// Update query
	void update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	// Get default copy record
	qso_num_t get_default_number();
	// Initialise fields
	void initialise_fields();
	// Get defined fields
	string get_defined_fields();
	// Current QSO
	record* current_qso();
	//// Query QSO
	//record* query_qso();
	//// Original QSO
	//record* original_qso();
	// Curernt QSO number
	qso_num_t current_qso_num();
	// Update view and 
	void update_rig();
	// Gt call
	string get_call();
	// QTH has changed
	void check_qth_changed();
	// Ticker clock
	void ticker();


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
	void action_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	// Action handle dupe
	void action_handle_dupe(dupe_flags action);
	// Action merge from QRZ.com
	void action_save_merge();
	// Look in all.txt
	void action_look_all_txt();
	// Copy found text
	void action_copy_all_text(string text);

protected:
	// Callbacks
	// Logging mode
	static void cb_logging_mode(Fl_Widget* w, void* v);
	// QSL Viewer closed
	static void cb_qsl_viewer(Fl_Widget* w, void* v);

	// Logging mode
	logging_mode_t logging_mode_;
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
	// Previous value
	string previous_qth_;
	string previous_locator_;


	// Widgets
	// Contest group
	qso_contest* g_contest_;
	// Entry group
	qso_entry* g_entry_;
	// Query group
	qso_query* g_query_;
	// Button group
	qso_buttons* g_buttons_;
	// Group for freq/power/mode
	Fl_Group* grp_fpm_;
	// Logging mode
	Fl_Choice* ch_logmode_;
	// QSL Viewer window
	qsl_viewer* qsl_viewer_;


};


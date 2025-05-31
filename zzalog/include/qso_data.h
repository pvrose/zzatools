#pragma once

#include "fields.h"

#include <string>
#include <set>
#include <list>
#include <map>

#include <FL/Fl_Group.H>

using namespace std;

class qso_entry;
class qso_query;
class qso_buttons;
class qso_misc;
class qso_net_entry;
class qso_contest;
class record;
typedef size_t qso_num_t;
enum navigate_t : uchar;


// Default field sets
static const field_list QSO_FIELDS = {
	"QSO_DATE",
	"TIME_ON",
	"TIME_OFF",
	"CALL",
	"FREQ",
	"MODE",
	"TX_PWR",
	"RST_RCVD",
	"RST_SENT",
};
// Default fixed fields
static const field_list STN_FIELDS = {
	"MY_RIG",
	"MY_ANTENNA",
	"APP_ZZA_QTH",
	"STATION_CALLSIGN"
};


// This data contains the various QSO viewer and editor classes
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
		QSO_ENTER,       // Entering a new QSO
		QSO_BROWSE,      // View selected view in table view
		QSO_VIEW,        // View selected QSO in entry view
		QUERY_MATCH,     // Query: Compare new QSO with nearest match in log - add, reject or selectively merge
		QUERY_NEW,       // Query: Not able to find QSO - allow manual matching
		QUERY_WSJTX,     // Query found in WSJT-X ALL.TXT file
		QUERY_DUPE,      // Query: Two records in log found to be possible duplicates - select either, both or a merge
		QUERY_SWL,       // Query: SWL report
		QRZ_MERGE,       // Merge details downloaded from QRZ.com
		NET_STARTED,     // Started several QSOs in a net.
		NET_EDIT,        // Converting sngle QSO into a net by adding callsigns
		NET_ADDING,      // Adding a new QSO to the started net.
		SWITCHING,       // Switching state - used to ignore stuff
		QSO_WSJTX,       // Recording QSOs from WSJT-X - allows editing of recveived qso
		QSO_FLDIGI,      // Recording QSos from FlDIGI
		MANUAL_ENTRY,    // Allow the definition of a record to query
		TEST_PENDING,    // A contest QSO is pending
		TEST_ACTIVE,     // A contest QSO is being logged
	};

	// Used in dupe checking to control how dupe QSOs are handled
	enum dupe_flags : int {
		DF_1,			// Select the first QSO
		DF_2,           // Select the second QSO
		DF_MERGE,		// Merge the two QSOs
		DF_BOTH			// Use both (ie they are not dupes)
	};

	// Source of editing
	enum qso_init_t : uchar {
		QSO_ON_AIR,         // Start a QSO using current time and CAT if connected
		QSO_NONE,           // Start a QSO with no initial values
		QSO_COPY_CALL,      // Start a QSO copying callsign, station details and CAT conditions
		QSO_COPY_CONDX,     // Start a QSO copying station details and CAT conditions
		QSO_COPY_FOR_NET,   // Start a QSO copyin station details, CAT conditions and start time
		QSO_AS_WAS,         // Used with action_activate() to maintain the existing one
		QSO_COPY_WSJTX,     // Copy QSO to modem
		QSO_COPY_FLDIGI,    // Copy QSO from Fldigi
	};

public:
	qso_data(int X, int Y, int W, int H, const char* l);
	~qso_data();

	// Custom event handling
	virtual int handle(int event);

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
	// Update QSO
	void update_qso(qso_num_t log_num);
	// Update query
	void update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	// start a ,mode QSO
	record* start_modem_qso(string call, qso_init_t source);
	// Update modem QSO
	void update_modem_qso(bool log_it);
	//. Add a modemm QSO
	void enter_modem_qso(record*);
	// Get default copy record
	qso_num_t get_default_number();
	// // Initialise fields
	// void initialise_fields(qso_entry* entry);
	// Get defined fields
	string get_defined_fields();
	// Current QSO
	record* current_qso();
	// Current QSO number
	qso_num_t current_number();
	// Query QSO
	record* query_qso();
	// Update view and 
	void update_rig();
	// Gt call
	string get_call();
	// The QSO is being edited
	bool qso_editing(qso_num_t number);
	// Not actively editing
	bool inactive();


	// State transition actions:-
	// Create a new record per loging mode
	void action_activate(qso_init_t mode);
	// Clear all QSO infp
	void action_deactivate();
	// Add new record to book, add time on
	void action_start(qso_init_t mode);
	// Stop QSO - 
	bool action_save(bool continuing);
	// Cancel QSO - remove new record from book
	void action_cancel();
	// Copy selected QSO and allow it to be edited
	void action_edit();
	// View selected QSO in entry view (disable editing)
	void action_view(qso_num_t number = -1);
	// Copy edited QSO back to selected QSO
	void action_save_edit();
	// Cancel editing
	void action_cancel_edit();
	// Navigate
	void action_navigate(int target);
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
	void action_log_modem();
	// Create a query entry
	void action_query_entry();
	// Execute the query
	void action_exec_query();
	// Cancel the query
	void action_cancel_query();
	// Cancel the modem
	void action_cancel_modem();
	// Import the query
	void action_import_query();
	// Opne QRZ.com browser
	void action_qrz_com();
	// Update selected QSO with current CAT conditions
	void action_update_cat(bool clear);
	// Remember state before edit or view
	void action_remember_state();
	// Go to remembered state
	void action_return_state();
	// Add parse details to QSO
	void action_parse_qso();
	// Expand macro
	void action_expand_macro(string macro);

	qso_contest* contest();
	// Contest mode is active
	bool in_contest();
	// Can we navigate the net
	bool can_navigate(navigate_t target);

	// Edit - saved logging state
	logging_state_t edit_return_state_;

protected:
	// Callbacks

	// Logging state
	logging_state_t logging_state_;
	// Selected record
	record* selected_qso_;
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
	// Entry group
	qso_entry* g_entry_;
	// Query group
	qso_query* g_query_;
	// Net Entry group
	qso_net_entry* g_net_entry_;
	// Query entry group
	qso_entry* g_qy_entry_;
	// Misc info group
	qso_misc* g_misc_;
	// Button group
	qso_buttons* g_buttons_;
	// Group for freq/power/mode
	Fl_Group* grp_fpm_;


};


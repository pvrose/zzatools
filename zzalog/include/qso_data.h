#pragma once

#include "fields.h"

#include <string>
#include <set>
#include <list>
#include <map>

#include <FL/Fl_Group.H>



class qso_buttons;
class contest_scorer;
class qso_entry;
class qso_misc;
class qso_net_entry;
class qso_operation;
class qso_query;
class record;
typedef size_t qso_num_t;
enum navigate_t : uchar;


//! Default field std::set for qso_entry.
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
//! Default fixed fields
static const field_list STN_FIELDS = {
	"MY_RIG",
	"MY_ANTENNA",
};


//! This class contains the various QSO viewer and editor views.
class qso_data :
    public Fl_Group
{
public:

	//! QSO view modes
	enum logging_state_t {
		QSO_INACTIVE,    //!< No QSO being edited
		QSO_PENDING,     //!< Collecting data for QSO - not qctive
		QSO_STARTED,     //!< QSO started
		QSO_EDIT,        //!< Editing existing QSO
		QSO_ENTER,       //!< Entering a new QSO
		QSO_BROWSE,      //!< View selected view in table view
		QSO_VIEW,        //!< View selected QSO in entry view
		QUERY_MATCH,     //!< Query: Compare new QSO with nearest match in log - add, reject or selectively merge
		QUERY_NEW,       //!< Query: Not able to find QSO - allow manual matching
		QUERY_WSJTX,     //!< Query found in WSJT-X ALL.TXT file
		QUERY_DUPE,      //!< Query: Two records in log found to be possible duplicates - select either, both or a merge
		QUERY_SWL,       //!< Query: SWL report
		QRZ_MERGE,       //!< Merge details downloaded from QRZ.com (existing record)
		QRZ_COPY,        //!< Copy details downloaded from QRZ.com (new record)
		NET_STARTED,     //!< Started several QSOs in a net.
		NET_EDIT,        //!< Converting sngle QSO into a net by adding callsigns
		NET_ADDING,      //!< Adding a new QSO to the started net.
		SWITCHING,       //!< Switching state - used to ignore stuff
		QSO_WSJTX,       //!< Recording QSOs from WSJT-X - allows editing of recveived qso
		QSO_FLDIGI,      //!< Recording QSos from FlDIGI
		MANUAL_ENTRY,    //!< Allow the definition of a record to query
		TEST_PENDING,    //!< A contest QSO is pending
		TEST_ACTIVE,     //!< A contest QSO is being logged
	};

	//! Used in dupe checking to control how dupe QSOs are handled
	enum dupe_flags : int {
		DF_1,			//!< Select the first QSO
		DF_2,           //!< Select the second QSO
		DF_MERGE,		//!< Merge the two QSOs
		DF_BOTH			//!< Use both (ie they are not dupes)
	};

	//! Source of editing
	enum qso_init_t : uchar {
		QSO_ON_AIR,         //!< Start a QSO using current time and CAT if connected
		QSO_NONE,           //!< Start a QSO with no initial values
		QSO_COPY_CALL,      //!< Start a QSO copying callsign, station details and CAT conditions
		QSO_COPY_CONDX,     //!< Start a QSO copying station details and CAT conditions
		QSO_COPY_FOR_NET,   //!< Start a QSO copyin station details, CAT conditions and start time
		QSO_AS_WAS,         //!< Used with action_activate() to maintain the existing one
		QSO_COPY_WSJTX,     //!< Copy QSO to modem
		QSO_COPY_FLDIGI,    //!< Copy QSO from Fldigi
	};

public:
	//! Constructor

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_data(int X, int Y, int W, int H, const char* L);
	//! Destructor.
	~qso_data();

	//! Override Fl_Group::handle():

	//! Accept focus for keyboard F1 to open userguide.
	//! Handle keyboard navigation keys.
	virtual int handle(int event);

	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets after data change.
	void enable_widgets();

	//! Create a dummy QSO
	record* dummy_qso();
	//! Start a new QSO
	void start_qso(qso_init_t mode);
	//! End QSO
	void end_qso();
	//! Edit QSO
	void edit_qso();

	//! Get logging state
	qso_data::logging_state_t logging_state();
	//! Update QSO
	void update_qso(qso_num_t log_num);
	//! Update query
	
	//! \param query Target logging_state_t to handle query.
	//! \param match_num Index in full log of possible matching SO record
	//! \param query_num Index of QSO being queried.
	void update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	//! start a modem QSO feom \p source modem with \p call, returns the QSO record.
	record* start_modem_qso(std::string call, qso_init_t source);
	//! Update modem QSO and if \p log_it is true save the record.
	void update_modem_qso(bool log_it);
	//! Add a modemm QSO
	void enter_modem_qso(record*);
	//! Returns index of default copy record
	qso_num_t get_default_number();
	//! Returns defined fields
	std::string get_defined_fields();
	//! Returns current QSO
	record* current_qso();
	//! Returns xcrrent QSO number
	qso_num_t current_number();
	//! Returns query QSO
	record* query_qso();
	//! Update QSO with details from rig.
	void update_rig();
	//! Returns callsign
	std::string get_call();
	//! Returns true if the qso with index \p number is being edited.
	bool qso_editing(qso_num_t number);
	//! Returns true if not actively editing
	bool inactive();
	//! Returns default station information
	std::string get_default_station(char t);
	//! Update record from station information
	void update_station_fields(record* qso = nullptr);
	//! Update station choices
	void update_station_choices();
	//! Update fields in qso_entry and qso_query except if it originated the change (\p src).
	void update_fields(Fl_Widget* src);


	// State transition actions:-
	//! Create a new record per logging \p mode
	void action_activate(qso_init_t mode);
	//! Clear all QSO info
	void action_deactivate();
	//! Add new record to book, add TIME_ON.
	void action_start(qso_init_t mode);
	//! Save the QSO and if \p continuing is true continue editing.
	bool action_save(bool continuing);
	//! Cancel QSO - remove new record from book
	void action_cancel();
	//! Copy selected QSO and allow it to be edited
	void action_edit();
	//! View selected QSO with index \p number in entry view (disable editing)
	void action_view(qso_num_t number = -1);
	//! Copy edited QSO back to selected QSO
	void action_save_edit();
	//! Cancel editing
	void action_cancel_edit();
	//! Navigate to \p target.
	void action_navigate(int target);
	//! Browse record
	void action_browse();
	//! Action cancel browse
	void action_cancel_browse();
	//! Action add query
	void action_add_query();
	//! Action reject query
	void action_reject_query();
	//! Action reject manual query
	void action_reject_manual();
	//! Action merge query
	void action_merge_query();
	//! Action find match
	void action_find_match();
	//! Action query

	//! \param query Target logging_state_t to handle query.
	//! \param match_num Index in full log of possible matching SO record
	//! \param query_num Index of QSO being queried.
	void action_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num);
	//! Action handle dupe function \p action.
	void action_handle_dupe(dupe_flags action);
	//! Action merge from QRZ.com
	void action_save_merge();
	//! Look in all.txt
	void action_look_all_txt();
	//! Create a net from current QSO
	void action_create_net();
	//! Add a QSO to the net - copy existing qso start times or not
	void action_add_net_qso();
	//! Save the whole net
	void action_save_net_all();
	//! Save a net edit
	void action_save_net_edit();
	//! Cancel the whole net
	void action_cancel_net_all();
	//! Cancel an individual net edit
	void action_cancel_net_edit();
	//! Start a new QSO.
	void action_new_qso(record* qso, qso_init_t mode);
	//! Delete the selected QSO
	void action_delete_qso();
	//! Use the QSO received from the modem
	void action_log_modem();
	//! Create a query entry
	void action_query_entry();
	//! Execute the query
	void action_exec_query();
	//! Cancel the query
	void action_cancel_query();
	//! Cancel the modem
	void action_cancel_modem();
	//! Import the query
	void action_import_query();
	//! Opne QRZ.com browser
	void action_qrz_com();
	//! Update selected QSO with current CAT conditions.

	//! \param clear Overwrite existing data.
	void action_update_cat(bool clear);
	//! Remember state before edit or view
	void action_remember_state();
	//! Go to remembered state
	void action_return_state();
	//! Add parse details to QSO
	void action_parse_qso();

	//! Returns contest_scorer pane.
	contest_scorer* contest();
	//! Returns true if contest mode is active
	bool in_contest();
	//! Returns true if we can navigate the net
	bool can_navigate(navigate_t target);

	//! Edit - saved logging state
	logging_state_t edit_return_state_;

protected:
	//! Logging state
	logging_state_t logging_state_;
	//! Selected record
	record* selected_qso_;
	//! Flag to inhibit drawing update
	bool inhibit_drawing_;
	//! Current starting mode
	qso_init_t previous_mode_;
	//! Index number of potential match
	qso_num_t potential_match_;
	//! INdex number of QSO being queried.
	qso_num_t query_number_;

	//! Peek/QRZ merge/QRZ copy - save logging state
	logging_state_t interrupted_state_;
	//! QSO being peeked
	record* peeked_qso_;

	// Widgets
	//! Station operations group
	qso_operation* g_station_;
	//! Entry group
	qso_entry* g_entry_;
	//! Query group
	qso_query* g_query_;
	//! Net Entry group
	qso_net_entry* g_net_entry_;
	//! Query entry group
	qso_entry* g_qy_entry_;
	//! Misc info group
	qso_misc* g_misc_;
	//! Button group
	qso_buttons* g_buttons_;
	//! Group for freq/power/mode
	Fl_Group* grp_fpm_;


};


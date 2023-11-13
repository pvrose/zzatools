#pragma once
#include <map>
#include <list>

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

using namespace std;

class qso_data;

class qso_buttons :
    public Fl_Group
{
public:
	qso_buttons(int X, int Y, int W, int H, const char* l = nullptr);
	~qso_buttons();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

	enum button_type {
		ACTIVATE,
		START_QSO,
		ADD_QSO,
		EDIT_QSO,
		COPY_QSO,
		CLONE_QSO,
		VIEW_QSO,
		CANCEL_EDIT,
		CANCEL_BROWSE,
		CANCEL_QSO,
		DELETE_QSO,
		QUIT_QSO,
		SAVE_QSO,
		SAVE_EDIT,
		SAVE_EXIT,
		WORKED_B4,
		PARSE,
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
		LOOK_ALL_TXT,
		START_NET,
		EDIT_NET,
		SAVE_NET,
		CANCEL_NET,
		ADD_NET_QSO,
		SAVE_EDIT_NET,
		CANCEL_PEEK,
		EDIT_PEEK,
		ENTER_QUERY,
		EXEC_QUERY,
		CANCEL_QUERY,
		IMPORT_QUERY,
		CANCEL_VIEW,
		QRZ_COM,
	};


	struct button_action {
		const char* label;
		const char* tooltip;
		Fl_Color colour;
		Fl_Callback* callback;
		void* userdata;
	};

	// Log QSO (start first if in QSO_PENDING)
	static void cb_save(Fl_Widget* w, void* v);
	// Quit QSO logging - delete QSO; clear fields
	static void cb_cancel(Fl_Widget* w, void* v);
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
	//// Edit QTH details
	//static void cb_bn_edit_qth(Fl_Widget* w, void* v);
	// Navigate buttons
	static void cb_bn_navigate(Fl_Widget*, void* v);
	// Browse call back
	static void cb_bn_browse(Fl_Widget* w, void* v);
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
	// Save all qsos
	static void cb_bn_save_all(Fl_Widget* w, void* v);
	// Cancel all QSOs
	static void cb_bn_cancel_all(Fl_Widget* w, void* v);
	// Edit net
	static void cb_bn_add_net(Fl_Widget* w, void* v);
	// Start a net
	static void cb_bn_start_net(Fl_Widget* w, void* v);
	// Delete the selected QSO
	static void cb_bn_delete_qso(Fl_Widget* w, void* v);
	// Cancel peek operation
	static void cb_bn_cancel_peek(Fl_Widget* w, void* v);
	// Edit peeked QSO
	static void cb_bn_edit_peek(Fl_Widget* w, void* v);
	// Enter query data
	static void cb_bn_query_entry(Fl_Widget* w, void* v);
	// Execute query
	static void cb_bn_execute_query(Fl_Widget* w, void* v);
	// Cancel query
	static void cb_bn_cancel_query(Fl_Widget* w, void* v);
	// Import query
	static void cb_bn_import_query(Fl_Widget* w, void* v);
	// View QSO
	static void cb_bn_view_qso(Fl_Widget* w, void* v);
	// Open QRZ.com
	static void cb_bn_qrz_com(Fl_Widget* w, void* v);


protected:

	// Disanle all buttons
	void disable_widgets();

	// Maps that require both qso_data a
	//static map<qso_data::logging_state_t, list<qso_buttons::button_type> > button_map_;
	//static map<button_type, button_action> action_map_;
	// Number of buttons
	static const int MAX_ACTIONS = 12;

	// parent
	qso_data* qso_data_;
	// Action buttos
	Fl_Button* bn_action_[MAX_ACTIONS];

};


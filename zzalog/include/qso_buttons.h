#pragma once
#include <map>
#include <list>

#include <FL/Fl_Group.H>

using namespace std;

class qso_data;
class Fl_Button;

//! This class provides the user controls within the qsl_manager dashboard.

//! It defines all possible buttons, but only a selection are used in any
//! one logging state
class qso_buttons :
    public Fl_Group
{
public:
	//! constructor

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_buttons(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
	~qso_buttons();

	//! Override of Fl_Group::handle takes focus to le keyboard F1 open userguide.
	virtual int handle(int event);

	//! Instatntiate component widgets.
	void create_form(int X, int Y);
	//! Configure widgets after data changes.
	void enable_widgets();

	//! All the possible buttons
	enum button_type {
		ACTIVATE,           //!< Set qso_manager ready to log QSOs.
		START_QSO,          //!< Start logging a real-time QSO.
		ADD_QSO,            //!< Start logging a completed QSO.
		EDIT_QSO,           //!< Edit an existing QSO.
		COPY_QSO,           //!< Add a QSO copying all fieds from a previous QSO.
		CLONE_QSO,          //!< Add a QSO copying fields other than call from a previous QSO.
		VIEW_QSO,           //!< View an existing QSO using qso_entry.
		CANCEL_EDIT,        //!< Cancel editing a QSO, restoring its original state.
		CANCEL_BROWSE,      //!< Cancel brwosing (looking in qso_browse).
		CANCEL_QSO,         //!< Cancel a real-time QSO.
		DELETE_QSO,         //!< Delete a previous QSO record.
		QUIT_QSO,           //!< Quit entry mode
		SAVE_QSO,           //!< Save the current QSO.
		SAVE_EDIT,          //!< Save the QSO being edited and carry on editing.
		SAVE_EXIT,          //!< Save the QSO being edited and leave edit mode.
		SAVE_VIEW,          //!< Save the QSO being edited and remain in viewing mode.
		SAVE_NEW,           //!< Save the current QSO and start a new one.
		SAVE_CONTINUE,      //!< Save the current QSO setting TIME_OFF and keep editing it. 
		NAV_FIRST,          //!< Navigate to first QSO in log (or net in net entry mode). 
		NAV_PREV,           //!< Navigate to previous QSO.
		NAV_NEXT,           //!< Navigate to next QSO.
		NAV_LAST,           //!< Navigate to last QSO.
		ADD_QUERY,          //!< Add the QSO being queried to log.
		REJECT_QUERY,       //!< Reject the QSO being queries.
		MERGE_QUERY,        //!< Merge details from the QSO being queried to the displayed QSO.
		FIND_QSO,           //!< Find a possible match to this QSO and display it.
		BROWSE,             //!< Display the current QSO in qso_browse mode.
		KEEP_DUPE_1,        //!< When checking duplicates, keep the first and delete the second.
		KEEP_DUPE_2,        //!< When checking duplicates, keep the second and delete the first.
		MERGE_DUPE,         //!< When checking duplicates, merge datafrom the two QSOs.
		KEEP_BOTH_DUPES,    //!< When checking duplicates, keep both QSOs.
		MERGE_DONE,         //!< Merge complete, continue importing.
		LOOK_ALL_TXT,       //!< Search for possible match in WSJT-X ALL.TXT file.
		START_NET,          //!< Start net input mode.
		EDIT_NET,           //!< Display selected QSO and others around it that may form a net.
		SAVE_NET,           //!< SAve all QSOs in the net while entering them.
		CANCEL_NET,         //!< Cancel all QSOs im the net.
		ADD_NET_QSO,        //!< Start a new QSO as part of the net.
		SAVE_EDIT_NET,      //!< Save all QSOs in the net while editing them.
		ENTER_QUERY,        //!< Enter data to start a search for a specific QSO.
		EXEC_QUERY,         //!< Search for the QSO that matches any entered details.
		CANCEL_QUERY,       //!< Cancel a search query.
		IMPORT_QUERY,       //!< Test the search details as an import query.
		CANCEL_VIEW,        //!< Cancel the view of the current QSO.
		QRZ_COM,            //!< Search the contact callsign in QRZ.com.
		UPDATE_CAT,         //!< Update the QSO with the current rig information, 
		                    //!< but don't change any existing data.
		REPLACE_CAT,        //!< Update the QSO with the current rig information,
		                    //!< replacing existing data.
		RESTART,            //!< Cancel current QSO and start a new one.
		PARSE_QSO,          //!< Add DXCC parsing information to current QSO.
		UPDATE_STATION,     //!< Updatethe QSO with the current station details.
	};

	//! Fields in button configuration table used when instatntaiting the buttons. 
	struct button_action {
		const char* label;      //!< Its label
		const char* tooltip;    //!< The tooltip to display
		Fl_Callback* callback;  //!< Callback action
		void* userdata;         //!< Callback data
	};

	//! Callback to log QSO (start first if in QSO_PENDING)
	static void cb_save(Fl_Widget* w, void* v);
	//! Callback to quit QSO logging - delete QSO; clear fields
	static void cb_cancel(Fl_Widget* w, void* v);
	//! Callback to activate QSO logging 
	static void cb_activate(Fl_Widget* w, void* v);
	//! Callback to start QSO (log first if already in QSO_STARTED, Activate in QSO_INACTIVE))
	static void cb_start(Fl_Widget* w, void* v);
	//! Callback to enter edit mode
	static void cb_edit(Fl_Widget* w, void* v);
	//! Callback for navigate buttons
	static void cb_bn_navigate(Fl_Widget*, void* v);
	//! Callback to browse QSO record.
	static void cb_bn_browse(Fl_Widget* w, void* v);
	//! Callback to add query
	static void cb_bn_add_query(Fl_Widget* w, void* v);
	//! Callback to reject query
	static void cb_bn_reject_query(Fl_Widget* w, void* v);
	//! Callback to merge query
	static void cb_bn_merge_query(Fl_Widget* w, void* v);
	//! Callback to find QSO
	static void cb_bn_find_match(Fl_Widget* w, void* v);
	//! Callback for dupe functions.
	static void cb_bn_dupe(Fl_Widget* w, void* v);
	//! Callback to save or reject merge.
	static void cb_bn_save_merge(Fl_Widget* w, void* v);
	//! Callback to fetch data from QRZ.com
	static void cb_bn_fetch_qrz(Fl_Widget* w, void* v);
	//! Callback to search WSJT-X ALL.TXT for QSO.
	static void cb_bn_all_txt(Fl_Widget* w, void* v);
	//! Callback to save all qsos
	static void cb_bn_save_all(Fl_Widget* w, void* v);
	//! Callback to cancel all QSOs
	static void cb_bn_cancel_all(Fl_Widget* w, void* v);
	//! Callback to edit net
	static void cb_bn_add_net(Fl_Widget* w, void* v);
	//! Callback to start a net
	static void cb_bn_start_net(Fl_Widget* w, void* v);
	//! Callback to delete the selected QSO
	static void cb_bn_delete_qso(Fl_Widget* w, void* v);
	//! Callback to enter query data
	static void cb_bn_query_entry(Fl_Widget* w, void* v);
	//! Callback to execute query
	static void cb_bn_execute_query(Fl_Widget* w, void* v);
	//! Callback to cancel query
	static void cb_bn_cancel_query(Fl_Widget* w, void* v);
	//! Callback to import query
	static void cb_bn_import_query(Fl_Widget* w, void* v);
	//! Callback to view QSO
	static void cb_bn_view_qso(Fl_Widget* w, void* v);
	//! Callback to open QRZ.com
	static void cb_bn_qrz_com(Fl_Widget* w, void* v);
	//! Callback to update details from rig.
	static void cb_bn_update_cat(Fl_Widget* w, void* v);
	//! Callback to cancel current QSO and restart
	static void cb_bn_restart(Fl_Widget* w, void* v);
	//! Callback to parse callsign and add to QSO.
	static void cb_bn_parse_qso(Fl_Widget* w, void* v);
	//! Callback to update station details
	static void cb_bn_update_station(Fl_Widget* w, void* v);


protected:

	//! Disanle all buttons
	void disable_widgets();

	//! Maximum number of buttons supported in any mode.
	static const int MAX_ACTIONS = 20;

	//! Parent instance of qso_data.
	qso_data* qso_data_;
	//! Displayed buttons.
	Fl_Button* bn_action_[MAX_ACTIONS];

};


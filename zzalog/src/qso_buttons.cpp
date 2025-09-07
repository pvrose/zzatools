#include "qso_buttons.h"
#include "qso_data.h"
#include "extract_data.h"
#include "book.h"
#include "cty_data.h"
#include "qso_manager.h"
#include "spec_data.h"
#include "drawing.h"
#include "callback.h"
#include "record.h"

#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Tooltip.H>

extern extract_data* extract_records_;
extern book* book_;
extern cty_data* cty_data_;
extern spec_data* spec_data_;
extern void open_html(const char*);


// Map showing the buttons available in each qso_data logging_state
map<qso_data::logging_state_t, list<qso_buttons::button_type> > button_map_ =
{
	{ qso_data::QSO_INACTIVE, {qso_buttons::ACTIVATE, qso_buttons::START_QSO, qso_buttons::ADD_QSO, 
		qso_buttons::EDIT_QSO, qso_buttons::COPY_QSO, qso_buttons::CLONE_QSO, qso_buttons::DELETE_QSO,
		qso_buttons::START_NET, qso_buttons::EDIT_NET, qso_buttons::BROWSE, qso_buttons::VIEW_QSO, 
		qso_buttons::ENTER_QUERY } },
	{ qso_data::QSO_PENDING, { qso_buttons::START_QSO, qso_buttons::ADD_QSO, qso_buttons::EDIT_QSO, qso_buttons::COPY_QSO, 
		qso_buttons::CLONE_QSO, qso_buttons::QUIT_QSO, qso_buttons::SAVE_QSO, 
		qso_buttons::DELETE_QSO, qso_buttons::START_NET, qso_buttons::BROWSE, qso_buttons::VIEW_QSO } },
	{ qso_data::QSO_STARTED, { qso_buttons::SAVE_QSO, qso_buttons::SAVE_VIEW, qso_buttons::SAVE_NEW,
		qso_buttons::SAVE_CONTINUE, qso_buttons::CANCEL_QSO, 
		qso_buttons::START_NET, qso_buttons::UPDATE_STATION, qso_buttons::WORKED_B4, qso_buttons::PARSE, qso_buttons::QRZ_COM } },
	{ qso_data::QSO_ENTER, { qso_buttons::SAVE_NEW, qso_buttons::SAVE_EXIT, 
		qso_buttons::SAVE_CONTINUE, qso_buttons::UPDATE_STATION, qso_buttons::CANCEL_QSO } },
	{ qso_data::QSO_EDIT, { qso_buttons::SAVE_EDIT, qso_buttons::SAVE_EXIT, 
		qso_buttons::SAVE_VIEW, qso_buttons::CANCEL_EDIT, 
		qso_buttons::EDIT_NET, qso_buttons::NAV_FIRST,
		qso_buttons::NAV_PREV, qso_buttons::NAV_NEXT, qso_buttons::NAV_LAST,
		qso_buttons::UPDATE_CAT, qso_buttons::REPLACE_CAT, qso_buttons::UPDATE_STATION, 
		qso_buttons::QRZ_COM, qso_buttons::PARSE_QSO } },
	{ qso_data::QSO_VIEW, { qso_buttons::EDIT_QSO, qso_buttons::CANCEL_VIEW, qso_buttons::ACTIVATE,
	    qso_buttons::START_QSO, 
		qso_buttons::NAV_FIRST, qso_buttons::NAV_PREV, qso_buttons::NAV_NEXT, qso_buttons::NAV_LAST ,
		qso_buttons::QRZ_COM, qso_buttons::LOOK_ALL_TXT,
		qso_buttons::EDIT_NET, qso_buttons::START_NET, qso_buttons::BROWSE } },
	{ qso_data::QSO_BROWSE, { qso_buttons::EDIT_QSO, qso_buttons::CANCEL_BROWSE, qso_buttons::VIEW_QSO, 
	    qso_buttons::NAV_FIRST,
		qso_buttons::NAV_PREV, qso_buttons::NAV_NEXT, qso_buttons::NAV_LAST, qso_buttons::QRZ_COM } },
	{ qso_data::QUERY_MATCH, { qso_buttons::ADD_QUERY, qso_buttons::REJECT_QUERY, qso_buttons::MERGE_QUERY,
		qso_buttons::NAV_PREV, qso_buttons::NAV_NEXT, qso_buttons::LOOK_ALL_TXT, qso_buttons::QRZ_COM }},
	{ qso_data::QUERY_NEW, { qso_buttons::ADD_QUERY, qso_buttons::REJECT_QUERY, qso_buttons::FIND_QSO, 
		qso_buttons::LOOK_ALL_TXT, qso_buttons::QRZ_COM }},
	{ qso_data::QUERY_WSJTX, { qso_buttons::ADD_QUERY, qso_buttons::REJECT_QUERY } },
	{ qso_data::QUERY_DUPE, { qso_buttons::KEEP_DUPE_1, qso_buttons::MERGE_DUPE, qso_buttons::KEEP_DUPE_2,
		qso_buttons::KEEP_BOTH_DUPES }},
	{ qso_data::QUERY_SWL, { qso_buttons::ADD_QUERY, qso_buttons::REJECT_QUERY } },
	{ qso_data::QRZ_MERGE, { qso_buttons::MERGE_DONE }},
	{ qso_data::QRZ_COPY, { qso_buttons::MERGE_DONE }},
	{ qso_data::NET_STARTED, {qso_buttons::SAVE_NET, qso_buttons::SAVE_QSO, qso_buttons::SAVE_CONTINUE,
		qso_buttons::CANCEL_QSO, qso_buttons::CANCEL_NET,
		qso_buttons::NAV_FIRST, qso_buttons::NAV_PREV, qso_buttons::NAV_NEXT, qso_buttons::NAV_LAST,
		qso_buttons::ADD_NET_QSO, qso_buttons::QRZ_COM }},
	{ qso_data::NET_EDIT, { qso_buttons::SAVE_EDIT_NET, qso_buttons::CANCEL_QSO, qso_buttons::CANCEL_NET, 
		qso_buttons::NAV_FIRST, qso_buttons::NAV_PREV, qso_buttons::NAV_NEXT, qso_buttons::NAV_LAST,
	    qso_buttons::ADD_NET_QSO, qso_buttons::UPDATE_CAT, qso_buttons::QRZ_COM }},
	{ qso_data::MANUAL_ENTRY, { qso_buttons::EXEC_QUERY, qso_buttons::IMPORT_QUERY, qso_buttons::CANCEL_QUERY, qso_buttons::LOOK_ALL_TXT }},
	{ qso_data::QSO_WSJTX, { qso_buttons::CANCEL_QSO }},
	{ qso_data::QSO_FLDIGI, { qso_buttons::CANCEL_QSO }},
	{ qso_data::TEST_PENDING, { qso_buttons::START_QSO }},
	{ qso_data::TEST_ACTIVE, { qso_buttons::SAVE_QSO, qso_buttons::CANCEL_QSO, qso_buttons::SAVE_NEW, qso_buttons::RESTART }}
};

// Map describing all the parameters for each button
map<qso_buttons::button_type, qso_buttons::button_action> action_map_ =
{
	{ qso_buttons::ACTIVATE, { "Activate", "Pre-load QSO fields based on logging mode", qso_buttons::cb_activate, 0 } },
	{ qso_buttons::START_QSO, { "Start QSO", "Start a QSO in real-time", qso_buttons::cb_start, (void*)qso_data::QSO_ON_AIR } },
	{ qso_buttons::EDIT_QSO, { "Edit QSO", "Edit the selected QSO", qso_buttons::cb_edit, 0 } },
	{ qso_buttons::VIEW_QSO, { "View QSO", "View the selected QSO in entry view", qso_buttons::cb_bn_view_qso, 0 } },
	{ qso_buttons::ADD_QSO, { "Add QSO", "Create a new record (no initialisation)", qso_buttons::cb_start, (void*)qso_data::QSO_NONE }},
	{ qso_buttons::COPY_QSO, { "Copy QSO", "Create a new record (copy call and conditions)", qso_buttons::cb_start, (void*)qso_data::QSO_COPY_CALL }},
	{ qso_buttons::CLONE_QSO, { "Clone QSO", "Create a new record (copy conditions)", qso_buttons::cb_start, (void*)qso_data::QSO_COPY_CONDX }},
	{ qso_buttons::BROWSE, { "Browse Log", "Browse the log without editing", qso_buttons::cb_bn_browse, 0}} ,
	{ qso_buttons::QUIT_QSO, { "Quit", "Quit entry mode", qso_buttons::cb_cancel, 0 } },
	{ qso_buttons::SAVE_QSO, { "Save", "Log the QSO (set start time if not set) and quit", qso_buttons::cb_save, (void*)qso_buttons::SAVE_QSO } },
	{ qso_buttons::CANCEL_QSO, { "Quit QSO", "Cancel the current QSO entry", qso_buttons::cb_cancel, 0 } },
	{ qso_buttons::DELETE_QSO, { "Delete QSO", "Delete the selected QSO", qso_buttons::cb_bn_delete_qso, 0 } },
	{ qso_buttons::WORKED_B4, { "B4?", "Display all previous QSOs with this callsign", qso_buttons::cb_wkb4, 0 } },
	{ qso_buttons::SAVE_EDIT, { "Save", "Copy changed record back to book", qso_buttons::cb_save, (void*)qso_buttons::SAVE_EDIT}},
	{ qso_buttons::SAVE_CONTINUE, { "Save && Edit", "Set TIME_OFF and allow continued edit", qso_buttons::cb_save, (void*)qso_buttons::SAVE_CONTINUE}},
	{ qso_buttons::SAVE_EXIT, { "Save && Exit", "Copy changed record and return to previous activity", qso_buttons::cb_save, (void*)qso_buttons::SAVE_EXIT }},
	{ qso_buttons::SAVE_VIEW, { "Save && View", "Copy changed record and allow view", qso_buttons::cb_save, (void*)qso_buttons::SAVE_VIEW }},
	{ qso_buttons::SAVE_NEW, { "Save && New", "Save QSO and start new QSO", qso_buttons::cb_save, (void*)qso_buttons::SAVE_NEW }},
	{ qso_buttons::CANCEL_EDIT, { "Cancel Edit", "Cancel the current QSO edit", qso_buttons::cb_cancel, 0 } },
	{ qso_buttons::CANCEL_VIEW, { "Cancel", "Cancel the current QSO view", qso_buttons::cb_cancel, 0 } },
    { qso_buttons::NAV_FIRST, { "@$->|", "Select first record in net or book", qso_buttons::cb_bn_navigate, (void*)NV_FIRST } },
	{ qso_buttons::NAV_PREV, { "@<-", "Select previous record in net or book", qso_buttons::cb_bn_navigate, (void*)NV_PREV } },
	{ qso_buttons::NAV_NEXT, { "@->", "Select next record in net or book", qso_buttons::cb_bn_navigate, (void*)NV_NEXT } },
	{ qso_buttons::NAV_LAST, { "@->|", "Select last record in net or book", qso_buttons::cb_bn_navigate, (void*)NV_LAST } },
	{ qso_buttons::PARSE, { "DX?", "Display the DX details for this callsign", qso_buttons::cb_parse, 0 } },
	{ qso_buttons::CANCEL_BROWSE, { "Quit Browse", "Quit browse mode", qso_buttons::cb_cancel, 0 } },
	{ qso_buttons::ADD_QUERY, { "Add QSO", "Add queried QSO to log", qso_buttons::cb_bn_add_query, 0 }},
	{ qso_buttons::REJECT_QUERY, {"Reject QSO", "Do not add queried QSO to log", qso_buttons::cb_bn_reject_query, 0} },
	{ qso_buttons::MERGE_QUERY, {"Merge QSO", "Merge query with logged QSO", qso_buttons::cb_bn_merge_query, 0 } },
	{ qso_buttons::FIND_QSO, { "@search", "Display possible match", qso_buttons::cb_bn_find_match, 0}},
	{ qso_buttons::KEEP_DUPE_1, { "Keep 1", "Keep first QSO and delete second", qso_buttons::cb_bn_dupe, (void*)qso_data::DF_1}},
	{ qso_buttons::KEEP_DUPE_2, { "Keep 2", "Keep second QSO and delete first", qso_buttons::cb_bn_dupe, (void*)qso_data::DF_2}},
	{ qso_buttons::MERGE_DUPE, { "Merge", "Merge the two records", qso_buttons::cb_bn_dupe, (void*)qso_data::DF_MERGE}},
	{ qso_buttons::KEEP_BOTH_DUPES, { "Keep 1 && 2", "Keep both records", qso_buttons::cb_bn_dupe, (void*)qso_data::DF_BOTH}},
	{ qso_buttons::MERGE_DONE, { "Done", "Save changes", qso_buttons::cb_bn_save_merge, 0} },
	{ qso_buttons::LOOK_ALL_TXT, { "@search ALL.TXT", "Look in WSJT-X ALL.TXT file for possible contact", qso_buttons::cb_bn_all_txt, 0 } },
	{ qso_buttons::START_NET, { "Start Net", "Start a QSO with more than one other station", qso_buttons::cb_bn_start_net, 0 } },
	{ qso_buttons::EDIT_NET, { "Edit Net", "Open all calls that overlap", qso_buttons::cb_bn_add_net, 0}},
	{ qso_buttons::SAVE_NET, { "Save Net", "Log all the QSOs and quit", qso_buttons::cb_bn_save_all, 0}},
	{ qso_buttons::CANCEL_NET, { "Quit Net", "Cancel all QSOs", qso_buttons::cb_bn_cancel_all, 0}},
	{ qso_buttons::ADD_NET_QSO, { "Add Call", "Add a QSO with this call to the net", qso_buttons::cb_bn_add_net, 0}},
	{ qso_buttons::SAVE_EDIT_NET, {"Save Net", "Save all QSOs in the net", qso_buttons::cb_bn_save_all, 0} },
	{ qso_buttons::ENTER_QUERY, { "Query", "Enter QSO details for search query", qso_buttons::cb_bn_query_entry, 0 }},
	{ qso_buttons::EXEC_QUERY, { "Check", "Execute query", qso_buttons::cb_bn_execute_query, 0 }},
	{ qso_buttons::CANCEL_QUERY, { "Cancel", "Cancel query", qso_buttons::cb_bn_cancel_query, 0 }},
	{ qso_buttons::IMPORT_QUERY, { "Test Import", "Test import query", qso_buttons::cb_bn_import_query, 0 }},
	{ qso_buttons::QRZ_COM, { "@search QRZ.com", "Display details in QRZ.com", qso_buttons::cb_bn_qrz_com, 0}},
	{ qso_buttons::UPDATE_CAT, { "Update CAT", "Use CAT info where current QSO has no value", qso_buttons::cb_bn_update_cat, (void*)false }},
	{ qso_buttons::REPLACE_CAT, { "Replace CAT", "Use current CAT info", qso_buttons::cb_bn_update_cat, (void*)true }},
	{ qso_buttons::RESTART, { "Restart", "Ditch current QSO and start anew", qso_buttons::cb_bn_restart, 0 }},
	{ qso_buttons::PARSE_QSO, { "Parse QSO", "Add DXCC, CQ, etc details to QSO", qso_buttons::cb_bn_parse_qso, 0 }},
	{ qso_buttons::UPDATE_STATION, { "U/d Station", "Add QTH, Operator and station callsigns to QSO", qso_buttons::cb_bn_update_station, 0 }},
};

// Constructor
qso_buttons::qso_buttons(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L),
	qso_data_((qso_data*)parent())
{
	tooltip("Control buttons");
	create_form(X, Y);
	enable_widgets();
}

// Destructor
qso_buttons::~qso_buttons() {
}

// Handle
int qso_buttons::handle(int event) {
	int result = Fl_Group::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		if (!result) take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("qso_buttons.html");
			return true;
		}
		break;
	}
	return result;
}

// Create all the buttons
void qso_buttons::create_form(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	label("Controls");
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;
	int max_x = curr_x;

	const int NUMBER_PER_ROW = 10;
	// Create the maximum number of buttons (MAX_ACTIONS) in rows of (NUMBER_PER_ROW)
	for (int ix = 0; ix < MAX_ACTIONS; ix++) {
		bn_action_[ix] = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "");
		if ((ix + 1) % NUMBER_PER_ROW == 0 && ix < MAX_ACTIONS) {
			curr_x += WBUTTON;
			max_x = max(max_x, curr_x);
			curr_x = X + GAP;
			curr_y += HBUTTON;
		}
		else {
			curr_x += WBUTTON;
			max_x = max(max_x, curr_x);
		}
	}
	if (MAX_ACTIONS % NUMBER_PER_ROW != 0) {
		curr_y += HBUTTON;
	}

	max_x += GAP;
	curr_y += GAP;
	resizable(nullptr);
	size(max_x - X, curr_y - Y);
	end();
}

// Configure the buttons according to the logging_state
void qso_buttons::enable_widgets() {
	int ix = 0;
	if (button_map_.find(qso_data_->logging_state()) != button_map_.end()) {
		// If we have a button map for the state use it - else deactivate all buttons
		const list<button_type>& buttons = button_map_.at(qso_data_->logging_state());
		// Activate the buttons we need and set their parameters
		for (auto bn = buttons.begin(); bn != buttons.end() && ix < MAX_ACTIONS; bn++, ix++) {
			const button_action& action = action_map_.at(*bn);
			bn_action_[ix]->label(action.label);
			bn_action_[ix]->tooltip(action.tooltip);
			//bn_action_[ix]->color(action.colour);
			//bn_action_[ix]->labelcolor(fl_contrast(FL_FOREGROUND_COLOR, action.colour));
			bn_action_[ix]->callback(action.callback, action.userdata);
			switch(*bn) {
				case NAV_FIRST:
				case NAV_LAST:
				case NAV_NEXT:
				case NAV_PREV:
				{
					if (qso_data_->can_navigate((navigate_t)(intptr_t)action.userdata)) {
						bn_action_[ix]->activate();
					} else {
						bn_action_[ix]->deactivate();
					}
					break;
				}
				default:
					bn_action_[ix]->activate();
			}
		}
	}
	// Deactivate any remaining buttons
	for (; ix < MAX_ACTIONS; ix++) {
		bn_action_[ix]->label("");
		bn_action_[ix]->tooltip("");
		bn_action_[ix]->color(FL_BACKGROUND_COLOR);
		bn_action_[ix]->callback((Fl_Callback*)nullptr);
		bn_action_[ix]->deactivate();
	}

}

// Deactivate all buttons to prevent double clicking
void qso_buttons::disable_widgets() {
	for (int ix = 0; ix < MAX_ACTIONS; ix++) {
		bn_action_[ix]->deactivate();
	}
}

// Activate- Go from qso_data::QSO_INACTIVE to qso_data::QSO_PENDING
// v is not used
void qso_buttons::cb_activate(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_VIEW:
		that->qso_data_->action_cancel_edit();
		// Drop through
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_activate(qso_data::QSO_ON_AIR);
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Start QSO - transition from qso_data::QSO_INACTIVE->qso_data::QSO_PENDING->qso_data::QSO_STARTED
// v is not used
void qso_buttons::cb_start(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	qso_data::qso_init_t mode = (qso_data::qso_init_t)(intptr_t)v;
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_activate(mode);
		// Fall into next state
	case qso_data::TEST_PENDING:
	case qso_data::QSO_PENDING:
		that->qso_data_->action_start(mode);
		break;
	case qso_data::QSO_VIEW:
		that->qso_data_->action_cancel_edit();
		that->qso_data_->action_activate(mode);
		that->qso_data_->action_start(mode);
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Save QSO - transition through qso_data::QSO_PENDING->qso_data::QSO_STARTED->qso_data::QSO_INACTIVE saving QSO
// v is used in QSO_EDIT state to indicate type of save.
void qso_buttons::cb_save(Fl_Widget* w, void* v) {
	qso_data* data = ancestor_view<qso_data>(w);
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	button_type edit_button = (button_type)(intptr_t)v;
	qso_num_t current = data->current_number();
	if (that) that->disable_widgets();
	switch (data->logging_state()) {
	case qso_data::QSO_PENDING:
		// If in pending then we can assume it's started
		data->action_start(qso_data::QSO_AS_WAS);
		// Two routes - QSO entry
	case qso_data::QSO_STARTED:
	case qso_data::TEST_ACTIVE:
		// Realtime entry - do not start another
		if (!data->action_save(edit_button == SAVE_CONTINUE)) break;
		switch (edit_button) {
			case SAVE_QSO: {
				data->action_activate(qso_data::QSO_AS_WAS);
				break;
			}
			case SAVE_NEW: {
				data->action_activate(qso_data::QSO_AS_WAS);
				data->action_start(qso_data::QSO_AS_WAS);
				break;
			}
			case SAVE_VIEW: {
				data->action_activate(qso_data::QSO_AS_WAS);
				data->action_view(current);
				break;
			}
			case SAVE_CONTINUE: {
				data->action_activate(qso_data::QSO_AS_WAS);
				data->action_view(current);
				data->action_edit();
				break;
			}
			default:
				break;
		}
		break;
	case qso_data::QSO_ENTER:
		// Batch entry - start another entry
		if(!data->action_save(edit_button == SAVE_CONTINUE)) break;
		switch(edit_button) {
			case SAVE_NEW: {
				data->action_activate(qso_data::QSO_NONE);
				data->action_start(qso_data::QSO_NONE);
				break;
			}
			case SAVE_EXIT: {
				data->action_deactivate();
				break;
			}
			default:
				break;
		}
		break;
		// QSO editing
	case qso_data::QSO_EDIT:
		data->action_save_edit();
		switch (edit_button) {
		case SAVE_EXIT:
			switch (data->edit_return_state_) {
			case qso_data::QSO_INACTIVE:
				break;
			case qso_data::QSO_PENDING:
				data->action_activate(qso_data::QSO_AS_WAS);
				break;
			case qso_data::MANUAL_ENTRY:
				data->action_query_entry();
				break;
			default:
				break;
			}
			break;
		case SAVE_EDIT:
			data->action_edit();
			break;
		case SAVE_VIEW:
			data->action_view();
			break;
		default:
			break;
		}
		break;
	case qso_data::NET_STARTED:
		data->action_save(edit_button == SAVE_CONTINUE);
		break;
	default:
		break;
	}
	if (that) that->enable_widgets();
}

// Cancel QSO - delete QSO; clear fields
// v is not used
void qso_buttons::cb_cancel(Fl_Widget* w, void* v) {
	qso_data* data = ancestor_view<qso_data>(w);
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	if (that) that->disable_widgets();
	switch (data->logging_state()) {
	case qso_data::QSO_PENDING:
		data->action_deactivate();
		break;
	case qso_data::QSO_STARTED:
	case qso_data::QSO_ENTER:
	case qso_data::TEST_ACTIVE:
		data->action_cancel();
		data->action_activate(qso_data::QSO_AS_WAS);
		break;
	case qso_data::QSO_EDIT:
	case qso_data::QSO_VIEW:
		data->action_cancel_edit();
		break;
	case qso_data::QSO_BROWSE:
		data->action_cancel_browse();
		break;
	case qso_data::NET_STARTED:
		data->action_cancel();
		break;
	case qso_data::NET_EDIT:
		data->action_cancel_net_edit();
		break;
	case qso_data::QSO_WSJTX:
	case qso_data::QSO_FLDIGI:
		data->action_cancel_modem();
		break;
	default:
		break;
	}
	if (that) that->enable_widgets();
}

// Edit QSO - transition to qso_data::QSO_EDIT
// v is not used
void qso_buttons::cb_edit(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	that->qso_data_->action_remember_state();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_edit();
		break;
	case qso_data::QSO_PENDING:
		that->qso_data_->action_deactivate();
		that->qso_data_->action_edit();
		break;
	case qso_data::QSO_BROWSE:
		that->qso_data_->action_cancel_browse();
		that->qso_data_->action_edit();
		break;
	case qso_data::QSO_VIEW:
		that->qso_data_->action_edit();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// View QSO - transition to qso_data::QSO_VIEW
// v is not used
void qso_buttons::cb_bn_view_qso(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	that->qso_data_->action_remember_state();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_view();
		break;
	case qso_data::QSO_PENDING:
		that->qso_data_->action_deactivate();
		that->qso_data_->action_view();
		break;
	case qso_data::QSO_BROWSE:
		that->qso_data_->action_cancel_browse();
		that->qso_data_->action_view();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback - Worked B4? button
// v is not used
void qso_buttons::cb_wkb4(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	extract_records_->extract_call(that->qso_data_->get_call());
	book_->selection(that->qso_data_->get_default_number(), HT_SELECTED);
	that->enable_widgets();
}

// Callback - Parse callsign
// v is not used
void qso_buttons::cb_parse(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	// Create a temporary record to parse the callsign
	record* tip_record = that->qso_data_->dummy_qso();
	string message = "";
	// Set the callsign in the temporary record
	tip_record->item("CALL", string(that->qso_data_->get_call()));
	// Parse the temporary record
	message = cty_data_->get_tip(tip_record);
	// Create a tooltip window at the parse button (in w) X and Y
	Fl_Window* qw = ancestor_view<qso_manager>(w);
	Fl_Window* tw = ::tip_window(message, qw->x_root() + w->x() + w->w() / 2, qw->y_root() + w->y() + w->h() / 2);
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
	delete tip_record;
	that->enable_widgets();
}

// CAllback - navigate buttons
// v - direction
void qso_buttons::cb_bn_navigate(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	navigate_t target = (navigate_t)(intptr_t)v;
	that->qso_data_->action_navigate(target);
	that->enable_widgets();
}

// Callback - browse
// v is not used
void qso_buttons::cb_bn_browse(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_PENDING:
	case qso_data::QSO_VIEW:
		that->qso_data_->action_deactivate();
		// Drop through
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_browse();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback - add query record
// v is not used
void qso_buttons::cb_bn_add_query(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW:
	case qso_data::QUERY_WSJTX:
	case qso_data::QUERY_SWL:
		that->qso_data_->action_add_query();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback - add query record
// v is not used
void qso_buttons::cb_bn_reject_query(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW:
	case qso_data::QUERY_WSJTX:
	case qso_data::QUERY_SWL:
		that->qso_data_->action_reject_query();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback - add query record
// v is not used
void qso_buttons::cb_bn_merge_query(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW:
		that->qso_data_->action_merge_query();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback - add query record
// v is not used
void qso_buttons::cb_bn_find_match(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QUERY_NEW:
		that->qso_data_->action_find_match();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback - dupe action
// v is not used
void qso_buttons::cb_bn_dupe(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	qso_data::dupe_flags action = (qso_data::dupe_flags)(intptr_t)v;
	switch (that->qso_data_->logging_state()) {
	case qso_data::QUERY_DUPE:
		that->qso_data_->action_handle_dupe(action);
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback QRZ merge action
// v is not used
void qso_buttons::cb_bn_save_merge(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QRZ_MERGE:
	case qso_data::QRZ_COPY:
		that->qso_data_->action_save_merge();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Callback Find QSO in WSJT-X
// v is not used
void qso_buttons::cb_bn_all_txt(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QUERY_NEW:
	case qso_data::QUERY_MATCH:
	case qso_data::QSO_VIEW:
	case qso_data::MANUAL_ENTRY:
		that->qso_data_->action_look_all_txt();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Save all qsos
// v is not used
void qso_buttons::cb_bn_save_all(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::NET_STARTED:
	case qso_data::NET_EDIT:
		that->qso_data_->action_save_net_all();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Cancel all QSOs
// v is not used
void qso_buttons::cb_bn_cancel_all(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::NET_STARTED:
	case qso_data::NET_EDIT:
		that->qso_data_->action_cancel_net_all();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Add a QSO to the net
// v is not used
void qso_buttons::cb_bn_add_net(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_VIEW:
		that->qso_data_->action_cancel_edit();
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_edit();
		// NB state may now be QSO_EDIT _or_ QSO_STARTED
	case qso_data::QSO_EDIT:
		that->qso_data_->action_create_net();
		break;
	case qso_data::NET_STARTED:
	case qso_data::NET_EDIT:
		that->qso_data_->action_add_net_qso();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Start a net
// v is not used
void qso_buttons::cb_bn_start_net(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_VIEW:
		that->qso_data_->action_cancel_edit();
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_activate(qso_data::QSO_ON_AIR);
	case qso_data::QSO_PENDING:
		that->qso_data_->action_start(qso_data::QSO_ON_AIR);
	case qso_data::QSO_STARTED:
		that->qso_data_->action_create_net();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Delete the current QSO
// v is not used
void qso_buttons::cb_bn_delete_qso(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
	case qso_data::QSO_PENDING:
		that->qso_data_->action_delete_qso();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Open a qso_query to define search criteria
// v is not used
void qso_buttons::cb_bn_query_entry(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
		that->qso_data_->action_query_entry();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Open a qso_query to define search criteria
// v is not used
void qso_buttons::cb_bn_execute_query(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	that->qso_data_->action_remember_state();
	switch (that->qso_data_->logging_state()) {
	case qso_data::MANUAL_ENTRY:
		that->qso_data_->action_exec_query();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Open a qso_query to define search criteria
// v is not used
void qso_buttons::cb_bn_cancel_query(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::MANUAL_ENTRY:
		that->qso_data_->action_cancel_query();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Open a qso_query to define search criteria
// v is not used
void qso_buttons::cb_bn_import_query(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::MANUAL_ENTRY:
		that->qso_data_->action_import_query();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Open a browser with QRZ.com
// v is not used
void qso_buttons::cb_bn_qrz_com(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_PENDING:
	case qso_data::TEST_PENDING:
	case qso_data::QSO_STARTED:
	case qso_data::QSO_EDIT:
	case qso_data::QSO_VIEW:
	case qso_data::QSO_BROWSE:
	case qso_data::NET_EDIT:
	case qso_data::NET_STARTED:
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW:
		that->qso_data_->action_qrz_com();
		break;
	default:
		break;
	}
	that->enable_widgets();
}

// Update QSO with CAT data
// v is not used
void qso_buttons::cb_bn_update_cat(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	bool clear = (bool)(intptr_t)v;
	switch(that->qso_data_->logging_state()) {
		case qso_data::QSO_EDIT:
		case qso_data::NET_EDIT: {
			that->qso_data_->action_update_cat(clear);
			break;
		default:
			break;
		}
	}
	that->enable_widgets();
}

// Cancel current QSO and restart
void qso_buttons::cb_bn_restart(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch (that->qso_data_->logging_state()) {
	case qso_data::TEST_ACTIVE: {
		that->qso_data_->action_cancel();
		that->qso_data_->action_activate(qso_data::QSO_ON_AIR);
		that->qso_data_->action_start(qso_data::QSO_ON_AIR);
		break;
	}
	default:
		break;
	}
	that->enable_widgets();
}

// Parse the QSO
void qso_buttons::cb_bn_parse_qso(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	that->disable_widgets();
	switch(that->qso_data_->logging_state()) {
	case qso_data::QSO_EDIT: {
		that->qso_data_->action_parse_qso();
		break;
	}
	default:
		break;
	}
	that->enable_widgets();
}

// Update station details
void qso_buttons::cb_bn_update_station(Fl_Widget* w, void* v) {
	qso_buttons* that = ancestor_view<qso_buttons>(w);
	switch(that->qso_data_->logging_state()) {
	case qso_data::QSO_STARTED:
	case qso_data::QSO_EDIT:
	case qso_data::QSO_ENTER:
		that->qso_data_->update_station_fields();
		break;
	default:
		break;
	}
	that->enable_widgets();
}
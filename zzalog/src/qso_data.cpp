#include "qso_data.h"

#include "book.h"
#include "config.h"
#include "cty_data.h"
#include "extract_data.h"
#include "import_data.h"
#include "menu.h"
#include "qrz_handler.h"
#include "qso_buttons.h"
#include "qso_contest.h"
#include "qso_entry.h"
#include "qso_manager.h"
#include "qso_misc.h"
#include "qso_net_entry.h"
#include "qso_operation.h"
#include "qso_qsl.h"
#include "qso_query.h"
#include "record.h"
#include "rig_if.h"
#include "status.h"
#include "spec_data.h"
#include "tabbed_forms.h"
#include "wsjtx_handler.h"

#include "utils.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tooltip.H>

extern book* book_;
extern extract_data* extract_records_;
extern status* status_;
extern cty_data* cty_data_;
extern spec_data* spec_data_;
extern tabbed_forms* tabbed_forms_;
extern import_data* import_data_;
extern book* navigation_book_;
extern qrz_handler* qrz_handler_;
extern wsjtx_handler* wsjtx_handler_;
extern menu* menu_;
extern double prev_freq_;
extern bool DARK;
extern fields* fields_;
extern config* config_;
extern string VENDOR;
extern string PROGRAM_ID;

// qso_group_
qso_data::qso_data(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
	, logging_state_(QSO_INACTIVE)
	, inhibit_drawing_(false)
	, previous_mode_(QSO_NONE)
	, edit_return_state_(QSO_INACTIVE)
{
	// Register default field names for qso_entry and qso_query
	fields_->collection("QSO Manager", QSO_FIELDS);
	fields_->link_app(FO_QSOVIEW, "QSO Manager");

	load_values();
}

// Destructor
qso_data::~qso_data() {
}

// Event handler - catch Page up, page down, home and end and navigate accordingly
int qso_data::handle(int event) {
	navigate_t dirn;
	bool do_nav = false;
	switch(event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		// Tell FLTK event handler we are interested in keyboard events
		return 1;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_Page_Up:
			dirn = NV_PREV;
			do_nav = true;
			break;
		case FL_Page_Down:
			dirn = NV_NEXT;
			do_nav = true;
			break;
		case FL_Home:
			dirn = NV_FIRST;
			do_nav = true;
			break;
		case FL_End:
			dirn = NV_LAST;
			do_nav = true;
			break;
		}
	}
	if (do_nav) {
		action_navigate(dirn);
		return false; 
	} else {
		return Fl_Group::handle(event);
	}
}

// Load values
void qso_data::load_values() {
}

// Create qso_data
void qso_data::create_form(int X, int Y) {
	int max_x = X;
	int max_y = Y;

	begin();
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	label("QSO Data");
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);

	int curr_y = Y + HTEXT;
	int curr_x = X + GAP;

	g_station_ = new qso_operation(curr_x, curr_y, 10, 10);

	curr_y += g_station_->h();

	// One or the other of the three groups below will be shown at a time

	// Single QSO entry 
	g_entry_ = new qso_entry(curr_x, curr_y, 10, 10);
	g_entry_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_entry_->labelfont(FL_BOLD);
	g_entry_->labelsize(FL_NORMAL_SIZE + 2);

	max_x = max(max_x, g_entry_->x() + g_entry_->w());
	max_y = max(max_y, g_entry_->y() + g_entry_->h());
	// Multiple QSO entry
	g_net_entry_ = new qso_net_entry(curr_x, curr_y, 10, 10);
	max_x = max(max_x, g_net_entry_->x() + g_net_entry_->w());
	max_y = max(max_y, g_net_entry_->y() + g_net_entry_->h());

	// Query form
	g_query_ = new qso_query(curr_x, curr_y, g_net_entry_->w(), g_net_entry_->h());

	max_x = max(max_x, g_query_->x() + g_query_->w());
	max_y = max(max_y, g_query_->y() + g_query_->h());

	// Manual QSO query - displays a qso_entry form
	g_qy_entry_ = new qso_entry(curr_x, curr_y, 10, 10);
	g_qy_entry_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_qy_entry_->labelfont(FL_BOLD);
	g_qy_entry_->labelsize(FL_NORMAL_SIZE + 2);
	max_x = max(max_x, g_qy_entry_->x() + g_qy_entry_->w());
	max_y = max(max_y, g_qy_entry_->y() + g_qy_entry_->h());

	g_entry_->size(g_entry_->w(), g_net_entry_->h());
	g_qy_entry_->size(g_entry_->w(), g_net_entry_->h());

	// Misc info
	curr_x = max_x + GAP;
	g_misc_ = new qso_misc(curr_x, curr_y, WBUTTON * 7 / 2, g_net_entry_->h());

	max_x = max(max_x, g_misc_->x() + g_misc_->w());


	curr_y = max_y;
	curr_y += GAP;
	curr_x = X + GAP;

	// Display the buttons for the particular logging_stte_
	g_buttons_ = new qso_buttons(curr_x, curr_y, 10, 10);

	max_x = max(max_x, g_buttons_->x() + g_buttons_->w());
	max_x += GAP;
	curr_y += g_buttons_->h() + GAP;

	resizable(nullptr);
	size(max_x - X, curr_y - Y);
	end();

	// Check if contest is active
	if (in_contest()) {
		action_activate(QSO_ON_AIR);
	}
	else {
		action_view();
	}
	enable_widgets();
	// May have added widgets 
	

}

// Enable QSO widgets
void qso_data::enable_widgets() {
	if (!inhibit_drawing_) {
		// Disable log mode menu item from CAT if no CAT
		qso_manager* mgr = (qso_manager*)parent();
		if (mgr->created_) {
			// rig_if* rig = mgr->rig();
			mgr->qsl_control()->enable_widgets();
			if (menu_) menu_->update_items();
			if (config_) config_->update();
		}

		string call;
		if (current_qso()) call = current_qso()->item("CALL");
		else call = "N/A";
		// Enable station details
		g_station_->qso(current_qso());
		char l[128];
		switch (logging_state_) {
		case QSO_INACTIVE:
			// No QSO - show the entry form, hide the others
			g_entry_->label("QSO Entry is not enabled");
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(nullptr, -1);
			g_misc_->enable_widgets();
			break;
		case QSO_PENDING:
			// Real-time logging - waiting to start QSO
			g_entry_->label("QSO Entry - prepared for real-time logging.");
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case TEST_PENDING:
			// Real-time logging - waiting to start QSO
			snprintf(l, sizeof(l), "QSO Entry - Contest %s pending QSOs", contest()->contest_id().c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_STARTED:
			// Real-time logging - QSO started
			snprintf(l, sizeof(l), "QSO Entry - %s - logging new contact", call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case TEST_ACTIVE:
			// Real-time logging - QSO started
			snprintf(l, sizeof(l), "QSO Entry - Contest %s logging %s",
				contest()->contest_id().c_str(), call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_ENTER:
			// Back-logging - enter QSO details
			snprintf(l, sizeof(l), "QSO Entry - %s - logging old contact", call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_EDIT:
			// Editing an existing QSO
			snprintf(l, sizeof(l), "QSO Entry - %s - editing existing contact", call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_VIEW:
			// Viewing (read-only) an existing QSO
			snprintf(l, sizeof(l), "QSO Entry - %s - read only", call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_BROWSE:
			// Browsing - display using tabular view
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", call.c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			if (visible_r()) g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QUERY_DUPE:
			// Show two logged QSOs in query form to check for duplicate
		case QUERY_MATCH:
			// Show an imported QSO and possible match in log check if matcehd
		case QRZ_MERGE:
		case QRZ_COPY:
			// Show details downloaded from QRZ.com and check any matches
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", call.c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			if (visible_r()) g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QUERY_NEW:
			// Show an imported QSO (no match found) and check if real
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", call.c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			if (visible_r()) g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QUERY_WSJTX:
			// Show an imported QSO and possible match found in WSJTX All.TXT file
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - Possible match found in ALL.TXT", call.c_str());
			g_query_->copy_label(l);
			if (visible_r()) g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case NET_STARTED:
			// Display multiple QSOs in real-time
			g_entry_->hide();
			g_query_->hide();
			g_net_entry_->label("Net Entry - active real-time logging");
			if (visible_r()) g_net_entry_->show();
			g_net_entry_->enable_widgets();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case NET_EDIT:
			// Display looged QSOs that (may be) a net
			g_entry_->hide();
			g_query_->hide();
			g_net_entry_->label("Net Entry - off-air logging");
			if (visible_r()) g_net_entry_->show();
			g_net_entry_->enable_widgets();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_WSJTX:
			// Display real-time Qso from WSJT-X
			snprintf(l, sizeof(l), "QSO Entry - %s - record received from WSJTX", call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case QSO_FLDIGI:
			// Display real-time QSO received from FLDID
			snprintf(l, sizeof(l), "QSO Entry - %s - record received from FLDIGI", call.c_str());
			g_entry_->copy_label(l);
			if (visible_r()) g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		case MANUAL_ENTRY:
			// Display entry form to search for qSOs in log
			g_entry_->hide();
			g_query_->hide();
			if (visible_r()) g_qy_entry_->show();
			g_qy_entry_->label("Enter QSO details for search");
			g_qy_entry_->enable_widgets();
			g_misc_->activate();
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
			break;
		default:
			break;
		}
		g_buttons_->enable_widgets();

		// Redraw this as some of the above labels may have extended into it.
		redraw();
	}
}

// Update QSO
void qso_data::update_qso(qso_num_t log_num) {
	switch (logging_state_) {
	case QSO_INACTIVE:
	{
		// Copy selected QSO 
		break;
	}
	case QSO_PENDING:
	case TEST_PENDING:
		if (log_num != g_entry_->qso_number()) {
			// Deactivate then reactivate with new QSO
			action_deactivate();
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			g_misc_->qso(current_qso(), current_number());
		}
		break;
	case QSO_STARTED:
	case QSO_ENTER:
	case TEST_ACTIVE:
		// Ack whether to save or quit then activate new QSO
		if (log_num != g_entry_->qso_number()) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while logging a QSO", "Save QSO", "Quit QSO", "Ignore")) {
			case 0:
				// Save QSO
				if(!action_save(false)) break;
				book_->selection(book_->item_number(log_num));
				action_deactivate();;
				break;
			case 1:
				// Cancel QSO
				action_cancel();
				book_->selection(book_->item_number(log_num));
				action_deactivate();;
				break;
			case 2:
				// Ignore the selection request
				book_->selection(book_->item_number(g_entry_->qso_number()));
				break;
			}
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			g_misc_->qso(current_qso(), current_number());
		}
		break;
	case QSO_EDIT:
		// Ask whether to save or quit then open new QSO in edit mode
		if (log_num != g_entry_->qso_number()) {
			if (book_->is_dirty_record(g_entry_->qso())) {
				fl_beep(FL_BEEP_QUESTION);
				switch (fl_choice("Trying to select a different record while editing a record", "Save edit", "Cancel edit", "Ignore")) {
				case 0:
					// Save QSO
					action_save_edit();
					action_edit();
					break;
				case 1:
					// Cancel QSO
					action_cancel_edit();
					action_edit();
					break;
				case 2:
					// Ignore the selection request
					book_->selection(book_->item_number(g_entry_->qso_number()));
					break;
				}
			}
			else {
				// Record not changed so just cancel the edit
				action_cancel_edit();
			}
			action_edit();
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			g_misc_->qso(current_qso(), current_number());
		}
		break;
	case QSO_BROWSE:
		// Open new record in browse mode
		action_cancel_browse();
		action_browse();
		break;
	case QSO_VIEW:
		action_view();
		break;
	case NET_STARTED:
	case NET_EDIT:
		if (!g_net_entry_->qso_in_net(log_num)) {
			// Selected QSO is not part of the net, save or cancel the net
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while logging a net", "Save Net", "Quit Net", "Ignore")) {
			case 0:
				// Save QSO
				action_save_net_all();
				action_deactivate();;
				break;
			case 1:
				// Cancel QSO
				action_cancel_net_all();
				action_deactivate();;
				break;
			case 2:
				// Ignore the selection request
				book_->selection(book_->item_number(g_net_entry_->qso_number()));
				break;
			}
		}
		else {
			// Switch to the selected QSO as part of the net if necessary
			if (log_num != g_net_entry_->qso_number()) {
				g_net_entry_->select_qso(log_num);
			}
			g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			g_misc_->qso(current_qso(), current_number());
			g_misc_->enable_widgets();
		}
		break;
	default:
		break;
	}
}

// Update query
void qso_data::update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num) {
	switch (logging_state_) {
	case QSO_PENDING:
	case QSO_INACTIVE:
	case QSO_VIEW:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QUERY_DUPE:
	case QUERY_SWL:
	case MANUAL_ENTRY:
		action_query(query, match_num, query_num);
		break;
	case QSO_EDIT:
		status_->misc_status(ST_WARNING, "DASH: Query received while editing QSO. Existing edit saved.");
		action_save_edit();
		action_query(query, match_num, query_num);
		break;
	default:
		status_->misc_status(ST_ERROR, "DASH: Query received when not expected. Query ignored.");
		break;
	}
}

// Update modem QSO
record* qso_data::start_modem_qso(string call, qso_init_t source) {
	bool allow_modem = true;
	bool cancelled = false;
	switch (logging_state_) {
	case QSO_PENDING:
		action_deactivate();
		// drop down
	case QSO_INACTIVE:
		break;	

	case QSO_EDIT:
		if (book_->is_dirty_record(g_entry_->qso())) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while editing a record", "Save edit", "Cancel edit", "Ignore")) {
			case 0:
				// Save QSO
				action_save_edit();
				break;
			case 1:
				// Cancel QSO
				action_cancel_edit();
				break;
			case 2:
				// Ignore the modem request
				allow_modem = false;
				break;
			}
		} else {
			action_cancel_edit();
		}
		break;

	case QSO_VIEW:
		action_cancel_edit();
		break;

	case QSO_WSJTX:
	case QSO_FLDIGI:
	// We are altready displaying a modem QSO
		cancelled = true;
		book_->enable_save(false, "Prevent save ehen cancelling to create a new QSO");
		action_cancel_modem();
		break;

	default:
		status_->misc_status(ST_ERROR, "DASH: Getting a modem update when not expected");
		return nullptr;
	}
	if (allow_modem) {
		action_activate(source);
		action_start(source);
		if (cancelled) book_->enable_save(true, "Remove temporary block");
		current_qso()->item("CALL", call);
		return current_qso();
	} else {
		return nullptr;
	}
}

// Update modem QSO
void qso_data::update_modem_qso(bool log_it) {
	switch (logging_state_) {
	case QSO_WSJTX:
	case QSO_FLDIGI: {
		if (log_it) {
			action_log_modem();
			action_deactivate();
			action_view();
		} else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			g_misc_->qso(current_qso(), current_number());
			tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, g_entry_->qso_number());
			enable_widgets();
		}
		break;
	}
	default: {
		break;
	}
	}
}

// Enter modem QSO
void qso_data::enter_modem_qso(record* qso) {
	switch (logging_state_) {
		case QSO_INACTIVE:
		case QSO_PENDING: {
			g_entry_->qso(qso);
			g_entry_->copy_default_to_qso();
			g_entry_->append_qso();
			logging_state_= QSO_EDIT;
			g_entry_->update_rig();
			g_entry_->copy_cat_to_qso();
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			g_misc_->qso(current_qso(), current_number());
			action_save_edit();
			tabbed_forms_->update_views(nullptr, HT_INSERTED, g_entry_->qso_number());
			enable_widgets();
		}
		default:
			break;
	}
}

// Save the settings
void qso_data::save_values() {
}

string qso_data::get_defined_fields() {
	return g_entry_->get_defined_fields();
}

// Get default record to copy
qso_num_t qso_data::get_default_number() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QSO_VIEW:
	case MANUAL_ENTRY:
		return book_->selection();
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_ENTER:
	case QSO_EDIT:
	case TEST_PENDING:
	case TEST_ACTIVE:
		return g_entry_->qso_number();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
		return g_query_->qso_number();
	case NET_EDIT:
	case NET_STARTED:
		return g_net_entry_->qso_number();
	default:
		return -1;
	}
}

// Action - create a new QSL in the appropriate copy of qso_entry
void qso_data::action_new_qso(record* qso, qso_init_t mode) {
	qso_entry* qe;
	switch (logging_state_) {
	case NET_STARTED:
	case NET_EDIT:
	case NET_ADDING:
		qe = g_net_entry_->entry();
		break;
	case MANUAL_ENTRY:
		qe = g_qy_entry_;
		break;
	default:
		qe = g_entry_;
		break;
	}
	rig_if* rig = ((qso_manager*)parent())->rig();
	qso_init_t new_mode = (mode == QSO_AS_WAS) ? previous_mode_ : mode;
	// Check fields
	qe->initialise_fields();
	switch (new_mode) {
	case QSO_NONE:
		// Just copy the station details
		qe->copy_qso_to_qso(qso, qso_entry::CF_COPY);
		g_misc_->qso(current_qso(), current_number());
		break;
	case QSO_ON_AIR:
		// Copy station details and get read rig details for band etc. 
		// If rig not connected use same as original
		if (rig && rig->is_good()) {
			qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC);
			qe->copy_cat_to_qso();
		}
		else {
			qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		}
		qe->copy_clock_to_qso();
		if (in_contest()) qe->copy_contest_to_qso();
		g_misc_->qso(current_qso(), current_number());
		break;
	case QSO_COPY_CONDX:
		// Clone the QSO - get station and band from original QSO
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		qe->copy_clock_to_qso();
		g_misc_->qso(current_qso(), current_number());
		break;
	case QSO_COPY_CALL:
		// Copy the QSO - as abobe but also same callsign and details
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT | qso_entry::CF_CONTACT);
		qe->copy_clock_to_qso();
		g_misc_->qso(current_qso(), current_number());
		break;
	case QSO_COPY_FOR_NET:
		// Clone the QSO - get time, station and band from original QSO
		qe->copy_qso_to_qso(qso, qso_entry::CF_TIME | qso_entry::CF_DATE | qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		qe->copy_clock_to_qso();
		g_misc_->qso(current_qso(), current_number());
		break;
	case QSO_COPY_WSJTX:
	case QSO_COPY_FLDIGI:
		// Set QSO and Update the display
		qe->qso(-1);
		qe->copy_default_to_qso();
		qe->copy_clock_to_qso();
		qe->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		g_misc_->qso(current_qso(), current_number());
		break;
	default:
		break;
	}
	qe->set_focus_call();
	previous_mode_ = new_mode;
}


// Action ACTIVATE: transition from QSO_INACTIVE to QSO_PENDING
void qso_data::action_activate(qso_init_t mode) {
	record* source_record = book_->get_record();
	if (contest()->contest_active()) logging_state_ = TEST_PENDING;
	else logging_state_ = QSO_PENDING;
	action_new_qso(source_record, mode);
	update_rig();
}

// Action START - transition from QSO_PENDING to QSO_STARTED
void qso_data::action_start(qso_init_t mode) {
	g_entry_->append_qso();
	book_->new_record(true);
	book_->enable_save(false, "Starting real-time QSO");
	book_->selection(book_->item_number(g_entry_->qso_number()), HT_INSERTED);
	switch (mode) {
	case QSO_NONE:
		logging_state_ = QSO_ENTER;
		break;
	case QSO_COPY_WSJTX:
		logging_state_ = QSO_WSJTX;
		break;
	case QSO_COPY_FLDIGI:
		logging_state_ = QSO_FLDIGI;
		break;
	default:
		if (in_contest()) logging_state_ = TEST_ACTIVE;
		else logging_state_ = QSO_STARTED;
		break;
	}
	enable_widgets();
}

// Action SAVE - transition from QSO_STARTED to QSO_INACTIVE while saving record
// Continuing - the QSO will be left open for logging to complete
bool qso_data::action_save(bool continuing) {
	record* qso = nullptr;
	item_num_t item_number = -1;
	qso_num_t qso_number = -1;
	switch (logging_state_) {
	case QSO_STARTED:
	case QSO_WSJTX:
	case QSO_FLDIGI:
	case QSO_ENTER:
		item_number = book_->item_number(g_entry_->qso_number());
		qso = g_entry_->qso();
		qso_number = g_entry_->qso_number();
		break;
	case NET_STARTED:
		item_number = book_->item_number(g_net_entry_->qso_number());
		qso = g_net_entry_->qso();
		qso_number = g_net_entry_->qso_number();
		break;
	case TEST_ACTIVE:
		item_number = book_->item_number(g_entry_->qso_number());
		qso = g_entry_->qso();
		qso_number = g_entry_->qso_number();
		contest()->increment_serial();
		break;
	default:
		break;
	}
	if (!qso->is_valid()) {
		fl_alert("QSO has necessary fields (CALL, TIME ON and FREQ) missing");
		return false;
	}
	// On-air logging add date/time off
	switch (previous_mode_) {
	case QSO_ON_AIR:
		// All on-air modes - set cime-off to now
		if (qso->item("TIME_OFF") == "") {
			// Add end date/time - current time of interactive entering
			// Get current date and time in UTC
			string timestamp = now(false, "%Y%m%d%H%M%S");
			qso->item("QSO_DATE_OFF", timestamp.substr(0, 8));
			// Time as HHMMSS - always log seconds.
			qso->item("TIME_OFF", timestamp.substr(8));
		}
		qso->item("QSO_COMPLETE", string(""));
		break;
	case QSO_COPY_CALL:
	case QSO_COPY_CONDX:
	case QSO_COPY_FOR_NET:
		// All on-air modes - set cime-off to now
		if (qso->item("TIME_OFF") == "") {
			// Add end date/time - current time of interactive entering
			// Get current date and time in UTC
			string timestamp = now(false, "%Y%m%d%H%M%S");
			qso->item("QSO_DATE_OFF", timestamp.substr(0, 8));
			// Time as HHMMSS - always log seconds.
			qso->item("TIME_OFF", timestamp.substr(8));
		}
		qso->item("QSO_COMPLETE", string(""));
		// Put the record in its correct position and save that position
		item_number = book_->correct_record_position(item_number);
		qso_number = book_->record_number(item_number);
		break;
	case QSO_NONE:
		qso->item("QSO_COMPLETE", string(""));
		// Deliberately drop through
	case QSO_COPY_WSJTX:
	case QSO_COPY_FLDIGI:
		// Put the record in its correct position and save that position
		item_number = book_->correct_record_position(item_number);
		qso_number = book_->record_number(item_number);
		break;
	default:
		break;
	}

	if (!continuing) {

		// check whether record has changed - when parsed
		if (cty_data_->update_qso(qso)) {
		}

		// check whether record has changed - when validated
		if (spec_data_->validate(qso, item_number)) {
		}

		book_->add_use_data(qso);
		extract_records_->check_add_record(qso_number);

		book_->enable_save(true, "Saving QSO");

		// Upload QSO to QSL servers
		book_->upload_qso(qso_number);
	}

	switch (logging_state_) {
	case QSO_STARTED:
	case QSO_ENTER:
		logging_state_ = SWITCHING;
		book_->selection(item_number, HT_INSERTED);
		action_deactivate();;
		action_activate(previous_mode_);
		break;
	case TEST_ACTIVE:
		logging_state_ = SWITCHING;
		book_->selection(item_number, HT_INSERTED);
		logging_state_ = TEST_PENDING;
		action_activate(previous_mode_);
		break;
	case QSO_WSJTX:
	case QSO_FLDIGI:
		logging_state_ = SWITCHING;
		book_->selection(item_number, HT_INSERTED);
		action_deactivate();;
		break;
	case NET_STARTED:
		g_net_entry_->remove_entry();
		logging_state_ = SWITCHING;
		if (g_net_entry_->entries() == 0) {
			book_->selection(item_number, HT_INSERTED);
			action_deactivate();;
		}
		else {
			book_->selection(g_net_entry_->qso_number(), HT_INSERTED_NODXA);
			logging_state_ = NET_STARTED;
		}
		break;
	default:
		break;
	}
	enable_widgets();
	return true;
}

// Action CANCEL - Transition from QSO_STARTED to QSO_INACTIVE without saving record
void qso_data::action_cancel() {
	logging_state_t saved_state = logging_state_;
	action_deactivate();;
	book_->delete_record(true);
	logging_state_ = saved_state;

	switch (logging_state_) {
	case QSO_STARTED:
		g_entry_->delete_qso();
		action_deactivate();;
		break;
	case TEST_ACTIVE:
		g_entry_->delete_qso();
		logging_state_ = TEST_PENDING;
		break;
	case QSO_WSJTX:
	case QSO_FLDIGI:
		action_deactivate();;
		break;
	case NET_STARTED:
		g_net_entry_->remove_entry();
		if (g_net_entry_->entries() == 0) {
			action_return_state();
		}
		break;
	default:
		break;
	}
	enable_widgets();
	book_->enable_save(true, "Canceling real-time QSO");
}

// Action DELETE - we should be inactive but leave this code 
void qso_data::action_delete_qso() {
	logging_state_t saved_state = logging_state_;
	action_deactivate();;
	book_->delete_record(true);
	action_activate(QSO_NONE);

	// Now restore the original state
	switch (saved_state) {
	case QSO_INACTIVE:
		action_deactivate();
		break;
	case QSO_PENDING:
		break;
	default:
		break;
	}

	enable_widgets();
}

// Action DEACTIVATE - Transition from QSO_PENDING to QSO_INACTIVE
void qso_data::action_deactivate() {
	switch(logging_state_) {
	case QSO_INACTIVE: 
		// do nothing
		break;
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_EDIT:
	case QSO_ENTER:
	case QSO_VIEW:
	case QSO_WSJTX:
	case QSO_FLDIGI:
	case TEST_PENDING:
	case TEST_ACTIVE:
		g_entry_->delete_qso();
		break;
	case QSO_BROWSE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QUERY_DUPE:
		g_query_->clear_query();
		break;
	case MANUAL_ENTRY:
		g_qy_entry_->delete_qso();
		break;
	case NET_STARTED:
	case NET_EDIT:
	case NET_ADDING:
		while (g_net_entry_->entries()) {
			g_net_entry_->remove_entry();
		}
		//// Restore holding entry
		//g_net_entry_->add_entry();
		break;
	default:
		break;
	}
	logging_state_ = QSO_INACTIVE;
	g_misc_->qso(nullptr, -1);
	enable_widgets();
}

// Action EDIT - Transition from QSO_INACTIVE to QSO_EDIT
void qso_data::action_edit() {
	// Save a copy of the current record
	qso_num_t qso_number = get_default_number();
	g_entry_->qso(qso_number);
	record* qso = g_entry_->qso();
	// If the QSO looks incomplete ask if it is so and restart it else edit it
	if (qso && qso->item("TIME_OFF") == "" && logging_state_ == QSO_STARTED) {
		time_t now = time(nullptr);
		time_t qso_start = qso->timestamp();
		if (difftime(now, qso_start) < 1800.0) {
			if (fl_choice("It looks as if the QSO you want to edit is still active - do you want to continue?", "Yes", "No", nullptr) == 0) {
				logging_state_ = QSO_STARTED;
			} else {
				logging_state_ = QSO_EDIT;
			}
		}
	} else {
		logging_state_ = QSO_EDIT;
	}
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
}

// Action VIEW - Transition from QSO_INACTIVE to QSO_VIEW
void qso_data::action_view(qso_num_t number) {
	// Save a copy of the current record
	qso_num_t qso_number = number == -1 ? get_default_number() : number;
	logging_state_ = QSO_VIEW;
	g_entry_->qso(qso_number);
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
}

// Action SAVE EDIT - Transition from QSO_EDIT to QSO_INACTIVE while saving changes
void qso_data::action_save_edit() {
	// We no longer need to maintain the copy of the original QSO
	record* qso = g_entry_->qso();
	if (qso) {
		qso_num_t qso_number = g_entry_->qso_number();
		item_num_t item_number = book_->item_number(qso_number);
		book_->upload_qso(qso_number);
		cty_data_->update_qso(qso);
		spec_data_->validate(qso, item_number);
		book_->add_use_data(qso);
		book_->enable_save(true, "Saving edit");

		g_entry_->delete_qso();
	}
	((qso_manager*)parent())->enable_widgets();
	action_return_state();
	enable_widgets();
}

// ACtion CANCEL EDIT - Transition from QSO_EDIT to QSO_INACTIVE scrapping changes
void qso_data::action_cancel_edit() {
	// Copy original back to the book
	if (logging_state_ == QSO_EDIT) {
		if (g_entry_->original_qso()) {
			*book_->get_record(g_entry_->qso_number(), false) = *g_entry_->original_qso();
		}
	}
	g_entry_->delete_qso();
	action_return_state();
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
}

// Action CANCEL in BROWSE 
void qso_data::action_cancel_browse() {
	g_entry_->qso(g_query_->qso_number());
	g_query_->clear_query();
	action_return_state();
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
}

// Action navigate button
void qso_data::action_navigate(int target) {
	inhibit_drawing_ = true;
	switch (logging_state_) {
	case QSO_EDIT:
		// Save, navigate to new QSO and open editor
		action_save_edit();
		navigation_book_->navigate((navigate_t)target);
		action_edit();
		break;
	case QSO_VIEW:
		// Save, navigate to new QSO and open editor
		action_cancel_edit();
		navigation_book_->navigate((navigate_t)target);
		action_view();
		break;
	case QSO_PENDING:
		// Deactivate, navigate and go pending again
		action_deactivate();
		navigation_book_->navigate((navigate_t)target);
		action_activate(previous_mode_);
		break;
	case QSO_BROWSE:
		// Close browser, navigate and reopen browser
		action_cancel_browse();
		navigation_book_->navigate((navigate_t)target);
		action_browse();
		break;
	case QUERY_MATCH:
		// Navigate book to new compare record and reopen query
		book_->navigate((navigate_t)target);
		action_query(logging_state_, book_->selection(), 0);
		break;
	case NET_STARTED:
	case NET_EDIT: {
		// "Navigate" - select left anf right of tabs
		g_net_entry_->navigate((navigate_t) target);
		break;
	}
	default:
		break;
	}
	inhibit_drawing_ = false;
	enable_widgets();
}

// Action browse
void qso_data::action_browse() {
	qso_num_t qso_number = get_default_number();
	logging_state_ = QSO_BROWSE;
	g_query_->set_query("Browsing record",qso_number);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
}

// Action query
void qso_data::action_query(logging_state_t query, qso_num_t match_number, qso_num_t query_number) {
	switch (query) {
	case QUERY_MATCH:
		g_query_->set_query(import_data_->match_question(), match_number, import_data_->get_record(query_number, false));
		break;
	case QUERY_NEW:
		potential_match_ = match_number;
		query_number_ = query_number;
		g_query_->set_query(import_data_->match_question(), -1, import_data_->get_record(query_number, false));
		break;
	case QUERY_SWL:
		g_query_->set_query(import_data_->match_question(), match_number, import_data_->get_record(query_number, false));
		break;
	case QUERY_DUPE:
		// Note record numbers relate to book even if it is extracted data that refered the dupe check
		g_query_->set_query(navigation_book_->match_question(), match_number, book_->get_record(query_number, false), false);
		break;
	case QRZ_MERGE: {
		// We are using selected record and merge data accordingly
		bool ok = true;
		if (!qrz_handler_->has_session()) {
			// Try and open an XML Database session
			ok = qrz_handler_->open_session();
		}
		if (ok) {
			// Access it
			if (match_number == -1) {
				ok = qrz_handler_->fetch_details(current_qso());
			}
			else {
				ok = qrz_handler_->fetch_details(book_->get_record(match_number, false));
			}
			if (ok) g_query_->set_query(qrz_handler_->get_merge_message(), match_number, qrz_handler_->get_record());
		}
		if (!ok) {
			// Fall-back to the web-page interface
			qrz_handler_->open_web_page(book_->get_record(match_number, false)->item("CALL"));
		}
		break;
	}
	case QRZ_COPY: {
		// We are using selected record and merge data accordingly
		bool ok = true;
		if (!qrz_handler_->has_session()) {
			// Try and open an XML Database session
			ok = qrz_handler_->open_session();
		}
		if (ok) {
			ok = qrz_handler_->fetch_details(current_qso());
			if (ok) g_query_->set_query(qrz_handler_->get_merge_message(), current_qso(), qrz_handler_->get_record());
		}
		if (!ok) {
			// Fall-back to the web-page interface
			qrz_handler_->open_web_page(current_qso()->item("CALL"));
		}
		break;
	}
	default:
		// TODO trap this sensibly
		return;
	}
	logging_state_ = query;
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
	g_buttons_->enable_widgets();
	// Move window to top
	// parent()->show();
}

// Action add query - add query QSO to book
void qso_data::action_add_query() {
	import_data_->save_update();
	g_query_->clear_query();
	action_deactivate();;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();

}

// Action reject query - do nothing
void qso_data::action_reject_query() {
	import_data_->discard_update(true);
	g_query_->clear_query();
	action_deactivate();;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// Action reject manual query - do nothing
void qso_data::action_reject_manual() {
	g_query_->clear_query();
	action_deactivate();;
	enable_widgets();
}

// Action merge query
void qso_data::action_merge_query() {
	import_data_->merge_update();
	g_query_->clear_query();
	action_deactivate();;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// ACtion find match
void qso_data::action_find_match() {
	update_query(QUERY_MATCH, potential_match_, query_number_);
}

// Action handle dupe
void qso_data::action_handle_dupe(dupe_flags action) {
	switch (action) {
	case DF_1:
		// Discard the queried possible duplicate
		navigation_book_->reject_dupe(false);
		break;
	case DF_2:
		// Keep the queried possible duplicate record and discard the original record
		navigation_book_->reject_dupe(true);
		break;
	case DF_MERGE:
		// Discard the queried duplicate record after merging the two records
		navigation_book_->merge_dupe();
		break;
	case DF_BOTH:
		// Queried duplicate record is not a duplicate, keep both it and the original record
		navigation_book_->accept_dupe();
		break;
	}
	g_query_->clear_query();
	action_deactivate();;
	enable_widgets();
	// Restart the duplicate check process
	navigation_book_->check_dupes(true);

}

// Action save as a result of a merge
void qso_data::action_save_merge() {
	// We no longer need to maintain the copy of the original QSO
	switch (logging_state_) {
	case QRZ_MERGE: {
		book_->add_use_data(g_query_->qso());
		g_query_->clear_query();
		logging_state_ = interrupted_state_;
		enable_widgets();
		book_->selection(-1, HT_MINOR_CHANGE);
		break;
	}
	case QRZ_COPY: {
		g_query_->clear_query();
		logging_state_ = interrupted_state_;

		enable_widgets();
		g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		break;
	}
	default:
		break;
	}
}

// ACtion look in ALL.TXT
void qso_data::action_look_all_txt() {
	switch(logging_state_) {
		case QUERY_NEW:
		case QUERY_MATCH:
			if (wsjtx_handler_->match_all_txt(g_query_->query_qso(), true)) {
				logging_state_ = QUERY_WSJTX;
			}
			break;
		case MANUAL_ENTRY:
			if (wsjtx_handler_->match_all_txt(g_qy_entry_->qso(), false)) {
				status_->misc_status(ST_NOTE, "DASH: ALL.TXT search found QSO: see above");
			} else {
				status_->misc_status(ST_WARNING, "DASH: ALL.TXT search did not find QSO");
			}
			break;
		case QSO_VIEW:
			if (wsjtx_handler_->match_all_txt(g_entry_->qso(), false)) {
				status_->misc_status(ST_NOTE, "DASH: ALL.TXT search found QSO: see above");
			} else {
				status_->misc_status(ST_WARNING, "DASH: ALL.TXT search did not find QSO");
			}
			break;
		default:
			break;
	}
	enable_widgets();
}

// Create a net from current QSO and others which overlap
void qso_data::action_create_net() {
	qso_num_t qso_number = g_entry_->qso_number();
	record* qso = g_entry_->qso();
	string call = get_call();
	char msg[128];
	g_net_entry_->set_qso(qso_number);
	qso_entry* w = g_net_entry_->entry();
	switch (logging_state_) {
	case QSO_STARTED:
		w->copy_cat_to_qso();
		logging_state_ = NET_STARTED;
		break;
	case QSO_EDIT:
		logging_state_ = NET_EDIT;
		break;
	default:
		snprintf(msg, sizeof(msg), "DASH: Trying to create a net for %s when neither started nor editing", call.c_str());
		status_->misc_status(ST_SEVERE, msg);
		break;
	}
	g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	g_misc_->qso(current_qso(), current_number());
	// Now add the remaining QSOs in the net - first look earlier
	qso_num_t other_number = qso_number - 1;
	while (qso->match_records(book_->get_record(other_number, false)) == MT_OVERLAP) {
		book_->enable_save(false, "Adding QSO to net");
		g_net_entry_->add_entry();
		g_net_entry_->set_qso(other_number);
		g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		// g_misc_->qso(current_qso(), current_number());
		other_number--;
	}
	// Now look later
	other_number = qso_number + 1;
	while (qso->match_records(book_->get_record(other_number, false)) == MT_OVERLAP) {
		book_->enable_save(false, "Adding QSO to net");
		g_net_entry_->add_entry();
		g_net_entry_->set_qso(other_number);
		g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		// g_misc_->qso(current_qso(), current_number());
		other_number++;
	}
	g_net_entry_->entry(w);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
	g_net_entry_->set_focus_call();
}
	

// Add a QSO to the net - copy existing qso start times or not
void qso_data::action_add_net_qso() {
	record* qso = g_net_entry_->qso();
	// Create the entry tab
	g_net_entry_->add_entry();
	// Create the new QSO therein
	switch (logging_state_) {
	case NET_STARTED:
		logging_state_ = NET_ADDING;
		action_new_qso(qso, QSO_ON_AIR);
		logging_state_ = NET_STARTED;
		break;
	case NET_EDIT:
		action_new_qso(qso, QSO_COPY_FOR_NET);
		break;
	default:
		break;
	}
	// Add it to the book
	book_->enable_save(false, "Adding a QSO to a net");
	g_net_entry_->append_qso();
	book_->selection(book_->item_number(g_net_entry_->qso_number()), HT_INSERTED);
	enable_widgets();
}

// Save the whole net
void qso_data::action_save_net_all() {
	// Only save the book once all records have been saved
	// Uplaod the QSOs after all have been saved
	book_->enable_save(false, "Starting multi-QSO save");
	book_->allow_upload(false);
	bool ok = true;
	extract_data* upload_qsos = new extract_data;
	upload_qsos->use_mode(extract_data::ALL);
	while (g_net_entry_->entries() && ok) {
		switch (logging_state_) {
		case NET_STARTED:
			// SAving record number for later upload
			upload_qsos->add_record(g_net_entry_->qso_number());
			ok = action_save(false);
			break;
		case NET_EDIT:
			action_save_net_edit();
			break;
		default:
			break;
		}
	}
	// Restore the place-holder entry
	if (ok) g_net_entry_->add_entry();
	book_->enable_save(true, "Finished multi-QSO save");
	book_->allow_upload(true);
	if (upload_qsos->size()) upload_qsos->upload();
	enable_widgets();
}

// Save a QSO in NET_EDIT
void qso_data::action_save_net_edit() {
	// We no longer need to maintain the copy of the original QSO
	book_->add_use_data(g_net_entry_->qso());
	book_->enable_save(true, "Saving net edit QSO");
	g_net_entry_->remove_entry();
	if (g_net_entry_->entries() == 0) {
		// Do NOT use deactivate()
		logging_state_ = QSO_INACTIVE;
	}
	else {
		logging_state_ = NET_EDIT;
	}
	enable_widgets();
}

// Cancel the whole net
void qso_data::action_cancel_net_all() {
	book_->enable_save(false, "Starting multi-QSO cancel");
	while (g_net_entry_->entries()) {
		switch(logging_state_) {
		case NET_STARTED: 
			action_cancel();
			break;
		case NET_EDIT:
			action_cancel_net_edit();
			break;
		default:
			break;
		}
	}
	book_->enable_save(true, "Finished multi-QSO cancel");
	// Restore the place-holder entry
	g_net_entry_->add_entry();
	enable_widgets();
}

// Cancel an individual QSO in net edit
void qso_data::action_cancel_net_edit() {
	// Copy original back to the book
	*book_->get_record(g_net_entry_->qso_number(), false) = *g_net_entry_->original_qso();
	g_net_entry_->remove_entry();
	if (g_net_entry_->entries() == 0) {
		// do NOT use deactivate() - that sets 
		logging_state_ = QSO_INACTIVE;
	}
	else {
		g_net_entry_->entry()->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		g_misc_->qso(current_qso(), current_number());
		logging_state_ = NET_EDIT;
	}
	book_->enable_save(true, "Cancelled multi-QSO edit");
	enable_widgets();
}

// Start a modem record
void qso_data::action_log_modem() {
	// Add to book
	book_->enable_save(false, "Logging real-time modem QSO");
	record* qso = current_qso();
	rig_if* rig = ((qso_manager*)parent())->rig();
	// Do a sanity check on the rig
	double modem_freq;
	qso->item("FREQ", modem_freq);
	double rig_freq = rig->get_dfrequency(true);
	if (abs(modem_freq - rig_freq) > 0.010) {
		fl_message("Check which rig is selected as frequency from rig is different fro that being logged!");
		fl_beep(FL_BEEP_ERROR);
	} 
	int modem_pwr;
	qso->item("TX_PWR", modem_pwr);
	if (modem_pwr == 0) {
		// Get power from rig
		qso->item("TX_PWR", rig->get_tx_power(true));
	}
	// The QSO is complete
	action_save(false);
	
	book_->selection(book_->item_number(g_entry_->qso_number()), HT_INSERTED);
	book_->enable_save(true, "Logged real-time modem QSO");
	enable_widgets();
}

// Cacncel a modem record
void qso_data::action_cancel_modem() {
	switch(logging_state_) {
		case QSO_WSJTX: {
			wsjtx_handler_->delete_qso(current_qso()->item("CALL"));
			action_deactivate();
			g_entry_->delete_qso();
			book_->delete_record(true);
			book_->enable_save(true, "Cancel real-time modem QSO");
			break;
		}
		case QSO_FLDIGI: {
			action_deactivate();
			g_entry_->delete_qso();
			book_->delete_record(true);
			book_->enable_save(true, "Cancel real-time modem QSO");
			break;
		}
		default:
			break;
	}
	enable_widgets();
}

// Create a query entry
void qso_data::action_query_entry() {
	g_qy_entry_->qso(-1);
	logging_state_ = MANUAL_ENTRY;
	g_qy_entry_->copy_default_to_qso();
	g_qy_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	g_misc_->qso(current_qso(), current_number());
	enable_widgets();
}

// Execute the query
void qso_data::action_exec_query() {
	record* qso = g_qy_entry_->qso();
	extract_records_->clear_criteria();
	qso->update_band();
	// First search callsign
	if (qso->item("CALL").length()) {
		extract_records_->extract_field("CALL", qso->item("CALL"), false);
	}
	if (qso->item("BAND").length()) {
		extract_records_->extract_field("BAND", qso->item("BAND"), true);
	}
	if (qso->item("MODE").length()) {
		extract_records_->extract_field("MODE", qso->item("MODE"), true);
	}
	char msg[128];
	if (extract_records_->size()) {
		snprintf(msg, sizeof(msg), "DASH: %zu matching records found", extract_records_->size());
		status_->misc_status(ST_NOTE, msg);
		// Display the first record
		book_->selection(extract_records_->record_number(0), HT_SELECTED);
		action_edit();
	}
	else {
		snprintf(msg, sizeof(msg), "DASH: No matching records found - try import");
		status_->misc_status(ST_ERROR, msg);
		enable_widgets();
	}
}

// Abandon the query
void qso_data::action_cancel_query() {
	g_qy_entry_->delete_qso();
	action_deactivate();;
	enable_widgets();
}

// Import the query
void qso_data::action_import_query() {
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();
	import_data_->load_record(g_qy_entry_->qso());
	// If completed OK go to manual entry again
	if (import_data_->update_complete()) {
		action_query_entry();
	}
}

// Open QRZ.com page
void qso_data::action_qrz_com() {
	interrupted_state_ = logging_state_;
	switch(logging_state_) {
	case qso_data::QSO_PENDING:
	case qso_data::TEST_PENDING:
	case qso_data::QSO_WSJTX: {
		qso_num_t qso_n = current_number();
		action_query(QRZ_COPY, qso_n, -1);
		break;
	}
	case qso_data::QSO_STARTED:
	case qso_data::QSO_EDIT:
	case qso_data::NET_EDIT:
	case qso_data::NET_STARTED: {
		qso_num_t qso_n = current_number();
		action_query(QRZ_MERGE, qso_n, -1);
		break;
	}
	case qso_data::QSO_VIEW:
	case qso_data::QSO_BROWSE: {
		qrz_handler_->open_web_page(current_qso()->item("CALL"));
		break;
	}
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW: {
		qrz_handler_->open_web_page(query_qso()->item("CALL"));
		break;
	}
	default:
		break;
	} 
}

// Action update CAT
void qso_data::action_update_cat(bool clear) {
	switch (logging_state_) {
		case QSO_EDIT: {
			g_entry_->copy_cat_to_qso(clear);
			g_entry_->copy_qso_to_display(qso_entry::CF_CAT);
			break;
		}
		case NET_EDIT: {
			g_net_entry_->entry()->copy_cat_to_qso(clear);
			g_net_entry_->entry()->copy_qso_to_display(qso_entry::CF_CAT);
			break;
		}
		default:
			break;
	}
}

// Remember whether inactive or pending before an edit or view
void qso_data::action_remember_state() {
	switch (logging_state_) {
		case QSO_INACTIVE:
		case QSO_PENDING:
		case MANUAL_ENTRY:
			edit_return_state_ = logging_state_;
			break;
		default:
			break;
	}
}

// REturn to remembered state
void qso_data::action_return_state() {
	action_deactivate();
	switch (edit_return_state_) {
		case QSO_INACTIVE:
			break;
		case QSO_PENDING:
			action_activate(previous_mode_);
			break;
		case MANUAL_ENTRY:
			action_query_entry();
			break;
		default:
			break;
	}
}

// Parse the qso
void qso_data::action_parse_qso() {
	record* qso = current_qso();
	// command parsing enabled - parse record
	bool parse_result = cty_data_->update_qso(qso);
	// update band
	bool changed = qso->update_band(true);
	if (changed || parse_result) {
		g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	}
	enable_widgets();
}

// Dummy QSO
record* qso_data::dummy_qso() {
	record* dummy = new record;
	string timestamp = now(false, "%Y%m%d%H%M%S");
	// Get current date and time in UTC
	dummy->item("QSO_DATE", timestamp.substr(0, 8));
	// Time as HHMMSS - always log seconds.
	dummy->item("TIME_ON", timestamp.substr(8));
	dummy->item("QSO_DATE_OFF", string(""));
	dummy->item("TIME_OFF", string(""));
	dummy->item("CALL", string(""));
	// otherwise leave blank so that we enter it manually later.
	dummy->item("FREQ", string(""));
	dummy->item("FREQ_RX", string(""));
	dummy->item("MODE", string(""));
	dummy->item("SUBMODE", string(""));
	dummy->item("TX_PWR", string(""));
	// initialise fields
	dummy->item("RX_PWR", string(""));
	dummy->item("RST_SENT", string(""));
	dummy->item("RST_RCVD", string(""));
	dummy->item("NAME", string(""));
	dummy->item("QTH", string(""));
	dummy->item("GRIDSQUARE", string(""));

	return dummy;
}

// Update QSO with info from rig
void qso_data::update_rig() {
	// Get freq etc from QSO or rig
// Get present values data from rig
	if (logging_state_ == QSO_PENDING) {
		rig_if* rig = ((qso_manager*)parent())->rig();
		if (!rig || rig->is_good() || rig->has_no_cat()) {
			g_entry_->update_rig();
			g_entry_->copy_cat_to_qso();
		}
	}
	enable_widgets();
}

// Start a QSO as long as we are in the correct state
void qso_data::start_qso(qso_init_t mode) {
	switch (logging_state_) {
	case qso_data::QSO_INACTIVE:
		action_activate(mode);
		// drop through
	case qso_data::QSO_PENDING:
		action_start(mode);
		break;
	case qso_data::QSO_STARTED:
	case NET_STARTED:
		status_->misc_status(ST_ERROR, "DASH: Cannot start a QSO when one already started");
		break;
	case qso_data::QSO_EDIT:
	case NET_EDIT:
		status_->misc_status(ST_ERROR, "DASH: Cannot start a QSO while editing an existing one");
		break;
	case QSO_ENTER:
		status_->misc_status(ST_ERROR, "DASH: Cannot start a QSO when enterrring another");
		break;
	default:
		break;
	}
}

// End a QSO as long as we are in the correct state
void qso_data::end_qso() {
	switch (logging_state_) {
	case qso_data::QSO_INACTIVE:
		status_->misc_status(ST_ERROR, "DASH: Cannot end a QSO that hasn't been started");
		break;
	case qso_data::QSO_PENDING:
		action_start(previous_mode_);
		// drop through
	case qso_data::QSO_STARTED:
	case qso_data::QSO_ENTER:
		action_save(false);
		break;
	case qso_data::QSO_EDIT:
		action_save_edit();
		break;
	case NET_STARTED:
	case NET_EDIT:
		action_save_net_all();
		break;
	default:
		break;
	}
}

// Edit a QSO as long as we are in the correct state
void qso_data::edit_qso() {
	switch (logging_state_) {
	case qso_data::QSO_INACTIVE:
		action_edit();
		break;
	case qso_data::QSO_PENDING:
	case qso_data::QSO_STARTED:
	case NET_STARTED:
		status_->misc_status(ST_ERROR, "DASH: Cannot edit a QSO when in on-air");
		break;
	case qso_data::QSO_EDIT:
	case NET_EDIT:
	case QSO_ENTER:
		status_->misc_status(ST_ERROR, "DASH: Cannot edit another QSO while editing an existing one");
		break;
	default:
		break;
	}
}

// Get logging state
qso_data::logging_state_t qso_data::logging_state() {
	return logging_state_;
}

// Current QSO
record* qso_data::current_qso() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_ENTER:
	case QSO_EDIT:
	case QSO_VIEW:
	case QSO_WSJTX:
	case QSO_FLDIGI:
	case TEST_ACTIVE:
	case TEST_PENDING:
		return g_entry_->qso();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QRZ_MERGE:
	case QRZ_COPY:
		return g_query_->qso();
	case NET_STARTED:
	case NET_EDIT:
		return g_net_entry_->qso();
	case MANUAL_ENTRY:
		return g_qy_entry_->qso();
	default:
		return nullptr;
	}
}

// Current QSO
record* qso_data::query_qso() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_ENTER:
	case QSO_EDIT:
	case QSO_VIEW:
	case QSO_WSJTX:
	case QSO_FLDIGI:
	case TEST_ACTIVE:
	case TEST_PENDING:
	case NET_STARTED:
	case NET_EDIT:
	case MANUAL_ENTRY:
		return nullptr;
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QRZ_MERGE:
	case QRZ_COPY:
		return g_query_->query_qso();
	default:
		return nullptr;
	}
}

// Current QSO number
qso_num_t qso_data::current_number() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_ENTER:
	case QSO_EDIT:
	case QSO_VIEW:
	case QSO_WSJTX:
	case QSO_FLDIGI:
	case TEST_PENDING:
	case TEST_ACTIVE:
		return g_entry_->qso_number();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QRZ_MERGE:
	case QRZ_COPY:
		return g_query_->qso_number();
	case NET_STARTED:
	case NET_EDIT:
		return g_net_entry_->qso_number();
	case MANUAL_ENTRY:
		return g_qy_entry_->qso_number();
	default:
		return book_->selection();
	}
}

// Call in editor
string qso_data::get_call() {
	record* qso = current_qso();
	if (qso) {
		return qso->item("CALL");
	}
	else {
		return "";
	}
}

// The supplied QSO number is being edited
bool qso_data::qso_editing(qso_num_t number) {
	switch (logging_state_) {
	case QSO_EDIT:
		if (number == g_entry_->qso_number()) return true;
		else return false;
	case NET_EDIT:
	case NET_ADDING:
		return g_net_entry_->qso_in_net(number);
	default:
		return false;
	}
}

// We are not actively doing anything
bool qso_data::inactive() {
	switch(logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case TEST_PENDING:
	case QSO_VIEW:
		return true;
	default:
		return false;
	}
}

// Return the contest form
qso_contest* qso_data::contest() {
	return g_misc_->contest();
}

// Return contest active
bool qso_data::in_contest() {
	return contest()->contest_active();
}

// Return whether can navigate
bool qso_data::can_navigate(navigate_t target) {
	switch(logging_state_) {
		case QSO_EDIT:
		case QSO_VIEW:
		case QSO_PENDING:
		case QSO_BROWSE: {
			switch(target) {
				case NV_FIRST:
				case NV_PREV: {
					if (navigation_book_->selection() == 0) {
						return false;
					} else {
						return true;
					}
				}
				case NV_LAST:
				case NV_NEXT: {
					if (navigation_book_->selection() >= navigation_book_->size() - 1) {
						return false;
					} else {
						return true;
					}
				}
				default: return false;
			}
		}
		case QUERY_MATCH: {
			switch (target) {
				case NV_FIRST:
				case NV_PREV: {
					if (book_->selection() == 0) {
						return false;
					} else { 
						return true;
					}
				}
				case NV_LAST:
				case NV_NEXT: {
					if (book_->selection() >= book_->size() - 1) {
						return false;
					} else {
						return true;
					}
				}
				default: return false;
			}
		}
		case NET_EDIT:
		case NET_STARTED: {
			return g_net_entry_->can_navigate(target);
		}
		default: return false;
	}
}

// Get default station information
string qso_data::get_default_station(char item) {
	switch ((qso_manager::stn_item_t)item) {
	case qso_manager::CALLSIGN:
		return g_station_->current_call();
	case qso_manager::OP:
		return g_station_->current_oper();
	case qso_manager::QTH:
		return g_station_->current_qth();
	default:
		return "";
	}
}

// Update the QSO from the current station information
void qso_data::update_station_fields(record* qso) {
	record* ud_qso = (qso == nullptr) ? current_qso() : qso;
	g_station_->update_qso(ud_qso);
	return;
}

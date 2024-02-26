#include "qso_data.h"
#include "qso_manager.h"
#include "qth_dialog.h"
#include "settings.h"
#include "rig_if.h"
#include "book.h"
#include "status.h"
#include "extract_data.h"
#include "cty_data.h"
#include "spec_data.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "qrz_handler.h"
#include "wsjtx_handler.h"
#include "menu.h"
#include "utils.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tooltip.H>

extern Fl_Preferences* settings_;
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

// qso_group_
qso_data::qso_data(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
	, logging_state_(QSO_INACTIVE)
	, inhibit_drawing_(false)
	, previous_mode_(QSO_NONE)
{
	load_values();
}

// Destructor
qso_data::~qso_data() {
}

// Load values
void qso_data::load_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	// Read field settings
	Fl_Preferences field_settings(dash_settings, "Field Lists");
	int num_contests = field_settings.entries();
	for (int ix = 0; ix < num_contests; ix++) {
		string contest = field_settings.entry(ix);
		char* fields;
		field_settings.get(field_settings.entry(ix), fields, "");
		if (strlen(fields) > 0) {
			split_line(string(fields), 	qso_entry::field_map_[contest], ',');
		}
	}
	// Set logging mode -default is On-air with or without rig connection
	rig_if* rig = ((qso_manager*)parent())->rig();
	bool have_rig = rig && rig->is_good();
}

// Create qso_data
void qso_data::create_form(int X, int Y) {
	int max_x = X;

	begin();
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	label("QSO Data");
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);

	int curr_y = Y + HTEXT;
	int top = Y;
	int curr_x = X + GAP;

	// One or the other of the two groups below will be shown at a time
	g_entry_ = new qso_entry(curr_x, curr_y, 10, 10);
	g_entry_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_entry_->labelfont(FL_BOLD);
	g_entry_->labelsize(FL_NORMAL_SIZE + 2);
	g_entry_->labelcolor(FL_DARK_BLUE);

	max_x = max(max_x, g_entry_->x() + g_entry_->w());
	g_query_ = new qso_query(curr_x, curr_y, 10, 10);

	max_x = max(max_x, g_query_->x() + g_query_->w());

	g_net_entry_ = new qso_net_entry(curr_x, curr_y, 10, 10);
	max_x = max(max_x, g_net_entry_->x() + g_net_entry_->w());

	g_qy_entry_ = new qso_entry(curr_x, curr_y, 10, 10);
	g_qy_entry_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_qy_entry_->labelfont(FL_BOLD);
	g_qy_entry_->labelsize(FL_NORMAL_SIZE + 2);
	g_qy_entry_->labelcolor(FL_DARK_MAGENTA);
	max_x = max(max_x, g_qy_entry_->x() + g_qy_entry_->w());

	curr_y = max(g_entry_->y() + g_entry_->h(), g_query_->y() + g_query_->h());
	curr_y = max(curr_y, g_net_entry_->y() + g_net_entry_->h());
	curr_y += GAP;

	g_buttons_ = new qso_buttons(curr_x, curr_y, 10, 10);

	max_x = max(max_x, g_buttons_->x() + g_buttons_->w());
	max_x += GAP;
	curr_y += g_buttons_->h() + GAP;

	resizable(nullptr);
	size(max_x - X, curr_y - Y);
	end();

	enable_widgets();

}

// Enable QSO widgets
void qso_data::enable_widgets() {
	if (!inhibit_drawing_) {
		// Disable log mode menu item from CAT if no CAT
		qso_manager* mgr = (qso_manager*)parent();
		if (mgr->created_) {
			rig_if* rig = mgr->rig();
			mgr->qsl_control()->enable_widgets();
			if (menu_) menu_->update_items();
		}

		char l[128];
		switch (logging_state_) {
		case QSO_INACTIVE:
			g_entry_->label("QSO Entry is not enabled");
			g_entry_->labelcolor(DARK ? fl_lighter(FL_RED) : FL_RED);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_PENDING:
			g_entry_->label("QSO Entry - prepared for real-time logging.");
			g_entry_->labelcolor(DARK ? fl_lighter(FL_BLUE) : FL_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_STARTED:
			snprintf(l, sizeof(l), "QSO Entry - %s - logging new contact", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->labelcolor(DARK ? fl_lighter(FL_BLUE) : FL_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_ENTER:
			snprintf(l, sizeof(l), "QSO Entry - %s - logging old contact", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->labelcolor(DARK ? fl_lighter(FL_BLUE) : FL_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_EDIT:
			snprintf(l, sizeof(l), "QSO Entry - %s - editing existing contact", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->labelcolor(DARK ? fl_lighter(FL_BLUE) : FL_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_VIEW:
			snprintf(l, sizeof(l), "QSO Entry - %s - read only", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->labelcolor(DARK ? fl_lighter(FL_BLUE) : FL_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_BROWSE:
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", g_query_->qso()->item("CALL").c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			break;
		case QUERY_DUPE:
		case QUERY_MATCH:
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", g_query_->qso()->item("CALL").c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			break;
		case QUERY_NEW:
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", g_query_->query_qso()->item("CALL").c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			break;
		case QUERY_WSJTX:
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - Possible match found in ALL.TXT", g_query_->query_qso()->item("CALL").c_str());
			g_query_->copy_label(l);
			g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_qy_entry_->hide();
			break;
		case NET_STARTED:
			g_entry_->hide();
			g_query_->hide();
			g_net_entry_->label("Net Entry - active real-time logging");
			g_net_entry_->show();
			g_net_entry_->enable_widgets();
			g_qy_entry_->hide();
			break;
		case NET_EDIT:
			g_entry_->hide();
			g_query_->hide();
			g_net_entry_->label("Net Entry - off-air logging");
			g_net_entry_->show();
			g_net_entry_->enable_widgets();
			g_qy_entry_->hide();
			break;
		case QSO_MODEM:
			snprintf(l, sizeof(l), "QSO Entry - %s - record received from modem app", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_qy_entry_->hide();
			break;
		case MANUAL_ENTRY:
			g_entry_->hide();
			g_query_->hide();
			g_qy_entry_->show();
			g_qy_entry_->label("Enter QSO details for search");
			g_qy_entry_->enable_widgets();
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
		if (log_num != g_entry_->qso_number()) {
			// Deactivate then reactivate with new QSO
			action_deactivate();
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		}
		break;
	case QSO_STARTED:
	case QSO_ENTER:
		// Ack whether to save or quit then activate new QSO
		if (log_num != g_entry_->qso_number()) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while logging a QSO", "Save QSO", "Quit QSO", "Ignore")) {
			case 0:
				// Save QSO
				if(!action_save()) break;
				book_->selection(book_->item_number(log_num));
				logging_state_ = QSO_INACTIVE;
				break;
			case 1:
				// Cancel QSO
				action_cancel();
				book_->selection(book_->item_number(log_num));
				logging_state_ = QSO_INACTIVE;
				break;
			case 2:
				// Ignore the selection request
				book_->selection(book_->item_number(g_entry_->qso_number()));
				break;
			}
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		}
		break;
	case QSO_EDIT:
		// Ask whether to save or quit then open new QSO in edit mode
		if (log_num != g_entry_->qso_number()) {
			if (g_entry_->qso()->is_dirty()) {
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
				logging_state_ = QSO_INACTIVE;
				break;
			case 1:
				// Cancel QSO
				action_cancel_net_all();
				logging_state_ = QSO_INACTIVE;
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
		}
		break;
	}
	// action_view_qsl();
}

// Update query
void qso_data::update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num) {
	switch (logging_state_) {
	case QSO_PENDING:
	case QSO_INACTIVE:
	case QSO_VIEW:
	case QUERY_NEW:
	case QUERY_WSJTX:
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
record* qso_data::start_modem_qso(string call) {
	bool allow_modem = true;
	switch (logging_state_) {
	case QSO_PENDING:
		action_deactivate();
		// drop down
	case QSO_INACTIVE:
		break;	

	case QSO_EDIT:
		if (g_entry_->qso()->is_dirty()) {
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

	case QSO_MODEM:
	// We are altready displaying a modem QSO
		action_cancel_modem();
		break;

	default:
		status_->misc_status(ST_ERROR, "DASH: Getting a modem update when not expected");
		return nullptr;
	}
	if (allow_modem) {
		printf("DEBUG DASH Received request to create a record for %s\n", call.c_str());
		action_activate(QSO_COPY_MODEM);
		action_start(QSO_COPY_MODEM);
		current_qso()->item("CALL", call);
		printf("DEBUG DASH generated %s - %p\n", call.c_str(), current_qso());
		return current_qso();
	} else {
		return nullptr;
	}
}

// Update modem QSO
void qso_data::update_modem_qso(bool log_it) {
	switch (logging_state_) {
	case QSO_MODEM: {
		if (log_it) {
			action_log_modem();
			logging_state_ = QSO_INACTIVE;
		} else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
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

// Save the settings
void qso_data::save_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	// Read field settings
	Fl_Preferences field_settings(dash_settings, "Field Lists");
	for (auto ix = qso_entry::field_map_.begin(); ix != qso_entry::field_map_.end(); ix++) {
		field_settings.set(ix->first.c_str(), join_line(ix->second, ',').c_str());
	}
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
		return book_->selection();
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_ENTER:
	case QSO_EDIT:
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
	// printf("DEBUG: action_new_qso %p\n", qso);
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
	// printf("DEBUG: Creating QSO in %p\n", qe);
	rig_if* rig = ((qso_manager*)parent())->rig();
	qso_init_t new_mode = (mode == QSO_AS_WAS) ? previous_mode_ : mode;
	switch (new_mode) {
	case QSO_NONE:
		// Just copy the station details
		// printf("DEBUG: Off-line contact\n");
		qe->copy_qso_to_qso(qso, qso_entry::CF_COPY);
		break;
	case QSO_ON_AIR:
		// Copy station details and get read rig details for band etc. 
		// If rig not connected use same as original
		// printf("DEBUG: On-air contact\n");
		if (rig && rig->is_good()) {
			qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC);
			qe->copy_cat_to_qso();
		} else {
			qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		}
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_CONDX:
		// Clone the QSO - get station and band from original QSO
		// printf("DEBUG: clone ontact\n");
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_CALL:
		// Copy the QSO - as abobe but also same callsign and details
		// printf("DEBUG: Clone+ contact\n");
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT | qso_entry::CF_CONTACT);
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_FOR_NET:
		// Clone the QSO - get time, station and band from original QSO
		// printf("DEBUG: Clone++ contact\n");
		qe->copy_qso_to_qso(qso, qso_entry::CF_TIME | qso_entry::CF_DATE | qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_MODEM:
		// Set QSO and Update the display
		qe->qso(-1);
		// ancestor_view<qso_manager>(this)->update_import_qso(qso);
		qe->copy_default_to_qso();
		qe->copy_clock_to_qso();
		qe->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		break;
	}
	previous_mode_ = new_mode;
}


// Action ACTIVATE: transition from QSO_INACTIVE to QSO_PENDING
void qso_data::action_activate(qso_init_t mode) {
	// printf("DEBUG: action_activate\n");
	record* source_record = book_->get_record();
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	logging_state_ = QSO_PENDING;
	action_new_qso(source_record, mode);
	enable_widgets();
}

// Action START - transition from QSO_PENDING to QSO_STARTED
void qso_data::action_start(qso_init_t mode) {
	// printf("DEBUG: action_start\n");
	// Add to book
	// action_new_qso(current_qso(), mode);
	g_entry_->append_qso();
	book_->enable_save(false);
	book_->selection(book_->item_number(g_entry_->qso_number()), HT_INSERTED);
	switch (mode) {
	case QSO_NONE:
		logging_state_ = QSO_ENTER;
		break;
	case QSO_COPY_MODEM:
		logging_state_ = QSO_MODEM;
		break;
	default:
		logging_state_ = QSO_STARTED;
		break;
	}
	enable_widgets();
}

// Action SAVE - transition from QSO_STARTED to QSO_INACTIVE while saving record
bool qso_data::action_save() {
	// printf("DEBUG: action_save\n");
	record* qso = nullptr;
	item_num_t item_number = -1;
	qso_num_t qso_number = -1;
	switch (logging_state_) {
	case QSO_STARTED:
	case QSO_MODEM:
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
		// // Increment contest serial number
		// g_contest_->increment_serial();
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
		// Put the record in its correct position and save that position
		item_number = book_->correct_record_position(item_number);
		qso_number = book_->record_number(item_number);
		break;
	case QSO_NONE:
	case QSO_COPY_MODEM:
		// Put the record in its correct position and save that position
		item_number = book_->correct_record_position(item_number);
		qso_number = book_->record_number(item_number);
		break;
	}

	// check whether record has changed - when parsed
	if (cty_data_->update_qso(qso)) {
	}

	// check whether record has changed - when validated
	if (spec_data_->validate(qso, item_number)) {
	}

	book_->add_use_data(qso);

	book_->enable_save(true);

	// Upload QSO to QSL servers
	book_->upload_qso(qso_number);

	switch (logging_state_) {
	case QSO_STARTED:
	case QSO_MODEM:
		logging_state_ = SWITCHING;
		book_->modified(true);
		book_->selection(item_number, HT_INSERTED);
		logging_state_ = QSO_INACTIVE;
		break;
	case NET_STARTED:
		g_net_entry_->remove_entry();
		logging_state_ = SWITCHING;
		if (g_net_entry_->entries() == 0) {
			book_->modified(true);
			book_->selection(item_number, HT_INSERTED);
			logging_state_ = QSO_INACTIVE;
		}
		else {
			book_->modified(true);
			book_->selection(g_net_entry_->qso_number(), HT_INSERTED_NODXA);
			logging_state_ = NET_STARTED;
		}
		break;
	}
	enable_widgets();
	return true;
}

// Action CANCEL - Transition from QSO_STARTED to QSO_INACTIVE without saving record
void qso_data::action_cancel() {
	// printf("DEBUG: action_cancel\n");
	// book_->delete_record() will change the selected record - we need ti be inactive to ignore it
	logging_state_t saved_state = logging_state_;
	logging_state_ = QSO_INACTIVE;
	book_->delete_record(true);
	logging_state_ = saved_state;

	switch (logging_state_) {
	case QSO_STARTED:
		g_entry_->delete_qso();
		logging_state_ = QSO_INACTIVE;
		break;
	case QSO_MODEM:
		logging_state_ = QSO_INACTIVE;
		break;
	case NET_STARTED:
		g_net_entry_->remove_entry();
		if (g_net_entry_->entries() == 0) {
			logging_state_ = QSO_INACTIVE;
		}
		break;
	}
	enable_widgets();
	g_entry_->check_qth_changed();
}

// Action DELETE - we should be inactive but leave this code 
void qso_data::action_delete_qso() {
	// printf("DEBUG: action_delete_qso\n");
	// book_->delete_record() will change the selected record - we need ti be inactive to ignore it
	logging_state_t saved_state = logging_state_;
	logging_state_ = QSO_INACTIVE;
	book_->delete_record(true);
	// TODO: Probably a frig - ne need to set up the qso_entry that is now exposed
	action_activate(QSO_NONE);

	// Now restore the original state
	switch (saved_state) {
	case QSO_INACTIVE:
		action_deactivate();
		break;
	case QSO_PENDING:
		break;
	}

	enable_widgets();
}

// Action DEACTIVATE - Transition from QSO_PENDING to QSO_INACTIVE
void qso_data::action_deactivate() {
	// printf("DEBUG: action_deactivate\n");
	g_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action EDIT - Transition from QSO_INACTIVE to QSO_EDIT
void qso_data::action_edit() {
	// printf("DEBUG: action_edit\n");
	// Save a copy of the current record
	qso_num_t qso_number = get_default_number();
	edit_return_state_ = logging_state_;
	g_entry_->qso(qso_number);
	record* qso = g_entry_->qso();
	// If the QSO looks incomplete ask if it is so and restart it else edit it
	if (qso->item("TIME_OFF") == "") {
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
	enable_widgets();
}

// Action VIEW - Transition from QSO_INACTIVE to QSO_VIEW
void qso_data::action_view() {
	// printf("DEBUG: action_view\n");
	// Save a copy of the current record
	qso_num_t qso_number = get_default_number();
	edit_return_state_ = logging_state_;
	logging_state_ = QSO_VIEW;
	g_entry_->qso(qso_number);
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action SAVE EDIT - Transition from QSO_EDIT to QSO_INACTIVE while saving changes
void qso_data::action_save_edit() {
	// printf("DEBUG: action_save_edit\n");
	// We no longer need to maintain the copy of the original QSO
	record* qso = g_entry_->qso();
	book_->add_use_data(qso);
	if (qso->is_dirty()) book_->modified(true);
	g_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// ACtion CANCEL EDIT - Transition from QSO_EDIT to QSO_INACTIVE scrapping changes
void qso_data::action_cancel_edit() {
	// printf("DEBUG: action_cancel_edit\n");
	// Copy original back to the book
	if (g_entry_->original_qso()) {
		*book_->get_record(g_entry_->qso_number(), false) = *g_entry_->original_qso();
	}
	g_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action CANCEL in BROWSE 
void qso_data::action_cancel_browse() {
	// printf("DEBUG: action_cancel_browse\n");
	g_entry_->qso(g_query_->qso_number());
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action navigate button
void qso_data::action_navigate(int target) {
	// printf("DEBUG: action_navigate\n");
	inhibit_drawing_ = true;
	switch (logging_state_) {
	case QSO_EDIT:
		// Save, navigate to new QSO and open editor
		action_save_edit();
		navigation_book_->navigate((navigate_t)target);
		action_edit();
		// action_view_qsl();
		break;
	case QSO_VIEW:
		// Save, navigate to new QSO and open editor
		action_cancel_edit();
		navigation_book_->navigate((navigate_t)target);
		action_view();
		// action_view_qsl();
		break;
	case QSO_PENDING:
		// Deactivate, navigate and go pending again
		action_deactivate();
		navigation_book_->navigate((navigate_t)target);
		action_activate(previous_mode_);
		// action_view_qsl();
		break;
	case QSO_BROWSE:
		// Close browser, navigate and reopen browser
		action_cancel_browse();
		navigation_book_->navigate((navigate_t)target);
		action_browse();
		// action_view_qsl();
		break;
	case QUERY_MATCH:
		// Navigate book to new compare record and reopen query
		book_->navigate((navigate_t)target);
		action_query(logging_state_, book_->selection(), 0);
		break;
	}
	inhibit_drawing_ = false;
	enable_widgets();
}

// Action browse
void qso_data::action_browse() {
	// printf("DEBUG: action_browse\n");
	qso_num_t qso_number = get_default_number();
	logging_state_ = QSO_BROWSE;
	g_query_->set_query("Browsing record",qso_number);
	enable_widgets();
}

// Action query
void qso_data::action_query(logging_state_t query, qso_num_t match_number, qso_num_t query_number) {
	// printf("DEBUG: action_query\n");
	switch (query) {
	case QUERY_MATCH:
		g_query_->set_query(import_data_->match_question(), match_number, import_data_->get_record(query_number, false));
		break;
	case QUERY_NEW:
		potential_match_ = match_number;
		query_number_ = query_number;
		g_query_->set_query(import_data_->match_question(), -1, import_data_->get_record(query_number, false));
		break;
	case QUERY_DUPE:
		// Note record numbers relate to book even if it is extracted data that refered the dupe check
		g_query_->set_query(navigation_book_->match_question(), match_number, book_->get_record(query_number, false), false);
		break;
	case QRZ_MERGE:
		g_query_->set_query(qrz_handler_->get_merge_message(), match_number, qrz_handler_->get_record());
		break;
	default:
		// TODO trap this sensibly
		return;
	}
	logging_state_ = query;
	enable_widgets();
	g_buttons_->enable_widgets();
	// Move window to top
	parent()->show();
}

// Action add query - add query QSO to book
void qso_data::action_add_query() {
	// printf("DEBUG: action_add_query\n");
	import_data_->save_update();
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();

}

// Action reject query - do nothing
void qso_data::action_reject_query() {
	printf("DEBUG: action_reject_query\n");
	import_data_->discard_update(true);
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// Action reject manual query - do nothing
void qso_data::action_reject_manual() {
	// printf("DEBUG: action_reject_manual\n");
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action merge query
void qso_data::action_merge_query() {
	// printf("DEBUG: action_merge_query\n");
	import_data_->merge_update();
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// ACtion find match
void qso_data::action_find_match() {
	// printf("DEBUG: action_find_match\n");
	update_query(QUERY_MATCH, potential_match_, query_number_);
}

// Action handle dupe
void qso_data::action_handle_dupe(dupe_flags action) {
	// printf("DEBUG: action_handle_dupe\n");
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
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the duplicate check process
	navigation_book_->check_dupes(true);

}

// Action save as a result of a merge
void qso_data::action_save_merge() {
	// printf("DEBUG: action_save_merge\n");
	// We no longer need to maintain the copy of the original QSO
	book_->add_use_data(g_query_->qso());
	book_->modified(true);
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	qrz_handler_->merge_done();
}

// ACtion look in ALL.TXT
void qso_data::action_look_all_txt() {
	// printf("DEBUG: action_look_all_txt\n");
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
	}
	enable_widgets();
}

// Create a net from current QSO and others which overlap
void qso_data::action_create_net() {
	printf("DEBUG: action_create_net\n");
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
	// Now add the remaining QSOs in the net - first look earlier
	qso_num_t other_number = qso_number - 1;
	while (qso->match_records(book_->get_record(other_number, false)) == MT_OVERLAP) {
		g_net_entry_->add_entry();
		g_net_entry_->set_qso(other_number);
		g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		other_number--;
	}
	// Now look later
	other_number = qso_number + 1;
	while (qso->match_records(book_->get_record(other_number, false)) == MT_OVERLAP) {
		g_net_entry_->add_entry();
		g_net_entry_->set_qso(other_number);
		g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		other_number++;
	}
	g_net_entry_->entry(w);
	enable_widgets();
}
	

// Add a QSO to the net - copy existing qso start times or not
void qso_data::action_add_net_qso() {
	// printf("DEBUG: action_add_net_qso\n");
	record* qso = g_net_entry_->qso();
	// Create the entry tab
	g_net_entry_->add_entry();
	// Create the new QSO therein
	switch (logging_state_) {
	case NET_STARTED:
		logging_state_ = NET_ADDING;
		// printf("DEBUG: Add net QSO\n");
		action_new_qso(qso, QSO_ON_AIR);
		logging_state_ = NET_STARTED;
		break;
	case NET_EDIT:
		action_new_qso(qso, QSO_COPY_FOR_NET);
		break;
	}
	// Add it to the book
	g_net_entry_->append_qso();
	book_->selection(book_->item_number(g_net_entry_->qso_number()), HT_INSERTED);
	enable_widgets();
}

// Save the whole net
void qso_data::action_save_net_all() {
	// printf("DEBUG: action_save_net_all\n");
	// Only save the book once all records have been saved
	book_->enable_save(false);
	book_->allow_upload(false);
	bool ok = true;
	while (g_net_entry_->entries() && ok) {
		switch (logging_state_) {
		case NET_STARTED:
			ok = action_save();
			break;
		case NET_EDIT:
			action_save_net_edit();
			break;
		}
	}
	// Restore the place-holder entry
	if (ok) g_net_entry_->add_entry();
	book_->enable_save(true);
	book_->allow_upload(true);
	enable_widgets();
}

// Save a QSO in NET_EDIT
void qso_data::action_save_net_edit() {
	// printf("DEBUG: action_save_net_edit\n");
	// We no longer need to maintain the copy of the original QSO
	book_->add_use_data(g_net_entry_->qso());
	book_->modified(true);
	g_net_entry_->remove_entry();
	if (g_net_entry_->entries() == 0) {
		logging_state_ = QSO_INACTIVE;
	}
	else {
		logging_state_ = NET_EDIT;
	}
	enable_widgets();
}

// Cancel the whole net
void qso_data::action_cancel_net_all() {
	// printf("DEBUG: action_cancel_net_all\n");
	book_->enable_save(false);
	while (g_net_entry_->entries()) {
		switch(logging_state_) {
		case NET_STARTED: 
			action_cancel();
			break;
		case NET_EDIT:
			action_cancel_net_edit();
			break;
		}
	}
	book_->enable_save(true);
	// Restore the place-holder entry
	g_net_entry_->add_entry();
	enable_widgets();
}

// Cancel an individual QSO in net edit
void qso_data::action_cancel_net_edit() {
	// printf("DEBUG: action_cancel_net_edit\n");
	// Copy original back to the book
	*book_->get_record(g_net_entry_->qso_number(), false) = *g_net_entry_->original_qso();
	g_net_entry_->remove_entry();
	if (g_net_entry_->entries() == 0) {
		logging_state_ = QSO_INACTIVE;
	}
	else {
		g_net_entry_->entry()->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		logging_state_ = NET_EDIT;
	}
	book_->enable_save(true);
	enable_widgets();
}

// Start a modem record
void qso_data::action_log_modem() {
	// printf("DEBUG: action_add_modem\n");
	// Add to book
	book_->enable_save(false);
	record* qso = current_qso();
	if (qso->item("TX_PWR") == "") {
		// Get power from rig
		rig_if* rig = ((qso_manager*)parent())->rig();
		qso->item("TX_PWR", rig->get_tx_power(true));
	}
	// The QSO is complete
	action_save();
	// Open in view
	action_view();
	
	book_->selection(book_->item_number(g_entry_->qso_number()), HT_INSERTED);
	book_->enable_save(true);
	enable_widgets();
}

// Cacncel a modem record
void qso_data::action_cancel_modem() {
	printf("DEBUG DASH - Cancelling modem %s - %p\n", current_qso()->item("CALL").c_str(), current_qso());
	logging_state_ = QSO_INACTIVE;
	wsjtx_handler_->delete_qso(current_qso()->item("CALL"));
	g_entry_->delete_qso();
	book_->delete_record(true);
	book_->enable_save(true);
	enable_widgets();
}

// Create a query entry
void qso_data::action_query_entry() {
	// printf("DEBUG: action_query_entry\n");
	g_qy_entry_->qso(-1);
	logging_state_ = MANUAL_ENTRY;
	g_qy_entry_->copy_default_to_qso();
	g_qy_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Execute the query
void qso_data::action_exec_query() {
	// printf("DEBUG: action_exec_query\n");
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
		edit_return_state_ = logging_state_;
		logging_state_ = QSO_EDIT;
		// Display the first record
		g_entry_->qso(extract_records_->record_number(0));
		g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		enable_widgets();
	}
	else {
		snprintf(msg, sizeof(msg), "DASH: No matching records found - try import");
		status_->misc_status(ST_ERROR, msg);
		enable_widgets();
	}
}

// Abandon the query
void qso_data::action_cancel_query() {
	// printf("DEBUG: action_cancel_query\n");
	g_qy_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Import the query
void qso_data::action_import_query() {
	// printf("DEBUG: action_import_query\n");
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
	// printf("DEBUG: action_qrz_com\n");
	record* qso = current_qso();
	qrz_handler_->open_web_page(qso->item("CALL"));
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

void qso_data::update_rig() {
	// Get freq etc from QSO or rig
// Get present values data from rig
	if (logging_state_ == QSO_PENDING) {
		if (((qso_manager*)parent())->rig()->is_good()) {
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
		action_save();
		break;
	case qso_data::QSO_EDIT:
		action_save_edit();
		break;
	case NET_STARTED:
	case NET_EDIT:
		action_save_net_all();
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
	case QSO_MODEM:
		return g_entry_->qso();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QRZ_MERGE:
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

// Current QSO number
qso_num_t qso_data::current_number() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_STARTED:
	case QSO_ENTER:
	case QSO_EDIT:
	case QSO_VIEW:
	case QSO_MODEM:
		return g_entry_->qso_number();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QUERY_WSJTX:
	case QRZ_MERGE:
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

// Update time and rig infoF
void qso_data::ticker() {
	g_entry_->copy_clock_to_qso();
	g_net_entry_->ticker();
	g_entry_->copy_cat_to_qso();
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

// Inactive
bool qso_data::inactive() {
	switch(logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_VIEW:
		return true;
	default:
		return false;
	}
}
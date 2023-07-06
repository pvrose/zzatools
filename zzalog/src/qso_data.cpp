#include "qso_data.h"
#include "qso_manager.h"
#include "qsl_viewer.h"
#include "qth_dialog.h"
#include "settings.h"
#include "rig_if.h"
#include "book.h"
#include "status.h"
#include "extract_data.h"
#include "cty_data.h"
#include "spec_data.h"
#include "band_view.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "qrz_handler.h"
#ifdef _WIN32
#include "dxa_if.h"
#endif

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
#ifdef _WIN32
extern dxa_if* dxa_if_;
#endif
extern double prev_freq_;


// qso_group_
qso_data::qso_data(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
	, logging_state_(QSO_INACTIVE)
	, inhibit_drawing_(false)
	, previous_mode_(QSO_NONE)
{
	load_values();
	qsl_viewer_ = new qsl_viewer(10, 10);
	qsl_viewer_->callback(cb_qsl_viewer);
	qsl_viewer_->hide();
}

// Destructor
qso_data::~qso_data() {
}

// Load values
void qso_data::load_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
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

	g_contest_ = new qso_contest(curr_x, curr_y, 10, 10);

	max_x = max(max_x, g_contest_->x() + g_contest_->w());
	curr_y = g_contest_->y() + g_contest_->h() + GAP;

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

	g_peek_ = new qso_entry(curr_x, curr_y, 10, 10);
	g_peek_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_peek_->labelfont(FL_BOLD);
	g_peek_->labelsize(FL_NORMAL_SIZE + 2);
	g_peek_->labelcolor(FL_DARK_YELLOW);
	max_x = max(max_x, g_peek_->x() + g_peek_->w());

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
		rig_if* rig = ((qso_manager*)parent())->rig();

		g_contest_->enable_widgets();

		char l[128];
		switch (logging_state_) {
		case QSO_INACTIVE:
			g_entry_->label("QSO Entry is not enabled");
			g_entry_->labelcolor(FL_DARK_RED);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_PENDING:
			g_entry_->label("QSO Entry - prepared for real-time logging.");
			g_entry_->labelcolor(FL_DARK_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_STARTED:
			snprintf(l, sizeof(l), "QSO Entry - %s - logging new contact", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->labelcolor(FL_DARK_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_EDIT:
			snprintf(l, sizeof(l), "QSO Entry - %s - editing existing contact", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->labelcolor(FL_DARK_BLUE);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_BROWSE:
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", g_query_->qso()->item("CALL").c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_peek_->hide();
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
			g_peek_->hide();
			g_qy_entry_->hide();
		case QUERY_NEW:
			g_entry_->hide();
			snprintf(l, sizeof(l), "QSO Query - %s - %s", g_query_->query_qso()->item("CALL").c_str(), g_query_->query_message().c_str());
			g_query_->copy_label(l);
			g_query_->show();
			g_query_->enable_widgets();
			g_net_entry_->hide();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case NET_STARTED:
			g_entry_->hide();
			g_query_->hide();
			g_net_entry_->label("Net Entry - active real-time logging");
			g_net_entry_->show();
			g_net_entry_->enable_widgets();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case NET_EDIT:
			g_entry_->hide();
			g_query_->hide();
			g_net_entry_->label("Net Entry - off-air logging");
			g_net_entry_->show();
			g_net_entry_->enable_widgets();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_MODEM:
			snprintf(l, sizeof(l), "QSO Entry - %s - record received from modem app", current_qso()->item("CALL").c_str());
			g_entry_->copy_label(l);
			g_entry_->show();
			g_entry_->enable_widgets();
			g_net_entry_->hide();
			g_query_->hide();
			g_peek_->hide();
			g_qy_entry_->hide();
			break;
		case QSO_PEEK:
		case QSO_PEEK_ED:
			snprintf(l, sizeof(l), "QSO Peek - %s", current_qso()->item("CALL").c_str());
			g_entry_->hide();
			g_net_entry_->hide();
			g_query_->hide();
			g_peek_->copy_label(l);
			g_peek_->show();
			g_peek_->enable_widgets();
			g_qy_entry_->hide();
			break;
		case MANUAL_ENTRY:
			g_entry_->hide();
			g_query_->hide();
			g_peek_->hide();
			g_qy_entry_->show();
			g_qy_entry_->label("Enter QSO details for search");
			g_qy_entry_->enable_widgets();
			break;
		}
		g_buttons_->enable_widgets();
	}
}

// Update QSO
void qso_data::update_qso(qso_num_t log_num) {
	switch (logging_state_) {
	case QSO_INACTIVE:
	{
		// Copy selected QSO 
		record* qso = book_->get_record();
		g_entry_->copy_qso_to_qso(qso, qso_entry::CF_ALL_FLAGS);
		break;
	}
	case QSO_PENDING:
		if (log_num != g_entry_->qso_number()) {
			// Deactivate then reactivate with new QSO
			action_deactivate();
			action_activate(previous_mode_);
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		}
		break;
	case QSO_STARTED:
	case QSO_MODEM:
		// Ack whether to save or quit then activate new QSO
		if (log_num != g_entry_->qso_number()) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while logging a QSO", "Save QSO", "Quit QSO", nullptr)) {
			case 0:
				// Save QSO
				action_save();
				break;
			case 1:
				// Cancel QSO
				action_cancel();
				break;
			}
			// Actions will have changed selection - change it back.
			logging_state_ = QSO_INACTIVE;
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
				switch (fl_choice("Trying to select a different record while editing a record", "Save edit", "Cancel edit", nullptr)) {
				case 0:
					// Save QSO
					action_save_edit();
					break;
				case 1:
					// Cancel QSO
					action_cancel_edit();
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
	case NET_STARTED:
	case NET_EDIT:
		if (!g_net_entry_->qso_in_net(log_num)) {
			// Selected QSO is not part of the net, save or cancel the net
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while logging a net", "Save Net", "Quit Net", nullptr)) {
			case 0:
				// Save QSO
				action_save_net_all();
				break;
			case 1:
				// Cancel QSO
				action_cancel_net_all();
				break;
			}
			// Actions will have changed selection - change it back.
			logging_state_ = QSO_INACTIVE;
		}
		else {
			// Switch to the selected QSO as part of the net if necessary
			if (log_num != g_net_entry_->qso_number()) {
				g_net_entry_->select_qso(log_num);
			}
			g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		}
		break;
	case QSO_PEEK:
	case QSO_PEEK_ED:
		g_peek_->copy_qso_to_qso(book_->get_record(log_num, false), qso_entry::CF_ALL_FLAGS);
		g_peek_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		break;
	}
	action_view_qsl();
}

// Update query
void qso_data::update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num) {
	switch (logging_state_) {
	case QSO_PENDING:
	case QSO_INACTIVE:
	case QUERY_NEW:
	case MANUAL_ENTRY:
		action_query(query, match_num, query_num);
		break;
	default:
		// TODO:
		break;
	}
}

// Update modem QSO
void qso_data::update_modem_qso(record* qso) {
	if (qso == nullptr) {
		switch (logging_state_) {
		case QSO_MODEM:
			action_cancel_modem();
			break;
		}
	}
	else {
		switch (logging_state_) {
		case QSO_PENDING:
			action_deactivate();
			// drop down
		case QSO_INACTIVE:
			action_add_modem(qso);
			break;

		case QSO_MODEM:
			action_update_modem(qso);
			break;

		default:
			status_->misc_status(ST_ERROR, "DASH: Getting a modem update when not expected");
			return;
		}
	}
	enable_widgets();
}

// Callback - QSL viewer "closing" - make it hide instead
void qso_data::cb_qsl_viewer(Fl_Widget* w, void* v) {
	qsl_viewer* qsl = (qsl_viewer*)w;
	if (qsl->visible()) qsl->hide();
}

// Save the settings
void qso_data::save_values() {
	// Dashboard configuration
}

// Initialise fields from format definitions
void qso_data::initialise_fields(qso_entry* entry) {
	string preset_fields;
	bool lock_preset = false;
	bool new_fields = false;
	switch (g_contest_->mode()) {
	case qso_contest::NO_CONTEST:
	case qso_contest::PAUSED:
		// Non -contest mode
		preset_fields = "RST_SENT,RST_RCVD,NAME,QTH";
		new_fields = true;
		lock_preset = false;
		break;
	case qso_contest::CONTEST:
		// Contest mode
		preset_fields = g_contest_->contest_fields();
		new_fields = true;
		lock_preset = true;
		break;
	case qso_contest::NEW:
		// Do not change existing
		preset_fields = "";
		new_fields = false;
		lock_preset = true;
		break;
	case qso_contest::DEFINE:
		// Define new exchange - provide base RS/Serno
		preset_fields = "";
		new_fields = true;
		lock_preset = false;
		break;
	case qso_contest::EDIT:
		// Unlock existing definition 
		preset_fields = g_contest_->contest_fields();
		new_fields = true;
		lock_preset = false;
		break;
	}
	// TODO this shoule be somewhere else
	//// Set contest format
	//ch_format_->value(exch_fmt_id_.c_str());
	// Default Contest TX values
	if (g_contest_->mode() == qso_contest::CONTEST) {
		// Automatically create a pending QSO
		if (logging_state_ == QSO_INACTIVE) {
			action_activate(previous_mode_);
		}
	}
	entry->initialise_fields(preset_fields, new_fields, lock_preset);
	entry->initialise_values(preset_fields, g_contest_->serial_number());
}

string qso_data::get_defined_fields() {
	return g_entry_->get_defined_fields();
}

// Get default record to copy
qso_num_t qso_data::get_default_number() {
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QUERY_NEW:
		return book_->selection();
	case QSO_PENDING:
	case QSO_STARTED:
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
	qso_entry* qe;
	switch (logging_state_) {
	case NET_STARTED:
	case NET_EDIT:
	case NET_ADDING:
		qe = (qso_entry*)g_net_entry_->entry();
		break;
	case QSO_PEEK:
	case QSO_PEEK_ED:
		qe = g_peek_;
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
	switch (new_mode) {
	case QSO_NONE:
		// Just copy the station details
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC);
		break;
	case QSO_ON_AIR:
		// Copy station details and get read rig details for band etc. 
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC);
		qe->copy_cat_to_qso();
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_CONDX:
		// Clone the QSO - get station and band from original QSO
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_CALL:
		// Copy the QSO - as abobe but also same callsign and details
		qe->copy_qso_to_qso(qso, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT | qso_entry::CF_CONTACT);
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_FOR_NET:
		// Clone the QSO - get time, station and band from original QSO
		qe->copy_qso_to_qso(qso, qso_entry::CF_TIME | qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		qe->copy_clock_to_qso();
		break;
	case QSO_COPY_MODEM:
		// Set QSO and Update the display
		qe->qso(qso);
		ancestor_view<qso_manager>(this)->update_import_qso(qso);
		qe->copy_clock_to_qso();
		qe->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		break;
	}
	previous_mode_ = new_mode;
}


// Action ACTIVATE: transition from QSO_INACTIVE to QSO_PENDING
void qso_data::action_activate(qso_init_t mode) {
	record* source_record = book_->get_record();
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	logging_state_ = QSO_PENDING;
	action_new_qso(source_record, mode);
	enable_widgets();
}

// Action START - transition from QSO_PENDING to QSO_STARTED
void qso_data::action_start(qso_init_t mode) {
	// Add to book
	action_new_qso(g_entry_->qso(), mode);
	g_entry_->append_qso();
	book_->selection(book_->item_number(g_entry_->qso_number()), HT_INSERTED);
	logging_state_ = QSO_STARTED;
	enable_widgets();
}

// Action SAVE - transition from QSO_STARTED to QSO_INACTIVE while saving record
void qso_data::action_save() {
	record* qso = nullptr;
	item_num_t item_number = -1;
	qso_num_t qso_number = -1;
	switch (logging_state_) {
	case QSO_STARTED:
	case QSO_MODEM:
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
		// Increment contest serial number
		g_contest_->increment_serial();
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
}

// Action CANCEL - Transition from QSO_STARTED to QSO_INACTIVE without saving record
void qso_data::action_cancel() {
	// book_->delete_record() will change the selected record - we need ti be inactive to ignore it
	logging_state_t saved_state = logging_state_;
	logging_state_ = QSO_INACTIVE;
	book_->delete_record(true);
	logging_state_ = saved_state;

	switch (logging_state_) {
	case QSO_STARTED:
	case QSO_MODEM:
		g_entry_->delete_qso();
		logging_state_ = QSO_INACTIVE;
#ifdef WIN32
		if (dxa_if_) dxa_if_->clear_dx_loc();
#endif
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
	g_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action EDIT - Transition from QSO_INACTIVE to QSO_EDIT
void qso_data::action_edit() {
	// Save a copy of the current record
	qso_num_t qso_number = get_default_number();
	edit_return_state_ = logging_state_;
	logging_state_ = QSO_EDIT;
	g_entry_->qso(qso_number);
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action SAVE EDIT - Transition from QSO_EDIT to QSO_INACTIVE while saving changes
void qso_data::action_save_edit() {
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
	// Copy original back to the book
	*book_->get_record(g_entry_->qso_number(), false) = *g_entry_->original_qso();
	g_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action CANCEL in BROWSE 
void qso_data::action_cancel_browse() {
	g_entry_->qso(g_query_->qso_number());
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action navigate button
void qso_data::action_navigate(int target) {
	inhibit_drawing_ = true;
	logging_state_t saved_state = logging_state_;
	switch (logging_state_) {
	case QSO_EDIT:
		action_save_edit();
		break;
	case QSO_PENDING:
		action_deactivate();
		break;
	case QSO_BROWSE:
		action_cancel_browse();
		break;
	}
	// We should now be inactive - navigate to new QSO
	navigation_book_->navigate((navigate_t)target);
	// And restore state
	switch (saved_state) {
	case QSO_EDIT:
		action_edit();
		action_view_qsl();
		break;
	case QSO_PENDING:
		action_activate(previous_mode_);
		action_view_qsl();
		break;
	case QSO_BROWSE:
		action_browse();
		action_view_qsl();
		break;
	case QUERY_MATCH:
		action_query(saved_state, book_->selection(), 0);
		break;
	}
	inhibit_drawing_ = false;
	enable_widgets();
}

// Action view qsl
void qso_data::action_view_qsl(bool force) {
	if (force || qsl_viewer_->visible()) {
		record* qso = book_->get_record(book_->item_number(get_default_number()), false);
		if (qso) {
			char title[128];
			snprintf(title, 128, "QSL Status: %s %s %s %s %s",
				qso->item("CALL").c_str(),
				qso->item("QSO_DATE").c_str(),
				qso->item("TIME_ON").c_str(),
				qso->item("BAND").c_str(),
				qso->item("MODE", true, true).c_str());
			qsl_viewer_->copy_label(title);
			qsl_viewer_->set_qso(qso, current_number());
			qsl_viewer_->show();
			char msg[128];
			snprintf(msg, 128, "DASH: %s", title);
			status_->misc_status(ST_LOG, msg);
		}
	}
}


// Action browse
void qso_data::action_browse() {
	qso_num_t qso_number = get_default_number();
	logging_state_ = QSO_BROWSE;
	g_query_->set_query("Browsing record",qso_number);
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
	case QUERY_DUPE:
		// Note record numbers relate to book even if it is extracted data that refered the dupe check
		g_query_->set_query(navigation_book_->match_question(), match_number, book_->get_record(query_number, false));
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
	import_data_->save_update();
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();

}

// Action reject query - do nothing
void qso_data::action_reject_query() {
	import_data_->discard_update(true);
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// Action reject manual query - do nothing
void qso_data::action_reject_manual() {
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action merge query
void qso_data::action_merge_query() {
	import_data_->merge_update();
	g_query_->clear_query();
	logging_state_ = QSO_INACTIVE;
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
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the duplicate check process
	navigation_book_->check_dupes(true);

}

// Action save as a result of a merge
void qso_data::action_save_merge() {
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
	Fl_Preferences datapath_settings(settings_, "Datapath");
	char* temp;
	datapath_settings.get("WSJT-X", temp, "");
	string filename = string(temp) + "/ALL.TXT";
	ifstream* all_file = new ifstream(filename.c_str());
	// This will take a while so display the timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	// calculate the file size and initialise the progress bar
	streampos startpos = all_file->tellg();
	all_file->seekg(0, ios::end);
	streampos endpos = all_file->tellg();
	long file_size = (long)(endpos - startpos);
	status_->misc_status(ST_NOTE, "LOG: Starting to parse ALL.TXT");
	status_->progress(file_size, OT_RECORD, "Looking for queried call in WSJT-X data", "bytes");
	// reposition back to beginning
	all_file->seekg(0, ios::beg);
	bool start_copying = false;
	bool stop_copying = false;
	// Get user callsign from settings
	string my_call = ((qso_manager*)parent())->get_default(qso_manager::CALLSIGN);
	// Get search items from record
	string their_call = g_query_->query_qso()->item("CALL");
	string datestamp = g_query_->query_qso()->item("QSO_DATE").substr(2);
	string timestamp = g_query_->query_qso()->item("TIME_ON");
	string mode = g_query_->query_qso()->item("MODE");
	char msg[256];
	// Mark QSO incomplete 
	g_query_->query_qso()->item("QSO_COMPLETE", string("N"));
	g_query_->redraw();
	int count = 0;
	// Now read the file - search for the QSO start time
	while (all_file->good() && !stop_copying) {
		string line;
		getline(*all_file, line);
		count += line.length() + 1;
		status_->progress(count, OT_RECORD);

		// Does the line contain sought date, time, both calls and "Tx" or "Transmitting"
		if (line.substr(0, 6) == datestamp &&
			line.substr(7, 4) == timestamp.substr(0, 4) &&
			(line.find("Transmitting") != string::npos || line.find("Tx")) &&
			line.find(my_call) != string::npos &&
			line.find(their_call) != string::npos &&
			line.find(mode) != string::npos) {
			if (!start_copying) {
				snprintf(msg, 256, "DASH: %s %s %s %s %s Found possible match",
					datestamp.c_str(),
					timestamp.c_str(),
					mode.c_str(),
					my_call.c_str(),
					their_call.c_str());
				status_->misc_status(ST_WARNING, msg);
			}
			start_copying = true;
		}
		if (start_copying) {
			if (line.find(my_call) != string::npos &&
				line.find(their_call) != string::npos) {
				// It has both calls - copy to buffer, and parse for QSO details (report, grid and QSO completion
				snprintf(msg, 256, "DASH: %s", line.c_str());
				status_->misc_status(ST_NOTE, msg);
				action_copy_all_text(line);
			}
			else if (line.find(my_call) == string::npos &&
				line.find(their_call) == string::npos) {
				// It has neither call - ignore
			}
			else {
				// It has one or the other call - indicates QSO complete
				stop_copying = true;
				g_query_->query_qso()->item("QSO_COMPLETE", string(""));
			}
		}
	}
	if (stop_copying) {
		status_->progress("Found record!", OT_RECORD);
		// If we are complete then say so
		if (g_query_->query_qso()->item("QSO_COMPLETE") != "N" && g_query_->query_qso()->item("QSO_COMPLETE") != "?") {
			all_file->close();
			fl_cursor(FL_CURSOR_DEFAULT);
		}
		enable_widgets();
	}
	else {
		all_file->close();
		snprintf(msg, 100, "LOG: Cannot find contact with %s in WSJT-X text.all file.", their_call.c_str());
		status_->progress("Did not find record", OT_RECORD);
		status_->misc_status(ST_WARNING, msg);
		fl_cursor(FL_CURSOR_DEFAULT);
	}
}

void qso_data::action_copy_all_text(string text) {
	record* qso = g_query_->query_qso();
	bool tx_record;
	// After this initial processing pos will point to the start og the QSO decode string - look for old-style transmit record
	size_t pos = text.find("Transmitting");
	if (pos != string::npos) {
		pos = text.find(qso->item("MODE"));
		pos += qso->item("MODE").length() + 3;
		tx_record = true;
		// Nothing else to get from this record
	}
	else {
		// Now see if it's a new-style Tx record
		pos = text.find("Tx");
		if (pos != string::npos) {
			// Get frequency of transmission - including audio offset
			string freq = text.substr(14, 9);
			// Replace leading spaces with zeroes
			for (size_t i = 0; freq[i] == ' '; i++) {
				freq[i] = '0';
			}
			string freq_offset = text.substr(43, 4);
			// Replace leading spaces with zeroes
			for (size_t i = 0; freq_offset[i] == ' '; i++) {
				freq_offset[i] = '0';
			}
			double frequency = stod(freq) + (stod(freq_offset) / 1000000.0);
			qso->item("FREQ", to_string(frequency));
			pos = 48;
			tx_record = true;
		}
		else {
			// Look for a new-style Rx record
			pos = text.find("Rx");
			if (pos != string::npos) {
				pos = 48;
				tx_record = false;
			}
			else {
				// Default to old-style Rx record
				pos = 24;
				tx_record = false;
			}
		}
	}
	// Now parse the exchange
	vector<string> words;
	split_line(text.substr(pos), words, ' ');
	string report = words.back();
	if (report == "RR73" || report == "RRR") {
		// If we've seen the R-00 then mark the QSO complete, otherwise mark in provisional until we see the 73
		if (qso->item("QSO_COMPLETE") == "?") {
			qso->item("QSO_COMPLETE", string(""));
		}
		else if (qso->item("QSO_COMPLETE") == "N") {
			qso->item("QSO_COMPLETE", string("?"));
		}
	}
	else if (report == "73") {
		// A 73 definitely indicates QSO compplete
		qso->item("QSO_COMPLETE", string(""));
	}
	else if (report[0] == 'R') {
		// The first of the rogers
		if (qso->item("QSO_COMPLETE") == "N") {
			qso->item("QSO_COMPLETE", string("?"));
		}
		// Update reports if they've not been provided
		if (tx_record && !qso->item_exists("RST_SENT")) {
			qso->item("RST_SENT", report.substr(1));
		}
		else if (!tx_record && !qso->item_exists("RST_RCVD")) {
			qso->item("RST_RCVD", report.substr(1));
		}
		// Update callsign
		if (tx_record) {
			string& dx_call = words[0];
			if (dx_call[0] == '<') {
				dx_call = dx_call.substr(1, dx_call.length() - 2);
			}
			string qso_call = qso->item("CALL");
			if (dx_call != qso_call) {
				int comp_len = min(dx_call.length(), qso_call.length());
				if (dx_call.substr(0, comp_len) == qso_call.substr(0, comp_len)) {
					qso->item("CALL", dx_call);
					char msg[128];
					snprintf(msg, sizeof(msg), "DASH: Replacing callsign %s with %s", qso_call.c_str(), dx_call.c_str());
					status_->misc_status(ST_WARNING, msg);
				}
				else {
					char msg[128];
					snprintf(msg, sizeof(msg), "DASH: Callsigns dissimilar %s v %s", qso_call.c_str(), dx_call.c_str());
					status_->misc_status(ST_WARNING, msg);
				}
			}
		}
	}
	else if (!tx_record && !qso->item_exists("GRIDSQUARE")) {
		// Update gridsquare if it's not been provided
		qso->item("GRIDSQUARE", report);
	}
	else if (report[0] == '-' || (report[0] >= '0' && report[0] <= '9')) {
		// Numeric report
		if (tx_record && !qso->item_exists("RST_SENT")) {
			qso->item("RST_SENT", report);
		}
		else if (!tx_record && !qso->item_exists("RST_RCVD")) {
			qso->item("RST_RCVD", report);
		}
	}
}

// Create a net from current QSO and others which overlap
void qso_data::action_create_net() {
	qso_num_t qso_number = g_entry_->qso_number();
	record* qso = g_entry_->qso();
	string call = get_call();
	char msg[128];
	g_net_entry_->set_qso(qso_number);
	g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	Fl_Widget* w = g_net_entry_->entry();
	switch (logging_state_) {
	case QSO_STARTED:
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
	}
	// Add it to the book
	g_net_entry_->append_qso();
	book_->selection(book_->item_number(g_net_entry_->qso_number()), HT_INSERTED);
	enable_widgets();
}

// Save the whole net
void qso_data::action_save_net_all() {
	// Only save the book once all records have been saved
	book_->enable_save(false);
	while (g_net_entry_->entries()) {
		switch (logging_state_) {
		case NET_STARTED:
			action_save();
			break;
		case NET_EDIT:
			action_save_net_edit();
			break;
		}
	}
	// Restore the place-holder entry
	g_net_entry_->add_entry();
	book_->enable_save(true);
	enable_widgets();
}

// Save a QSO in NET_EDIT
void qso_data::action_save_net_edit() {
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
	book_->enable_save(false);
	while (g_net_entry_->entries()) {
		action_cancel();
	}
	book_->enable_save(true);
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
		logging_state_ = QSO_INACTIVE;
	}
	else {
		((qso_entry*)g_net_entry_->entry())->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		logging_state_ = NET_EDIT;
	}
	book_->enable_save(true);
	enable_widgets();
}

// Start a modem record
void qso_data::action_add_modem(record* qso) {
	// Add to book
	book_->enable_save(false);
	action_new_qso(qso, QSO_COPY_MODEM);
	g_entry_->append_qso();
	(ancestor_view<qso_manager>(this))->update_import_qso(qso);
	logging_state_ = QSO_MODEM;
	book_->selection(book_->item_number(g_entry_->qso_number()), HT_INSERTED);
	enable_widgets();
}

// Update or replace a modem record
void qso_data::action_update_modem(record* qso) {
	// Compare with existing
	if (qso != current_qso()) {
		// This is a new record as the previous one did not complete
		action_cancel();
		action_add_modem(qso);
	}
	else {
		if (qso->item("QSO_COMPLETE") == "") {
			// The QSO is complete
			action_save();
			book_->enable_save(true);
		}
		else {
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
		}
	}
	enable_widgets();
}

// Cancel modem operation
void qso_data::action_cancel_modem() {
	if (current_qso()->item("QSO_COMPLETE") == "") {
		// Complete so should save it
		action_save();
		book_->enable_save(true);
	}
	else {
		action_cancel();
	}
}

// Action PEEK - interrupt current state and peek at supplied qso
void qso_data::action_peek(qso_num_t number) {
	// SAve the current state unless it is already peeking
	// Either QSO_PEEK or QSO_PEEK_ED
	switch (logging_state_) {
	case QSO_INACTIVE:
	case QSO_PENDING:
	case QSO_EDIT:
	case QSO_BROWSE:
	case NET_EDIT:
		interrupted_state_ = logging_state_;
		logging_state_ = QSO_PEEK_ED;
		break;
	case QSO_STARTED:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QRZ_MERGE:
	case NET_STARTED:
	case NET_ADDING:
	case SWITCHING:
	case QSO_MODEM:
		interrupted_state_ = logging_state_;
		logging_state_ = QSO_PEEK;
		break;
	}
	g_peek_->qso(number);
	g_peek_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action CANCEL_PEEK - restore interrupted state
void qso_data::action_cancel_peek() {
	logging_state_ = interrupted_state_;
	enable_widgets();
}

// Action EDIT_PEEK - close down existing edit
void qso_data::action_edit_peek() {
	bool existing_entry = false;
	switch (interrupted_state_) {
	case QSO_EDIT:
		if (g_entry_->qso()->is_dirty()) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while editing a record", "Save edit", "Cancel edit", nullptr)) {
			case 0:
				// Save QSO
				action_save_edit();
				break;
			case 1:
				// Cancel QSO
				action_cancel_edit();
				break;
			}
		}
		else {
			action_cancel_edit();
		}
	case NET_EDIT:
		if (!g_net_entry_->qso_in_net(g_peek_->qso_number())) {
			// Selected QSO is not part of the net, save or cancel the net
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Trying to select a different record while logging a net", "Save Net", "Quit Net", nullptr)) {
			case 0:
				// Save QSO
				action_save_net_all();
				break;
			case 1:
				// Cancel QSO
				action_cancel_net_all();
				break;
			}
			// Actions will have changed selection - change it back.
			logging_state_ = QSO_INACTIVE;
		}
		else {
			// Switch to the selected QSO as part of the net if necessary
			if (g_peek_->qso_number() != g_net_entry_->qso_number()) {
				g_net_entry_->select_qso(g_peek_->qso_number());
			}
			g_net_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			existing_entry = true;
			logging_state_ = NET_EDIT;
		}
	}
	// Save a copy of the current record
	if (!existing_entry) {
		edit_return_state_ = logging_state_;
		logging_state_ = QSO_EDIT;
		g_entry_->qso(g_peek_->qso_number());
		g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	}
	enable_widgets();
}

// Create a query entry
void qso_data::action_query_entry() {
	g_qy_entry_->qso(-1);
	logging_state_ = MANUAL_ENTRY;
	g_qy_entry_->copy_default_to_qso();
	g_qy_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
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
		snprintf(msg, sizeof(msg), "DASH: %d matching records found", extract_records_->size());
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
	g_qy_entry_->delete_qso();
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Import the query
void qso_data::action_import_query() {
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();
	import_data_->load_record(g_qy_entry_->qso());
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
			//if (band_view_) {
			//	double freq;
			//	g_entry_->qso()->item("FREQ", freq);;
			//	band_view_->update(freq * 1000.0);
			//	prev_freq_ = freq;
			//}
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
	case QSO_EDIT:
	case QSO_MODEM:
		return g_entry_->qso();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QRZ_MERGE:
		return g_query_->qso();
	case NET_STARTED:
	case NET_EDIT:
		return g_net_entry_->qso();
	case QSO_PEEK:
	case QSO_PEEK_ED:
		return g_peek_->qso();
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
	case QSO_EDIT:
	case QSO_MODEM:
		return g_entry_->qso_number();
	case QSO_BROWSE:
	case QUERY_DUPE:
	case QUERY_MATCH:
	case QUERY_NEW:
	case QRZ_MERGE:
		return g_query_->qso_number();
	case NET_STARTED:
	case NET_EDIT:
		return g_net_entry_->qso_number();
	case QSO_PEEK:
	case QSO_PEEK_ED:
		return g_peek_->qso_number();
	case MANUAL_ENTRY:
		return g_qy_entry_->qso_number();
	default:
		return book_->selection();
	}
}

// Update time and rig info
void qso_data::ticker() {
	switch (previous_mode_) {
	case QSO_ON_AIR:
		g_entry_->copy_clock_to_qso();
		g_net_entry_->ticker();
		g_entry_->copy_cat_to_qso();
		break;
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

// Contest mode
qso_contest::contest_mode_t qso_data::contest_mode() {
	return g_contest_->mode();
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

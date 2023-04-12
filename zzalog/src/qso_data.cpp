#include "qso_data.h"
#include "qso_manager.h"
#include "qsl_viewer.h"
#include "qth_dialog.h"
#include "settings.h"
#include "rig_if.h"
#include "book.h"
#include "status.h"
#include "extract_data.h"
#include "pfx_data.h"
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
extern pfx_data* pfx_data_;
extern spec_data* spec_data_;
extern band_view* band_view_;
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
	, current_qso_(nullptr)
	, original_qso_(nullptr)
	, logging_mode_(LM_OFF_AIR)
	, logging_state_(QSO_INACTIVE)
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
	int new_lm;
	rig_if* rig = ((qso_manager*)parent())->rig();
	bool have_rig = rig && rig->is_good();
	logging_mode_t default_lm = have_rig ? LM_ON_AIR_CAT : LM_ON_AIR_COPY;
	dash_settings.get("Logging Mode", new_lm, default_lm);

	// If we are set to "On-air with CAT connection" check connecton
	if (!have_rig && new_lm == LM_ON_AIR_CAT) new_lm = LM_ON_AIR_COPY;

	logging_mode_ = (logging_mode_t)new_lm;

}

// Create qso_data
void qso_data::create_form(int X, int Y) {
	int max_w = 0;

	begin();
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	label("QSO Data");
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);

	// Choice widget to select the reqiuired logging mode
	ch_logmode_ = new Fl_Choice(x() + GAP + WLLABEL, y() + HTEXT, 8 * WBUTTON, HTEXT, "QSO initialisation");
	ch_logmode_->align(FL_ALIGN_LEFT);
	ch_logmode_->add("Current date and time - used for parsing only");
	ch_logmode_->add("All fields blank");
	ch_logmode_->add("Current date and time, data from CAT");
	ch_logmode_->add("Current date and time, data from selected QSO - including call");
	ch_logmode_->add("Current date and time, data from selected QSO - excluding call");
	ch_logmode_->add("Current date and time, no other data");
	ch_logmode_->value(logging_mode_);
	ch_logmode_->callback(cb_logging_mode, &logging_mode_);
	ch_logmode_->tooltip("Select the logging mode - i.e. how to initialise a new QSO record");

	int curr_y = ch_logmode_->y() + ch_logmode_->h() + GAP;;
	int top = ch_logmode_->y() + ch_logmode_->h();
	int curr_x = X + GAP;

	g_contest_ = new qso_contest(curr_x, curr_y, 10, 10);

	max_w = max(max_w, g_contest_->x() + g_contest_->w() + GAP);
	curr_y = g_contest_->y() + g_contest_->h() + GAP;

	// One or the other of the two groups below will be shown at a time
	g_entry_ = new qso_entry(curr_x, curr_y, 10, 10);

	max_w = max(max_w, g_entry_->x() + g_entry_->w() + GAP);
	g_query_ = new qso_query(curr_x, curr_y, 10, 10);

	max_w = max(max_w, g_query_->x() + g_query_->w() + GAP);
	curr_y = max(g_entry_->y() + g_entry_->h(), g_query_->y() + g_query_->h()) + GAP;

	g_buttons_ = new qso_buttons(curr_x, curr_y, 10, 10);

	max_w = max(max_w, g_buttons_->x() + g_buttons_->w() + GAP);
	curr_y += g_buttons_->h() + GAP;

	ch_logmode_->size(max_w - ch_logmode_->x() - GAP, ch_logmode_->h());

	resizable(nullptr);
	size(max_w, curr_y - Y);
	end();

	initialise_fields();
}

// Enable QSO widgets
void qso_data::enable_widgets() {
	// Disable log mode menu item from CAT if no CAT
	rig_if* rig = ((qso_manager*)parent())->rig();
	if (rig == nullptr || !rig->is_good()) {
		ch_logmode_->mode(LM_ON_AIR_CAT, ch_logmode_->mode(LM_ON_AIR_CAT) | FL_MENU_INACTIVE);
	}
	else {
		ch_logmode_->mode(LM_ON_AIR_CAT, ch_logmode_->mode(LM_ON_AIR_CAT) & ~FL_MENU_INACTIVE);
	}
	ch_logmode_->redraw();

	g_contest_->enable_widgets();
	g_entry_->enable_widgets();
	g_query_->enable_widgets();
	g_buttons_->enable_widgets();

}

// Update QSO
void qso_data::update_qso(qso_num_t log_qso) {
	if (log_qso == current_rec_num_) {
		switch (logging_state_) {
		case QSO_INACTIVE:
			// Do nothing
			break;
		case QSO_PENDING:
		case QSO_STARTED:
		case QSO_EDIT:
			// Update the view if another view changes the record
			g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
			redraw();
			break;
		case QSO_BROWSE:
			g_query_->redraw();
			break;
		}
	}
	else {
		switch (logging_state_) {
		case QSO_INACTIVE:
			// Do nothing
			break;
		case QSO_PENDING:
			// Deactivate then reactivate with new QSPO
			action_deactivate();
			current_rec_num_ = log_qso;
			current_qso_ = book_->get_record(log_qso, false);
			action_activate();
			break;
		case QSO_STARTED:
			// Ack whether to save or quit then activate new QSO
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
			current_rec_num_ = log_qso;
			current_qso_ = book_->get_record(log_qso, true);
			break;
		case QSO_EDIT:
			// Ask whether to save or quit then open new QSO in edit mode
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
			current_rec_num_ = log_qso;
			current_qso_ = book_->get_record(log_qso, true);
			action_edit();
			break;
		case QSO_BROWSE:
			// Open new record in browse mode
			action_cancel_browse();
			current_rec_num_ = log_qso;
			current_qso_ = book_->get_record(log_qso, true);
			action_browse();
			break;
		}
	}
}

// Update query
void qso_data::update_query(logging_state_t query, qso_num_t match_num, qso_num_t query_num) {
	switch (logging_state_) {
	case QSO_PENDING:
	case QSO_INACTIVE:
		current_rec_num_ = match_num;
		query_rec_num_ = query_num;
		action_query(query);
		break;
	case QUERY_NEW:
		action_query(query);
		break;
	default:
		// TODO:
		break;
	}
}

// Select logging mode
void qso_data::cb_logging_mode(Fl_Widget* w, void* v) {
	cb_value<Fl_Choice, logging_mode_t>(w, v);
	qso_data* that = ancestor_view<qso_data>(w);
	// Deactivate and reactivate to pick up logging mode changes
	if (that->logging_state_ == QSO_PENDING) {
		that->action_deactivate();
		that->action_activate();
	}
}

// Callback - QSL viewer "closing" - make it hide instead
void qso_data::cb_qsl_viewer(Fl_Widget* w, void* v) {
	qsl_viewer* qsl = (qsl_viewer*)w;
	if (qsl->visible()) qsl->hide();
}

// Save the settings
void qso_data::save_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	dash_settings.set("Logging Mode", (int)logging_mode_);
}

// Initialise fields from format definitions
void qso_data::initialise_fields() {
	string preset_fields;
	bool lock_preset;
	bool new_fields;
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
		preset_fields = "RST_RCVD,SRX";
		new_fields = true;
		lock_preset = false;
		break;
	case qso_contest::EDIT:
		// Unlock existing definition 
		preset_fields = "";
		new_fields = false;
		lock_preset = false;
		break;
	}
	g_entry_->initialise_fields(preset_fields, new_fields, lock_preset);
	// TODO this shoule be somewhere else
	//// Set contest format
	//ch_format_->value(exch_fmt_id_.c_str());
	// Default Contest TX values
	if (g_contest_->mode() == qso_contest::CONTEST) {
		// Automatically create a pending QSO
		if (logging_state_ == QSO_INACTIVE) {
			action_set_current();
			action_activate();
		}
	}
	g_entry_->initialise_values(preset_fields, g_contest_->serial_number());
}

string qso_data::get_defined_fields() {
	return g_entry_->get_defined_fields();
}

// Get default record to copy
record* qso_data::get_default_record() {
	switch (logging_state_) {
	case QSO_INACTIVE:
		switch (logging_mode_) {
		case LM_OFF_AIR:
		case LM_ON_AIR_CAT:
		case LM_ON_AIR_TIME:
			current_rec_num_ = book_->size() - 1;
			return book_->get_latest();
		case LM_ON_AIR_COPY:
		case LM_ON_AIR_CLONE:
			current_rec_num_ = book_->selection();
			return book_->get_record();
		default:
			return nullptr;
		}
	default:
		return current_qso_;
	}
}

// Action ACTIVATE: transition from QSO_INACTIVE to QSO_PENDING
void qso_data::action_activate() {
	record* source_record = current_qso_;
	current_qso_ = new record;
	current_rec_num_ = -1;
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	logging_state_ = QSO_PENDING;
	switch (logging_mode_) {
	case LM_OFF_AIR:
		// Just copy the station details
		g_entry_->copy_qso_to_qso(source_record, qso_entry::CF_RIG_ETC);
		break;
	case LM_ON_AIR_CAT:
		// Copy station details and get read rig details for band etc.
		g_entry_->copy_qso_to_qso(source_record, qso_entry::CF_RIG_ETC);
		g_entry_->copy_cat_to_qso();
		g_entry_->copy_clock_to_qso();
		break;
	case LM_ON_AIR_CLONE:
		// Clone the QSO - get station and band from original QSO
		g_entry_->copy_qso_to_qso(source_record, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT);
		g_entry_->copy_clock_to_qso();
		break;
	case LM_ON_AIR_COPY:
		// Copy the QSO - as abobe but also same callsign and details
		g_entry_->copy_qso_to_qso(source_record, qso_entry::CF_RIG_ETC | qso_entry::CF_CAT | qso_entry::CF_CONTACT);
		g_entry_->copy_clock_to_qso();
		break;
	case LM_ON_AIR_TIME:
		// Copy the station details and set the current date/time.
		g_entry_->copy_qso_to_qso(source_record, qso_entry::CF_RIG_ETC);
		g_entry_->copy_clock_to_qso();
		break;
	}
	initialise_fields();
	enable_widgets();
}

// Action START - transition from QSO_PENDING to QSO_STARTED
void qso_data::action_start() {
	// Add to book
	current_rec_num_ = book_->append_record(current_qso_);
	book_->selection(book_->item_number(current_rec_num_), HT_INSERTED);
	logging_state_ = QSO_STARTED;
	enable_widgets();
}

// Action SAVE - transition from QSO_STARTED to QSO_INACTIVE while saving record
void qso_data::action_save() {
	bool old_save_enabled = book_->save_enabled();
	item_num_t item_number = book_->item_number(current_rec_num_);
	book_->enable_save(false);
	// On-air logging add date/time off
	switch (logging_mode_) {
	case LM_ON_AIR_CAT:
	case LM_ON_AIR_COPY:
	case LM_ON_AIR_CLONE:
	case LM_ON_AIR_TIME:
		// All on-air modes - set cime-off to now
		if (current_qso_->item("TIME_OFF") == "") {
			// Add end date/time - current time of interactive entering
			// Get current date and time in UTC
			string timestamp = now(false, "%Y%m%d%H%M%S");
			current_qso_->item("QSO_DATE_OFF", timestamp.substr(0, 8));
			// Time as HHMMSS - always log seconds.
			current_qso_->item("TIME_OFF", timestamp.substr(8));
		}
		// Increment contest serial number
		g_contest_->increment_serial();
		break;
	case LM_OFF_AIR:
		book_->correct_record_position(item_number);
		break;
	}

	// check whether record has changed - when parsed
	if (pfx_data_->parse(current_qso_)) {
	}

	// check whether record has changed - when validated
	if (spec_data_->validate(current_qso_, item_number)) {
	}

	book_->add_use_data(current_qso_);

	// Upload QSO to QSL servers
	book_->upload_qso(current_rec_num_);
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();

	// If new or changed then update the fact and let every one know
	book_->modified(true);
	book_->selection(item_number, HT_INSERTED);
	book_->enable_save(old_save_enabled);
}

// Action CANCEL - Transition from QSO_STARTED to QSO_INACTIVE without saving record
void qso_data::action_cancel() {
	// book_->delete_record() will change the selected record - we need ti be inactive to ignore it
	logging_state_ = QSO_INACTIVE;
	book_->delete_record(true);
	delete current_qso_;
	current_qso_ = nullptr;
	check_qth_changed();
	enable_widgets();
}

// Action DEACTIVATE - Transition from QSO_PENDING to QSO_INACTIVE
void qso_data::action_deactivate() {
	delete current_qso_;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// Action EDIT - Transition from QSO_INACTIVE to QSO_EDIT
void qso_data::action_edit() {
	// Save a copy of the current record
	original_qso_ = new record(*current_qso_);
	logging_state_ = QSO_EDIT;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action SAVE EDIT - Transition from QSO_EDIT to QSO_INACTIVE while saving changes
void qso_data::action_save_edit() {
	// We no longer need to maintain the copy of the original QSO
	book_->add_use_data(current_qso_);
	book_->modified(true);
	delete original_qso_;
	original_qso_ = nullptr;
	current_qso_ = nullptr;
	current_rec_num_ = -1;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
}

// ACtion CANCEL EDIT - Transition from QSO_EDIT to QSO_INACTIVE scrapping changes
void qso_data::action_cancel_edit() {
	// Copy original back to the book
	*book_->get_record(current_rec_num_, false) = *original_qso_;
	delete original_qso_;
	original_qso_ = nullptr;
	current_qso_ = nullptr;
	current_rec_num_ = -1;
	logging_state_ = QSO_INACTIVE;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action CANCEL in BROWSE 
void qso_data::action_cancel_browse() {
	current_qso_ = nullptr;
	current_rec_num_ = -1;
	logging_state_ = QSO_INACTIVE;
	g_entry_->copy_qso_to_display(qso_entry::CF_ALL_FLAGS);
	enable_widgets();
}

// Action navigate button
void qso_data::action_navigate(int target) {
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
	book_->navigate((navigate_t)target);
	action_set_current();
	// And restore state
	switch (saved_state) {
	case QSO_EDIT:
		action_edit();
		action_view_qsl();
		break;
	case QSO_PENDING:
		action_activate();
		action_view_qsl();
		break;
	case QSO_BROWSE:
		action_browse();
		action_view_qsl();
		break;
	case QUERY_MATCH:
		current_rec_num_ = book_->selection();
		action_query(saved_state);
		break;
	}
}

// Action view qsl
void qso_data::action_view_qsl() {
	switch (logging_state_) {
	case QSO_EDIT:
	case QSO_BROWSE:
		if (current_qso_) {
			char title[128];
			snprintf(title, 128, "QSL Status: %s %s %s %s %s",
				current_qso_->item("CALL").c_str(),
				current_qso_->item("QSO_DATE").c_str(),
				current_qso_->item("TIME_ON").c_str(),
				current_qso_->item("BAND").c_str(),
				current_qso_->item("MODE", true, true).c_str());
			qsl_viewer_->copy_label(title);
			qsl_viewer_->set_qso(current_qso_, current_rec_num_);
			qsl_viewer_->show();
			char msg[128];
			snprintf(msg, 128, "DASH: %s", title);
			status_->misc_status(ST_LOG, msg);
		}
		break;
	default:
		status_->misc_status(ST_SEVERE, "DASH: No QSO selected - cannot view QSL status");
		break;
	}
}


// Action browse
void qso_data::action_browse() {
	logging_state_ = QSO_BROWSE;
	original_qso_ = nullptr;
	query_qso_ = nullptr;
	g_query_->label("Browsing record");
	enable_widgets();
}

// Action query
void qso_data::action_query(logging_state_t query) {
	switch (query) {
	case QUERY_MATCH:
		current_qso_ = book_->get_record(current_rec_num_, false);
		// And save a copy of it
		original_qso_ = new record(*current_qso_);
		query_qso_ = import_data_->get_record(query_rec_num_, false);
		g_query_->copy_label(import_data_->match_question().c_str());
		break;
	case QUERY_NEW:
		current_qso_ = nullptr;
		original_qso_ = nullptr;
		query_qso_ = import_data_->get_record(query_rec_num_, false);;
		g_query_->copy_label(import_data_->match_question().c_str());
		break;
	case QUERY_DUPE:
		// Note record numbers relate to book even if it is extracted data that refered the dupe check
		current_qso_ = book_->get_record(current_rec_num_, false);
		original_qso_ = new record(*current_qso_);
		query_qso_ = book_->get_record(query_rec_num_, false);
		g_query_->copy_label(navigation_book_->match_question().c_str());
		break;
	case QRZ_MERGE:
		current_qso_ = book_->get_record(current_rec_num_, false);
		original_qso_ = new record(*current_qso_);
		query_qso_ = qrz_handler_->get_record();
		g_query_->copy_label(qrz_handler_->get_merge_message().c_str());
		break;

	default:
		// TODO trap this sensibly
		return;
	}
	logging_state_ = query;
	enable_widgets();
	// Move window to top
	parent()->show();
}

// Action add query - add query QSO to book
void qso_data::action_add_query() {
	import_data_->save_update();
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();

}

// Action reject query - do nothing
void qso_data::action_reject_query() {
	import_data_->discard_update(true);
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// Action merge query
void qso_data::action_merge_query() {
	import_data_->merge_update();
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the update process
	import_data_->update_book();
}

// ACtion find match
void qso_data::action_find_match() {
	update_query(QUERY_MATCH, current_rec_num_, query_rec_num_);
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
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	// Restart the duplicate check process
	navigation_book_->check_dupes(true);

}

// Action table double click
void qso_data::action_handle_dclick(int col, string field) {
	switch (logging_state_) {
	case QUERY_MATCH:
	case QUERY_DUPE:
	case QRZ_MERGE:
		switch (col) {
		case 0:
			// Treat as if clicking row header
			g_entry_->action_add_field(-1, field);
			break;
		case 1:
			// Copy log field
			current_qso_->item(field, query_qso_->item(field));
			break;
		case 2:
			// Copy original record
			current_qso_->item(field, original_qso_->item(field));
			break;
		}
	}
	enable_widgets();
}

// Action save as a result of a merge
void qso_data::action_save_merge() {
	// We no longer need to maintain the copy of the original QSO
	book_->add_use_data(current_qso_);
	book_->modified(true);
	delete original_qso_;
	original_qso_ = nullptr;
	current_qso_ = nullptr;
	logging_state_ = QSO_INACTIVE;
	enable_widgets();
	qrz_handler_->merge_done();
}

// ACtion look in all.txt
void qso_data::action_look_all_txt() {
	Fl_Preferences datapath_settings(settings_, "Datapath");
	char* temp;
	datapath_settings.get("WSJT-X", temp, "");
	string filename = string(temp) + "/all.txt";
	ifstream* all_file = new ifstream(filename.c_str());
	// This will take a while so display the timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	// calculate the file size and initialise the progress bar
	streampos startpos = all_file->tellg();
	all_file->seekg(0, ios::end);
	streampos endpos = all_file->tellg();
	long file_size = (long)(endpos - startpos);
	status_->misc_status(ST_NOTE, "LOG: Starting to parse all.txt");
	status_->progress(file_size, OT_RECORD, "Looking for queried call in WSJT-X data", "bytes");
	// reposition back to beginning
	all_file->seekg(0, ios::beg);
	bool start_copying = false;
	bool stop_copying = false;
	// Get user callsign from settings
	string my_call = ((qso_manager*)parent())->get_default(qso_manager::CALLSIGN);
	// Get search items from record
	string their_call = query_qso_->item("CALL");
	string datestamp = query_qso_->item("QSO_DATE").substr(2);
	string timestamp = query_qso_->item("TIME_ON");
	string mode = query_qso_->item("MODE");
	char msg[256];
	// Mark QSO incomplete 
	query_qso_->item("QSO_COMPLETE", string("N"));
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
			start_copying = true;
			snprintf(msg, 256, "DASH: %s %s %s %s %s Found possible match",
				datestamp.c_str(),
				timestamp.c_str(),
				mode.c_str(),
				my_call.c_str(),
				their_call.c_str());
			status_->misc_status(ST_NOTE, msg);
		}
		if (start_copying) {
			if (line.find(my_call) != string::npos &&
				line.find(their_call) != string::npos) {
				// It has both calls - copy to buffer, and parse for QSO details (report, grid and QSO completion
				snprintf(msg, 256, "DASH: %s", line.c_str());
				status_->misc_status(ST_LOG, msg);
				action_copy_all_text(line);
			}
			else if (line.find(my_call) == string::npos &&
				line.find(their_call) == string::npos) {
				// It has neither call - ignore
			}
			else {
				// It has one or the other call - indicates QSO complete
				stop_copying = true;
				query_qso_->item("QSO_COMPLETE", string(""));
			}
		}
	}
	if (stop_copying) {
		status_->progress("Found record!", OT_RECORD);
		// If we are complete then say so
		if (query_qso_->item("QSO_COMPLETE") != "N" && query_qso_->item("QSO_COMPLETE") != "?") {
			all_file->close();
			fl_cursor(FL_CURSOR_DEFAULT);
		}
	}
	all_file->close();
	snprintf(msg, 100, "LOG: Cannot find contact with %s in WSJT-X text.all file.", their_call.c_str());
	status_->progress("Did not find record", OT_RECORD);
	status_->misc_status(ST_WARNING, msg);
	fl_cursor(FL_CURSOR_DEFAULT);
}

void qso_data::action_copy_all_text(string text) {
	bool tx_record;
	// After this initial processing pos will point to the start og the QSO decode string - look for old-style transmit record
	size_t pos = text.find("Transmitting");
	if (pos != string::npos) {
		pos = text.find(query_qso_->item("MODE"));
		pos += query_qso_->item("MODE").length() + 3;
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
			query_qso_->item("FREQ", to_string(frequency));
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
		if (query_qso_->item("QSO_COMPLETE") == "?") {
			query_qso_->item("QSO_COMPLETE", string(""));
		}
		else if (query_qso_->item("QSO_COMPLETE") == "N") {
			query_qso_->item("QSO_COMPLETE", string("?"));
		}
	}
	else if (report == "73") {
		// A 73 definitely indicates QSO compplete
		query_qso_->item("QSO_COMPLETE", string(""));
	}
	else if (report[0] == 'R') {
		// The first of the rogers
		if (query_qso_->item("QSO_COMPLETE") == "N") {
			query_qso_->item("QSO_COMPLETE", string("?"));
		}
		// Update reports if they've not been provided
		if (tx_record && !query_qso_->item_exists("RST_SENT")) {
			query_qso_->item("RST_SENT", report.substr(1));
		}
		else if (!tx_record && !query_qso_->item_exists("RST_RCVD")) {
			query_qso_->item("RST_RCVD", report.substr(1));
		}
	}
	else if (!tx_record && !query_qso_->item_exists("GRIDSQUARE")) {
		// Update gridsquare if it's not been provided
		query_qso_->item("GRIDSQUARE", report);
	}
	else if (report[0] == '-' || (report[0] >= '0' && report[0] <= '9')) {
		// Numeric report
		if (tx_record && !query_qso_->item_exists("RST_SENT")) {
			query_qso_->item("RST_SENT", report);
		}
		else if (!tx_record && !query_qso_->item_exists("RST_RCVD")) {
			query_qso_->item("RST_RCVD", report);
		}
	}

}

// Set current QSO from selected record
void qso_data::action_set_current() {
	current_qso_ = book_->get_record();
	current_rec_num_ = book_->selection();
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

// Check if QTH has changed and action change (redraw DxAtlas
void qso_data::check_qth_changed() {
	record* current = get_default_record();
	if (current) {
		if (current->item("MY_GRIDSQUARE", true) != previous_locator_ ||
			current->item("APP_ZZA_QTH") != previous_qth_) {
			previous_locator_ = current->item("MY_GRIDSQUARE", true);
			previous_qth_ = current->item("APP_ZZA_QTH");
#ifdef _WIN32
			if (dxa_if_) dxa_if_->update(HT_LOCATION);
#endif
		}
	}
}

void qso_data::update_rig() {
	// Get freq etc from QSO or rig
// Get present values data from rig
	if (logging_state_ == QSO_PENDING) {
		switch (logging_mode_) {
		case LM_OFF_AIR:
		case LM_ON_AIR_TIME:
			// Do nothing
			break;
		case LM_ON_AIR_CAT: {
			if (((qso_manager*)parent())->rig()->is_good()) {
				g_entry_->copy_cat_to_qso();
				if (band_view_) {
					double freq;
					current_qso_->item("FREQ", freq);;
					band_view_->update(freq * 1000.0);
					prev_freq_ = freq;
				}
			}
			else {
				// We have now disconnected rig - disable selecting this logging mode
				logging_mode_ = LM_ON_AIR_TIME;
				enable_widgets();
			}
			break;
		}
		case LM_ON_AIR_COPY:
		case LM_ON_AIR_CLONE:
		{
			break;
		}
		}
	}
}

// Start a QSO as long as we are in the correct state
void qso_data::start_qso() {
	switch (logging_state_) {
	case qso_data::QSO_INACTIVE:
		action_activate();
		// drop through
	case qso_data::QSO_PENDING:
		action_start();
		break;
	case qso_data::QSO_STARTED:
		status_->misc_status(ST_ERROR, "DASH: Cannot start a QSO when one already started");
		break;
	case qso_data::QSO_EDIT:
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
		action_start();
		// drop through
	case qso_data::QSO_STARTED:
		action_save();
		break;
	case qso_data::QSO_EDIT:
		action_save_edit();
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
		status_->misc_status(ST_ERROR, "DASH: Cannot edit a QSO when in on-air");
		break;
	case qso_data::QSO_EDIT:
		status_->misc_status(ST_ERROR, "DASH: Cannot edit another QSO while editing an existing one");
		break;
	}
}

// Get logging mode
qso_data::logging_mode_t qso_data::logging_mode() {
	return logging_mode_;
}

// Set logging mode
void qso_data::logging_mode(qso_data::logging_mode_t mode) {
	logging_mode_ = mode;
}

// Get logging state
qso_data::logging_state_t qso_data::logging_state() {
	return logging_state_;
}

// Current QSO
record* qso_data::current_qso() {
	return current_qso_;
}

// Current QSO
record* qso_data::original_qso() {
	return original_qso_;
}

// Current QSO
record* qso_data::query_qso() {
	return query_qso_;
}

// Current QSO record number
qso_num_t qso_data::current_qso_num() {
	return current_rec_num_;
}

// Update time
void qso_data::ticker() {
	g_entry_->copy_clock_to_qso();
	g_entry_->copy_cat_to_qso();
}

// Call in editor
string qso_data::get_call() {
	return g_entry_->get_call();
}
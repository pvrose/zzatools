#include "import_data.h"
#include "pfx_data.h"
#include "spec_data.h"
#include "tabbed_forms.h"
#include "status.h"
#include "scratchpad.h"
#include "eqsl_handler.h"
#include "lotw_handler.h"
#include "rig_if.h"
#include "../zzalib/utils.h"
#include "menu.h"

#include <sstream>
#include <ctime>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>

using namespace zzalog;

extern status* status_;
extern book* book_;
extern Fl_Preferences* settings_;
extern pfx_data* pfx_data_;
extern spec_data* spec_data_;
extern tabbed_forms* tabbed_view_;
extern eqsl_handler* eqsl_handler_;
extern lotw_handler* lotw_handler_;
extern rig_if* rig_if_;
extern menu* menu_;
extern scratchpad* scratchpad_;
extern void add_rig_if();

// Constructor
import_data::import_data() :
	book()
{
	// Initialise
	book_type_ = OT_IMPORT;
	update_files_ = nullptr;
	empty_files_ = nullptr;
	close_pending_ = false;
	update_in_progress_ = false;
	update_is_new_ = false;
	update_mode_ = NONE;
	number_to_import_ = 0;
	number_updated_ = 0;
	number_accepted_ = 0;
	number_checked_ = 0;
	number_added_ = 0;
	number_rejected_ = 0;
	num_update_files_ = 0;
	update_files_ = nullptr;
	empty_files_ = nullptr;
	sources_ = nullptr;
	match_question_ = "";
	close_pending_ = false;
	next_logging_mode_ = LM_OFF_AIR;
	timer_count_ = nan("");
	Fl_Preferences update_settings(settings_, "Real Time Update");
	int enable;
	update_settings.get("Enabled", enable, (int)true);
	auto_enable_ = (bool)enable;
}

// Destructor
import_data::~import_data()
{
	delete[] empty_files_;
	delete[] update_files_;
	Fl_Preferences update_settings(settings_, "Real Time Update");
	update_settings.set("Enabled", auto_enable_);
}

// Timer callback - Called every second so that a countdown can be displayed
void import_data::cb_timer_imp(void* v) {
	import_data* that = (import_data*)v;
	that->timer_count_--;
	if (that->timer_count_ <= 0.0) {
		// Countdown reached zero so update
		that->auto_update();
	}
	else {
		// Restart the timer
		Fl::repeat_timeout(1.0, cb_timer_imp, v);
		status_->progress((int)that->timer_count_);
	}
}

// Start the auto update process
bool import_data::start_auto_update() {
	// Tell user
	status_->misc_status(ST_NOTE, "AUTO IMPORT: Started");
	close_pending_ = false;
	update_mode_ = WAIT_AUTO;

	// Get the number of files to import
	Fl_Preferences update_settings(settings_, "Real Time Update");
	Fl_Preferences files_settings(update_settings, "Files");
	num_update_files_ = files_settings.groups();
	if (num_update_files_ > 0) {
		// Create auto-import information
		update_files_ = new string[num_update_files_];
		empty_files_ = new int[num_update_files_];
		sources_ = new string[num_update_files_];
		last_timestamps_ = new string[num_update_files_];
		// For each auto-impor file
		for (int i = 0; i < num_update_files_; i++) {
			// Get the information
			sources_[i] = files_settings.group(i);
			Fl_Preferences file_settings(files_settings, sources_[i].c_str());
			char * temp;
			file_settings.get("Filename", temp, "");
			update_files_[i] = temp;
			free(temp);
			file_settings.get("Empty On Read", empty_files_[i], false);
			// Arbbitrary previous timestamp
			file_settings.get("Timestamp", temp, "20190601000000");
			last_timestamps_[i] = temp;
			free(temp);
		}
		// Set immediate timer to actuate the auto-import
		timer_count_ = 0.0;
		Fl::add_timeout(0.0, import_data::cb_timer_imp, (void*)this);
		menu_->logging(LM_IMPORTED);
	}
	else {
		// No files to update - tell calling routine to open rig again and also change flag in menu
		status_->misc_status(ST_WARNING, "AUTO IMPORT: No files available to import.");
		auto_enable_ = false;
	}
	menu_->update_items();
	return auto_enable_;
}

// Start an automatic (on timer) update from the defined locations
void import_data::auto_update() {
	// Report auto-update
	status_->misc_status(ST_NOTE, "AUTO IMPORT: File load started");
	update_mode_ = READ_AUTO;

	delete_contents(true);
	// Set last timestamp to the latest it can be
	last_timestamp_ = now(false, "%Y%m%d%H%M%S");
	// For each auto-import file
	for (int i = 0; i < num_update_files_; i++) {
		// Timer will only restart when update is complete
		bool failed = false;
		char timestamp[16];
#ifdef _WIN32
		int fd = _sopen(update_files_[i].c_str(), _O_RDONLY, _SH_DENYNO);
		struct _stat status;
		int result = _fstat(fd, &status);
		strftime(timestamp, 16, "%Y%m%d%H%M%S", gmtime(&status.st_mtime));
#else
		// TODO: Code Posix version of the above
#endif
		if (timestamp > last_timestamps_[i]) {
			// Load data from the auto-import file - concatenating all files
			char message[256];
			sprintf(message, "AUTO IMPORT: %s (%s)", update_files_[i].c_str(), sources_[i].c_str());
			status_->misc_status(ST_NOTE, message);
			load_data(update_files_[i]);
			last_timestamps_[i] = timestamp;
			// Remember the earliest of the files that has changed
			if (timestamp < last_timestamp_) {
				last_timestamp_ = timestamp;
			}
		}
		else {
			char message[256];
			sprintf(message, "AUTO IMPORT: %s not changed", update_files_[i].c_str());
			status_->misc_status(ST_WARNING, message);
		}
	}
	if (size()) {
		// Tell user and merge records from the auto-import
		update_mode_ = AUTO_IMPORT;
		status_->misc_status(ST_NOTE, "AUTO IMPORT: merging data");
		status_->progress(size(), book_type_, "records");
		number_checked_ = 0;
		number_accepted_ = 0;
		number_updated_ = 0;
		number_added_ = 0;
		number_rejected_ = 0;
		number_to_import_ = size();
		update_book();
	}
	else {
		// Tell user no data and restart the timer. Even though nothing to import we need to set AUTO_IMPORT.
		update_mode_ = AUTO_IMPORT;
		status_->misc_status(ST_WARNING, "AUTO IMPORT: No data - skipped");
		finish_update(false);
	}
}

// Delete the mismatch record in the update - delete it and erase 
void import_data::discard_update(bool notify /*= true*/) {
	// Delete the record and remove from this book
	auto it = begin();
	delete *it;
	erase(it);
	if (notify) {
		number_rejected_++;
	}
}

// Accept the record from the update - it will have been copied to main log book
void import_data::accept_update() {
	// Reove the record from the book
	auto it = begin();
	erase(it);
}

// Combine the record from the log and the update
void import_data::merge_update() {
	// Merge the first record in this book into the selected record from the main log book
	record* import_record = at(0);
	record* book_record = book_->get_record();
	// Grid square may change (more accurate)
	hint_t hint;
	book_record->merge_records(import_record, false, &hint);
	if (pfx_data_->parse(book_record) == PR_CHANGED) {
		book_->modified(true, false);
	}
	if (spec_data_->validate(book_record, book_->selection())) {
		book_->modified(true, false);
	}
	// Tell views to update with new record
	book_->selection(-1, hint);
	// Delete the record from this book
	discard_update(false);
	number_accepted_++;
}

// Add the update record to the log - called by record_form
void import_data::save_update() {
	// Get parse and validation settings
	record* import_record = at(0);
	if (pfx_data_->parse(import_record) == PR_CHANGED) {
	}
	int record_number = book_->insert_record(import_record);
	if (spec_data_->validate(import_record, record_number)) {
	}
	// This may result in the card beng fetched twice
	if (update_mode_ == EQSL_UPDATE) {
		eqsl_handler_->enqueue_request(record_number);
	}
	// Copy record to the main log
	book_->insert_record(import_record);
	accept_update();
	book_->selection(record_number, HT_INSERTED);
}

// repeat the auto-import timer
void import_data::repeat_auto_timer() {
	// Get the polling interval from settings
	Fl_Preferences update_settings(settings_, "Real Time Update");
	update_settings.get("Polling Interval", timer_count_, AUTO_IP_DEF);

	// Restart the timer
	Fl::repeat_timeout(1.0, cb_timer_imp, (void*)this);
	// Tell user - display countdown so that the bar gets bigger as it counts down the seconds
	status_->progress((int)timer_count_, OT_IMPORT, "seconds", true);
	update_mode_ = WAIT_AUTO;
}

// Start or continue analysing the update data  
void import_data::update_book() {
	if (size() == 0) {
		// We have finished updating as there are no records left
		finish_update();
	}
	else {
		// Prevent view update with every record imported
		inhibit_view_update_ = true;
		old_enable_save_ = book_->save_enabled();
		book_->enable_save(false);
		// Clear flags
		update_in_progress_ = false;
		update_is_new_ = false;
		string update_source;
		// Keep track on number left to import
		int number_update_records = size();
		// Initialise flags
		bool is_updated = false;
		bool cancel_update = false;
		bool update_pending = false;
		bool update_ignored = false;
		update_in_progress_ = false;
		update_is_new_ = false;
		int offset = 0;
		// Process the records - always process the zeroth one as it gets deleted
		// update_in_progress_ indicates we want to drop out to present user with a 
		// choice.
		// If we have killed an update by another means, update_mode_ will be set to NONE
		for (int ix = 0; ix < number_update_records && !update_in_progress_ && update_mode_ != NONE; ix++) {
			// Get first QSO in update log
			record* import_record = at(0);
			// Skip records prior to last in log if we are automatically merging from e.g. fldigi
			string qso_timestamp = import_record->item("QSO_DATE_OFF") +
				import_record->item("TIME_OFF");
			// TODO: If we encounter a modem program that does not log TIME_OFF, we may have to use 
			// TIME_ON, but this could introduce a race hazard. 
			// If we have no seconds - force it to the last second of the minute so that it will be checked
			if (qso_timestamp.length() == 12) {
				qso_timestamp += "59";
			}
			// If the record is earlier than the last update
			if (qso_timestamp <= last_timestamp_ && update_mode_ == AUTO_IMPORT) {
				// We should have already imported so just delete the record
				discard_update(false);
				update_ignored = true;
				status_->progress(number_to_import_ - size());
			}
			else {
				// Some fields may require conversion (e.g. eQSL uses RST_SENT from contact's perspective
				convert_update(import_record);
				bool found_match = false;
				// Find the position of the record either equal in time or just after
				offset = book_->get_insert_point(import_record);
				int matched_record_num = 0;
				number_checked_++;
				// There may be a slight discrepancy in time so check 2 records either side of this position  
				// Any more than this is presented to the user to search for possible match
				// Start at previous record or beginning of the book
				for (int test_record = (offset < 2) ? 0 : (offset - 2);
					// Stop at next record, possible match or match found or end of book 
					test_record <= (offset + 2) && !update_in_progress_ && !found_match && test_record != book_->size();
					test_record++) {
					// Get potential match QSO
					record* record = book_->get_record(test_record, false);
					// Compare QSO records - Import record should have fewer fields
					match_result_t match_result = import_record->match_records(record);
					// LotW returns countries as listed in the ADIF spec,
					// whereas parsing has used DxAtlas names so accept difference for LotW 
					// but change to a possible match for any other source
					if (match_result == MT_LOC_MISMATCH) {
						if (update_mode_ != LOTW_UPDATE) {
							match_result = MT_POSSIBLE;
						}
					}
					// Select on result of match
					switch (match_result) {
						// Match exactly - may have additional fields in update so update log record from update record
						// Location mismatch for LoTW and everything else OK
					case MT_EXACT:
					case MT_LOC_MISMATCH:
						found_match = true;
						// merge_records the matching record from the import record and delete the import record
						if (record->merge_records(import_record, update_mode_ == LOTW_UPDATE)) {
							char message[256];
							snprintf(message, 256, "IMPORT: Updated record. %s %s %s %s %s",
								record->item("QSO_DATE").c_str(), record->item("TIME_ON").c_str(),
								record->item("CALL").c_str(),
								record->item("BAND").c_str(), record->item("MODE").c_str());
							status_->misc_status(ST_LOG, message);
							is_updated = true;
							number_updated_++;
						}
						number_accepted_++;
						// For eQSL.cc request the eQSL e-card. These are queued not to overwhelm eQSL.cc
						if (update_mode_ == EQSL_UPDATE) {
							eqsl_handler_->enqueue_request(test_record);
						}
						// Accepted - discard this record
						discard_update(false);
						break;
						// No match found - do nothing
					case MT_NOMATCH:
					case MT_SWL_NOMATCH:
						found_match = false;
						break;
						// SWL matches band and mode and time - move the import record into log
					case MT_SWL_MATCH:
						// Insert the record
						book_->insert_record_at(offset, import_record);
						// Fetch the e-card from eQSL.cc
						if (update_mode_ == EQSL_UPDATE) {
							eqsl_handler_->enqueue_request(offset);
						}
						// Remove the record from this book
						accept_update();
						is_updated = true;
						found_match = true;
						number_updated_++;
						number_accepted_++;
						break;
						// All fields match (time within 30 minutes) - 
						// can update without query and delete record
					case MT_PROBABLE:
						found_match = true;
						if (record->merge_records(import_record, update_mode_ == LOTW_UPDATE)) {
							is_updated = true;
							number_updated_++;
						}
						number_accepted_++;
						// Fetch e-card from eQSL.cc
						if (update_mode_ == EQSL_UPDATE) {
							eqsl_handler_->enqueue_request(test_record);
						}
						discard_update(false);
						break;
						// Call, band and mode match (and time within 30 minutes) - update after query
					case MT_POSSIBLE:
						update_in_progress_ = true;
						match_question_ = "Call, band and mode match - some fields differ. Please select correct values.";
						book_->selection(test_record, HT_IMPORT_QUERY);
						found_match = true;
						break;
					}
					if (found_match) {
						// Note matching record
						matched_record_num = test_record;
					}
				}
				// Unexpected new record (update from log) - set flags to display new record - 
				// user will either accept, reject or search for match
				if (!found_match && !cancel_update && update_mode_ != AUTO_IMPORT && update_mode_ != FILE_IMPORT) {
					update_in_progress_ = true;
					update_is_new_ = true;
					match_question_ = "Cannot find matching record - click ""Accept"" to add to log.";
					// we have (probably) looked beyond the end of the book, select the last record
					if ((unsigned)offset >= book_->size()) {
						offset = book_->size() - 1;
					}
					book_->selection(offset, HT_IMPORT_QUERYNEW);
				}
				// Expected New record (merging logs) - Move from update log to main log. 
				else if (!found_match && (update_mode_ == AUTO_IMPORT || update_mode_ == FILE_IMPORT)) {
					// If Auto update or importing a file then record needs parsing etc.
					import_record->update_timeoff();
					if (update_mode_ == AUTO_IMPORT) {
						pfx_data_->parse(import_record);
					}
					spec_data_->validate(import_record, -1);
					// If auto updating add station details
					if (update_mode_ == AUTO_IMPORT) {
						import_record->user_details();
					}

					book_->insert_record_at(offset, import_record);
					number_updated_++;
					number_added_++;
					accept_update();
					is_updated = true;
				}
			}
			// Update import progress
			// Update progress and update views every so often
			status_->progress(number_to_import_ - size());
		}
		// Allow view updates
		inhibit_view_update_ = false;
		// If we have updated any record
		if (is_updated) {
			book_->modified(true, false);
		}
		// If we are dropping out to present a query - activate record view
		if (update_in_progress_) {
			tabbed_view_->activate_pane(OT_RECORD, true);
		}
		// Leaving after completing update so tidy up and select most recent record
		if (!update_in_progress_) {
			finish_update();
		}
	}
}

// Stop importing - about to do something else. Either crash or gracefully complete
void import_data::stop_update(logging_mode_t mode, bool immediate) {
	// Turn auto-import off
	Fl::remove_timeout(cb_timer_imp);
	// If immediately crashing
	if (immediate) {
		update_mode_ = NONE;
	}
	switch (update_mode_) {
	case AUTO_IMPORT:
	case FILE_IMPORT:
	case EQSL_UPDATE:
	case LOTW_UPDATE:
		// Set gradual stopping - will stop when update currently in progress completes
		next_logging_mode_ = mode;
		close_pending_ = true;
		// Go back to updating book
		if (size()) update_book();
		break;
	case NONE:
	case WAIT_AUTO:
		// 
		delete_contents(true);
		close_pending_ = false;
		break;
	case READ_AUTO:
		// Cancel any read in progress
		input_.close();
		break;
	}
}

// Tidy up after an update
void import_data::finish_update(bool merged /*= true*/) {
	// Restore state of save_enabled
	book_->enable_save(old_enable_save_);
	// No import data so select last record and activate main log view
	if (merged && size() == 0) {
		char message[256];
		sprintf(message, "IMPORT: %d records read, %d checked, %d updated, %d accepted, %d added, %d rejected",
			number_to_import_, number_checked_, number_updated_, number_accepted_, number_added_, number_rejected_);
		status_->misc_status(ST_OK, message);
		if (number_updated_) {
			book_->selection(book_->size() - 1, HT_ALL);
		}
		else {
			book_->selection(book_->size() - 1, HT_SELECTED);
		}
		tabbed_view_->activate_pane(OT_MAIN, true);
	}
	// Restart auto-timer if we aren't waiting to stop it - restarting timer will change update_mode
	if (update_mode_ == AUTO_IMPORT && !close_pending_) {
		Fl_Preferences update_settings(settings_, "Real Time Update");
		Fl_Preferences files_settings(update_settings, "Files");
		for (int i = 0; i < num_update_files_; i++) {
			Fl_Preferences modem_settings(files_settings, sources_[i].c_str());
			modem_settings.set("Timestamp", last_timestamps_[i].c_str());
		}
		repeat_auto_timer();
		// Get book to update the progress 
		book_->modified(book_->modified());
	}
	// We are waiting to finish the update
	if (close_pending_) {
		// Change logging mode 
		menu_->logging(next_logging_mode_);
		// Stop timer
		Fl::remove_timeout(cb_timer_imp);
		// Delete the update files
		delete[] update_files_;
		update_files_ = nullptr;
		delete[] empty_files_;
		empty_files_ = nullptr;
	}
	// If we are merging eQSL, release the card queue
	if (update_mode_ == EQSL_UPDATE) {
		eqsl_handler_->enable_fetch(eqsl_handler::EQ_START);
	}
	// If we are not connected to a rig
	if (!rig_if_) {
		scratchpad_->update();
	}
	// Allow the book to save (and save if modified)
	book_->enable_save(true);
	close_pending_ = false;
	update_mode_ = NONE;
	number_updated_ = 0;
}

// Where an update has come from a QSL server, some ADIF fields are renamed to the viewpoiint of this 
// log not the server
void import_data::convert_update(record* record) {
	// For each field in the record
	for (auto it = record->begin(); it != record->end(); it++) {
		string field_name = it->first;
		string update_name;
		string value = it->second;
		bool override = false;
		// Sometimes need to compare different fields - RST_SENT vs RST_RCVD
		// To avoid the name being switched back, a '!' is prepended to the field anme in some cases
		switch (update_mode_) {
		// Update from eQSL - contact SENT => user RCVD
		case EQSL_UPDATE:
			// QSL_SENT => EQSL_QSL_RCVD
			if (field_name == "QSL_SENT") {
				update_name = "EQSL_QSL_RCVD";
				override = true;
			}
			else if (
				// QSL_SENT_VIA => QSL_RCVD_VIA
				field_name == "QSL_SENT_VIA") {
				update_name = "!QSL_RCVD_VIA";
				override = true;
			}
			else if (
				// RST_SENT => RST_RCVD - note use temporary name until all changes done.
				field_name == "RST_SENT") {
				update_name = "!RST_RCVD";
			}
			else if (
				// RST_RCVD => RST_SENT - note use temporary name until all changes done.
				field_name == "RST_RCVD") {
				update_name = "!RST_SENT";
			}
			else if (
				// TX_PWR => RX_PWR
				field_name == "TX_PWR") {
				update_name = "!RX_PWR";
			}
			else if (
				// RX_PWR => TX_PWR
				field_name == "RX_PWR") {
				update_name = "!TX_PWR";
			}
			else if (
				// APP_EQSL_SWL => is_swl
				field_name == "APP_EQSL_SWL") {
				update_name = "SWL";
			}
			else {
				update_name = field_name;
			}
			break;
		case LOTW_UPDATE:
			// Update from LotW - QSL => LOTW_QSL
			if (field_name == "QSL_RCVD") {
				update_name = "LOTW_QSL_RCVD";
				override = true;
			}
			else if (field_name == "QSLRDATE") {
				update_name = "LOTW_QSLRDATE";
				override = true;
			}
			else {
				update_name = field_name;
			}
			break;
		default: // MERGE or UNKNOWN 
			update_name = field_name;
			break;
		}
		// If field name changed move from one to the other
		if (update_name != field_name) {
			record->change_field_name(field_name, update_name);
		}
	}
	// Set EQSL_QSLRDATE to todays date 
	if (update_mode_ == EQSL_UPDATE) {
		// Rename !RST_RCVD etc back again
		set<string> erasees;
		erasees.clear();
		// For each field in the record
		for (auto it = record->begin(); it != record->end(); it++) {
			string field_name = it->first;
			string value = it->second;
			// If the field name starts with a bang
			if (field_name[0] == '!') {
				// Change it back to without
				record->change_field_name(field_name, field_name.substr(1));
				erasees.insert(field_name);
			}
		}
		// Remove all the fields starting with a bang
		for (auto it = erasees.begin(); it != erasees.end(); it++) {
			record->erase(*it);
		}
		// Add EQSL_QSLRDATE - date eQSL received and the eQSL timestamp
		string qsl_rdate = now(false, "%Y%m%d");
		record->item("EQSL_QSLRDATE", qsl_rdate);

		// Delete QSLMSG
		record->item("QSLMSG", string(""));
	}

}

// Returns that there is no update in progress
bool import_data::update_complete() {
	switch (update_mode_) {
	case WAIT_AUTO:
	case NONE:
	case READ_AUTO:
		return true;
	default:
		return false;
	}
}

// Get the data from the QSL server
bool import_data::download_data(import_data::update_mode_t server) {
	stringstream adif;
	bool result = true;
	// Tidy up import book - complete any existing update
	stop_update(LM_IMPORTED, false);
	while (!update_complete()) Fl::wait();
	// Disable saving each record
	book_->enable_save(false);
	switch (server) {
	case EQSL_UPDATE: 
		// Fetch inbox from eQSL.cc into local stream, stop fetching eQSL cards until merge complete
		update_mode_ = server;
		status_->misc_status(ST_NOTE, "IMPORT: Downloading eQSL");
		eqsl_handler_->enable_fetch(eqsl_handler::EQ_PAUSE);
		result = eqsl_handler_->download_eqsl_log(&adif);
		break;
	case LOTW_UPDATE:
		// Fetch inbox from LotW into local stream
		update_mode_ = server;
		status_->misc_status(ST_NOTE, "IMPORT: Downloading LotW");
		result = lotw_handler_->download_lotw_log(&adif);
		break;
	}
	// If the download was successful
	if (result) {
		// Go the start of the stream again
		adif.seekg(0, adif.beg);
		// This download data will be ADI format - load the stream into this book
		adi_reader* reader = new adi_reader();
		reader->load_book(this, adif);
		delete reader;
		if (server == LOTW_UPDATE) {
			// Process the LotW header 
			process_lotw_header();
		}
		// Switch the view to the import view and select forst record
		tabbed_view_->activate_pane(OT_IMPORT, true);
		selection(record_number(0));
		merge_data();
	}
	else {
		update_mode_ = NONE;
	}
	return result;
}

// Merge data
void import_data::merge_data() {
	// Tell user
	status_->misc_status(ST_NOTE, "IMPORT: Merging files started");
	status_->progress(size(), book_type_, "records");
	number_to_import_ = size();
	number_accepted_ = 0;
	number_updated_ = 0;
	number_checked_ = 0;
	number_added_ = 0;
	number_rejected_ = 0;
	// Merge this book into main log book
	update_book();
	// If we have no user query - switch to main log view
	if (!update_in_progress_) {
		tabbed_view_->activate_pane(OT_MAIN, true);
	}
}

// overload of book::load_data, sets the update mode and if successful switches view to import view
bool import_data::load_data(string filename) {
	// This breaks auto-update
	if (update_mode_ != READ_AUTO) {
		update_mode_ = FILE_IMPORT;
	}
	bool result = book::load_data(filename);
	if (result) {
		tabbed_view_->activate_pane(OT_IMPORT, true);
		selection(record_number(0));
	}
	return result;
}

// Process LotW header - merges
void import_data::process_lotw_header() {
	// Header contains last qsl received data to allow next download to start of where this ended
	string update = header()->item("APP_LOTW_LASTQSL");
	if (update.length() > 10) {
		// Remember this in settings
		string setting = update.substr(0, 4) + update.substr(5, 2) + update.substr(8, 2);
		Fl_Preferences qsl_settings(settings_, "QSL");
		Fl_Preferences lotw_settings(qsl_settings, "LotW");
		lotw_settings.set("Last Accessed", setting.c_str());
	}
	else {
		// Not a valid date provided
		status_->misc_status(ST_WARNING, "IMPORT: Invalid last QSL date received from LotW");
	}
	// LotW ADIF will always give load failed as there is an non-compliant record at the end
	// that indicates end-of-file
	// Replace this with "file loaded" if the number of records agrees with the 
	// stated number in the header record
	int num_records;
	header()->item("APP_LOTW_NUMREC", num_records);
	if ((size_t)num_records == size()) {
		status_->misc_status(ST_OK, "IMPORT: LotW download done.");
		status_->progress(num_records);
	}
}

// Menu enable of auto update
void import_data::auto_enable(bool enabled) {
	auto_enable_ = enabled;
	if (auto_enable_ && update_mode_ == NONE) {
		start_auto_update();
	}
}

// Return the current state of auto-enable
bool import_data::auto_enable() {
	return auto_enable_;
}

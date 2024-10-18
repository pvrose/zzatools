#include "import_data.h"
#include "cty_data.h"
#include "spec_data.h"
#include "tabbed_forms.h"
#include "status.h"
#include "eqsl_handler.h"
#include "lotw_handler.h"
#include "club_handler.h"
#include "utils.h"
#include "menu.h"
#include "qso_manager.h"
#include "adi_reader.h"
#include "record.h"

#include <sstream>
#include <ctime>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <share.h>
#else
#include <stdio.h>
#endif

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>




extern status* status_;
extern book* book_;
extern Fl_Preferences* settings_;
extern cty_data* cty_data_;
extern spec_data* spec_data_;
extern tabbed_forms* tabbed_forms_;
extern eqsl_handler* eqsl_handler_;
extern lotw_handler* lotw_handler_;
extern club_handler* club_handler_;
extern menu* menu_;
extern qso_manager* qso_manager_;

// Constructor - this book is used to contain data being imported. It adds functionality to support this
import_data::import_data() :
	book(OT_IMPORT)
{
	// Initialise
	update_files_ = nullptr;
	empty_files_ = nullptr;
	close_pending_ = false;
	update_in_progress_ = false;
	update_is_new_ = false;
	update_mode_ = NONE;
	number_to_import_ = 0;
	number_modified_ = 0;
	number_matched_ = 0;
	number_checked_ = 0;
	number_added_ = 0;
	number_rejected_ = 0;
	number_clublog_ = 0;
	number_swl_ = 0;
	num_update_files_ = 0;
	update_files_ = nullptr;
	empty_files_ = nullptr;
	match_question_ = "";
	close_pending_ = false;
	last_added_number_ = 0;
}

// Destructor
import_data::~import_data()
{
	delete[] empty_files_;
	delete[] update_files_;
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
	if (book_record->merge_records(import_record, false, &hint)) {
		book_->modified(true, false);
	}
	if (cty_data_->update_qso(book_record)) {
		book_->modified(true, false);
	}
	if (spec_data_->validate(book_record, book_->selection())) {
		book_->modified(true, false);
	}
	// Tell views to update with new record
	book_->selection(-1, hint);
	// Delete the record from this book
	discard_update(false);
	number_matched_++;
}

// Add the update record to the log - called by record_form
void import_data::save_update() {
	// Get parse and validation settings
	record* import_record = at(0);
	qso_manager_->update_import_qso(import_record);
	if (cty_data_->update_qso(import_record)) {
	}
	// Need to copy it over before calling the next two to get the record number
	int record_number = book_->insert_record(import_record);
	accept_update();
	number_added_++;
	last_added_number_ = record_number;
	book_->selection(record_number, HT_INSERTED);

	if (spec_data_->validate(import_record, record_number)) {
	}
	// This may result in the card being fetched twice
	if (update_mode_ == EQSL_UPDATE) {
		eqsl_handler_->enqueue_request(record_number);
	}
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
		if (!update_in_progress_) {
			book_->enable_save(false, "Starting update from import");
		} else {
			// Fetch QSL card for the queried QSO
			if (update_mode_ == EQSL_UPDATE) {
				eqsl_handler_->enqueue_request(record_number(0));
			}
		}
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
		char message[256];
		// Process the records - always process the zeroth one as it gets deleted
		// update_in_progress_ indicates we want to drop out to present user with a 
		// choice.
		// If we have killed an update by another means, update_mode_ will be set to NONE
		for (int ix = 0; ix < number_update_records && !update_in_progress_ && update_mode_ != NONE; ix++) {
			// Get first QSO in update log
			record* import_record = at(0);
			// Skip records prior to last in log if we are automatically merging from e.g. fldigi
			// NB. Assumes that the modem program logs TIME_OFF. Using tIME_ON can introduce a race condition
			string qso_timestamp = import_record->item("QSO_DATE_OFF") +
				import_record->item("TIME_OFF");
			// If we have no seconds - force it to the last second of the minute so that it will be checked
			if (qso_timestamp.length() == 12) {
				qso_timestamp += "59";
			}
			// Some fields may require conversion (e.g. eQSL uses RST_SENT from contact's perspective
			convert_update(import_record);
			bool found_match = false;
			// Find the position of the record either equal in time or just after
			offset = book_->get_insert_point(import_record);
			int matched_record_num = 0;
			number_checked_++;
			// Need separate checks for SWL report - wait until all records checked to see if one is a 2-way SWL match
			bool had_swl_match = false;
			// There may be a slight discrepancy in time so check 2 records either side of this position  
			// Any more than this is presented to the user to search for possible match
			// Start at previous record or beginning of the book
			int tries[] = { 0, -1, -2, 1, 2};
			for (int ix = 0;
				// Stop at next record, possible match or match found or end of book 
				ix < 5 && !update_in_progress_ && !found_match;
				ix++) {
				int test_record = offset + tries[ix];
				// If the test record is outwith the book skip the check
				if (test_record < 0 || test_record >= book_->size()) continue;
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
					// MT_2XSWL_MATCH can be reported as MT_EXACT
					had_swl_match = false;
					// merge_records the matching record from the import record and delete the import record
					if (record->merge_records(import_record, update_mode_ == LOTW_UPDATE)) {
						snprintf(message, 256, "IMPORT: Updated record. %s %s %s %s %s",
							record->item("QSO_DATE").c_str(), record->item("TIME_ON").c_str(),
							record->item("CALL").c_str(),
							record->item("BAND").c_str(), record->item("MODE").c_str());
						status_->misc_status(ST_LOG, message);
						is_updated = true;
						number_modified_++;
						if (record->item("CLUBLOG_QSO_UPLOAD_STATUS") == "M") {
							number_clublog_++;
						}
					}
					number_matched_++;
					// For eQSL.cc request the eQSL e-card. These are queued not to overwhelm eQSL.cc
					if (update_mode_ == EQSL_UPDATE) {
						eqsl_handler_->enqueue_request(test_record);
					}
					// Accepted - discard this record
					discard_update(false);
					break;
					// 2-way SWL match - ignore it
				case MT_2XSWL_MATCH:
					found_match = true;
					had_swl_match = false;
					// Rejected - discard this record
					discard_update(false);
					break;
					// No match found - do nothing
				case MT_NOMATCH:
				case MT_SWL_NOMATCH:
					found_match = false;
					break;
					// SWL matches band and mode and time - wait until end of search to add it.
				case MT_SWL_MATCH:
					had_swl_match = true;
					found_match = false;
					break;
					// All fields match (time within 30 minutes) - 
					// can update without query and delete record
				case MT_PROBABLE:
					found_match = true;
					if (record->merge_records(import_record, update_mode_ == LOTW_UPDATE)) {
						is_updated = true;
						number_modified_++;
					}
					number_matched_++;
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
					snprintf(message, sizeof(message), "LOG: Possible match found with %s", import_record->item("CALL").c_str());
					status_->misc_status(ST_LOG, message);
					book_->selection(test_record, HT_IMPORT_QUERY);
					found_match = true;
					break;
					// Call, band and mode match but time > 30 minutes 
				case MT_UNLIKELY:
					switch (update_mode_) {
					case FILE_IMPORT:
					case FILE_UPDATE:
						// Matches are likely to be closer than 30 minutes so ignore those outwith this
						break;
					default:
						update_in_progress_ = true;
						match_question_ = "Call, band and mode match - time differs > 30m. Please select correct values.";
						book_->selection(test_record, HT_IMPORT_QUERY);
						found_match = true;
						break;
					}
					break;
				}
				if (found_match) {
					// Note matching record
					matched_record_num = test_record;
				}
			}
			if (had_swl_match) {
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
				matched_record_num = offset;
				number_swl_++;
				had_swl_match = false;
			}
			// Unexpected new record (update from log) - set flags to display new record - 
			// user will either accept, reject or search for match
			if (!found_match && !cancel_update && update_mode_ != FILE_IMPORT && 
				update_mode_ != DATAGRAM && update_mode_ != CLIPBOARD) {
				update_in_progress_ = true;
				update_is_new_ = true;
				match_question_ = "Cannot find matching record - select appropriate action";
				// we have (probably) looked beyond the end of the book, select the last record
				if ((unsigned)offset >= book_->size()) {
					offset = book_->size() - 1;
				}
				book_->selection(offset, HT_IMPORT_QUERYNEW);
			}
			// Expected new record (merging logs) - Move from update log to main log. 
			else if (!found_match && (update_mode_ == FILE_IMPORT || 
				update_mode_ == DATAGRAM || update_mode_ == CLIPBOARD)) {
				// If Auto update or importing a file then record needs parsing etc.
				import_record->update_timeoff();
				if (update_mode_ == FILE_IMPORT ||
					update_mode_ == DATAGRAM || update_mode_ == CLIPBOARD) {
					add_use_data(import_record);
					cty_data_->update_qso(import_record);
					spec_data_->validate(import_record, -1);
					qso_manager_->update_import_qso(import_record);
				}

				book_->insert_record_at(offset, import_record);
				number_added_++;
				last_added_number_ = offset;
				accept_update();
				is_updated = true;
			}
		}
		// Update import progress
		// Update progress and update views every so often
		status_->progress(number_to_import_ - size(), book_type());
		// Allow view updates
		inhibit_view_update_ = false;
		// If we have updated any record
		if (is_updated) {
			book_->modified(true, false);
		}
		// Leaving after completing update so tidy up and select most recent record
		if (!update_in_progress_) {
			finish_update();
		}
	}
}

// Stop importing - about to do something else. Either immediately or gracefully complete
void import_data::stop_update(bool immediate) {
	// If immediately crashing - stop expecting further updates
	if (immediate) {
		update_mode_ = NONE;
	}
	switch (update_mode_) {
	case FILE_IMPORT:
	case EQSL_UPDATE:
	case LOTW_UPDATE:
	case DATAGRAM:
	case CLIPBOARD:
		// Set gradual stopping - will stop when update currently in progress completes
		close_pending_ = true;
		// Go back to updating book
		if (size()) update_book();
		break;
	case NONE:
		// Empty this book (used for immediate stop)
		if (immediate) {
			delete_contents(true);
			status_->progress("Auto-update stopped", OT_IMPORT);
		}
		close_pending_ = false;
		break;
	}
}

// Tidy up after an update
void import_data::finish_update(bool merged /*= true*/) {
	// No import data so select last record and activate main log view
	if (merged && size() == 0) {
		char message[256];
		if (update_mode_ == LOTW_UPDATE) {
			sprintf(message, "IMPORT: LOTW %d records read, %d checked, %d modified, %d matched, %d added, %d SWLs added, %d rejected, %d changed ClubLog",
				number_to_import_, number_checked_, number_modified_, number_matched_, number_added_, number_swl_, number_rejected_, number_clublog_);
		}
		else {
			string source;
			switch (update_mode_) {
			case EQSL_UPDATE:
				source = "EQSL";
				break;
			case FILE_IMPORT:
				source = "FILE";
				break;
			case DATAGRAM:
				source = "UDP";
				break;
			case CLIPBOARD:
				source = "CLIPBOARD";
				break;
			}
			sprintf(message, "IMPORT: %s %d records read, %d checked, %d modified, %d matched, %d added, %d SWLs added, %d rejected",
				source.c_str(), number_to_import_, number_checked_, number_modified_, number_matched_, number_added_, number_swl_, number_rejected_);
		}
		status_->misc_status(ST_OK, message);
		if (number_modified_ || number_added_ || number_swl_) {
			// Some records have been changed or added
			book_->selection(book_->size() - 1, HT_ALL);
		}
		else {
			// No change, select the latest record.
			book_->selection(book_->size() - 1, HT_SELECTED);
		}
		// Switch to main now the QSO has been copied
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Allow any updates to be displayed before we do anything else
		Fl::wait();
		if (number_added_ == 1) {
			// One new record added from whatever source, send latest QSO to QSL servers
			if (!book_->upload_qso(last_added_number_)) {
				status_->misc_status(ST_WARNING, "IMPORT: Upload to one or more QSL sites failed");
			}
		}
	}
	// We are waiting to finish the update
	if (close_pending_) {
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
	// Allow the book to save (and save if modified)
	close_pending_ = false;
	number_modified_ = 0;
	update_mode_ = NONE;
	// Restore state of save_enabled
	book_->enable_save(true, "Finished update from import");
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
		bool ignore = false;
		// Sometimes need to compare different fields - RST_SENT vs RST_RCVD
		// To avoid the name being switched back, a '!' is prepended to the field name in some cases
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
				// OPERATOR => APP_ZZA_OPERATOR
				field_name == "OPERATOR") {
				update_name = "APP_ZZA_OPERATOR";
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
			else if (
				// OPERATOR => APP_ZZA_OPERATOR
				field_name == "OPERATOR") {
				update_name = "APP_ZZA_OPERATOR";
			}
			else {
				update_name = field_name;
			}

			break;
		default: // MERGE or UNKnowN 
			update_name = field_name;
			break;
		}
		if (ignore) {
			record->item(field_name, string(""));
		}
		else {
			// If field name changed move from one to the other
			if (update_name != field_name) {
				record->change_field_name(field_name, update_name);
			}
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
	case NONE:
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
	stop_update(false);
	while (!update_complete()) Fl::check();
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
		// Load the stream
		load_stream(adif, server);

	}
	else {
		update_mode_ = NONE;
	}
	return result;
}

// Load from a data stream
void import_data::load_stream(stringstream& adif, import_data::update_mode_t server) {
	if (server == DATAGRAM || server == CLIPBOARD) {
		update_mode_ = server;
	}
	// This download data will be ADI format - load the stream into this book
	adi_reader* reader = new adi_reader();
	reader->load_book(this, adif);
	delete reader;
	if (server == LOTW_UPDATE) {
		// Process the LotW header 
		process_lotw_header();
	}
	// Switch the view to the import view and select first record
	tabbed_forms_->activate_pane(OT_IMPORT, true);
	// selection(record_number(0));
	merge_data();
}

// Merge data
void import_data::merge_data(import_data::update_mode_t mode) {
	if (mode != EXISTING) {
		update_mode_ = mode;
	}
	// Tell user
	status_->misc_status(ST_NOTE, "IMPORT: Merging files started");
	status_->progress(size(), book_type_, "Merging data from file", "records");
	// Reset counts
	number_to_import_ = size();
	number_matched_ = 0;
	number_modified_ = 0;
	number_checked_ = 0;
	number_added_ = 0;
	number_rejected_ = 0;
	number_clublog_ = 0;
	number_swl_ = 0;
	last_added_number_ = 0;
	// Merge this book into main log book
	update_book();
	// If we have no user query - switch to main log view
	if (!update_in_progress_) {
		tabbed_forms_->activate_pane(OT_MAIN, true);
	}
}

// overload of book::load_data, sets the update mode and if successful switches view to import view
bool import_data::load_data(string filename, update_mode_t mode) {
	// This breaks auto-update
	update_mode_ = mode;
	bool result = book::load_data(filename);
	if (result) {
		tabbed_forms_->activate_pane(OT_IMPORT, true);
		selection(record_number(size() - 1));
		status_->misc_status(ST_NOTE, "Records read, select Import->Merge... to add them to log");
	}
	else {
		// Read failed, delete any records that may have been loaded and tidy up.
		clear();
		finish_update();
	}
	return result;
}

// Import a single record
void import_data::load_record(record* qso, update_mode_t mode) {
	update_mode_ = mode;
	append_record(qso);
	merge_data();
}

// Process LotW header - merges
void import_data::process_lotw_header() {
	if (header()->item("PROGRAMID") != "LoTW") {
		status_->misc_status(ST_WARNING, "IMPORT: It does not appear the ADIF came from LoTW");
	}
	else {
		status_->misc_status(ST_OK, "IMPORT: LotW download done.");
	}
}

// Number of update files
int import_data::number_update_files() {
	return num_update_files_;
}


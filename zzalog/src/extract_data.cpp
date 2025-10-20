#include "extract_data.h"

#include "tabbed_forms.h"
#include "adi_writer.h"
#include "club_handler.h"
#include "eqsl_handler.h"
#include "lotw_handler.h"
#include "main.h"
#include "qrz_handler.h"
#include "qso_manager.h"
#include "record.h"
#include "search_dialog.h"
#include "spec_data.h"
#include "status.h"

#include "utils.h"

#include <sstream>

#include <FL/fl_ask.H>

// Constructor
extract_data::extract_data() :
	book(OT_EXTRACT)
	, use_mode_(NONE)
{
	// This book contains extract data
	extract_criteria_.clear();
	mapping_.clear();
	rev_mapping_.clear();
}

// Destructor
extract_data::~extract_data()
{
	// Same as book::delete_contents without deleting the records - these are wanted
	clear();
	dirty_qsos_.clear();
	filename_ = "";
	format_ = FT_NONE;
	delete header_;
	header_ = nullptr;

}

// Add seaach criteria
int extract_data::criteria(search_criteria_t criteria, extract_data::extract_mode_t mode /*=SEARCH*/) {
	if (criteria.combi_mode == XM_NEW) {
		// Starting a new search
		if (use_mode_ != NONE) {
			status_->misc_status(ST_WARNING, "EXTRACT: New search, cancelling existing search");
			extract_criteria_.clear();
		}
	}
	else if (mode != use_mode_) {
		status_->misc_status(ST_ERROR, "EXTRACT: A follow-on search has been requested but is incompatible. Ignored!");
		return -1;
	}
	// std::set criteria
	use_mode_ = mode;
	// Append to the std::set of criteria
	extract_criteria_.push_back(criteria);
	// Get the most recent
	criteria_ = &extract_criteria_.back();
	// Get the matching record from the main log book
	extract_records();
	return size();
}

// Add all records that match in main log to this log
void extract_data::extract_records() {
	item_num_t count = 0;
	char message[100];
	status_->misc_status(ST_NOTE, "EXTRACT: Started");
	status_->misc_status(ST_NOTE, short_comment().c_str());
	switch (criteria_->combi_mode) {
	case XM_NEW:
		// new results - remove existing results
		this->clear();
		mapping_.clear();
		rev_mapping_.clear();
		// copy header across and append reason for search
		if (book_->header()) {
			header_ = new record(*book_->header());
			header_->header(header_->header() + '\n' + comment());
		}
		else {
			header_ = new record;
			header_->header(comment());
		}
		status_->progress(book_->get_count(), OT_EXTRACT, "Extracting New", "records");
		// For all records in main log book
		for (item_num_t ixb = 0; ixb < book_->get_count(); ixb++) {
			record* ext_record = book_->get_record(ixb, false);
			// Compare record against seaarch criteria
			if (match_record(ext_record)) {
				// It matches, copy reference to this book
				push_back(ext_record);
				int ixe = size() - 1;
				// Add to both ways mappings
				mapping_.insert(mapping_.begin() + ixe, ixb);
				rev_mapping_[ixb] = ixe;
				count += 1;
			}
			status_->progress(ixb + 1, OT_EXTRACT);
		}
		snprintf(message, 100, "EXTRACT: %zu records extracted, %zu total", count, size());
		break;
	case XM_AND:
		// Logical AND between existing and new criteria - i.e. only those that match both
		// Append new reason for search to header
		if (header_ == nullptr) header_ = new record;
		header_->header(header_->header() + '\n' + comment());
		status_->progress(get_count(), OT_EXTRACT, "Extracting AND", "records");
		// For all records in this book
		for (item_num_t ixe = 0, ixb =  0; ixe < get_count(); ) {
			// Compare record against these search criteria
			if (!match_record(get_record(ixe, false))) {
				// If it doesn't match, remove the record pointer from this book
				erase(begin() + ixe);
				int ixb = mapping_[ixe];
				mapping_.erase(mapping_.begin() + ixe);
				rev_mapping_.erase(ixb);
				count += 1;
			}
			else {
				// Remap reverse mapping to the new index in this book.
				rev_mapping_[mapping_[ixe]] = ixe;
				ixe++;
			}
			status_->progress(++ixb, OT_EXTRACT);
		}
		snprintf(message, 100, "EXTRACT: %zu records deleted, %zu total", count, size());
		break;
	case XM_OR: 
		// Logical OR between existing search and new criteria - i.e. those that match either
		// Append new reason for search to header
		if (header_ == nullptr) header_ = new record;
		header_->header(header_->header() + '\n' + comment());
		status_->progress(book_->get_count(), OT_EXTRACT, "Extracting OR", "records");
		// For all records in main log book
		for (item_num_t ixb = 0; ixb < book_->get_count(); ixb++) {
			record* test_record = book_->get_record(ixb, false);
			// Compare record against search criteria
			if (match_record(test_record)) {
				// If it matches find insert point in this book
				item_num_t ixe = get_insert_point(test_record);
				// If this record is not already at the insert point
				if (get_record(ixe, false) != book_->get_record(ixb, false)) {
					// Add the record to this book
					insert_record_at(ixe, test_record);
					mapping_.insert(mapping_.begin() + ixe, ixb);
					rev_mapping_[ixb] = ixe;
					count += 1;
				}
			}
			status_->progress(ixb + 1, OT_EXTRACT);
		}
		snprintf(message, 100, "EXTRACT: %zu records added, %zu total", count, size());
		break;
	}
	status_->misc_status(ST_OK, message);
	// Collect use data for the extraction
	for (auto it = begin(); it != end(); it++) {
		add_use_data(*it);
	}
	navigation_book_ = this;
	qso_manager_->enable_widgets();

}

// Repeat the extractions
void extract_data::reextract() {
	status_->misc_status(ST_NOTE, "EXTRACT: Re-extracting existing criteria");
	// Clear the book without deleting the records
	clear();
	// For all sets of criteria in the history stack
	for (auto it = extract_criteria_.begin(); it != extract_criteria_.end(); it++) {
		// Repeat the extraction
		criteria_ = &(*it);
		extract_records();
	}
}

// clear criteria
void extract_data::clear_criteria(bool redraw) {
	status_->misc_status(ST_NOTE, "EXTRACT: Clearing all criteria");
	// Clear the book without deleting the records
	clear();
	// Clear all the sets of criteria
	extract_criteria_.clear();
	// Clear the mappings
	mapping_.clear();
	rev_mapping_.clear();
	// now tidy up this book, records have already been removed so will not be deleted
	delete_contents(true);
	use_mode_ = NONE;
	// Set navigation book to main log
	if (redraw) {
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Cause the views to be redrawn
		selection(-1, HT_EXTRACTION);
		qso_manager_->enable_widgets();
	}
}

// Convert item index in this book to the record index in the main log book
inline qso_num_t extract_data::record_number(item_num_t item_number) {
	if (size() > 0) 
		// Return the mapping from this book to the main book
		return mapping_[item_number];
	else return -1;
}

// Convert record index in the main log book to the item index in this book
// nearest = true will choose the closest item to the record number
inline item_num_t extract_data::item_number(qso_num_t record_number, bool nearest /*=false*/) {
	// Return the mapping item from main book  to this book
	if (size() == 0) {
		if (nearest) {
			// Nearest is first item
			return 0;
		}
		else {
			// Default
			return -1;
		}
	}
	else {
		// Try and find the mapping
		auto it = rev_mapping_.find(record_number);
		if (nearest && it == rev_mapping_.end()) {
			// Need to find nearest mapping
			// Get the bounds of the search (initially first and last items)
			item_num_t lbound = 0;
			item_num_t ubound = mapping_.size() - 1;
			if (mapping_[lbound] > record_number) {
				// Record is before first item - return the first item
				return 0;
			}
			if (mapping_[ubound] < record_number) {
				// Record is above the last item - return the last item
				return ubound;
			}
			// Binary slice the array until found
			// Keep comparing the record number between upper bound and lower bound and move one or 
			// other of the bounds until they differ by one, then put it there.
			item_num_t test;
			while (ubound - 1 != lbound) {
				// Compare with the half-way point 
				test = (lbound + ubound) / 2;
				if (record_number > mapping_[test]) {
					// It's between half-way and upper-bound, move lower-bound to half-way
					lbound = test;
				}
				else {
					// It's between half-way and lower-bound, move upper-bound to half-way
					ubound = test;
				}
			}
			// Return the closer of the two - use the higher if they are equidisstant
			if (mapping_[ubound] - record_number <= record_number - mapping_[lbound]) {
				return ubound;
			}
			else {
				return lbound;
			}
		}
		else {
			// Return the exact mapping if it exists of -1 if it doesn't
			if (it != rev_mapping_.end()) {
				return it->second;
			}
			else {
				return -1;
			}
		}
	}
}

// Describe the search criteria so it can be added to the header comment
std::string extract_data::comment() {
	std::string result;
	if (criteria_) {
		result = "Data extracted for criteria:-\n";
		// Add the main condition
		switch (criteria_->condition) {
		case XC_DXCC:
			result += "DXCC ";
			break;
		case XC_CQZ:
			result += "CQ Zone ";
			break;
		case XC_ITUZ:
			result += "ITU Zone ";
			break;
		case XC_CONT:
			result += "Continent ";
			break;
		case XC_SQ2:
		case XC_SQ4:
			result += "Grid square ";
			break;
		case XC_CALL:
			result += "Callsign ";
			break;
		case XC_UNFILTERED:
			result += "Unfiltered";
			break;
		case XC_FIELD:
			result += "Field (" + criteria_->field_name + ") ";
			break;
		default:
			break;
		}
		if (criteria_->condition != XC_UNFILTERED) {
			// Add condition label
			result += " " + comparator_labels_[(int)criteria_->comparator] + " " + criteria_->pattern + ".\n";
		}
		if (criteria_->by_dates) {
			// Date-range restricted
			result += "Between " + criteria_->from_date += " and " + criteria_->to_date + ".\n";
		}
		// Band and Mode
		result += "Band: " + criteria_->band + "; Mode: " + criteria_->mode + "; ";
		// Confirmation status
		if (criteria_->confirmed_eqsl) result += "Confirmed eQSL; ";
		if (criteria_->confirmed_lotw) result += "Confirmed LotW; ";
		if (criteria_->confirmed_card) result += "Confirmed Card; ";
		result += '\n';
		// And whether results are new, anded or ored
		switch (criteria_->combi_mode) {
		case XM_NEW:
			result += "New search.\n";
			break;
		case XM_AND:
			result += "Results meet both this and previous criteria.\n";
			break;
		case XM_OR:
			result += "Results meet either this or previous criteria.\n";
			break;
		}
	}
	else {
		result = "No extract criteria are defined.\n";
	}
	return result;
}

// Describe the search criteria so it can be added to the header comment
std::string extract_data::short_comment() {
	std::string result;
	if (criteria_) {
		result = "EXTRACT: ";
		// And whether results are new, anded or ored
		switch (criteria_->combi_mode) {
		case XM_AND:
			result += "& ";
			break;
		case XM_OR:
			result += "| ";
			break;
		default:
			break;
		}
		// Only select those which didn't match
		// Add the main condition
		switch (criteria_->condition) {
		case XC_DXCC:
			result += "DX";
			break;
		case XC_CQZ:
			result += "CQ";
			break;
		case XC_ITUZ:
			result += "ITU";
			break;
		case XC_CONT:
			result += "CONT";
			break;
		case XC_SQ2:
		case XC_SQ4:
			result += "LOC";
			break;
		case XC_CALL:
			result += "CALL";
			break;
		case XC_UNFILTERED:
			result += "ALL";
			break;
		case XC_FIELD:
			result += criteria_->field_name;
			break;
		default:
			break;
		}
		if (criteria_->condition != XC_UNFILTERED) {
			// Add if using regular expression matching
			result += " " + comparator_labels_[(int)criteria_->comparator] +  " '" + criteria_->pattern + "'";
		}
		result += " ";
		if (criteria_->by_dates) {
			// Date-range restricted
			result += " (" + criteria_->from_date += ":" + criteria_->to_date + ") ";
		}
		// Band and Mode
		result += "On " + criteria_->band + " " + criteria_->mode + " " + criteria_->my_call + " ";
		// Confirmation status
		if (criteria_->confirmed_eqsl) result += "eQSL ";
		if (criteria_->confirmed_lotw) result += "LotW ";
		if (criteria_->confirmed_card) result += "Card ";
	}
	else {
		result = "EXTRACT: No criteria are defined.";
	}
	return result;
}

// Extract records that need sending to the named server (eQSL, LotW, mail or ClubLog)
void extract_data::extract_qsl(extract_data::extract_mode_t server) {
	std::string reason;
	std::string field_name;
	// Define the field that shows it's not been sent
	switch (server) {
	case EQSL:
		reason = "upload to eQSL";
		field_name = "EQSL_QSL_SENT";
		break;
	case LOTW:
		reason = "upload to LotW";
		field_name = "LOTW_QSL_SENT";
		break;
	case CARD:
		reason = "printing labels";
		field_name = "QSL_SENT";
		break;
	case CLUBLOG:
		reason = "upload to clubLog";
		field_name = "CLUBLOG_QSO_UPLOAD_STATUS";
		break;
	case EMAIL:
		reason = "Generate e-mail and PNG";
		field_name = "QSL_SENT";  
		break;
	case QRZCOM:
		reason = "upload to QRZ.com";
		field_name = "QRZCOM_QSO_UPLOAD_STATUS";
		break;
	default:
		break;
	}
	// Now check that they are all for the current station
	std::string station = qso_manager_->get_default(qso_manager::CALLSIGN);

	// Extract those records not sent to QSL server !(*QSL_SENT==Y) 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*search_comp_t comparator*/ XP_NE,
		/*bool by_dates*/ false,
		/*std::string from_date*/"",
		/*std::string to_date;*/"",
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*std::string field_name; */ field_name,
		/*std::string pattern;*/ "Y",
		/*std::string my_call*/ station
	};
	status_->misc_status(ST_NOTE, "EXTRACT: Extracting QSOs not sent already");
	criteria(new_criteria, server);
	// Only send those whose QSO is complete !(QSO_COMPLETE==N)
	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*search_comp_t comparator*/ XP_NE,
		/*bool by_dates*/ false,
		/*std::string from_date*/"",
		/*std::string to_date;*/"",
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_AND,
		/*std::string field_name; */ "QSO_COMPLETE",
		/*std::string pattern;*/ "N",
		/*std::string my_call*/ station
	};
	status_->misc_status(ST_NOTE, "EXTRACT: Removing incomplete QSOs");
	criteria(new_criteria, server);
	if (server == LOTW || server == CLUBLOG || server == QRZCOM) {
		// Only send those to which are QSOs !(SWL==Y)
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_NE,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*std::string field_name; */ "SWL",
			/*std::string pattern;*/ "Y",
			/*std::string my_call*/ station
		};
		status_->misc_status(ST_NOTE, "EXTRACT: Removing replies to SWL reports");
		criteria(new_criteria, server);
	}
	if (server == CARD) {
		// Only those for which we intend to send a card (QSL_SENT==Q - Queued)
		// QSL_SENT==R - Requested
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_REGEX,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*std::string field_name; */ "QSL_SENT",
			/*std::string pattern;*/ "[QR]",
			/*std::string my_call*/ station
		};
		status_->misc_status(ST_NOTE, "EXTRACT: Extracting queued cards only");
		criteria(new_criteria, server);
		// Now those which received via bureau
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_EQ,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*std::string field_name; */ "QSL_SENT_VIA",
			/*std::string pattern;*/ "B",
			/*std::string my_call*/ station
		};
		status_->misc_status(ST_NOTE, "EXTRACT: Extracting cards for Bureau");
		criteria(new_criteria, server);
		if (size()) sort_records("DXCC", false);
		tabbed_forms_->update_views(nullptr, HT_RESET_ORDER, 0);
	}
	if (server == EMAIL) {
		// Only those for which we intend to send a card (QSL_SENT==Q - Queued)
		// QSL_SENT==R - Requested
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_REGEX,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*std::string field_name; */ "QSL_SENT",
			/*std::string pattern;*/ "[QR]",
			/*std::string my_call*/ station
		};
		status_->misc_status(ST_NOTE, "EXTRACT: Extracting queued cards only");
		criteria(new_criteria, server);
		// Now those which received via bureau
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_EQ,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*std::string field_name; */ "QSL_SENT_VIA",
			/*std::string pattern;*/ "E",
			/*std::string my_call*/ station
		};
		status_->misc_status(ST_NOTE, "EXTRACT: Extracting cards for sending by e-mail");
		criteria(new_criteria, server);
	}
	// Remove those previously marked as rejected QSL_SENT (or equivalent) = N
	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*search_comp_t comparator*/ XP_NE,
		/*bool by_dates*/ false,
		/*std::string from_date*/"",
		/*std::string to_date;*/"",
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_AND,
		/*std::string field_name; */ field_name,
		/*std::string pattern;*/ "N",
		/*std::string my_call*/ station
	};
	char msg[128];
	snprintf(msg, sizeof(msg), "EXTRACT: Removing QSOs with %s=N - rejected", field_name.c_str());
	status_->misc_status(ST_NOTE, msg);
	criteria(new_criteria, server);

	if (size() == 0) {
		// No records match these criteria
		char message[128];
		snprintf(message, 128, "EXTRACT: No records for %s!", reason.c_str());
		status_->misc_status(ST_WARNING, message);
		// Clear the criteria - which also clears upload_in_progress
		clear_criteria();
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Select most recent QSO
		book_->selection(book_->size() - 1, HT_EXTRACTION);
	}
	else {
		// Records match
		char format[] = "EXTRACT: %d records extracted for %s";
		char* message = new char[strlen(format) + reason.length() + 10];
		sprintf(message, format, size(), reason.c_str());
		status_->misc_status(ST_OK, message);
		delete[] message;
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
		// Select first extracted record
		selection(0, HT_EXTRACTION);
	}
}

// Extract special - no eQSL image
void extract_data::extract_no_image() {
	std::string reason_name = "eQSL received";
	search_criteria_t new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*search_comp_t comparator*/ XP_EQ,
		/*bool by_dates*/ false,
		/*std::string from_date*/"",
		/*std::string to_date;*/"",
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*std::string field_name; */ "EQSL_QSL_RCVD",
		/*std::string pattern;*/ "Y",
		/*std::string my_call*/ "Any"
	};
	criteria(new_criteria);
	if (size() == 0) {
		// No records match these criteria
		status_->misc_status(ST_WARNING, "EXTRACT: No records match quick extract");
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Select most recent QSO
		book_->selection(book_->size() - 1, HT_EXTRACTION);
	}
	else {
		// Records match
		char format[] = "EXTRACT: %d records extracted %s";
		char message[128];
		snprintf(message, sizeof(message), format, size(), reason_name.c_str());
		status_->misc_status(ST_OK, message);
		// Now check existance of card image
		int count = 0;
		// For all records in this book
		item_num_t total = get_count();
		size_t checked = 0;
		status_->progress((int)total, OT_EXTRACT, "Extracting records with no card image", "records");
		for (item_num_t ixe = 0; ixe < get_count(); ) {
			// Check file exists for this record
			std::string filename = eqsl_handler_->card_filename_l(get_record(ixe, false));
			if (eqsl_handler_->card_file_valid(filename)) {
				// If it exists, remove the record pointer from this book
				erase(begin() + ixe);
				int ixb = mapping_[ixe];
				mapping_.erase(mapping_.begin() + ixe);
				rev_mapping_.erase(ixb);
				count += 1;
			}
			else {
				// Remap reverse mapping to the new index in this book.
				rev_mapping_[mapping_[ixe]] = ixe;
				ixe++;
			}
			checked++;
			status_->progress(checked, OT_EXTRACT);
		}
		snprintf(message, sizeof(message), "EXTRACT: %d records deleted, %zu total", count, size());
		status_->misc_status(ST_OK, message);
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
		// Select first extracted record
		selection(0, HT_EXTRACTION);
	}
}

// Extract records for special fixed criteria
void extract_data::extract_special(extract_data::extract_mode_t reason) {
	search_criteria_t new_criteria;
	std::string reason_name;
	switch (reason) {
	case NO_NAME:
		// Extract those records that have NAME field empty 
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_EQ,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_NEW,
			/*std::string field_name; */ "NAME",
			/*std::string pattern;*/ "",
			/*std::string my_call*/ "Any"
		};
		reason_name = "missing name";
		break;
	case NO_QTH:
		// Extract those records that have QTH field empty
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_EQ,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_NEW,
			/*std::string field_name; */ "QTH",
			/*std::string pattern;*/ "",
			/*std::string my_call*/ "Any"
		};
		reason_name = "missing QTH";
		break;
	case LOCATOR:
		// Extract those records with GRIDSQUARE empty or only 2 or 4 character locators 
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*search_comp_t comparator*/ XP_REGEX,
			/*bool by_dates*/ false,
			/*std::string from_date*/"",
			/*std::string to_date;*/"",
			/*std::string band;*/ "Any",
			/*std::string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_NEW,
			/*std::string field_name; */ "GRIDSQUARE",
			/*std::string pattern;*/ ".{0,4}",
			/*std::string my_call*/ "Any"
		};
		reason_name = "with insufficient locator";
		break;
	default:
		status_->misc_status(ST_ERROR, "EXTRACT: Unknown special extract");
		break;
	}
	criteria(new_criteria);
	if (size() == 0) {
		// No records match these criteria
		status_->misc_status(ST_WARNING, "EXTRACT: No records match quick extract");
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Select most recent QSO
		book_->selection(book_->size() - 1, HT_EXTRACTION);
	}
	else {
		// Records match
		char format[] = "EXTRACT: %d records extracted %s";
		char* message = new char[strlen(format) + reason_name.length() + 10];
		sprintf(message, format, size(), reason_name.c_str());
		status_->misc_status(ST_OK, message);
		delete[] message;
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
		// Select first extracted record
		selection(0, HT_EXTRACTION);
	}
}

// Upload the extracted data to the appropriate QSL server (eQSL, LotW or ClubLog)
void extract_data::upload() {
	switch (use_mode_) {
	case EQSL:
		if (eqsl_handler_->upload_eqsl_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: eQSL upload requested!");
		}
		break;
	case LOTW:
		if (lotw_handler_->upload_lotw_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: LotW upload requested!");
		}
		break;
	case CLUBLOG:
		if (club_handler_->upload_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: Clublog upload requested!");
		}
		break;
	case QRZCOM:
		if (qrz_handler_->upload_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: QRZ.com upload requested!");
		}
		break;
	case ALL:
		if (eqsl_handler_->upload_eqsl_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: eQSL upload requested!");
		}
		if (lotw_handler_->upload_lotw_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: LotW upload requested!");
		}
		if (club_handler_->upload_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: Clublog upload requested!");
		}
		if (qrz_handler_->upload_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: QRZ.com upload requested!");
		}
		break;
	default:
		status_->misc_status(ST_ERROR, "EXTRACT: Unknown server!");
		break;
	}
	// Remove the contents and criteria
	clear_criteria();
}

// Change the record selection (& update any necessary controls)
item_num_t extract_data::selection(item_num_t num_item, hint_t hint /* = HT_SELECTED */, view* requester /* = nullptr */, item_num_t num_other /* = 0 */) {
	// Set the current item in this view
	if ((signed)num_item != -1) {
		current_item_ = num_item;
	}
	// And select the same record in the main log view
	return item_number(book_->selection(record_number(current_item_), hint, requester, record_number(num_other)));
}

// Extract all records for callsign
void extract_data::extract_call(std::string callsign) {
	// Now check that they are all for the current station
	std::string station = qso_manager_->get_default(qso_manager::CALLSIGN);
	// Extract those records where CALL matches callsign 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_CALL,
		/*search_comp_t comparator*/ XP_EQ,
		/*bool by_dates*/ false,
		/*std::string from_date*/"",
		/*std::string to_date;*/"",
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*std::string field_name; */ "",
		/*std::string pattern;*/ callsign,
		/*std::string my_call;*/ station
	};
	criteria(new_criteria, SEARCH);
	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*search_comp_t comparator*/ XP_NE,
		/*bool by_dates*/ false,
		/*std::string from_date*/"",
		/*std::string to_date;*/"",
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_AND,
		/*std::string field_name; */ "QSO_COMPLETE",
		/*std::string pattern;*/ "N",
		/*std::string my_call;*/ station
	};
	criteria(new_criteria, SEARCH);
	if (size() == 0 || (book_->new_record() && size() == 1)) {
		// No records match these criteria
		status_->misc_status(ST_NOTE, "EXTRACT: Not worked this call before");
	}
	else {
		// Some records match
		char format[] = "EXTRACT: Call work %d times before - see extract page";
		char* message = new char[strlen(format) + 10];
		sprintf(message, format, size());
		status_->misc_status(ST_OK, message);
		delete[] message;
		// Select first record in std::list and display the extraction page
		selection(0, HT_EXTRACTION);
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
	}

}

// Extract all records for specified field
void extract_data::extract_field(std::string field_name, std::string value, bool and_search, std::string start, std::string endd) {
	// Now check that they are all for the current station
	std::string station = qso_manager_->get_default(qso_manager::CALLSIGN);
	// Extract those records where CALL matches callsign 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*search_comp_t comparator*/ XP_EQ,
		/*bool by_dates*/ start.length() != 0 || endd.length() != 0,
		/*std::string from_date*/start,
		/*std::string to_date;*/endd,
		/*std::string band;*/ "Any",
		/*std::string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ and_search ? XM_AND : XM_NEW,
		/*std::string field_name; */ field_name,
		/*std::string pattern;*/ value,
		/*std::string my_call;*/ station
	};
	criteria(new_criteria, SEARCH);
}

// Add the record to the extract std::list
void extract_data::add_record(qso_num_t record_num) {
	record* record = book_->get_record(record_num, false);
	item_num_t insert_point = get_insert_point(record);
	insert_record_at(insert_point, record);
	mapping_.insert(mapping_.begin() + insert_point, record_num);
	rev_mapping_[record_num] = insert_point;
	add_use_data(record);
	qso_manager_->enable_widgets();
}

// Compare records
bool extract_data::comp_records(record* lhs, record* rhs, std::string field, bool reversed) {
	if (field.length()) {
		if (reversed) {
			return rhs->item(field) < lhs->item(field);
		}
		else {
			return lhs->item(field) < rhs->item(field);
		}
	}
	else {
		if (reversed) {
			return rhs->timestamp() < lhs->timestamp();
		}
		else {
			return lhs->timestamp() < rhs->timestamp();
		}
	}
}


// Sort records according to field_name
void extract_data::sort_records(std::string field_name, bool reversed) {

	fl_cursor(FL_CURSOR_WAIT);
	item_num_t count = size();
	char message[100];
	snprintf(message, 100, "EXTRACT: Starting sorting %zu records on %s", size(), field_name.c_str());
	status_->misc_status(ST_NOTE, message);
	status_->progress(size(), book_type(), "Hanging log", "Records");

	sort_node* root = new sort_node(record_number(0), nullptr);
	// Tree sort - hang the qsos on the tree
	for (size_t ix = 1; ix < size(); ix++) {
		bool hung = false;
		sort_node* hanger = root;
		while (!hung) {
			if (comp_records(at(ix), at(item_number(hanger->qso_num)), field_name, reversed)) {
				if (hanger->left == nullptr) {
					sort_node* node = new sort_node(record_number(ix), hanger);
					hanger->left = node;
					hung = true;
				}
				else {
					hanger = hanger->left;
				}
			}
			else {
				if (hanger->right == nullptr) {
					sort_node* node = new sort_node(record_number(ix), hanger);
					hanger->right = node;
					hung = true;
				}
				else {
					hanger = hanger->right;
				}
			}
		}
		status_->progress(ix, book_type());
	}
	// Now unravel the tree
	status_->progress(size(), book_type(), "Picking log", "Records");
	clear();
	// Clear the mappings
	mapping_.clear();
	rev_mapping_.clear();
	// now tidy up this book, records have already been removed so will not be deleted
	delete_contents(true);

	int copied = pick_node(root);
	if (copied != count) {
		char msg[128];
		snprintf(msg, sizeof(msg), "DEBUG: Wrong number picked from tree %d picked, %zd hung\n", copied, count);
		status_->misc_status(ST_ERROR, msg);
	}
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Place the node and all those beneath it in the extracted std::list
int extract_data::pick_node(sort_node* n) {
	int result = 0;
	// Add all the nodes on the left
	if (n->left) {
		result += pick_node(n->left);
	}
	// We have none remaining in the left, so this node is next
	item_num_t item = size();
	push_back(book_->at(n->qso_num));
	mapping_.push_back(n->qso_num);
	rev_mapping_[n->qso_num] = item;
	result++;
	status_->progress(item + 1, book_type());
	// Add all the nodes on the right
	if (n->right) {
		result += pick_node(n->right);
	}
	// Now return all the nodes picked
	return result;
}

// Undo the above sort 
void extract_data::correct_record_order() {
	sort_records("", false);
	//fl_cursor(FL_CURSOR_WAIT);
	//item_num_t count = size();
	//int num_scans = 0;
	//char message[100];
	//snprintf(message, 100, "EXTRACT: Starting sorting %zu records on date/time", size());
	//status_->misc_status(ST_NOTE, message);
	//status_->progress(size(), book_type(), "Undoing custom sort", "Sorting passes");
	//// Repeat until we no longer swap anything
	//// NB: We may have to implement more efficient sort algorithm, if size() increases much.
	//// Current 2K+ records takes a couple of seconds
	//while (count > 0) {
	//	count = 0;
	//	// Compare each record with its immediate follower - swap if it's larger
	//	for (item_num_t ix = 0; ix < size() - 1; ix++) {
	//		if (*at(ix) > *at(ix + 1)) {
	//			swap_records(ix, ix + 1);
	//			count++;
	//		}
	//	}
	//	num_scans++;
	//	status_->progress(num_scans, book_type());
	//}
	//snprintf(message, 100, "EXTRACT: Done - %d passes required", num_scans);
	//status_->misc_status(ST_OK, message);
	//// Note may have taken fewer passes than primed progress bar with - stop progress if it has
	//if (num_scans < (signed)size()) {
	//	status_->progress("Taken fewer passes", book_type());
	//}
	//fl_cursor(FL_CURSOR_DEFAULT);
}

// Return whether an existing upload is in progress
bool extract_data::upload_in_progress() {
	switch (use_mode_) {
	case EQSL:
	case LOTW:
	case CLUBLOG:
		return true;
	default:
		return false;
	}
}

extract_data::extract_mode_t extract_data::use_mode() {
	return use_mode_;
}

void extract_data::use_mode(extract_data::extract_mode_t mode) {
	use_mode_ = mode;
}

// Check  that the record metches our current criteria and process it if it does
void extract_data::check_add_record(qso_num_t record_num) {
	record* qso = book_->get_record(book_->item_number(record_num), false);
	if (meets_criteria(qso)) {
		if (record_num == book_->size() - 1) {
		// It's the last in the book so we can simply add it
			add_record(record_num);
		}
		else {
		// Else we have to re-extract
			reextract();
		}
	} 
}

// Does the record meet our current criteria
bool extract_data::meets_criteria(record* qso) {
	bool match = false;
	// For all sets of criteria in the history stack
	for (auto it = extract_criteria_.begin(); it != extract_criteria_.end(); it++) {
		// Repeat the extraction
		criteria_ = &(*it);
		switch (criteria_->combi_mode) {
		case XM_NEW:
			match = false;
			//Compare record against seaarch criteria
			if (match_record(qso)) {
				match = true;
			}
			break;
		case XM_AND:
			if (match && !match_record(qso)) {
				match = false;
			}
			break;
		case XM_OR:
			if (!match && match_record(qso)) {
				match = true;
			}
			break;
		}
	}
	return match;
}

// Map records
void extract_data::map_record(qso_num_t record_num) {
	int ixe = size() - 1;
	// Add to both ways mappings
	mapping_.insert(mapping_.begin() + ixe, record_num);
	rev_mapping_[record_num] = ixe;

}
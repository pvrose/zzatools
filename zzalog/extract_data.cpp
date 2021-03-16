#include "extract_data.h"
#include "prefix.h"
#include "pfx_data.h"
#include "../zzalib/utils.h"
#include "tabbed_forms.h"
#include "status.h"
#include "adi_writer.h"
#include "eqsl_handler.h"
#include "lotw_handler.h"
#include "spec_data.h"
#include "club_handler.h"

#include <sstream>

#include <FL/fl_ask.H>

using namespace zzalog;

extern book* book_;
extern book* navigation_book_;
extern pfx_data* pfx_data_;
extern tabbed_forms* tabbed_forms_;
extern status* status_;
extern eqsl_handler* eqsl_handler_;
extern lotw_handler* lotw_handler_;
extern club_handler* club_handler_;
extern spec_data* spec_data_;

// Constructor
extract_data::extract_data() :
	book()
	, use_mode_(NONE)
{
	// This book contains extract data
	book_type_ = OT_EXTRACT;
	extract_criteria_.clear();
	mapping_.clear();
	rev_mapping_.clear();
}

// Destructor
extract_data::~extract_data()
{
	// Same as book::delete_contents without deleting the records - these are wanted
	clear();
	modified(false);
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
	// set criteria
	use_mode_ = mode;
	// Append to the set of criteria
	extract_criteria_.push_back(criteria);
	// Get the most recent
	criteria_ = &extract_criteria_.back();
	// Get the matching record from the main log book
	extract_records();
	return size();
}

// Add all records that match in main log to this log
void extract_data::extract_records() {
	record_num_t count = 0;
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
		// For all records in main log book
		for (record_num_t ixb = 0; ixb < book_->size(); ixb++) {
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
		}
		snprintf(message, 100, "EXTRACT: %d records extracted, %d total", count, size());
		break;
	case XM_AND:
		// Logical AND between existing and new criteria - i.e. only those that match both
		// Append new reason for search to header
		if (header_ == nullptr) header_ = new record;
		header_->header(header_->header() + '\n' + comment());
		// For all records in this book
		for (record_num_t ixe = 0; ixe < get_count(); ) {
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
		}
		snprintf(message, 100, "EXTRACT: %d records deleted, %d total", count, size());
		break;
	case XM_OR: 
		// Logical OR between existing search and new criteria - i.e. those that match either
		// Append new reason for search to header
		if (header_ == nullptr) header_ = new record;
		header_->header(header_->header() + '\n' + comment());
		// For all records in main log book
		for (record_num_t ixb = 0; ixb < book_->get_count(); ixb++) {
			record* test_record = book_->get_record(ixb, false);
			// Compare record against search criteria
			if (match_record(test_record)) {
				// If it matches find insert point in this book
				record_num_t ixe = get_insert_point(test_record);
				// If this record is not already at the insert point
				if (get_record(ixe, false) != book_->get_record(ixb, false)) {
					// Add the record to this book
					insert_record_at(ixe, test_record);
					mapping_.insert(mapping_.begin() + ixe, ixb);
					rev_mapping_[ixb] = ixe;
					count += 1;
				}
			}
		}
		snprintf(message, 100, "EXTRACT: %d records added, %d total", count, size());
		break;
	}
	status_->misc_status(ST_OK, message);
	navigation_book_ = this;

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
	current_item_ = 0;
	// now tidy up this book
	delete_contents(true);
	use_mode_ = NONE;
	// Set navigation book to main log
	if (redraw) {
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Cause the views to be redrawn
		selection(-1, HT_EXTRACTION);
	}
}

// Convert item index in this book to the record index in the main log book
inline record_num_t extract_data::record_number(record_num_t item_number) {
	if (size() > 0) 
		// Return the mapping from this book to the main book
		return mapping_[item_number];
	else return -1;
}

// Convert record index in the main log book to the item index in this book
// nearest = true will choose the closest item to the record number
inline record_num_t extract_data::item_number(record_num_t record_number, bool nearest /*=false*/) {
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
			record_num_t lbound = 0;
			record_num_t ubound = mapping_.size() - 1;
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
			record_num_t test;
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
string extract_data::comment() {
	string result;
	if (criteria_) {
		result = "Data extracted for criteria:-\n";
		// Add the main condition
		switch (criteria_->condition) {
		case XC_DXCC:
			result += "DXCC ";
			break;
		case XC_GEO:
			result += "Prefix ";
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
		}
		if (criteria_->condition != XC_UNFILTERED) {
			if (criteria_->by_regex) {
				// Add if using regular expression matching
				result += "matches regex " + criteria_->pattern + ".\n";
			}
			else {
				// Or exact matching
				result += "= " + criteria_->pattern + ".\n";
			}
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
		// Only select those which didn't match
		if (criteria_->negate_results) result += "Results negated. ";
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
string extract_data::short_comment() {
	string result;
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
		}
		// Only select those which didn't match
		if (criteria_->negate_results) result += "¬ ";
		// Add the main condition
		switch (criteria_->condition) {
		case XC_DXCC:
			result += "DX";
			break;
		case XC_GEO:
			result += "PX";
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
		}
		if (criteria_->condition != XC_UNFILTERED) {
			if (criteria_->by_regex) {
				// Add if using regular expression matching
				result += "~" + criteria_->pattern;
			}
			else {
				// Or exact matching
				result += "=" + criteria_->pattern;
			}
		}
		result += " ";
		if (criteria_->by_dates) {
			// Date-range restricted
			result += " (" + criteria_->from_date += ":" + criteria_->to_date + ") ";
		}
		// Band and Mode
		result += "On " + criteria_->band + " " + criteria_->mode + " ";
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
	string reason;
	string field_name;
	// Define the field that shows it's not been sent
	switch (server) {
	case EQSL:
		reason = "eQSL";
		field_name = "EQSL_QSL_SENT";
		break;
	case LOTW:
		reason = "LotW";
		field_name = "LOTW_QSL_SENT";
		break;
	case CARD:
		reason = "Printed card";
		field_name = "QSL_SENT";
		break;
	case CLUBLOG:
		reason = "ClubLog";
		field_name = "CLUBLOG_QSO_UPLOAD_STATUS";
		break;
	}

	// Extract those records not sent to QSL server !(*QSL_SENT==Y) 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*bool by_regex*/ false,
		/*bool by_dates*/ false,
		/*string from_date*/"",
		/*string to_date;*/"",
		/*string band;*/ "Any",
		/*string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*bool negate_results;*/ true,
		/*string field_name; */ field_name,
		/*string pattern;*/ "Y"
	};
	criteria(new_criteria, server);
	// Only send those whose QSO is complete !(QSO_COMPLETE==N)
	new_criteria = {
		/*search_cond_t condition*/ XC_FIELD,
		/*bool by_regex*/ false,
		/*bool by_dates*/ false,
		/*string from_date*/"",
		/*string to_date;*/"",
		/*string band;*/ "Any",
		/*string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_AND,
		/*bool negate_results;*/ true,
		/*string field_name; */ "QSO_COMPLETE",
		/*string pattern;*/ "N"
	};
	criteria(new_criteria, server);
	if (server == LOTW || server == CLUBLOG) {
		// Only send those to which are QSOs !(SWL==Y)
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*bool by_regex*/ false,
			/*bool by_dates*/ false,
			/*string from_date*/"",
			/*string to_date;*/"",
			/*string band;*/ "Any",
			/*string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*bool negate_results;*/ true,
			/*string field_name; */ "SWL",
			/*string pattern;*/ "Y"
		};
		criteria(new_criteria, server);
	}
	if (server == CARD) {
		// Only those for which we have received a card (QSL_RCVD==Y) - i.e. QSLL
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*bool by_regex*/ false,
			/*bool by_dates*/ false,
			/*string from_date*/"",
			/*string to_date;*/"",
			/*string band;*/ "Any",
			/*string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_AND,
			/*bool negate_results;*/ false,
			/*string field_name; */ "QSL_RCVD",
			/*string pattern;*/ "Y"
		};
		criteria(new_criteria, server);
		sort_records("DXCC", false);
		tabbed_forms_->update_views(nullptr, HT_RESET_ORDER, 0);

	}
	if (size() == 0) {
		// No records match these criteria
		status_->misc_status(ST_WARNING, "EXTRACT: No records to upload!");
		tabbed_forms_->activate_pane(OT_MAIN, true);
		// Select most recent QSO
		book_->selection(book_->size() - 1, HT_EXTRACTION);
	}
	else {
		// Records match
		char format[] = "EXTRACT: %d records extracted for sending to %s";
		char* message = new char[strlen(format) + reason.length() + 10];
		sprintf(message, format, size(), reason.c_str());
		status_->misc_status(ST_OK, message);
		delete[] message;
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
		// Select first extracted record
		selection(0, HT_EXTRACTION);
	}
}

// Extract records for special fixed criteria
void extract_data::extract_special(extract_data::extract_mode_t reason) {
	search_criteria_t new_criteria;
	string reason_name;
	switch (reason) {
	case NO_NAME:
		// Extract those records that have NAME field empty 
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*bool by_regex*/ false,
			/*bool by_dates*/ false,
			/*string from_date*/"",
			/*string to_date;*/"",
			/*string band;*/ "Any",
			/*string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_NEW,
			/*bool negate_results;*/ false,
			/*string field_name; */ "NAME",
			/*string pattern;*/ ""
		};
		reason_name = "missing name";
		break;
	case NO_QTH:
		// Extract those records that have QTH field empty
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*bool by_regex*/ false,
			/*bool by_dates*/ false,
			/*string from_date*/"",
			/*string to_date;*/"",
			/*string band;*/ "Any",
			/*string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_NEW,
			/*bool negate_results;*/ false,
			/*string field_name; */ "QTH",
			/*string pattern;*/ ""
		};
		reason_name = "missing QTH";
		break;
	case LOCATOR:
		// Extract those records with GRIDSQUARE empty or only 2 or 4 character locators 
		new_criteria = {
			/*search_cond_t condition*/ XC_FIELD,
			/*bool by_regex*/ true,
			/*bool by_dates*/ false,
			/*string from_date*/"",
			/*string to_date;*/"",
			/*string band;*/ "Any",
			/*string mode;*/ "Any",
			/*bool confirmed_eqsl;*/ false,
			/*bool confirmed_lotw;*/ false,
			/*bool confirmed_card;*/ false,
			/*search_combi_t combi_mode;*/ XM_NEW,
			/*bool negate_results;*/ true,
			/*string field_name; */ "GRIDSQUARE",
			/*string pattern;*/ "[A-R]{2}[0-9]{2}[A-X]{2}([0-9]{2})?"
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
			status_->misc_status(ST_OK, "EXTRACT: eQSL upload successful!");
		}
		break;
	case LOTW:
		if (lotw_handler_->upload_lotw_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: LotW upload successful!");
		}
		break;
	case CLUBLOG:
		if (club_handler_->upload_log(this)) {
			status_->misc_status(ST_OK, "EXTRACT: Clublog upload successful!");
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
record_num_t extract_data::selection(record_num_t num_item, hint_t hint /* = HT_SELECTED */, view* requester /* = nullptr */, record_num_t num_other /* = 0 */) {
	// Set the current item in this view
	if ((signed)num_item != -1) {
		current_item_ = num_item;
	}
	// And select the same record in the main log view
	return book_->selection(record_number(current_item_), hint, requester, num_other);
}

// Extract all records for callsign
void extract_data::extract_call(string callsign) {
	// Extract those records where CALL matches callsign 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_CALL,
		/*bool by_regex*/ false,
		/*bool by_dates*/ false,
		/*string from_date*/"",
		/*string to_date;*/"",
		/*string band;*/ "Any",
		/*string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*bool negate_results;*/ false,
		/*string field_name; */ "",
		/*string pattern;*/ callsign
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
		// Select first record in list and display the extraction page
		selection(0, HT_EXTRACTION);
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
	}

}

// Add the record to the extract list
void extract_data::add_record(record_num_t record_num) {
	record* record = book_->get_record(record_num, false);
	record_num_t insert_point = get_insert_point(record);
	insert_record_at(insert_point, record);
	mapping_.insert(mapping_.begin() + insert_point, record_num);
	rev_mapping_[record_num] = insert_point;
}

// Swap two records
void extract_data::swap_records(record_num_t first, record_num_t second) {
	// Swap records
	record* record_1 = at(first);
	at(first) = at(second);
	at(second) = record_1;
	// Swap mapping data
	record_num_t record_num_1 = mapping_.at(first);
	mapping_.at(first) = mapping_.at(second);
	mapping_.at(second) = record_num_1;
	// Repair the reverse mapping
	rev_mapping_[mapping_.at(first)] = first;
	rev_mapping_[mapping_.at(second)] = second;
}

// Sort records according to field_name
void extract_data::sort_records(string field_name, bool reversed) {
	fl_cursor(FL_CURSOR_WAIT);
	record_num_t count = size();
	int num_scans = 0;
	char message[100];
	snprintf(message, 100, "EXTRACT: Starting sorting %d records on %s", size(), field_name.c_str());
	status_->misc_status(ST_NOTE, message);
	status_->progress(size(), book_type(), "Sorting passes");
	// Repeat until we no longer swap anything
	// NB: We may have to implement more efficient sort algorithm, if size() increases much.
	// Current 2K+ records takes a couple of seconds
	while (count > 0) {
		count = 0;
		// Compare each record with its immediate follower - swap if it's larger
		for (record_num_t ix = 0; ix < size() - 1; ix++) {
			if (spec_data_->datatype_indicator(field_name) == 'N') {
				// Numeric field - compare the numeric value - basically ignore white space
				double item_1;
				double item_2;
				at(ix)->item(field_name, item_1);
				at(ix + 1)->item(field_name, item_2);
				if ((!reversed && item_1 > item_2) ||
					(reversed && item_1 < item_2)) {
					swap_records(ix, ix + 1);
					count++;
				}
			}
			else {
				// compare string value
				if ((!reversed && at(ix)->item(field_name, true) > at(ix + 1)->item(field_name, true)) ||
					(reversed && at(ix)->item(field_name, true) < at(ix + 1)->item(field_name, true))) {
					swap_records(ix, ix + 1);
					count++;
				}
			}
		}
		num_scans++;
		status_->progress(num_scans, book_type());
	}
	snprintf(message, 100, "EXTRACT: Done - %d passes required", num_scans);
	status_->misc_status(ST_OK, message);
	// Note may have taken fewer passes than primed progress bar with - stop progress if it has
	if (num_scans < (signed)size()) {
		status_->progress("Taken fewer passes", book_type());
	}
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Undo the above sort 
void extract_data::correct_record_order() {
	fl_cursor(FL_CURSOR_WAIT);
	record_num_t count = size();
	int num_scans = 0;
	char message[100];
	snprintf(message, 100, "EXTRACT: Starting sorting %d records on date/time", size());
	status_->misc_status(ST_NOTE, message);
	status_->progress(size(), book_type(), "Sorting passes");
	// Repeat until we no longer swap anything
	// NB: We may have to implement more efficient sort algorithm, if size() increases much.
	// Current 2K+ records takes a couple of seconds
	while (count > 0) {
		count = 0;
		// Compare each record with its immediate follower - swap if it's larger
		for (record_num_t ix = 0; ix < size() - 1; ix++) {
			if (*at(ix) > *at(ix + 1)) {
				swap_records(ix, ix + 1);
				count++;
			}
		}
		num_scans++;
		status_->progress(num_scans, book_type());
	}
	snprintf(message, 100, "EXTRACT: Done - %d passes required", num_scans);
	status_->misc_status(ST_OK, message);
	// Note may have taken fewer passes than primed progress bar with - stop progress if it has
	if (num_scans < (signed)size()) {
		status_->progress("Taken fewer passes", book_type());
	}
	fl_cursor(FL_CURSOR_DEFAULT);
}
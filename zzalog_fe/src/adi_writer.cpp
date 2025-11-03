#include "adi_writer.h"

#include "book.h"
#include "fields.h"
#include "main.h"
#include "record.h"
#include "spec_data.h"
#include "status.h"

#include "utils.h"

#include <fstream>
#include<ostream>
#include <cmath>

#include <FL/Fl.H>
#include <FL/fl_draw.H>


// Default constructor
adi_writer::adi_writer()
{
	clean_records_ = false;
	out_book_ = nullptr;
	current_ = 0;
}

// Default constructor
adi_writer::~adi_writer()
{
}

// write book to output stream. If fields is not null then only the specified fields
bool adi_writer::store_book(book* out_book, std::ostream& out, bool clean, field_list* fields /* = nullptr */) {
	// Takes a finite time so put the timer cursor up.
	fl_cursor(FL_CURSOR_WAIT);
	bool result = true;
	clean_records_ = clean;
	out_book_ = out_book;

	// configure progress bar - progress is counted by number of records processed
	status_->misc_status(ST_NOTE, "LOG: Started writing ADI");
	status_->progress(out_book->size() + 1, out_book->book_type(), "Storing ADIF", "records");

	// If exported data check ADIF compliance
	if (out_book->book_type() != OT_MAIN) {
		unsigned char check = adif_compliance(out_book, fields);
		if (check & NON_LATIN) {
			status_->misc_status(ST_ERROR, "Exported data contains characters outwith ASCII or ISO-8859-1");
		}
		else if (check & LATIN_1) {
			status_->misc_status(ST_ERROR, "Exported data contains non-ASCII but within ISO-8859-1");
		}
	}

	// For all records and while the output is successful
	if (out_book->header()) {
		store_record(out_book->header(), out, result, nullptr);
		status_->progress(1, out_book->book_type());
	}
	for (current_ = 0; current_ < out_book->size() && result; current_++) {
		// Output the record
		store_record(out_book->get_record(current_, false), out, result, fields);
		status_->progress(current_ + 1, out_book->book_type());
	}
	// Update the progress bar with complete or failed
	if (result) {
		status_->progress(out_book->size() + 1, out_book->book_type());
		status_->misc_status(ST_OK, "LOG: Writing done!");
	}
	else {
		status_->misc_status(ST_ERROR, "LOG: Writing failed");
		status_->progress("Write failed", out_book->book_type());
	}
	clean_records_ = false;
	// restore the cursor
	fl_cursor(FL_CURSOR_DEFAULT);
	return result;
}

// write record to output stream. If fields is not null, then only these fields. Output result not used
std::ostream& adi_writer::store_record(record* record, std::ostream& out, bool& result, field_list* fields /* = nullptr */) {
	// convert to text
	to_adif(record, out, fields);
	if (clean_records_) out_book_->delete_dirty_record(record);
	return out;
}

// Convert item to ADIF format text and send to the output stream
std::string adi_writer::item_to_adif(record* record, std::string field) {
	std::string value;
	std::string adif_text = "";
	unsigned int len_value;
	//  <KEYWORD:length[:type]>VALUE
	value = record->item(field);
	len_value = value.length();
	// minimum size of "<FIELD:n>VALUE " plus a safety margin
	int out_size = len_value + field.length() + (int)log10(len_value) + 6;
	// Get the type indicator from the ADIF spec database
	char type_indicator = spec_data_->datatype_indicator(field);
	// Don't write out any items that are an empty std::string.
	if (len_value > 0) {
		char* temp = nullptr;
		if (field.length() > 3 && field.substr(0, 3) == "APP") {
			// All application defined fields should include type character 
			if (type_indicator == ' ') {
				// But if it was not present on the input ADIF don't create one
				temp = new char[out_size];
				sprintf(temp, "<%s:%d>%s ", field.c_str(), len_value, value.c_str());
			}
			else {
				// Add 2 bytes for the ":%c" for the type indicator
				temp = new char[out_size + 2];
				sprintf(temp, "<%s:%d:%c>%s ", field.c_str(), len_value, type_indicator, value.c_str());
			}
		}
		else if (field.length() > 7 && field.substr(0, 7) == "USERDEF") {
			// user defined fields require type and optional std::list or range of values
			std::string list_range = spec_data_->userdef_values(field);
			if (list_range.length() > 0) {
				// Add enough bytes to cover the type indicator and the userdef range length
				len_value += 3 + list_range.length();
				temp = new char[out_size + 3 + list_range.length()];
				sprintf(temp, "<%s:%d:%c>%s,{%s} ", field.c_str(), len_value, type_indicator, value.c_str(), list_range.c_str());
			}
			else {
				// Add enough bytes for the type indicator
				temp = new char[out_size + 2];
				sprintf(temp, "<%s:%d:%c>%s ", field.c_str(), len_value, type_indicator, value.c_str());
			}
		}
		else {
			// ADIF defined fields without type indicator
			temp = new char[out_size];
			sprintf(temp, "<%s:%d>%s ", field.c_str(), len_value, value.c_str());
		}
		// output the ADIF format text to the stream
		return std::string(temp);
		delete[] temp;
	}
	return "";
}

// Convert record to ADIF format text
void adi_writer::to_adif(record* record, std::ostream& out, field_list* fields /* = nullptr */) {
	std::string temp;

	// Header - write out any comment first - 
	if (record->is_header()) {
		out << record->header();
	}
	// Write out each ADIF field
	for (auto it = record->begin(); it != record->end(); it++) {
		std::string field = it->first;
		std::string value = it->second;
		bool in_filter = false;
		if (fields) {
			for (auto it = fields->begin(); it != fields->end() && !in_filter; it++) {
				if ((*it) == field) {
					in_filter = true;
				}
			}
		}
		// If field name is valid and either header record, no field filtering or the field is in the filter
		if (field != "" && (record->is_header() || fields == nullptr || in_filter)) {
			// Test whether field name end in _INTL
			if (field.length() > 5 && field.substr(field.length() - 5) == "_INTL") {
				std::string non_intl_field = field.substr(0, field.length() - 5);
				if (!record->item_exists(non_intl_field)) {
					// ..._INTL exists and other doesn't - output it as non_intl
					record->change_field_name(field, non_intl_field);
					out << item_to_adif(record, non_intl_field);
				} // else if both exists don't output _INTL
			}
			else {
				// send the field to the output stream
				out << item_to_adif(record, field);
			}
		}
	}
	if (record->is_header()) {
		std::string year = now(false, "%Y");
		char copyright[128];
		snprintf(copyright, sizeof(copyright), DATA_COPYRIGHT.c_str(), year.c_str());
		// Add red-tape after header fields
		out << endl;
		out << PROGRAM_ID << endl;
		out << copyright << endl;
		out << "<EOH>" << endl << endl;
	}
	else {
		// Add <EOR>
		out << "<EOR>" << endl << endl;
	}
}

// Calculate the progress towards saving
double adi_writer::progress() {
	return (double)current_ / (double)out_book_->size();
}

// Categorise UTF-8 character (ASCII, ISO-8859-1 or neither)
unsigned char adi_writer::adif_char(unsigned int utf8) {
	if (utf8 < 0x20) {
		return CONTROL;
	}
	else if (utf8 >= 0x20 && utf8 <= 0x7E) {
		return 0;
	}
	else if (utf8 >= 0xA0 && utf8 <= 0xFF) {
		// ISO-8859-1
		return LATIN_1;
	}
	else {
		return NON_LATIN;
	}
}

// Check contents are ADIF compliant
unsigned char adi_writer::adif_compliance(book* b, field_list* fields) {
	unsigned char result = 0;
	for (auto qso : *b) {
		if (fields) {
			for (auto field : *fields) {
				std::string item = qso->item(field);
				const char* pos = item.c_str();
				const char* pend = pos + item.length();
				int len;
				while (pos < pend) {
					unsigned int utf8 = fl_utf8decode(pos, pend, &len);
					result |= adif_char(utf8);
					pos += len;
				}
			}
		}
		else {
			for (auto field : *qso) {
				const char* pos = field.second.c_str();
				const char* pend = pos + field.second.length();
				int len;
				while (pos < pend) {
					unsigned int utf8 = fl_utf8decode(pos, pend, &len);
					result |= adif_char(utf8);
					pos += len;
				}
			}
		}
	}
	return result;
}

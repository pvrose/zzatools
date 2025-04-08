#include "adi_writer.h"

#include "status.h"
#include "record.h"
#include "book.h"
#include "files.h"
#include "spec_data.h"
#include "utils.h"
#include "fields.h"

#include <fstream>
#include <ostream>
#include <cmath>

#include <FL/Fl.H>
#include <FL/fl_draw.H>




extern status* status_;
extern spec_data* spec_data_;
extern string COPYRIGHT;
extern string PROGRAM_ID;

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
bool adi_writer::store_book(book* out_book, ostream& out, bool clean, field_list* fields /* = nullptr */) {
	// Takes a finite time so put the timer cursor up.
	fl_cursor(FL_CURSOR_WAIT);
	load_result_t result = LR_GOOD;
	clean_records_ = clean;
	out_book_ = out_book;

	// configure progress bar - progress is counted by number of records processed
	status_->misc_status(ST_NOTE, "LOG: Started writing ADI");
	status_->progress(out_book->size() + 1, out_book->book_type(), "Storing ADIF", "records");
	// For all records and while the output is successful
	if (out_book->header()) {
		store_record(out_book->header(), out, result, nullptr);
		status_->progress(1, out_book->book_type());
	}
	for (current_ = 0; current_ < out_book->size() && result == LR_GOOD; current_++) {
		// Output the record
		store_record(out_book->get_record(current_, false), out, result, fields);
		status_->progress(current_ + 1, out_book->book_type());
	}
	// Update the progress bar with complete or failed
	if (result == LR_GOOD) {
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
ostream& adi_writer::store_record(record* record, ostream& out, load_result_t& result, field_list* fields /* = nullptr */) {
	// convert to text
	to_adif(record, out, fields);
	if (clean_records_) out_book_->delete_dirty_record(record);
	return out;
}

// Convert item to ADIF format text and send to the output stream
string adi_writer::item_to_adif(record* record, string field) {
	string value;
	string adif_text = "";
	unsigned int len_value;
	//  <KEYWORD:length[:type]>VALUE
	value = record->item(field);
	len_value = value.length();
	// minimum size of "<FIELD:n>VALUE " plus a safety margin
	int out_size = len_value + field.length() + (int)log10(len_value) + 6;
	// Get the type indicator from the ADIF spec database
	char type_indicator = spec_data_->datatype_indicator(field);
	// Don't write out any items that are an empty string.
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
			// user defined fields require type and optional list or range of values
			string list_range = spec_data_->userdef_values(field);
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
		return string(temp);
		delete[] temp;
	}
	return "";
}

// Convert record to ADIF format text
void adi_writer::to_adif(record* record, ostream& out, field_list* fields /* = nullptr */) {
	string temp;

	// Header - write out any comment first - 
	if (record->is_header()) {
		out << record->header();
	}
	// Write out each ADIF field
	for (auto it = record->begin(); it != record->end(); it++) {
		string field = it->first;
		string value = it->second;
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
				string non_intl_field = field.substr(0, field.length() - 5);
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
		// Add red-tape after header fields
		out << endl;
		out << PROGRAM_ID << endl;
		out << COPYRIGHT << endl;
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

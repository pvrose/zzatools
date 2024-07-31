#include "adx_writer.h"
#include "adx_reader.h"
#include "spec_data.h"
#include "book.h"
#include "record.h"

#include "status.h"
#include "utils.h"

#include <map>
#include <string>

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>




extern spec_data* spec_data_;
extern status* status_;

using namespace std;

// Standard constructor
adx_writer::adx_writer()
	: record_(nullptr)
	, my_book_(nullptr)
	, field_name_("")
	, value_("")
	, type_indicator_(' ')
	, clean_records_(false)
{
}

// Satndard destructor
adx_writer::~adx_writer()
{
}

// Generate XML for the records in book and send them to the output stream
bool adx_writer::store_book(book* book, ostream& os, bool clean) {
	bool ok = true;
	clean_records_ = clean;
	// Takes time so display the timer cursor
	fl_cursor(FL_CURSOR_WAIT);

	// Get the book
	my_book_ = book;
	if (my_book_ == nullptr) {
		status_->misc_status(ST_SEVERE, "LOG: No book to write");
		ok = false;
	}
	else {
		// configure progress widget
		status_->misc_status(ST_NOTE, "LOG: XML Generation started");
		status_->progress(book->size(), book->book_type(), "Converting ADIF to XML", "records");
		// Write the outer element - will iteratively write sub-elements
		ok &= write_element(AET_NONE);
		if (ok) {
			// Successful
			status_->misc_status(ST_OK, "LOG: XML Generation done!");
			status_->progress(book->size(), book->book_type());
		} else {
			// Not successful
			status_->misc_status(ST_ERROR, "LOG: XML Generation failed");
			status_->progress("Process failed", book->book_type());
		}
	}
	clean_records_ = false;

	fl_cursor(FL_CURSOR_DEFAULT);
	// now write the destination to a file.
	if (ok) {
		status_->misc_status(ST_NOTE, "LOG: XML Storing started");
		status_->progress(book->size(), book->book_type(), "Storing XML", "records");
		ok &= data(os);
	}
	if (ok) {
		status_->misc_status(ST_OK, "LOG: XML Storing done!");
		status_->progress(1, book->book_type());
		return ok;
	}
	else {
		status_->misc_status(ST_ERROR, "LOG: XML Storing failed");
		status_->progress("Write failed", book->book_type());
		return false;
	}

}

// Generate an individual element - this will iteratively generate further elements
bool adx_writer::write_element(adx_element_t element) {

	string name; 
	string data;
	char datatype;
	string temp;
	int index1;
	int index2;
	int total;
	bool ok = true;
	map<string, string>* attributes;
	switch (element) {
	case AET_NONE:
		// Initial prolog <?xml version="1.0" encoding="utf-8" ?>
		name = "xml";
		data = "version=\"1.0\" encoding=\"utf-8\" ";
		ok = process_instr(name, data);
		// Top-level element
		ok &= write_element(AET_ADX);
		break;
	case AET_ADX:
		// ADX element
		name = "ADX";
		ok = start_element(name, nullptr);
		if (ok) {
			// write HEADER element if the book has a header record
			record_ = my_book_->header();
			if (record_ != nullptr) {
				ok &= write_element(AET_HEADER);
			}
		}
		if (ok) {
			// write RECORDS element
			ok &= write_element(AET_RECORDS);
		}
		ok &= end_element(name);
		break;
	case AET_HEADER:
		// HEADER element
		name = "HEADER";
		ok = start_element(name, nullptr);
		if (record_->header() != "") {
			// Add XML comment for the header comment
			value_ = record_->header();
			ok &= write_element(AET_COMMENT);
		}
		// Iterate through the fields - either USERDEF, APP or named field
		for (auto it = record_->begin(); it != record_->end() && ok; it++) {
			field_name_ = it->first;
			value_ = record_->item(field_name_, true);
			if (field_name_.length() > 7 && field_name_.substr(0,7) == "USERDEF") {
				ok = write_element(AET_USERDEFH);
			}
			else if (field_name_.length() > 3 && field_name_.substr(0,3) == "APP") {
				ok = write_element(AET_APP);
			}
			else {
				ok = write_element(AET_FIELD);
			}
		}
		ok &= end_element(name);
		break;
	case AET_FIELD:
		// FIELD element
		ok = start_element(field_name_, nullptr);
		ok &= characters(value_);
		ok &= end_element(field_name_);
		break;
	case AET_USERDEFH:
		// Header USERDEF element
		/* attributes:
		FIELDID = field_name_.substr(7)
		TYPE = theApp.GetReference()->GetDataTypeIndicator(field_name_)
		if (TYPE == "E") ENUM = ....->GetUserdefValue() 
		if (TYPE == "N") RANGE = ....->GetUserdefValue() unless ""
		*/
		attributes = new map<string, string>;
		attributes->clear();
		// Add attribute FIELDID="<id>" - represents USERDEFn used in book
		(*attributes)["FIELDID"] = field_name_.substr(7);
		// value_ is the name of the user defined field, and it's that type indicator we want
		datatype = spec_data_->datatype_indicator(value_);
		// add attribute TYPE="<type>"
		(*attributes)["TYPE"] = datatype;
		// Get the enum list or range - returns empty string if not enum or numeric or its unbounded
		temp = spec_data_->userdef_values(value_);
		if (temp != "") {
			string temp2 = '{' + temp + '}';
			// add attribute ENUM="{list}" if it's an enumeration
			if (datatype == 'E') {
				(*attributes)["ENUM"] = temp2;
			}
			// add attribute RANGE="{<lb>:<ub>}"
			else if (datatype == 'N') {
				(*attributes)["RANGE"] = temp2;
			}
		}
		// write USERDEF element
		name = "USERDEF";
		ok &= start_element(name, attributes);
		ok &= characters(value_);
		ok &= end_element(name);
		break;
	case AET_APP:
		// APP elements
		/* attributes
		PROGRAMID = field_name_ bewteen 1st and 2nd "_"
		FIELDNAME = field_name_ after 2nd "_"
		TYPE = theApp.GetReference()->GetDataTypeIndicator(field_name_)
		*/
		// parse the APP_<program>_<name>
		index1 = field_name_.find('_', 0);
		index2 = field_name_.find('_', index1 + 1);
		// add PROGRAM_ID=<id> attribute
		attributes = new map<string, string>;
		attributes->clear();
		(*attributes)["PROGRAMID"] = field_name_.substr(index1 + 1, index2 - index1 - 1);
		// add FIELDNAME=<name> attribute
		(*attributes)["FIELDNAME"] = field_name_.substr(index2 + 1);
		// add TYPE=<type> attribute
		datatype = spec_data_->datatype_indicator(field_name_);
		(*attributes)["TYPE"] = datatype;
		// write APP element
		name = "APP";
		ok &= start_element(name, attributes);
		ok &= characters(value_);
		ok &= end_element(name);
		break;
	case AET_RECORDS:
		// RECORDS element
		name = "RECORDS";
		ok &= start_element(name, nullptr);
		total = my_book_->get_count();
		// iterate through the records - abandon if error
		for (current_ = 0; current_ < (unsigned)total && ok; current_++) {
			record_ = my_book_->get_record(current_, false);
			// Write RECORD element
			ok &= write_element(AET_RECORD);
			if (clean_records_) {
				record_->clean();
			}
			// Update progress every record
			status_->progress(current_, my_book_->book_type());
		}
		ok &= end_element(name);
		break;
	case AET_RECORD:
		// RECORD element
		name = "RECORD";
		ok &= start_element(name, nullptr);
		for (auto it = record_->begin(); it != record_->end() && ok; it++) {
			// Iterate through the fields 
			field_name_ = it->first;
			// Get field name
			value_ = record_->item(field_name_);
			if (value_ != "") {
				// Only write field if it's not an empty string - USERDEF, APP or named field.
				if (spec_data_->is_userdef(field_name_)) {
					ok &= write_element(AET_USERDEFR);
				}
				else if (field_name_.length() > 3 && field_name_.substr(0,3) == "APP") {
					ok &= write_element(AET_APP);
				}
				else {
					ok &= write_element(AET_FIELD);
				}
			}
		}
		ok &= end_element(name);
		break;
	case AET_USERDEFR:
		// Record USERDEF element
		/* attributes:
		FIELDNAME = field_name_
		*/
		name = "USERDEF";
		// Add FIELDNAME=<name> attribute
		attributes = new map<string, string>;
		attributes->clear();
		(*attributes)["FIELDNAME"] = field_name_;
		// write element
		ok &= start_element(name, attributes);
		ok &= characters(value_);
		ok &= end_element(name);
		break;
	case AET_COMMENT:
		// Add a comment
		name = "";
		ok &= declaration(xml_element::COMMENT, name, value_);
	}
	if (!ok) {
		status_->misc_status(ST_ERROR, "LOG: Error occured while writing XML!");
	}
	return ok;
}

// Calculate progress towards saving file
double adx_writer::progress() {
	return (double)current_ / (double)my_book_->size();
}

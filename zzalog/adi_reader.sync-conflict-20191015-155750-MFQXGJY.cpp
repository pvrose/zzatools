#include "adi_reader.h"
#include "book.h"
#include "files.h"
#include "record.h"
#include "status.h"
#include "spec_data.h"
#include "pfx_data.h"

#include <istream>
#include <fstream>
#include <string>
#include <cstdio>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

using namespace zzalog;
using namespace std;

extern status* status_;
extern spec_data* spec_data_;
extern pfx_data* pfx_data_;
extern bool closing_;


// Helper class that reads and decodes an ADIF .adi format file and stores it a book container
adi_reader::adi_reader()
	: expecting_header_(false)
	, previous_count_(0)
	, byte_count_(0)
	, file_size_(0)
	, my_book_(nullptr)
{
}

// Default destructor
adi_reader::~adi_reader()
{
}

// Load Record.
// Data is read off the input stream (in) and stored as an array of ADIF items in the record.
istream& adi_reader::load_record(record* in_record, istream& in, load_result_t& result) {

	// Read the first character
	char c;
	in.get(c);

	// Initialise header 
	string header = "";
	bool eor = false;
	result = LR_GOOD;
	string bad_field = "";
	bool has_bad_field = false;

	// Only the header won't not start with a < character
	// Read upto the next < character or EOF
	while (c != '<' && in.good()) {
		// Capture all upto first < as header comment
		header += c;
		in.get(c);
	}

	// EOF => just read any characters after last record
	if (in.eof()) result = LR_EOF;
	else if (!in.good()) result = LR_BAD;

	// If this contains any character and we are looking for a header - treat this as a header 
	if (expecting_header_ == true && header.length() > 0) {
		in_record->header(header);
	}

	// Now turn off header checking
	expecting_header_ = false;

	// Until the end of record indicated by <EOR> or <EOH> - note in.good() is continually 
	// checked to prevent running off the end
	unsigned int num_fields = 0;
	while (in.good() && !eor) {
		// Each ADIF item is "<NAME:l[:T]>VALUE    " until <EOR> or <EOH>
		string field = "";
		string value = "";
		// Until : or > is read
		in.get(c);
		while (c != ':' && c != '>' && in.good()) {
			field += c;
			in.get(c);
		}
		if (!in.good()) result = LR_BAD;
		// convert field name to upper-case
		field = to_upper(field);
		// If field is <EOH> or <EOR> we have reached the end of the record
		if (field == "EOH" && in_record->is_header() || field == "EOR") {
			eor = true;
		}
		// Set read fail if we see EOH for a non-header record
		if (field == "EOH" && !in_record->is_header()) {
			status_->misc_status(ST_ERROR, "<EOH> found when not expecting it!");
			result = LR_BAD;
		}
		else {
			// Neither EOR nor EOH - therefore an ADIF item entry-  <NAME:l[:T]>VALUE 
			if (field != "EOH" && field != "EOR") {
				unsigned int count = 0;
				// Default datatype is String
				char type_indicator = ' ';
				// Get count the number of characters - first read the colon
				in.get(c);
				// Now read all numeric characters
				while (c >= '0' && c <= '9' && in.good()) {
					count = (count * 10) + c - '0';
					in.get(c);
				}
				// Read any colon
				while (c == ':' && in.good()) {
					in.get(c);
				}
				// save data type - if '>' read then there wasn't one. This doesn't check that there is only one character
				while (c != '>' && in.good()) {
					type_indicator = c;
					in.get(c);
				}
				// Read the >
				in.get(c);
				if (!in.good()) result = LR_BAD;
				// Read data item value
				for (unsigned int u = 0; u < count && in.good(); u++) {
					// Add CR if LF seen without previous CR - TODO: do we need to handle CR only?
					// adjust number of bytes read as count assumes CR/LF pairs.
					if (c == '\n' && (value.length() == 0 || value.back() != '\r')) {
						value += '\r';
						u += 1;
					}
					value += c;
					in.get(c);
				}
				if (!in.good()) result = LR_BAD;
				else {
					// Ignore all data until next < (or EOF) - it is likely to be white-space but ADIF says ignore anyway
					while (c != '<' && in.good()) {
						in.get(c);
						if (c != ' ' && c != '\t' && c != '\n' && c != 'r' && c != '<' && !in_record->is_header()) {
							// Report band item to the status log
							has_bad_field = true;
							bad_field = field;
						}
					}
					// Now create the hash-pair if length non-zero
					if (count > 0 && in.good()) {
						// Start assuming it's a valid field
						bool field_valid = true;
						bool external_app = false;
						// Add User defined fields to the reference data base if found in a header
						// <USERDEFn:sz:ty>name[,{list or range}]
						if (in_record->is_header() && field.length() > 7 && field.substr(0, 7) == "USERDEF") {
							string list_range = "";
							int pos_comma = value.find(',');
							if (pos_comma != -1) {
								list_range = value.substr(pos_comma + 2, value.length() - pos_comma - 3);
								value = value.substr(0, pos_comma);
							}
							int id_userdef = stoi(field.substr(7));
							field_valid = spec_data_->add_userdef(id_userdef, value, type_indicator, list_range);
						}
						// Add application defined field to the reference data base 
						// <APP_[PROG_ID]:sz[:ty]>value
						if (field.length() > 3 && field.substr(0, 3) == "APP") {
							// Convert any legacy APP_ZZALOG_... to APP_ZZA_.... Set modified to save the book at the end of load
							if (field.length() > 11 && field.substr(0, 11) == "APP_ZZALOG_") {
								string part2 = field.substr(11);
								field = "APP_ZZA_" + part2;
								my_book_->modified(true, false);
							}
							// Ignore all non-ZZALOG app specific fields (except APP_EQSL_SWL and any in the header.
							if (field.substr(0, 8) != "APP_ZZA_" || in_record->is_header()) {
								if (field == "APP_EQSL_SWL") {
									field = "SWL";
									my_book_->modified(true, false);
								}
								else {
									field_valid = false;
									external_app = true;
								}
							}
						}
						// If the data type is not valid then the field isn't
						if (field_valid && spec_data_->datatype(field).length() == 0) {
							field_valid = false;
						}
						// If the user or app. defined field is valid or it's an ADIF defined field
						if (field_valid) {
							// Get the expected datatype
							char data_type_indicator = spec_data_->datatype_indicator(field);
							// All enumerated values are treated as upper-case
							if (data_type_indicator == 'E') {
								value = to_upper(value);
							}
							// Add the item to the record
							in_record->item(field, value);
						}
						else {

							// Field ignored so set modified
							my_book_->modified(true, false);
							if (!external_app) {
								has_bad_field = true;
								bad_field = field;
							}
						}
					}
				}
			}
		}
	}
	if (has_bad_field) {
		// Report band item to the status log.
		char message[100];
		sprintf(message, "ADI READ: %s %s %s - Problem with field %s",
			in_record->item("QSO_DATE").c_str(),
			in_record->item("TIME_ON").c_str(),
			in_record->item("CALL").c_str(),
			bad_field.c_str());
		status_->misc_status(ST_ERROR, message);
	}
	return in;
}

// load data to book
// Data is read off the input stream (in) and records generated and added to book
bool adi_reader::load_book(book* book, istream& in) {
	// Start off eith good status
	load_result_t result = LR_GOOD;
	// Expect a header as the first record
	expecting_header_ = true;
	// Save book for load_record tp use if it updates the record
	my_book_ = book;
	// This will take a while so display the timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	file_size_ = (long)(endpos - startpos);
	status_->misc_status(ST_NOTE, "ADI READ: Started");
	status_->progress(file_size_, book->book_type(), "bytes");
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialise progress counters
	byte_count_ = 0;
	previous_count_ = 0;
	// While we have data to read
	while (in.good() && !closing_) {
		// Create a new record
		record* in_record = new record;
		// Read it from the input stream
		load_record(in_record, in, result);
		if (result == LR_GOOD) {
			if (in_record->is_header()) {
				// Store any header as the header record
				book->header(in_record);
			}
			else {
				// Otherwise add the record in its time-order position in the book
				book->insert_record(in_record);
			}
			// Check progress and update bar every record read
			byte_count_ = (long)in.tellg();
			status_->progress(byte_count_);
		}
		else {
			// Bad record
			delete in_record;
		}
	}
	// Restore normal cursor
	fl_cursor(FL_CURSOR_DEFAULT);
	// Update progress bar with complete or failed.
	if (in.fail() && !in.eof()) {
		status_->progress("Load failed");
		status_->misc_status(ST_ERROR, "ADI READ: Failed");
		return false;
	}
	else {
		status_->progress(file_size_);
		status_->misc_status(ST_OK, "ADI READ: Done!");
		return true;
	}
}

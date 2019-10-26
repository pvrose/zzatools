#include "adx_reader.h"
#include "../zzalib/utils.h"
#include "spec_data.h"
#include "pfx_data.h"
#include "status.h"
#include "files.h"

#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include <FL/Fl_Preferences.H>

using namespace zzalog;

extern spec_data* spec_data_;
extern pfx_data* pfx_data_;
extern Fl_Preferences* settings_;
extern status* status_;

// Constructor
adx_reader::adx_reader() :
	xml_reader(),
	in_(nullptr),
	num_comments_(0),
	num_ignored_(0),
	num_records_(0),
	my_book_(nullptr),
	field_name_(""),
	value_(""),
	datatype_(""),
	available_values_(""),
	record_(nullptr),
	line_num_(0),
	column_num_(0),
	modified_(false),
	file_size_(0),
	previous_count_(0)
{
	elements_.clear();
	userdef_fields_.clear();
}

// Destructor
adx_reader::~adx_reader()
{
	elements_.clear();
	userdef_fields_.clear();
}

// load data to book from the input stream
bool adx_reader::load_book(book* book, istream& in) {
	in_ = &in;
	load_result_t result = LR_GOOD;
	int count = 0;
	// Takes time so set the timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	my_book_ = book;
	// calculate the file size
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	file_size_ = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "ADX READ: Started");
	status_->progress(file_size_, book->book_type(), "bytes");
	// Call the XML parser
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "ADX READ: Done!");
		status_->progress(file_size_, book->book_type());
		fl_cursor(FL_CURSOR_DEFAULT);
		return true;
	}
	else {
		// Read failed - report failure
		status_->misc_status(ST_ERROR, "ADX READ: Failed");
		status_->progress("Load failed", book->book_type());
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}
}

// Overloaded XML handlers
// Start XML element <name [attributes]> 
bool adx_reader::start_element(string name, map<string, string>* attributes) {
	// Default to upper case for all text comparisons
	string element_name = to_upper(name);
	// decode OK
	bool error = false;
	// Initialise field and value data items
	field_name_ = "";
	value_ = "";

	// Start the specific element types
	if (element_name == "APP") {
		error |= start_app(attributes);
	}
	else if (element_name == "RECORD") {
		error |= start_record();
	}
	else if (element_name == "ADX") {
		error |= start_adx();
	}
	else if (element_name == "HEADER") {
		error |= start_header();
	}
	else if (element_name == "RECORDS") {
		error |= start_records();
	}
	else if (element_name == "USERDEF") {
		error |= start_userdef(attributes);
	}
	else {
		// Default must be a field element
		error |= start_field(element_name);
	}

	// Tidy up attributes as we no longer need them
	delete attributes;

	if (error) {
		// Start element reported an error - report it to user
		char message[128];
		sprintf(message, "XML parsing incorrect for element type %s", element_name.c_str());
		string temp = message;
		if (report_error(temp, true)) {
			return true;
		}
		else {
			return false;
		}
	}

	return false;
}

// End XML element detected
bool adx_reader::end_element(string name) {
	// Get the element name and convert it to upper case
	string element_name = to_upper(name);

	adx_element_t element_type = elements_.back();
	elements_.pop_back();

	bool ok = true;
	char error_msg[128];

	// </[Fieldname]
	if (element_name == field_name_) {
		// Check expect an end field element and we are processing a record
		if (element_type != AET_FIELD || record_ == nullptr) {
			sprintf(error_msg, "End of Field %s out of context.", field_name_.c_str());
			ok = false;
		}
		else {
			// Copy the ADIF item to the current record
			record_->item(field_name_, value_);
		}
	}
	else if (element_name == "APP") {
		// Check expected
		if (element_type != AET_APP || record_ == nullptr) {
			sprintf(error_msg, "End of App-specific Field %s out of context.", field_name_.c_str());
			ok = false;
		}
		else {
			if (!ignore_app_) {
				// Copy the ADIF item to the current record
				record_->item(field_name_, value_);
			}
		}
	}
	else if (element_name == "USERDEF" && element_type == AET_USERDEFH) {
		// USERDEF elment within a HEADER element
		// Add to the local list of user defined fields
		userdef_fields_.insert(value_);
		// Get the ID number - assume it is integer
		int id = stoi(field_name_.substr(7));
		// Add to the user defined fields list in ADIF database
		ok = spec_data_->add_userdef(id, value_, datatype_[0], available_values_);
		// Copy the ADIF item to the current record (header)
		record_->item(field_name_, value_);
	}
	else if (element_name == "USERDEF" && element_type == AET_USERDEFR && record_ != nullptr) {
		// USERDEF element within a RECORD element
		// Add item to the record
		record_->item(field_name_, value_);
	}
	else if (element_name == "USERDEF") {
		// Unexpecetd elsewhere
		sprintf(error_msg, "End of USERDEF %s found out of context.", field_name_.c_str());
		ok = false;
	}
	else if (element_name == "RECORD") {
		// Check expected
		if (element_type != AET_RECORD || record_ == nullptr) {
			sprintf(error_msg, "End of RECORD out of context.");
			ok = false;
		}
		else {
			if (record_->item("QSO_DATE") != "") {
				// Record has a QSO date - so has validity
				// Add the record to the book
				my_book_->insert_record(record_);
				num_records_++;
			}
			else {
				// report to user that no QSO_DATE
				sprintf(error_msg, "Record does not supply QSO_DATE. Continue will ignore record.");
				string temp = error_msg;
				if (report_error(temp, true)) {
					return true;
				}
			}
			// Update progress every record
			long current_count = (long)in_->tellg();
			status_->progress(current_count, my_book_->book_type());
		}
		// Clear the record - destroyed when book destroyed
		record_ = nullptr;
	}
	else if (element_name == "ADX") {
		// Check expected
		if (element_type != AET_ADX) {
			strcpy(error_msg, "End of ADX element out of context.");
			ok = false;
		}
		// Report if any XML constructs have been ignored
		if (num_ignored_ > 0) {
			char message[256];
			sprintf(message, "ADX READ: %d XML constructs have been ignored", num_ignored_);
			status_->misc_status(ST_WARNING, message);
		}
	}
	else if (element_name == "HEADER") {
		// Check expected
		if (element_type != AET_HEADER || record_ == nullptr) {
			strcpy(error_msg, "End of HEADER element out of context.");
			ok = false;
		}
		else {
			my_book_->header(record_);
			// Update progress every record
			long current_count = (long)in_->tellg();
			status_->progress(current_count, my_book_->book_type());
		}
		// Clear the record - destroyed when book destroyed
		record_ = nullptr;
	}
	else if (element_name == "RECORDS") {
		// Check expected
		if (element_type != AET_RECORDS) {
			strcpy(error_msg, "End of RECORDS element out of context.");
			ok = false;
		}
		// else nothing to do
	}
	if (!ok) {
		// Report error in end element
		string temp = error_msg;
		char message[100];
		sprintf(message, "ADX READ: %s %s %s - Problem with record in XML",
			record_->item("QSO_DATE").c_str(),
			record_->item("TIME_ON").c_str(),
			record_->item("CALL").c_str());
		status_->misc_status(ST_WARNING, message);
		if (report_error(temp, false)) {
			return false;
		}
		else {
			return true;
		}
	}
	return false;
}

// XML data characters received
bool adx_reader::characters(string content) {
	if (!elements_.empty()) {
		// We are processing elements - look at top of element stack
		switch (elements_.back()) {
		case AET_ADX:
		case AET_HEADER:
		case AET_RECORD:
		case AET_RECORDS:
			// TODO: review whether this is necessary as it takes 25% of CPU ADX READ
			//// Expect white space
			//if (!regex_match(content.c_str(), REGEX_WHITE_SPACE)) {
			//	char* temp = new char[40 + content.length()];
			//	sprintf(temp, "Data %s where white-space expected.", content.c_str());
			//	string error_msg = temp;
			//	if (!report_error(error_msg, TRUE)) {
			//		return false;
			//	}
			//}
			break;
		case AET_APP:
		case AET_FIELD:
		case AET_USERDEFR:
		case AET_USERDEFH:
			// Assume the XML processor is correct and converts all CR/LF pairs or singletons to LF
			// Convert to CR/LF for windows
			for (size_t i = 0; i < content.length(); i++) {
				if (content[i] == '\n') {
					value_ += "\r\n";
				}
				else {
					value_ += content[i];
				}
			}
			break;
		}
	}
	return TRUE;
}

// Explicitly ignore declaration tags - except comments
bool adx_reader::declaration(xml_element::element_t element_type, string name, string content) {
	switch (element_type) {
	case xml_element::COMMENT:
		// Add the content to the header comment if the comment is in the header
		if (record_ != nullptr && record_->is_header()) {
			string comment;
			if (record_->header().length()) {
				comment = record_->header() + '\n' + content;
			}
			else {
				comment = content;
			}
			record_->header(comment);
		}
		num_comments_++;
		break;
	default:
		num_ignored_++;
		break;
	}
	return true;
}

// Explicitly ignore processing instructions
bool adx_reader::processing_instr(string name, string content) {
	num_ignored_++;
	return true;
}

// start APP element - using supplied attributes
// <APP PROGRAMID="[id]" FIELDNAME="[fieldname]" TYPE="[datatype]">[data]</APP>
bool adx_reader::start_app(map<string, string>* attributes) {
	// Add element type to stack
	elements_.push_back(AET_APP);
	// Get the field name from the attributes
	string id;
	string field_name;
	bool error = false;
	// Now read the attributes
	for (auto it = attributes->begin(); it != attributes->end(); it++) 
	{
		// NAME="value"
		string attr_name = to_upper(it->first);
		string attr_value = it->second;
		// switch on name
		if (attr_name == "PROGRAMID") {
			// Program id.
			id = to_upper(attr_value);
			// Convert program ID ZZALOG to ZZA and mark we ghave modified the data
			if (id == "ZZALOG") {
				id = "ZZA";
				my_book_->modified(true, false);
			}
		}
		else if (attr_name == "FIELDNAME") {
			// app-defined field name
			field_name = to_upper(attr_value);
		}
		else if (attr_name == "TYPE") {
			// app-define field data type
			datatype_ = to_upper(attr_value);
		}
		else {
			// Not a valid attribute name for APP element
			error = true;
		}
		if (!error) {
			ignore_app_ = false;
			if (id == "EQSL" && field_name == "SWL") {
				field_name_ = "SWL";
			}
			else if (id == "ZZA") {
				// Generate field name APP_[Progid]_[Name]
				char temp[128];
				sprintf(temp, "APP_%s_%s", id.c_str(), field_name.c_str());
				field_name_ = temp;
			}
			else {
				ignore_app_ = true;
			}
		}
	}
	// Clear the value
	value_ = "";
	if (record_ == nullptr) {
		// Not currently processing a record - cannot have this element
		char temp[128];
		sprintf(temp, "Application specific field %s found out of context. ", field_name_.c_str());
		string message = temp;
		if (!report_error(message, true)) {
			return true;
		}
	}
	return false;
}

// Start RECORD element
// <RECORD> field elements </RECORD>
bool adx_reader::start_record() {
	if (record_ != nullptr) {
		// We don't allow nested RECORD elements
		string message = "RECORD element found out of context.";
		if (!report_error(message, true)) {
			return true;
		}
	}
	// Add to element stack 
	elements_.push_back(AET_RECORD);
	// Create a new record
	record_ = new record();
	return false;
}

// Start ADX element
// <ADX> header element, records element</ADX>
bool adx_reader::start_adx() {
	if (!elements_.empty()) {
		// This is the top element so the stack should be empty
		string message = "ADX element found out of context.";
		if (!report_error(message, true)) {
			return true;
		}
	}
	// Note we are processing <ADX>...</ADX>
	elements_.push_back(AET_ADX);
	// Empty the book - TODO: should we already have an empty book?
	my_book_->clear();
	return false;
}

// Start HEADER element
// <HEADER> field elements </HEADER>
bool adx_reader::start_header() {
	if (my_book_->header() != nullptr || elements_.empty() || elements_.back() != AET_ADX) {
		// We shouldn't have processed a HEADER, and top-of-stack should be ADX
		string message = "HEADER element found out of context.";
		if (!report_error(message, true)) {
			return true;
		}
	}
	// Create a header record
	record_ = new record;
	record_->header("");
	// Note we are processing <HEADER>...</HEADER>
	elements_.push_back(AET_HEADER);
	return false;
}

// Start RECORDS element
// <RECORDS> record elements </RECORD>
bool adx_reader::start_records() {
	if (elements_.empty() || elements_.back() != AET_ADX) {
		// Top-of-stack should be ADX
		string message = "RECORDS element found out of context.";
		if (!report_error(message, true)) {
			return true;
		}
	}
	// Note that we are processing <RECORDS>...<RECORDS>
	elements_.push_back(AET_RECORDS);
	return false;

}

// Start USERDEF element
// in header element
// <USERDEF FIELDID="n" TYPE="DATATYPEINDICATOR" ENUM="{A,B, ... N}" RANGE="{LOWERBOUND:UPPERBOUND}">FIELDNAME</USERDEF>
// in record element
// <USERDEF FIELDNAME="FIELDNAME">DATA</USERDEF>
bool adx_reader::start_userdef(map<string, string>* attributes) {
	if (elements_.empty() || (elements_.back() != AET_HEADER) && (elements_.back() != AET_RECORD)) {
		// In neither a header nor a record elemenmt
		string message = "USERDEF element found out of context.";
		if (!report_error(message, true)) {
			return true;
		}
	}
	else {
		if (elements_.back() == AET_HEADER) {
			// We are processing a USERDEF in a HEADER
			elements_.push_back(AET_USERDEFH);
			// For each attribute
			for (auto it = attributes->begin(); it != attributes->end(); it++) {
				// name and value
				string attr_name = to_upper(it->first);
				string attr_value = it->second;
				// switch on name
				if (attr_name == "FIELDID") {
					// we are USERDEFn
					field_name_ = "USERDEF" + attr_value;
				}
				if (attr_name == "TYPE") {
					// datatype
					datatype_ = to_upper(attr_value);
				}
				if (attr_name == "ENUM" || attr_name == "RANGE") {
					// either ENUM or RANGE
					available_values_ = to_upper(attr_value.substr(1, attr_value.length() - 2));
				}
				// All other attributes are ignored
			}
		}
		else if (elements_.back() == AET_RECORD) {
			// We are processing a USERDEF in a RECORD
			elements_.push_back(AET_USERDEFR);
			// For each attribute
			for (auto it = attributes->begin(); it != attributes->end(); it++) {
				// Name and value
				string attr_name = to_upper(it->first);
				string attr_value = it->second;

				if (attr_name == "FIELDNAME") {
					field_name_ = to_upper(attr_value);
					// Check we have seen an equivalent USERDEF in the header defining the fieldname
					if (userdef_fields_.find(field_name_) == userdef_fields_.end()) {
						char temp[125];
						sprintf(temp, "USERDEF %s not defined in header", field_name_.c_str());
						string message = temp;
						if (report_error(message, true)) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

// Start field elemenet 
// <FIELD-NAME>DATA</FIELD-NAME>
bool adx_reader::start_field(string field_name) {
	if (elements_.empty() || (elements_.back() != AET_HEADER && elements_.back() != AET_RECORD)) {
		// We must be processing a HEADER or RECORD element
		char temp[128];
		sprintf(temp, "Field element %s found out of context.", field_name.c_str());
		string message = temp;
		if (!report_error(message, true)) {
			return true;
		}
	}
	// We are in a field element
	elements_.push_back(AET_FIELD);
	// Capitalise field-name
	field_name_ = to_upper(field_name);
	value_ = "";
	return false;
}


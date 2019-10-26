#include "specx_reader.h"
#include "spec_data.h"
#include "status.h"
#include "../zzalib/utils.h"

using namespace zzalog;

extern status* status_;

// Constructor
specx_reader::specx_reader() :
	xml_reader()
	, data_(nullptr)
	, dataset_name_("")
	, dataset_enumeration_(false)
	, record_data_(nullptr)
	, element_data_("")
	, in_file_(nullptr)
	, num_ignored_(0)
{
	elements_.clear();
	column_headers_.clear();
}

// Destructor
specx_reader::~specx_reader()
{
	elements_.clear();
	if (record_data_ != nullptr) {
		record_data_->clear();
		delete record_data_;
	}
}

// Load the specification data from the suppled stream - return the ADIF version skimmed from data in version
bool specx_reader::load_data(spec_data* data, istream& in, string& version) {
	data_ = data;
	in_file_ = &in;
	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "SPEC READ: Started");
	status_->progress(file_size, OT_ADIF, "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "SPEC READ: Done!");
		status_->progress(file_size, OT_ADIF);
		version = adif_version_;
		data->erase("Primary_Administrative_Subdivision");
		data->erase("Secondary_Administrative_Subdivision");
		fl_cursor(FL_CURSOR_DEFAULT);
		return true;
	}
	else {
		// Read failed - report failure
		status_->misc_status(ST_FATAL, "SPEC READ: Failed");
		status_->progress("Load failed", OT_ADIF);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}
}

// Start an XML element
bool specx_reader::start_element(string name, map<string, string>* attributes) {
	string element_name = to_upper(name);
	bool result = true;

	// Check in order of the most comment element type - call appropriate start method
	if (element_name == "VALUE") result = start_value(attributes) | result;
	else if (element_name == "RECORD") result = start_record() | result;
	else if (element_name == "ENUMERATION") result = start_enumeration(attributes) | result;
	else if (element_name == "HEADER") result = start_header() | result;
	else if (element_name == "ADIF") result = start_adif(attributes) | result;
	else if (element_name == "DATATYPES") result = start_datatypes() | result;
	else if (element_name == "ENUMERATIONS") result = start_enumerations() | result;
	else if (element_name == "FIELDS") result = start_fields() | result;
	else {
		char* message = new char[50 + element_name.length()];
		sprintf(message, "SPEC_READ: Unexpected XML element %s encountered - ignored", element_name.c_str());
		status_->misc_status(ST_WARNING, message);
		delete[] message;
		num_ignored_++;
		result = false;
	}

	// Need to delete attributes
	if (attributes != nullptr) {
		attributes->clear();
		delete attributes;
	}
	return result;
}

// Special element
bool specx_reader::declaration(xml_element::element_t element_type, string name, string content) {
	// ignored
	num_ignored_++;
	return true;
}

// Processing instruction
bool specx_reader::processing_instr(string name, string content) {
	// ignored 
	num_ignored_++;
	return true;
}

// characters - save the element data
bool specx_reader::characters(string content) {
	if (elements_.size()) {
		switch (elements_.back()) {
		case SXE_VALUEH:
		case SXE_VALUER:
			element_data_ = content;
			break;
		default:
			break;
		}
	}
	return true;
}

// Start <adif vesrion="ADIF Version" status="Released" created="ISO Date">
bool specx_reader::start_adif(map<string, string>* attributes) {
	if (elements_.size()) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected adif element");
		return false;
	}
	else {
		elements_.push_back(SXE_ADIF);
		// Process the attributes
		for (auto it = attributes->begin(); it != attributes->end(); it++) {
			// Remember all the attributes
			string name = to_upper(it->first);
			if (name == "VERSION") {
				adif_version_ = it->second;
			}
			else if (name == "STATUS") {
				file_status_ = it->second;
			}
			else if (name == "CREATED") {
				created_ = it->second;
			}
			else {
				char* message = new char[50 + (it->second).length()];
				sprintf(message, "SPEC READ: Invalid attribute %s in ADIF element", it->second.c_str());
				status_->misc_status(ST_ERROR, message);
				delete[] message;
				return false;
			}
		}
	}
	return true;
}

// Start <dataTypes>
bool specx_reader::start_datatypes() {
	if (elements_.back() != SXE_ADIF) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected dataTypes element");
		return false;
	}
	else {
		elements_.push_back(SXE_DATATYPES);
		// Create a dataset "Data Types"
		dataset_name_ = "Data Types";
		char message[120];
		sprintf(message, "SPEC READ: %s", dataset_name_.c_str());
		status_->misc_status(ST_NOTE, message);
		return true;
	}
}

// Start header
bool specx_reader::start_header() {
	if (elements_.back() != SXE_DATATYPES && elements_.back() != SXE_ENUM && elements_.back() != SXE_FIELDS) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected header element");
		return false;
	}
	else {
		elements_.push_back(SXE_HEADER);
		column_headers_.clear();
		// No action
		return true;
	}
}

// Start header value
bool specx_reader::start_value(map<string, string>* attributes) {
	switch (elements_.back()) {
	case SXE_HEADER:
		elements_.push_back(SXE_VALUEH);
		// No action
		return true;
	case SXE_RECORD:
		elements_.push_back(SXE_VALUER);
		for (auto it = attributes->begin(); it != attributes->end(); it++) {
			string att_name = to_upper(it->first);
			if (att_name == "NAME") {
				// Remember item name
				item_name_ = it->second;
			}
			else {
				char* message = new char[50 + it->second.length()];
				sprintf(message, "SPEC READ: Incorrect XML - unknown value attribute %s", it->second.c_str());
				status_->misc_status(ST_ERROR, message);
				delete[] message;
				return false;
			}
		}
		return true;
	default:
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected value element");
		return false;
	}
}

// Start record
bool specx_reader::start_record() {
	if (elements_.back() != SXE_DATATYPES && elements_.back() != SXE_ENUM && elements_.back() != SXE_FIELDS) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected record element");
		return false;
	}
	else {
		elements_.push_back(SXE_RECORD);
		// Create a new set of record items
		record_data_ = new map<string, string>;
		return true;
	}
}

// Start enumerations
bool specx_reader::start_enumerations() {
	if (elements_.back() != SXE_ADIF) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected enumerations element");
		return false;
	}
	else {
		elements_.push_back(SXE_ENUMS);
		dataset_enumeration_ = true;
		// Do nothing
		return true;
	}
}

// Start enumeration
bool specx_reader::start_enumeration(map<string, string>* attributes) {
	if (elements_.back() != SXE_ENUMS) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected dataTypes element");
		return false;
	}
	else {
		elements_.push_back(SXE_ENUM);
		column_headers_.clear();
		for (auto it = attributes->begin(); it != attributes->end(); it++) {
			string att_name = to_upper(it->first);
			if (att_name == "NAME") {
				dataset_name_ = it->second;
				char message[120];
				sprintf(message, "SPEC READ: %s", dataset_name_.c_str());
				status_->misc_status(ST_NOTE, message);
			}
			else {
				char* message = new char[50 + it->second.length()];
				sprintf(message, "SPEC READ: Incorrect XML - unknown value attribute %s", it->second.c_str());
				status_->misc_status(ST_ERROR, message);
				delete[] message;
				return false;
			}
		}
		return true;
	}
}

// Start fields
bool specx_reader::start_fields() {
	if (elements_.back() != SXE_ADIF) {
		status_->misc_status(ST_ERROR, "SPEC READ: Incorrect XML - unexpected fields element");
		return false;
	}
	else {
		elements_.push_back(SXE_FIELDS);
		dataset_name_ = "Fields";
		char message[120];
		sprintf(message, "SPEC READ: %s", dataset_name_.c_str());
		status_->misc_status(ST_NOTE, message);
		return true;
	}
}

// End element - call the appropriate handler for the element type
bool specx_reader::end_element(string name) {
	string element_name = to_upper(name);
	specx_element_t element = elements_.back();
	elements_.pop_back();
	// Go to the specific end_... method
	switch (element) {
	case SXE_ADIF:
		if (element_name == "ADIF") return end_adif();
		break;
	case SXE_DATATYPES:
		if (element_name == "DATATYPES") return end_datatypes();
		break;
	case SXE_HEADER:
		if (element_name == "HEADER") return end_header();
		break;
	case SXE_VALUEH:
		if (element_name == "VALUE") return end_header_value();
		break;
	case SXE_RECORD:
		if (element_name == "RECORD") return end_record();
		break;
	case SXE_VALUER:
		if (element_name == "VALUE") return end_record_value();
		break;
	case SXE_ENUMS:
		if (element_name == "ENUMERATIONS") return end_enumerations();
		break;
	case SXE_ENUM:
		if (element_name == "ENUMERATION") return end_enumeration();
		break;
	case SXE_FIELDS:
		if (element_name == "FIELDS") return end_fields();
		break;
	}
	char* message = new char[50 + name.length()];
	sprintf(message, "SPEC DATA: Invalid XML - mismatch start and end of element %s", name.c_str());
	status_->misc_status(ST_ERROR, message);
	delete[] message;
	return false;
}

// end element for ADIF
bool specx_reader::end_adif() {
	// End of the XML - no action
	if (num_ignored_) {
		char message[128];
		sprintf(message, "SPEC READ: %d XML elements ignored", num_ignored_);
		status_->misc_status(ST_WARNING, message);
	}
	return true;
}

// End element for datatypes
bool specx_reader::end_datatypes() {
	dataset_name_ = "";
	column_headers_.clear();
	return true;
}

// End element for header
bool specx_reader::end_header() {
	// Do nothing
	return true;
}

// End element for a value within header
bool specx_reader::end_header_value() {
		// Add the column name - in order of the header value elements
	column_headers_.push_back(element_data_);
	return true;
}

// End record element
bool specx_reader::end_record() {
	// Add the record to the dataset
	// For PAS and SAS need to check DXCC entity code is the same - if not create a new dataset
	if (dataset_name_.length() >= 34 && dataset_name_.substr(0, 34) == "Primary_Administrative_Subdivision") {
		string this_code = record_data_->at("DXCC Entity Code");
		dataset_name_ = "Primary_Administrative_Subdivision[" + this_code + "]";
	}
	else if (dataset_name_.length() >= 36 && dataset_name_.substr(0, 36) == "Secondary_Administrative_Subdivision") {
		string this_code = record_data_->at("DXCC Entity Code");
		dataset_name_ = "Secondary_Administrative_Subdivision[" + this_code + "]";
	}
	spec_dataset* dataset = (*data_)[dataset_name_];
	if (dataset == nullptr) {
		dataset = new spec_dataset;
		dataset->column_names = column_headers_;
		(*data_)[dataset_name_] = dataset;
	}
		// If the record has been deleted
	if (record_data_->find("Deleted") != record_data_->end() && record_data_->at("Deleted") == "true") {
		record_name_ += " Deleted";
	}
	dataset->data[record_name_] = record_data_;
	record_data_ = nullptr;
	int byte_count = (int)in_file_->tellg();
	status_->progress(byte_count, OT_ADIF);
	return true;
}

// Reached the end of the record item
bool specx_reader::end_record_value() {
	if (record_data_ == nullptr) {
		status_->misc_status(ST_FATAL, "SPEC READ: Programming error");
		return false;
	}
	else {
		if (dataset_enumeration_) {
			if (item_name_ == column_headers_[1]) {
				// Save the record name (in upper case)
				record_name_ = to_upper(element_data_);
				return true;
			}
			else if (item_name_ == column_headers_[0]) {
				// ignore
				return true;
			}
			else {
				// Save the data item
				(*record_data_)[item_name_] = element_data_;
				return true;
			}
		}
		else {
			if (item_name_ == column_headers_[0]) {
				// Save therecord name (as-is)
				record_name_ = element_data_;
				return true;
			}
			else {
				// Save the data item
				(*record_data_)[item_name_] = element_data_;
				return true;
			}
		}
	}
}

// End of enumerations
bool specx_reader::end_enumerations() {
	dataset_enumeration_ = false;
	return true;
}

// End of enumeration
bool specx_reader::end_enumeration() {
	column_headers_.clear();
	dataset_name_ = "";
	return true;
}

// End of fields
bool specx_reader::end_fields() {
	column_headers_.clear();
	dataset_name_ = "";
	return true;
}

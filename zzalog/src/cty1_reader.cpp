#include "cty1_reader.h"
#include "status.h"
#include "cty_data.h"

#include <list>



extern status* status_;
extern bool closing_;

// Constructor
cty1_reader::cty1_reader() {
	ignore_processing_ = false;
	current_entity_ = nullptr;
	current_prefix_ = nullptr;
	//current_exception_ = nullptr;
	//current_invalid_ = nullptr;
	//current_zone_exc_ = nullptr;
	//current_entity_ = nullptr;
	//current_prefix_ = nullptr;
	data_ = nullptr;
	file_ = nullptr;
}

// Destructor
cty1_reader::~cty1_reader() {
}

// Overloadable XML handlers
// Start element
bool cty1_reader::start_element(string name, map<string, string>* attributes) {
	if (!ignore_processing_) {
		if (elements_.size()) {
			string enclosure = elements_.back();
			elements_.push_back(name);
			if (enclosure == "entities" && name == "entity") {
				current_entity_ = new cty_entity;
			}
			else if (enclosure == "prefixes" && name == "prefix") {
				current_prefix_ = new cty_prefix;
			}
			else if (enclosure == "exceptions" && name == "exception") {
				current_exception_ = new cty_exception;
				current_exception_->exc_type_ = cty_exception::EXC_OVERRIDE;
			}
			else if (enclosure == "invalid_operations" && name == "invalid") {
				current_exception_ = new cty_exception;
				current_exception_->exc_type_ = cty_exception::EXC_INVALID;
				current_exception_->dxcc_id_ = -1;
			}
			else if (enclosure == "zone_exceptions" && name == "zone_exception") {
				current_exception_ = new cty_exception;
				current_exception_->exc_type_ = cty_exception::EXC_OVERRIDE;
			}
		}
		else {
			elements_.push_back(name);
			if (name == "clublog") {
				if (attributes && attributes->find("date") != attributes->end()) {
					timestamp_ = xmldt2date(attributes->at("date"));
				}
			}
		}
	}
	return true;
}

// End element
bool cty1_reader::end_element(string name) {
	if (ignore_processing_) {
		// We stop ignoring
		if (name == elements_.back()) {
			ignore_processing_ = false;
			elements_.pop_back();
		}
	}
	else {
		// Capture the exception or invalid record. Move the record to the database
		string element = elements_.back();
		elements_.pop_back();
		string enclosure = elements_.size() ? elements_.back() : "";
		if ((enclosure == "exceptions" && element == "exception") ||
			(enclosure == "zone_exceptions" && element == "zone_exception") ) {
			// Add it to the list, which if necessay create
			data_->add_exception(current_match_, current_exception_);
			current_exception_ = nullptr;
			current_match_ = "";
		}
		else if ((enclosure == "invalid_operations" && element == "invalid")) {
			// Add it to the list, which if necessay create
			data_->add_exception(current_match_, current_exception_);
			current_exception_ = nullptr;
			current_match_ = "";
		}
		else if ((enclosure == "prefixes" && element == "prefix")) {
			// Add it to the list, which if necessay create
			data_->add_prefix(current_match_, current_prefix_);
			current_prefix_ = nullptr;
			current_match_ = "";
		}
		else if (enclosure == "entities" && element == "entity") {
			data_->add_entity(current_entity_);
			current_entity_ = nullptr;
		}
		else if (elements_.size() && elements_.back() == "entity") {
			// Build up the exception record from the various child elements
			if (element == "adif") current_entity_->dxcc_id_ = stoi(value_);
			else if (element == "name") current_entity_->name_ = value_;
			else if (element == "prefix") current_entity_->nickname_ = value_;
			else if (element == "cqz") current_entity_->cq_zone_ = stoi(value_);
			else if (element == "cont") current_entity_->continent_ = value_;
			else if (element == "long") current_entity_->coordinates_.longitude = stod(value_);
			else if (element == "lat") current_entity_->coordinates_.latitude = stod(value_);
			else if (element == "deleted" && value_ == "true") current_entity_->deleted_ = true;
			else if (element == "start") {
				current_entity_->time_validity_.start = xmldt2date(value_);
			}
			else if (element == "end") {
				current_entity_->time_validity_.finish = xmldt2date(value_);
			}
		}
		else if (elements_.size() && elements_.back() == "prefix") {
			// Build up the exception record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "adif") current_prefix_->dxcc_id_ = stoi(value_);
			else if (element == "entity") current_prefix_->name_ = value_;
			else if (element == "cqz") current_prefix_->cq_zone_ = stoi(value_);
			else if (element == "cont") current_prefix_->continent_ = value_;
			else if (element == "long") current_prefix_->coordinates_.longitude = stod(value_);
			else if (element == "lat") current_prefix_->coordinates_.latitude = stod(value_);
			else if (element == "start") {
				current_prefix_->time_validity_.start = xmldt2date(value_);
			}
			else if (element == "end") {
				current_prefix_->time_validity_.finish = xmldt2date(value_);
			}
		}
		else if (elements_.size() && elements_.back() == "exception") {
			// Build up the exception record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "adif") current_exception_->dxcc_id_ = stoi(value_);
			else if (element == "entity") current_exception_->name_ = value_;
			else if (element == "cqz") current_exception_->cq_zone_ = stoi(value_);
			else if (element == "cont") current_exception_->continent_ = value_;
			else if (element == "long") current_exception_->coordinates_.longitude = stod(value_);
			else if (element == "lat") current_exception_->coordinates_.latitude = stod(value_);
			else if (element == "start") {
				current_exception_->time_validity_.start = xmldt2date(value_);
			}
			else if (element == "end") {
				current_exception_->time_validity_.finish = xmldt2date(value_);
			}
		}
		else if (elements_.size() && elements_.back() == "invalid") {
			// Build up the invalid record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "start") {
				current_exception_->time_validity_.start = xmldt2date(value_);
			}
			else if (element == "end") {
				current_exception_->time_validity_.finish = xmldt2date(value_);
			}
		}
		else if (elements_.size() && elements_.back() == "zone_exception") {
			// Build up the exception record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "cqz") current_exception_->cq_zone_ = stoi(value_);
			else if (element == "start") {
				current_exception_->time_validity_.start = xmldt2date(value_);
			}
			else if (element == "end") {
				current_exception_->time_validity_.finish = xmldt2date(value_);
			}
		}
	}
	number_read_++;
	if (number_read_ % 1000 == 0 || elements_.size() <= 1) {
		// Report progress every 1000 elements
		int bytes = (int)file_->tellg();
		status_->progress(bytes, OT_PREFIX);
	}
	return true;
}

// Special element - not expected
bool cty1_reader::declaration(xml_element::element_t element_type, string name, string content) {
	return false;
}

// Processing instruction - ignored
bool cty1_reader::process_instr(string name, string content) {
	return false;
}

// characters - set the element value
bool cty1_reader::characters(string content) {
	value_ = content;
	return true;
}

// Load data from specified file into and add each record to the map
bool cty1_reader::load_data(cty_data* data, istream& in, string& version) {
	fl_cursor(FL_CURSOR_WAIT);
	data_ = data;
	file_ = &in;
	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "CTY DATA: Started importing data");
	status_->progress(file_size, OT_PREFIX, "Importing country data from clublog.org", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "CTY DATA: Import done!");
		version = timestamp_;
		fl_cursor(FL_CURSOR_DEFAULT);
		return true;
	}
	else if (closing_) {
		status_->misc_status(ST_WARNING, "CTY DATA: Cancelled as close-down requested");
		status_->progress("Load cancelled", OT_PREFIX);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}
	else {
		// Read failed - report failure
		status_->misc_status(ST_ERROR, "CTY DATA: Import failed");
		status_->progress("Load failed", OT_PREFIX);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}

}

// Get date in format %Y%m%d from XML date time value.
string cty1_reader::xmldt2date(string xml_date) {
	string result = xml_date.substr(0, 4) + xml_date.substr(5, 2) + xml_date.substr(8, 2) +
		xml_date.substr(11, 2) + xml_date.substr(14, 2);
	return result;
}
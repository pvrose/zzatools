#include "exc_reader.h"
#include "status.h"
#include "exc_data.h"

#include <list>

using namespace zzalog;

extern status* status_;
extern bool closing_;

/* XML structure

<clublog>
:
:
<exceptions>
	<exception record='6002'>
		<call>VE1ST/NA14</call>
		<entity>CANADA</entity>
		<adif>1</adif>
		<cqz>4</cqz>
		<cont>NA</cont>
		<long>-97.14</long>
		<lat>49.90</lat>
	</exception>
	:
	:
</exceptions>
:
:
<invalid_operations>
	<invalid record='489'>
		<call>T88A</call>
		<start>1995-05-01T00:00:00+00:00</start>
		<end>1995-12-31T23:59:59+00:00</end>
	</invalid>
	:
	:
</invalid_operations>
:
:
</clublog>


*/

// Constructor
exc_reader::exc_reader() {
	ignore_processing_ = false;
	current_exception_ = nullptr;
	current_invalid_ = nullptr;
}

// Destructor
exc_reader::~exc_reader() {
}

// Overloadable XML handlers
// Start element
bool exc_reader::start_element(string name, map<string, string>* attributes) {
	if (!ignore_processing_) {
		elements_.push_back(name);
		if (name == "entities" || name == "prefixes" || name == "zone_exceptions") {
			// We are only interested in exception and invalid records
			ignore_processing_ = true;
		}
		else if (name == "exception") {
			current_exception_ = new exc_entry;
		}
		else if (name == "invalid") {
			current_invalid_ = new invalid;
		}
	}
	return true;
}

// End element
bool exc_reader::end_element(string name) {
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
		if (element == "exception") {
			// The records are stored as list of records mapped from the callsign
			if (data_->entries_.find(current_exception_->call) == data_->entries_.end()) {
				list<exc_entry*>* entries = new list<exc_entry*>;
				(data_->entries_)[current_exception_->call] = *entries;
			}
			data_->entries_.at(current_exception_->call).push_back(current_exception_);
			current_exception_ = nullptr;
		}
		else if (element == "invalid") {
			// The records are stored as list of records mapped from the callsign
			if (data_->invalids_.find(current_invalid_->call) == data_->invalids_.end()) {
				list<invalid*>* invalids = new list<invalid*>;
				(data_->invalids_)[current_invalid_->call] = *invalids;
			}
			data_->invalids_.at(current_invalid_->call).push_back(current_invalid_);
			current_invalid_ = nullptr;
		}
		else if (elements_.size() && elements_.back() == "exception") {
			// Build up the exception record from the various child elements
			if (element == "call") current_exception_->call = value_;
			else if (element == "adif") current_exception_->adif_id = stoi(value_);
			else if (element == "cqz") current_exception_->cq_zone = stoi(value_);
			else if (element == "cont") current_exception_->continent = value_;
			else if (element == "long") current_exception_->longitude = stod(value_);
			else if (element == "lat") current_exception_->latitude = stod(value_);
			else if (element == "start") current_exception_->start = convert_xml_datetime(value_);
			else if (element == "end") current_exception_->end = convert_xml_datetime(value_);
		}
		else if (elements_.size() && elements_.back() == "invalid") {
			// Build up the invalid record from the various child elements
			if (element == "call") current_invalid_->call = value_;
			else if (element == "start") current_invalid_->start = convert_xml_datetime(value_);
			else if (element == "end") current_invalid_->end = convert_xml_datetime(value_);
		}
	}
	if (elements_.size() <= 3) {
		// Report progress only when the depth of elements is less than three.
		int bytes = (int)file_->tellg();
		status_->progress(bytes, OT_PREFIX);
	}
	return true;
}

// Special element - not expected
bool exc_reader::declaration(xml_element::element_t element_type, string name, string content) {
	return false;
}

// Processing instruction - ignored
bool exc_reader::processing_instr(string name, string content) {
	return false;
}

// characters - set the element value
bool exc_reader::characters(string content) {
	value_ = content;
	return true;
}

// Load data from specified file into and add each record to the map
bool exc_reader::load_data(exc_data* data, istream& in, string& version) {
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
	status_->misc_status(ST_NOTE, "EXCEPTION: Started");
	status_->progress(file_size, OT_PREFIX, "Extracting Extraction data from XML", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "EXCEPTION: Done!");
		status_->progress(file_size, OT_PREFIX);
		version = timestamp_;
		fl_cursor(FL_CURSOR_DEFAULT);
		return true;
	}
	else if (closing_) {
		status_->misc_status(ST_WARNING, "EXCEPTION: Cancelled as close-down requested");
		status_->progress("Load cancelled", OT_PREFIX);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}
	else {
		// Read failed - report failure
		status_->misc_status(ST_FATAL, "EXCEPTION: Failed");
		status_->progress("Load failed", OT_PREFIX);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}

}

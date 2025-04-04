#include "cty_reader.h"
#include "status.h"
#include "cty_data.h"

#include <list>



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
<zone_exceptions>
	<zone_exception record='59'>
		<call>KD6WW/VY0</call>
		<zone>1</zone>
		<start>2003-07-30T00:00:00+00:00</start>
		<end>2003-07-31T23:59:59+00:00</end>
	</zone_exception>
	:
	:
</zone_exceptions>
:
:
</clublog>


*/

// Constructor
cty_reader::cty_reader() {
	ignore_processing_ = false;
	current_exception_ = nullptr;
	current_invalid_ = nullptr;
	current_zone_exc_ = nullptr;
	current_entity_ = nullptr;
	current_prefix_ = nullptr;
	data_ = nullptr;
	file_ = nullptr;
}

// Destructor
cty_reader::~cty_reader() {
}

// Overloadable XML handlers
// Start element
bool cty_reader::start_element(string name, map<string, string>* attributes) {
	if (!ignore_processing_) {
		if (elements_.size()) {
			string enclosure = elements_.back();
			elements_.push_back(name);
			if (enclosure == "entities" && name == "entity") {
				current_entity_ = new cty_data::entity_entry;
			}
			else if (enclosure == "prefixes" && name == "prefix") {
				current_prefix_ = new cty_data::prefix_entry;
			}
			else if (enclosure == "exceptions" && name == "exception") {
				current_exception_ = new cty_data::exc_entry;
			}
			else if (enclosure == "invalid_operations" && name == "invalid") {
				current_invalid_ = new cty_data::invalid_entry;
			}
			else if (enclosure == "zone_exceptions" && name == "zone_exception") {
				current_zone_exc_ = new cty_data::zone_entry;
			}
		}
		else {
			elements_.push_back(name);
		}
	}
	return true;
}

// End element
bool cty_reader::end_element(string name) {
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
		if (enclosure == "exceptions" && element == "exception") {
			// The records are stored as list of records mapped from the callsign
			if (data_->entries_.find(current_exception_->call) == data_->entries_.end()) {
				list<cty_data::exc_entry*>* entries = new list<cty_data::exc_entry*>;
				(data_->entries_)[current_exception_->call] = *entries;
			}
			data_->entries_.at(current_exception_->call).push_back(current_exception_);
			current_exception_ = nullptr;
		}
		else if (enclosure == "invalid_operations" && element == "invalid") {
			// The records are stored as list of records mapped from the callsign
			if (data_->invalids_.find(current_invalid_->call) == data_->invalids_.end()) {
				list<cty_data::invalid_entry*>* invalids = new list<cty_data::invalid_entry*>;
				(data_->invalids_)[current_invalid_->call] = *invalids;
			}
			data_->invalids_.at(current_invalid_->call).push_back(current_invalid_);
			current_invalid_ = nullptr;
		}
		else if (enclosure == "zone_exceptions" && element == "zone_exception") {
			// The records are stored as list of records mapped from the callsign
			if (data_->zones_.find(current_zone_exc_->call) == data_->zones_.end()) {
				list<cty_data::zone_entry*>* zones = new list<cty_data::zone_entry*>;
				(data_->zones_)[current_zone_exc_->call] = *zones;
			}
			data_->zones_.at(current_zone_exc_->call).push_back(current_zone_exc_);
			current_zone_exc_ = nullptr;
		}
		else if (enclosure == "entities" && element == "entity") {
			// The records are stored as list of records mapped from the callsign
			if (data_->entities_.find(current_entity_->adif_id) == data_->entities_.end()) {
				cty_data::entity_entry* entity = new cty_data::entity_entry;
				(data_->entities_)[current_entity_->adif_id] = entity;
			}
			data_->entities_[current_entity_->adif_id] = current_entity_;
			current_entity_ = nullptr;
		}
		else if (enclosure == "prefixes" && element == "prefix") {
			// The records are stored as list of records mapped from the callsign
			if (data_->prefixes_.find(current_prefix_->call) == data_->prefixes_.end()) {
				list<cty_data::prefix_entry*>* prefixes = new list<cty_data::prefix_entry*>;
				(data_->prefixes_)[current_prefix_->call] = *prefixes;
			}
			data_->prefixes_.at(current_prefix_->call).push_back(current_prefix_);
			current_prefix_ = nullptr;
		}
		else if (elements_.size() && elements_.back() == "entity") {
			// Build up the exception record from the various child elements
			if (element == "adif") current_entity_->adif_id = stoi(value_);
			else if (element == "name") current_entity_->name = value_;
			else if (element == "prefix") current_entity_->prefix = value_;
			else if (element == "cqz") current_entity_->cq_zone = stoi(value_);
			else if (element == "cont") current_entity_->continent = value_;
			else if (element == "long") current_entity_->longitude = stod(value_);
			else if (element == "lat") current_entity_->latitude = stod(value_);
			else if (element == "start") current_entity_->start = convert_xml_datetime(value_);
			else if (element == "end") current_entity_->end = convert_xml_datetime(value_);
			else if (element == "deleted" && value_ == "TRUE") current_entity_->deleted = true;
			else if (element == "whitelist" && value_ == "TRUE") current_entity_->whitelist = true;
			else if (element == "whitelist_start") current_entity_->whitelist_start =
				convert_xml_datetime(value_);
		}
		else if (elements_.size() && elements_.back() == "prefix") {
			// Build up the exception record from the various child elements
			if (element == "call") current_prefix_->call = value_;
			else if (element == "entity") current_prefix_->entity = value_;
			else if (element == "adif") current_prefix_->adif_id = stoi(value_);
			else if (element == "cqz") current_prefix_->cq_zone = stoi(value_);
			else if (element == "cont") current_prefix_->continent = value_;
			else if (element == "long") current_prefix_->longitude = stod(value_);
			else if (element == "lat") current_prefix_->latitude = stod(value_);
			else if (element == "start") current_prefix_->start = convert_xml_datetime(value_);
			else if (element == "end") current_prefix_->end = convert_xml_datetime(value_);
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
		else if (elements_.size() && elements_.back() == "zone_exception") {
			// Build up the exception record from the various child elements
			if (element == "call") current_zone_exc_->call = value_;
			else if (element == "cqz") current_zone_exc_->cq_zone = stoi(value_);
			else if (element == "start") current_zone_exc_->start = convert_xml_datetime(value_);
			else if (element == "end") current_zone_exc_->end = convert_xml_datetime(value_);
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
bool cty_reader::declaration(xml_element::element_t element_type, string name, string content) {
	return false;
}

// Processing instruction - ignored
bool cty_reader::processing_instr(string name, string content) {
	return false;
}

// characters - set the element value
bool cty_reader::characters(string content) {
	value_ = content;
	return true;
}

// Load data from specified file into and add each record to the map
bool cty_reader::load_data(cty_data* data, istream& in, string& version) {
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
	status_->misc_status(ST_NOTE, "EXCEPTION: Started extracting data");
	status_->progress(file_size, OT_PREFIX, "Extracting Exception data from XML", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "EXCEPTION: Extraction done!");
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
		status_->misc_status(ST_ERROR, "EXCEPTION: Extraction failed");
		status_->progress("Load failed", OT_PREFIX);
		fl_cursor(FL_CURSOR_DEFAULT);
		return false;
	}

}

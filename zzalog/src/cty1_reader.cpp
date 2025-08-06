#include "cty1_reader.h"
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
cty1_reader::cty1_reader() {
	ignore_processing_ = false;
	current_entity_ = nullptr;
	current_pattern_ = nullptr;
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
				current_entity_ = new cty_data::ent_entry;
			}
			else if (enclosure == "prefixes" && name == "prefix") {
				current_pattern_ = new cty_data::patt_entry;
				current_pattern_->type = 0;
			}
			else if (enclosure == "exceptions" && name == "exception") {
				current_pattern_ = new cty_data::patt_entry;
				current_pattern_->type = cty_data::DXCC_EXCEPTION;
			}
			else if (enclosure == "invalid_operations" && name == "invalid") {
				current_pattern_ = new cty_data::patt_entry;
				current_pattern_->type = cty_data::INVALID_CALL;
				current_pattern_->dxcc_id = -1;
			}
			else if (enclosure == "zone_exceptions" && name == "zone_exception") {
				current_pattern_ = new cty_data::patt_entry;
				current_pattern_->type = cty_data::CQZ_EXCEPTION;
			}
		}
		else {
			elements_.push_back(name);
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
			(enclosure == "invalid_operations" && element == "invalid") ||
			(enclosure == "zone_exceptions" && element == "zone_exception") ||
			(enclosure == "prefixes" && element == "prefix")) {
			// Add it to the list, which if necessay create
			if (data_->patterns.find(current_match_) == data_->patterns.end()) {
				data_->patterns[current_match_] = { current_pattern_ };
			}
			else {
				data_->patterns[current_match_].push_back(current_pattern_);
			}
			current_pattern_ = nullptr;
			current_match_ = "";
		}
		else if (enclosure == "entities" && element == "entity") {
			// Add it to the list, which if necessay create
			if (data_->entities.find(current_entity_->dxcc_id) == data_->entities.end()) {
				data_->entities[current_entity_->dxcc_id] = current_entity_;
			}
			else {
				char msg[128];
				snprintf(msg, sizeof(msg), "CTY DATA: Altready have record for entity %d - %s",
					current_entity_->dxcc_id, current_entity_->name.c_str());
				status_->misc_status(ST_ERROR, msg);
			}
			current_entity_ = nullptr;
		}
		else if (elements_.size() && elements_.back() == "entity") {
			// Build up the exception record from the various child elements
			if (element == "adif") current_entity_->dxcc_id = stoi(value_);
			else if (element == "name") current_entity_->name = value_;
			else if (element == "prefix") current_entity_->nickname = value_;
			else if (element == "cqz") current_entity_->cq_zone = stoi(value_);
			else if (element == "cont") current_entity_->continent = value_;
			else if (element == "long") current_entity_->location.longitude = stod(value_);
			else if (element == "lat") current_entity_->location.latitude = stod(value_);
			else if (element == "deleted" && value_ == "true") current_entity_->deleted = true;
			else if (element == "start") {
				current_entity_->validity.start = convert_xml_datetime(value_);
				current_entity_->has_validity = true;
			}
			else if (element == "end") {
				current_entity_->validity.end = convert_xml_datetime(value_);
				current_entity_->has_validity = true;
			}
		}
		else if (elements_.size() && elements_.back() == "prefix") {
			// Build up the exception record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "adif") current_pattern_->dxcc_id = stoi(value_);
			else if (element == "cqz") current_pattern_->cq_zone = stoi(value_);
			else if (element == "cont") current_pattern_->continent = value_;
			else if (element == "long") current_pattern_->location.longitude = stod(value_);
			else if (element == "lat") current_pattern_->location.latitude = stod(value_);
			else if (element == "start") {
				current_pattern_->validity.start = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
			else if (element == "end") {
				current_pattern_->validity.end = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
		}
		else if (elements_.size() && elements_.back() == "exception") {
			// Build up the exception record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "adif") current_pattern_->dxcc_id = stoi(value_);
			else if (element == "cqz") current_pattern_->cq_zone = stoi(value_);
			else if (element == "cont") current_pattern_->continent = value_;
			else if (element == "long") current_pattern_->location.longitude = stod(value_);
			else if (element == "lat") current_pattern_->location.latitude = stod(value_);
			else if (element == "start") {
				current_pattern_->validity.start = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
			else if (element == "end") {
				current_pattern_->validity.end = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
		}
		else if (elements_.size() && elements_.back() == "invalid") {
			// Build up the invalid record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "start") {
				current_pattern_->validity.start = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
			else if (element == "end") {
				current_pattern_->validity.end = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
		}
		else if (elements_.size() && elements_.back() == "zone_exception") {
			// Build up the exception record from the various child elements
			if (element == "call") current_match_ = value_;
			else if (element == "cqz") current_pattern_->cq_zone = stoi(value_);
			else if (element == "start") {
				current_pattern_->validity.start = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
			else if (element == "end") {
				current_pattern_->validity.end = convert_xml_datetime(value_);
				current_pattern_->type |= cty_data::TIME_DEPENDENT;
			}
		}
		else if (elements_.size() && elements_.back() == "clublog") {
			// Add all patterns to the entities
			for (auto it : data_->patterns) {
				string match = it.first;
				for (auto ita : it.second) {
					int dxcc_id = ita->dxcc_id;
					if (data_->entities.find(dxcc_id) != data_->entities.end()) {
						data_->entities[dxcc_id]->patterns[it.first].push_back(ita);
					}
				}
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
bool cty1_reader::load_data(cty_data::all_data* data, istream& in, string& version) {
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
	status_->progress(file_size, OT_PREFIX, "Importing Exception data from XML", "bytes");
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

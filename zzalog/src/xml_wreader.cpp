#include "xml_wreader.h"
#include "utils.h"
#include "status.h"

extern status* status_;



xml_wreader::xml_wreader() :
    xml_reader()
    , reader_id_("DEFAULT")
{

}

xml_wreader::~xml_wreader() {

}

bool xml_wreader::start_element(std::string name, std::map<std::string, std::string>* attributes) {
	std::string element_name = to_upper(name);
	bool result = true;
	char msg[128];
	if (element_map_.find(element_name) == element_map_.end()) {
		snprintf(msg, sizeof(msg), "%s: Unexpected XML element %s encountered - ignored", 
			reader_id_.c_str(), name.c_str());
		status_->misc_status(ST_WARNING, msg);
		if (attributes != nullptr) {
			attributes->clear();
			delete attributes;
		}
		return false;
	} else {
		char e = element_map_.at(element_name);
		if (method_map_.find(e) == method_map_.end()) {
			snprintf(msg, sizeof(msg), "%s: Element method table screwed!", reader_id_.c_str());
			status_->misc_status(ST_SEVERE, msg);
			return false;
		} else {
			methods m = method_map_.at(e);
			if (m.start_method) {
				result = m.start_method(this, attributes);
			} else {
				result = true;
			}
			return result;
		}
	}
}

// End element - call the appropriate handler
bool xml_wreader::end_element(std::string name) {
	std::string element_name = to_upper(name);
	char msg[128];
	if (element_map_.find(element_name) == element_map_.end()) {
		snprintf(msg, sizeof(msg), "%s: Unexpected XML element %s encountered - ignored",
			reader_id_.c_str(), element_name.c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	} // else
	char e = element_map_.at(element_name);
	char exp_e = elements_.back();
	elements_.pop_back();
	if (e != exp_e) {
		snprintf(msg, sizeof(msg), "%s: Invalid XML - Not expecting element %s", 
			reader_id_.c_str(), name.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	} 
	// else
	if (method_map_.find(e) == method_map_.end()) {
		snprintf(msg, sizeof(msg), "%s: Element method table screwed!", reader_id_.c_str());
		status_->misc_status(ST_SEVERE, msg);
	return false;
	} // else
	methods m = method_map_.at(e);
	if (m.end_method) {
		return m.end_method(this);
	} else {
		return true;
	}
}

bool xml_wreader::declaration(xml_element::element_t e, std::string n, std::string c) {
	return true;
}

bool xml_wreader::process_instr(std::string n, std::string c) {
	return true;
}

bool xml_wreader::characters(std::string content) {
	char msg[128];
	if (elements_.size()) {
		char e = elements_.back();
		if (method_map_.find(e) == method_map_.end()) {
			snprintf(msg, sizeof(msg), "%s: Element method table screwed!", reader_id_.c_str());
			status_->misc_status(ST_SEVERE, msg);
			return false;
		} // else
		methods m = method_map_.at(e);
		if (m.chars_method) {
			return m.chars_method(this, content);
		} else {
			return true;
		}
	}
	return true;
}


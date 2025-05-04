#include "rig_reader.h"
#include "rig_data.h"
#include "rig_if.h"
#include "status.h"

#include "hamlib/rig.h"

using namespace std;

extern status* status_;
extern bool closing_;
extern string PROGRAM_VERSION;

// Consgructor
rig_reader::rig_reader() :
    xml_reader()
    , data_(nullptr)
    , rig_data_(nullptr)
    , rig_name_("")
    , app_name_("")
    , in_file_(nullptr)
{
    elements_.clear();
}

// Destructor
rig_reader::~rig_reader() {
    elements_.clear();
}

bool rig_reader::load_data(map<string, rig_data_t*>* data, istream& in) {
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
	status_->misc_status(ST_NOTE, "RIG DATA: Started");
	status_->progress(file_size, OT_RIGS, "Converting XML into Rig CAT database", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "RIG DATA: Done!");
		status_->progress(file_size, OT_RIGS);
		return true;
	}
	else if (closing_) {
		status_->misc_status(ST_WARNING, "RIG_DATA: Read cancelled as close-down requested");
		status_->progress("Load abandoned", OT_RIGS);
		return false;
	} else {
		// Read failed - report failure
		status_->misc_status(ST_FATAL, "RIG DATA: Read failed");
		status_->progress("Load failed", OT_RIGS);
		return false;
	}
}

bool rig_reader::start_element(string name, map<string, string>* attributes) {
	string element_name = to_upper(name);
	bool result = true;
	if (element_map_.find(element_name) == element_map_.end()) {
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG DATA: Unexpected XML element %s encountered - ignored");
		status_->misc_status(ST_WARNING, msg);
		if (attributes != nullptr) {
			attributes->clear();
			delete attributes;
		}
		return false;
	} else {
		rigs_element_t e = element_map_.at(element_name);
		if (method_map_.find(e) == method_map_.end()) {
			status_->misc_status(ST_SEVERE, "RIG DATA: Element method table screwed!");
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
bool rig_reader::end_element(string name) {
	string element_name = to_upper(name);
	char msg[128];
	if (element_map_.find(element_name) == element_map_.end()) {
		snprintf(msg, sizeof(msg), "RIG DATA: Unexpected XML element %s encountered - ignored");
		status_->misc_status(ST_WARNING, msg);
		return false;
	} // else
	rigs_element_t e = element_map_.at(element_name);
	rigs_element_t exp_e = elements_.back();
	elements_.pop_back();
	if (e != exp_e) {
		snprintf(msg, sizeof(msg), "RIG DATA: Invalid XML - Not expecting element %s", name.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	} 
	// else
	if (method_map_.find(e) == method_map_.end()) {
		status_->misc_status(ST_SEVERE, "RIG DATA: Element method table screwed!");
		return false;
	} // else
	methods m = method_map_.at(e);
	if (m.end_method) {
		return m.end_method(this);
	} else {
		return true;
	}
}

bool rig_reader::declaration(xml_element::element_t e, string n, string c) {
	return true;
}

bool rig_reader::processing_instr(string n, string c) {
	return true;
}

bool rig_reader::characters(string content) {
	if (elements_.size()) {
		rigs_element_t e = elements_.back();
		if (method_map_.find(e) == method_map_.end()) {
			status_->misc_status(ST_SEVERE, "RIG DATA: Element method table screwed!");
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

// <rigs version="...">
bool rig_reader::start_rigs(rig_reader* that, map<string, string>* attributes) {
	if (that->elements_.size()) {
		status_->misc_status(ST_ERROR, "RIG DATA: unexpected RIGS element ");
		return false;
	}
	// else
	that->elements_.push_back(RIG_RIGS);
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "VERSION") {
			if (!that->check_version(it.second)) {
				return false;
			}
			return true;
		} 
		// else 
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG DATA: Unexpected attribute %=% in RIGS element",
			it.first.c_str(), it.second.c_str());
		return false;
	}
	return true;
}

// <rig name="FlRig">
bool rig_reader::start_rig(rig_reader* that, map<string, string>*attributes) {
	char msg[128];
	// Only expect in RIGS
	if (that->elements_.back() != RIG_RIGS) {
		status_->misc_status(ST_ERROR, "RIG DATA: Unexpected RIG element");
		return false;
	}
	// else
	that->elements_.push_back(RIG_RIG);
	// get the name
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "NAME") {
			if (that->data_->find(it.second) != that->data_->end()) {
				snprintf(msg, sizeof(msg), "RIG DATA: Duplicate rig %s found",
					it.second.c_str());
				status_->misc_status(ST_ERROR, msg);
				return false;
			}
			// else
			that->rig_name_ = it.second;
			that->rig_data_ = new rig_data_t;
			(*that->data_)[it.second] = that->rig_data_;
			return true;
		}
		// else
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG DATA: Unexpected attribute %=% in RIG element",
			it.first.c_str(), it.second.c_str());
		return false;
	}
	return true;
}

// <value name=...>....
bool rig_reader::start_value(rig_reader* that, map<string, string>* attributes) {
	switch(that->elements_.back()) {
	case RIG_RIG:
	case RIG_APP:
		that->elements_.push_back(RIG_VALUE);
		// get the name
		for (auto it : *attributes) {
			string name = to_upper(it.first);
			if (name == "NAME") {
				// Iyt might be an empty value
				that->value_data_ = "";
				that->value_name_ = it.second;
				return true;
			}
			// else
			char msg[128];
			snprintf(msg, sizeof(msg), "RIG DATA: Unexpected attribute %=% in VALUE element",
				it.first.c_str(), it.second.c_str());
			return false;
		}
	default:
		status_->misc_status(ST_ERROR, "RIG DATA: Unexpected VALUE element");
		return false;
	}
}

// <app name="FlRig">
bool rig_reader::start_app(rig_reader* that, map<string, string>*attributes) {
	// Only expect in RIGS
	if (that->elements_.back() != RIG_RIG) {
		status_->misc_status(ST_ERROR, "RIG DATA: Unexpected APP element");
		return false;
	}
	// else
	that->elements_.push_back(RIG_APP);
	// get the name
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "NAME") {
			that->app_name_ = it.second;
			that->app_data_ = new cat_data_t;
			that->rig_data_->cat_data.push_back(that->app_data_);
			that->app_data_->nickname = it.second;
			that->app_data_->hamlib = new hamlib_data_t;
			return true;
		}
		// else
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG DATA: Unexpected attribute %=% in APP element",
			it.first.c_str(), it.second.c_str());
		return false;
	}
	return true;
}

// </rigs>
bool rig_reader::end_rigs(rig_reader* that) {
	if (that->elements_.size()) {
		status_->misc_status(ST_ERROR, "RIG DATA: Closing </rigs> encountered while incomplete elements are outstanding");
		return false;
	}
	return true;	
}

// </rig>
bool rig_reader::end_rig(rig_reader* that) {
	that->rig_data_ = nullptr;
	that->rig_name_ = "";
	return true;
}

// </app>
bool rig_reader::end_app(rig_reader* that) {
	// There are a few items in hamlib data that need to be tidied up
	hamlib_data_t* hd = that->app_data_->hamlib;
	const rig_caps* capabilities = rig_get_caps(hd->model_id);
	if (capabilities == nullptr) {
		char msg[128];
		snprintf(msg, sizeof(msg), "RIG: No CAT details for %s", that->rig_name_.c_str());
		status_->misc_status(ST_WARNING, msg);
	}
	else {
		if (capabilities->model_name != hd->model ||
			capabilities->mfg_name != hd->mfr) {
			char msg[128];
			snprintf(msg, 128, "RIG: Saved model id %d (%s/%s) does not match supplied rig model %s/%s using hamlib values",
				hd->model_id,
				capabilities->mfg_name,
				capabilities->model_name,
				hd->mfr.c_str(),
				hd->model.c_str());
			status_->misc_status(ST_WARNING, msg);
			hd->model = capabilities->model_name;
			hd->mfr = capabilities->mfg_name;
		}
		hd->port_type = capabilities->port_type;
	}

	that->app_data_ = nullptr;
	that->app_name_ = "";
	return true;
}

// </value>
bool rig_reader::end_value(rig_reader* that) {
	char msg[128];
	rigs_element_t parent = that->elements_.back();
	rig_data_t* rd = that->rig_data_;
	cat_data_t* ad;
	hamlib_data_t* hd;
	switch(parent) {
	case RIG_RIG: 
		if (that->value_name_ == "Default App") 
			rd->default_app = stoi(that->value_data_);
		else if (that->value_name_ == "Antenna")
			rd->antenna = that->value_data_;
		else if (that->value_name_ == "Instantaneous Values")
			rd->use_instant_values = (bool)stoi(that->value_data_);
		else {
			snprintf(msg, sizeof(msg), "RIG DATA: Unexpected value item %s: %s in RIG %s",
				that->value_name_.c_str(),
				that->value_data_.c_str(),
				that->rig_name_.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
		return true;
	case RIG_APP:
		ad = rd->cat_data.back();
		hd = ad->hamlib;
		if (that->value_name_ == "Rig Model") hd->model = that->value_data_;
		else if (that->value_name_ == "Manufacturer") hd->mfr = that->value_data_;
		else if (that->value_name_ == "Port") hd->port_name = that->value_data_;
		else if (that->value_name_ == "Baud Rate") 
			hd->baud_rate = stoi(that->value_data_);
		else if (that->value_name_ == "Model ID") 
			hd->model_id = stoi(that->value_data_);
		else if (that->value_name_ == "Timeout") 
			hd->timeout = stod(that->value_data_);
		else if (that->value_name_ == "Maximum Timeouts")
			hd->max_to_count = stoi(that->value_data_);
		else if (that->value_name_ == "S-meter Hold")
			hd->num_smeters = stoi(that->value_data_);
		else if (that->value_name_ == "Command") ad->app = that->value_data_;
		else if (that->value_name_ == "Override Hamlib") 
			ad->override_hamlib = (bool)stoi(that->value_data_);
		else if (that->value_name_ == "Power Mode")
			hd->power_mode = (power_mode_t)stoi(that->value_data_);
		else if (that->value_name_ == "Maximum Power") 
			hd->max_power = stod(that->value_data_);
		else if (that->value_name_ == "Frequency Mode")
			hd->freq_mode = (freq_mode_t)stoi(that->value_data_);
		else if (that->value_name_ == "Crystal Frequency")
			hd->frequency = stod(that->value_data_);
		else if (that->value_name_ == "Amplifier Gain")
			hd->gain = stoi(that->value_data_);
		else if (that->value_name_ == "Transverter Offset")
			hd->freq_offset = stod(that->value_data_);
		else if (that->value_name_ == "Transverter Power") 
			hd->tvtr_power = stod(that->value_data_);
		else if (that->value_name_ == "Accessories")
			hd->accessory = (accessory_t)stoi(that->value_data_);
		else if (that->value_name_ == "Start Automatically")
			ad->auto_start = (bool)stoi(that->value_data_);
		else if (that->value_name_ == "Connect Automatically")
			ad->auto_connect = (bool)stoi(that->value_data_);
		else if (that->value_name_ == "Connect Delay")
			ad->connect_delay = stod(that->value_data_);
		else {
			snprintf(msg, sizeof(msg), "RIG DATA: Unexpected value item %s: %s in RIG/APP %s/%s",
				that->value_name_.c_str(),
				that->value_data_.c_str(),
				that->rig_name_.c_str(),
				that->app_name_.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
		return true;
	default:
		return false;
	}
}

bool rig_reader::chars_value(rig_reader* that, string content) {
	that->value_data_ = content;
	return true;
}

// Check app version is >= version in settings file
bool rig_reader::check_version(string v) {
	vector<string> file_words;
	split_line(v, file_words, '.');
	vector<string> prog_words;
	split_line(PROGRAM_VERSION, prog_words, '.');
	if (stoi(prog_words[0]) > stoi(file_words[0])) {
		return true;
	} else if (prog_words[0] == file_words[0]) {
		if (stoi(prog_words[1]) > stoi(prog_words[1])) {
			return true;
		}
		else if (prog_words[1] == file_words[1]) {
			if (stoi(prog_words[2]) >= stoi(file_words[2])) {
				return true;
			}
		}
	}
	return false;
}
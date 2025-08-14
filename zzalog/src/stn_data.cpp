#include "stn_data.h"
#include "stn_reader.h"
#include "stn_writer.h"
#include "status.h"

extern status* status_;
extern string default_data_directory_;

stn_data::stn_data()
{
	load_failed_ = false;
	load_data();
}

stn_data::~stn_data() {
	store_data();
}

// Load data from station.xml
void stn_data::load_data() {
	string filename = default_data_directory_ + "station.xml";
	ifstream is;
	char msg[128];
	is.open(filename, ios_base::in);
	if (is.good()) {
		stn_reader* reader = new stn_reader();
		if (reader->load_data(&qths_, &opers_, &calls_, is)) {
			status_->misc_status(ST_OK, "STN DATA: XML Loaded OK");
		}
		else {
			load_failed_ = true;
			snprintf(msg, sizeof(msg), "STN DATA: File %s did not load correctly", filename.c_str());
			status_->misc_status(ST_ERROR, msg);
		}
	}
	else {
		snprintf(msg, sizeof(msg), "STN DATA: File %s may not exist", filename.c_str());
		status_->misc_status(ST_WARNING, msg);
	}
}

// Store data to station.xml
void stn_data::store_data() {
	if (!load_failed_) {
		string filename = default_data_directory_ + "station.xml";
		ofstream os;
		os.open(filename, ios_base::out);
		if (os.good()) {
			stn_writer* writer = new stn_writer();
			if (!writer->store_data(&qths_, &opers_, &calls_, os)) {
				status_->misc_status(ST_ERROR, "STN DATA: Error writing XML");
			}
		}
		else {
			char msg[128];
			snprintf(msg, sizeof(msg), "STN DATA: Failed to open %s", filename.c_str());
		}
	}
}

// Add a specific item - returns true if added
bool stn_data::add_qth_item(string id, qth_value_t item, string value) {
	if (qths_.find(id) == qths_.end()) {
		// New QTH
		qth_info_t* info = new qth_info_t;
		info->data = { { item, value } };
		qths_[id] = info;
		return true;
	} else {
		if (qths_.at(id)->data.find(item) == qths_.at(id)->data.end()) {
			// New item
			qths_.at(id)->data[item] = value;
			return true;
		}
		else {
			qths_.at(id)->data.at(item) = value;
			return true;
		}
	}
}

// remove an item
void stn_data::remove_qth_item(string id, qth_value_t item) {
	if (qths_.find(id) != qths_.end()) {
		qths_.at(id)->data.erase(item);
	}
}

// Add a new QTH
bool stn_data::add_qth(string id, qth_info_t* qth) {
	if (qths_.find(id) == qths_.end()) {
		// New QTH
		qths_[id] = qth;
		return true;
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "STN DATA: Already have data for QTH %s", id.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

// Add a specific item - returns true if added
bool stn_data::add_oper_item(string id, oper_value_t item, string value) {
	if (opers_.find(id) == opers_.end()) {
		// New QTH
		oper_info_t* info = new oper_info_t;
		info->data = { { item, value } };
		opers_[id] = info;
		return true;
	}
	else {
		if (opers_.at(id)->data.find(item) == opers_.at(id)->data.end()) {
			// New item
			opers_.at(id)->data[item] = value;
			return true;
		}
		else {
			opers_.at(id)->data.at(item) = value;
			return true;
		}
	}
}

// Add a new operator
bool stn_data::add_oper(string id, oper_info_t* oper) {
	if (opers_.find(id) == opers_.end()) {
		// New QTH
		opers_[id] = oper;
		return true;
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "STN DATA: Already have data for Operator \"%s\"", id.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
}

// Fetch the QTH info
const qth_info_t* stn_data::get_qth(string id) {
	if (qths_.find(id) == qths_.end()) {
		//char msg[128];
		//snprintf(msg, sizeof(msg), "STN DATA: No data for QTH \"%s\"", id.c_str());
		//status_->misc_status(ST_ERROR, msg);
		return nullptr;
	}
	else {
		return qths_.at(id);
	}
}
// Fetch the Operatot info
const oper_info_t* stn_data::get_oper(string id) {
	if (opers_.find(id) == opers_.end()) {
		//char msg[128];
		//snprintf(msg, sizeof(msg), "STN DATA: No data for Operator \"%s\"", id.c_str());
		//status_->misc_status(ST_ERROR, msg);
		return nullptr;
	}
	else {
		return opers_.at(id);
	}
}

// Get all QTHs
const map<string, qth_info_t*>* stn_data::get_qths() {
	return &qths_;
}

// Get all operators
const map<string, oper_info_t*>* stn_data::get_opers() {
	return &opers_;
}

// Add a new callsign
bool stn_data::add_call(string call) {
	if (calls_.find(call) != calls_.end()) {
		return false;
	}
	else {
		calls_[call] = "";
		return true;
	}
}

// get existing calls
const map<string, string>* stn_data::get_calls() {
	return &calls_;
}

// Known call
bool stn_data::known_call(string call) {
	if (calls_.find(call) == calls_.end()) return false;
	else return true;
}

// Known call
bool stn_data::known_qth(string call) {
	if (qths_.find(call) == qths_.end()) return false;
	else return true;
}

// Known call
bool stn_data::known_oper(string call) {
	if (opers_.find(call) == opers_.end()) return false;
	else return true;
}

// Get descriptor for a specific call
string stn_data::get_call_descr(string id) {
	if (calls_.find(id) == calls_.end()) {
		return "";
	}
	else {
		return calls_.at(id);
	}
}



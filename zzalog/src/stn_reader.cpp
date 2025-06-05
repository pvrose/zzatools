#include "stn_reader.h"
#include "stn_data.h"
#include "status.h"

using namespace std;

extern status* status_;
extern bool closing_;


// Constructor
stn_reader::stn_reader() :
	xml_wreader()
	, qths_(nullptr)
	, opers_(nullptr)
	, qth_name_("")
	, oper_name_("")
	, item_name_("")
	, item_value_("")
{
	elements_.clear();
	xml_wreader::method_map_ = method_map_;
	xml_wreader::element_map_ = element_map_;
}

stn_reader::~stn_reader() {
	elements_.clear();
}

// Loaddata
bool stn_reader::load_data(
	map<string, qth_info_t*>* qths,
	map<string, oper_info_t*>* opers,
	map<string, string>* scalls,
	istream& in) {
	qths_ = qths;
	opers_ = opers;
	scalls_ = scalls;
	in_file_ = &in;
	// calculate the file size and initialise the progress bar
	streampos startpos = in.tellg();
	in.seekg(0, ios::end);
	streampos endpos = in.tellg();
	long file_size = (long)(endpos - startpos);
	// reposition back to beginning
	in.seekg(0, ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "STN DATA: Started");
	status_->progress(file_size, OT_STN, "Converting XML into Station database", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(in)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "STN DATA: Done!");
		status_->progress(file_size, OT_STN);
		return true;
	}
	else if (closing_) {
		status_->misc_status(ST_WARNING, "STN DATA: Read cancelled as close-down requested");
		status_->progress("Load abandoned", OT_STN);
		return false;
	}
	else {
		// Read failed - report failure
		status_->misc_status(ST_FATAL, "STN DATA: Read failed");
		status_->progress("Load failed", OT_RIGS);
		return false;
	}
}

// The start methods
bool stn_reader::start_station(xml_wreader* rdr, map<string, string>* attributes) {
	if (rdr->elements_.size()) {
		status_->misc_status(ST_ERROR, "STN DATA: Unexpected STATION element");
		return false;
	}
	rdr->elements_.push_back(STN_STATION);
	return true;
}

bool stn_reader::start_location(xml_wreader* rdr, map<string, string>* attributes) {
	// Only expect in STATION
	if (rdr->elements_.back() != STN_STATION) {
		status_->misc_status(ST_ERROR, "STN DATA: Unexpected LOCATION element");
		return false;
	}
	rdr->elements_.push_back(STN_LOCATION);
	stn_reader* that = (stn_reader*)rdr;
	char msg[128];
	string description = "";
	// Getthe name
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "ID") {
			if (that->qths_->find(it.second) != that->qths_->end()) {
				snprintf(msg, sizeof(msg), "STN DATA: Duplicate QTH %s found",
					it.second.c_str());
				status_->misc_status(ST_ERROR, msg);
				return false;
			}
			// else
			that->qth_name_ = it.second;
			that->qth_ = new qth_info_t;
			(*that->qths_)[it.second] = that->qth_;
		}
		else if (name == "DESCRIPTION") {
			description = it.second;
		}
		else {
			// else
			char msg[128];
			snprintf(msg, sizeof(msg), "STN DATA: Unexpected attribute %s=%s in LOCATION element",
				it.first.c_str(), it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
	}
	that->qth_->description = description;
	return true;

}

bool stn_reader::start_item(xml_wreader* rdr, map<string, string>* attributes) {
	stn_reader* that = (stn_reader*)rdr;
	switch (rdr->elements_.back()) {
	case STN_LOCATION:
	case STN_OPERATOR:
		that->elements_.push_back(STN_ITEM);
		// get the name
		for (auto it : *attributes) {
			string name = to_upper(it.first);
			if (name == "NAME") {
				// Iyt might be an empty value
				that->item_value_ = "";
				that->item_name_ = it.second;
				return true;
			}
			// else
			char msg[128];
			snprintf(msg, sizeof(msg), "STN DATA: Unexpected attribute %s=%s in ITEM element",
				it.first.c_str(), it.second.c_str());
			return false;
		}
	default:
		status_->misc_status(ST_ERROR, "STN DATA: Unexpected ITEM element");
		return false;
	}
}

bool stn_reader::start_operator(xml_wreader* rdr, map<string, string>* attributes) {
	// Only expect in STATION
	if (rdr->elements_.back() != STN_STATION) {
		status_->misc_status(ST_ERROR, "STN DATA: Unexpected OPERATOR element");
		return false;
	}
	rdr->elements_.push_back(STN_OPERATOR);
	stn_reader* that = (stn_reader*)rdr;
	char msg[128];
	string description;
	// Getthe name
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "ID") {
			if (that->opers_->find(it.second) != that->opers_->end()) {
				snprintf(msg, sizeof(msg), "STN DATA: Duplicate Operator %s found",
					it.second.c_str());
				status_->misc_status(ST_ERROR, msg);
				return false;
			}
			// else
			that->oper_name_ = it.second;
			that->oper_ = new oper_info_t;
			(*that->opers_)[it.second] = that->oper_;
		}
		else if (name == "DESCRIPTION") {
			description = it.second;
		}
		else {
			// else
			char msg[128];
			snprintf(msg, sizeof(msg), "STN DATA: Unexpected attribute %s=%s in OPERATOR element",
				it.first.c_str(), it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
	}
	that->oper_->description = description;
	return true;
}

bool stn_reader::start_scallsign(xml_wreader* rdr, map<string, string>* attributes) {
	// Only expect in STATION
	if (rdr->elements_.back() != STN_STATION) {
		status_->misc_status(ST_ERROR, "STN DATA: Unexpected OPERATOR element");
		return false;
	}
	rdr->elements_.push_back(STN_SCALLSIGN);
	stn_reader* that = (stn_reader*)rdr;
	string call = "";
	string description = "";
	for (auto it : *attributes) {
		string name = to_upper(it.first);
		if (name == "CALL") call = it.second;
		else if (name == "DESCRIPTION") description = it.second;
		else {
			// else
			char msg[128];
			snprintf(msg, sizeof(msg), "STN DATA: Unexpected attribute %s=%s in SCALLSIGN element",
				it.first.c_str(), it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
	}
	(*that->scalls_)[call] = description;
	return true;
}

// End mehods
bool stn_reader::end_station(xml_wreader* rdr) {
	if (rdr->elements_.size()) {
		status_->misc_status(ST_ERROR, "STN DATA: Closing </station> encountered while incomplete elements are outstanding");
		return false;
	}
	return true;
}

bool stn_reader::end_location(xml_wreader* rdr) {
	stn_reader* that = (stn_reader*)rdr;
	that->qth_ = nullptr;
	that->qth_name_ = "";
	return true;
}

map<string, qth_value_t> stn_reader::qth_value_map_ = {
	{ "Street", STREET },
	{ "City", CITY },
	{ "Postcode", POSTCODE },
	{ "Locator", LOCATOR },
	{ "Country", DXCC_NAME },
	{ "DXCC", DXCC_ID },
	{ "Primary Subdivision", PRIMARY_SUB },
	{ "Secondary Subdivision", SECONDARY_SUB },
	{ "CQ Zone", CQ_ZONE },
	{ "ITU Zone", ITU_ZONE },
	{ "Continent", CONTINENT },
	{ "IOTA", IOTA },
	{ "WAB", WAB }
};

map<string, oper_value_t> stn_reader::oper_value_map_ = {
	{ "Name", NAME },
	{ "Callsign", CALLSIGN }
};

bool stn_reader::end_item(xml_wreader* rdr) {
	char msg[128];
	stn_reader* that = (stn_reader*)rdr;
	stn_element_t parent = (stn_element_t)that->elements_.back();
	switch (parent) {
	case STN_LOCATION: {
		if (qth_value_map_.find(that->item_name_) == qth_value_map_.end()) {
			snprintf(msg, sizeof(msg), "STN DATA: Unexpected value item %s: %s in QTH %s",
				that->item_name_.c_str(),
				that->item_value_.c_str(),
				that->qth_name_.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
		qth_value_t v = qth_value_map_.at(that->item_name_);
		that->qth_->data[v] = that->item_value_;
		break;
	}
	case STN_OPERATOR: {
		if (oper_value_map_.find(that->item_name_) == oper_value_map_.end()) {
			snprintf(msg, sizeof(msg), "STN DATA: Unexpected value item %s: %s in Operator %s",
				that->item_name_.c_str(),
				that->item_value_.c_str(),
				that->oper_name_.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
		oper_value_t v = oper_value_map_.at(that->item_name_);
		that->oper_->data[v] = that->item_value_;
		break;
	}
	default: 
		return false;
	}
	return false;
}

bool stn_reader::end_operator(xml_wreader* rdr) {
	stn_reader* that = (stn_reader*)rdr;
	that->oper_ = nullptr;
	that->oper_name_ = "";
	return true;
}

// Character methods
bool stn_reader::chars_item(xml_wreader* rdr, string content) {
	stn_reader* that = (stn_reader*)rdr;
	that->item_value_ = content;
	return true;
}


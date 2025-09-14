#include "contest_reader.h"

#include "contest_data.h"
#include "status.h"

#include "drawing.h"

extern bool closing_;
extern status* status_;


contest_reader::contest_reader() :
	xml_wreader()
	, the_data_(nullptr)
	, contest_data_(nullptr)
	, contest_id_("")
	, contest_ix_("")
{
	elements_.clear();
	xml_wreader::method_map_ = method_map_;
	xml_wreader::element_map_ = element_map_;
}

contest_reader::~contest_reader() {
	elements_.clear();
}

// Load data
bool contest_reader::load_data(contest_data* d, std::istream& is) {
	the_data_ = d;
	in_file_ = &is;
	// calculate the file size and initialise the progress bar
	std::streampos startpos = is.tellg();
	is.seekg(0, std::ios::end);
	std::streampos endpos = is.tellg();
	long file_size = (long)(endpos - startpos);
	// reposition back to beginning
	is.seekg(0, std::ios::beg);
	// Initialsie the progress
	status_->misc_status(ST_NOTE, "CONTEST: Started");
	status_->progress(file_size, OT_CONTEST, "Converting XML into Rig CAT database", "bytes");
	// Call the XML parser - calls back to the overides herein
	if (parse(is)) {
		// Read successful - complete progress
		status_->misc_status(ST_OK, "CONTEST: Done!");
		status_->progress(file_size, OT_CONTEST);
		return true;
	}
	else if (closing_) {
		status_->progress("Load abandoned", OT_CONTEST);
		status_->misc_status(ST_WARNING, "RIG_DATA: Read cancelled as close-down requested");
		return false;
	}
	else {
		// Read failed - report failure
		status_->progress("Load failed", OT_CONTEST);
		status_->misc_status(ST_FATAL, "CONTEST: Read failed");
		return false;
	}
}

// Start <CONTESTS>
bool contest_reader::start_contests(xml_wreader* that, std::map<std::string, std::string>* attributes) {
	if (that->elements_.size()) {
		status_->misc_status(ST_ERROR, "CONTEST: unexpected CONTESTS element");
		return false;
	} 
	// else
	that->elements_.push_back(CT_CONTESTS);
	if (attributes != nullptr && attributes->size()) {
		status_->misc_status(ST_ERROR, "CONTEST: Unexpected attributes in CONTESTS element");
		return false;
	}
	return true;
}

// Start <CONTEST id= index=>
bool contest_reader::start_contest(xml_wreader* wr, std::map<std::string, std::string>* attributes) {
	contest_reader* that = (contest_reader*)wr;
	char msg[128];
	if (that->elements_.back() != CT_CONTESTS) {
		status_->misc_status(ST_ERROR, "CONTEST: Unexpected CONTEST element");
		return false;
	}
	// else
	that->elements_.push_back(CT_CONTEST);
	// get ID and Index
	for (auto it : *attributes) {
		std::string name = to_upper(it.first);
		if (name == "ID") {
			that->contest_id_ = it.second;
		}
		else if (name == "INDEX") {
			that->contest_ix_ = it.second;
		}
		else {
			snprintf(msg, sizeof(msg), "CONTEST: Unexpected attribute %s=%s in CONTEST element",
				it.first.c_str(), it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
	}
	std::map<std::string, std::map<std::string, ct_data_t*> >& contests = ((contest_reader*)that)->the_data_->contests_;
	if (contests.find(that->contest_id_) == contests.end()) {
		contests[that->contest_id_] = {};
	}
	std::map<std::string, ct_data_t*>& ct_group = contests.at(that->contest_id_);
	if (ct_group.find(that->contest_ix_) != ct_group.end()) {
		snprintf(msg, sizeof(msg), "CONTEST: We have already read contest %s.%s", that->contest_id_.c_str(), that->contest_ix_.c_str());
		status_->misc_status(ST_ERROR, msg);
		return false;
	}
	ct_group[that->contest_ix_] = new ct_data_t;
	that->contest_data_ = ct_group.at(that->contest_ix_);
	return true;
}

// Start <TIMEFRAME start= finish=>
bool contest_reader::start_timeframe(xml_wreader* wr, std::map<std::string, std::string>* attributes) {
	contest_reader* that = (contest_reader*)wr;
	char msg[128];
	if (that->elements_.back() != CT_CONTEST) {
		status_->misc_status(ST_ERROR, "CONTEST: Unexpected TIMEFRAME element");
		return false;
	}
	// else
	that->elements_.push_back(CT_TIMEFRAME);
	// Get start and finish
	for (auto it : *attributes) {
		std::string name = to_upper(it.first);
		if (name == "START") {
			that->contest_data_->date.start = std::chrono::system_clock::from_time_t(that->convert_xml_datetime(it.second));
		}
		else if (name == "FINISH") {
			that->contest_data_->date.finish = std::chrono::system_clock::from_time_t(that->convert_xml_datetime(it.second));
		}
		else {
			snprintf(msg, sizeof(msg), "CONTEST: Unexpected attribute %s=%s in TIMEFRAME element",
				it.first.c_str(), it.second.c_str());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
	}
	return true;
}

// Start <VALUE name=>
bool contest_reader::start_algorithm(xml_wreader* wr, std::map<std::string, std::string>* attributes) {
	contest_reader* that = (contest_reader*)wr;
	switch (that->elements_.back()) {
	case CT_CONTEST:
		that->elements_.push_back(CT_ALGORITHM);
		// get the name
		for (auto it : *attributes) {
			std::string name = to_upper(it.first);
			if (name == "NAME") {
				// Iyt might be an empty value
				that->contest_data_->algorithm = it.second;
				return true;
			}
			// else
			char msg[128];
			snprintf(msg, sizeof(msg), "CONTEST: Unexpected attribute %s=%s in ALGORITHM element",
				it.first.c_str(), it.second.c_str());
			return false;
		}
	default:
		status_->misc_status(ST_ERROR, "CONTEST: Unexpected ALGORITHM element");
		return false;
	}
	return true;
}

// End </CONTESTS>
bool contest_reader::end_contests(xml_wreader* that) {
	if (that->elements_.size()) {
		status_->misc_status(ST_ERROR, "CONTEST: Closing </contests> encountered while incomplete elements are outstanding");
		return false;
	}
	return true;
}

// End </CONTEST>
bool contest_reader::end_contest(xml_wreader* wr) {
	contest_reader* that = (contest_reader*)wr;
	that->contest_data_ = nullptr;
	that->contest_id_ = "";
	that->contest_ix_ = "";
	return true;
}


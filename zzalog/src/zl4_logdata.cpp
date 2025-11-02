#include "zl4_logdata.h"

#include "main.h"
#include "record.h"
#include "settings.h"

#include <exception>
#include <fstream>
#include <regex>
#include <string>

using string = std::string;

//! Constructor
zl4_logdata::zl4_logdata()
{	
	settings s;
	s.get<std::string>("Data Directory", data_directory_, "./logdata");
	s.get<qso_id>("Next QSO ID", next_qso_id_, 1);
	th_launder_ = new std::thread(th_launder_data, this);
}

//! Destructor
//! Stop launder thread and clean up
zl4_logdata::~zl4_logdata()
{
	if (th_launder_) {
		while(any_dirty() || deleted_qsos_.size() > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		th_launder_->join();
		delete th_launder_;
		th_launder_ = nullptr;
	}
}

//! Thread function to launder data to filestore
//! \param that zl4_logdata instance
//! Launder data to filestore continually if any records are dirty
//! or any records have been deleted
//! This runs in a separate thread
//! This function loops forever until the zl4_logdata instance is destroyed
//! It sleeps for 1 second between checks
//! If any records are dirty or any records have been deleted
//! it calls launder_data to write them to filestore
void zl4_logdata::th_launder_data(zl4_logdata* that)
{
	while (true) {
		if (that->any_dirty() || that->deleted_qsos_.size() > 0) {
			that->launder_data();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

//! Load ZL4 log data from filestore
//! \return true if successful
//! Loads all log data from filestore into memory
//! records are kept in filestore one per file named by QSO ID split 100 per directory
//! e.g. logdata/00/00/1.adi, logdata/00/00/2.adi ... logdata/99/99/99.adi etc.
bool zl4_logdata::load_data()
{
	log_data_.clear();
	for (qso_id id = 1; id < next_qso_id_; id++) {
		string filename = qso_filename(id);
		std::ifstream i(filename);
		if (!i.is_open()) {
			// File does not exist - skip
			continue;
		}
		string data;
		while (i.is_open()) {
			string line;
			std::getline(i, line);
			data += line + "\n";
		}
		record rec;
		from_adif(data, rec);
		log_data_[id] = { rec, false };
	}
	return true;
}

//! Write dirty records to filestore
//! \return true if successful
bool zl4_logdata::launder_data()
{
	for (auto& item : log_data_) {
		qso_id id = item.first;
		log_record_t& log_rec = item.second;
		if (log_rec.dirty) {
			string filename = qso_filename(id);
			string data;
			to_adif(data, log_rec.rec);
			std::ofstream o(filename);
			if (o.is_open()) {
				o << data;
				o.close();
				log_rec.dirty = false;
			}
			else {
				printf("ZL4_LOGDATA: Failed to launder QSO ID %u to file %s\n",
					id, filename.c_str());
				return false;
			}
		}
	}
	for (auto& id : deleted_qsos_) {
		string filename = qso_filename(id);
		if (remove(filename.c_str()) != 0) {
			printf("ZL4_LOGDATA: Failed to delete QSO ID %u file %s\n",
				id, filename.c_str());
			return false;
		}
		else {
			deleted_qsos_.remove(id);
		}
	}
	return true;
}

//! Get record by QSO ID

//! \param id QSO identifier
//! \return reference to record
record* zl4_logdata::get(qso_id& id)
{
	if (log_data_.size() == 0) {
		id = 0;
		return nullptr;
	}
	while (log_data_.find(id) == log_data_.end()) id = (id + 1) % next_qso_id_;
	return &log_data_[id].rec;
}

//! Get record by search criteria

//! \param id QSO identifier to start search from
//! \param criteria Search criteria
//! \return reference to record
//! Returns the next record matching the search criteria starting from QSO ID id
//! If no match is found id is set to 0 and nullptr returned
//! The search is circular - it wraps around to the start of the log data
//! If id is 0 the search starts from the beginning of the log data
//! If a match is found id is updated to the QSO ID of the matching record
//! If no match is found id is set to 0
//! If log data is empty nullptr is returned and id is set to 0
//! If multiple records match the criteria the first one found is returned
//! If no records match the criteria nullptr is returned and id is set to 0
record* zl4_logdata::get_next(qso_id& id, search_criteria_t& criteria)
{
	if (log_data_.size() == 0) {
		id = 0;
		return nullptr;
	}
	qso_id start_id = id;
	if (id == 0) id = 1;
	do {
		if (log_data_.find(id) != log_data_.end()) {
			log_record_t& log_rec = log_data_[id];
			if (matches(log_rec.rec, criteria)) {
				return &log_rec.rec;
			}
		}
		id = (id + 1) % next_qso_id_;
		if (id == 0) id = 1;
	} while (id != start_id);
	// No match found
	id = 0;
	return nullptr;
}

//! Remove record from log data.
//! \param id QSO identifier
//! \return true if successful
//! Removes the record with QSO ID id from the log data
//! If the record does not exist false is returned
//! If the record is successfully removed true is returned
//! If log data is empty false is returned
//! If id is 0 false is returned
//! If the record is not found false is returned
//! If the record is found and removed true is returned
//! If the record is removed it is moved to deleted_qsos so that launder_data will delete it from filestore	
bool zl4_logdata::erase(qso_id& id)
{
	if (log_data_.size() == 0 || id == 0) {
		return false;
	}
	auto it = log_data_.find(id);
	if (it != log_data_.end()) {
		log_data_.erase(it);
		deleted_qsos_.push_back(id);
		return true;
	}
	else {
		return false;
	}
}

//! Add or update a record in the log data.

//! \param id QSO identifier (add record if 0) - returns new QSO ID
//! \param rec Record to add or update
//! \return true if successful
//! If id is 0 a new record is added and id is updated to the new QSO ID
//! If id is non-zero the existing record with that QSO ID is updated
//! If the record is successfully added or updated true is returned
bool zl4_logdata::put(qso_id& id, const record& rec)
{
	if (id == 0) {
		// Add new record
		log_record_t log_rec;
		log_rec.rec = rec;
		log_rec.dirty = true;
		log_data_[next_qso_id_] = log_rec;
		id = next_qso_id_;
		next_qso_id_++;
		return true;
	}
	else {
		// Update existing record
		auto it = log_data_.find(id);
		if (it != log_data_.end()) {
			log_record_t& log_rec = it->second;
			log_rec.rec = rec;
			log_rec.dirty = true;
			return true;
		}
		else {
			// Record not found
			return false;
		}
	}
}

//! Return filename for QSO ID

//! \param id QSO identifier
//! \return filename
//! Returns the filename for the record with QSO ID id
string zl4_logdata::qso_filename(qso_id id)
{
	char dir1[4];
	char dir2[4];
	sprintf(dir1, "%02u", (id / 10000) % 100);
	sprintf(dir2, "%02u", (id / 100) % 100);
	string filename = data_directory_ + "/" + string(dir1) + "/" + string(dir2) + "/" + std::to_string(id) + ".adi";
	return filename;
}


//! Return true if search criteria matches record

//! \param rec QSO record
//! \param criteria Search criteria
//! \return true if record matches criteria
//! Returns true if the record matches the search criteria
bool zl4_logdata::matches(record& rec, search_criteria_t& criteria)
{	// Two level matching
	bool match = basic_match(rec, criteria) && refine_match(rec, criteria);
	return match;
}

//! Return true if the record matches the field comparison criteria.

//! \param rec QSO record to match.
//! \param criteria Search criteria.
//! \return true if the field comparison check agrees, false if not.
bool zl4_logdata::basic_match(record& rec, search_criteria_t& criteria)
{	// See if record matches criterion
	switch (criteria.condition) {
	case XC_UNFILTERED:
		// match all records
		return true;
	case XC_CALL:
		// match by callsign
		return match_string(criteria.pattern, criteria.comparator, rec.item("CALL"));
		break;
	case XC_CONT:
		// match by continent
		return match_string(criteria.pattern, criteria.comparator, rec.item("CONT"));
		break;
	case XC_CQZ:
		// match by CQ Zone number
		return match_int(criteria.pattern, criteria.comparator, rec.item("CQZ"));
		break;
	case XC_DXCC:
	{
		if (criteria.pattern.length() == 0) {
			// Null std::string - check records with no value
			return match_string(criteria.pattern, criteria.comparator, rec.item("DXCC"));
		}
		else {
			// Not a nickname so match against the raw value
			return match_int(criteria.pattern, criteria.comparator, rec.item("DXCC"));
		}
	}
	case XC_FIELD:
		// match by a specified field
		return match_string(criteria.pattern, criteria.comparator, rec.item(criteria.field_name));
		break;
	case XC_ITUZ:
		// match by ITU zone number
		return match_int(criteria.pattern, criteria.comparator, rec.item("ITUZ"));
		break;
	case XC_SQ2:
		// match by first two characters of locator
		// condition too short
		if (criteria.pattern.length() < 2) break;
		// gridsquare in record too short
		if (criteria.comparator != XP_REGEX && rec.item("GRIDSQUARE").length() < 2) break;
		return match_string(criteria.pattern.substr(0, 2), criteria.comparator, rec.item("GRIDSQUARE").substr(0, 2));
		break;
	case XC_SQ4:
		// match by first 4 charactes of locator
		if (criteria.pattern.length() < 4) break;
		if (criteria.comparator != XP_REGEX && rec.item("GRIDSQUARE").length() < 4) break;
		return match_string(criteria.pattern.substr(0, 4), criteria.comparator, rec.item("GRIDSQUARE").substr(0, 4));
		break;
	default:
		return false;
	}
	// we should never get here
	return false;
}

//! Refine match.

//! Returns whether the record matches the additional refinement criteria:
//! - Band matches.
//! - Mode matches.
//! - Date within sleected range.
//! \param record QSO record to match.
//! \param criteria Search criteria.
//! \return true if the addition criteria match, false if not.
bool zl4_logdata::refine_match(record& record, search_criteria_t& criteria)
{
	// now refine by dates
	if (criteria.by_dates) {
		std::string record_date = record.item("QSO_DATE");
		// confirm the match is between specified dates - inclusive
		if (record_date < criteria.from_date || record_date > criteria.to_date) {
			return false;
		}
	}
	// now refine by band - confirm if the record is on that band
	if (criteria.band != "Any" && criteria.band != record.item("BAND")) {
		return false;
	}
	// Refine by mode - confirm if the record has that mode 
	if (criteria.mode != "Any" && criteria.mode != record.item("MODE") &&
		criteria.mode != record.item("SUBMODE")) {
		return false;
	}
	// Refine by call - confirm if the record matches my_call (STATION_CALLSIGN)
	if (criteria.my_call != "Any" && criteria.my_call != record.item("STATION_CALLSIGN")) {
		return false;
	}
	// Refine by eQSL card - confirm if eQSL confirmation
	if (criteria.confirmed_eqsl && record.item("EQSL_QSL_RCVD") != "Y") {
		return false;
	}
	// Refine by LotW - confirm if LotW confirmation
	if (criteria.confirmed_eqsl && record.item("LOTW_QSL_RCVD") != "Y") {
		return false;
	}
	// Refine by card - confirm if card confirmation
	if (criteria.confirmed_card && record.item("QSL_RCVD") != "Y") {
		return false;
	}
	return true;
}

//! item matching - std::string value.
bool zl4_logdata::match_string(std::string test, int comparator, std::string value)
{
	switch ((search_comp_t)comparator) {
	case XP_REGEX:
		try {
			std::regex pattern(test, std::regex::icase);
			return std::regex_search(value, pattern);
		}
		catch (...) {
			return false;
		}
	case XP_NE:
		return (to_upper(value) != to_upper(test));
	case XP_LT:
		return (to_upper(value) < to_upper(test));
	case XP_LE:
		return (to_upper(value) <= to_upper(test));
	case XP_EQ:
		return (to_upper(value) == to_upper(test));
	case XP_GE:
		return (to_upper(value) >= to_upper(test));
	case XP_GT:
		return (to_upper(value) > to_upper(test));
	default:
		return false;
	}
}

//! integer item matching - ignores things like leading zeros and trailing non numeric characters
//! \param test the value against which the field is being compared.
//! \param comparator search_comp_t operator for the comparison.
//! \param value the value from the QSO record.
//! \return true if the values match, false if not.
//! Converts the test and value to integers and compares them
bool zl4_logdata::match_int(std::string test, int comparator, std::string value)
{
	try {
		return match_int(std::stoi(test), comparator, std::stoi(value));
	}
	catch (const std::invalid_argument&) {
		return false;
	}
}

//! integer item matching - compares integer values
//! 
//! \param test the value against which the field is being compared.
//! \param comparator search_comp_t operator for the comparison.
//! \param value the value from the QSO record.
//! \return true if the values match, false if not.
//! Compares the integer values according to the comparator
//! If comparator is XP_REGEX false is returned
bool zl4_logdata::match_int(int test, int comparator, int value)
{
	switch ((search_comp_t)comparator) {
	case XP_REGEX:
		return false;
	case XP_NE:
		return (value != test);
	case XP_LT:
		return (value < test);
	case XP_LE:
		return (value <= test);
	case XP_EQ:
		return (value == test);
	case XP_GE:
		return (value >= test);
	case XP_GT:
		return (value > test);
	default:
		return false;
	}
}

//! Return true if any record is dirty
//! \return true if any record is dirty
//! Returns true if any record in log data is dirty
//! If log data is empty false is returned
//! If any record is dirty true is returned
//! If no records are dirty false is returned
//! This is used to determine if launder_data needs to be called
bool zl4_logdata::any_dirty()
{
	for (auto& item : log_data_) {
		log_record_t& log_rec = item.second;
		if (log_rec.dirty) {
			return true;
		}
	}
	return false;
}

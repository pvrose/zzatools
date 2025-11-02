#pragma once

#include "record.h"
#include "search.h"

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <thread>

//! This class provides the data for all the log records and updates the filestore copies.
class zl4_logdata
{
public:
	//! Constructor	
	zl4_logdata();
	//! Destructor
	~zl4_logdata();

	//! Load ZL4 log data from filestore
	bool load_data();
	//! Write dirty records to filestore
	bool launder_data();

	//! Get record by QSO ID
	
	//! \param id QSO identifier
	record* get(qso_id& id);

	//! Get record by search criteria
	
	//! \param id QSO identifier to start search from
	//! \param criteria Search criteria
	record* get_next(qso_id& id, search_criteria_t& criteria);

	//! Remove record from log data.
	
	//! \param id QSO identifier
	bool erase(qso_id& id);

	//! Add or update a record in the log data.
	
	//! \param id QSO identifier (add record if 0) - returns new QSO ID
	//! \param rec Record to add or update
	bool put(qso_id& id, const record& rec);

protected:

	//! Thread function to launder data to filestore
	static void th_launder_data(zl4_logdata* that);

	//! Return filename for QSO ID
	std::string qso_filename(qso_id id);

	//! Return true if search criteria matches record
	bool matches(record& rec, search_criteria_t& criteria);

	//! Return true if any record is dirty
	bool any_dirty();

	//! Basic match.

	//! Returns whether the record matches the field comparison criteria.
	//! \param rec QSO record to match.
	//! \param criteria Search criteria.
	//! \return true if the field comparison check agrees, false if not.
	bool basic_match(record& rec, search_criteria_t& criteria);

	//! Refine match.

	//! Returns whether the record matches the additional refinement criteria:
	//! - Band matches.
	//! - Mode matches.
	//! - Date within sleected range.
	//! - Station callsign matches.
	//! - QSL checks.
	//! \param record QSO record to match.
	//! \param criteria Search criteria.
	//! \return true if the addition criteria match, false if not.
	bool refine_match(record& record, search_criteria_t& criteria);

	//! item matching - std::string value.

	//! Returns whether the \p value matches the \p test according to the \p comparator.
	//! \param test the value against which the field is being compared.
	//! \param value the value from the QSO record.
	//! \param comparator search_comp_t operator for the comparison.
	//! \return true if the values match, false if not.
	bool match_string(std::string test, int comparator, std::string value);

	//! Item matching - integer value of std::string items.

	//! \see match_string.
	bool match_int(std::string test, int comparator, std::string value);

	//! Item matching - integer values.

	//! \see match_string.
	bool match_int(int test, int comparator, int value);

	//! Log data record structure
	struct log_record_t {
		record rec;         //!< QSO record
		bool dirty;        //!< true if record has been modified since last launder
	};

	std::map<qso_id, log_record_t> log_data_;   //!< Log data records mapped by QSO ID

	qso_id next_qso_id_;                       //!< Next QSO ID to use for new records

	std::thread* th_launder_;                  //!< Thread to launder data to filestore

	std::string data_directory_;              //!< Directory where log data is stored

	std::list <qso_id> deleted_qsos_;            //!< List of deleted QSO IDs

};


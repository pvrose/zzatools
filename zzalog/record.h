/*
ZZALOG - Amateur Radio Log
© - Copyright 2017, Philip Rose, GM3ZZA
All Rights Reserved

record.h - Individual record data item: header file
*/
#ifndef __RECORD__
#define __RECORD__

#include "utils.h"

#include <map>
#include <string>
#include <istream>
#include <ostream>

using namespace std;

namespace zzalog {

	// When updating log - result of comparision between log and update
	enum match_result_t : char {
		MT_NOMATCH = 0,      // No matching record found in log
		MT_EXACT,            // An exact match found in log
		MT_PROBABLE,         // A close match - same band/date/call but time out by upto 30 minutes
		MT_POSSIBLE,         // call found but something important differs
		MT_LOC_MISMATCH,     // A close match but a location field differs
		MT_SWL_MATCH,        // An SWL report that is a close match to existing activity
		MT_SWL_NOMATCH       // An SWL report that is no match for any activity
	};

	// Logging mode - used when initialising a record
	enum logging_mode_t {
		LM_OFF_AIR,     // Off-line logging
		LM_RADIO_CONN,  // radio connected to log
		LM_RADIO_DISC,  // Realtime logging, no rigconnection
		LM_IMPORTED     // Auto-import
	};

	// The records are kept in a container with size_t as index
	typedef size_t record_num_t;

	// This class represents a single record as a container of field items NAME=>VALUE
	class record : public map<string, string> {
	public:

		// Constructors and Destructors
	public:
		// Default constructor
		record();
		// Constructor that pre-populates certain fields
		record(logging_mode_t type);
		// Copy constructor
		record(const record& rhs);
		// Assignment operator
		record& operator= (const record& rhs);
		// Destructor
		virtual ~record();

		// Public methods
	public:
		// Set an item pair
		void item(string field, string value, bool formatted = false);
		// Get an item - as string
		string item(string field, bool formatted = false, bool indirect = false);
		// get an item - as an integer
		void item(string field, int& value);
		// get an item - as a double
		void item(string field, double& value);
		// is the QSO valid - has a minimum subset of fields
		bool is_valid();
		// does the item exist - in the map and not an empty string
		bool item_exists(string field);
		// set the header information
		void header(string comment);
		// get the header information
		string header();
		// is a header record
		bool is_header();
		// is the record more recent
		bool operator > (record& rhs);
		// delete all the derived fields
		void unparse();
		// get the latitude and longitude of the contacted station
		lat_long_t location(bool my_station);
		// update BAND from FREQ
		bool update_band(bool force = false);
		// combine records
		bool merge_records(record* record, bool allow_locn_mismatch = false);
		// records are duplicates
		match_result_t match_records(record* record);
		// Add timeoff if its isn't set
		void update_timeoff();
		// change the field name
		void change_field_name(string from, string to);
		// End record by adding certain fields
		void end_record(logging_mode_t mode);
		// Write to an item merging data from other items
		string item_merge(string data);
		// get the timestamp
		time_t timestamp(bool time_off = false);
		// items match between records
		bool items_match(record* record, string field_name);
		// Set user details - returns true if item gets modified
		bool user_details();

		// protected methods
	protected:
		// Delete all contents
		void delete_contents();

		// protected attributes
	protected:
		// record is a header
		bool is_header_;
		// header comment
		string header_comment_;
		// incomplete record read
		bool is_incomplete_;

		// Expecting a header record
		static bool expecting_header_;

	public:
		// Avoid reporting errors too many times
		static bool inhibit_error_reporting_;

	};

}

#endif

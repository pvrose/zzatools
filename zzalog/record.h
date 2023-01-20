/*
ZZALOG - Amateur Radio Log
© - Copyright 2017, Philip Rose, GM3ZZA
All Rights Reserved

record.h - Individual record data item: header file
*/
#ifndef __RECORD__
#define __RECORD__

#include "../zzalib/utils.h"

#include <map>
#include <string>
#include <istream>
#include <ostream>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// When updating log - result of comparision between log and update
	enum match_result_t : char {
		MT_NOMATCH = 0,      // No matching record found in log
		MT_EXACT,            // An exact match found in log
		MT_PROBABLE,         // A close match - same band/date/call but time out by upto 30 minutes
		MT_POSSIBLE,         // call found but something important differs
		MT_LOC_MISMATCH,     // A close match but a location field differs
		MT_SWL_MATCH,        // An SWL report that is a close match to existing activity
		MT_SWL_NOMATCH,      // An SWL report that is no match for any activity
		MT_2XSWL_MATCH       // An SWL report matches an existing SWL report
	};

	// Location source
	enum location_t {
		LOC_NONE,        // not derived
		LOC_LATLONG,     // LAT/LON pair
		LOC_GRID2,       // 2-letter gridsquare
		LOC_GRID4,       // 4-character gridsquare
		LOC_GRID6,       // 6-character 
		LOC_GRID8        // 8-character
	};

	// record display format and save precision for frequency
	enum display_freq_t : int {
		FREQ_Hz,
		FREQ_kHz,
		FREQ_MHz,
		FREQ_END,
	};

	// record display format for dates
	enum display_date_t {
		DATE_YYYYMMDD,
		DATE_YYYY_MM_DD,
		DATE_DD_MM_YYYY,
		DATE_MM_DD_YYYY,
		DATE_DD_MON_YYYY,
		DATE_END,
	};

	// record display format for times
	enum display_time_t {
		TIME_HHMMSS,
		TIME_HHMM,
		TIME_HH_MM_SS,
		TIME_HH_MM,
		TIME_END,
	};


	// The records are kept in a container with size_t as index
	typedef size_t record_num_t;

	// forward declaration
	enum hint_t : unsigned char;

	// This class represents a single record as a container of field items NAME=>VALUE
	class record : public map<string, string> {
	public:

		// Constructors and Destructors
	public:
		// Default constructor
		record();
		//// Constructor that pre-populates certain fields
		//record(logging_mode_t type, record* copy_from);
		// Copy constructor
		record(const record& rhs);
		// Assignment operator
		record& operator= (const record& rhs);
		// Destructor
		virtual ~record();

		// Public methods
	public:
		// Set an item pair returns true if succeeded
		void item(string field, string value, bool formatted = false, bool dirty = true);
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
		// get the latitude and longitude of the contacted station (ignotre less than 6 character gridsquares)
		lat_long_t location(bool my_station, location_t& source);
		// Get the latitude and longuitude without specifying a source
		lat_long_t location(bool my_station);
		// update BAND from FREQ
		bool update_band(bool force = false);
		// combine records - update result with hint to use in subsequent update
		bool merge_records(record* record, bool allow_locn_mismatch = false, hint_t* result = nullptr);
		// records are duplicates
		match_result_t match_records(record* record);
		// Add timeoff if its isn't set
		void update_timeoff();
		// change the field name
		void change_field_name(string from, string to);
		//// End record by adding certain fields
		//void end_record(logging_mode_t mode);
		// Write to an item merging data from other items
		string item_merge(string data, bool indirect = false);
		// get the timestamp
		time_t timestamp(bool time_off = false);
		// items match between records
		bool items_match(record* record, string field_name);
		// Set user details - returns true if item gets modified
		bool user_details();
		// Formatting methos
		static string format_freq(display_freq_t format, string value);
		static string format_date(display_date_t format, string value);
		static string format_time(display_time_t format, string value);
		static string unformat_freq(display_freq_t format, string value);
		static string unformat_date(display_date_t format, string value);
		static string unformat_time(display_time_t format, string value);
		// Record is dirty
		bool is_dirty();
		// Clear dirty marker
		void clean();
		// Delete all contents
		void delete_contents();


		// protected methods
	protected:
		// protected attributes
	protected:
		// record is a header
		bool is_header_;
		// header comment
		string header_comment_;
		// incomplete record read
		bool is_incomplete_;
		// Record is dirty
		bool is_dirty_;

		// Expecting a header record
		static bool expecting_header_;

	public:
		// Avoid reporting errors too many times
		static bool inhibit_error_reporting_;

	};

}

#endif

/*
ZZALOG - Amateur Radio Log
� - Copyright 2017, Philip Rose, GM3ZZA
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
#include <chrono>

using namespace std;

	//! When updating log - result of comparision between log and update
	enum match_result_t : char {
		MT_NOMATCH = 0,      //!< No matching record found in log
		MT_EXACT,            //!< An exact match found in log
		MT_PROBABLE,         //!< A close match - same band/date/call but time out by upto 30 minutes
		MT_POSSIBLE,         //!< Call found but something important differs
		MT_UNLIKELY,         //!< A close match - same band/date/call but time out by > 30 mins
		MT_LOC_MISMATCH,     //!< A close match but a location field differs
		MT_SWL_MATCH,        //!< An SWL report that is a close match to existing activity
		MT_SWL_NOMATCH,      //!< An SWL report that is no match for any activity
		MT_2XSWL_MATCH,      //!< An SWL report matches an existing SWL report
		MT_OVERLAP,          //!< The two records have same freq/mode and times overlap
	};

	//! Flags used in match_records
	enum match_flags_t : uchar {
		MR_NONE,             //!< No special match instructions - default
		MR_ALLOW_LOC = 1,    //!< ALlow limited location mismatch (used for LOTW)
		MR_ALLOW_QSLS = 2,   //!< Allow processing of QSL_SENT* (used for OQRS)
	};


	//! Location source
	enum location_t : uchar {
		LOC_NONE,        //!< not derived
		LOC_PREFIX,      //!< Obtained from prefix data
		LOC_LATLONG,     //!< LAT/LON pair
		LOC_GRID2,       //!< 2-letter gridsquare
		LOC_GRID4,       //!< 4-character gridsquare
		LOC_GRID6,       //!< 6-character 
		LOC_GRID8        //!< 8-character
	};

	// forward declaration
	enum hint_t : uchar;

	typedef size_t qso_num_t;    // QSO number

	//! This class represents a single QSO record as a container of field items NAME=>VALUE
	class record : public map<string, string> {
	public:

		// Constructors and Destructors
	public:
		//! Default constructor
		record();
		//! Copy constructor
		record(const record& rhs);
		//! Assignment operator
		record& operator= (const record& rhs);
		//! Destructor
		virtual ~record();

		// Public methods
	public:
		//! Set an item.
		
		//! \param field Field name.
		//! \param value Field value.
		//! \param formatted if true the displayed format is converted to ADIF format.
		//! \param dirty if true ther record is marked dirty if the contents change.
		void item(string field, string value, bool formatted = false, bool dirty = true);
		//! Returns the item
		
		//! \param field Field name.
		//! \param formatted if true converts data to the displayed format.
		//! \return Field value.
		string item(string field, bool formatted = false);
		//! Gets an integer item
		
		//! \param field Field name
		//! \param value Receives field value converted to an integer, 0 if it cannot be.
		void item(string field, int& value);
		//! Gets a double-precision value
		
		//! \param field Field name
		//! \param value Receives field value converted to a double-precision, NAN if it cannot be.
		void item(string field, double& value);
		//! Gets a long long item
		
		//! \param field Field name
		//! \param value Receives field value converted to an unsigned 64-bit integer, 0 if it cannot be.
		void item(string field, unsigned long long& value);
		//! Returns true if the QSO is valid - has a minimum subset of fields
		bool is_valid();
		//! Returns true if the item named \p field exists and is not an empty string.
		bool item_exists(string field);
		//! Set the header information
		void header(string comment);
		//! Returns the header information
		string header();
		//! Returns true if the record is a header record.
		bool is_header();
		//! Comparison operator. One record is greater than another if the start date and time is later.
		bool operator > (record& rhs);
		//! delete all the derived fields
		void unparse();
		//! Returns the latitude and longitude.
		
		//! \param my_station if true returns the user's coordinates, otherwise of the contacted station.
		//! \param source Receives an indication of how the coordinates were calculated.
		lat_long_t location(bool my_station, location_t& source);
		//! Get the latitude and longuitude without specifying a source.
		
		//! \param my_station if true returns the user's coordinates, otherwise of the contacted station.
		lat_long_t location(bool my_station);
		//! update BAND from FREQ. If \p force is false do not overwrite an existing BAND field.
		bool update_band(bool force = false);
		//! combine records - update result with hint to use in subsequent update.
		
		//! \param record QSO record to be merged into this one.
		//! \param flags match_flags_t to use to control the match.
		//! \param result Receives a hint that describes the merge.
		bool merge_records(record* record, match_flags_t flags = MR_NONE, hint_t* result = nullptr);
		//! Returns match_result_t between QSO \p record and this QSO record.
		match_result_t match_records(record* record);
		//! Add timeoff if its isn't set
		void update_timeoff();
		//! Update DISTANCE and ANT_AZ fields. 
		void update_bearing();
		//! change the field name from value in \p from to value in \p to.
		void change_field_name(string from, string to);
		//! Returns a string where field names in angle brackets in \p data are replaced by their values.
		string item_merge(string data, bool indirect = false);
		//! Returns the timestamp as time_t of the record.
		
		//! \param time_off if true use QSO_DATE_OFF + TIME_OFF rather than QS_DATE + TIME_ON for the QSO time.
		time_t timestamp(bool time_off = false);
		//! Returns the timestamp as time_point of the record.
		
		//! \param time_off if true use QSO_DATE_OFF + TIME_OFF rather than QS_DATE + TIME_ON for the QSO time.
		chrono::system_clock::time_point ctimestamp(bool time_off = false);
		//! Itema \p field_name match between \p record and this record.
		bool items_match(record* record, string field_name);
		//! Delete all contents
		void delete_contents();
		//! Delete QSL statuses.
		void invalidate_qsl_status();

		// protected attributes
	protected:
		//! record is a header
		bool is_header_;
		//! header comment
		string header_comment_;
		//! incomplete record read
		bool is_incomplete_;

		//! Expecting a header record
		static bool expecting_header_;

	public:
		//! Avoid reporting errors too many times
		static bool inhibit_error_reporting_;

	};

#endif

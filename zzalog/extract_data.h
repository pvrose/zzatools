#ifndef __EXTRACT_DATA__
#define __EXTRACT_DATA__

#include "book.h"
#include "search.h"

#include <vector>
#include <list>

using namespace std;

namespace zzalog {
	// This class is a container of records that have been extracted from the main log. This inheritance provides
	// additional features required for this. As only pointers to the records are kept in the main log, this log
	// contains pointers to these records and not copies of the records themselves.
	class extract_data :
		public book

	{
	public:
		// Reason for the extract
		enum extract_mode_t {
			NONE,        // empty extract
			SEARCH,      // search results
			EQSL,        // upload to eQSL.cc
			LOTW,        // upload to arrl.org/LotW
			CARD,        // printing cards.
			CLUBLOG,     // upload to ClubLog
			NO_NAME,     // Special search for empty NAME item
			NO_QTH,      // Special search for empty QTH item
			LOCATOR,     // Special seatch for non-6 or 8 character gridsquare
			DUMMY        // end of enumeration
		};

		extract_data();
		virtual ~extract_data();

		// Add/replace the search criteria
		int criteria(search_criteria_t criteria, extract_mode_t mode = SEARCH);
		// Reextract data as things may have changed
		void reextract();
		// Clear the serach criteria
		void clear_criteria();
		// Extract for specific criteria - QSL uploads
		void extract_qsl(extract_mode_t server);
		// Upload data - on saved server
		void upload();
		// Extract for specific criteria - all records with call
		void extract_call(string callsign);
		// Add a record to the list
		void add_record(record_num_t record_num);
		// Sort records by filed_name
		void sort_records(string field_name, bool reverse);
		// Special extract (NO_NAME, NO_QTH, LOCATOR_4)
		void extract_special(extract_mode_t reason);
		// Correct record order
		void correct_record_order();

		// return the real record number
		virtual record_num_t record_number(record_num_t item);
		// return the virtual record number
		virtual record_num_t item_number(record_num_t record, bool nearest = false);
		// Change the selected record (& update any necessary controls)
		virtual record_num_t selection(record_num_t num_record, hint_t hint = HT_SELECTED, view* requester = nullptr, record_num_t num_other = 0);


	protected:
		// Extract records for the criteria
		void extract_records();
		// Generate description extract criterion for use in the header comment
		string comment();
		// Swap two records
		void swap_records(record_num_t record_num_1, record_num_t record_num_2);

		// The list of extract criteria
		list<search_criteria_t> extract_criteria_;
		// The records 
		vector<record_num_t> mapping_;
		// reverse mapping
		map<record_num_t, record_num_t> rev_mapping_;
		// Current use mode
		extract_mode_t use_mode_;

	};

}
#endif

#ifndef __IMPORT_DATA__
#define __IMPORT_DATA__

#include "book.h"


#include <string>
#include <vector>

using namespace std;

namespace zzalog {

	// auto-import polling period - 15s -> 5 min (default 1 min)
	const double AUTO_IP_MAX = 300.0;
	const double AUTO_IP_MIN = 15.0;
	const double AUTO_IP_DEF = 60.0;


	// This class inherits book and provides the additional functionality required
	// for importing additional data to the main log. Records are initially held in this book
	// and copied or merged into the main log. Any problems encountered are refered to the 
	// user for resolution
	class import_data : public book
	{
	public:
		// Update or import mode
		enum update_mode_t {
			NONE,            // Book has no data
			AUTO_IMPORT,     // Book contains data being auto-imported 
			FILE_IMPORT,     // Book contains data manually loaded from a file
			EQSL_UPDATE,     // Book contains data downloaded from eQSL.cc
			LOTW_UPDATE,     // Book contains data downloaded from arrl.org/lotw
			WAIT_AUTO,       // Book is reserved for the next auto-import session
			READ_AUTO        // Data is being read - we can interrupt the process
		};

	public:
		import_data();
		~import_data();

		// Timer callback for auto-update
		static void cb_timer_imp(void* v);

		// public methods
	public:
		// Start the auto update process
		bool start_auto_update();
		// Start an automatic (on timer) update from the defined locations
		void auto_update();
		// Delete the mismatch record in the update 
		void discard_update(bool notify = true);
		// Accept the record from the update
		void accept_update();
		// Combine the record from the log and the update
		void merge_update();
		// Add the update record to the log
		void save_update();
		// Start or continue analysing the update data
		void update_book();
		// Stop importing
		void stop_update(logging_mode_t mode, bool immediate);
		// is update complete
		bool update_complete();
		// download data from QSL server
		bool download_data(update_mode_t server);
		// Merge the data into book_
		void merge_data();
		// load data for import
		bool load_data(string filename);
		// Auto-update enabled
		bool auto_enable();
		void auto_enable(bool enabled);


	protected:
		// repeat the auto-import timer
		void repeat_auto_timer();
		// Finish the update process
		void finish_update(bool merged = true);
		// Convert the update record
		void convert_update(record* record);
		// Special LotW header processing
		void process_lotw_header();

	protected:
		// a record update query is in progress
		bool update_in_progress_;
		// the update is a new record
		bool update_is_new_;
		// Whete the update is comming from
		update_mode_t update_mode_;
		// Number of records in import
		int number_to_import_;
		// The number of records in an import update
		int number_updated_;
		// The number of records that have been accepted in an import update
		int number_accepted_;
		// The number of records checked
		int number_checked_;
		// Timestamp of the previous automatic update
		string last_auto_update_;
		// Timestamp when update starts
		string this_auto_update_;
		// Number of filed being imported 
		int num_update_files_;
		// Names of the files
		string* update_files_;
		// That they should be emptied - use of bool or char gets confused with C-strings
		int* empty_files_;
		// The sources of the auto-imports
		string* sources_;
		// Close is pending
		bool close_pending_;
		// Logging mode to enacy after close
		logging_mode_t next_logging_mode_;
		// Timer count (in seconds)
		double timer_count_;
		// Auto-update enbled
		bool auto_enable_;
		// Original value of book_->enable_save_
		bool old_enable_save_;

	};

}
#endif


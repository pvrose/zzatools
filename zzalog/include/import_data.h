#ifndef __IMPORT_DATA__
#define __IMPORT_DATA__

#include "book.h"
#include "qso_manager.h"

#include <string>
#include <vector>
#include <ctime>

using namespace std;



	// auto-import polling period - 15s -> 5 min (default 1 min)
	const double AUTO_IP_MAX = 300.0;
	const double AUTO_IP_MIN = 15.0;
	const double AUTO_IP_DEF = 60.0;


	// This class inherits book and provides the additional functionality required
	// for importing additional data to the main log. Pointers to imported Records are initially held in this book
	// and pointers copied or records merged into the main log. Any problems encountered are refered to the 
	// user for resolution
	class import_data : public book
	{
	public:
		// Update or import mode
		enum update_mode_t {
			NONE,            // Book has no data
			AUTO_IMPORT,     // Book contains data being auto-imported 
			FILE_IMPORT,     // Book contains data manually loaded from a file (assume new records are correct)
			FILE_UPDATE,     // Book contains data to be merged from a file (i.e. check it exists)
			EQSL_UPDATE,     // Book contains data downloaded from eQSL.cc
			LOTW_UPDATE,     // Book contains data downloaded from arrl.org/lotw
			WAIT_AUTO,       // Book is reserved for the next auto-import session
			READ_AUTO,       // Data is being read - we can interrupt the process
			DATAGRAM,        // Book contains data received from WSJT-X datagram
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
		void discard_update(bool notify);
		// Accept the record from the update
		void accept_update();
		// Combine the record from the log and the update
		void merge_update();
		// Add the update record to the log
		void save_update();
		// Start or continue analysing the update data
		void update_book();
		// Stop importing
		void stop_update(bool immediate);
		// is update complete
		bool update_complete();
		// download data from QSL server
		bool download_data(update_mode_t server);
		// Merge the data into book_
		void merge_data();
		// load data for import
		bool load_data(string filename, update_mode_t mode);
		// Number of update files
		int number_update_files();
		// Load from a data stream
		void load_stream(stringstream& adif, update_mode_t mode);
		// Import process is auto-update or timer set for one
		bool is_auto_update();

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
		// The number of records that have been modified as a result of inport
		int number_modified_;
		// The number of records that matched in an import update
		int number_matched_;
		// The number of records checked
		int number_checked_;
		// The number of new records added
		int number_added_;
		// The number of rejected
		int number_rejected_;
		// The number of records with clublog data modified
		// as a result of LotW QSL information update
		int number_clublog_;
		// The number of SWL records added
		int number_swl_;
		// Number of files being imported 
		int num_update_files_;
		// Names of the files
		string* update_files_;
		// That they should be emptied - use of bool or char gets confused with C-strings
		int* empty_files_;
		// The sources of the auto-imports
		string* sources_;
		// Last read timestamps of update files
		string* last_timestamps_;
		// Earliest last update
		string last_timestamp_;
		// Close is pending
		bool close_pending_;
		// Start of timer
		time_t timer_start_;
		// Timer period
		double timer_period_;
		// Original value of book_->enable_save_
		bool old_enable_save_;
		// Last added record number
		record_num_t last_added_number_;

	};
#endif


#ifndef __LOTW_HANDLER__
#define __LOTW_HANDLER__

#include "fields.h"

#include <sstream>
#include <string>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

class book;
class record;
typedef size_t item_num_t;


	// The timestamp format required by the ARRL header record.
	const char LOTW_TIMEFORMAT[] = "%Y-%m-%d %H:%M:%S";

	// Default fiels to use in LotW 
	const field_list LOTW_FIELDS = {
		"CALL",
		"QSO_DATE",
		"TIME_ON",
		"TIME_OFF",
		"BAND",
		"MODE",
		"SUBMODE",
		"RST_SENT",
		"STATION_CALLSIGN"
	};


	// This class handles the interface to ARRL's Logbook-of-the-World application
	class lotw_handler
	{
	public:
		lotw_handler();
		~lotw_handler();

		// output and load data to LotW
		bool upload_lotw_log(book* book, bool mine = false);
		// download the data from LotW
		bool download_lotw_log(stringstream* adif);
		// Upload single QSO
		bool upload_single_qso(item_num_t record_num);
		// thtread callback
		static void cb_upload_done(void* v);
		// thtead-side upload QSO
		void th_upload(const char* command);
		// thread runner
		static void thread_run(lotw_handler* that);
		// main-side upload complete
		bool upload_done(int response);

	protected:
		// get user details
		bool user_details(string* username, string* password, string* last_access);
		// Download the data
		bool download_adif(const char* url, stringstream* adif);
		// Validate the data
		bool validate_adif(stringstream* adif);
		// Set the list of QSO fields
		void set_adif_fields();

		// Upload thread
		thread* th_upload_;
		// Enable for threads
		atomic<bool> run_threads_;
		// interface data
		queue<char*> upload_queue_;
		// REcords changed queue
		queue<record*> upload_done_queue_;
		queue<size_t> upload_done_szq_;
		// interface lock
		mutex upload_lock_;
		// Upload response queue
		atomic<int> upload_response_;
		// ADIF Fields
		field_list adif_fields_;

	};
#endif // !__LOTW_HANDLER__

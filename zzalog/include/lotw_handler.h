#ifndef __LOTW_HANDLER__
#define __LOTW_HANDLER__

#include "book.h"

#include <sstream>
#include <string>
#include <thread>


	// The timestamp format required by the ARRL header record.
	const char LOTW_TIMEFORMAT[] = "%Y-%m-%d %H:%M:%S";


	// This class handles the interface to ARRL's Logbook-of-the-World application
	class lotw_handler
	{
	public:
		lotw_handler();
		~lotw_handler();

		// output and load data to LotW
		bool upload_lotw_log(book* book);
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

		// Upload book
		book* upload_book_;
		// Upload thread
		thread* th_upload_;
		// Enable for threads
		atomic<bool> run_threads_;
		// interface busy
		atomic<bool> upload_if_busy_;
		// interface data
		atomic<char*> upload_request_;
		// Upload response queue
		atomic<int> upload_response_;

	};
#endif // !__LOTW_HANDLER__

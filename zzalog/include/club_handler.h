#ifndef __CLUBLOG_HANDLER__
#define __CLUBLOG_HANDLER__

#include "record.h"
#include "book.h"
#include "url_handler.h"

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

#include <FL/Fl_Help_Dialog.H>


	class club_handler
	{
	private:
		// ClubLog API developers key for gm3zza@btinternet.com
		const char* api_key_ = "ca1445fb25fef92b03c65f2484ef4d77e903e6f4";
	public:
		club_handler();
		~club_handler();

		// Upload the saved log to ClubLog using putlogs.php interface
		bool upload_log(book* book);
		// Download the exception file
		bool download_exception(string filename);
		// Upload a single QSO
		bool upload_single_qso(qso_num_t record_num);

	protected:
		// Generate the fields for posting the log
		void generate_form(vector<url_handler::field_pair>& fields, record* the_qso);
		// Get names of ADIF fields to export
		void generate_adif(set < string > &fields);
		// Unzip the downloaded  exceptions file
		bool unzip_exception(string filename);
		// Get reference directory
		void get_reference(string& dir_name);
		// Copy QSO to ADIF string
		string to_adif(record* this_record, set<string>& fields);
		// thtread callback
		static void cb_upload_done(void* v);
		// thtead-side upload QSO
		void th_upload(record* qso);
		// thread runner
		static void thread_run(club_handler* that);
		// main-side upload complete
		bool upload_done(bool response);
		// Help dialog  to display file received from post
		Fl_Help_Dialog* help_dialog_;
		// Upload thread
		thread* th_upload_;
		// Enable for threads
		atomic<bool> run_threads_;
		// interface data
		queue<record*> upload_queue_;
		queue<record*> upload_done_queue_;
		// interface lock
		mutex upload_lock_;
		// Upload response queue
		string upload_error_;
		// Response
		atomic<bool> upload_response_;
		// Single QSO ADIF
		string single_qso_;
	};

#endif
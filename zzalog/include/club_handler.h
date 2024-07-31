#ifndef __CLUBLOG_HANDLER__
#define __CLUBLOG_HANDLER__

#include "url_handler.h"
#include "fields.h"

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

#include <FL/Fl_Help_Dialog.H>

class record;
class book;
class url_handler;
typedef size_t qso_num_t;

	// Default fiels to use in Clublog 
	const field_list CLUBLOG_FIELDS = {
		"QSO_DATE",
		"TIME_ON",
		"TIME_OFF",
		"QSLRDATE",
		"QSLSDATE",
		"CALL",
		"OPERATOR",
		"MODE",
		"BAND",
		"BAND_RX",
		"FREQ",
		"QSL_RCVD",
		"LOTW_QSL_RCVD",
		"QSL_SENT",
		"DXCC",
		"PROP_MODE",
		"CREDIT_GRANTED",
		"RST_SENT",
		"RST_RCVD",
		"NOTES",
		"GRIDSQUARE"
	};


	// This class handles access to the ClubLog website
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
		// Unzip the downloaded  exceptions file
		bool unzip_exception(string filename);
		// Get reference directory
		void get_reference(string& dir_name);
		// Copy QSO to ADIF string
		string to_adif(record* this_record, field_list& fields);
		// thtread callback
		static void cb_upload_done(void* v);
		// thtead-side upload QSO
		void th_upload(record* qso);
		// thread runner
		static void thread_run(club_handler* that);
		// main-side upload complete
		bool upload_done(bool response);
		// Set ADIF fields
		void set_adif_fields();
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
		// ADIF Fields
		field_list adif_fields_;
	};

#endif
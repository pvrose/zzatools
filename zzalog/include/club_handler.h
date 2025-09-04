#ifndef __CLUBLOG_HANDLER__
#define __CLUBLOG_HANDLER__

#include "url_handler.h"
#include "fields.h"

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

class record;
class book;
class url_handler;
typedef size_t qso_num_t;

	//! Default fields for exporting to Clublog.org 
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


	//! This class handles access to the ClubLog.org website.
	class club_handler
	{
	private:
		//! \cond
		// ClubLog API developers key for gm3zza@btinternet.com
		const char* api_key_ = "ca1445fb25fef92b03c65f2484ef4d77e903e6f4";
		//! \endcond
	public:
		//! Constructor.
		club_handler();
		//! Destructor.
		~club_handler();

		//! Upload the saved log to ClubLog.org using putlogs.php interface
		
		//! \param book the set of extracted records to upload.
		//! \return true if upload successful, false if not.
		bool upload_log(book* book);
		//! Download the exception file.
		
		//! \param filename into which to download exception file.
		//! \return true if successful, false if not.
		bool download_exception(string filename);
		//! Upload a single QSO
		
		//! \param record_num the index in the logfile to upload.
		//! \return false.
		bool upload_single_qso(qso_num_t record_num);
		//! Download OQRS Request data.
		
		//! \param adif a stringstream to be used by adif_reader.
		bool download_oqrs(stringstream* adif);

	protected:
		//! Generate the data for url_handler for uploading QSO data.
		
		//! \param fields the form data for HTTP POST.
		//! \param the_qso if nullptr generate data for a file upload, otherwise for a single QSO record 
		//! with the ADIF for the QSO as in-line text.
		void generate_form(vector<url_handler::field_pair>& fields, record* the_qso);
		//! Generate the data for url_hamdler for fetching OQRS requests.
		
		//! \param req output stream to send the fetch request information.
		void generate_oqrs(ostream& req);
		//! Unzip the downloaded  exceptions file.
		bool unzip_exception(string filename);
		//! Get reference directory name.
		void get_reference(string& dir_name);
		//! Copy QSO to ADIF string.
		
		//! \param this_record the QSO record to convert.
		//! \param fields the specific fields to copy/
		//! \return ADIF .adi format as a text string.
		string to_adif(record* this_record, field_list& fields);
		//! Callback from the thread that executes the IP actions.
		
		//! Calls upload_done().
		//! \param v pointer to the club_handler instance that started the action.
		static void cb_upload_done(void* v);
		//! Upload QSO using the separate thread.
		
		//! \param qso the QSO record to upload.
		void th_upload(record* qso);
		//! Run a separate thread to handle IP traffic.
		
		//! \param that used to pass a pointer to the calling instance of club_handler 
		//! to the thread.
		static void thread_run(club_handler* that);
		//! Process the result from uploading to Clublog.org.
		
		//! Runs on the main thread.
		//! \param response true if the upload was successful, false if not.
		bool upload_done(bool response);
		//! Set ADIF fields required by Clublog.org into internal state.
		void set_adif_fields();
		//! Thread in which to run IP activity for uploading QSOs.
		thread* th_upload_;
		//! Flag to keep running the thread until ZZALOG closes.
		
		//! Written by main thread and read by upload thread.
		atomic<bool> run_threads_;
		//! Queue of requests for uploading QSOs.
		queue<record*> upload_queue_;
		//! Queue of responses from uploading QSOs.
		queue<record*> upload_done_queue_;
		//! Semaphore to control access to the upload_queue_.
		mutex upload_lock_;
		//! Holds an error response from the upload.
		string upload_error_;
		//! Flag to indicate whether the upload was successful or not.
		
		//! Written by upload thread and read by main thread.
		atomic<bool> upload_response_;
		//! The ADIF .adi representation of the single QSO upload.
		string single_qso_;
		//! List of fileds that need to uploaded to Clublog.org for each QSO.
		field_list adif_fields_;
	};

#endif
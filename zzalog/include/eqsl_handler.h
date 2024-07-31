#ifndef __EQSL_HANDLER__
#define __EQSL_HANDLER__

// #include "record.h"
// #include "import_data.h"
// #include "adi_reader.h"
#include "url_handler.h"
#include "fields.h"

#include <deque>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>

using namespace std;

class record;
class book;
class Fl_Help_View;
class Fl_Window;
typedef size_t item_num_t;
typedef size_t qso_num_t;


	// eQSL throttling - 10s
	const double EQSL_THROTTLE = 10.0;
	const char EQSL_TIMEFORMAT[] = "%Y%m%d";

	// Default fiels to use in eqsl 
	const field_list EQSL_FIELDS = {
		"QSO_DATE",
		"TIME_ON",
	    "TIME_OFF",
		"BAND",
		"MODE", 
		"CALL",
		"RST_SENT",
		"QSL_MSG"
	};

	// This class manages the requirements for uploading data to and downloading data from eQSL.cc
	class eqsl_handler
	{
	public:
		// Response for an eQSL request
		enum response_t {
			ER_OK,         // OK
			ER_SKIPPED,    // Request skipped
			ER_THROTTLED,  // Request throttled by eQSL
			ER_FAILED,     // Request failed
			ER_HTML_ERR,   // HTML error
			ER_DUPLICATE   // Duplicate request
		};
		// Data required to repeat throttled requests
		struct request_t {
			item_num_t record_num;
			bool force;
			request_t()
				: record_num(0)
				, force(false)
			{
			}
			request_t(item_num_t n, bool f)
				: record_num(n)
				, force(f)
			{}
		};
		// Queue control
		enum queue_control_t {
			EQ_PAUSE,      // pause
			EQ_START,      // start
			EQ_ABANDON     // abandon
		};
		// Request queue
		typedef queue<request_t> queue_t;
		// Information required when removing entry from queue
		struct dequeue_param_t {
			queue_t* queue;
			eqsl_handler* handler;
			dequeue_param_t() {
				queue = nullptr;
				handler = nullptr;
			}
			dequeue_param_t(queue_t* q, eqsl_handler* h)
				: queue(q)
				, handler(h)
			{
			}
		};
		// Upload response
		struct upload_response_t {
			response_t status;
			string error_message;
			string html;
			record* qso;
			upload_response_t() {
				status = ER_OK;
				error_message = "";
				html = "";
				qso = nullptr;
			}
		};
	public:
		eqsl_handler();
		~eqsl_handler();

		// enqueue a request to fetch a qsl card
		void enqueue_request(qso_num_t record_num, bool force = false);
		// download the data from eqsl
		bool download_eqsl_log(stringstream* adif);
		// queue timer
		void enable_fetch(queue_control_t control);
		// Upload the data
		bool upload_eqsl_log(book* book);
		// is there active fetching
		bool requests_queued();
		// get the local card file-name
		string card_filename_l(record* record, bool use_default = false);
		// does the file exist and is it a valid png file
		bool card_file_valid(string& filename);
		// Enqueue single record
		bool upload_single_qso(qso_num_t record_num);

	protected:
		// call backs
		// timer cb to pop the queue
		static void cb_timer_deq(void* v);
		// request an eqsl card 
		response_t request_eqsl(request_t request);
		// get the remote filename of the card
		response_t card_filename_r(record* record, string& filename, string& filetype);
		// copy the card to local file-store
		response_t download(string remote_filename, string local_filename);
		// get user details
		bool user_details(string* username, string* password, string* last_access, string* qsl_message, string* swl_message);
		// Get the ADIF filename
		response_t adif_filename(string& filename);
		// Download the data
		response_t download_adif(string& filename, stringstream* adif);
		// Generate list of adif fields
		void set_adif_fields();
		// Generate list of POST FORM fields
		void form_fields(vector<url_handler::field_pair>&);
		// parse bad record
		map<string, string> parse_warning(string text);
		// thtread callback
		static void cb_upload_done(void* v);
		// thtead-side upload QSO
		bool th_upload_qso(record* qso);
		// thread runner
		static void thread_run(eqsl_handler* that);

		// main-side upload complete
		bool upload_done(upload_response_t* response);
		// Opne the Help Viewer to display the response from eQSL.
		void display_response(string response);

	protected:
		// The throttled request queue
		queue_t request_queue_;
		// dequeue parameter
		dequeue_param_t dequeue_parameter_;
		// Request queue is allowed to empty
		bool empty_queue_enable_;
		// Username
		string username_;
		// Password
		string password_;
		// Upload thread
		thread* th_upload_;
		// Enable for threads
		atomic<bool> run_threads_;
		// interface data
		queue<record*> upload_queue_;
		// Interface q lock
		mutex upload_lock_;
		// Upload response queue
		atomic<upload_response_t*> upload_response_;
		// Set of field names
		field_list adif_fields_;
		
		// Window and Help viewer for displaying response
		Fl_Window* help_window_;
		Fl_Help_View* help_viewer_;

	};
#endif

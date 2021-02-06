#ifndef __EQSL_HANDLER__
#define __EQSL_HANDLER__

#include "record.h"
#include "import_data.h"
#include "adi_reader.h"
#include "../zzalib/url_handler.h"

#include <deque>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <set>

#include <FL/Fl_Help_Dialog.H>

using namespace std;

namespace zzalog {

	// eQSL throttling - 10s
	const double EQSL_THROTTLE = 10.0;
	const char EQSL_TIMEFORMAT[] = "%Y%m%d";

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
			ER_HTML_ERR    // HTML error
		};
		// Data required to repeat throttled requests
		struct request_t {
			record_num_t record_num;
			bool force;
			request_t()
				: record_num(0)
				, force(false)
			{
			}
			request_t(record_num_t n, bool f)
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

	public:
		eqsl_handler();
		~eqsl_handler();

		// enqueue a request to fetch a qsl card
		void enqueue_request(record_num_t record_num, bool force = false);
		// download the data from eqsl
		bool download_eqsl_log(stringstream* adif);
		// queue timer
		void enable_fetch(queue_control_t control);
		// Upload the data
		bool upload_eqsl_log(book* book);
		// is there active fetching
		bool requests_queued();
		// get the local card file-name
		string card_filename_l(record* record);
		// Set/Clear debug_enabled_
		void debug_enable(bool value);
		// Upload single record
		bool upload_single_qso(record_num_t record_num);

	protected:
		// call backs
		// timer cb to pop the queue
		static void cb_timer_deq(void* v);
		// request an eqsl card 
		response_t request_eqsl(request_t request);
		// does the file exist and is it a valid png file
		bool card_file_valid(string& filename);
		// get the remote filename of the card
		response_t card_filename_r(record* record, string& filename);
		// copy the card to local file-store
		response_t download(string remote_filename, string local_filename);
		// get user details
		bool user_details(string* username, string* password, string* last_access, string* qsl_message, string* swl_message);
		// Get the ADIF filename
		response_t adif_filename(string& filename);
		// Download the data
		response_t download_adif(string& filename, stringstream* adif);
		// Generate list of adif fields
		void adif_fields(set<string>& fields);
		// Generate list of POST FORM fields
		void form_fields(vector<url_handler::field_pair>&);
		// parse bad record
		map<string, string> parse_warning(string text);

	protected:
		// The throttled request queue
		queue_t request_queue_;
		// dequeue parameter
		dequeue_param_t dequeue_parameter_;
		// Request queue is allowed to empty
		bool empty_queue_enable_;
		// Help dialog
		Fl_Help_Dialog* help_dialog_;
		// Queue enabled for debug
		bool debug_enabled_;
		// Username
		string username_;
		// Password
		string password_;
		// Book saving ebaled
		bool save_enabled_;
	};

}
#endif
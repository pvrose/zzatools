#ifndef __QRZ_HANDLER__
#define __QRZ_HANDLER__

#include <string>
#include <istream>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

using namespace std;

class book;
class record;
class xml_element;
class Fl_Help_Dialog;
typedef size_t qso_num_t;

// This class provides the interface to the QRZ.com XML database look-up - it currently
// is restricted to looking up details based on callsign. 
// Structure to hold API data per QRZ.com logbook		
struct qrz_api_data {
	bool used;
	string key;
	unsigned long long last_logid;
	string last_download;
};


	class qrz_handler
	{
	public:
		qrz_handler();
		~qrz_handler();

		// Load the API information
		void load_data();
		// Update the API information
		void store_data();

		

		// Open the QRZ.com session
		bool open_session();
		// Fetch the contacts details and present the user with a merge opportunity
		bool fetch_details(record* qso);
		// Callback to say done
		void merge_done();
		// REturn message
		string get_merge_message();
		// Get merge data
		record* get_record();
		// Is there a session?
		bool has_session();
		// Open web-page using browser
		void open_web_page(string callsign);
		// Download update - for STATION_CALLSIGN == station
		bool download_qrzlog_log(stringstream* adif);
		// Upload current QSL.
		bool upload_single_qso(qso_num_t qso_number);
		// Upload extracted log
		bool upload_log(book* log);


	protected:
		
		enum insert_resp_t { IR_GOOD, IR_FAIL, IR_DUPLICATE, IR_ERROR };
		// Upload response
		struct upload_resp_t {
			insert_resp_t success = IR_ERROR;
			string message = "";
			record* qso = nullptr;
			unsigned long long logid = 0ull;
		};
		// Decode session response
		bool decode_session_response(istream& response);
		// DEcode details response
		bool decode_details_response(istream& response);
		// Query user on merge
		bool query_merge();
		// Generate URI for session request
		string generate_session_uri();
		// Generate URI for details request
		string generate_details_uri(string callsign);
		// Fetch user details
		bool user_details();
		// Decode an XML element - into a data map
		bool decode_xml(xml_element* element);
		// Decode QRZDatabase element
		bool decode_top(xml_element* element);
		// Decode Session element
		bool decode_session(xml_element* element);
		// Decode Callsign element
		bool decode_callsign(xml_element* element);
		// Generate fetch request
		bool fetch_request(qrz_api_data* api, ostream& request);
		// Decode fetch response
		bool fetch_response(qrz_api_data* api, istream& response, int& count, string& adif);
		// Generate insert request
		bool insert_request(qrz_api_data* api, ostream& request, record* qso);
		// Decode insert response
		insert_resp_t insert_response(qrz_api_data* api, istream& response, 
			string& fail_reason, unsigned long long& logid);
		// Callback when insert complete
		static void cb_upload_done(void* v);
		// Call to upload thread to upload qso
		void th_upload_qso(record* qso);
		// Run the thread in the background
		static void thread_run(qrz_handler* that);
		// Upload done (run in main thread)
		void upload_done(upload_resp_t* resp);

		// The key returned from the login session attempt
		string session_key_;
		// User's QRZ.com account name
		string username_;
		// User's QRZ.com password
		string password_;
		// Current response data
		map< string, string> data_;
		// and as a record
		record* qrz_info_;
		// QRZ database version
		string qrz_version_;
		// Merge complete
		bool merge_done_;
		// Use XML Database
		bool use_xml_database_;
		// Non-subscriber
		bool non_subscriber_;
		// Use API
		bool use_api_;
		// Upload per QSO
		bool upload_qso_;
		// Web dialog
		Fl_Help_Dialog* web_dialog_;
		// API data
		map<string, qrz_api_data*>* api_data_;
		// Upload thread
		thread* th_upload_;
		// Allow thread to run
		atomic<bool> run_threads_;
		// Lock between main and upload thread
		mutex upload_lock_;
		// Upload response
		atomic<upload_resp_t*> upload_resp_;
		// Upload queue
		queue<record*> upload_queue_;

	};

#endif



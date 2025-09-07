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
typedef size_t qso_num_t;

struct qsl_call_data;


//! This class provides the interface to the QRZ.com.

//! It handles both XML database look-up - it currently
//! is restricted to looking up details based on callsign. 
//! It also handles interfaceing to the logbook held by
//! QRZ.com.
class qrz_handler
{
public:
	//! Constructor.
	qrz_handler();
	//! Destructor.
	~qrz_handler();

	//! Load data from database of login credentials for QRZ.com.
	void load_data();

	//! Open the QRZ.com session
	bool open_session();
	//! Fetch the contacts details from \p qso recotrd and present the user with a merge opportunity.
	bool fetch_details(record* qso);
	//! Report merge complete.
	void merge_done();
	//! Returns the message presented to user with the merge request.
	string get_merge_message();
	//! Returns data received from QRZ.com for an information request.
	record* get_record();
	//! Retun true if we have a valid QRZ.com query session.
	bool has_session();
	//! Open QRZ.com web page for \p call.
	void open_web_page(string callsign);
	//! Download update to QRZ.com logbook foe station callsign into stream \p adif.
	bool download_qrzlog_log(stringstream* adif);
	//! Upload QSL for QSO with index \p qso_number in full log.
	bool upload_single_qso(qso_num_t qso_number);
	//! Upload QSOs extracted into \p log.
	bool upload_log(book* log);


protected:
	//! Upload result.
	enum insert_resp_t {
		IR_GOOD,                  //!< Successful.
		IR_FAIL,                  //!< Failed: RESULT=FAIL received from QRZ.com.
		IR_DUPLICATE,             //!< Duplicate QSO: RESULT=REPLACE received from QRZ.com.
		IR_ERROR                  //!< Error: Other error if RESULT=OK not received from QRZ.com.
	};
	//! Upload response - details.
	struct upload_resp_t {
		insert_resp_t success = IR_ERROR;  //!< Result.
		string message = "";               //!< Reason for error.
		record* qso = nullptr;             //!< QSO record for which this response is.
		unsigned long long logid = 0ull;   //!< QSO log id as known to QRZ.com (universally unique).
	};
	//! Decode session \p response. 
	bool decode_session_response(istream& response);
	//! Decode details \p response.
	bool decode_details_response(istream& response);
	//! Query user on merge.
	bool query_merge();
	//! Generate URI for session request.
	string generate_session_uri();
	//! Generate URI for details request for \p callsign.
	string generate_details_uri(string callsign);
	//! Fetch user details.
	bool user_details();
	//! Decode re.ceived XML \p element - into a data map.
	bool decode_xml(xml_element* element);
	//! Decode QRZ Database \p element.
	bool decode_top(xml_element* element);
	//! Decode Session \p element.
	bool decode_session(xml_element* element);
	//! Decode Callsign \p element.
	bool decode_callsign(xml_element* element);
	//! Generate fetch request for logbook described in \p api into stream \p request.
	bool fetch_request(qsl_call_data* api, ostream& request);
	//! Decode fetch response for logbook described in \p api from stream \p response.
	
	//! ADIF text is received in \p adif with the \p count records.
	bool fetch_response(qsl_call_data* api, istream& response, int& count, string& adif);
	//! Generate insert request for \p qso using logbook described in \p api into stream \p request.
	bool insert_request(qsl_call_data* api, ostream& request, record* qso);
	//! Decode insert response.
	
	//! \param api QRZ.com logbook credentials.
	//! \param response received data stream.
	//! \param fail_reason Message describing any fail.
	//! \param logid QRZ.com universal QSO identifier.
	insert_resp_t insert_response(qsl_call_data* api, istream& response, 
		string& fail_reason, unsigned long long& logid);
	//! Callback when insert upload complete, sent from upload thread  to main thread.
	static void cb_upload_done(void* v);
	//! Call to upload thread to upload \p qso.
	void th_upload_qso(record* qso);
	//! Run the thread to upload QSOs in the background
	static void thread_run(qrz_handler* that);
	//! Upload done (run in main thread with response \p resp).
	void upload_done(upload_resp_t* resp);

	//! The key returned from the login session attempt
	string session_key_;
	//! User's QRZ.com account name
	string username_;
	//! User's QRZ.com password
	string password_;
	//! Current response data as name=value pairs
	map< string, string> data_;
	//! Current response data as fields of q QSO record.
	record* qrz_info_;
	//! QRZ database version.
	string qrz_version_;
	//! Merge complete.
	bool merge_done_;
	//! Use XML Database.
	bool use_xml_database_;
	//! Non-subscriber to QRZ.com XML features.
	bool non_subscriber_;
	//! Use API to access QRZ.com logbooks.
	bool use_api_;
	//! Upload per QSO.
	bool upload_qso_;
	//! API data: credentials for all user's logbooks on QRZ.com.
	map<string, qsl_call_data*>* api_data_;
	//! Thread in which uploads to QRZ.com are run.
	thread* th_upload_;
	//! Allow thread to run: set false when closing.
	atomic<bool> run_threads_;
	//! Lock between main and upload thread used when passing requests and responses.
	mutex upload_lock_;
	//! Upload response
	atomic<upload_resp_t*> upload_resp_;
	//! Upload request queue
	queue<record*> upload_queue_;

};

#endif



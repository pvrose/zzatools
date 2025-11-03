#ifndef __QRZ_HANDLER__
#define __QRZ_HANDLER__

#include "pugixml.hpp"

#include <string>
#include<istream>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>



class book;
class record;
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
	std::string get_merge_message();
	//! Returns data received from QRZ.com for an information request.
	record* get_record();
	//! Retun true if we have a valid QRZ.com query session.
	bool has_session();
	//! Open QRZ.com web page for \p call.
	void open_web_page(std::string callsign);
	//! Download update to QRZ.com logbook foe station callsign into stream \p adif.
	bool download_qrzlog_log(std::stringstream* adif);
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
		std::string message = "";               //!< Reason for error.
		record* qso = nullptr;             //!< QSO record for which this response is.
		unsigned long long logid = 0ull;   //!< QSO log id as known to QRZ.com (universally unique).
	};
	//! Decode \p response. 
	bool decode_response(std::istream& response);
	//! Query user on merge.
	bool query_merge();
	//! Generate URI for session request.
	std::string generate_session_uri();
	//! Generate URI for details request for \p callsign.
	std::string generate_details_uri(std::string callsign);
	//! Fetch user details.
	bool user_details();
	//! Decode Session \p element.
	bool decode_session(pugi::xml_node node);
	//! Decode Callsign \p element.
	bool decode_callsign(pugi::xml_node node);
	//! Generate fetch request for logbook described in \p api into stream \p request.
	bool fetch_request(qsl_call_data* api, std::ostream& request);
	//! Decode fetch response for logbook described in \p api from stream \p response.
	
	//! ADIF text is received in \p adif with the \p count records.
	bool fetch_response(qsl_call_data* api, std::istream& response, int& count, std::string& adif);
	//! Generate insert request for \p qso using logbook described in \p api into stream \p request.
	bool insert_request(qsl_call_data* api, std::ostream& request, record* qso);
	//! Decode insert response.
	
	//! \param api QRZ.com logbook credentials.
	//! \param response received data stream.
	//! \param fail_reason Message describing any fail.
	//! \param logid QRZ.com universal QSO identifier.
	insert_resp_t insert_response(qsl_call_data* api, std::istream& response, 
		std::string& fail_reason, unsigned long long& logid);
	//! Callback when insert upload complete, sent from upload std::thread  to main std::thread.
	static void cb_upload_done(void* v);
	//! Call to upload std::thread to upload \p qso.
	void th_upload_qso(record* qso);
	//! Run the std::thread to upload QSOs in the background
	static void thread_run(qrz_handler* that);
	//! Upload done (run in main std::thread with response \p resp).
	void upload_done(upload_resp_t* resp);

	//! The key returned from the login session attempt
	std::string session_key_;
	//! User's QRZ.com account name
	std::string username_;
	//! User's QRZ.com password
	std::string password_;
	//! Current response data as fields of q QSO record.
	record* qrz_info_;
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
	std::map<std::string, qsl_call_data*>* api_data_;
	//! Thread in which uploads to QRZ.com are run.
	std::thread* th_upload_;
	//! Allow std::thread to run: std::set false when closing.
	std::atomic<bool> run_threads_;
	//! Lock between main and upload std::thread used when passing requests and responses.
	std::mutex upload_lock_;
	//! Upload response
	std::atomic<upload_resp_t*> upload_resp_;
	//! Upload request std::queue
	std::queue<record*> upload_queue_;

};

#endif



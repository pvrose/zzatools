#include "qrz_handler.h"

#include "status.h"
#include "tabbed_forms.h"
#include "book.h"
#include "url_handler.h"
#include "menu.h"
#include "record.h"
#include "import_data.h"
#include "qso_manager.h"
#include "adi_reader.h"
#include "adi_writer.h"
#include "qsl_dataset.h"

#include "pugixml.hpp"

#include <sstream>
#include <iostream>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>

extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern url_handler* url_handler_;
extern book* navigation_book_;
extern std::string PROG_ID;
extern std::string PROGRAM_VERSION;
extern uint32_t seed_;
extern import_data* import_data_;
extern qso_manager* qso_manager_;
extern book* book_;
extern bool DEBUG_THREADS;
extern qsl_dataset* qsl_dataset_;

// Constructor
qrz_handler::qrz_handler() :
	username_("")
	, password_("")
	, session_key_("")
	, qrz_info_(nullptr)
	, merge_done_(false)
{

	run_threads_ = true;
	if (DEBUG_THREADS) printf("QRZ MAIN: Starting std::thread\n");
	th_upload_ = new std::thread(thread_run, this);

	// Got no log-in details - raise a warning for now.
	if (!user_details()) {
		status_->misc_status(ST_WARNING, "QRZ: login details have not been established yet. Do so before first use");
	}
	if (!url_handler_) {
		url_handler_ = new url_handler();
	}
}

// Destructor
qrz_handler::~qrz_handler() {
	// Close the upload std::thread
	run_threads_ = false;
	th_upload_->join();
	delete th_upload_;
	// Delete dynamic data
	delete qrz_info_;
}

// Open the QRZ session
bool qrz_handler::open_session() {
	// We are unable to log-in to QRZ
	if (!user_details()) {
		status_->misc_status(ST_ERROR, "QRZ: login details are not available");
		return false;
	}
	// We can use the XML database - note this is subscription only
	if (use_xml_database_) {
		// Generate XML request.
		std::string uri = generate_session_uri();
		std::stringstream response;
		// Send Session requesst to QRZ
		if (!url_handler_->read_url(uri, &response)) {
			int length = uri.length() + 50;
			char* message = new char[length];
			snprintf(message, length, "QRZ: Failed to access %s", uri.c_str());
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		session_key_ = "";
		if (decode_response(response)) {
			if (non_subscriber_) {
				// User is not a subscriber - ask whether to continue
				fl_beep(FL_BEEP_QUESTION);
				if (fl_choice("You are not a subscriber to the QRZ XML Database so will only receive limited info.\n"
					"Do you wish to continue or use the standard web page?", "Continue", "Use Web Page", nullptr) == 1) {
					use_xml_database_ = false;
					server_data_t* qrz_data = qsl_dataset_->get_server_data("QRZ");
					qrz_data->use_xml = false;
					return false;
				}
			}
			status_->misc_status(ST_OK, "QRZ: session started");
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

// Fetch the contacts details and present the user with a merge opportunity
bool qrz_handler::fetch_details(record* qso) {
	if (!user_details()) {
		status_->misc_status(ST_ERROR, "QRZ: login details are not available");
		return false;
	}
	if (!session_key_.length()) {
		status_->misc_status(ST_ERROR, "QRZ: session has not been estalished");
		return false;
	}
	// Request 
	status_->misc_status(ST_NOTE, std::string("QRZ: Fetching information for " + qso->item("CALL")).c_str());
	std::string uri = generate_details_uri(qso->item("CALL"));
	std::stringstream response;
	// Send Session requesst to QRZ
	if (!url_handler_->read_url(uri, &response)) {
		int length = uri.length() + 50;
		char* message = new char[length];
		snprintf(message, length, "QRZ: Failed to access %s", uri.c_str());
		status_->misc_status(ST_ERROR, message);
	}
	// Create a new record
	delete qrz_info_;
	qrz_info_ = new record;
	if (decode_response(response)) {
		status_->misc_status(ST_OK, std::string("QRZ: query for " + qso->item("CALL") + " completed.").c_str());
		return true;
	}
	else {
		return false;
	}
}

// Decode session response
bool qrz_handler::decode_response(std::istream& response) {
	char msg[128];
	pugi::xml_document doc;
	doc.load(response);

	pugi::xml_node n_top = doc.document_element();
	if (strcmp(n_top.name(), "QRZDatabase") != 0) {
		status_->misc_status(ST_ERROR, "QRZ: Invalid response from QRZ.com");
		return false;
	}

	for (auto n_child : n_top.children()) {
		if (strcmp(n_child.name(), "Session") == 0) {
			decode_session(n_child);
		}
		else if (strcmp(n_child.name(), "Callsign") == 0) {
			decode_callsign(n_child);
		}
		else {
			std::snprintf(msg, sizeof(msg), "QRZ: Response item %s ignored",
				n_child.name());
			status_->misc_status(ST_NOTE, msg);
		}
	}
	return true;
}

// Query user on merge
bool qrz_handler::query_merge() {
	merge_done_ = false;
	// Get the record view to merge data
	qso_manager_->merge_qrz_com();
	while (!merge_done_) Fl::check();
	status_->misc_status(ST_OK, "QRZ: Update complete");
	return false;
}

// Generate URI for session request
std::string qrz_handler::generate_session_uri() {
	std::string result;
	char format[] = "http://xmldata.qrz.com/xml/current/?username=%s;password=%s;agent=%s%s";
	int length = strlen(format) + username_.length() + password_.length() + PROG_ID.length() + PROGRAM_VERSION.length();
	char* uri = new char[length];
	snprintf(uri, length, format, username_.c_str(), password_.c_str(), PROG_ID.c_str(), PROGRAM_VERSION.c_str());
	result = uri;
	return result;
}

// Generate URI for details request
std::string qrz_handler::generate_details_uri(std::string callsign) {
	std::string result;
	char format[] = "http://xmldata.qrz.com/xml/current/?s=%s;callsign=%s";
	int length = strlen(format) + session_key_.length() + callsign.length();
	char* uri = new char[length];
	snprintf(uri, length, format, session_key_.c_str(), callsign.c_str());
	result = uri;
	return result;
}

// Fetch user details - returns false if not enabled or whether username or password are blank strings
bool qrz_handler::user_details() {
	server_data_t* qrz_data = qsl_dataset_->get_server_data("QRZ");
	bool enabled = qrz_data->use_xml;
	if (enabled) {
		username_ = qrz_data->user;
		password_ = qrz_data->password;
		use_xml_database_ = qrz_data->use_xml;
		if (!username_.length() || !password_.length()) {
			return false;
		}
		else {
			return true;
		}
	}
	else {
		return false;
	}
}

// Decode Session element
bool qrz_handler::decode_session(pugi::xml_node node) {
	char msg[128];
	for (auto datum : node.children()) {
		if (strcmp(datum.name(), "Key") == 0) {
			std::string key = datum.text().as_string();
			if (session_key_.length() && session_key_ != key) {
				status_->misc_status(ST_ERROR, "QRZ: Incompatible session keys received - see log");
				status_->misc_status(ST_LOG, std::string("QRZ: Previously " + session_key_).c_str());
				status_->misc_status(ST_LOG, std::string("QRZ: Currently  " + key).c_str());
				return false;
			}
			else {
				session_key_ = key;
			}
		}
		// Error detected - stop decoding
		if (strcmp(datum.name(), "Error") == 0) {
			std::snprintf(msg, sizeof(msg), "QRZ: Error: %s", datum.text().as_string());
			status_->misc_status(ST_ERROR, msg);
			return false;
		}
		// Warning message received
		else if (strcmp(datum.name(), "Message") ==0) {
			std::snprintf(msg, sizeof(msg), "QRZ: Warning: %s", datum.text().as_string());
			status_->misc_status(ST_WARNING, msg);
		}
		// Check subscription status
		else if (strcmp(datum.name(), "SubExp") ==0) {
			if (strcmp(datum.text().as_string(), "non-subscriber") == 0) {
				status_->misc_status(ST_WARNING, "QRZ: Not a QRZ.com subscriber");	
				non_subscriber_ = true;
			}
			else {
				std::snprintf(msg, sizeof(msg), "QRZ: QRZ Subscription expires %s",
					datum.text().as_string());
				status_->misc_status(ST_NOTE, msg);
				non_subscriber_ = false;
			}
		}
	}
	return true;
}

// Decode Callsign element
bool qrz_handler::decode_callsign(pugi::xml_node node) {
	// Get all the children
	bool valid_coords = false;
	for (auto datum : node.children()) {
		const char* name = datum.name();
		std::string value = datum.text().as_string();
		if (strcmp(name, "call") == 0) {
			qrz_info_->item("CALL", value);
		}
		else if (strcmp(name, "dxcc") == 0) {
			qrz_info_->item("DXCC", value);
		}
		else if (strcmp(name, "fname") == 0) {
			qrz_info_->item("NAME", value);
		}
		else if (strcmp(name, "addr1") == 0) {
			qrz_info_->item("ADDRESS", value);
		}
		else if (strcmp(name, "addr2") == 0) {
			qrz_info_->item("QTH", value);
		}
		else if (strcmp(name, "state") == 0) {
			qrz_info_->item("STATE", value);
		}
		else if (strcmp(name, "lat") == 0) {
			qrz_info_->item("LAT", value);
		}
		else if (strcmp(name, "lon") == 0) {
			qrz_info_->item("LON", value);
		}
		else if (strcmp(name, "grid") == 0) {
			qrz_info_->item("GRIDSQUARE", value);
		}
		else if (strcmp(name, "geoloc") == 0) {
			if (value == "user" || value == "geocode") {
				valid_coords = true;
			}
			else {
				valid_coords = false;
			}
		}
		else if (strcmp(name, "fips") == 0) {
			qrz_info_->item("CNTY", value);
		}
		else if (strcmp(name, "land") == 0) {
			qrz_info_->item("COUNTRY", value);
		}
		else if (strcmp(name, "email") == 0) {
			qrz_info_->item("EMAIL", value);
		}
		else if (strcmp(name, "url") == 0) {
			qrz_info_->item("WEB", value);
		}
		else if (strcmp(name, "iota") == 0) {
			qrz_info_->item("IOTA", value);
		}
		else if (strcmp(name, "cqzone") == 0) {
			qrz_info_->item("CQZ", value);
		}
		else if (strcmp(name, "ituzone") == 0) {
			qrz_info_->item("ITUZ", value);
		}
	}
	if (!valid_coords) {
		qrz_info_->item("LAT", (string)"");
		qrz_info_->item("LON", (string)"");
	}
	return true;
}

// Callback to say done
void qrz_handler::merge_done() {
	merge_done_ = true;
}

// REturn message
std::string qrz_handler::get_merge_message() {
	return "Please merge in data downloaded from QRZ database";
}

// Get merge data
record* qrz_handler::get_record() {
	return qrz_info_;
}

// Do we have a session
bool qrz_handler::has_session() {
	if (session_key_.length()) {
		return true;
	}
	else {
		return false;
	}
}

// Non-subscriber access to QRZ.com web interface
void qrz_handler::open_web_page(std::string callsign) {
	// Open browser with QRZ URL 
	char uri[256];
	snprintf(uri, sizeof(uri), "http://www.qrz.com/lookup?callsign=%s", callsign.c_str());
	status_->misc_status(ST_NOTE, std::string("QRZ: Launching QRZ web page for " + callsign).c_str());
	fl_open_uri(uri);
}

// Load the call_data from settings
void qrz_handler::load_data() {
	server_data_t* qrz_data = qsl_dataset_->get_server_data("QRZ");
	use_api_ = qrz_data->use_api;
	upload_qso_ = qrz_data->upload_per_qso;
	api_data_ = &qrz_data->call_data;
}

// Request download for callsign
bool qrz_handler::download_qrzlog_log(std::stringstream* adif) {
	char msg[128];
	load_data();
	if (!use_api_) {
		strcpy(msg, "QRZ: The API interface is not-supported");
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	std::string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
	if (api_data_->find(callsign) == api_data_->end() ||
		!api_data_->at(callsign)->used) {
		snprintf(msg, sizeof(msg), "QRZ: No logbook exists for call %s", callsign.c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	int count = 0;
	// Create a 1M holding std::string (for debug)
	std::string sadif;
	sadif.reserve(1024 * 1024);
	std::stringstream request;
	std::stringstream response;
	fetch_request(api_data_->at(callsign), request);
	url_handler_->post_url("https://logbook.qrz.com/api", "", &request, &response);
	response.seekg(0, std::ios::beg);
	if (!fetch_response(api_data_->at(callsign), response, count, sadif)) {
		status_->misc_status(ST_WARNING, "QRZ: Fetch data over API has failed");
	} else {
		snprintf(msg, sizeof(msg), "QRZ: Fetch finished %d records downloaded!",
				count);
		status_->misc_status(ST_NOTE, msg);
		// Copy the first page of records to the stream
		*adif << sadif;
	}
	if (count > 0) {
		return true;
	} else {
		return false;
	}
}

// Generate the fetch request
bool qrz_handler::fetch_request(qsl_call_data* api, std::ostream& req) {
	// Key
	req << "KEY=" << api->key << "&";
	// Action
	req << "ACTION=FETCH" <<"&";
	// Options
	std::string lastdate = api->last_download.substr(0,4) + "-" +
		api->last_download.substr(4,2) + "-" + api->last_download.substr(6,2);
 	req << "OPTION=MODSINCE:" << lastdate << '\n';
	return true;
}

// Decode the fetch response and copy to import_data
bool qrz_handler::fetch_response(qsl_call_data* api, std::istream& resp, int& count, std::string& adif) {
	bool ok = true;
	char msg[128];
	bool done = false;
	bool got_count = false;
	bool got_result = false;
	bool got_adif = false;
	bool failed = false;
	enum { RESULT, COUNT, ADIF } state;
	bool is_logid = false;
	int count_logid = 0;
	bool eod = false; // end of data
	int count_records = 0;
	std::string buffer;
	std::string buff1;
	size_t pos = 0;
	unsigned long long last_logid = api->last_logid;
	// Read the entire stream into buffer
	while (!resp.eof()) {
		getline(resp, buff1, '\0');
		buffer += buff1;
	}
	while (!done && ok) {
		// Get the next bit of data
		size_t amper = buffer.find('&', pos);
		std::string data;
		if (amper == std::string::npos) {
			data = buffer.substr(pos);
			eod = true;
		} else {
			data = buffer.substr(pos, amper - pos);
		}
		// Read until next ampersand (or EOF)
		int len = data.length();
		if (len > 7 && data.substr(0, 7) == "RESULT=") {
			state = RESULT;
			got_result = true;
			if (data.substr(7) == "FAIL") {
				status_->misc_status(ST_WARNING, "QRZ: Download failed - probably no data available");
				failed = true;
			}
			else if (data.substr(7) != "OK") {
				snprintf(msg, sizeof(msg), "QRZ: Invalid - %s", data.c_str());
				status_->misc_status(ST_ERROR, msg);
			}
		} else if (len > 6 && data.substr(0,6) == "COUNT=") {
			state = COUNT;
			got_count = true;
			count = std::stoi(data.substr(6));
		} else if (len >= 5 && data.substr(0,5) == "ADIF=") {
			state = ADIF;
			got_adif = true;
			if (len > 5) {
				adif += data.substr(5);
			}
		} else if (state == ADIF && len > 3 && data.substr(0,3) == "lt;") {
			// esacped '<'
			adif += '<';
			if (len > 3) {
				adif += data.substr(3);
			}
			if (len > 20 && data.substr(3, 17) == "app_qrzlog_logid:") {
				// Special processing - required after next '>'
				is_logid = true;
				count_logid = std::stoi(data.substr(20));
			}
		} else if (state == ADIF && len > 3 && data.substr(0,3) == "gt;") {
			if (is_logid) {
				// Special processing for APP_QRZLOG_LOGID - update last_logid
				unsigned long long logid = stoull(data.substr(3, count_logid));
				if (logid > last_logid) last_logid = logid;
				is_logid = false;
				count_records++;
			}
			adif += '>';
			if (len > 3) {
				adif += data.substr(3);
			}
		} else if (state == ADIF) {
			// Copy ampersand and data
			adif += '&';
			adif += data;
		} else if (eod) {
			// The ADIF will tail off
			if (state == ADIF) adif += data;
		} else {
			snprintf(msg, sizeof(msg), "QRZ: Unexpected '&' followed by %s", data.c_str());
			status_->misc_status(ST_ERROR, msg);
			ok = false;
		} 
		if (ok && eod) {
			if (got_result || got_count || got_adif) {
				done = true;
			} else {
				ok = false;
			}
			if (!got_result) 
				status_->misc_status(ST_ERROR, "QRZ: EOF reached with no RESULT=");
			if (!got_count) 
				status_->misc_status(ST_ERROR, "QRZ: EOF reached with no COUNT=");
			if (!got_adif && !failed) 
				status_->misc_status(ST_ERROR, "QRZ: EOF reached with no ADIF=");
		}
		if (!got_count) count = count_records;
		pos = amper + 1;
	}
	if (ok)  {
		std::string today = now(false, "%Y%m%d");
		api->last_download = today;
		api->last_logid = last_logid;
	}
	return ok;
}

bool qrz_handler::upload_single_qso(qso_num_t qso_number) {
	char msg[128];
	load_data();
	// Check whether we can upload if so if we need to
	if (!use_api_) {
		strcpy(msg, "QRZ: The API interface is not-supported");
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	if (!upload_qso_) {
		strcpy(msg, "QRZ: Uploading per QSO is disabled");
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	record* qso = book_->get_record(qso_number, false);
	std::string sent_status = qso->item("QRZCOM_QSO_UPLOAD_STATUS");
	if (sent_status == "Y") {
		strcpy(msg, "QRZ: Already uploaded this QSO - not uploading");
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	if (sent_status == "N") {
		strcpy(msg, "QRZ: QSO marked not for update to QRZ.com - not uploading");
		status_->misc_status(ST_NOTE, msg);
		return false;
	}
	std::string callsign = qso->item("STATION_CALLSIGN");
	if (api_data_->find(callsign) == api_data_->end() ||
		!api_data_->at(callsign)->used) {
		snprintf(msg, sizeof(msg), "QRZ: No logbook exists for call %s", callsign.c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	book_->enable_save(false, "Uploading to QRZ.com");
	// Now send to upload std::thread to process
	upload_lock_.lock();
	if (DEBUG_THREADS) printf("EQSL MAIN: Enqueueing eQSL request %s\n", qso->item("CALL").c_str());
	upload_queue_.push(qso);
	upload_lock_.unlock();
	return true;
}

bool qrz_handler::upload_log(book* log) {
	bool ok = true;
	for (item_num_t ix = 0; ix < log->size() && ok; ix++) {
		ok = upload_single_qso(log->record_number(ix));
		if (!ok) {
			status_->misc_status(ST_WARNING, "QRZ: IUpload log failed - see above message");
		}
	}
	return ok;
}

// Upload std::thread - sit in a loop waiting for upload requests
void qrz_handler::thread_run(qrz_handler* that) {
	if (DEBUG_THREADS) printf("QRZ THREAD: Thread started\n");
	while (that->run_threads_) {
		// Wait until qso placed on interface
		while (that->run_threads_ && that->upload_queue_.empty()) {
			this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		// Process it
		that->upload_lock_.lock();
		if (!that->upload_queue_.empty()) {
			record* qso = that->upload_queue_.front();
			that->upload_queue_.pop();
			if (DEBUG_THREADS) printf("QRZ THREAD: Received request %s\n", qso->item("CALL").c_str());
			that->upload_lock_.unlock();
			that->th_upload_qso(qso);
		}
		else {
			that->upload_lock_.unlock();
		}
		this_thread::yield();
	}
}

// Upload a single QSO in th ethead
void qrz_handler::th_upload_qso(record* qso) {
	std::stringstream request;
	std::stringstream response;
	std::string callsign = qso->item("STATION_CALLSIGN");
	qsl_call_data* call_data = api_data_->at(callsign);
	std::string fail_message;
	insert_request(call_data, request, qso);
	url_handler_->post_url("https://logbook.qrz.com/api", "", &request, &response);
	response.seekg(0, std::ios::beg);
	upload_resp_t* resp = new upload_resp_t;
	resp->success = insert_response(call_data, response, resp->message, resp->logid);
	resp->qso = qso;
	// Send response back to 
	upload_resp_ = resp;
	if (DEBUG_THREADS) printf("QRZ THREAD: Calling std::thread callback\n");
	Fl::awake(cb_upload_done, (void*)this);
	this_thread::yield();
}

// Upload done: wrapper
void qrz_handler::cb_upload_done(void* v) {
	if (DEBUG_THREADS) printf("QRZ MAIN: Entered std::thread callback handler\n");
	qrz_handler* that = (qrz_handler*)v;
	that->upload_done(that->upload_resp_);
}

// Upload done - main functionality
void qrz_handler::upload_done(upload_resp_t* resp) {
	char msg[128];
	switch(resp->success) {
		case IR_GOOD:
		resp->qso->item("QRZCOM_QSO_UPLOAD_STATUS", std::string("Y"));
		resp->qso->item("QRZCOM_QSO_UPLOAD_DATE", now(false, "%Y%m%d"));
		resp->qso->item("APP_QRZLOG_LOGID", to_string(resp->logid));
		snprintf(msg, sizeof(msg), "QRZ: %s %s %s QSL uploaded (logid=%llu)",
			resp->qso->item("QSO_DATE").c_str(),
			resp->qso->item("TIME_ON").c_str(),
			resp->qso->item("CALL").c_str(),
			resp->logid);
		status_->misc_status(ST_OK, msg);
		book_->enable_save(true, "Uploaded to QRZ.com");
		break;
	case IR_DUPLICATE:
		resp->qso->item("QRZCOM_QSO_UPLOAD_STATUS", std::string("Y"));
		resp->qso->item("QRZCOM_QSO_UPLOAD_DATE", now(false, "%Y%m%d"));
		resp->qso->item("APP_QRZLOG_LOGID", to_string(resp->logid));
		snprintf(msg, sizeof(msg), "QRZ: %s %s %s QSL uploaded (duplicate logid=%llu)",
			resp->qso->item("QSO_DATE").c_str(),
			resp->qso->item("TIME_ON").c_str(),
			resp->qso->item("CALL").c_str(),
			resp->logid);
		status_->misc_status(ST_WARNING, msg);
		book_->enable_save(true, "Uploaded to QRZ.com");
		break;
	case IR_FAIL:
		snprintf(msg, sizeof(msg), "QRZ: QSL upload failed - %s", resp->message.c_str());
		status_->misc_status(ST_ERROR, msg);
		break;
	case IR_ERROR:
		snprintf(msg, sizeof(msg), "QRZ: QSL upload protocol error %s", resp->message.c_str());
		status_->misc_status(ST_ERROR, msg);
		break;
	}
}

// Generate insert request
bool qrz_handler::insert_request(qsl_call_data* api, std::ostream& request, record* qso) {
	// Key
	request << "KEY=" << api->key << "&";
	// Action insert
	request << "ACTION=INSERT&";
	// Option Replace
	request << "OPTION=REPLACE&";
	// aDIF
	request << "ADIF=";
	adi_writer::to_adif(qso, request);
	return true;
}

// Decode insert response
qrz_handler::insert_resp_t qrz_handler::insert_response(
	qsl_call_data* api, 
	std::istream& response, 
	std::string& fail_reason,
	unsigned long long& logid
) {
	std::string buffer;
	std::string buff1;
	bool done = false;
	insert_resp_t resp = IR_GOOD;
	size_t pos = 0;
	// Read the entire stream into buffer
	while (!response.eof()) {
		getline(response, buff1, '\0');
		buffer += buff1;
	}
	while (!done) {
		// Get the next bit of data
		size_t amper = buffer.find('&', pos);
		std::string data;
		if (amper == std::string::npos) {
			data = buffer.substr(pos);
			done = true;
		} else {
			data = buffer.substr(pos, amper - pos);
		}
		// Read until next ampersand (or EOF)
		int len = data.length();
		if (len > 7 && data.substr(0, 7) == "RESULT=") {
			if (data.substr(7) == "FAIL") {
				resp = IR_FAIL;
			} else if (data.substr(7) == "REPLACE") {
				resp = IR_DUPLICATE;
			} else if (data.substr(7) != "OK") {
				resp = IR_ERROR;
				fail_reason += "Error RESULT=" + data.substr(7) + " ";
			}
		} else if (len > 6 && data.substr(0,6) == "COUNT=") {
			if (data.substr(6) != "0" && data.substr(6) != "1") {
				resp = IR_ERROR;
				fail_reason += "Error COUNT=" + data.substr(7) + " "
;			} 
		} else if (len > 6 && data.substr(0,6) == "LOGID=") {
			logid = stoull(data.substr(6));
		} else if (len > 7 && data.substr(0,7) == "REASON=") {
			fail_reason = data.substr(7);
		}
		pos = amper + 1;
	}
	return resp;
}
#include "qrz_handler.h"

#include "status.h"
#include "tabbed_forms.h"
#include "book.h"
#include "url_handler.h"
#include "xml_reader.h"
#include "xml_element.h"
#include "menu.h"
#include "record.h"
#include "import_data.h"
#include "qso_manager.h"
#include "adi_reader.h"

#include <sstream>
#include <iostream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Help_Dialog.H>




extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern url_handler* url_handler_;
extern book* navigation_book_;
extern menu* menu_;
extern Fl_Preferences* settings_;
extern string PROG_ID;
extern string PROGRAM_VERSION;
extern uint32_t seed_;
extern import_data* import_data_;
extern qso_manager* qso_manager_;

// Constructor
qrz_handler::qrz_handler() :
	username_("")
	, password_("")
	, session_key_("")
	, qrz_info_(nullptr)
	, qrz_version_("")
	, merge_done_(false)
{
	// Got no log-in details - raise a warning for now.
	if (!user_details()) {
		status_->misc_status(ST_WARNING, "QRZ: login details have not been established yet. Do so before first use");
	}
	if (!url_handler_) {
		url_handler_ = new url_handler();
	}
	web_dialog_ = new Fl_Help_Dialog;
	load_data();
	data_.clear();
}

// Destructor
qrz_handler::~qrz_handler() {
	delete qrz_info_;
	data_.clear();
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
		string uri = generate_session_uri();
		stringstream response;
		// Send Session requesst to QRZ
		if (!url_handler_->read_url(uri, &response)) {
			int length = uri.length() + 50;
			char* message = new char[length];
			snprintf(message, length, "QRZ: Failed to access %s", uri.c_str());
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		session_key_ = "";
		if (decode_session_response(response)) {
			if (non_subscriber_) {
				// User is not a subscriber - ask whether to continue
				fl_beep(FL_BEEP_QUESTION);
				if (fl_choice("You are not a subscriber to the QRZ XML Database so will only receive limited info.\n"
					"Do you wish to continue or use the standard web page?", "Continue", "Use Web Page", nullptr) == 1) {
					use_xml_database_ = false;
					Fl_Preferences qrz_settings(settings_, "QRZ");
					qrz_settings.set("Use XML Database", false);
					settings_->flush();
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
bool qrz_handler::fetch_details() {
	if (!user_details()) {
		status_->misc_status(ST_ERROR, "QRZ: login details are not available");
		return false;
	}
	if (!session_key_.length()) {
		status_->misc_status(ST_ERROR, "QRZ: session has not been estalished");
		return false;
	}
	// Request 
	record* query = navigation_book_->get_record();
	status_->misc_status(ST_NOTE, string("QRZ: Fetching information for " + query->item("CALL")).c_str());
	string uri = generate_details_uri(query->item("CALL"));
	stringstream response;
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
	if (decode_details_response(response) && 
		query_merge()) {
		status_->misc_status(ST_OK, string("QRZ: query for " + query->item("CALL") + " completed.").c_str());
		return true;
	}
	else {
		return false;
	}
}

// Decode session response
bool qrz_handler::decode_session_response(istream& response) {
	qrz_version_ = "";
	xml_reader* reader = new xml_reader;
	reader->parse(response);
	xml_element* top_element = reader->element();
	bool result = decode_xml(top_element);
	delete top_element;
	return result;
}

// Decode details response
bool qrz_handler::decode_details_response(istream& response) {
	xml_reader* reader = new xml_reader;
	reader->parse(response);
	xml_element* top_element = reader->element();
	bool result = decode_xml(top_element);
	delete top_element;
	return result;
}

// Query user on merge
bool qrz_handler::query_merge() {
	merge_done_ = false;
	// Get the record view to merge data
	navigation_book_->selection(-1, HT_MERGE_DETAILS);
	while (!merge_done_) Fl::check();
	status_->misc_status(ST_OK, "QRZ: Update complete");
	return false;
}

// Generate URI for session request
string qrz_handler::generate_session_uri() {
	string result;
	char format[] = "http://xmldata.qrz.com/xml/current/?username=%s;password=%s;agent=%s%s";
	int length = strlen(format) + username_.length() + password_.length() + PROG_ID.length() + PROGRAM_VERSION.length();
	char* uri = new char[length];
	snprintf(uri, length, format, username_.c_str(), password_.c_str(), PROG_ID.c_str(), PROGRAM_VERSION.c_str());
	result = uri;
	return result;
}

// Generate URI for details request
string qrz_handler::generate_details_uri(string callsign) {
	string result;
	char format[] = "http://xmldata.qrz.com/xml/current/?s=%s;callsign=%s";
	int length = strlen(format) + session_key_.length() + callsign.length();
	char* uri = new char[length];
	snprintf(uri, length, format, session_key_.c_str(), callsign.c_str());
	result = uri;
	return result;
}

// Fetch user details - returns false if not enabled or whether username or password are blank strings
bool qrz_handler::user_details() {
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences qrz_settings(qsl_settings, "QRZ");
	int enabled;
	qrz_settings.get("Enable", enabled, false);
	if (enabled) {
		char* temp;
		qrz_settings.get("User", temp, "");
		username_ = temp;
		free(temp);
		qrz_settings.get("Password", temp, "");
		password_ = temp;
		qrz_settings.get("Use XML Database", (int&)use_xml_database_, false);
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
// Decode an XML element
bool qrz_handler::decode_xml(xml_element* element) {
	if (element->parent() == nullptr) {
		// Top element - must be named QRZDatabase
		if (element->name() != "QRZDatabase") {
			string message = "QRZ: unexpected XML " + element->name() + " received";
			status_->misc_status(ST_ERROR, message.c_str());
			return false;
		}
		else {
			return decode_top(element);
		}
	}
	else if (element->name() == "Session") {
		return decode_session(element);
	}
	else if (element->name() == "Callsign") {
		return decode_callsign(element);
	}
	else {
		data_[element->name()] = element->content();
		return true;
	}
}

	// Decode QRZDatabase element
bool qrz_handler::decode_top(xml_element* element) {
	// Check the QRZ Database version matches that previously received
	map <string, string>* attributes = element->attributes();
	if (attributes->find("version") != attributes->end()) {
		string version = attributes->at("version");
		if (qrz_version_.length() && qrz_version_ != version) {
			char format[] = "QRZ: database version mis-match: previously %s currently %s";
			int length = strlen(format) + version.length() + qrz_version_.length();
			char* message = new char[length];
			snprintf(message, length, format, qrz_version_.c_str(), version.c_str());
			status_->misc_status(ST_WARNING, message);
		}
	}
	// now decode all the children
	bool result = true;
	for (int i = 0; i < element->count() && result; i++) {
		result &= decode_xml(element->child(i));
	}
	return result;
}

// Decode Session element
bool qrz_handler::decode_session(xml_element* element) {
	data_.clear();
	// Get all the children - loaded ito data_
	bool result = true;
	for (int i = 0; i < element->count() && result; i++) {
		result &= decode_xml(element->child(i));
	}
	if (!result) return false;
	// Check the key is as received previously
	if (data_.find("Key") != data_.end()) {
		string key = data_.at("Key");
		if (session_key_.length() && session_key_ != key) {
			status_->misc_status(ST_ERROR, "QRZ: Incompatible session keys received - see log");
			status_->misc_status(ST_LOG, string("QRZ: Previously " + session_key_).c_str());
			status_->misc_status(ST_LOG, string("QRZ: Currently  " + key).c_str());
			return false;
		}
		else {
			session_key_ = key;
		}
	}
	// Error detected - stop decoding
	if (data_.find("Error") != data_.end()) {
		status_->misc_status(ST_ERROR, string("QRZ: error: " + data_.at("Error")).c_str());
		return false;
	}
	// Warning message received
	if (data_.find("Message") != data_.end()) {
		status_->misc_status(ST_WARNING, string("QRZ: warning: " + data_.at("Message")).c_str());
	}
	// Check subscription status
	if (data_.find("SubExp") != data_.end() && data_["SubExp"] == "non-subscriber") {
		non_subscriber_ = true;
	}
	else {
		non_subscriber_ = false;
	}
	return true;
}

// Decode Callsign element
bool qrz_handler::decode_callsign(xml_element* element) {
	data_.clear();
	// Get all the children - loaded ito data_
	bool result = true;
	for (int i = 0; i < element->count() && result; i++) {
		result &= decode_xml(element->child(i));
	}
	if (!result) return false;
	// now for each received element in turn
	// call->CALL
	qrz_info_->item("CALL", data_["call"]);
	// dxcc->DXCC
	qrz_info_->item("DXCC", data_["dxcc"]);
	// fname ->NAME
	qrz_info_->item("NAME", data_["fname"]);
	// addr1->ADDRESS
	qrz_info_->item("ADDRESS", data_["addr1"]);
	// addr2->QTH
	qrz_info_->item("QTH", data_["addr2"]);
	// state->STATE
	qrz_info_->item("STATE", data_["state"]);
	// lon, lat and gridsquare
	if (data_["geoloc"] == "user" || data_["geoloc"] == "geocode") {
		// Convert lat/lon to ADIF format
		qrz_info_->item("LAT", data_["LAT"], true);
		qrz_info_->item("LON", data_["LON"], true);
		qrz_info_->item("GRIDSQUARE", data_["grid"]);
	}
	else if (data_["geoloc"] == "grid") {
		qrz_info_->item("GRIDSQUARE", data_["grid"]);
	}
	// county
	qrz_info_->item("CNTY", data_["state"] + ',' + data_["county"]);
	// land->COUNTRY
	qrz_info_->item("COUNTRY", data_["land"]);
	// qslmgr->QSL_VIA
	qrz_info_->item("QSL_VIA", data_["qslmgr"]);
	// email->EMAIL
	qrz_info_->item("EMAIL", data_["email"]);
	// url->WEB
	qrz_info_->item("WEB", data_["url"]);
	// iota->IOTA
	qrz_info_->item("IOTA", data_["iota"]);
	// cqzone
	qrz_info_->item("CQZ", data_["cqzone"]);
	// ituzone
	qrz_info_->item("ITUZ", data_["ituzone"]);
	return true;
}

// Callback to say done
void qrz_handler::merge_done() {
	merge_done_ = true;
}

// REturn message
string qrz_handler::get_merge_message() {
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
void qrz_handler::open_web_page(string callsign) {
	// Open browser with QRZ URL 
	char uri[256];
	snprintf(uri, sizeof(uri), "http://www.qrz.com/lookup?callsign=%s", callsign.c_str());
	status_->misc_status(ST_NOTE, string("QRZ: Launching QRZ web page for " + callsign).c_str());
	fl_open_uri(uri);
}

// Load the api_data from settings
void qrz_handler::load_data() {
	Fl_Preferences qrz_settings(settings_, "QSL/QRZ");
	int itemp; // For bool settings
	char* stemp; // For Cstring settings
	char* btemp; // For binary settings - encrypted string
	unsigned long long uultemp; // For uint64 (as binary)
	qrz_settings.get("Use API", itemp, false);
	use_api_ = itemp;
	Fl_Preferences api_settings(qrz_settings, "Logbooks");
	uchar offset = hash8(api_settings.path());
	int num_groups = api_settings.groups();
	for (int ix = 0; ix < num_groups; ix++) {
		string call = api_settings.group(ix);
		Fl_Preferences call_settings(api_settings, call.c_str());
		qrz_api_data* data = new qrz_api_data;
		call_settings.get("Key Length", itemp, 128);
		btemp = new char[itemp];
		call_settings.get("Key", btemp, (void*)"", 0, &itemp);
		string crypt = string(btemp, itemp);
		string key = xor_crypt(crypt, seed_, offset);
		delete[] btemp;
		data->key = key;
		call_settings.get("Last Log ID", (void*)&data->last_logid, (void*)&uultemp, sizeof(uint64_t), sizeof(uint64_t));
		call_settings.get("Last Download", stemp, "");
		data->last_download = stemp;
		free(stemp);
		re_slash(call);
		api_data_[call] = data;
		call_settings.get("In Use", itemp, false);
		data->used = itemp;
	}
}

// Store the api_data to settings
void qrz_handler::store_data() {
	Fl_Preferences api_settings(settings_, "QSL/QRZ/Logbooks");
	uchar offset = hash8(api_settings.path());
	for (auto it = api_data_.begin(); it != api_data_.end(); it++) {
		string call = it->first;
		de_slash(call);
		Fl_Preferences call_settings(api_settings, call.c_str());
		string encrypt = xor_crypt(it->second->key, seed_, offset);
		call_settings.set("Key Length", (int)encrypt.length());
		call_settings.set("Key", (void*)encrypt.c_str(), encrypt.length());
		call_settings.set("Last Log ID", (void*)&it->second->last_logid, sizeof(uint64_t));
		call_settings.set("Last Download", it->second->last_download.c_str());
		call_settings.set("In Use", (int)it->second->used);
	}
}

// Request download for callsign
bool qrz_handler::download_qrzlog_log(stringstream* adif) {
	char msg[128];
	if (!use_api_) {
		strcpy(msg, "QRZ: The API interface is not-supported");
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
	if (api_data_.find(callsign) == api_data_.end() ||
		!api_data_.at(callsign)->used) {
		snprintf(msg, sizeof(msg), "QRZ: No logbook exists for call %s", callsign.c_str());
		status_->misc_status(ST_WARNING, msg);
		return false;
	}
	bool done = false;
	const int MAX_PER_REQUEST = 200;
	const int REQUESTS = 1;
	int count = 0;
	int total = 0;
	int request_count = 0;
	bool ok = true;
	while(!done && ok) {
		stringstream request;
		stringstream response;
			fetch_request(api_data_.at(callsign), request, MAX_PER_REQUEST);
		url_handler_->post_url("https://logbook.qrz.com/api", "", &request, &response);
		response.seekg(0, ios::beg);
		if (!fetch_response(api_data_.at(callsign), response, count, *adif)) {
			status_->misc_status(ST_WARNING, "QRZ: Fetch data over API has failed");
			ok = false;
		} else {
			total += count;
			request_count++;
			if (count != MAX_PER_REQUEST || request_count == REQUESTS) {
				snprintf(msg, sizeof(msg), "QRZ: Fetch finished %d records downloaded in %d pages!",
					total, request_count);
				status_->misc_status(ST_NOTE, msg);
				done = true;
			} else {
				snprintf(msg, sizeof(msg), "QRZ: Fetch continuing %d downloaded - last logid %u", 
					total, api_data_.at(callsign)->last_logid);
				status_->misc_status(ST_LOG, msg);
			}
		}
	}
	if (total > 0) {
		// Write back the API data to settings
		store_data();
		return true;
	} else {
		return false;
	}
}

// Generate the fetch request
bool qrz_handler::fetch_request(qrz_api_data* api, ostream& req, int count) {
	// Key
	req << "KEY=" << api->key << "&";
	// Action
	req << "ACTION=FETCH" <<"&";
	// Options
	string lastdate = api->last_download.substr(0,4) + "-" +
		api->last_download.substr(4,2) + "-" + api->last_download.substr(6,2);
 	req << "OPTION=AFTERLOGID:" << (api->last_logid + 1ull) << ",";
	req << "MAX:" << count << '\n';
	return true;
}

// Decode the fetch response and copy to import_data
bool qrz_handler::fetch_response(qrz_api_data* api, istream& resp, int& count, stringstream& adif) {
	bool ok = true;
	char msg[128];
	bool done = false;
	bool got_count = false;
	bool got_result = false;
	bool got_adif = false;
	enum { RESULT, COUNT, ADIF } state;
	bool is_logid = false;
	int count_logid = 0;
	bool eod = false; // end of data
	int count_records = 0;
	string buffer;
	string buff1;
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
		string data;
		if (amper == string::npos) {
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
			if (data.substr(7) != "OK") {
				snprintf(msg, sizeof(msg), "QRZ: Invalid - %s", data.c_str());
				status_->misc_status(ST_ERROR, msg);
			}
		} else if (len > 6 && data.substr(0,6) == "COUNT=") {
			state = COUNT;
			got_count = true;
			count = stoi(data.substr(6));
		} else if (len >= 5 && data.substr(0,5) == "ADIF=") {
			state = ADIF;
			got_adif = true;
			if (len > 5) {
				adif << data.substr(5);
			}
		} else if (state == ADIF && len > 3 && data.substr(0,3) == "lt;") {
			// esacped '<'
			adif << '<';
			if (len > 3) {
				adif << data.substr(3);
			}
			if (len > 20 && data.substr(3, 17) == "app_qrzlog_logid:") {
				// Special processing - required after next '>'
				is_logid = true;
				count_logid = stoi(data.substr(20));
			}
		} else if (state == ADIF && len > 3 && data.substr(0,3) == "gt;") {
			if (is_logid) {
				// Special processing for APP_QRZLOG_LOGID - update last_logid
				unsigned long long logid = stoull(data.substr(3, count_logid));
				if (logid > last_logid) last_logid = logid;
				is_logid = false;
				count_records++;
			}
			adif << '>';
			if (len > 3) {
				adif << data.substr(3);
			}
		} else if (state == ADIF) {
			// Copy ampersand and data
			adif << '&';
			adif << data;
		} else if (eod) {
			// The ADIF will tail off
			if (state == ADIF) adif << data;
		} else {
			snprintf(msg, sizeof(msg), "QRZ: Unexpected '&' followed by %s", data);
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
			if (!got_adif) 
				status_->misc_status(ST_ERROR, "QRZ: EOF reached with no ADIF=");
		}
		if (ok) api->last_logid = last_logid;
		if (!got_count) count = count_records;
		pos = amper + 1;
	}
	return ok;
}

unsigned long long qrz_handler::last_logid(qrz_api_data* api, string adif) {
	stringstream ss;
	ss << adif;
	book* temp = new book(OT_NONE);
	adi_reader* reader = new adi_reader();
	reader->load_book(temp, ss);
	delete reader;
	unsigned long long logid = 0ull;
	unsigned long long max_logid = 0ull;
	for (auto it = temp->begin(); it != temp->end(); it++) {
		(*it)->item("APP_QRZLOG_LOGID", logid);
		max_logid = max(max_logid, logid);
	}
	delete temp;
	return max_logid;
}

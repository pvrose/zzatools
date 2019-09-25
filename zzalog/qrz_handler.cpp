#include "qrz_handler.h"

#include "status.h"
#include "tabbed_forms.h"
#include "book.h"
#include "url_handler.h"
#include "xml_reader.h"
#include "menu.h"
#include "version.h"

#include <sstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Help_Dialog.H>

using namespace zzalog;

extern status* status_;
extern tabbed_forms* tabbed_view_;
extern url_handler* url_handler_;
extern book* navigation_book_;
extern menu* menu_;
extern Fl_Preferences* settings_;

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
		status_->misc_status(ST_WARNING, "QRZ.com: login details have not been established yet. Do so before first use");
	}
	if (!url_handler_) {
		url_handler_ = new url_handler();
	}
	web_dialog_ = new Fl_Help_Dialog;
	data_.clear();
}

// Destructor
qrz_handler::~qrz_handler() {
	delete qrz_info_;
	data_.clear();
}

// Open the QRZ.com session
bool qrz_handler::open_session() {
	// We are unable to log-in to QRZ.com
	if (!user_details()) {
		status_->misc_status(ST_ERROR, "QRZ.com: login details are not available");
		return false;
	}
	// We can use the XML database - note this is subscription only
	if (use_xml_database_) {
		// Generate XML request.
		string uri = generate_session_uri();
		stringstream response;
		// Send Session requesst to QRZ.com
		if (!url_handler_->read_url(uri, &response)) {
			int length = uri.length() + 50;
			char* message = new char[length];
			snprintf(message, length, "QRZ.com: Failed to access %s", uri.c_str());
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		session_key_ = "";
		if (decode_session_response(response)) {
			if (non_subscriber_) {
				// User is not a subscriber - ask whether to continue
				if (fl_choice("You are not a subscriber to the QRZ.com XML Database so will only receive limited info.\n"
					"Do you wish to continue or use the standard web page?", "Continue", "Use Web Page", nullptr) == 1) {
					use_xml_database_ = false;
					Fl_Preferences qrz_settings(settings_, "QRZ");
					qrz_settings.set("Use XML Database", false);
					return false;
				}
			}
			status_->misc_status(ST_OK, "QRZ.com: session started");
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
		status_->misc_status(ST_ERROR, "QRZ.com: login details are not available");
		return false;
	}
	if (!session_key_.length()) {
		status_->misc_status(ST_ERROR, "QRZ.com: session has not been estalished");
		return false;
	}
	// Request 
	record* query = navigation_book_->get_record();
	status_->misc_status(ST_NOTE, string("QRZ.com: Fetching information for " + query->item("CALL")).c_str());
	string uri = generate_details_uri(query->item("CALL"));
	stringstream response;
	// Send Session requesst to QRZ.com
	if (!url_handler_->read_url(uri, &response)) {
		int length = uri.length() + 50;
		char* message = new char[length];
		snprintf(message, length, "QRZ.com: Failed to access %s", uri.c_str());
		status_->misc_status(ST_ERROR, message);
	}
	// Create a new record
	delete qrz_info_;
	qrz_info_ = new record;
	if (decode_details_response(response) && 
		query_merge()) {
		status_->misc_status(ST_OK, string("QRZ.com: query for " + query->item("CALL") + " completed.").c_str());
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

// DEcode details response
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
	tabbed_view_->activate_pane(OT_RECORD, true);
	while (!merge_done_) Fl::wait();
	status_->misc_status(ST_OK, "QRZ.com: Update complete");
	return false;
}

// Generate URI for session request
string qrz_handler::generate_session_uri() {
	string result;
	char format[] = "http://xmldata.qrz.com/xml/current/?username=%s;password=%s;agent=%s%s";
	int length = strlen(format) + username_.length() + password_.length() + PROG_ID.length() + VERSION.length();
	char* uri = new char[length];
	snprintf(uri, length, format, username_.c_str(), password_.c_str(), PROG_ID.c_str(), VERSION.c_str());
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

// Fetch user details - returns false if not enabled or wither username or password are blank strings
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
			string message = "QRZ.com: unexpected XML " + element->name() + " received";
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
			char format[] = "QRZ.com: database version mis-match: previously %s currently %s";
			int length = strlen(format) + version.length() + qrz_version_.length();
			char* message = new char[length];
			snprintf(message, length, format, qrz_version_.c_str(), version.c_str());
			status_->misc_status(ST_WARNING, message);
		}
	}
	// Now decode all the children
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
			status_->misc_status(ST_ERROR, "QRZ.com: Incompatible session keys received - see log");
			status_->misc_status(ST_LOG, string("QRZ.com: Previously " + session_key_).c_str());
			status_->misc_status(ST_LOG, string("QRZ.com: Currently  " + key).c_str());
			return false;
		}
		else {
			session_key_ = key;
		}
	}
	// Error detected - stop decoding
	if (data_.find("Error") != data_.end()) {
		status_->misc_status(ST_ERROR, string("QRZ.com: error: " + data_.at("Error")).c_str());
		return false;
	}
	// Warning message received
	if (data_.find("Message") != data_.end()) {
		status_->misc_status(ST_WARNING, string("QRZ.com: warning: " + data_.at("Message")).c_str());
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
	// Now for each received element in turn
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
	return "Please merge in data downloaded from QRZ.com database";
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

void qrz_handler::open_web_page(string callsign) {
	// If we have a selected record - get browser executable from settings
	string browser = menu_->get_browser();
	if (browser.length() == 0) {
		return;
	}
	// Open browser with QRZ.com URL 
#ifdef _WIN32
	char format[] = "start \"%s\" http://www.qrz.com/lookup?callsign=%s";
#else
	char format[] = "\"%s\" http://www.qrz.com/lookup?callsign=%s &";
#endif
	char* url = new char[strlen(format) + callsign.length() + browser.length() + 10];
	sprintf(url, format, browser.c_str(), callsign.c_str());
	status_->misc_status(ST_NOTE, string("QRZ.com: Launching QRZ.com web page for " + callsign).c_str());
	int result = system(url);
	delete[] url;
}


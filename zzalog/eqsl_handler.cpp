#include "eqsl_handler.h"

#include "record.h"
#include "book.h"
#include "status.h"
#include "adi_writer.h"

#include "../zzalib/utils.h"
#include "../zzalib/url_handler.h"
#include "../zzalib/callback.h"

#include <cstdio>
#include <fstream>
#include <istream>
#include <sstream>

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Help_Dialog.H>

using namespace zzalog;
using namespace zzalib;

extern book* book_;
extern status* status_;
extern Fl_Preferences* settings_;
extern zzalib::url_handler* url_handler_;

// Constructor
eqsl_handler::eqsl_handler()
	: empty_queue_enable_(false)
	, dequeue_parameter_({ &request_queue_, this })
	, help_dialog_(nullptr)
	, debug_enabled_(true)
{
}

// Destructor
eqsl_handler::~eqsl_handler()
{
	delete help_dialog_;
}

/* The request queue is filled as the downloaded update is processed. It is emptied one request every 10 s
   to avoid overwhelming the eQSL server. 
*/

// Put the image request on to the queue 
void eqsl_handler::enqueue_request(record_num_t record_num, bool force /*=false*/) {
	if (debug_enabled_) {
		// Enqueue request
		request_queue_.push(request_t(record_num, force));
		// Update status
		char message[512];
		sprintf(message, "EQSL: %d Card requests pending", request_queue_.size());
		status_->misc_status(ST_NOTE, message);
		// Disable saving
		book_->enable_save(false);
	}
}

// handle the timeout for the request queue - it takes the first request in the queue and sends it to eQSL.cc
void eqsl_handler::cb_timer_deq(void* v) {
	// Get the dequeue parameters
	dequeue_param_t* param = (dequeue_param_t*)v;
	queue_t* request_queue = param->queue;
	eqsl_handler* that = param->handler;
	if (!request_queue->empty() && that->empty_queue_enable_) {
		// send the next eQSL request in the queue - but leave it in the queue
		request_t request = request_queue->front();
		// Let user know what we are doing
		char message[512];
		sprintf(message, "EQSL: Downloading card %s", book_->get_record(request.record_num, false)->item("CALL").c_str());
		status_->misc_status(ST_NOTE, message);
		// Request the eQSL card
		response_t response = that->request_eqsl(request);
		bool cards_skipped = false;
		switch (response) {
		case ER_FAILED:
			// Failed - ask if retry
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("eQSL download failed, do you want try again or cancel all", fl_yes, fl_cancel, fl_no)) {
			case 0:
				// Do nothing
				break;
			case 1:
				// Cancel - delete all requests in the queue
				while (!request_queue->empty())	request_queue->pop();
				cards_skipped = true;
				break;
			case 2:
				// Request failed and repeat not wanted - remove request from queue
				request_queue->pop();
				cards_skipped = true;
				break;
			}
			break;
		case ER_OK:
			// request succeeded - remove request from queue
			request_queue->pop();
			break;
		case ER_SKIPPED:
			// request skipped - remove request from queue
			request_queue->pop();
			break;
		case ER_THROTTLED:
			// Request remains on queue to be tried again
			break;
		case ER_HTML_ERR:
			// HTML error
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Internet access failed, do you want to try again or cancel all?", fl_yes, fl_cancel, fl_no)) {
			case 0:
				// Yes - do nothing
				break;
			case 1:
				// Cancel - delete all requests in the queue
				while (!request_queue->empty())	request_queue->pop();
				cards_skipped = true;
				break;
			case 2:
				// Request failed and repeat not wanted - delete request from queue
				request_queue->pop();
				cards_skipped = true;
				break;
			}
			break;
		}
		// Set the timeout again if the queue is still not empty and fetches are enabled
		if (!request_queue->empty() && that->empty_queue_enable_) {
			// Let user know
			request = request_queue->front();
			// Now peek the queue and select the front request so user sees the QSO being request
			book_->selection(request.record_num);
			sprintf(message, "EQSL: %d card requests pending - next request %s", request_queue->size(), book_->get_record()->item("CALL").c_str());
			status_->misc_status(ST_NOTE, message);

			switch (response) {
			case ER_SKIPPED:
			case ER_HTML_ERR:
				// Can issue it immediately as this request wasn't made - assumes user has fixed the internet problem
				Fl::repeat_timeout(0.0, cb_timer_deq, v);
				break;
			default:
				// Wait for the throttle period - currently 10 s.
				Fl::repeat_timeout(EQSL_THROTTLE, cb_timer_deq, v);
				break;
			}
		}
		else {
			if (cards_skipped) {
				status_->misc_status(ST_WARNING, "EQSL: Card fetching done, but 1 or more failed");
			}
			else {
				status_->misc_status(ST_OK, "EQSL: Card fetching done!");
			}
			// Re-enable saving
			book_->enable_save(true);
		}
	}
}

// Make the eQSL card image request
eqsl_handler::response_t eqsl_handler::request_eqsl(request_t request) {
	// Get the record
	record* record = book_->get_record(request.record_num, false);
	// get where to store the card locally
	string local_filename = card_filename_l(record);
	if (card_file_valid(local_filename) && !request.force) {
		// already have the card and it's valid PNG and we are not deliberately requesting it again
		char message[200];
		snprintf(message, 200, "EQSL: Not fetching %s as already have valid image", record->item("CALL").c_str());
		status_->misc_status(ST_NOTE, message);
		return ER_SKIPPED;
	}
	else {
		// Update status with 
		char message[256];
		sprintf(message, "EQSL: Info: Fetching %s", local_filename.c_str());
		status_->misc_status(ST_NOTE, message);
		// fetch it
		string remote_filename;
		// Request the filename from eQSL.cc
		response_t response= card_filename_r(record, remote_filename);
		switch (response) {
		case ER_OK:
			// Try and download the file
			if (download(remote_filename, local_filename)) {
				return ER_FAILED;
			}
			else {
				return ER_OK;
			}
			break;
		default:
			return response;
			break;
		}
	}
}

// Get the local filename for the card - hard-defined directory structure, but configurable top directory name
string eqsl_handler::card_filename_l(record* record) {
	string card_call = record->item("CALL");
	string call = card_call;
	// Replace all / in call-sign with _ - e.g. PA/GM3ZZA/P => PA_GM3ZZA_P
	int index = call.find('/');
	while (index != string::npos) {
		card_call[index] = '_';
		index = call.find('/', index + 1);
	}
	// Get QSO details
	string qso_date = record->item("QSO_DATE");
	string time_on = record->item("TIME_ON");
	string mode = record->item("MODE");
	string band = record->item("BAND");
	// Location of top-directory for QSL card images
	Fl_Preferences datapath_settings(settings_, "Datapath");
	string qsl_directory;
	char * temp;
	datapath_settings.get("QSLs", temp, "");
	qsl_directory = temp;
	free(temp);
	// If the directory name is not defined, open a chooser to get it.
	if (!qsl_directory.length()) {
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select reference file directory");
		chooser->preset_file(qsl_directory.c_str());
		while (chooser->show()) {}
		qsl_directory = chooser->filename();
		datapath_settings.set("QSLs", qsl_directory.c_str());
		delete chooser;
	}
	char save_filename[2048];
	// Create file name e.g. <dir-name>/20M/PSK/GM3ZZA__202007201424.png
	// NB we used to have APP_ZZA_EQSL_TS as thetimestamp from the eQSL record, but we now do this after merging the data.
	sprintf(save_filename, "%s/%s/%s/%s__%s%s.png", qsl_directory.c_str(), band.c_str(), mode.c_str(), card_call.c_str(), qso_date.c_str(), time_on.substr(0, 4).c_str());
	return save_filename;
}

// Check to see if the file already exists and is a valid PNG file.
bool eqsl_handler::card_file_valid(string& filename) {
	// Create the directory structure to this file
	fl_make_path_for_file(filename.c_str());

	// Check if PNG file already exists
	bool file_exists = false;
	// Try and open the file
	ifstream file(filename.c_str());
	if (!file.good()) {
		// File doesn't exist
		return false;
	}
	else {
		// Read the first four characters to check it's a valid PNG files
		char buffer[5];
		memset(&buffer, 0, 5);
		char png_test[] = { (char)0x89, 'P', 'N', 'G', '\0' };
		file.read(buffer, 4);
		if (file.gcount() == 4) {
			// File is at least 4 characters long
			if (strcmp(buffer, png_test) == 0) {
				// Starts with the PNG marker
				return true;
			}
			else {
				// Not a valid PNG file
				return false;
			}
		}
		else {
			// Not a valid PNG file (too short)
			return false;
		}
		file.close();
	}
}

// get the remote filename of the card
eqsl_handler::response_t eqsl_handler::card_filename_r(record* record, string& card_filename) {
	// Get username and password for building url to fetch front page
	response_t response = ER_OK;
	string username;
	string password;
	char message[256];
	// Get users details
	if (!user_details(&username, &password, nullptr, nullptr, nullptr)) {
		sprintf(message, "EQSL: User or password is missing: U=%s, P=%s", username.c_str(), password.c_str());
		status_->misc_status(ST_ERROR, message);
		return ER_FAILED;
	}
	sprintf(message, "EQSL: %s: Getting remote filename", record->item("CALL").c_str());
	status_->misc_status(ST_NOTE, message);

	// url to get the front page of the eQSL card fetch interface
	char url_format[] = "http://www.eqsl.cc/qslcard/GeteQSL.cfm?Username=%s&Password=%s&CallsignFrom=%s&QSOYear=%s&QSOMonth=%s&QSODay=%s&QSOHour=%s&QSOMinute=%s&QSOBand=%s&QSOMode=%s";
	char url[2048];
	string call = record->item("CALL");
	string qso_date = record->item("QSO_DATE");
	string time_on = record->item("TIME_ON");
	string mode = record->item("MODE");
	string band = record->item("BAND");
	// Apply fields to complete the URL
	sprintf(url, url_format, username.c_str(), password.c_str(), call.c_str(), qso_date.substr(0, 4).c_str(), 
		qso_date.substr(4, 2).c_str(), qso_date.substr(6, 2).c_str(),
		time_on.substr(0, 2).c_str(), time_on.substr(2, 2).c_str(), band.c_str(), mode.c_str());
	// Stream to receive the data
	stringstream eqsl_ss;
	// Download page
	if (url_handler_->read_url(url, &eqsl_ss)) {
		// Page fetched OK.
		// Signatures to look for
		string error_signature = "Error:";
		string image_signature = "<img src=\"";
		string throttle_signature = "Warning: Processor Overload - Throttling invoked";
		string text_line;
		bool got_card_filename = false;
		int char_pos = 0;
		string file = "";
		// Interpret returned page - read a line at a time looking for one of the signatures
		while (getline(eqsl_ss, text_line) && eqsl_ss.good() && !got_card_filename) {
			file += text_line;
			// First look for Error: and set status to error message
			char_pos = text_line.find(error_signature);
			if (char_pos != string::npos) {
				// We have an error
				got_card_filename = false;
				response = ER_FAILED;
				sprintf(message, "EQSL: %s fetching card %s", text_line.substr(char_pos).c_str(), call.c_str());
				status_->misc_status(ST_ERROR, message);
			}
			else {
				// No error on this line - check for throttled warning
				char_pos = text_line.find(throttle_signature);
				if (char_pos != string::npos) {
					// We have been throttled
					response = ER_THROTTLED;
					got_card_filename = false;
					sprintf(message, "EQSL: throttle fetching card %s", call.c_str());
					status_->misc_status(ST_ERROR, message);
				}
				else {
					// The look for <img src=" and set image file name to what then follows
					char_pos = text_line.find(image_signature);
					if (char_pos != string::npos) {
						// Position to after the first quote
						char_pos += image_signature.length();
						// now look for second quote ending the filename
						int end_pos = text_line.find("\"", char_pos);
						// incorrectly formatted response
						if (end_pos == string::npos) {
							response = ER_FAILED;
							got_card_filename = false;
							sprintf(message, "EQSL: error: %s Cannot find image file name", call.c_str());
							status_->misc_status(ST_ERROR, message);
						}
						// Extract file name from response
						else {
							card_filename = text_line.substr(char_pos, end_pos - char_pos);
							response = ER_OK;
							got_card_filename = true;
						}
					}
				}
			}
		}
		return response;
	}
	else {
		// Failed to get first page
		sprintf(message, "EQSL: Fail to open HTML connection for card %s", call.c_str());
		status_->misc_status(ST_ERROR, message);
		return ER_HTML_ERR;
	}
}

// Download the card image file to local filestore
eqsl_handler::response_t eqsl_handler::download(string remote_filename, string local_filename) {

	// We have a remote file name - prepend with web-site to generate url to fetch image
	char url[2048];
	char message[256];
	sprintf(url, "http://www.eqsl.cc%s", remote_filename.c_str());
	sprintf(message, "EQSL: Getting remote image");
	status_->misc_status(ST_NOTE, message);
	// Create an output stream to the local filename and fetch the file (handled directly by url_handler)
	ofstream* os = new ofstream(local_filename, ios_base::out | ios_base::binary);
	if (url_handler_->read_url(url, os)) {
		os->close();
		status_->misc_status(ST_OK, "EQSL: Download successful");
		return ER_OK;
	}
	// Failed to get card image page
	else {
		os->close();
		status_->misc_status(ST_ERROR, "EQSL: HTML error fetching card file");
		return ER_HTML_ERR;
	}
}

// Returns user details from the settings
bool eqsl_handler::user_details(
	string* username, 
	string* password, 
	string* last_access, 
	string* qsl_message, 
	string* swl_message) {

	// Get username and password for building url to fetch card
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences eqsl_settings(qsl_settings, "eQSL");
	char * temp;
	if (username != nullptr) {
		eqsl_settings.get("User", temp, "");
		*username = temp;
		free(temp);
	}
	if (password != nullptr) {
		eqsl_settings.get("Password", temp, "");
		*password = temp;
		free(temp);
	}
	if (last_access != nullptr) {
		eqsl_settings.get("Last Accessed", temp, "19700101");
		*last_access = temp;
		free(temp);
	}
	if (qsl_message != nullptr) {
		eqsl_settings.get("QSL Message", temp, "");
		*qsl_message = temp;
		free(temp);
	}
	if (swl_message != nullptr) {
		eqsl_settings.get("SWL Message", temp, "");
		*swl_message = temp;
		free(temp);
	}
	if (username == nullptr || *username == "" || password == nullptr || *password == "") {
		// User name or password not set
		return false;
	}
	else {
		return true;
	}
}

// Download the eQSL inbox - ADIF is an internal stringstream that is later loaded into import_data_ in ADIF format
bool eqsl_handler::download_eqsl_log(stringstream* adif) {
	string filename;
	// Takes time as it's online
	fl_cursor(FL_CURSOR_WAIT);
	// get eQSL.cc filename
	if (adif_filename(filename) == ER_OK) {
		// Download inbox from eQSL.cc if it looks like a valid 
		if (download_adif(filename, adif) == ER_OK) {
			fl_cursor(FL_CURSOR_DEFAULT);
			return true;
		}
	}
	fl_cursor(FL_CURSOR_DEFAULT);
	return false;
}

// Download the inbox front page
eqsl_handler::response_t eqsl_handler::adif_filename(string& filename) {
	// Get login details
	string username;
	string password;
	string last_access;
	char message[256];
	if (!user_details(&username, &password, &last_access, nullptr, nullptr)) {
		sprintf(message, "EQSL: User or password is missing: U=%s, P=%s", username.c_str(), password.c_str());
		status_->misc_status(ST_ERROR, message);
		return ER_FAILED;
	}
	// url for in-box at eQSL.cc
	char url_format[] = "http://www.eqsl.cc/qslcard/DownloadInBox.cfm?Username=%s&Password=%s&RcvdSince=%s";
	char url[2048];
	sprintf(url, url_format, username.c_str(), password.c_str(), last_access.c_str());
	response_t result = ER_OK;
	stringstream eqsl_ss;
	strcpy(message, "EQSL: Querying in-box...");
	status_->misc_status(ST_NOTE, message);
	// Fetch first page to get URL of ADIF file with update
	if (url_handler_->read_url(url, &eqsl_ss)) {
		string text_line;
		bool ack_seen = false;
		bool nak_seen = false;
		bool error_seen = false;
		bool tag_seen = false;
		// Signature strings
		string ack_signature = "Your ADIF log file has been built";
		string nak_signature = "You have no log entries";
		string error_signature = "Error: ";
		// ADIF file name tag
		string tag_signature = "HREF=\"";
		string error_message;
		// Read each line in turn until we find ADIF tag, NAK, error or reach the end of the page
		while (getline(eqsl_ss, text_line) && !(ack_seen && tag_seen) && !nak_seen && !error_seen) {
			if (!ack_seen) {
				// Look for ACK signature 
				size_t pos = text_line.find(ack_signature);
				if (pos != string::npos) {
					ack_seen = true;
				}
				// Look for NAK signature
				pos = text_line.find(nak_signature);
				if (pos != string::npos) {
					nak_seen = true;
				}
				// Look for error signature and save error message
				pos = text_line.find(error_signature);
				if (pos != string::npos) {
					// Ignore any end header tag - if it's there
					int pos2 = text_line.find("</H");
					error_seen = true;
					if (pos2 > 0) {
						error_message = text_line.substr(pos, pos2 - pos);
					}
					else {
						error_message = text_line.substr(pos);
					}
				}
			}
			else {
				// Only get here if ACKed - look for HREF=
				int pos_start = text_line.find(tag_signature);
				if (pos_start != string::npos) {
					tag_seen = true;
					// Get the file name - skip the HREF="
					pos_start += 6;
					// Look for second quote character
					int pos_end = text_line.find("\"", pos_start);
					// url of ADIF file
					filename = text_line.substr(pos_start, pos_end - pos_start);
				}
			}
		}
		if (nak_seen) {
			// Nak signature found
			strcpy(message, "EQSL: No records to download");
			status_->misc_status(ST_OK, message);
			result = ER_SKIPPED;
		}
		else if (error_seen) {
			// Error message captured above
			sprintf(message, "EQSL: %s", error_message.c_str());
			status_->misc_status(ST_ERROR, message);
			result = ER_FAILED;
		}
		else if (!ack_seen || !tag_seen) {
			// No signature comment found
			strcpy(message, "EQSL: Unrecognised download data");
			status_->misc_status(ST_ERROR, message);
			result = ER_FAILED;
		}
	}
	else {
		result = ER_HTML_ERR;
	}

	// Remember now as the last download date
	if (result == ER_OK) {
		last_access = now(false, EQSL_TIMEFORMAT);
		Fl_Preferences qsl_settings(settings_, "QSL");
		Fl_Preferences eqsl_settings(qsl_settings, "eQSL");
		eqsl_settings.set("Last Accessed", last_access.c_str());
	}

	return result;
}

// Download the eQSL inbox
eqsl_handler::response_t eqsl_handler::download_adif(string& filename, stringstream* adif) {
	// Tell user
	char message[256];
	sprintf(message, "EQSL: Download %s...", filename.c_str());
	status_->misc_status(ST_NOTE, message);
	// Fetch the ADIF file 
	string url = "http://www.eqsl.cc/qslcard/" + filename;
	if (url_handler_->read_url(url, adif)) {
		// Successful
		strcpy(message, "EQSL: log download done!");
		status_->misc_status(ST_OK, message);
		return ER_OK;
	}
	else {
		// Failed
		strcpy(message, "EQSL: Log download failed");
		status_->misc_status(ST_ERROR, message);
		return ER_HTML_ERR;
	}
}

// Suspend requesting eQSL images
void eqsl_handler::enable_fetch(queue_control_t control) {
	empty_queue_enable_ = control == EQ_START;
	switch (control) {
	case EQ_START:
		// start timer immediately
		Fl::add_timeout(0, cb_timer_deq, (void*)&dequeue_parameter_);
		break;
	case EQ_PAUSE:
		// remove timer
		Fl::remove_timeout(cb_timer_deq, (void*)&dequeue_parameter_);
		break;
	case EQ_ABANDON:
		// remove timer and empty queue
		Fl::remove_timeout(cb_timer_deq, (void*)&dequeue_parameter_);
		while (!request_queue_.empty()) request_queue_.pop();
		break;
	}
}

// Request queue is not empty
bool eqsl_handler::requests_queued() {
	return !request_queue_.empty();
}

// Upload updates to eQSL.cc log
bool eqsl_handler::upload_eqsl_log(book* book) {
	// Clear existing help dialog
	if (help_dialog_) {
		help_dialog_->value("");
	}
	// Get login details
	string qsl_message;
	string swl_message;
	string error_message;
	response_t status = ER_OK;
	if (!user_details(&username_, &password_, nullptr, &qsl_message, &swl_message)) {
		// User details in settings are not all present
		char* message = new char[50 + username_.length() + password_.length()];
		sprintf(message, "EQSL: User or password is missing: U=%s, P=%s", username_.c_str(), password_.c_str());
		status_->misc_status(ST_ERROR, message);
		delete[] message;
		return false;
	}
	// Takes time - show timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	bool ok = true;
	// stream the book data 
	// Add QSL or SWL message to the record
	for (size_t pos = 0; pos < book->size(); pos++) {
		status = ER_OK;
		// Merge info from record into QSL message
		record* record = book->get_record(pos, false);
		// Add QSL_MSG field depending on QSO or SWL report
		string this_message;
		if (record->item("SWL") == "Y") {
			this_message = record->item_merge(swl_message);
		}
		else {
			this_message = record->item_merge(qsl_message);
		}
	}
	// Get the fields that we upload to eQSL
	set<string> fields;
	adif_fields(fields);
	// Create an internal stringstream to accept the ADIF data
	stringstream ss;
	adi_writer* writer = new adi_writer;
	writer->store_book(book, ss, &fields);
	// Revert to start of stream
	ss.seekg(ss.beg);
	// Get the HTTP POST FORM fields
	vector<url_handler::field_pair> f_fields;
	form_fields(f_fields);
	stringstream response;
	// Post the form to eQSL.cc with the stream data attached. WE'll get a response to indicate success or not
	if (url_handler_->post_form("https://www.eqsl.cc/qslcard/ImportADIF.cfm", f_fields, &ss, &response)) {
		// Successfully uploaded
		string warning_text = "";
		string text_line;
		bool valid_response = false;
		int num_errors = 0;
		int num_warnings = 0;
		set<map<string, string>> bad_records;
		bad_records.clear();
		// Get to the start of the response 
		response.seekg(response.beg);
		// Examine each line of response until a fail seen
		while (!response.eof()) {
			bool error = false;
			bool warning = false;
			getline(response, text_line);
			if (valid_response) {
				bool finished = false;
				size_t pos = 0;
				while (!finished) {
					// We have seen the signature comment so can start interpreting text
					string ack_signature = "Information: From:";
					string error_signature = "Error:";
					string warning_signature = "Warning:";
					// Note there may be more than signature per line. - we looke for the first info, error and warning
					size_t pos_i = text_line.find(ack_signature, pos);
					size_t pos_e = text_line.find(error_signature, pos);
					size_t pos_w = text_line.find(warning_signature, pos);
					// Information: From: - indicates the QSO was uploaded OK.
					if (pos_i < pos_e && pos_i < pos_w) {
						pos = text_line.find("<BR>", pos_i);
						
					}
					// Error: - a fatal error in the sURL sent, e.g. User wrong
					else if (pos_e < pos_i && pos_e < pos_w) {
						pos = text_line.find("<BR>", pos_e);
						if (pos == string::npos) {
							error_message = text_line.substr(pos_e);
						}
						else {
							error_message = text_line.substr(pos_e, pos - pos_e);
						}
						status = ER_FAILED;
						error = true;
						num_errors++;
					}
					// Warning: The sURL was OK, but the QSO was not updated - e.g. duplicate
					//     Warning: Y=2020 M=05 D=31 LA6MNA 10M FT8 Bad record: Duplicate<BR>Warning: Y=2020 M=05 D=31 PD2DVB 10M FT8 Bad record: Duplicate<BR>\r\n
					else if (pos_w < pos_i && pos_w < pos_e) {
						pos = text_line.find("<BR>", pos_w);
						if (pos == string::npos) {
							error_message = text_line.substr(pos_w);
						}
						else {
							error_message = text_line.substr(pos_w, pos - pos_w);
						}
						warning_text = error_message;
						// There are some warnings that are not of the above format - e.g. "Bad QSO Date"
						if (warning_text.substr(0, 10) == "Warning: Y") {
							bad_records.insert(parse_warning(warning_text));
						}
						warning = true;
						status = ER_SKIPPED;
						num_warnings++;
					}
					else {
						finished = true;
					}
				}
			}
			// Signature comment for successful access
			string valid_signature = "<!-- Reply form eQSL.cc ADIF Real-time Interface -->";
			if (text_line.find(valid_signature) != string::npos) {
				valid_response = true;
			}
			if (error || warning) {
				char* message = new char[256];
				snprintf(message, 256, "EQSL: %s", error_message.c_str());
				switch (status) {
				case ER_SKIPPED:
					// Warning message 
					status_->misc_status(ST_WARNING, message);
					break;
				case ER_FAILED:
					// Error message
					status_->misc_status(ST_ERROR, message);
					break;
				}
			}
		}
		// Display full response
		response.seekg(response.beg);
		if (help_dialog_) {
			string help_text = help_dialog_->value();
			help_text += '\n' + response.str();
			help_dialog_->value(help_text.c_str());
		}
		else {
			help_dialog_ = new Fl_Help_Dialog;
			help_dialog_->value(response.str().c_str());
			help_dialog_->show();
		}
		// now update book - don't try and save after each record
		book_->enable_save(false);
		for (size_t pos = 0; pos < book->size(); pos++) {
			record* record = book->get_record(pos, false);
			bool dont_update = false;
			for (auto it = bad_records.begin(); it != bad_records.end() && !dont_update; it++) {
				if (record->item("QSO_DATE") == (*it).at("QSO_DATE") &&
					record->item("CALL") == (*it).at("CALL") &&
					record->item("BAND") == (*it).at("BAND") &&
					record->item("MODE") == (*it).at("MODE") &&
					(*it).at("DUPLICATE") == "NO") {
					dont_update = true;
				}
			}
			if (!dont_update) {
				record->item("EQSL_QSLSDATE", now(false, "%Y%m%d"));
				record->item("EQSL_QSL_SENT", string("Y"));
			}
		}
		book_->enable_save(true);

		// Update status with succesful uploads and remove extracted records
		if (num_errors || num_warnings) {
			char ok_message[256];
			sprintf(ok_message, "EQSL: %d QSLs uploaded %d errors %d warnings", book->size(), num_errors, num_warnings);
			status_->misc_status(ST_OK, ok_message);
		}

	}
	fl_cursor(FL_CURSOR_DEFAULT);
	return status == ER_OK;
}

// Specify the fields requested by eQSL.cc
void eqsl_handler::adif_fields(set<string>& fields) {
	fields.clear();
	fields.insert("QSL_MSG");
	fields.insert("CALL");
	fields.insert("QSO_DATE");
	fields.insert("TIME_ON");
	fields.insert("BAND");
	fields.insert("MODE");
	fields.insert("RST_SENT");
}

// Specify the fields required by eQSL.cc in the HTTP POST FORM
void eqsl_handler::form_fields(vector<url_handler::field_pair>& fields) {
	fields.clear();
	fields.push_back({ "Filename", "", "eqsl.adi", "application/octet.stream" });
	fields.push_back({ "EQSL_USER", username_, "", "" });
	fields.push_back({ "EQSL_PSWD", password_, "", "" });
}

// parse bad record
map<string, string> eqsl_handler::parse_warning(string text) {
	// Warning: Y=2020 M=05 D=31 LA6MNA 10M FT8 Bad record : Duplicate
	vector<string> words;
	split_line(text, words, ' ');
	map<string, string> result;
	result["QSO_DATE"] = words[1].substr(2) + words[2].substr(2) + words[3].substr(2);
	result["CALL"] = words[4];
	result["BAND"] = words[5];
	result["MODE"] = words[6];
	if (words.back() == "Duplicate") {
		result["DUPLICATE"] = "YES";
	}
	else {
		result["DUPLICATE"] = "NO";
	}
	return result;
}

// Set/clear debug_enabled
void eqsl_handler::debug_enable(bool value) {
	debug_enabled_ = value;
}



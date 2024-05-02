#include "club_handler.h"
#include "url_handler.h"
#include "adi_writer.h"
#include "status.h"
#include "callback.h"
#include "cty_data.h"
#include "book.h"
#include "qso_manager.h"

#include <sstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Native_File_Chooser.H>



using namespace std;

extern url_handler* url_handler_;
extern Fl_Preferences* settings_;
extern status* status_;
extern book* book_;
extern qso_manager* qso_manager_;
extern bool DEBUG_THREADS;

// Constructor 
club_handler::club_handler() {
	// Create the URL handler if it hasn't already been done
	if (!url_handler_) url_handler_ = new url_handler;
	// Initialise thread interface
	run_threads_ = true;
	upload_response_ = 0;
	if (DEBUG_THREADS) printf("CLUBLOG MAIN: Starting thread\n");
	th_upload_ = new thread(thread_run, this);

}

club_handler::~club_handler() {
	run_threads_ = false;
	th_upload_->join();
	delete th_upload_;
}


// Upload the saved log to ClubLog using putlogs.php interface
bool club_handler::upload_log(book* book) {
	if (book->size()) {
		fl_cursor(FL_CURSOR_WAIT);
		status_->misc_status(ST_NOTE, "CLUBLOG: Starting upload");
		// Get the book data and write it to the stream
		stringstream ss;
		adi_writer* writer = new adi_writer;
		set<string> adif_fields;
		generate_adif(adif_fields);
		writer->store_book(book, ss, &adif_fields);
		// Get back to start of stream
		ss.seekg(ss.beg);
		// Get the parameters and make available for the HTTP POST FORM
		vector<url_handler::field_pair> fields;
		generate_form(fields, nullptr);
		stringstream resp;
		// Post the form
		bool ok;
		if (!url_handler_->post_form("https://clublog.org/putlogs.php", fields, &ss, &resp)) {
			// Display error message received from post
			char* message = new char[resp.str().length() + 30];
			sprintf(message, "CLUBLOG: Upload failed - %s", resp.str().c_str());
			status_->misc_status(ST_ERROR, message);
			ok = false;
		}
		else {
			// Update all records sent with the fact that they have been uploaded and when
			status_->misc_status(ST_OK, "CLUBLOG: Upload successful");
			book_->enable_save(false, "Updating Clublog status");
			ok = true;
			string today = now(false, "%Y%m%d");
			for (auto it = book->begin(); it != book->end(); it++) {
				(*it)->item("CLUBLOG_QSO_UPLOAD_DATE", today);
				(*it)->item("CLUBLOG_QSO_UPLOAD_STATUS", string("Y"));
			}
			// Go back to last entry in book.
			book_->selection(book_->size() - 1, HT_SELECTED);
			// Force the book to save itself with these changes
			book_->modified(true);
			book_->enable_save(true, "Updated Clublog status");
		}
		fl_cursor(FL_CURSOR_DEFAULT);
		return ok;
	}
	else {
		fl_cursor(FL_CURSOR_DEFAULT);
		status_->misc_status(ST_WARNING, "CLUBLOG: No data to upload!");
		return false;
	}
}

// Generate the fields in the form
void club_handler::generate_form(vector<url_handler::field_pair>& fields, record* the_qso) {
	// Read the settings that define user's access 
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	char* email;
	clublog_settings.get("Email", email, "");
	fields.push_back({"email", email, "", ""});
	free(email);
	char* password;
	clublog_settings.get("Password", password, "");
	fields.push_back({ "password", password, "", "" });
	free(password);
	if (the_qso != nullptr) {
		// get logging callsign from QSO record
		string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
		fields.push_back({ "callsign", callsign.c_str(), "", "" });
		// Get string ADIF
		fields.push_back({ "adif", single_qso_, "", "" });
	}
	else {
		// Get callsign from settings
		string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
		fields.push_back({ "callsign", callsign.c_str(), "", ""});
		// Set file to empty string to use the supplied data stream
		fields.push_back({ "file", "", "clublog.adi", "application/octet-stream" });
	}
	// Hard-coded API Key for this application
	fields.push_back({ "api", api_key_, "", "" });
}

// Identify the ADIF fields to export
void club_handler::generate_adif(set < string > &adif_fields) {
	adif_fields.insert("QSO_DATE");
	adif_fields.insert("TIME_ON");
	adif_fields.insert("TIME_OFF");
	adif_fields.insert("QSLRDATE");
	adif_fields.insert("QSLSDATE");
	adif_fields.insert("CALL");
	adif_fields.insert("OPERATOR");
	adif_fields.insert("MODE");
	adif_fields.insert("BAND");
	adif_fields.insert("BAND_RX");
	adif_fields.insert("FREQ");
	adif_fields.insert("QSL_RCVD");
	adif_fields.insert("LOTW_QSL_RCVD");
	adif_fields.insert("QSL_SENT");
	adif_fields.insert("DXCC");
	adif_fields.insert("PROP_MODE");
	adif_fields.insert("CREDIT_GRANTED");
	adif_fields.insert("RST_SENT");
	adif_fields.insert("RST_RCVD");
	adif_fields.insert("NOTES");
	adif_fields.insert("GRIDSQUARE");
}

// Download the exception file
bool club_handler::download_exception(string filename) {
	// Start downloading exception file
	status_->misc_status(ST_NOTE, "CLUBLOG: Starting to download exception file");
	string zip_filename = filename + ".gz";
	ofstream os(zip_filename, ios::trunc | ios::out | ios::binary);
	string url = "http://cdn.clublog.org/cty.php?api=" + string(api_key_);
	if (url_handler_->read_url(url, &os)) {
		os.close();
		status_->misc_status(ST_NOTE, "CLUBLOG: Exception file downloaded successfully - unzipping it");
		return unzip_exception(zip_filename);
	} else {
		os.close();
		status_->misc_status(ST_ERROR, "CLUBLOG: Exception file download failed.");
		return false;
	}
}

// Unzip the downloaded  exceptions file
bool club_handler::unzip_exception(string filename) {
	// Read the settings that define user's access 
	string ref_dir;
	get_reference(ref_dir);
	char cmd[256];
#ifdef _WIN32
	snprintf(cmd, sizeof(cmd), "\"%s\" e %s -o%s -y", "C:/Program Files/7-Zip/7z", filename.c_str(), ref_dir.c_str());
#else
	snprintf(cmd, sizeof(cmd), "gunzip -f %s", filename.c_str());
#endif

	//Fl_Preferences qsl_settings(settings_, "QSL");
	//Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	//char* cmd_executable;
	//clublog_settings.get("Unzip Command", cmd_executable, "C:/Program Files (x86)/7-Zip/7z");
	//char* switch_format;
	//clublog_settings.get("Unzip Switches", switch_format, "e %s -o%s -y");
	//char* cmd_format = new char[strlen(switch_format) + strlen(cmd_executable) + 10];
	//// Add quotes around the executable in case it is in C:\Program Files (x86)
	//sprintf(cmd_format, "\"%s\" %s", cmd_executable, switch_format);
	//char cmd[200];
	//snprintf(cmd, 199, cmd_format, filename.c_str(), ref_dir.c_str());

	//delete[] cmd_format;
	//free(cmd_executable);
	//free(switch_format);
	char msg[128];
	snprintf(msg, sizeof(msg), "CLUBLOG: Unzipping started: %s", cmd);
	status_->misc_status(ST_NOTE, msg);
	int result = system(cmd);
	if (result < 0) {
		status_->misc_status(ST_ERROR, "CLUBLOG: Unzipping failed");
		return false;
	}
	// This assumes 7z is the executble
	switch (result) {
	case 0:
		status_->misc_status(ST_OK, "CLUBLOG: Unzipping successful");
		return true;
	case 1:
		status_->misc_status(ST_WARNING, "CLUBLOG: Unzipping incurred a warning");
		return true;
	case 2:
		status_->misc_status(ST_ERROR, "CLUBLOG: Unzipping failed - fatal error");
		return false;
	case 7:
		status_->misc_status(ST_ERROR, "CLUBLOG: Unzipping failed - command-line error");
		return false;
	case 8:
		status_->misc_status(ST_ERROR, "CLUBLOG: Unzipping failed - insufficient memory");
		return false;
	case 255:
		status_->misc_status(ST_ERROR, "CLUBLOG: Unzipping failed - stopped by user");
		return false;
	}
	return true;
}

// Get reference directory
void club_handler::get_reference(string& dir_name) {
	// Get the reference data directory from the settings
	Fl_Preferences datapath(settings_, "Datapath");
	char* temp = nullptr;
	// If the path is not in the settings
	if (!datapath.get("Reference", temp, "")) {
		// Open a chooser to get it
		//Fl_File_Chooser* chooser = new Fl_File_Chooser("", nullptr, Fl_File_Chooser::DIRECTORY,
		//	"Select reference file directory");
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select reference file directory");
		chooser->preset_file(temp);
		while (chooser->show()) {}
		dir_name = chooser->filename();
		delete chooser;
		datapath.set("Reference", dir_name.c_str());
	}
	else {
		dir_name = temp;
	}
	if (temp) free(temp);
}

// Upload the single specified QSO in real time
bool club_handler::upload_single_qso(qso_num_t record_num) {
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences club_settings(qsl_settings, "ClubLog");
	int upload_qso;
	club_settings.get("Upload per QSO", upload_qso, false);
	if (upload_qso == false) {
		status_->misc_status(ST_WARNING, "CLUBLOG: Uploading per QSO is disabled.");
	}
	record* this_record = book_->get_record(record_num, false);
	if (this_record->item("CLUBLOG_QSO_UPLOAD_STATUS") == "Y") {
		char message[128];
		snprintf(message, 128, "CLUBLOG: QSO %s %s %s already uploaded - not uploading",
			this_record->item("QSO_DATE").c_str(),
			this_record->item("TIME_ON").c_str(),
			this_record->item("CALL").c_str());
		status_->misc_status(ST_WARNING, message);
		upload_qso = false;
	}
	if (upload_qso) {
		// Suspend saving
		book_->enable_save(false, "Uploading to Clublog");
		record* this_record = book_->get_record(record_num, false);
		if (DEBUG_THREADS) printf("CLUBLOG MAIN: Queueing request %s\n", this_record->item("CALL").c_str());
		upload_lock_.lock();
		upload_queue_.push(this_record);
		upload_done_queue_.push(this_record);
		upload_lock_.unlock();
	}
	return false;
}

// Upload QSO to clublog (in thread)
void club_handler::th_upload(record* this_record) {
	set<string> adif_fields;
	generate_adif(adif_fields);
	single_qso_ = to_adif(this_record, adif_fields);
	// Get the parameters and make available for the HTTP POST FORM
	vector<url_handler::field_pair> fields;
	generate_form(fields, this_record);
	stringstream resp;
	// Post the form
	bool ok;
	if (!url_handler_->post_form("https://clublog.org/realtime.php", fields, nullptr, (ostream*)&resp)) {
		ok = false;
		upload_error_ = resp.str();
	}
	else {
		ok = true;
		upload_error_ = "";

	}
	upload_response_ = ok;
	if (DEBUG_THREADS) printf("CLUBLOG THREAD: Calling thread callback result = %d(%s)\n",
	    ok, upload_error_.c_str());
	Fl::awake(cb_upload_done, (void*)this);
	this_thread::yield();
}

bool club_handler::upload_done(bool response) {
	if (response == false) {
		// Display error message received from post
		char message[128];
		snprintf(message, sizeof(message), "CLUBLOG: Upload failed - %s", upload_error_.c_str());
		status_->misc_status(ST_ERROR, message);
	}
	else {
		// Update all records sent with the fact that they have been uploaded and when
		char message[200];
		record* this_record = upload_done_queue_.front();
		snprintf(message, 200, "CLUBLOG: %s %s %s QSL uploaded",
			this_record->item("QSO_DATE").c_str(),
			this_record->item("TIME_ON").c_str(),
			this_record->item("CALL").c_str());
		status_->misc_status(ST_OK, message);
		string today = now(false, "%Y%m%d");
		this_record->item("CLUBLOG_QSO_UPLOAD_DATE", today);
		this_record->item("CLUBLOG_QSO_UPLOAD_STATUS", string("Y"));
		// Force the book to save itself with these changes
		book_->modified(true);
		book_->enable_save(true, "Uploaded to Clublog");
	}
	upload_done_queue_.pop();
	return response;
}

void club_handler::cb_upload_done(void* v) {
	if (DEBUG_THREADS) printf("CLUBLOG MAIN: Entered thread callback handler\n");
	club_handler* that = (club_handler*)v;
	that->upload_done(that->upload_response_);
}

void club_handler::thread_run(club_handler* that) {
	if (DEBUG_THREADS) printf("CLUBLOG THREAD: Thread started\n");
	while (that->run_threads_) {
		// Wait until qso placed on interface
		while (that->run_threads_ && that->upload_queue_.empty()) {
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
		// Process it
		that->upload_lock_.lock();
		if (!that->upload_queue_.empty()) {
			record* qso = that->upload_queue_.front();
			that->upload_queue_.pop();
			if (DEBUG_THREADS) printf("CLUBLOG THREAD: Received request %s\n", qso->item("CALL").c_str());
			that->upload_lock_.unlock();
			that->th_upload(qso);
		}
		else {
			that->upload_lock_.unlock();
		}
		this_thread::yield();
	}
}

string club_handler::to_adif(record* this_record, set<string> &fields) {
	string result = "";
	for (auto it = fields.begin(); it != fields.end(); it++) {
		result += adi_writer::item_to_adif(this_record, *it) + " ";
	}
	result += " <EOR>";
	return result;
}

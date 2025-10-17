#include "lotw_handler.h"
#include "adi_writer.h"
#include "status.h"
#include "utils.h"
#include "book.h"
#include "url_handler.h"
#include "callback.h"
#include "extract_data.h"
#include "fields.h"
#include "record.h"
#include "qsl_dataset.h"

#include <cstdlib>

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Help_Dialog.H>

extern status* status_;
extern book* book_;
extern url_handler* url_handler_;
extern bool DEBUG_THREADS;
extern fields* fields_;
extern qsl_dataset* qsl_dataset_;

// Constructor
lotw_handler::lotw_handler()
{
	// Initialise std::thread interface
	run_threads_ = true;
	upload_response_ = 0;	
	if (DEBUG_THREADS) printf("LOTW MAIN: Starting std::thread\n");
	th_upload_ = new std::thread(thread_run, this);

	set_adif_fields();

}

// Destructor
lotw_handler::~lotw_handler()
{
	// Close down upload std::thread
	run_threads_ = false;
	th_upload_->join();
	delete th_upload_;
}

// Export extracted records, sign them and upload to LotW
bool lotw_handler::upload_lotw_log(book* book, bool mine) {
	status_->misc_status(ST_DEBUG, "LOTW: uploading extracted data");
	fl_cursor(FL_CURSOR_WAIT);
	// Get LotW settings
	server_data_t* lotw_data = qsl_dataset_->get_server_data("LotW");
	std::string filename = lotw_data->export_file;
	std::string new_filename = "";
	bool ok = true;
	// lotw_settings.get("Export File", filename, "");
	// Open a file chooser to get file to export to - allows user to cancel upload
	Fl_Native_File_Chooser* chooser = nullptr;
	if (filename.length() == 0) {
		chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
		chooser->title("Please select file for sending to LotW");
		chooser->filter("ADI Files\t*.adi");
		// No file selected indicates user cancelled
		ok = (chooser->show() == 0);
		new_filename = chooser->filename();
	}
	else {
		new_filename = filename;
		// free(filename);
	}
	if (chooser != nullptr && !ok) {
		// User cancelled
		status_->misc_status(ST_ERROR, "LOTW: No file selected, upload cancelled");
		ok = false;
	}
	else {
		// Update settings
		char message[256];
		snprintf(message, 256, "LOTW: Writing file %s", new_filename.c_str());
		status_->misc_status(ST_DEBUG, message);
		lotw_data->export_file = new_filename;
		// lotw_settings.set("Export File", new_filename.c_str());
		// Set the fields to export 
		// Write the book (only the above fields)
		if (book->size() && book->store_data(std::string(new_filename), true, &adif_fields_)) {
#ifdef WIN32
			// Get the TQSL (an app that signs the upload) executable
			std::string tqsl_executable = "tqsl";
#else
			std::string tqsl_executable = "tqsl";
#endif			
			if (ok) {
				// Get Callsign from first (maybe only) record in book
				record* record_0 = book->get_record(0, false);
				std::string callsign = record_0->item("STATION_CALLSIGN");
				// Generate TQSL command line - note the executable may have spaces in its filename
				char* command = new char[256];
				snprintf(command, 256, "\"%s\" -x -u -d %s -c %s", tqsl_executable.c_str(), new_filename.c_str(), callsign.c_str());
				status_->misc_status(ST_NOTE, "LOTW: Signing and uploading QSLs to LotW");
				status_->misc_status(ST_LOG, command);
				if (DEBUG_THREADS) printf("LOTW MAIN: Uploading QSOs to LotW\n");
				book_->enable_save(false, "Uploading to LotW");
				upload_lock_.lock();
				upload_done_szq_.push(book->size());
				for (size_t ix = 0; ix < book->size(); ix++) {
					upload_done_queue_.push(book->get_record(ix, false));
				}
				upload_queue_.push(command);
				upload_lock_.unlock();
				if (mine) {
					delete book;
				}
			}
		}
		else {
			if (book->size()) {
				// Extraction of data failed
				status_->misc_status(ST_ERROR, "LOTW: Failed to create upload file, upload cancelled");
				ok = false;
			}
			else {
				status_->misc_status(ST_ERROR, "LOTW: No records to upload, upload abandoned");
				ok = false;
			}
		}

	}
	delete chooser;
	fl_cursor(FL_CURSOR_DEFAULT);
	return ok;
}

// Download the data from LotW
bool lotw_handler::download_lotw_log(std::stringstream* adif) {
	// Get user details from the settings
	std::string username;
	std::string password;
	std::string last_done;
	bool ok = false;
	fl_cursor(FL_CURSOR_WAIT);
	if (!user_details(&username, &password, &last_done)) {
		status_->misc_status(ST_ERROR, "LOTW: Access requires username and password!");
		return false;
	}
	// Set URL to get LotW in-box
	std::string url_format = "https://lotw.arrl.org/lotwuser/lotwreport.adi?login=%s&password=%s&qso_query=1&qso_qsl=yes&qso_qslsince=%s-%s-%s&qso_qsldetail=yes";
	char * url = new char[url_format.length() + username.length() + password.length() + last_done.length()];
	sprintf(url, 
		url_format.c_str(), 
		username.c_str(), 
		password.c_str(), 
		last_done.substr(0,4).c_str(), 
		last_done.substr(4,2).c_str(), 
		last_done.substr(6,2).c_str());
	// Download in-box in ADI format
	if (download_adif(url, adif)) {
		// Check it - it may be an HTML report providing why it failed
		if (validate_adif(adif)) {
			ok = true;
		} else {
			// display HTML in in help browser
			status_->misc_status(ST_WARNING, "LOTW: data is HTML not ADIF - opening browser");
			adif->seekg(adif->beg);
			Fl_Help_Dialog* help_dlg = new Fl_Help_Dialog;
			// std::stringstream needs converting to a C++ std::string then a C-std::string
			help_dlg->value(adif->str().c_str());
			help_dlg->show();
			ok = false;
		}
	}
	else {
		// It's neither ADIF nor HTML: display whatever has been downloaded
		adif->seekg(adif->beg);
		if (adif->peek() != EOF) {
			// Data has been downloaded - display it a text browser
			status_->misc_status(ST_WARNING, "LOTW: data neither HTML nor ADIF - opening browser");
			Fl_Text_Buffer *buff = new Fl_Text_Buffer;
			Fl_Text_Display * disp = new Fl_Text_Display(0, 0, 640, 400, "Unexpected data received from LotW");
			disp->buffer(buff);
			buff->text(adif->str().c_str());
			disp->show();
		}
		else {
			// Nothing downloaded
			status_->misc_status(ST_ERROR, "LOTW Failure to download, nothing to show!");
		}
		ok = false;
	}
	if (ok) {
		server_data_t* lotw_data = qsl_dataset_->get_server_data("LotW");
		lotw_data->last_downloaded = now(false, "%Y%m%d");
	}
	fl_cursor(FL_CURSOR_DEFAULT);
	return ok;
}

// get user details - std::set any parameter to nullptr to skip setting it
bool lotw_handler::user_details(std::string* username, std::string* password, std::string* last_access) {
	server_data_t* lotw_data = qsl_dataset_->get_server_data("LotW");
	if (username != nullptr) {
		*username = lotw_data->user;
	}
	// Check password
	if (password != nullptr) {
		*password = lotw_data->password;
	}
	if (last_access != nullptr) {
		*last_access = lotw_data->last_downloaded;
	}
	// If we've not got a username or password
	if (username == nullptr || *username == "" || password == nullptr || *password == "") {
		return false;
	}
	else {
		return true;
	}
}

// Download the data at URL into streaming data
bool lotw_handler::download_adif(const char* url, std::stringstream* adif) {
	status_->misc_status(ST_NOTE, "LOTW: Downloading log...");
	// Fetch the ADIF file
	if (url_handler_->read_url(url, adif)) {
		status_->misc_status(ST_OK, "LOTW: Log downloaded");
		return true;
	}
	else {
		status_->misc_status(ST_ERROR, "LOTW: Log download failed");
		return false;
	}
}

// Validate the data
bool lotw_handler::validate_adif(std::stringstream* adif) {
	// see if the returned data is HTML or ADIF
	// Go back tothe beginning of the stream
	adif->seekg(adif->beg);
	std::string line;
	getline(*adif, line);
	if (line.find("<!DOCTYPE HTML") != std::string::npos) {
		// It's HTML - indicates an error happened
		return false;
	}
	else {
		// It should be ADIF
		return true;
	}
}

// Upload single QSO
bool lotw_handler::upload_single_qso(qso_num_t record_num) {
	server_data_t* lotw_data = qsl_dataset_->get_server_data("LotW");
	bool upload_qso = lotw_data->upload_per_qso;
	if (upload_qso == false) {
		status_->misc_status(ST_WARNING, "LOTW: Uploading per QSO is disabled.");
	}
	record* this_record = book_->get_record(record_num, false);
	if (this_record->item("LOTW_SENT") == "Y") {
		char message[128];
		snprintf(message, 128, "LOTW: QSO %s %s %s already uploaded - not uploading",
			this_record->item("QSO_DATE").c_str(),
			this_record->item("TIME_ON").c_str(),
			this_record->item("CALL").c_str());
		status_->misc_status(ST_WARNING, message);
		upload_qso = false;
	}
	else if (this_record->item("LOTW_SENT") == "N") {
		char message[128];
		snprintf(message, 128, "LOTW: QSO %s %s %s marked \"No QSL\" - not uploading",
			this_record->item("QSO_DATE").c_str(),
			this_record->item("TIME_ON").c_str(),
			this_record->item("CALL").c_str());
		status_->misc_status(ST_WARNING, message);
		upload_qso = false;
	}
	if (upload_qso) {
		// Create book with single record
		extract_data* one_qso = new extract_data;
		one_qso->add_record(record_num);
		// TODO: Check QSO is from QTHs/Current/Callsign
		if (upload_lotw_log(one_qso, true)) {
			record* this_record = book_->get_record(record_num, false);
			char ok_message[256];
			sprintf(ok_message, "LOTW: %s %s %s QSL uploaded",
				this_record->item("QSO_DATE").c_str(),
				this_record->item("TIME_ON").c_str(),
				this_record->item("CALL").c_str());
			status_->misc_status(ST_OK, ok_message);
			return true;
		} 
	}
	return false;
}

// LotW upload ic complete - tidy up QSos affected
bool lotw_handler::upload_done(int result) {
	// Analyse result received from TQSL - responses documented in TQSL help
	bool ok = false;
	char default_message[100];
	snprintf(default_message, sizeof(default_message), "LOTW: Unknown reponse %d", result);
	switch (result) {
	case 0:
		status_->misc_status(ST_OK, "LOTW: all qsos submitted were signed and uploaded");
		ok = true;
		break;
	case 1:
		status_->misc_status(ST_WARNING, "LOTW: command failed or cancelled by user");
		ok = false;
		break;
	case 2:
		status_->misc_status(ST_ERROR, "LOTW: rejected by LoTW");
		ok = false;
		break;
	case 3:
		status_->misc_status(ST_ERROR, "LOTW: unexpected response from TQSL server");
		ok = false;
		break;
	case 4:
		status_->misc_status(ST_ERROR, "LOTW: TQSL error");
		ok = false;
		break;
	case 5:
		status_->misc_status(ST_ERROR, "LOTW: TQSLlib error");
		ok = false;
		break;
	case 6:
		status_->misc_status(ST_ERROR, "LOTW: unable to open input file");
		ok = false;
		break;
	case 7:
		status_->misc_status(ST_ERROR, "LOTW: unable to open output file");
		ok = false;
		break;
	case 8:
		status_->misc_status(ST_WARNING, "LOTW: No QSOs were processed since some QSOs were duplicates or out of date range");
		ok = true;
		break;
	case 9:
		status_->misc_status(ST_WARNING, "LOTW: Some QSOs were processed, and some QSOs were ignored because they were duplicates or out of date range");
		ok = true;
		break;
	case 10:
		status_->misc_status(ST_ERROR, "LOTW: Command syntax error");
		ok = false;
		break;
	case 11:
		status_->misc_status(ST_ERROR, "LOTW: LoTW Connection error (no network or LoTW is unreachable)");
		ok = false;
		break;
	default:
		status_->misc_status(ST_ERROR, default_message);
		if (fl_choice("Please say if upload successful or not.", "Yes", "No", nullptr) == 0) {
			ok = true;
		} else {
			ok = false;
		}
		break;
	}
	size_t count = upload_done_szq_.front();
	upload_done_szq_.pop();
	for (size_t ix = 0; ix < count; ix++) {
		record* qso = upload_done_queue_.front();
		upload_done_queue_.pop();
		if (ok) {
			if (qso->item("LOTW_QSLSDATE") == "") {
				qso->item("LOTW_QSLSDATE", now(false, "%Y%m%d"));
			}
			if (qso->item("LOTW_QSL_SENT") != "Y") {
				qso->item("LOTW_QSL_SENT", std::string("Y"));
			}
		}
	}
	book_->enable_save(true, "Uploaded to LotW");
	return ok;
}

// Run tne std::thread to handle the LotW interface
void lotw_handler::thread_run(lotw_handler* that) {
	if (DEBUG_THREADS) printf("LOTW THREAD: Thread started\n");
	while (that->run_threads_) {
		// Wait until qso placed on interface
		while (that->run_threads_ && that->upload_queue_.empty()) {
			this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		// Process it
		that->upload_lock_.lock();
		if (!that->upload_queue_.empty()) {
			const char* command = that->upload_queue_.front();
			that->upload_queue_.pop();
			if (DEBUG_THREADS) printf("LOTW THREAD: Received request %s\n", command);
			that->upload_lock_.unlock();
			that->th_upload(command);
		}
		else {
			that->upload_lock_.unlock();
		}
		this_thread::yield();
	}
}

// Run TQSL and wait for response - runs in separate std::thread
void lotw_handler::th_upload(const char* command) {
	// Blocking system request
	int result = system(command);
	// 
	upload_response_ = result;
	if (DEBUG_THREADS) printf("LOTW THREAD: Calling std::thread callback result = %d\n", result);
	Fl::awake(cb_upload_done, (void*)this);
	this_thread::yield();
}

// Callback in main std::thread after upload complete
void lotw_handler::cb_upload_done(void* v) {
	if (DEBUG_THREADS) printf("LOTW MAIN: Entered std::thread callback handler\n");
	lotw_handler* that = (lotw_handler*)v;
	that->upload_done(that->upload_response_);
}

// Specify the fields requested by eQSL.cc
void lotw_handler::set_adif_fields() {
	// Ser default values if necessary
	(void)fields_->collection("Upload/LotW", LOTW_FIELDS);
	// Now copy to the std::set 
	adif_fields_ = fields_->field_names("Upload/LotW");
}



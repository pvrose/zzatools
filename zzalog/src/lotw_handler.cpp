#include "lotw_handler.h"
#include "adi_writer.h"
#include "status.h"
#include "utils.h"
#include "book.h"
#include "url_handler.h"
#include "callback.h"
#include "extract_data.h"


#include <cstdlib>

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Help_Dialog.H>




extern Fl_Preferences* settings_;
extern status* status_;
extern book* book_;
extern url_handler* url_handler_;

// Constructor
lotw_handler::lotw_handler()
{
}

// Destructor
lotw_handler::~lotw_handler()
{
}

// Export extracted records, sign them and upload to LotW
bool lotw_handler::upload_lotw_log(book* book) {
	status_->misc_status(ST_DEBUG, "LOTW: uploading extracted data");
	fl_cursor(FL_CURSOR_WAIT);
	// Get LotW settings
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	char* filename;
	string new_filename = "";
	bool ok = true;
	lotw_settings.get("Export File", filename, "");
	// Open a file chooser to get file to export to - allows user to cancel upload
	Fl_Native_File_Chooser* chooser = nullptr;
	if (strlen(filename) == 0) {
		chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
		chooser->title("Please select file for sending to LotW");
		chooser->filter("ADI Files\t*.adi");
		// No file selected indicates user cancelled
		ok = (chooser->show() == 0);
		new_filename = chooser->filename();
	}
	else {
		new_filename = filename;
		free(filename);
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
		lotw_settings.set("Export File", new_filename.c_str());
		// Set the fields to export 
		set<string> fields;
		fields.clear();
		fields.insert("CALL");
		fields.insert("QSO_DATE");
		fields.insert("TIME_ON");
		fields.insert("BAND");
		fields.insert("MODE");
		fields.insert("SUBMODE");
		fields.insert("RST_SENT");
		fields.insert("STATION_CALLSIGN");
		// Write the book (only the above fields)
		if (book->size() && book->store_data(string(new_filename), true, &fields)) {
			// Get the TQSL (an app that signs the upload) executable
			string tqsl_executable;
			char* temp;
			Fl_Preferences datapath_settings(settings_, "Datapath");
			datapath_settings.get("TQSL Executable", temp, "");
			tqsl_executable = temp;
			free(temp);
			// We have no value in the settings for it
			if (!tqsl_executable.length()) {
#ifdef WIN32
				// Create an Open dialog; the default file name extension is ".exe".
				Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
				chooser->title("Please locate TQSL executable");
				chooser->filter("Applications\t*.exe");
#else
				// TODO: POsix equivalent filename pattern
//				Fl_File_Chooser* chooser = new Fl_File_Chooser("", "Applications(*.exe)", Fl_File_Chooser::SINGLE, "Please locate TQSL executable");
#endif
				if (chooser->show() != 0) {
					// No executable found - cancel upload
					status_->misc_status(ST_ERROR, "LOTW: TQSL Executable not found, upload cancelled");
					ok = false;
				}
				else {
					// Set the value in the settings
					tqsl_executable = chooser->filename();
					datapath_settings.set("TQSL Executable", tqsl_executable.c_str());
				}
				delete chooser;
			}
			if (ok) {
				// Get Callsign from first (maybe only) record in book
				record* record_0 = book->get_record(0, false);
				string callsign = record_0->item("STATION_CALLSIGN");
				// Generate TQSL command line - note the executable may have spaces in its filename
				char* command = new char[256];
				// TODO: Check command format in Linux
				snprintf(command, 256, "\"%s\" -x -u -d %s -c %s", tqsl_executable.c_str(), new_filename.c_str(), callsign.c_str());
				status_->misc_status(ST_NOTE, "LOTW: Signing and uploading QSLs to LotW");
				// Launch TQSL - signs and uploads data: Note this is a blocking action
				int result = system(command);
				delete[] command;
				// Analyse result received from TQSL - responses documented in TQSL help
				string default_message = "LOTW: Unknown response";
				switch (result) {
				case 0:
					status_->misc_status(ST_OK, "LOTW: all qsos submitted were signed and uploaded");
					ok = true;
					break;
				case 1:
					status_->misc_status(ST_WARNING, "LOTW: cancelled by user");
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
					ok = false;
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
					status_->misc_status(ST_ERROR, default_message.c_str());
					ok = false;
					break;
				}
			}
			if (ok) {
				// Good response received
				bool updated = false;
				// For each entry extracted for signing - add that is has been sent and when
				// Note for duplicates this will correct for the fact that these fields had wrongly been set
				for (auto it = book->begin(); it != book->end(); it++) {
					if ((*it)->item("LOTW_QSLSDATE") == "") {
						(*it)->item("LOTW_QSLSDATE", now(false, "%Y%m%d"));
						updated = true;
					}
					if ((*it)->item("LOTW_QSL_SENT") != "Y") {
						(*it)->item("LOTW_QSL_SENT", string("Y"));
						updated = true;
					}
				}
				book_->modified(updated);
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
bool lotw_handler::download_lotw_log(stringstream* adif) {
	// Get user details from the settings
	string username;
	string password;
	string last_done;
	bool ok = false;
	fl_cursor(FL_CURSOR_WAIT);
	if (!user_details(&username, &password, &last_done)) {
		status_->misc_status(ST_ERROR, "LOTW: Access requires username and password!");
		return false;
	}
	// Set URL to get LotW in-box
	string url_format = "https://lotw.arrl.org/lotwuser/lotwreport.adi?login=%s&password=%s&qso_query=1&qso_qsl=yes&qso_qslsince=%s-%s-%s&qso_qsldetail=yes";
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
			// stringstream needs converting to a C++ string then a C-string
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
		Fl_Preferences qsl_settings(settings_, "QSL");
		Fl_Preferences lotw_settings(qsl_settings, "LotW");
		lotw_settings.set("Last Accessed", now(false, "%Y%m%d").c_str());
	}
	fl_cursor(FL_CURSOR_DEFAULT);
	return ok;
}

// get user details - set any parameter to nullptr to skip setting it
bool lotw_handler::user_details(string* username, string* password, string* last_access) {
	// Get LotW login details from the settings
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	char * temp;
	if (username != nullptr) {
		lotw_settings.get("User", temp, "");
		*username = temp;
		free(temp);
	}
	if (password != nullptr) {
		lotw_settings.get("Password", temp, "");
		*password = temp;
		free(temp);
	}
	if (last_access != nullptr) {
		string today = now(false, "%Y%m%d");
		lotw_settings.get("Last Accessed", temp, today.c_str());
		*last_access = temp;
		free(temp);
		if (last_access->length() != 8) {
			*last_access = today;
		}
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
bool lotw_handler::download_adif(const char* url, stringstream* adif) {
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
bool lotw_handler::validate_adif(stringstream* adif) {
	// see if the returned data is HTML or ADIF
	// Go back tothe beginning of the stream
	adif->seekg(adif->beg);
	string line;
	getline(*adif, line);
	if (line.find("<!DOCTYPE HTML") != string::npos) {
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
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	int upload_qso;
	lotw_settings.get("Upload per QSO", upload_qso, false);
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
	if (upload_qso) {
		// Create book with single record
		extract_data* one_qso = new extract_data;
		one_qso->add_record(record_num);
		// TODO: Check QSO is from QTHs/Current/Callsign
		if (upload_lotw_log(one_qso)) {
			record* this_record = book_->get_record(record_num, false);
			char ok_message[256];
			sprintf(ok_message, "LOTW: %s %s %s QSL uploaded",
				this_record->item("QSO_DATE").c_str(),
				this_record->item("TIME_ON").c_str(),
				this_record->item("CALL").c_str());
			status_->misc_status(ST_OK, ok_message);
			delete one_qso;
			return true;
		} 
		delete one_qso;
	}
	return false;
}

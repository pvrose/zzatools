#include "club_handler.h"
#include "../zzalib/url_handler.h"
#include "adi_writer.h"
#include "status.h"
#include "../zzalib/callback.h"
#include "exc_data.h"
#include "book.h"

#include <sstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Native_File_Chooser.H>

using namespace zzalog;
using namespace zzalib;
using namespace std;

extern url_handler* url_handler_;
extern Fl_Preferences* settings_;
extern status* status_;
extern book* book_;

// Constructor 
club_handler::club_handler() {
	// Create the URL handler if it hasn't already been done
	if (!url_handler_) url_handler_ = new url_handler;
	help_dialog_ = nullptr;
}

club_handler::~club_handler() {
	delete help_dialog_;
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
		generate_form(fields);
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
			book_->enable_save(true);
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
void club_handler::generate_form(vector<url_handler::field_pair>& fields) {
	// Read the settings that define user's access 
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	char* current;
	qths_settings.get("Current", current, "");
	Fl_Preferences current_settings(qths_settings, current);
	free(current);
	char* email;
	clublog_settings.get("Email", email, "");
	fields.push_back({"email", email, "", ""});
	free(email);
	char* password;
	clublog_settings.get("Password", password, "");
	fields.push_back({ "password", password, "", "" });
	free(password);
	char* callsign;
	current_settings.get("Callsign", callsign, "");
	fields.push_back({ "callsign", callsign, "", "" });
	free(callsign);
	// Set file to empty string to use the supplied data stream
	fields.push_back({ "file", "", "clublog.adi", "application/octet-stream" });
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
bool club_handler::download_exception() {
	// Start downloading exception file
	status_->misc_status(ST_NOTE, "CLUBLOG: Starting to download exception file");
	string ref_dir;
	get_reference(ref_dir);
	string filename = ref_dir + "cty.xml.gz";
	ofstream os(filename, ios::trunc | ios::out | ios::binary);
	string url = "https://cdn.clublog.org/cty.php?api=" + string(api_key_);
	if (url_handler_->read_url(url, &os)) {
		os.close();
		status_->misc_status(ST_NOTE, "CLUBLOG: Exception file downloaded successfully - unzipping it");
		return unzip_exception(filename);
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
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	char* cmd_executable;
	clublog_settings.get("Unzip Command", cmd_executable, "C:/Program Files (x86)/7-Zip/7z");
	char* switch_format;
	clublog_settings.get("Unzip Switches", switch_format, "e %s -o%s -y");
	char* cmd_format = new char[strlen(switch_format) + strlen(cmd_executable) + 10];
	// Add quotes around the executable in case it is in C:\Program Files (x86)
	sprintf(cmd_format, "\"%s\" %s", cmd_executable, switch_format);
	char cmd[200];
	snprintf(cmd, 199, cmd_format, filename.c_str(), ref_dir.c_str());
	delete[] cmd_format;
	free(cmd_executable);
	free(switch_format);
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

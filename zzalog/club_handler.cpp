#include "club_handler.h"
#include "url_handler.h"
#include "adi_writer.h"
#include "status.h"
#include "../zzalib/callback.h"
#include "exc_data.h"

#include <sstream>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_File_Chooser.H>

using namespace zzalog;
using namespace std;

extern url_handler* url_handler_;
extern Fl_Preferences* settings_;
extern status* status_;

club_handler::club_handler() {
	if (!url_handler_) url_handler_ = new url_handler;
}

club_handler::~club_handler() {

}


// Upload the saved log to ClubLog using putlogs.php interface
bool club_handler::upload_log(book* book) {
	if (book->size()) {
		status_->misc_status(ST_NOTE, "CLUBLOG: Starting upload");
		// Get the book data
		stringstream ss;
		adi_writer* writer = new adi_writer;
		writer->store_book(book, ss);
		// Get back to start of stream
		ss.seekg(ss.beg);
		// Get the parameters
		map<string, string> fields;
		generate_form(fields);
		stringstream resp;
		// Post the form
		bool ok;
		if (!url_handler_->post_form("https://clublog.org/putlogs.php", fields, &ss, &resp)) {
			// Display error message received from post
			status_->misc_status(ST_ERROR, "CLUBLOG: Upload failed - see separate dialog");
			ok = false;
		}
		else {
			status_->misc_status(ST_OK, "CLUBLOG: Upload successful - see separate dialog");
			ok = true;
		}
		// Display the received response
		if (!help_dialog_) {
			help_dialog_ = new Fl_Help_Dialog;
		}
		help_dialog_->value(resp.str().c_str());
		help_dialog_->show();
		return ok;
	}
	else {
		status_->misc_status(ST_WARNING, "CLUBLOG: No data to upload!");
		return false;
	}
}

// Generate the fields in the form
void club_handler::generate_form(map<string, string>& fields) {
	// Read the settings that define user's access 
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences qths_settings(stations_settings, "QTHs");
	char* current;
	qths_settings.get("Current", current, "");
	Fl_Preferences current_settings(qths_settings, current);
	free(current);
	char* callsign;
	current_settings.get("Callsign", callsign, "");
	fields["callsign"] = callsign;
	free(callsign);
	char* email;
	clublog_settings.get("e-Mail", email, "");
	fields["email"] = email;
	free(email);
	char* password;
	clublog_settings.get("Password", password, "");
	fields["password"] = password;
	free(password);
	// Set file to empty string to use the supplied data stream
	fields["file"] = "";
	// Hard-coded API Key for this application
	fields["api"] = api_key_;
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
	char* cmd_format;
	clublog_settings.get("Unzip Command", cmd_format, "\"C://Program Files (x86)/7-Zip/7z\" e %s -o%s -y");
	char cmd[200];
	snprintf(cmd, 199, cmd_format, filename.c_str(), ref_dir.c_str());
	free(cmd_format);
	int result = system(cmd);
	if (result < 0) {
		status_->misc_status(ST_ERROR, "CLUBLOG: Unzipping failed");
		return false;
	}
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
		Fl_File_Chooser* chooser = new Fl_File_Chooser("", nullptr, Fl_File_Chooser::DIRECTORY,
			"Select reference file directory");
		chooser->callback(cb_chooser, &dir_name);
		chooser->textfont(FONT);
		chooser->textsize(FONT_SIZE);
		chooser->show();
		while (chooser->visible()) Fl::wait();
		delete chooser;
		datapath.set("Reference", dir_name.c_str());
	}
	else {
		dir_name = temp;
	}
	if (temp) free(temp);

}

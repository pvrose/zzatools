#include "status.h"

#include "banner.h"
#include "intl_widgets.h"
#include "main_window.h"
#include "menu.h"
#include "qso_manager.h"
#include "ticker.h"

#include "callback.h"
#include "utils.h"

#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>



extern menu* menu_;
extern main_window* main_window_;
extern status* status_;
extern qso_manager* qso_manager_;
extern bool READ_ONLY;
extern std::string PROGRAM_ID;
extern std::string PROGRAM_VERSION;
extern bool DEBUG_PRETTY;
extern std::string VENDOR;
extern ticker* ticker_;
extern banner* banner_;
extern std::string default_data_directory_;
extern bool keep_banner_;

// Constructor
status::status() :
	  report_filename_("")
	, report_file_(nullptr)
	, file_unusable_(false)
{
	// Get report filename from the settings
	report_filename_ = default_data_directory_ + "status.txt";

}

// Destructor
status::~status()
{
	if (report_file_) report_file_->close();
}

// Add a progress item to the stack
void status::progress(uint64_t max_value, object_t object, const char* description, const char* suffix) {
	// Turrn off file viewer update to improve performance
	// no_update_viewer = true;
	// Initialise it
	// Reset previous value as a new progress
	// Start a new progress bar process - create the progress item (total expected count, objects being counted, up/down and what view it's for)
	banner_->start_progress(max_value, object, description, suffix);
}

// Update progress to the new specified value
void status::progress(uint64_t value, object_t object) {
	// Update progress item
	banner_->add_progress(value);
}

// Update progress with a message - e.g. cancel it and display why cancelled
void status::progress(const char* message, object_t object) {
	banner_->cancel_progress(message);
}

// Update miscellaneous status - std::set text and colour, log the status
void status::misc_status(status_t status, const char* label) {
	// Start each entry with a timestamp
	std::string timestamp = now(false, "%Y/%m/%d %H:%M:%S", true);
	char f_message[256];
	// X YYYY/MM/DD HH:MM:SS Message 
	// X is a single letter indicating the message severity
	snprintf(f_message, sizeof(f_message), "%c %s %s\n", STATUS_CODES.at(status), timestamp.c_str(), label);
	banner_->add_message(status, label);

	if (!report_file_) {
		// Append the status to the file
		// Try to open the file. Open and close it each message
		// Save previous files
		for (char c = '8'; c > '0'; c--) {
			// Rename will fail if file does not exist, so no need to test file exists
			std::string oldfile = report_filename_ + c;
			char c2 = c + 1;
			std::string newfile = report_filename_ + c2;
			fl_rename(oldfile.c_str(), newfile.c_str());
		}
		fl_rename(report_filename_.c_str(), (report_filename_ + '1').c_str());
		// Create a new file 
		report_file_ = new std::ofstream(report_filename_, std::ios::out | std::ios::trunc);
		if (!report_file_->good()) {
			// File didn't open correctly
			delete report_file_;
			report_file_ = nullptr;
			file_unusable_ = true;
			fl_alert("STATUS: Failed to open status report file %s", report_filename_.c_str());
			// It doesn'r exist get a new filename
			// open file dialog, get it and std::set it.
			report_filename_ = "";
			while (report_filename_.length() == 0) {
				// Create an Open dialog; the default file name extension is ".txt".
				Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
				chooser->title("Select file name for status report");
				chooser->filter("Text files\t*.txt");
				if (chooser->show() == 0) {
					report_filename_ = chooser->filename();
					Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
					Fl_Preferences status_settings(settings, "Status");
					status_settings.set("Report File", report_filename_.c_str());
				}
				delete chooser;
			}
			// Create a new file 
			report_file_ = new std::ofstream(report_filename_, std::ios::out | std::ios::trunc);
		}
	}
	if (report_file_) {
		// File did open correctly - write all message to file
		*report_file_ << f_message;
		report_file_->flush();
	}

	// Depending on the severity: LOG, NOTE, OK, WARNING, ERROR, SEVERE or FATAL
	// Beep on the last three.
	switch(status) {
	case ST_SEVERE:
		// Open status file viewer and update it.
		banner_->show();
		fl_beep(FL_BEEP_ERROR);
		// A severe error - ask the user whether to continue
		if (fl_choice("An error that resulted in reduced functionality occurred:\n%s\n\nDo you want to try to continue or quit?", "Continue", "Quit", nullptr, label, report_filename_.c_str()) == 1) {
			// Set the flag to continue showing the file viewer after all other windows have been hidden.
			keep_banner_ = true;
			main_window_->do_callback();
		}
		break;
	case ST_FATAL:
		// Open status file viewer and update it. Set the flag to keep it displayed after other windows have been hidden
		banner_->show();
		fl_beep(FL_BEEP_ERROR);
		// A fatal error - quit the application
		fl_message("An unrecoverable error has occurred, closing down - check status log");
		// Close the application down
		keep_banner_ = true;
		main_window_->do_callback();
		break;
	case ST_ERROR:
		// Open status file viewer and continue
		banner_->show();
		fl_beep(FL_BEEP_ERROR);
		break;
	default:
		break;
	}
}

// Return the terminal escape code for the particular colour
std::string status::colour_code(status_t status, bool fg) {
	char result[25];
	unsigned char r, g, b;
	if (fg) {
		Fl_Color colour = STATUS_COLOURS.at(status).fg;
		Fl::get_color(colour, r, g, b);
		snprintf(result, sizeof(result), "\033[38;2;%d;%d;%dm", r, g, b);
	} else {
		Fl_Color colour = STATUS_COLOURS.at(status).bg;
		Fl::get_color(colour, r, g, b);
		snprintf(result, sizeof(result), "\033[48;2;%d;%d;%dm", r, g, b);
	}
	return std::string(result);
}

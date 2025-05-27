#include "status.h"

#include "utils.h"
#include "callback.h"
#include "menu.h"
#include "intl_widgets.h"
#include "main_window.h"
#include "qso_manager.h"
#include "ticker.h"

#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>

using namespace std;

extern menu* menu_;
extern main_window* main_window_;
extern status* status_;
extern qso_manager* qso_manager_;
extern bool READ_ONLY;
extern string PROGRAM_ID;
extern string PROGRAM_VERSION;
extern bool DEBUG_STATUS;
extern bool DEBUG_PRETTY;
extern string VENDOR;
extern ticker* ticker_;
extern string default_data_directory_;

// Constructor
status::status() :
	  report_filename_("")
	, report_file_(nullptr)
	, file_unusable_(false)
	, previous_value_(-1)
{
	// Initialise attributes
	progress_items_.clear();

	// Get report filename from the settings
	report_filename_ = default_data_directory_ + "status.txt";

	// Set 500 ms ticker
	ticker_->add_ticker(this, cb_ticker, 5);

}

// Destructor
status::~status()
{
	ticker_->remove_ticker(this);
	if (report_file_) report_file_->close();
	for (auto it = progress_items_.begin(); it != progress_items_.end(); it++) {
		update_progress(it->first);
		delete (it->second);
	}
	progress_items_.clear();
}

// Add a progress item to the stack
void status::progress(uint64_t max_value, object_t object, const char* description, const char* suffix, bool countdown /*= false*/) {
	// Turrn off file viewer update to improve performance
	// no_update_viewer = true;
	// Initialise it
	if (progress_items_.find(object) != progress_items_.end()) {
		// We already have a progress bar process in place for this object
		char message[100];
		snprintf(message, 100, "%s PROGRESS: Already started progress", OBJECT_NAMES.at(object));
		misc_status(ST_ERROR, message);
	} else {
		// Reset previous value as a new progress
		previous_value_ = -1;
		// Start a new progress bar process - create the progress item (total expected count, objects being counted, up/down and what view it's for)
		char message[100];
		snprintf(message, 100, "%s: PROGRESS: Starting %s - %lld %s", 
			OBJECT_NAMES.at(object), description, max_value, suffix);
		misc_status(ST_PROGRESS, message);
		progress_item* item = new progress_item;
		item->max_value = max_value;
		if (countdown) {
			item->value = max_value;
		}
		else {
			item->value = 0;
		}
		item->countdown = countdown;
		item->suffix = new char[strlen(suffix) + 1];
		item->description = new char[strlen(description) + 1];
		item->prev_value = 0;
		strcpy(item->suffix, suffix);
		strcpy(item->description, description);
		progress_items_[object] = item;
		update_progress(object);
		// Use progress to check for 0 entries
		if (max_value == 0) {
			progress((uint64_t)0, object);
		}
	}
}

// Update the progress for the specified object
void status::update_progress(object_t object) {
	progress_item* item = progress_items_.at(object);
	if (item->value != previous_value_) {
		char label[100];
		int pc = 100;
		if (item->max_value > 0) {
			pc = item->value * 100 / item->max_value;
		}
		sprintf(label, "%s: PROGRESS: %lld/%lld %s (%d%%)", 
			OBJECT_NAMES.at(object), item->value, item->max_value, item->suffix, pc);
		misc_status(ST_PROGRESS, label);
		previous_value_ = item->value;
	}
}

// Update progress to the new specified value
void status::progress(uint64_t value, object_t object) {
	if (progress_items_.find(object) == progress_items_.end()) {
		char message[100];
		snprintf(message, 100, "%s: PROGRESS: has not started but %lld done", OBJECT_NAMES.at(object), value);
		misc_status(ST_ERROR, message);
	} else {
		// Update progress item
		progress_item* item = progress_items_.at(object);
		item->value = value;
		// update_progress(object);
		// If it's 100% (or 0% if counting down) delete item and draw the next in the stack - certain objects can overrun (this will give an error)
		if ((item->countdown && value <= 0) || (!item->countdown && value >= item->max_value)) {
			char message[100];
			update_progress(object);
			snprintf(message, 100, "%s: PROGRESS: %s finished (%lld %s)", 
				OBJECT_NAMES.at(object), item->description, item->max_value, item->suffix);
			misc_status(ST_PROGRESS, message);
			// Remove the item from the stack - even if it's not top of the stack
			delete item;
			progress_items_.erase(object);
		} else {
			// Let the time come in if it's waiting
			Fl::check();
		}
	}
}

// Update progress with a message - e.g. cancel it and display why cancelled
void status::progress(const char* message, object_t object) {
	if (progress_items_.find(object) == progress_items_.end()) {
		// WE didn't start the bar in the first place
		char msg[100];
		snprintf(msg, 100, "%s: PROGRESS: has not started %s", OBJECT_NAMES.at(object), message);
		misc_status(ST_ERROR, msg);
	}
	else {
		progress_item* item = progress_items_.at(object);
		char msg[100];
		update_progress(object);
		snprintf(msg, 100, "%s: PROGRESS: abandoned %s (%lld %s)", OBJECT_NAMES.at(object), message, item->value, item->suffix);
		misc_status(ST_PROGRESS, msg);
		delete item;
		progress_items_.erase(object);
		
	}
}

// 200 millisecond ticker - display latest progress
void status::ticker() {
	// Redraw all status 
	for (auto it = progress_items_.begin(); it != progress_items_.end(); it++) {
		// Display progress item at top of stack
		update_progress(it->first);
	}
}

// Static version
void status::cb_ticker(void* v) {
	((status*)v)->ticker();
}

// Update miscellaneous status - set text and colour, log the status
void status::misc_status(status_t status, const char* label) {
	// Start each entry with a timestamp
	string timestamp = now(false, "%Y/%m/%d %H:%M:%S", true);
	char f_message[128];
	// X YYYY/MM/DD HH:MM:SS Message 
	// X is a single letter indicating the message severity
	snprintf(f_message, sizeof(f_message), "%c %s %s\n", STATUS_CODES.at(status), timestamp.c_str(), label);

	if (DEBUG_STATUS) {
		if (DEBUG_PRETTY) {
			// Restore default colours
			const char restore[] = "\033[0m";
			const char faint[] = "\033[2m";
			printf("%s%s%s %s%s%s%s\n",
				faint,
				timestamp.c_str(),
				restore,
				colour_code(status, true).c_str(),
				colour_code(status, false).c_str(),
				label,
				restore);
		}
		else {
			cout << f_message;
		}
	}
	if (!report_file_) {
		// Append the status to the file
		// Try to open the file. Open and close it each message
		// Create a new file 
		report_file_ = new ofstream(report_filename_, ios::out | ios::trunc);
		if (!report_file_->good()) {
			// File didn't open correctly
			delete report_file_;
			report_file_ = nullptr;
			file_unusable_ = true;
			fl_alert("STATUS: Failed to open status report file %s", report_filename_.c_str());
			// It doesn'r exist get a new filename
			// open file dialog, get it and set it.
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
			report_file_ = new ofstream(report_filename_, ios::out | ios::trunc);
		}
	}
	if (report_file_) {
		// File did open correctly - write all message to file
		*report_file_ << f_message;
	}

	// Depending on the severity: LOG, NOTE, OK, WARNING, ERROR, SEVERE or FATAL
	// Beep on the last three.
	switch(status) {
	case ST_SEVERE:
		// Open status file viewer and update it.
		fl_beep(FL_BEEP_ERROR);
		// A severe error - ask the user whether to continue
		if (fl_choice("An error that resulted in reduced functionality occurred:\n%s\n\nDo you want to try to continue or quit?", "Continue", "Quit", nullptr, label, report_filename_.c_str()) == 1) {
			// Set the flag to continue showing the file viewer after all other windows have been hidden.
			main_window_->do_callback();
		}
		break;
	case ST_FATAL:
		// Open status file viewer and update it. Set the flag to keep it displayed after other windows have been hidden
		fl_beep(FL_BEEP_ERROR);
		// A fatal error - quit the application
		fl_message("An unrecoverable error has occurred, closing down - check status log");
		// Close the application down
		main_window_->do_callback();
		break;
	case ST_ERROR:
		// Open status file viewer and continue
		fl_beep(FL_BEEP_ERROR);
		break;
	case ST_NOTIFY:
		// Open status file viewer, beep and continue
		fl_beep(FL_BEEP_NOTIFICATION);
		break;
	default:
		break;
	}
}

// Return the terminal escape code for the particular colour
string status::colour_code(status_t status, bool fg) {
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
	return string(result);
}

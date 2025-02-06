/*
ZZALOG - Amateur radio log
� - Copyright 2018, Philip Rose, GM3ZZA
All Rights Reserved

main.cpp - application entry point
*/

// local header files

#include "utils.h"
#include "record.h"
#include "menu.h"
#include "book.h"
#include "cty_data.h"
#include "toolbar.h"
#include "spec_data.h"
#include "status.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "extract_data.h"
#include "lotw_handler.h"
#include "eqsl_handler.h"
#include "url_handler.h"
#include "spec_tree.h"
#include "report_tree.h"
#include "callback.h"
#include "drawing.h"
#include "intl_dialog.h"
#include "band_data.h"
#include "qrz_handler.h"
#include "club_handler.h"
#include "wsjtx_handler.h"
#include "fllog_emul.h"
#include "hamlib/rig.h"
#include "main_window.h"
#include "qso_manager.h"
#include "logo.h"
#include "wx_handler.h"
#include "ticker.h"
#include "fields.h"
#include "symbols.h"
#include "qsl_dataset.h"
#include "config.h"

// C/C++ header files
#include <ctime>
#include <string>
#include <list>
#include <cstdio>

// FLTK header files
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Help_Dialog.H>
// included to allow windows specifics to be called
#include <FL/platform.H>

using namespace std;

// Program strings
string COPYRIGHT = "\302\251 Philip Rose GM3ZZA 2018. All rights reserved.\nPrefix data courtesy of clublog.org";
string PROGRAM_ID = "ZZALOG";
string PROG_ID = "ZLG";
string PROGRAM_VERSION = "3.4.99";
string VENDOR = "GM3ZZA";
extern string TIMESTAMP;

// switches
// Debug levels
bool DEBUG_ERRORS = true;
bool DEBUG_THREADS = false;
bool DEBUG_CURL = false;
bool DEBUG_STATUS = true;
bool DEBUG_QUICK = false;
rig_debug_level_e HAMLIB_DEBUG_LEVEL = RIG_DEBUG_ERR;
bool AUTO_UPLOAD = true;
bool AUTO_SAVE = true;
bool READ_ONLY = false;
bool RESUME_SESSION = false;
bool VERBOSE = false;
bool HELP = false;
bool PRIVATE = false;
bool DARK = false;
uchar THEME;
bool DISPLAY_VERSION = false;
bool NEW_BOOK = false;

// Ticker values - main clock runs at 10 Hz - individual objects
// will get a divided down version
const double TICK = 0.1;      // 100 ms
const unsigned int TICK_SECOND = (int)(1.0 / TICK);


// FLTK externals
extern int FL_NORMAL_SIZE;

// Top level data items - these are declared as externals in each .cpp that uses them
book* book_ = nullptr;
import_data* import_data_ = nullptr;
extract_data* extract_records_ = nullptr;
book* navigation_book_ = nullptr;
tabbed_forms* tabbed_forms_ = nullptr;
menu* menu_ = nullptr;
toolbar* toolbar_ = nullptr;
status* status_ = nullptr;
Fl_Preferences* settings_ = nullptr;
cty_data* cty_data_ = nullptr;
spec_data* spec_data_ = nullptr;
eqsl_handler* eqsl_handler_ = nullptr;
lotw_handler* lotw_handler_ = nullptr;
url_handler* url_handler_ = nullptr;
qrz_handler* qrz_handler_ = nullptr;
main_window* main_window_ = nullptr;
intl_dialog* intl_dialog_ = nullptr;
band_data* band_data_ = nullptr;
club_handler* club_handler_ = nullptr;
wsjtx_handler* wsjtx_handler_ = nullptr;
fllog_emul* fllog_emul_ = nullptr;
qso_manager* qso_manager_ = nullptr;
wx_handler* wx_handler_ = nullptr;
ticker* ticker_ = nullptr;
fields* fields_ = nullptr;
qsl_dataset* qsl_dataset_ = nullptr;
config* config_ = nullptr;
// Recent files opened
list<string> recent_files_;

// Forward declarations
void backup_file();
void set_recent_file(string filename);
void save_switches();

// Flag to prevent more than one closure process at the same time
bool closing_ = false;
// Flag to mark everything loaded
bool initialised_ = false;
// Time loaded
time_t session_start_ = (time_t)0;
// Previous frequency
double prev_freq_ = 0.0;
// Sessions is a resumption
bool resuming_ = false;
// Ticker counter - max value = 0.1 * 2^64 seconds = a long time
uint64_t ticks_ = 0;
// Filename in arguments
char* filename_ = nullptr;
// Default station callsign
string default_station_ = "";
// Main logo
Fl_PNG_Image main_icon_("ZZALOG_ICON", ___rose_png, ___rose_png_len);
// Using backp
bool using_backup_ = false;
// Sticky switches mesasge
string sticky_message_ = "";

// Get the backup filename
string backup_filename(string source) {
	// This needs to be int and not bool as the settings.get() would corrupt the stack.
	Fl_Preferences backup_settings(settings_, "Backup");
	// Get back-up directory
	char* temp;
	backup_settings.get("Path", temp, "");
	string backup = temp;
	free(temp);
	while (backup.length() == 0) {
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select directory for backup");
		if (chooser->show() == 0) {
			backup = chooser->filename();
		}
		delete chooser;
	}
	// Save the result of the chooser
	backup_settings.set("Path", backup.c_str());
	settings_->flush();

	// ensure the correct delimiiter is appendded
	if (backup.back() != '/' && backup.back() != '\\') {
#ifdef _WIN32
		backup += '\\';
#else
		backup += '/';
#endif
	}
	// Create backup filename - use back-up directory and current file-name plus timestamp
	size_t last_period = source.find_last_of('.');
	size_t last_slash = source.find_last_of("/\\");
	string suffix = source.substr(last_period);
	string base_name;
	if (last_slash == string::npos) {
		base_name = source.substr(0, last_period);
	}
	else {
		base_name = source.substr(last_slash + 1, last_period - last_slash - 1);
	}
	record* last_record = book_->get_latest();
	string timestamp = now(false, "%Y%m%d_%H%MZ");
	backup += base_name + "_" + timestamp + suffix;
	return backup;
}

// This callback intercepts the close command and performs checks and tidies up
// Updates recent files settings
static void cb_bn_close(Fl_Widget* w, void*v) {
	// The close button can only be clicked at certain times in the closure process
	// when Fl::wait() is called.
	if (closing_) {
		status_->misc_status(ST_WARNING, "ZZALOG: Already closing!");
	}
	else {
		closing_ = true;
		// Stop the ticker
		status_->misc_status(ST_NOTE, "ZZALOG: Closing...");
		// Delete band view
		// Currently modifying a (potentially new) record
		if (book_ && (book_->modified_record() || book_->new_record()) ) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("You are currently modifying a record? Save or Quit?", "Save", "Quit", nullptr)) {
			case 0:
				// Save
				status_->misc_status(ST_NOTE, "ZZALOG: Saving current open record");
				qso_manager_->end_qso();
				break;
			case 1:
				// Quit - delete any new record
				status_->misc_status(ST_WARNING, "ZZALOG: Quitting current unsaved record");
				book_->delete_record(book_->new_record());
				break;
			}
		}
		// Wait for auto-import of files to complete
		if (import_data_) {
			if (!import_data_->update_complete()) {
				fl_beep(FL_BEEP_QUESTION);
				switch (fl_choice("There is an import in process. Do you want to let it finish or abandon it?", "Finish", "Abandon", nullptr)) {
				case 0:
					// Gracefully wait for import to complete
					status_->misc_status(ST_NOTE, "ZZALOG: Allowing current import to complete before closing");
					import_data_->stop_update(false);
					while (!import_data_->update_complete()) Fl::check();
					break;
				case 1:
					// Immediately stop the import
					status_->misc_status(ST_WARNING, "ZZALOG: Abandonimg current import");
					import_data_->stop_update(true);
					break;
				}
			}
		}
		// Wait for eQSL card downloads - user can cancel
		if (eqsl_handler_ && eqsl_handler_->requests_queued()) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("There are outstanding eQSL card image requests. Do you want to cancel download, wait or cancel exit?", "Cancel download", "Wait", "Cancel exit")) {
			case 0:
				// Cancel the download immediately
				status_->misc_status(ST_WARNING, "ZZALOG: Abandonning outstanding card image fetches");
				eqsl_handler_->enable_fetch(eqsl_handler::EQ_ABANDON);
				break;
			case 1:
				// Wait for the request queue to empty
				status_->misc_status(ST_NOTE, "ZZALOG: Continuing card image download before closing");
				while (eqsl_handler_->requests_queued()) Fl::check();
				break;
			case 2:
				// Cancel Exit - don't doing anything else
				status_->misc_status(ST_WARNING, "ZZALOG: Abandoning close down!");
				closing_ = false;
				return;
			}
		}

		// Check the book needs saving
		if (book_ && book_->modified()) {
			fl_beep(FL_BEEP_QUESTION);
			switch (fl_choice("Book has been modified. Do you want to save and exit, exit or cancel exit?", "Exit", "Save && Exit", "Cancel Exit")) {
			case 0:
				// Exit
				status_->misc_status(ST_WARNING, "ZZALOG: Closing without saving recent changes");
				break;
			case 1:
				status_->misc_status(ST_NOTE, "ZZALOG: Saving changes before closing");
				// Save and Exit
				if (READ_ONLY) {
					// Open the Save As dialog and save
					menu::cb_mi_file_saveas(w, (void*)OT_MAIN);
				}
				else {
					// Save the file
					menu::cb_mi_file_save(w, (void*)OT_MAIN);
				}
				break;
			case 2:
				// Cancel Exit - don't doing anything else
				status_->misc_status(ST_WARNING, "ZZALOG: Abandoning close down");
				closing_ = false;
				return;
			}
		}

		// Back up the book
		if (book_ && book_->been_modified()) {
			if (using_backup_) {
				status_->misc_status(ST_WARNING, "ZZALOG: Data saved to backup, not to last file");
			}
			else if (book_->filename().length()) {
				backup_file();
			} else {
				status_->misc_status(ST_WARNING, "ZZALOG: No filename: any changes will not be backed up");
			}
		}
		else {
			status_->misc_status(ST_NOTE, "ZZALOG: Book has not been modified, skipping backup");
		}

		// Save the window position
		Fl_Preferences windows_settings(settings_, "Windows");
		Fl_Preferences window_settings(windows_settings, "Main");
		window_settings.set("Left", main_window_->x_root());
		window_settings.set("Top", main_window_->y_root());
		window_settings.set("Width", main_window_->w());
		window_settings.set("Height", main_window_->h());
		settings_->flush();

		// Save sticky switches
		save_switches();

		// Hide all the open windows - this will allow Fl to close the app.
		for (Fl_Window* wx = Fl::first_window(); wx; wx = Fl::first_window()) wx->hide();

		// Exit and close application
		status_->misc_status(ST_OK, "ZZALOG: Closed");
		Fl_Single_Window::default_callback((Fl_Window*)w, v);
	}
}

// Callback to parse arguments
// See show_help() for meaning of switches
int cb_args(int argc, char** argv, int& i) {
	int i_orig = i;
	// Look for read_only (-r or --read_only)
	if (strcmp("-r", argv[i]) == 0 || strcmp("--read_only", argv[i]) == 0) {
		READ_ONLY = true;
		i += 1;
	}
	// Look for test mode (-t or --test) 
	else if (strcmp("-t", argv[i]) == 0 || strcmp("--test", argv[i]) == 0) {
		AUTO_UPLOAD = false;
		AUTO_SAVE = false;
		i += 1;
	}
	// Look for normal mode (-u or --usual) 
	else if (strcmp("-u", argv[i]) == 0 || strcmp("--usual", argv[i]) == 0) {
		AUTO_UPLOAD = true;
		AUTO_SAVE = true;
		i += 1;
	}
	// No auto upload
	else if (strcmp("-q", argv[i]) == 0 || strcmp("--quiet", argv[i]) == 0) {
		AUTO_UPLOAD = false;
		i += 1;
	} 
	// auto upload
	else if (strcmp("-n", argv[i]) == 0 || strcmp("--noisy", argv[i]) == 0) {
		AUTO_UPLOAD = true;
		i += 1;
	} 
	// auto save
	else if (strcmp("-a", argv[i]) == 0 || strcmp("--auto_save", argv[i]) == 0) {
		AUTO_SAVE = true;
		i += 1;
	}
	// No auto save
	else if (strcmp("-w", argv[i]) == 0 || strcmp("--wait_save", argv[i]) == 0) {
		AUTO_SAVE = false;
		i += 1;
	}
	// Resume session
	else if (strcmp("-m", argv[i]) == 0 || strcmp("--resume", argv[i]) == 0) {
		RESUME_SESSION = true;
		i += 1;
	}
	// Private log - do not update recent files
	else if (strcmp("-p", argv[i]) == 0 || strcmp("--private", argv[i]) == 0) {
		PRIVATE = true;
		i += 1;
	}
	// Help
	else if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
		HELP = true;
		i += 1;
	}
	// Dark
	else if (strcmp("-k", argv[i]) == 0 || strcmp("--dark", argv[i]) == 0) {
		DARK = true;
		i += 1;
	}
	// Dark
	else if (strcmp("-l", argv[i]) == 0 || strcmp("--light", argv[i]) == 0) {
		DARK = false;
		i += 1;
	}
	// Version
	else if (strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) {
		DISPLAY_VERSION = true;
		i+= 1;
	}
	// Colour
	else if (strncmp("-c=", argv[i], 3) == 0) {
		THEME = *(argv[i] + 3);
		i+=1;
	}
	else if (strncmp("--colour=", argv[i], 9) == 0) {
		THEME = *(argv[i] + 9);
		i+=1;
	}
	// New file
	else if (strcmp("-e", argv[i]) == 0 || strcmp("--new", argv[i]) == 0) {
		NEW_BOOK = true;
		i+=1;
	}
	// Debug
	else if (strcmp("-d", argv[i]) == 0 || strcmp("--debug", argv[i]) == 0) {
		i += 1;
		bool debugs = true;
		while (debugs && i < argc) {
			int save_i = i;
			if (strcmp("e", argv[i]) == 0 || strcmp("errors", argv[i]) == 0) {
				DEBUG_ERRORS = true;
				i += 1;
			}
			else if (strcmp("noe", argv[i]) == 0 || strcmp("noerrors", argv[i]) == 0) {
				DEBUG_ERRORS = false;
				i += 1;
			}
			else if (strcmp("t", argv[i]) == 0 || strcmp("threads", argv[i]) == 0) {
				DEBUG_THREADS = true;
				i += 1;
			}
			else if (strcmp("not", argv[i]) == 0 || strcmp("nothreads", argv[i]) == 0) {
				DEBUG_THREADS = false;
				i += 1;
			}
			else if (strcmp("c", argv[i]) == 0 || strcmp("curl", argv[i]) == 0) {
				DEBUG_CURL = true;
				i += 1;
			}
			else if (strcmp("noc", argv[i]) == 0 || strcmp("nocurl", argv[i]) == 0) {
				DEBUG_CURL = false;
				i += 1;
			}
			else if (strcmp("s", argv[i]) == 0 || strcmp("status", argv[i]) == 0) {
				DEBUG_STATUS = true;
				i += 1;
			}
			else if (strcmp("nos", argv[i]) == 0 || strcmp("nostatus", argv[i]) == 0) {
				DEBUG_STATUS = false;
				i += 1;
			}
			else if (strcmp("q", argv[i]) == 0 || strcmp("quick", argv[i]) == 0) {
				DEBUG_QUICK = true;
				i += 1;
			} 
			else if (strncmp("h=", argv[i], 2) == 0) {
				int v = atoi(argv[i] + 2);
				HAMLIB_DEBUG_LEVEL = (rig_debug_level_e)v;
				i += 1;
			}
			else if (strncmp("hamlib=", argv[i], 7) == 0) {
				int v = atoi(argv[i] + 7);
				HAMLIB_DEBUG_LEVEL = (rig_debug_level_e)v;
				i += 1;
			}
			// Not processed any parameter
			if (i == save_i) debugs = false;
		}
	}
	if (i == i_orig ) {
		// Not processed any argumant
		if (*argv[i] == '-') {
			int i_fltk = Fl::arg(argc, argv, i);
			if (i_fltk == 0) {
				char msg[128];
				snprintf(msg, sizeof(msg), "ZZALOG: Unrecognised switch %s", argv[i]);
				status_->misc_status(ST_ERROR, msg);
				i += 1;
			}
			return i;
		} else {
			filename_ = argv[i];
			i += 1;
			return i;
		}
	} else {
		return argc;
	}
}

// Show help listing
void show_help() {
	char text[] = 
	"zzalog [switches] [filename] \n"
	"\n"
	"switches:\n"
	"\t-a|--auto_save\tDo automatically save each change (sticky)\n"
	"\t-c=s|--colour=S\tUse colour theme ""s"" (sticky)\n"
	"\t\ts=[r|g|b|m|c|y|n]\n"
  	"\t-d|--debug [mode...]\n"
	"\t\tc|curl\tincrease verbosity from libcurl\n"
	"\t\t\tnoc|nocurl\n"
	"\t\te|errors\tprovide more details on errors\n"
	"\t\t\tnoe|noerrors\n"
	"\t\th=N|hamlib=N\tSet hamlib debug level (default ERRORS)\n"
	"\t\tq|quick\tShorten long timeout and polling intervals\n"
	"\t\ts|status\tPrint status messages to terminal\n"
	"\t\t\tnos|nostatus\n"
	"\t\tt|threads\tProvide debug tracing on thread use\n"
	"\t\t\tnot|nothreads\n"
	"\t-e|--new\tCreate new file\n"
	"\t-h|--help\tPrint this\n"
	"\t-k|--dark\tDark mode (sticky)\n"
	"\t-l|--light\tLight mode (sticky)\n"
	"\t-m|--resume\tResume the previous session\n"
	"\t-n|--noisy\tDo publish QSOs to online sites (sticky)\n"
	"\t-p|--private\tDo not update recent files list\n"
	"\t-q|--quiet\tDo not publish QSOs to online sites (sticky)\n"
	"\t-r|--read_only\tOpen file in read only mode\n"
	"\t-t|--test\tTest mode: infers -q -w\n"
	"\t-u|--usual\tNormal mode: infers -a -n\n"
	"\t-w|--wait_save\tDo not automatically save each change (sticky)\n"
	"\n";
	printf(text);
}

// Use supplied argument, or read the latest file from settings or open file chooser if that's an empty string
string get_file(char * arg_filename) {
	string result = "";
	if (!arg_filename || !(*arg_filename)) {
		// null argument or empty string - get the most recent file. (Recent Files->File1 in settings)
		Fl_Preferences recent_settings(settings_, "Recent Files");
		char *filename;
		if (recent_settings.get("File1", filename, "")) {
			// return the obtained filename
			result = string(filename);
			free(filename);
		}
		else {
			free(filename);
			Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
			chooser->title("Select log file name");
			chooser->filter("ADI Files\t*.adi\nADX Files\t*.adx");
			if (chooser->show() == 0) {
				result = chooser->filename();
			}
			delete chooser;
		}
	}
	else {
		result = arg_filename;
	}
	return result;
}

// Add some global properties
void add_properties() {
	Fl_Preferences user_settings(settings_, "User Settings");
	// Tooltip settings
	Fl_Preferences tip_settings(user_settings, "Tooltip");
	Fl_Font font;
	Fl_Fontsize size;
	float duration;
	int enabled;
	tip_settings.get("Duration", duration, (float)TIP_SHOW);
	tip_settings.get("Font Name", font, 0);
	tip_settings.get("Font Size", size, FL_NORMAL_SIZE);
	// Set the default tooltip properties
	Fl_Tooltip::size(size);
	Fl_Tooltip::font(font);
	Fl_Tooltip::delay(duration);
	// Default message properties
	fl_message_size_ = FL_NORMAL_SIZE;
	fl_message_font_ = 0;
	fl_message_title_default(PROGRAM_ID.c_str());
	// Default scrollbar
	Fl::scrollbar_size(10);
}

// Get the recent files from settings
void recent_files() {
	recent_files_.clear();
	Fl_Preferences recent_settings(settings_, "Recent Files");
	// Read the first four files
	for (int i = 1; i <= 4; i++) {
		char path[6];
		sprintf(path, "File%d", i);
		char* filename = nullptr;
		recent_settings.get(path, filename, "");
		// If we have a non empty string then add it to the list
		if (filename && filename[0]) {
			recent_files_.push_back(string(filename));
		}
		free(filename);
	}
}

// read in the prefix and adif reference data
void add_data() {
	// Note closing can get set during any of the below actions.
	if (!closing_) {
		if (!club_handler_) club_handler_ = new club_handler;
		// Get pfx_data
		cty_data_ = new cty_data;
	}
	if (!closing_) {
		// add ADIF specification data.
		spec_data_ = new spec_data;
		// This can only be done once it has been fully created
		spec_data_->process_bands();
		// Draw the specification view
		((spec_tree*)tabbed_forms_->get_view(OT_ADIF))->populate_tree(false);
	}
	// Add intl dialog
	if (!closing_) {
		intl_dialog_ = new intl_dialog;
		// Don't show here - add a menu item to show it.
	}
	// And band plan data
	if (!closing_) {
		band_data_ = new band_data;
	}
	// Add the QSL design data
	if (!closing_) {
		qsl_dataset_ = new qsl_dataset;
	}
	// Add config
	if (!closing_) {
		config_ = new config(WCONFIG, HCONFIG, "Configuration");
		config_->hide();
	}
}

// read in the log data
void add_book(char* arg) {
	if (!closing_) {
		// Create the book options and set them in the forms
		book_ = new book;
		navigation_book_ = book_;
		import_data_ = new import_data;
		extract_records_ = new extract_data;
		// Tell the views that a book now exists
		tabbed_forms_->books();

		if (!NEW_BOOK || filename_) {
			// Get filename and load the data
			string log_file = get_file(arg);

			if (!book_->load_data(log_file)) {
				Fl_Preferences backup_settings(settings_, "Backup");
				char * temp;
				backup_settings.get("Last Backup", temp, "");
				string backup = temp;
				// Cannot access book - try backup
				if (!closing_ && fl_choice("Load %s failed - load from backup %s", "Yes", "No", nullptr, log_file.c_str(), backup.c_str()) == 0) {
					char msg[100];
					snprintf(msg, sizeof(msg), "ZZALOG: Load %s failed, trying backup %s", log_file.c_str(), backup.c_str());
					status_->misc_status(ST_WARNING, msg);
					if (book_->load_data(backup)) {
						using_backup_ = true;
						status_->misc_status(ST_OK, "ZZALOG: Load backup successful");
					} else {
						status_->misc_status(ST_ERROR, "ZZALOG: Load backup failed");
					}
				} else {
					char msg[100];
					snprintf(msg, sizeof(msg), "ZZALOG: Load %s failed, no backup loaded", log_file.c_str());
					status_->misc_status(closing_ ? ST_WARNING : ST_ERROR, msg);
					book_->set_filename(log_file);
				}
			} else {
				// Move this file to the top of the recent file list
				set_recent_file(log_file);
			}
		}
	}
}

// Add the various interface handlers
void add_qsl_handlers() {
	if (!closing_) {
		// URL handler - basic HTML POST and GET
		if (url_handler_ == nullptr) url_handler_ = new url_handler;
		// eQSL - accesses the appropriate URLs to upload and download eQSL data
		if (eqsl_handler_ == nullptr) eqsl_handler_ = new eqsl_handler;
		// LotW - accesses the appropriate URL to download data, TQSL to sign and upload data
		if (lotw_handler_ == nullptr) lotw_handler_ = new lotw_handler;
		// QRZ.com - accesses the appropriate URL to get information about the other station
		if (qrz_handler_ == nullptr) qrz_handler_ = new qrz_handler;
		// ClubLog handler
		if (club_handler_ == nullptr) club_handler_ = new club_handler;
		// WSJT-X server
		if (wsjtx_handler_ == nullptr) {
			wsjtx_handler_ = new wsjtx_handler;
		}
		// FLLOG emulator
		if (fllog_emul_ == nullptr) fllog_emul_ = new fllog_emul;
		// Weather handler
		if (wx_handler_ == nullptr) wx_handler_ = new wx_handler;
	}
}

// Add operating qso_manager (AKA "Dashboard")
void add_dashboard() {
	if (!closing_) {
		if (!qso_manager_) {
			char l[128];
			snprintf(l, sizeof(l), "%s %s: Operating Dashboard", PROGRAM_ID.c_str(), PROGRAM_VERSION.c_str());
			qso_manager_ = new qso_manager(10, 10);
			qso_manager_->copy_label(l);
		}
		status_->misc_status(ST_NOTE, "DASH: Opened");
	}
}

// Set the text in the main window label
void main_window_label(string text) {
	// e.g. ZZALOG 3.0.0: <filename> - PROGRAM_VERSION includes (Debug) if compiled under _DEBUG
	string label = PROGRAM_ID + " " + PROGRAM_VERSION;
	if (text.length()) {
		label += ": " + text;
	}
	const char* current = main_window_->label();
	if (!current || label != string(current)) {
		main_window_->copy_label(label.c_str());
		if (!DEBUG_STATUS) printf("%s\n", label.c_str());
	}
}

// Create the main window
void create_window() {
	// Create the main window
	main_window_ = new main_window(WIDTH, HEIGHT);
	main_window_label("");
	// add callback to intercept close command
	main_window_->callback(cb_bn_close);

}

// Add all the widgets: menu, status and tool bars, and view displays
void add_widgets(int& curr_y) {
	// The menu: disable it until all the data is loaded
	menu_ = new menu(0, curr_y, WIDTH, MENU_HEIGHT);
	main_window_->add(menu_);
	menu_->enable(false);
	curr_y += menu_->h();
	// Toolbar - image buttons representing a number of menu and other commands - disable all menu related buttons
	toolbar_ = new toolbar(0, curr_y, WIDTH, TOOL_HEIGHT);
	main_window_->add(toolbar_);
	toolbar_->update_items();
	curr_y += toolbar_->h();
	// The main views - this is a set of tabs with each view
	tabbed_forms_ = new tabbed_forms(0, curr_y, WIDTH, HEIGHT - curr_y);
	main_window_->add(tabbed_forms_);
	// Display the main window. Don't show it until it's been resized
	main_window_->end();
}

// now resize the main window
void resize_window() {
	// Get the saved size and position of the window from the settings
	int left;
	int width;
	int top;
	int height;
	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences window_settings(windows_settings, "Main");
	window_settings.get("Left", left, 0);
	window_settings.get("Top", top, 100);
	window_settings.get("Width", width, WIDTH);
	window_settings.get("Height", height, HEIGHT);
	// Only allow the views to resize fully - the bars will resize horizontally
	main_window_->resizable(tabbed_forms_);
	// Get minimum resizing from all the children - horizontal limited by views and toolbar
	int min_w = max(tabbed_forms_->min_w(), toolbar_->min_w());
	// Vertical limited by view, the bars remain a fixed height
	int min_h = tabbed_forms_->min_h() + toolbar_->h() + menu_->h();
	main_window_->size_range(min_w, min_h);
	// Set the size to the setting or minimum specified by the view + bars if that's larger
	main_window_->resize(max(left, 0), max(top, 100), max(min_w, width), max(min_h, height));
}

// Tidy memory
void tidy() {
	// Tidy memory - this is not perfect
	// From inspection of the code - calling this a second time frees the memory
	fl_message_title_default(nullptr);
	delete wx_handler_;
	delete qso_manager_;
	delete wsjtx_handler_;
	delete club_handler_;
	delete qrz_handler_;
	delete lotw_handler_;
	delete eqsl_handler_;
	delete url_handler_;
	delete fllog_emul_;
	delete extract_records_;
	delete import_data_;
	delete book_;
	delete band_data_;
	delete intl_dialog_;
	// This will be used in toolbar_
	intl_dialog_ = nullptr;
	delete spec_data_;
	delete cty_data_;
	delete tabbed_forms_;
	delete toolbar_;
	delete fields_;
	delete status_;
	delete menu_;
	delete main_window_;
	delete settings_;
}

// Add the icon
void add_icon(const char* arg0) {
	// set the default Icon
	Fl_Window::default_icon(&main_icon_);
}

// Map argument letter to colour name
map<uchar, string> colours = {
	{ 'n', "None" },
	{ 'r', "Red" },
	{ 'g', "Green" },
	{ 'b', "Blue" },
	{ 'm', "Magenta" },
	{ 'c', "Cyan" },
	{ 'y', "Yellow" } 
};

// Display the arguments in the status log
void print_args(int argc, char** argv) {
	// Create a string to hold all the info
	int length = 20;
	for (int i = 0; i < argc; i++) {
		length += strlen(argv[i]);
	}
	char message[256];
	memset(message, 0, sizeof(message));
	// Generate the string
	strcpy(message, "ZZALOG: ");
	for (int i = 0; i < argc; i++) {
		strcat(message, argv[i]);
		strcat(message, " ");
	}
	strcat(message, "Started");
	status_->misc_status(ST_NOTE, message);
	snprintf(message, sizeof(message), "ZZALOG: %s", main_window_->label());
	status_->misc_status(ST_NOTE, message);
	snprintf(message, sizeof(message), "ZZALOG: Compiled %s", TIMESTAMP.c_str());
	status_->misc_status(ST_NOTE, message);

	if (DEBUG_ERRORS) status_->misc_status(ST_NOTE, "ZZALOG: -d e - Displaying debug error messages");
	if (DEBUG_THREADS) status_->misc_status(ST_NOTE, "ZZALOG: -d t - Displaying thread debug messages");
	if (DEBUG_CURL) status_->misc_status(ST_NOTE, "ZZALOG: -d c - Displaying more verbosity from libcurl");
	if (DEBUG_STATUS) status_->misc_status(ST_NOTE, "ZZALOG: -d s - Displaying status messages");
	if (DEBUG_QUICK) status_->misc_status(ST_WARNING, "ZZALOG: -d q - Reducing periods of some reguat events");
	snprintf(message, sizeof(message), "ZZALOG: -d h=%d - Hamlib debug level %d", 
		(int)HAMLIB_DEBUG_LEVEL, (int)HAMLIB_DEBUG_LEVEL);
	status_->misc_status(ST_NOTE, message);
	if (NEW_BOOK && !filename_) status_->misc_status(ST_NOTE, "ZZALOG: -e - Starting with empty file");
	if (NEW_BOOK && filename_) status_->misc_status(ST_WARNING, "ZZALOG: -e - filename specified, switch ignored");
	if (AUTO_UPLOAD) status_->misc_status(ST_NOTE, "ZZALOG: -n - QSOs uploaded to QSL sites automatically");
	else status_->misc_status(ST_WARNING, "ZZALOG: -q - QSOs are not being uploaded to QSL sites");
	if (AUTO_SAVE) status_->misc_status(ST_NOTE, "ZZALOG: -a - QSOs being saved automatically");
	else status_->misc_status(ST_WARNING, "ZZALOG: -w - QSOs are not being saved automatically");
	if (READ_ONLY) status_->misc_status(ST_WARNING, "ZZALOG: -r - File opened read-only");
	if (RESUME_SESSION) status_->misc_status(ST_NOTE, "ZZALOG: -m - Resuming previous session");
	if (PRIVATE) status_->misc_status(ST_WARNING, "ZZALOG: -p - This file not being noted on recent files list");
	if (DARK) status_->misc_status(ST_NOTE, "ZZALOG: -k - Opening in dark mode");
	else status_->misc_status(ST_NOTE, "ZZALOG: -l - Opening in normal FLTK colours");
	snprintf(message, sizeof(message), "ZZALOG: -c=%c - Thematic colour: %s", THEME, colours[THEME].c_str());
	status_->misc_status(ST_NOTE, message);
}

// Returns true if record is within current session.
bool in_current_session(record* this_record) {
	return difftime(this_record->timestamp(), session_start_) >= 0;
}

// Customise FLTK feature
void customise_fltk() {
	// Set default font size for all widgets
	FL_NORMAL_SIZE = 10;
	// FLTK 1.4 default contrast algorithm
	fl_contrast_mode(FL_CONTRAST_CIELAB);
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER,            "Courier New");
	Fl::set_font(FL_COURIER_BOLD,       "Courier New Bold");
	Fl::set_font(FL_COURIER_ITALIC,     "Courier New Italic");
	Fl::set_font(FL_COURIER_BOLD_ITALIC,"Courier New Bold Italic");	
#ifndef _WIN32
	// Use liberation fonts as closest to Windows fonts
	Fl::set_font(FL_TIMES,            "Liberation Serif");
	Fl::set_font(FL_TIMES_BOLD,       "Liberation Serif Bold");
	Fl::set_font(FL_TIMES_ITALIC,     "Liberation Serif Italic");
	Fl::set_font(FL_TIMES_BOLD_ITALIC,"Liberation Serif Bold Italic");	
	// Fl::set_font(FL_HELVETICA,            "Liberation Sans");
	// Fl::set_font(FL_HELVETICA_BOLD,       "Liberation Sans Bold");
	// Fl::set_font(FL_HELVETICA_ITALIC,     "Liberation Sans Italic");
	// Fl::set_font(FL_HELVETICA_BOLD_ITALIC,"Liberation Sans Bold Italic");	
#endif
	// Add label symbols
	fl_add_symbol("eyeshut", &draw_eyeshut, true);
	fl_add_symbol("eyeopen", &draw_eyeopen, true);
	fl_add_symbol("calendar", &draw_calendar, true);
	fl_add_symbol("mail", &draw_mail, true);
	// Customise colours
	bool vr, vg, vb;
	switch(THEME) {
		case 'r': {
			vr = true;
			vg = false;
			vb = false;
			break;
		}
		case 'g': {
			vr = false;
			vg = true;
			vb = false;
			break;
		}
		case 'b': {
			vr = false;
			vg = false;
			vb = true;
			break;
		}
		case 'm': {
			vr = true;
			vg = false;
			vb = true;
			break;
		}
		case 'c': {
			vr = false;
			vg = true;
			vb = true;
			break;
		}
		case 'y': {
			vr = true;
			vg = true;
			vb = false;
			break;
		}
		case 'n':
		default: {
			vr = true;
			vg = true;
			vb = true;
			break;
		}
	}

	if (DARK) {
		// Fl::foreground(240, 240, 240);
		// Fl::background2(vr ? 32 : 0, vg ? 32 : 0, vb ? 32 : 0);
		// Fl::background(vr ? 64 : 48, vg ? 64 : 48, vb ? 64 : 48);
		Fl::foreground(vr ? 255 : 240, vg ? 255 : 240, vb ? 255 : 240);
		Fl::background2(vr ? 16 : 0, vg ? 16 : 0, vb ? 16 : 0);
		Fl::background(vr ? 64 : 48, vg ? 64 : 48, vb ? 64 : 48);
	} else {
		Fl::foreground(vr ? 16 : 0, vg ? 16 : 0, vb ? 16 : 0);
		Fl::background2(vr ? 255 : 240, vg ? 255 : 240, vb ? 255 : 240);
		Fl::background(vr ? 192 : 174, vg ? 192 : 174, vb ? 192 : 174);
	}
	// Fl::scheme("gleam");
}

// Some switches get saved between sessions - so-called sticky switches
void read_saved_switches() {
	Fl_Preferences switch_settings(settings_, "Switches");
	int temp;
	char* stemp;
	char msg[128];
	memset(msg, 0, sizeof(msg));
	strcpy(msg, "ZZALOG: Sticky switches: ");
	switch_settings.get("Dark Mode", temp, false);
	DARK = (bool)temp;
	if (DARK) strcat(msg, "-k ");
	else strcat(msg, "-l ");
	switch_settings.get("Theme Colour", stemp, "n");
	THEME = stemp[0];
	strcat(msg, "-c=");
	size_t pos = strlen(msg);
	msg[pos++] = THEME;
	msg[pos++] = ' ';
	switch_settings.get("Auto Update QSOs", temp, false);
	AUTO_UPLOAD = (bool)temp;
	if (AUTO_UPLOAD) strcat(msg, "-n ");
	else strcat(msg, "-q ");
	switch_settings.get("Auto Save QSOs", temp, false);
	AUTO_SAVE = (bool)temp;
	if (AUTO_SAVE) strcat(msg, "-a");
	else strcat(msg, "-w");
	sticky_message_ = msg;
}

// Save "sticky" switches
void save_switches() {
	Fl_Preferences switch_settings(settings_, "Switches");
	char temp[2];
	temp[0] = THEME;
	temp[1] = '\0';
	switch_settings.set("Dark Mode", (int)DARK);
	switch_settings.set("Theme Colour", temp);
	switch_settings.set("Auto Update QSOs", (int)AUTO_UPLOAD);
	switch_settings.set("Auto Save QSOs", (bool)AUTO_SAVE);
	settings_->flush();
}

// Open preferences and save them - it is possible to corrupt the settings
// file if an exception occurs while they are being saved.
bool open_settings() {
	// Open the settings file 
	settings_ = new Fl_Preferences(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());

	char source[256];
	Fl_Preferences::Root root = settings_->filename(source, sizeof(source));
	if (root == Fl_Preferences::USER) {
		printf("ZZALOG: Opened settings %s\n", source);
	}
	char backup[256];
	strcpy(backup, source);
	strcat(backup, ".save");
	// In and out streams
	ifstream in(source);
	in.seekg(0, in.end);
	int length = (int)in.tellg();
	if (in.good() && length) {
		// We do have a (probably) valid settings file
		const int increment = 8000;
		in.seekg(0, in.beg);
		ofstream out(backup);
		bool ok = in.good() && out.good();
		char buffer[increment];
		int count = 0;
		// Copy file in 7999 byte chunks
		while (!in.eof() && ok) {
			in.read(buffer, increment);
			out.write(buffer, in.gcount());
			count += (int)in.gcount();
			ok = out.good() && (in.good() || in.eof());
		}
		in.close();
		out.close();

		if (ok) {
			printf("ZZALOG: Settings saved as %s\n", backup);
		}
	}
	else {
		// No settings file - check a new installation
		switch(fl_choice("This appears to be a new installation, Continue?", "Yes", "No", nullptr)) {
			case 0: {
				printf("ZZALOG: No settings file - creating new settings\n");
				break;
			}
			case 1: {
				printf("ZZALOG: Corrupt settings - abandoning\n");
				return false;
			}
			
		}
	}
	return true;
}

// The main app entry point
int main(int argc, char** argv)
{
	// Allow the main thread to respond to Fl::awake() requests
	Fl::lock();
	// Set default Fil Chooser on non-windows
	// Create the settings before anything else 
	if (!open_settings()) {
		return 255;
	}

	// Create the ticker first of all
	ticker_ = new ticker();

	// Ctreate status to handle status messages
	status_ = new status();

	// Read the fields data
	fields_ = new fields;

	// Read any switches that stick between calls
	read_saved_switches();
	// Parse command-line arguments - accept FLTK standard arguments and custom ones (in cb_args)
	int i = 1;
	Fl::args(argc, argv, i, cb_args);
	if (DISPLAY_VERSION) {
		// Display version
		printf("%s Version %s Compiled %s\n", 
			PROGRAM_ID.c_str(), 
			PROGRAM_VERSION.c_str(),
			TIMESTAMP.c_str());
		return 0;
	}
	if (HELP) {
		// Help requested - display help text and exit
		show_help();
		return 0;
	}
	// Now display sticky switch message
	status_->misc_status(ST_NOTE, sticky_message_.c_str());	

	// Switches affect the customisation
	customise_fltk();
	// Create window
	add_icon(argv[0]);
	create_window();
	add_properties();
	recent_files();

	// now show the window
	main_window_->show(argc, argv);
	// add the various drawn items
	int curr_y = 0;
	add_widgets(curr_y);
	print_args(argc, argv);
	// Resize the window
	resize_window();
	// Read in reference data - uses progress
	add_data();
	Fl::check();
	// Read in log book data - uses progress - use supplied argument for filename
	add_book(filename_);
	Fl::check();
	// Connect to the rig - load all hamlib backends once only here
	rig_set_debug(HAMLIB_DEBUG_LEVEL);
	rig_load_all_backends();
	// Add qso_manager
	add_dashboard();
	// Add qsl_handlers - note add_rig_if() may have added URL handler
	add_qsl_handlers();
	int code = 0;
	// We are now initialised
	initialised_ = true;
	if (!closing_) {
		// Now we have created everything add the windows items to the menu
		// Enable menu so that we can do thigs while waiting for Fllog client to appear
		menu_->add_windows_items();
		menu_->enable(true);
		menu_->redraw();
		// Only do this if we haven't tried to close
		fllog_emul_->run_server();
		// enable menu
		// Run the application until it is closed
		code = Fl::run();
	}
	// Delete everything we've created
	tidy();
	printf("ZZALOG Closed\n");
	return code;
}

// Copy existing data to back up file. force = true used by menu command, 
void backup_file() {
	string source = book_->filename();
	string backup = backup_filename(source);
	char* message = new char[backup.length() + 25];
	sprintf(message, "BACKUP: Writing %s", backup.c_str());
	status_->misc_status(ST_NOTE, message);
	delete[] message;
	// In and out streams
	ifstream in(source);
	in.seekg(0, in.end);
	int length = (int)in.tellg();
	const int increment = 8000;
	in.seekg(0, in.beg);
	status_->progress(length, OT_MAIN, "Copying data to backup", "bytes");
	ofstream out(backup);
	bool ok = in.good() && out.good();
	char buffer[increment];
	int count = 0;
	// Copy file in 7999 byte chunks
	while (!in.eof() && ok) {
		in.read(buffer, increment);
		out.write(buffer, in.gcount());
		count += (int)in.gcount();
		ok = out.good() && (in.good() || in.eof());
		status_->progress(count, OT_MAIN);
	}
	if (!ok) {
		status_->progress("Failed before completion", OT_MAIN);
	} else {
		if (count != length) {
			status_->progress(length, OT_MAIN);
		}
	}
	in.close();
	out.close();
	if (!ok) {
		// Report error
		status_->misc_status(ST_ERROR, "BACKUP: failed");
	} else {
		status_->misc_status(ST_OK, "BACKUP: Done");
		Fl_Preferences backup_settings(settings_, "Backup");
		backup_settings.set("Last Backup", backup.c_str());
		settings_->flush();
	}
}

// Add the current file to the recent files list
void set_recent_file(string filename) {
	// Do not add to recent file list if using backup or CLI inihibited
	if (!PRIVATE && !using_backup_) {

		// Add or move the file to the front of list
		recent_files_.remove(filename);
		recent_files_.push_front(filename);

		// Update recent files in the settings
		Fl_Preferences recent_settings(settings_, "Recent Files");
		// Clear the existing settings
		recent_settings.clear();
		// And write the top four names on the list to settings
		int i = 1;
		for (auto it = recent_files_.begin(); i <= 4 && it != recent_files_.end(); i++, it++) {
			char path[6];
			sprintf(path, "File%d", i);
			recent_settings.set(path, (*it).c_str());
		}
		settings_->flush();

		menu_->add_recent_files();

	}

}

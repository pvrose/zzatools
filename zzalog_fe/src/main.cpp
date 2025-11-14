/*
ZZALOG - Amateur radio log
� - Copyright 2018, Philip Rose, GM3ZZA
All Rights Reserved

main.cpp - application entry point
*/

// local header files
#include "main.h"


#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include "about_dialog.h"
#include "band_data.h"
#include "band_window.h"
#include "banner.h"
#include "book.h"
#include "club_handler.h"
#include "config.h"
#include "contest_data.h"
#include "cty_data.h"
#include "eqsl_handler.h"
#include "extract_data.h"
#include "fields.h"
#include "file_holder.h"
#include "fllog_emul.h"
#include "import_data.h"
#include "init_dialog.h"
#include "intl_dialog.h"
#include "logo.h"
#include "lotw_handler.h"
#include "main_window.h"
#include "menu.h"
#include "qrz_handler.h"
#include "qsl_dataset.h"
#include "qso_manager.h"
#include "record.h"
#include "report_tree.h"
#include "rig_data.h"
#include "settings.h"
#include "spec_data.h"
#include "spec_tree.h"
#include "status.h"
#include "stn_data.h"
#include "stn_dialog.h"
#include "symbols.h"
#include "tabbed_forms.h"
#include "ticker.h"
#include "timestamp.h"
#include "toolbar.h"
#include "url_handler.h"
#include "wsjtx_handler.h"
#include "wx_handler.h"

#include "hamlib/rig.h"

// C/C++ header files
#include <ctime>
#include <string>
#include <list>
#include <cstdio>
#include <cstdlib>

// FLTK header files
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
// included to allow windows specifics to be called
#include <FL/platform.H>



//! Program copyright - displayed in all windows.
std::string COPYRIGHT = "\302\251 Philip Rose GM3ZZA 2018-2025. All rights reserved.";
//! Third-party acknowledgments.
std::string PARTY3RD_COPYRIGHT = "Prefix data courtesy of clublog.org, country-files.com and dxatlas.com\n"
"ZZALOG is based in part on the work of the FLTK project https://www.fltk.org.";
//! Contact address for use in FLTK widget labels.
std::string CONTACT = "gm3zza@@btinternet.com";
//! Contact address for use in general texts.
std::string CONTACT2 = "gm3zza@btinternet.com";
//! Copyright placed in exported data items.
std::string DATA_COPYRIGHT = "\302\251 Philip Rose %s. This data may be copied for the purpose of correlation and analysis";
//! Program identifier: used in ADIF PROGRAM_ID field and filestore
std::string PROGRAM_ID = "ZZALOG";
//! Short-form program identifier.
std::string PROG_ID = "ZLG";
//! Program version. 
std::string PROGRAM_VERSION = "3.6.9+";
//! Program vendor.
std::string VENDOR = "GM3ZZA";

// Debug switches
//! Print errors -  by "-d e"
bool DEBUG_ERRORS = true;
//! Print std::thread debugging messages -  by "-d t"
bool DEBUG_THREADS = false;
//! Print libcurl debugging messages -  by "-d c"
bool DEBUG_CURL = false;
//! Reduce long duration tiemouts and waits -  by "-d q"
bool DEBUG_QUICK = false;
//! Print rig access debugging messages -  by "-d r"
bool DEBUG_RIGS = false;
//! Print callsign parsing messages -  by "-d d"
bool DEBUG_PARSE = false;
//! Set hamlib debugging verbosity level -  by "-d h=<level>"
rig_debug_level_e HAMLIB_DEBUG_LEVEL = RIG_DEBUG_ERR;

// Operation switches - _S versions used to override sticky switch
//! Automatically upload QSOs to QSL sites -  by "-n"
bool AUTO_UPLOAD = true;
//! Version of \p AUTO_UPLOAD read from settings
bool AUTO_UPLOAD_S = false;
//! Automatically save QSO record after each change -  by "-a"
bool AUTO_SAVE = true;
//! Version of \p AUTO_SAVE read from settings.
bool AUTO_SAVE_S = false;
//! Dark mode: Dark background, light forreground -  by "-k"
bool DARK = false;
//! Version of \p DARK read from settings.
bool DARK_S = false;
//! Print version details instead of running ZZALOG -  by "-v"
bool DISPLAY_VERSION = false;
//! Print command-line interface instead of running ZZALOG -  by "-h"
bool HELP = false;
//! Start with an empty logbook -  by "-e"
bool NEW_BOOK = false;
//! Do not add file to recent file std::list -  by "-p"
bool PRIVATE = false;
//! Open file in read-only mode -  by "-r"
bool READ_ONLY = false;
//! Resum logging including previous session -  by "-m"
bool RESUME_SESSION = false;
//! Development flag: used to enable/disable features only in development mode ("-g")
bool DEVELOPMENT_MODE = false;

//! Access to FLTK global attribute to  default text size throughout ZZALOG.
extern int FL_NORMAL_SIZE;

//! \cond
// Top level data items - these are declared as externals in each .cpp that uses them
band_data* band_data_ = nullptr;
band_window* band_window_ = nullptr;
banner* banner_ = nullptr;
book* book_ = nullptr;
book* navigation_book_ = nullptr;
club_handler* club_handler_ = nullptr;
config* config_ = nullptr;
contest_data* contest_data_ = nullptr;
cty_data* cty_data_ = nullptr;
eqsl_handler* eqsl_handler_ = nullptr;
extract_data* extract_records_ = nullptr;
fields* fields_ = nullptr;
fllog_emul* fllog_emul_ = nullptr;
import_data* import_data_ = nullptr;
intl_dialog* intl_dialog_ = nullptr;
lotw_handler* lotw_handler_ = nullptr;
main_window* main_window_ = nullptr;
menu* menu_ = nullptr;
qrz_handler* qrz_handler_ = nullptr;
qsl_dataset* qsl_dataset_ = nullptr;
qso_manager* qso_manager_ = nullptr;
rig_data* rig_data_ = nullptr;
spec_data* spec_data_ = nullptr;
status* status_ = nullptr;
stn_data* stn_data_ = nullptr;
stn_window* stn_window_ = nullptr;
tabbed_forms* tabbed_forms_ = nullptr;
ticker* ticker_ = nullptr;
toolbar* toolbar_ = nullptr;
url_handler* url_handler_ = nullptr;
wsjtx_handler* wsjtx_handler_ = nullptr;
wx_handler* wx_handler_ = nullptr;

//! List of files most recently opened. Maximum: 4 files. 
std::list<std::string> recent_files_;
//! \endcond
//! Flag to prevent more than one closure process at the same time.
bool closing_ = false;

//! Flag to mark everything loaded.
bool initialised_ = false;

//! Time loaded.
time_t session_start_ = (time_t)0;

//! Previous frequency.
double prev_freq_ = 0.0;

//! Filename in arguments.
char* filename_ = nullptr;

//! File is new (neither in argument or settings.
bool new_file_ = false;

//! Main logo.
Fl_PNG_Image main_icon_("ZZALOG_ICON", ___rose_png, ___rose_png_len);

//! Using backp.
bool using_backup_ = false;

//! Sticky switches mesasge.
std::string sticky_message_ = "";

//! Common seed to use in password encryption - maintaned with sessions.
uint32_t seed_ = 0;

//! Development directory
std::string development_directory_;

//! Default location for auto-generated compile fodder
std::string default_code_directory_ = "";

//! Do not close banner. Kept \p false unless banner is not deleted at ZZALOG closure in error cases.
bool keep_banner_ = false;

//! This run is a new installation
bool new_installation_ = false;

// Get the backup filename
std::string backup_filename(std::string source) {
	settings top_settings;
	settings behav_settings(&top_settings, "Behaviour");
	settings backup_settings(&behav_settings, "Backup");
	// Get back-up directory
	std::string backup;
	backup_settings.get<std::string>("Path", backup, "");
	while (backup.length() == 0) {
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select directory for backup");
		if (chooser->show() == 0) {
			backup = chooser->filename();
		}
		delete chooser;
	}
	// Save the result of the chooser
	backup_settings.set<std::string>("Path", backup);

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
	std::string suffix = source.substr(last_period);
	std::string base_name;
	if (last_slash == std::string::npos) {
		base_name = source.substr(0, last_period);
	}
	else {
		base_name = source.substr(last_slash + 1, last_period - last_slash - 1);
	}
	std::string timestamp = now(false, "%Y%m%d_%H%MZ");
	backup += base_name + "_" + timestamp + suffix;
	return backup;
}

// Restore from last backup
void restore_backup() {
	std::string filename = book_->filename();
	// Remove existing book
	status_->misc_status(ST_WARNING, "LOG: Closing current book!");
	menu::cb_mi_file_new(nullptr, nullptr);
	settings top_settings;
	settings behav_settings(&top_settings, "Behaviour");
	settings backup_settings(&behav_settings, "Backup");
	std::string backup;
	backup_settings.get<std::string>("Last Backup", backup, "");
	// Get backup data
	READ_ONLY = true;
	book_->load_data(backup);
}

// This callback intercepts the close command and performs checks and tidies up
// Updates recent files settings
void cb_bn_close(Fl_Widget* w, void*v) {
	// The close button can only be clicked at certain times in the closure process
	// when Fl::wait() is called.
	if (closing_) {
		status_->misc_status(ST_WARNING, "ZZALOG: Already closing!");
	}
	else {
		closing_ = true;
		// Stop the ticker
		ticker_->stop_all();
		banner_->show();
		banner_->redraw();
		Fl::check();
		status_->misc_status(ST_NOTE, "ZZALOG: Closing...");
		// Currently modifying a (potentially new) record
		if (book_ && (book_->is_dirty_record(book_->get_record()) || book_->new_record()) ) {
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
				// Wait for the request std::queue to empty
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
		if (book_ && (book_->is_dirty() || new_installation_)) {
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
		settings top_settings;
		settings view_settings(&top_settings, "Views");
		settings main_settings(&view_settings, "Main Window");
		main_settings.set("Left", main_window_->x_root());
		main_settings.set("Top", main_window_->y_root());
		main_settings.set("Width", main_window_->w());
		main_settings.set("Height", main_window_->h());

		// Save sticky switches
		save_switches();

		// Hide all the open windows - this will allow Fl to close the app.
		Fl_Window* wx = Fl::first_window();
		for (; wx; wx = Fl::first_window()) {
			// Keep the banner showing if we need to see a severe or fatal error.
			wx->hide();
		}
	}
}

// Callback to parse arguments
// See show_help() for meaning of switches
int cb_args(int argc, char** argv, int& i) {
	int i_orig = i;
	// auto save
	if (strcmp("-a", argv[i]) == 0 || strcmp("--auto_save", argv[i]) == 0) {
		AUTO_SAVE = true;
		AUTO_SAVE_S = true;
		i += 1;
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
			else if (strcmp("r", argv[i]) == 0 || strcmp("run", argv[i]) == 0) {
				DEBUG_RIGS = true;
				i += 1;
			}
			else if (strcmp("d", argv[i]) == 0 || strcmp("decode", argv[i]) == 0) {
				DEBUG_PARSE = true;
				i += 1;
			}
			// Not processed any parameter
			if (i == save_i) debugs = false;
		}
	}
	// New file
	else if (strcmp("-e", argv[i]) == 0 || strcmp("--new", argv[i]) == 0) {
		NEW_BOOK = true;
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
		DARK_S = true;
		i += 1;
	}
	// Dark
	else if (strcmp("-l", argv[i]) == 0 || strcmp("--light", argv[i]) == 0) {
		DARK = false;
		DARK_S = true;
		i += 1;
	}
	// Resume session
	else if (strcmp("-m", argv[i]) == 0 || strcmp("--resume", argv[i]) == 0) {
		RESUME_SESSION = true;
		i += 1;
	}
	// auto upload
	else if (strcmp("-n", argv[i]) == 0 || strcmp("--noisy", argv[i]) == 0) {
		AUTO_UPLOAD = true;
		AUTO_UPLOAD_S = true;
		i += 1;
	}
	// Private log - do not update recent files
	else if (strcmp("-p", argv[i]) == 0 || strcmp("--private", argv[i]) == 0) {
		PRIVATE = true;
		i += 1;
	}
	// No auto upload
	else if (strcmp("-q", argv[i]) == 0 || strcmp("--quiet", argv[i]) == 0) {
		AUTO_UPLOAD = false;
		AUTO_UPLOAD_S = true;
		i += 1;
	}
	// Look for read_only (-r or --read_only)
	else if (strcmp("-r", argv[i]) == 0 || strcmp("--read_only", argv[i]) == 0) {
		READ_ONLY = true;
		i += 1;
	}
	// Look for test mode (-t or --test) 
	else if (strcmp("-t", argv[i]) == 0 || strcmp("--test", argv[i]) == 0) {
		AUTO_UPLOAD = false;
		AUTO_SAVE = false;
		AUTO_UPLOAD_S = true;
		AUTO_SAVE_S = true;
		i += 1;
	}
	// Look for normal mode (-u or --usual) 
	else if (strcmp("-u", argv[i]) == 0 || strcmp("--usual", argv[i]) == 0) {
		AUTO_UPLOAD = true;
		AUTO_SAVE = true;
		AUTO_UPLOAD_S = true;
		AUTO_SAVE_S = true;
		i += 1;
	}
	// Version
	else if (strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) {
		DISPLAY_VERSION = true;
		i += 1;
	}
	// No auto save
	else if (strcmp("-w", argv[i]) == 0 || strcmp("--wait_save", argv[i]) == 0) {
		AUTO_SAVE = false;
		AUTO_SAVE_S = true;
		i += 1;
	}
	// Reset configuration
	else if (strcmp("-x", argv[i]) == 0 || strcmp("--reset", argv[i]) == 0) {
		i += 1;
		while (argv[i] && argv[i][0] != '-') {
			if (strcmp("adif", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_ADIF;
			}
			if (strcmp("apps", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_APPS;
			}
			if (strcmp("bandplan", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_BAND;
			}
			if (strcmp("contest", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_TEST;
			}
			if (strcmp("country", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_CALL;
			}
			if (strcmp("fields", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_FLDS;
			}
			if (strcmp("icons", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_ICON;
			}
			if (strcmp("intl", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_INTL;
			}
			if (strcmp("rigs", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_RIGS;
			}
			if (strcmp("settings", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_SETT;
			}
			if (strcmp("station", argv[i]) == 0) {
				DEBUG_RESET_CONFIG |= DEBUG_RESET_STN;
			}
			if (strcmp("all", argv[i]) == 0) {
				DEBUG_RESET_CONFIG = DEBUG_RESET_ALL;
			}
			i += 1;
		}
	}
	if (i == i_orig ) {
		// Not processed any argumant
		if (*argv[i] == '-') {
			int i_fltk = Fl::arg(argc, argv, i);
			if (i_fltk == 0) {
				printf("ZZALOG: Unrecognised switch %s", argv[i]);
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
  	"\t-d|--debug [mode...]\n"
	"\t\tc|curl\tincrease verbosity from libcurl\n"
	"\t\t\tnoc|nocurl\n"
	"\t\td|decode\tShow callsign decoding\n"
	"\t\te|errors\tprovide more details on errors\n"
	"\t\t\tnoe|noerrors\n"
	"\t\th=N|hamlib=N\tSet hamlib debug level (default ERRORS)\n"
	"\t\tp|pretty\tDisplay formated status message (Needs terminal support)\n"
	"\t\tnop|nopretty\n"
	"\t\tq|quick\tShorten long timeout and polling intervals\n"
	"\t\tr|rig\tPrint rig diagnostics\n"
	"\t\tt|threads\tProvide debug tracing on std::thread use\n"
	"\t\t\tnot|nothreads\n"
	"\t-e|--new\tCreate new file\n"
	"\t-h|--help\tPrint this\n"
	"\t-k|--dark\tDark mode (sticky)\n"
	"\t-l|--light\tLight mode (sticky)\n"
	"\t-m|--resume\tResume the previous session\n"
	"\t-n|--noisy\tDo publish QSOs to online sites (sticky)\n"
	"\t-p|--private\tDo not update recent files std::list\n"
	"\t-q|--quiet\tDo not publish QSOs to online sites (sticky)\n"
	"\t-r|--read_only\tOpen file in read only mode\n"
	"\t-t|--test\tTest mode: infers -q -w\n"
	"\t-u|--usual\tNormal mode: infers -a -n\n"
	"\t-w|--wait_save\tDo not automatically save each change (sticky)\n"
	"\t-x|--reset [data]...\tReset configuration data (more than 1 allowed\n"
	"\t\tadif\tADIF specification file (all.json)\n"
	"\t\tapps\tApps configuration file (apps.json)\n"
	"\t\tbandplan\tBand-plan data (band_plan.json)\n"
	"\t\tcontest\tContest data (contests.json)\n"
	"\t\tcountry\tCountry data (cty.xml, cty.csv, prefix.lst)\n"
	"\t\tfields\tFields data (fields.json)\n"
	"\t\ticons\tToolbar icons (various)\n"
	"\t\tintl\tInternational character set (intl_chars.txt)\n"
	"\t\trigs\tRig configuration data (rigs.json)\n"
	"\t\tsettings\tZZALOG configuration (ZZALOG.json)\n"
	"\t\tstation\tOperator/QTH/Callsign configuration (station.json)\n"
	"\t\tall\tAll files\n"
	"\n";
	printf(text);
}

// Use supplied argument, or read the latest file from settings or open file chooser if that's an empty std::string
std::string get_file(char * arg_filename) {
	std::string result = "";
	if (!arg_filename || !(*arg_filename)) {
		std::string filename = "";
		if (recent_files_.size()) filename = recent_files_.front();
		if (!filename.length()) {
			status_->misc_status(ST_WARNING, "ZZALOG: No log file - assuming a new installation.");
			stn_default defaults = stn_data_->defaults();
			std::string def_filename = to_lower(defaults.callsign) + ".adi";
			Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
			chooser->title("Select log file name");
			chooser->preset_file(def_filename.c_str());
			chooser->filter("ADI Files\t*.adi\nADX Files\t*.adx");
			if (chooser->show() == 0) {
				result = chooser->filename();
			}
			delete chooser;
			NEW_BOOK = true;
		}
		else {
			result = filename;
		}
	}
	else {
		result = arg_filename;
	}
	return result;
}

// Add some global properties
void add_properties() {
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings tip_settings(&view_settings, "Tooltip");
	Fl_Font font;
	Fl_Fontsize size;
	float duration;
	tip_settings.get("Duration", duration, (float)TIP_SHOW);
	tip_settings.get("Font Name", font, 0);
	tip_settings.get("Font Size", size, FL_NORMAL_SIZE);
	// Set the default tooltip properties
	Fl_Tooltip::size(size);
	Fl_Tooltip::font(font);
	Fl_Tooltip::delay(duration);
}

// Get the recent files from settings
void recent_files() {
	recent_files_.clear();
	settings top_settings;
	settings behav_settings(&top_settings, "Behaviour");
	behav_settings.get("Recent Files", recent_files_, {});
	if (recent_files_.size() > 4) {
		recent_files_.resize(4);
	}
}

// read in the prefix and adif reference data
void add_data() {
	// Note closing can get std::set during any of the below actions.
	if (!closing_) {
		// add ADIF specification data.
		spec_data_ = new spec_data;
		// Check if loaded
		if (!spec_data_->valid()) {
			// This sets a callback to close the app
			status_->misc_status(ST_FATAL, "Do not have a valid ADIF reference - check installation");
			Fl::check();
		}
		else {
			// This can only be done once it has been fully created
			spec_data_->process_bands();
			// Draw the specification view
			((spec_tree*)tabbed_forms_->get_view(OT_ADIF))->populate_tree(false);
		}
	}
	if (!closing_) {
		if (!club_handler_) club_handler_ = new club_handler;
		// Get pfx_data
		cty_data_ = new cty_data;
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
	// Add the Station details database
	if (!closing_) {
		stn_data_ = new stn_data;
		stn_window_ = new stn_window();
		stn_window_->hide();
		stn_data_->load_data();
	}
	// Add the contest details database
	if (!closing_) {
		contest_data_ = new contest_data;
	}
}

// read in the log data
void add_book(char* arg) {
	if (!closing_) {
		// Create the book options and std::set them in the forms
		book_ = new book;
		navigation_book_ = book_;
		import_data_ = new import_data;
		extract_records_ = new extract_data;
		// Tell the views that a book now exists
		tabbed_forms_->books();

		if (!NEW_BOOK || filename_) {
			// Get filename and load the data
			std::string log_file = get_file(arg);


			if (!book_->load_data(log_file)) {
				if (!NEW_BOOK) {
					settings top_settings;
					settings behav_settings(&top_settings, "Behaviour");
					settings backup_settings(&behav_settings, "Backup");
					std::string backup;
					backup_settings.get<std::string>("Last Backup", backup, "");
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
						snprintf(msg, sizeof(msg), "ZZALOG: Load %s failed, no backup loaded - assume a new logbook", log_file.c_str());
						status_->misc_status(ST_WARNING, msg);
						set_recent_file(log_file);
						book_->set_filename(log_file, true);

					}
				} else {
					char msg[100];
					snprintf(msg, sizeof(msg), "ZZALOG: Load %s failed, no backup loaded - assume a new logbook", log_file.c_str());
					status_->misc_status(ST_WARNING, msg);
					set_recent_file(log_file);
					book_->set_filename(log_file, true);

				}
			} else {
				// Move this file to the top of the recent file std::list
				set_recent_file(log_file);
				char msg[128];
				if (new_file_) {
					if (!book_->store_data(log_file, true)) {
						snprintf(msg, sizeof(msg), "ZZALOG: Failed to create %s", log_file.c_str());
						status_->misc_status(ST_ERROR, msg);
					}
				}
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
			std::string version = PROGRAM_VERSION;
			if (DEVELOPMENT_MODE) version += " DEVELOPMENT";
			snprintf(l, sizeof(l), "%s %s: Operating Dashboard", PROGRAM_ID.c_str(), version.c_str());
			qso_manager_ = new qso_manager(10, 10);
			qso_manager_->copy_label(l);
		}
		status_->misc_status(ST_NOTE, "DASH: Opened");
		qso_manager_->hide();
	}
}

// Set the text in the main window label
void main_window_label(std::string text) {
	// e.g. ZZALOG 3.0.0: <filename> - PROGRAM_VERSION includes (Debug) if compiled under _DEBUG
	std::string label = PROGRAM_ID + " " + PROGRAM_VERSION;
	if (DEVELOPMENT_MODE) label += " DEVELOPMENT";
	label += ": " + text;
	main_window_->copy_label(label.c_str());
}

// Create the main window
void create_window() {
	// Create the main window
	main_window_ = new main_window(WIDTH, HEIGHT);
	main_window_label("");
	// add callback to intercept close command
	main_window_->callback(cb_bn_close);
	main_window_->hide();

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
	// The main views - this is a std::set of tabs with each view
	tabbed_forms_ = new tabbed_forms(0, curr_y, WIDTH, HEIGHT - curr_y - FOOT_HEIGHT);
	main_window_->add(tabbed_forms_);
	curr_y += tabbed_forms_->h();
	// Add a footer (with copyright
	Fl_Box* footer = new Fl_Box(0, curr_y, WIDTH, FOOT_HEIGHT);
	footer->copy_label(std::string(COPYRIGHT + " " + CONTACT + "    ").c_str());
	footer->labelsize(FL_NORMAL_SIZE - 1);
	footer->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	main_window_->add(footer);
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
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings main_settings(&view_settings, "Main Window");
	main_settings.get<int>("Left", left, 0);
	main_settings.get<int>("Top", top, 100);
	main_settings.get<int>("Width", width, WIDTH);
	main_settings.get<int>("Height", height, HEIGHT);
	// Only allow the views to resize fully - the bars will resize horizontally
	main_window_->resizable(tabbed_forms_);
	// Get minimum resizing from all the children - horizontal limited by views and toolbar
	int min_w = std::max<int>(tabbed_forms_->min_w(), toolbar_->min_w());
	// Vertical limited by view, the bars remain a fixed height
	int min_h = tabbed_forms_->min_h() + toolbar_->h() + menu_->h();
	main_window_->size_range(min_w, min_h);
	// Set the size to the setting or minimum specified by the view + bars if that's larger
	int rx = left;
	int ry = top;
	int rw = std::max<int>(min_w, width);
	int rh = std::max<int>(min_h, height);
	int sx, sy, sw, sh;
	Fl::screen_work_area(sx, sy, sw, sh);
	if (rx < sx) rx = sx;
	else if (rx + rw > sx + sw) rx = std::max<int>(0, sx + sw - rw);
	if (ry < sy) ry = sy;
	else if (ry + rh > sy + sh) ry = std::max<int>(20, sy + sh - rh);
	main_window_->resize(rx, ry, rw, rh);
}

// Tidy memory
void tidy() {
	// Tidy memory - this is not perfect
	// From inspection of the code - calling this a second time frees the memory
	fl_message_title_default(nullptr);
	delete wx_handler_;
	delete config_;
	delete qso_manager_;
	delete rig_data_;
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
	delete contest_data_;
	delete stn_data_;
	delete qsl_dataset_;
	delete band_data_;
	delete intl_dialog_;
	// This will be used in toolbar_
	intl_dialog_ = nullptr;
	delete spec_data_;
	delete cty_data_;
	delete tabbed_forms_;
	delete toolbar_;
	delete fields_;
	if (closing_) status_->misc_status(ST_OK, "ZZALOG: Closed");
	delete status_;
	delete menu_;
	delete main_window_;
}

// Map argument letter to colour name
std::map<uchar, std::string> colours = {
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
	// Create a std::string to hold all the info
	int length = 20;
	for (int i = 0; i < argc; i++) {
		length += strlen(argv[i]);
	}
	char message[256];
	memset(message, 0, sizeof(message));
	// Generate the std::string
	strcpy(message, "ZZALOG: ");
	for (int i = 0; i < argc; i++) {
		strcat(message, argv[i]);
		strcat(message, " ");
	}
	strcat(message, "Started");
	status_->misc_status(ST_NOTE, message);
	snprintf(message, sizeof(message), "ZZALOG: %s %s", 
		PROGRAM_ID.c_str(), PROGRAM_VERSION.c_str());
	status_->misc_status(ST_NOTE, message);
	if (DEVELOPMENT_MODE) {
		status_->misc_status(ST_WARNING, "ZZALOG: Development mode");
	}
	snprintf(message, sizeof(message), "ZZALOG: Compiled %s", TIMESTAMP.c_str());
	status_->misc_status(ST_NOTE, message);

	if (DEBUG_ERRORS) status_->misc_status(ST_NOTE, "ZZALOG: -d e - Displaying debug error messages");
	if (DEBUG_THREADS) status_->misc_status(ST_NOTE, "ZZALOG: -d t - Displaying std::thread debug messages");
	if (DEBUG_CURL) status_->misc_status(ST_NOTE, "ZZALOG: -d c - Displaying more verbosity from libcurl");
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
	if (PRIVATE) status_->misc_status(ST_WARNING, "ZZALOG: -p - This file not being noted on recent files std::list");
	if (DARK) status_->misc_status(ST_NOTE, "ZZALOG: -k - Opening in dark mode");
	else status_->misc_status(ST_NOTE, "ZZALOG: -l - Opening in normal FLTK colours");
	snprintf(message, sizeof(message), "ZZALOG: -x (value = %x) - Reset file (bit signficant)",
		DEBUG_RESET_CONFIG);
	if (DEBUG_RESET_CONFIG) status_->misc_status(ST_WARNING, message);
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
#ifndef _WIN32
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, "Courier New");
	Fl::set_font(FL_COURIER_BOLD, "Courier New Bold");
	Fl::set_font(FL_COURIER_ITALIC, "Courier New Italic");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "Courier New Bold Italic");
	// Use liberation fonts as closest to Windows fonts
	Fl::set_font(FL_TIMES,            "Liberation Serif");
	Fl::set_font(FL_TIMES_BOLD,       "Liberation Serif Bold");
	Fl::set_font(FL_TIMES_ITALIC,     "Liberation Serif Italic");
	Fl::set_font(FL_TIMES_BOLD_ITALIC,"Liberation Serif Bold Italic");	
	// Fl::set_font(FL_HELVETICA,            "Liberation Sans");
	// Fl::set_font(FL_HELVETICA_BOLD,       "Liberation Sans Bold");
	// Fl::set_font(FL_HELVETICA_ITALIC,     "Liberation Sans Italic");
	// Fl::set_font(FL_HELVETICA_BOLD_ITALIC,"Liberation Sans Bold Italic");	
#else 
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, " Courier New");
	Fl::set_font(FL_COURIER_BOLD, "BCourier New");
	Fl::set_font(FL_COURIER_ITALIC, "ICourier New");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "PCourier New");
#endif
	// Add label symbols
	fl_add_symbol("eyeshut", &draw_eyeshut, true);
	fl_add_symbol("eyeopen", &draw_eyeopen, true);
	fl_add_symbol("calendar", &draw_calendar, true);
	fl_add_symbol("mail", &draw_mail, true);
	// Default message properties
	fl_message_size_ = FL_NORMAL_SIZE;
	fl_message_font_ = 0;
	fl_message_title_default(PROGRAM_ID.c_str());
	// Set foreground and background colours
	if (DARK) {
		Fl::foreground(240, 240, 240);             // 15/16 White
		Fl::background2(16, 16, 16);               // 1/16 white
		Fl::background(32, 32, 32);                // 1/8 white
	}
	else {
		Fl::foreground(16, 16, 16);                // 1/16 white
		Fl::background2(240, 240, 240);            // 15/16 white
		Fl::background(192, 192, 192);             // 3/4 white
	}
	// Default scrollbar
	Fl::scrollbar_size(10);
}

// Some switches get saved between sessions - so-called sticky switches
void read_saved_switches() {
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings overall_settings(&view_settings, "Overall");
	settings behav_settings(&top_settings, "Behaviour");
	// Read all the sticky switches
	char msg[128];
	strcpy(msg, "ZZALOG: Sticky switches: ");
	if (!DARK_S) {
		overall_settings.get<bool>("Dark Mode", DARK, false);
		if (DARK) strcat(msg, "-k ");
		else strcat(msg, "-l ");
	}
	if (!AUTO_UPLOAD_S) {
		behav_settings.get<bool>("Update per QSO", AUTO_UPLOAD, false);
		if (AUTO_UPLOAD) strcat(msg, "-n ");
		else strcat(msg, "-q ");
	}
	if (!AUTO_SAVE_S) {
		behav_settings.get<bool>("Save per QSO", AUTO_SAVE, false);
		if (AUTO_SAVE) strcat(msg, "-a ");
		else strcat(msg, "-w ");
	}

	sticky_message_ = msg;
}

// Save "sticky" switches
void save_switches() {
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings overall_settings(&view_settings, "Overall");
	settings behav_settings(&top_settings, "Behaviour");
	overall_settings.set<bool>("Dark Mode", DARK);
	behav_settings.set<bool>("Update per QSO", AUTO_UPLOAD);
	behav_settings.set("Save per QSO", AUTO_SAVE);
}

// Load all the hamlib data, and then the rig connection details
void load_rig_data() {
	rig_set_debug(HAMLIB_DEBUG_LEVEL);
	rig_load_all_backends();
	if (!closing_) {
		rig_data_ = new rig_data;
	}
}

// The main app entry point
int main(int argc, char** argv)
{
	// Allow the main std::thread to respond to Fl::awake() requests
	Fl::lock();
	// 
	printf("%s %s: Loading...\n", PROGRAM_ID.c_str(), PROGRAM_VERSION.c_str());
	// Parse command-line arguments - accept FLTK standard arguments and custom ones (in cb_args)
	int i = 1;
	Fl::args(argc, argv, i, cb_args);
	// Set the default data directories
	bool development;
	file_holder_ = new file_holder(argv[0], DEVELOPMENT_MODE);
	// Read any switches that stick between calls
	read_saved_switches();
	customise_fltk();

	// Create the ticker first of all
	ticker_ = new ticker();

	if (DISPLAY_VERSION) {
#ifndef WIN32
		std::string version = PROGRAM_VERSION;
		if (DEVELOPMENT_MODE) version += " DEVELOPMENT";
		// Display version
		printf("%s Version %s Compiled %s\n", 
			PROGRAM_ID.c_str(), 
			version.c_str(),
			TIMESTAMP.c_str());
		curl_version_info_data* data = curl_version_info(CURLVERSION_LAST);
		printf("|-With libraries\n  |- hamlib (%s)\n  |- FLTK (%d.%d.%d)\n  |- Curl (%s)\n",
			rig_version(),
			FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION,
				data->version); 
#else
		about_dialog* dlg = new about_dialog;
		dlg->show();
		while (dlg->visible()) Fl::check();
#endif
		return 0;
	}
	if (HELP) {
		// Help requested - display help text and exit
		show_help();
		return 0;
	}

	// Ctreate status to handle status messages
	status_ = new status();
	// Create banner
	banner_ = new banner(400, 200);
	std::string title = PROGRAM_ID + " " + PROGRAM_VERSION;
	if (DEVELOPMENT_MODE) title += " DEVELOPMENT";
	banner_->copy_label(title.c_str());

	// Now display sticky switch message
	status_->misc_status(ST_NOTE, sticky_message_.c_str());	
	print_args(argc, argv);

	// Read the fields data
	fields_ = new fields;

	// Create window
	create_window();
	add_properties();
	recent_files();

	// add the various drawn items
	int curr_y = 0;
	add_widgets(curr_y);
	// Resize the window
	resize_window();
	// Read in reference data - uses progress
	add_data();
	Fl::check();
	// Read in log book data - uses progress - use supplied argument for filename
	add_book(filename_);
	printf("%s\n", main_window_->label());
	Fl::check();
	// Connect to the rig - load all hamlib backends once only here
	load_rig_data();
	// Add qso_manager
	add_dashboard();
	// Add config - uses dynamic enumerated ADIF fields so needs the book loaded. and manager running
	if (!closing_) {
		config_ = new config(WCONFIG, HCONFIG, "Configuration");
		config_->hide();
	}
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
		// now show the window
		main_window_->show(argc, argv);
		qso_manager_->show();
		// Run the application until it is closed
		code = Fl::run();
	}
	// Delete everything we've created
	tidy();
	if (keep_banner_) {
		banner_->allow_close();
		banner_->show();
		while (banner_ && banner_->visible()) Fl::check();
	}
	return code;
}

// Copy existing data to back up file. force = true used by menu command, 
void backup_file() {
	std::string source = book_->filename();
	std::string backup = backup_filename(source);
	char* message = new char[backup.length() + 25];
	sprintf(message, "BACKUP: Writing %s", backup.c_str());
	status_->misc_status(ST_NOTE, message);
	delete[] message;
	// In and out streams
	std::ifstream in(source);
	in.seekg(0, in.end);
	int length = (int)in.tellg();
	const int increment = 8000;
	in.seekg(0, in.beg);
	status_->progress(length, OT_MAIN, "Copying data to backup", "bytes");
	std::ofstream out(backup);
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
		settings top_settings;
		settings behav_settings(&top_settings, "Behaviour");
		settings backup_settings(&behav_settings, "Backup");
		backup_settings.set("Last Backup", backup);
	}
}

// Add the current file to the recent files std::list
void set_recent_file(std::string filename) {
	// Do not add to recent file std::list if using backup or CLI inihibited
	if (!PRIVATE && !using_backup_ && filename.length()) {

		// Add or move the file to the front of std::list
		recent_files_.remove(filename);
		recent_files_.push_front(filename);

		// Update recent files in the settings
		settings top_settings;
		settings behav_settings(&top_settings, "Behaviour");
		if (recent_files_.size() > 4) {
			recent_files_.resize(4);
		}
		behav_settings.set("Recent Files", recent_files_);

		menu_->add_recent_files();

	}

}

void open_html(const char* file) {
	// OS dependent code to open a document
	std::string full_filename = file_holder_->get_directory(DATA_HTML) +
		"userguide/html/" + std::string(file);
	open_doc(full_filename);
}

void open_pdf() {
	std::string full_filename = file_holder_->get_directory(DATA_HTML) +
		"userguide/ZZALOG.pdf";
	open_doc(full_filename);
}

void open_doc(std::string full_filename) {
#ifdef _WIN32
	HINSTANCE result = ShellExecute(NULL, "open", full_filename.c_str(), NULL, NULL, SW_SHOWNORMAL);
	if ((intptr_t)result <= 32) {
		char msg[128];
		snprintf(msg, sizeof(msg), "ZZALOG: Error opening HTML %s. Error code: %d", 
			full_filename.c_str(),
			(int)(intptr_t)result);
		status_->misc_status(ST_ERROR, msg);
	}
#else 
	std::string cmd = "xdg-open \"" + full_filename + "\"";
	int res = system(cmd.c_str());
	if (res != 0) {
		char msg[128];
		snprintf(msg, sizeof(msg), "ZZALOG: Error opening HTML %s. Error code: %d", 
			full_filename.c_str(),
			res);
		status_->misc_status(ST_ERROR, msg);
	}
#endif
}

std::string recent_file(int n) {
	if (recent_files_.size() <= n) {
		return "";
	}
	else {
		int ix = 0;
		for (auto it = recent_files_.begin(); ; it++, ix++) {
			if (ix == n) {
				return *it;
			}
		}
	}
}
#pragma once


#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include "about_dialog.h"
#include "band_data.h"
#include "band_window.h"
#include "banner.h"
#include "book.h"
#include "club_handler.h"
#include "club_stn_dlg.h"
#include "config.h"
#include "contest_data.h"
#include "cty_data.h"
#include "eqsl_handler.h"
#include "extract_data.h"
#include "fields.h"
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

// FLTK header files
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
// included to allow windows specifics to be called
#include <FL/platform.H>




//! \mainpage ZZALOG Code Documentation
//! 
//! This document describes the code interfaces between the constituent parts
//! of ZZALOG. For the user interface see <A class="el" HREF=file:../../userguide/html/index.html>Userguide.</A>
//! 
//! \section revision Release History
//! For release details see <A class="e1" HREF=file:../../userguide/html/release_notes.html>Release Notes.</A>
//! \section ack Acknowledgements
//! ZZALOG uses the following third-party libraries:
//! - <A HREF=https://www.fltk.org/doc-1.4/index.html>FLTK</A>: Graphical user interface library.
//! - <A HREF=https://github.com/Hamlib/Hamlib/wiki/Hamlib>hamlib</A>: CAT Interface library.
//! - <A HREF=https://curl.se/libcurl/c/libcurl.html>libcurl</A>: HTTP support library.
//! - <A HREF=https://json.nlohmann.me/>nlohmann/json</A>: JSON support library.
//!
//! ZZALOG uses reference data from the following third-parties
//! - Clublog.org (https://clublog.org) Country data.
//! - Country-files.com (https://www.country-files.com) Country data.
//! - DxAtlas (http://dxatlas.com) Prefix data.
//! - ADIF (http://adif.org) ADIF Specification (JSON).
//! 
//! \copyright Philip Rose GM3ZZA 2018-2025. All rights reserved.
//!
//! ZZALOG is based in part on the work of the FLTK project <A HREF=https://www.fltk.org>https://www.fltk.org</A>.


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
//! Program identifier: used in ADIF PROGRAM_ID field and Fl_Preferences.
std::string PROGRAM_ID = "ZZALOG";
//! Short-form program identifier.
std::string PROG_ID = "ZLG";
//! Program version. 
std::string PROGRAM_VERSION = "3.6.7+";
//! Program vendor: used for Fl_Preferences.
std::string VENDOR = "GM3ZZA";

// Debug switches
//! Print errors - std::set by "-d e"
bool DEBUG_ERRORS = true;
//! Print std::thread debugging messages - std::set by "-d t"
bool DEBUG_THREADS = false;
//! Print libcurl debugging messages - std::set by "-d c"
bool DEBUG_CURL = false;
//! Reduce long duration tiemouts and waits - std::set by "-d q"
bool DEBUG_QUICK = false;
//! Print rig access debugging messages - std::set by "-d r"
bool DEBUG_RIGS = false;
//! Print callsign parsing messages - std::set by "-d d"
bool DEBUG_PARSE = false;
//! Set hamlib debugging verbosity level - std::set by "-d h=<level>"
rig_debug_level_e HAMLIB_DEBUG_LEVEL = RIG_DEBUG_ERR;

// Operation switches - _S versions used to override sticky switch
//! Automatically upload QSOs to QSL sites - std::set by "-n"
bool AUTO_UPLOAD = true;
//! Version of \p AUTO_UPLOAD read from settings
bool AUTO_UPLOAD_S = false;
//! Automatically save QSO record after each change - std::set by "-a"
bool AUTO_SAVE = true;
//! Version of \p AUTO_SAVE read from settings.
bool AUTO_SAVE_S = false;
//! Dark mode: Dark background, light forreground - std::set by "-k"
bool DARK = false;
//! Version of \p DARK read from settings.
bool DARK_S = false;
//! Print version details instead of running ZZALOG - std::set by "-v"
bool DISPLAY_VERSION = false;
//! Print command-line interface instead of running ZZALOG - std::set by "-h"
bool HELP = false;
//! Start with an empty logbook - std::set by "-e"
bool NEW_BOOK = false;
//! Do not add file to recent file std::list - std::set by "-p"
bool PRIVATE = false;
//! Open file in read-only mode - std::set by "-r"
bool READ_ONLY = false;
//! Resum logging including previous session - std::set by "-m"
bool RESUME_SESSION = false;

//! Access to FLTK global attribute to std::set default text size throughout ZZALOG.
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
tabbed_forms* tabbed_forms_ = nullptr;
ticker* ticker_ = nullptr;
toolbar* toolbar_ = nullptr;
url_handler* url_handler_ = nullptr;
wsjtx_handler* wsjtx_handler_ = nullptr;
wx_handler* wx_handler_ = nullptr;

//! List of files most recently opened. Maximum: 4 files. 
std::list<std::string> recent_files_;
//! \endcond

// Forward declarations
//! Get the backup filename.
//! \return filename.
std::string backup_filename(std::string source);
//! Restores file from backup location.
void restore_backup();

//! Callback for main_window and qso_manager.

//! \param w calling widget.
//! \param v not used.
static void cb_bn_close(Fl_Widget* w, void* v);

//! Callback used by FLTK when parsing command-line arguments.

//! \param argc count of arguments.
//! \param argv array of arguments.
//! \param i index of argument to decode.
//! \return index of next argumant to decode.
int cb_args(int argc, char** argv, int& i);

//! Print the help message
void show_help();

//! Get the specified logbook filename

//! \param arg_filename filename supplied by argument.
//! \return selected filename: argument if specified otherwise most recently file opened.
std::string get_file(char* arg_filename);

//! Add some global properties
void add_properties();

//! Read the recent file std::list from the settings.
void recent_files();

//! Read the following data items:

//! - ADIF specification.
//! - Callsign parsing database.
//! - International character std::set.
//! - Bandplan data.
//! - QSL Designs.
//! - Configured rig data.
//! - Contest specifications.
void add_data();

//! Read the logbook data.

//! \param arg filename supplied as command-line argument.
void add_book(char* arg);

//! Instantiate the following external protocol handlers:

//! - Generic HTTP and UDP handler.
//! - eQSL.cc interface.
//! - Logbook of the World interface.
//! - QRZ.com interface.
//! - Clublog.org interface.
//! - WSJT-X interface.
//! - FlDigi interface (FlLog emulator).
//! - openweather.org interface
void add_qsl_handlers();

//! Instantiate the QSO Manager (Dashboard)
void add_dashboard();

//! Label the main_window window as "[PROGRAM_ID] [PROGRAM_VERSION]: \a text".

//! \param text 
void main_window_label(std::string text);

//! Instantiate main_window.
void create_window();

//! Add the component widgets to the main_window.

//! \param curr_y Y-coordinate of last widget added plus its height.
void add_widgets(int& curr_y);

//! Resize and reposition main_window to as it was when last opened or nearest

//! position on current screem.
void resize_window();

//! Delete all created data items
void tidy();

//! Set the default icon for all windows.

//! \param arg0 not used.
void add_icon(const char* arg0);

//! Display the arguments in the status log.

//! \param argc number of arguments
//! \param argv array of arguments.
void print_args(int argc, char** argv);

//! Checks the supplied argument \a this_record is within the current session.

//! \param this_record QSO to check.
//! \return true if the QSO is within the surrent session.
bool in_current_session(record* this_record);

//! Customises various aspects when using FLTK widgets
void customise_fltk();

//! Read the sticky switches from the settings.
void read_saved_switches();

//! Save the sticky switches to the settings file.
void save_switches();

//! Open the settings file for saved configuration.

//! \return true if file was opened successfully, otherwise false.
bool open_settings();

//! Check whether system or user config (club or individual).
void check_settings();

//! Initialise hamlib
void load_rig_data();

//! Main program entry point.

//! \param argc number of command-line arguments
//! \param argv array of command-line arguments.
int main(int argc, char** argv);

//! Backs up file to separate location.
void backup_file();

//! Add the specified file to the recent files std::list.
void set_recent_file(std::string filename);

//! Open the user-guide at the specified page &lt;\a file&gt;.html.
void open_html(const char* file);

//! Open the file \p full_filename  with default application
void open_doc(std::string full_filename);

//! Open the PDF version of the User Guide
void open_pdf();

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

//! Default location for configuration files.
std::string default_data_directory_ = "";

//! Default location for documentatiom files.
std::string default_html_directory_ = "";

//! Default location for reference data
std::string default_ref_directory_ = "";

//! Do not close banner. Kept \p false unless banner is not deleted at ZZALOG closure in error cases.
bool keep_banner_ = false;

//! This run is a new installation
bool new_installation_ = false;

//! Default values of a station
stn_default station_defaults_;

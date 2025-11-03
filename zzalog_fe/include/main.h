#pragma once
//! \mainpage ZZALOG Code Documentation
//! 
//! This document describes the code interfaces between the constituent parts
//! of ZZALOG. For the user interface see <A class="el" HREF=file:../../userguide/html/index.html>Userguide.</A>
//! 
//! \section revision Release History
//! See <A class="e1" HREF=file:../../userguide/html/release_notes.html>Release Notes.</A>
//!
//! \section ack Acknowledgements
//! See <A class="e1" HREF=file:../../userguide/html/ack.html>Acknowledgements</A>
//!
//! \copyright Philip Rose GM3ZZA 2018-2025. All rights reserved.
//!
//! ZZALOG is based in part on the work of the FLTK project <A HREF=https://www.fltk.org>https://www.fltk.org</A>.

#include "hamlib/rig.h"

#include <list>
#include <string>

class band_data;
class band_window;
class banner;
class book;
class club_handler;
class config;
class contest_data;
class cty_data;
class eqsl_handler;
class extract_data;
class fields;
class fllog_emul;
class import_data;
class intl_dialog;
class lotw_handler;
class main_window;
class menu;
class qrz_handler;
class qsl_dataset;
class qso_manager;
class record;
class rig_data;
class spec_data;
class status;
class stn_data;
class stn_window;
class tabbed_forms;
class ticker;
class toolbar;
class url_handler;
class wsjtx_handler;
class wx_handler;
class Fl_PNG_Image;
class Fl_Widget;
enum rig_debug_level_e;

//! Program copyright - displayed in all windows.
extern std::string COPYRIGHT;
//! Third-party acknowledgments.
extern std::string PARTY3RD_COPYRIGHT;
//! Contact address for use in FLTK widget labels.
extern std::string CONTACT;
//! Contact address for use in general texts.
extern std::string CONTACT2;
//! Copyright placed in exported data items.
extern std::string DATA_COPYRIGHT;
//! Program identifier: used in ADIF PROGRAM_ID field and filestore
extern std::string PROGRAM_ID;
//! Short-form program identifier.
extern std::string PROG_ID;
//! Program version. 
extern std::string PROGRAM_VERSION;
//! Program vendor.
extern std::string VENDOR;

// Debug switches
//! Print errors -  by "-d e"
extern bool DEBUG_ERRORS;
//! Print std::thread debugging messages -  by "-d t"
extern bool DEBUG_THREADS;
//! Print libcurl debugging messages -  by "-d c"
extern bool DEBUG_CURL;
//! Reduce long duration tiemouts and waits -  by "-d q"
extern bool DEBUG_QUICK;
//! Print rig access debugging messages -  by "-d r"
extern bool DEBUG_RIGS;
//! Print callsign parsing messages -  by "-d d"
extern bool DEBUG_PARSE;
//! Set hamlib debugging verbosity level -  by "-d h=<level>"
extern rig_debug_level_e HAMLIB_DEBUG_LEVEL;

// Operation switches - _S versions used to override sticky switch
//! Automatically upload QSOs to QSL sites -  by "-n"
extern bool AUTO_UPLOAD;
//! Version of \p AUTO_UPLOAD read from settings
extern bool AUTO_UPLOAD_S;
//! Automatically save QSO record after each change -  by "-a"
extern bool AUTO_SAVE;
//! Version of \p AUTO_SAVE read from settings.
extern bool AUTO_SAVE_S;
//! Dark mode: Dark background, light forreground -  by "-k"
extern bool DARK;
//! Version of \p DARK read from settings.
extern bool DARK_S;
//! Print version details instead of running ZZALOG -  by "-v"
extern bool DISPLAY_VERSION;
//! Print command-line interface instead of running ZZALOG -  by "-h"
extern bool HELP;
//! Start with an empty logbook -  by "-e"
extern bool NEW_BOOK;
//! Do not add file to recent file std::list -  by "-p"
extern bool PRIVATE;
//! Open file in read-only mode -  by "-r"
extern bool READ_ONLY;
//! Resum logging including previous session -  by "-m"
extern bool RESUME_SESSION;
//! Development flag: used to enable/disable features only in development mode ("-g")
extern bool DEVELOPMENT_MODE;

//! Access to FLTK global attribute to  default text size throughout ZZALOG.
extern int FL_NORMAL_SIZE;

//! \cond
// Top level data items - these are declared as externals in each .cpp that uses them
extern band_data* band_data_;
extern band_window* band_window_;
extern banner* banner_;
extern book* book_;
extern book* navigation_book_;
extern club_handler* club_handler_;
extern config* config_;
extern contest_data* contest_data_;
extern cty_data* cty_data_;
extern eqsl_handler* eqsl_handler_;
extern extract_data* extract_records_;
extern fields* fields_;
extern fllog_emul* fllog_emul_;
extern import_data* import_data_;
extern intl_dialog* intl_dialog_;
extern lotw_handler* lotw_handler_;
extern main_window* main_window_;
extern menu* menu_;
extern qrz_handler* qrz_handler_;
extern qsl_dataset* qsl_dataset_;
extern qso_manager* qso_manager_;
extern rig_data* rig_data_;
extern spec_data* spec_data_;
extern status* status_;
extern stn_data* stn_data_;
extern stn_window* stn_window_;
extern tabbed_forms* tabbed_forms_;
extern ticker* ticker_;
extern toolbar* toolbar_;
extern url_handler* url_handler_;
extern wsjtx_handler* wsjtx_handler_;
extern wx_handler* wx_handler_;

//! List of files most recently opened. Maximum: 4 files. 
extern std::list<std::string> recent_files_;
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
void cb_bn_close(Fl_Widget* w, void* v);

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
//! - International character .
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

//! Get \n th recent file
std::string recent_file(int n);

//! Flag to prevent more than one closure process at the same time.
extern bool closing_;

//! Flag to mark everything loaded.
extern bool initialised_;

//! Time loaded.
extern time_t session_start_;

//! Previous frequency.
extern double prev_freq_;

//! Filename in arguments.
extern char* filename_;

//! File is new (neither in argument or settings.
extern bool new_file_;

//! Main logo.
extern Fl_PNG_Image main_icon_;

//! Using backp.
extern bool using_backup_;

//! Sticky switches mesasge.
extern std::string sticky_message_;

//! Common seed to use in password encryption - maintaned with sessions.
extern uint32_t seed_;

//! Do not close banner. Kept \p false unless banner is not deleted at ZZALOG closure in error cases.
extern bool keep_banner_;

//! This run is a new installation
extern bool new_installation_;

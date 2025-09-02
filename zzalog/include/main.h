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

using namespace std;


//! \mainpage ZZALOG Code Documentation
//! 
//! This document describes the code interfaces between the constituent parts
//! of ZZALOG. For the user interface see <A HREF=file:///../../userguide/html/index.html>Userguide</A>.
//! 
//! 
//! 
// Program strings
string COPYRIGHT = "\302\251 Philip Rose GM3ZZA 2018-2025. All rights reserved.";
string PARTY3RD_COPYRIGHT = "Prefix data courtesy of clublog.org, country-files.com and dxatlas.com\n"
"ZZALOG is based in part on the work of the FLTK project https://www.fltk.org.";
string CONTACT = "gm3zza@@btinternet.com";
string CONTACT2 = "gm3zza@btinternet.com";
string DATA_COPYRIGHT = "\302\251 Philip Rose %s. This data may be copied for the purpose of correlation and analysis";
string PROGRAM_ID = "ZZALOG";
string PROG_ID = "ZLG";
string PROGRAM_VERSION = "3.6.6+";
string VENDOR = "GM3ZZA";
// Target ADIF version number
string TARGET_ADIF_VN = "315";

// switches
// Debug levels
bool DEBUG_ERRORS = true;
bool DEBUG_THREADS = false;
bool DEBUG_CURL = false;
bool DEBUG_QUICK = false;
bool DEBUG_RIGS = false;
bool DEBUG_PARSE = false;
rig_debug_level_e HAMLIB_DEBUG_LEVEL = RIG_DEBUG_ERR;
// Operation switches - _S versions used to override sticky switch
bool AUTO_UPLOAD = true;
bool AUTO_UPLOAD_S = false;
bool AUTO_SAVE = true;
bool AUTO_SAVE_S = false;
bool DARK = false;
bool DARK_S = false;
bool DISPLAY_VERSION = false;
bool HELP = false;
bool NEW_BOOK = false;
bool PRIVATE = false;
bool READ_ONLY = false;
bool RESUME_SESSION = false;
bool VERBOSE = false;

// FLTK externals
extern int FL_NORMAL_SIZE;

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

// Recent files opened
list<string> recent_files_;

// Forward declarations
void backup_file();
void restore_backup();
void set_recent_file(string filename);
void save_switches();
void open_html(const char* file);

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
// File is new (neither in argument or settings
bool new_file_ = false;
// Default station callsign
string default_station_ = "";
// Main logo
Fl_PNG_Image main_icon_("ZZALOG_ICON", ___rose_png, ___rose_png_len);
// Using backp
bool using_backup_ = false;
// Sticky switches mesasge
string sticky_message_ = "";
// Common seed to use in password encryption - maintaned with sessions
uint32_t seed_ = 0;
// Defaults config files
string default_data_directory_ = "";
string default_html_directory_ = "";
// Preferences root - system or use
Fl_Preferences::Root prefs_mode_;
// Do not close banner
bool keep_banner_ = false;
// New installation
bool new_installation_ = false;

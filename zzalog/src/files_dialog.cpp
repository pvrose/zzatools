#include "files_dialog.h"

#include "callback.h"
#include "utils.h"
#include "import_data.h"
#include "intl_widgets.h"
#include "main_window.h"
#include "qso_manager.h"
#include "filename_input.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>




extern import_data* import_data_;
extern main_window* main_window_;
extern qso_manager* qso_manager_;
extern string VENDOR;
extern string PROGRAM_ID;

// Constructor
files_dialog::files_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)
{
	enable_tqsl_ = false;;
	enable_card_ = false;
	enable_backup_ = false;
	tqsl_executable_ = "";
	card_directory_ = "";
	ref_data_directory_ = "";
	backup_directory_ = "";
	status_log_file_ = "";
	unzipper_ = "";

	// initialise and create form
	load_values();
	create_form(X, Y);
	// Call back for the OK button
	callback(cb_bn_ok);
	// Enable vertical scrolling
	type(FL_VERTICAL);
}

// Destructor
files_dialog::~files_dialog()
{
	clear();
}

// Load initial values of the fields from the settings
void files_dialog::load_values() {
	// Get number of auto-import files
	int temp_bool = false;
	char * temp_string;

	// Settings groups required below
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences qsl_settings(settings, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	Fl_Preferences datapath_settings(settings, "Datapath");
	Fl_Preferences backup_settings(settings, "Backup");
	Fl_Preferences status_settings(settings, "Status");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");

	// TQSL Executable
	lotw_settings.get("Enable", (int&)enable_tqsl_, false);
	if (enable_tqsl_) {
		// Enabled get file name
		datapath_settings.get("TQSL Executable", temp_string, "");
		tqsl_executable_ = temp_string;
		free(temp_string);
	}
	else {
		// Else clear it
		tqsl_executable_ = "";
	}

	// eQSL e-card directory
	qsl_settings.get("Enable Save", temp_bool, false);
	enable_card_ = temp_bool;
	if (enable_card_) {
		datapath_settings.get("QSLs", temp_string, "");
		card_directory_ = temp_string;
		free(temp_string);
	}
	else {
		card_directory_ = "";
	}

	// Reference directory
	datapath_settings.get("Reference", temp_string, "");
	ref_data_directory_ = temp_string;
	free(temp_string);

	// Backup directory
	backup_settings.get("Path", temp_string, "");
	backup_directory_ = temp_string;
	free(temp_string);
	backup_settings.get("Enable", temp_bool, false);
	enable_backup_ = temp_bool;

	// Status log file
	status_settings.get("Report File", temp_string, "");
	status_log_file_ = temp_string;
	free(temp_string);

	// WSJT-X directory
	datapath_settings.get("WSJT-X", temp_string, "");
	wsjtx_directory_ = temp_string;
	free(temp_string);

	// Unzip command and switches
	clublog_settings.get("Unzip Command", temp_string, "C:/Program Files (X86)/7-Zip/7z.exe");
	unzipper_ = temp_string;
	free(temp_string);
	clublog_settings.get("Unzip Switches", temp_string, "e %s -o%s -y");
	unzip_switches_ = temp_string;
	free(temp_string);

	Fl_Preferences call_settings(qsl_settings, station_callsign_.c_str());
}

// create the form
void files_dialog::create_form(int X, int Y) {

	// widget positions - columns
	const int XGRP = EDGE;
	const int COL1 = XGRP + GAP;
	const int COL2 = COL1 + WRADIO + GAP;
	const int COL3 = COL2 + WEDIT + GAP;
	const int COL4 = COL3 + WBUTTON + GAP;
	const int COL5 = COL4 + WBUTTON + GAP;
	const int COL6 = COL5 + WBUTTON + GAP;
	const int XMAX = COL6 + WBUTTON + EDGE;
	// widget position - rows
	// Group 2, row 1
	const int GRP2 = EDGE;
	const int ROW2_1 = GRP2 + HTEXT;
	const int HGRP2 = ROW2_1 - GRP2 + max(HBUTTON, HTEXT) + GAP;
	// Group 3, row 1
	const int GRP3 = GRP2 + HGRP2;
	const int ROW3_1 = GRP3 + HTEXT;
	const int HGRP3 = ROW3_1 - GRP3 + max(HBUTTON, HTEXT) + GAP;
	// Group 4, row 1
	const int GRP4 = GRP3 + HGRP3;
	const int ROW4_1 = GRP4 + HTEXT;
	const int HGRP4 = ROW4_1 - GRP4 + max(HBUTTON, HTEXT) + GAP;
	// Group 5, row 1
	const int GRP5 = GRP4 + HGRP4;
	const int ROW5_1 = GRP5 + HTEXT;
	const int HGRP5 = ROW5_1 - GRP5 + max(HBUTTON, HTEXT) + GAP;
	// Group 7, row 1
	const int GRP7 = GRP5 + HGRP5;
	const int ROW7_1 = GRP7 + HTEXT;
	const int HGRP7 = ROW7_1 - GRP7 + max(HBUTTON, HTEXT) + GAP;
	// Group 7A, row 1
	const int GRP7A = GRP7 + HGRP7;
	const int ROW7A_1 = GRP7A + HTEXT;
	const int HGRP7A = ROW7A_1 - GRP7A + max(HBUTTON, HTEXT) + GAP;
	// Group 8, row 1
	const int GRP8 = GRP7A + HGRP7A;
	const int ROW8_1 = GRP8 + HTEXT;
	const int HGRP8 = ROW8_1 - GRP8 + max(HBUTTON, HTEXT) + GAP;

	// Bottom of required dialog
	
	Fl_Group* grp_tqsl = new Fl_Group(X + XGRP, Y + GRP2, XMAX, HGRP2, "TQSL Executable");
	grp_tqsl->box(FL_BORDER_BOX);
	grp_tqsl->labelsize(FL_NORMAL_SIZE + 2);
	grp_tqsl->labelfont(FL_BOLD);
	grp_tqsl->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Check box - TQSL executable is valid
	Fl_Check_Button* bn_tqsl_en = new Fl_Check_Button(X + COL1, Y + ROW2_1, WRADIO, HBUTTON);
	bn_tqsl_en->callback(cb_value<Fl_Check_Button, bool>, &enable_tqsl_);
	bn_tqsl_en->when(FL_WHEN_CHANGED);
	bn_tqsl_en->value(enable_tqsl_);
	bn_tqsl_en->tooltip("TQSL Executable filename is valid");
	// Input - TQSL Executable filename
	filename_input* in_tqsl_file = new filename_input(X + COL2, Y + ROW2_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_tqsl_file->callback(cb_value<Fl_Input_, string>, &tqsl_executable_);
	in_tqsl_file->when(FL_WHEN_CHANGED);
	in_tqsl_file->value(tqsl_executable_.c_str());
	in_tqsl_file->tooltip("Location of TQSL executable");
	in_tqsl_file->type(filename_input::FILE);
	in_tqsl_file->title("Please enter the TQSL executable");
#ifdef _WIN32
	in_tqsl_file->pattern("Executable\t*.exe");
#else
	in_tqsl_file->pattern("Executable\t*");
#endif

	grp_tqsl->end();

	Fl_Group* grp_card = new Fl_Group(X + XGRP, Y + GRP3, XMAX, HGRP2, "QSL Card Directoty");
	grp_card->labelsize(FL_NORMAL_SIZE + 2);
	grp_card->labelfont(FL_BOLD);
	grp_card->box(FL_BORDER_BOX);
	grp_card->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Check box - Enable downloading eQSL.cc card images
	Fl_Check_Button* bn_card_en = new Fl_Check_Button(X + COL1, Y + ROW3_1, WRADIO, HBUTTON);
	bn_card_en->callback(cb_value<Fl_Check_Button, bool>, &enable_card_);
	bn_card_en->when(FL_WHEN_CHANGED);
	bn_card_en->value(enable_card_);
	bn_card_en->tooltip("Enable downloading of eQSL e-cards");
	// Input - Card image directory
	filename_input* in_card_file = new filename_input(X + COL2, Y + ROW3_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_card_file->callback(cb_value<Fl_Input, string>, &card_directory_);
	in_card_file->when(FL_WHEN_CHANGED);
	in_card_file->value(card_directory_.c_str());
	in_card_file->tooltip("Directory to which to download eQSL cards");
	in_card_file->type(filename_input::DIRECTORY);
	in_card_file->title("Please enter the QSL card top-level directory");

	grp_card->end();

	Fl_Group* grp_ref_data = new Fl_Group(X + XGRP, Y + GRP4, XMAX, HGRP2, "Reference Data Directory");
	grp_ref_data->labelsize(FL_NORMAL_SIZE + 2);
	grp_ref_data->labelfont(FL_BOLD);
	grp_ref_data->box(FL_BORDER_BOX);
	grp_ref_data->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - directory name for the reference (ADIF spec, Prefix data and band plans)
	filename_input* in_ref_data_file = new filename_input(X + COL2, Y + ROW4_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_ref_data_file->callback(cb_value<Fl_Input, string>, &ref_data_directory_);
	in_ref_data_file->when(FL_WHEN_CHANGED);
	in_ref_data_file->value(ref_data_directory_.c_str());
	in_ref_data_file->tooltip("Directory containing reference data");
	in_ref_data_file->type(filename_input::DIRECTORY);
	in_ref_data_file->title("Please enter the reference data directory");

	grp_ref_data->end();

	Fl_Group* grp_backup = new Fl_Group(X + XGRP, Y + GRP5, XMAX, HGRP2, "Backup Directory");
	grp_backup->labelsize(FL_NORMAL_SIZE + 2);
	grp_backup->labelfont(FL_BOLD);
	grp_backup->box(FL_BORDER_BOX);
	grp_backup->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Check box - Enable automatic back up of data
	Fl_Check_Button* bn_backup_en = new Fl_Check_Button(X + COL1, Y + ROW5_1, WRADIO, HBUTTON);
	bn_backup_en->callback(cb_value<Fl_Check_Button, bool>, &enable_backup_);
	bn_backup_en->when(FL_WHEN_CHANGED);
	bn_backup_en->value(enable_backup_);
	bn_backup_en->tooltip("Enable data back-up when closing logging session");
	// Input - Backup directory name
	filename_input* in_backup_file = new filename_input(X + COL2, Y + ROW5_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_backup_file->callback(cb_value<Fl_Input, string>, &backup_directory_);
	in_backup_file->when(FL_WHEN_CHANGED);
	in_backup_file->value(backup_directory_.c_str());
	in_backup_file->tooltip("Directory to which to back-up the log");
	in_backup_file->type(filename_input::DIRECTORY);
	in_backup_file->title("Please enter the backup directory");

	grp_backup->end();

	Fl_Group* grp_status = new Fl_Group(X + XGRP, Y + GRP7, XMAX, HGRP7, "Status log file");
	grp_status->labelsize(FL_NORMAL_SIZE + 2);
	grp_status->labelfont(FL_BOLD);
	grp_status->box(FL_BORDER_BOX);
	grp_status->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - status log file name
	filename_input* in_status_file = new filename_input(X + COL2, Y + ROW7_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_status_file->callback(cb_value<Fl_Input, string>, &status_log_file_);
	in_status_file->when(FL_WHEN_CHANGED);
	in_status_file->value(status_log_file_.c_str());
	in_status_file->tooltip("Location of status log file");
	in_status_file->type(filename_input::FILE);
	in_status_file->title("Please enter the status log file");
	in_status_file->pattern("Text\t*.txt");

	grp_status->end();

	Fl_Group* grp_wsjtx = new Fl_Group(X + XGRP, Y + GRP7A, XMAX, HGRP7A, "WSJT-X directory");
	grp_wsjtx->labelsize(FL_NORMAL_SIZE + 2);
	grp_wsjtx->labelfont(FL_BOLD);
	grp_wsjtx->box(FL_BORDER_BOX);
	grp_wsjtx->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - status log file name
	filename_input* in_wsjtx_file = new filename_input(X + COL2, Y + ROW7A_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_wsjtx_file->callback(cb_value<Fl_Input, string>, &wsjtx_directory_);
	in_wsjtx_file->when(FL_WHEN_CHANGED);
	in_wsjtx_file->value(wsjtx_directory_.c_str());
	in_wsjtx_file->tooltip("Location of WSJT-X directory");
	in_wsjtx_file->type(filename_input::DIRECTORY);
	in_wsjtx_file->title("Please enter the WSJT-X directory");

	grp_wsjtx->end();

	Fl_Group* grp_unzip = new Fl_Group(X + XGRP, Y + GRP8, XMAX, HGRP8, "Unzip executable");
	grp_unzip->labelsize(FL_NORMAL_SIZE + 2);
	grp_unzip->labelfont(FL_BOLD);
	grp_unzip->box(FL_BORDER_BOX);
	grp_unzip->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - preferred web browser
	filename_input* in_unzipper = new filename_input(X + COL2, Y + ROW8_1, WEDIT, HTEXT);
	in_unzipper->callback(cb_value<Fl_Input, string>, &unzipper_);
	in_unzipper->when(FL_WHEN_CHANGED);
	in_unzipper->value(unzipper_.c_str());
	in_unzipper->tooltip("Location of unzipper executable");
	in_unzipper->type(filename_input::FILE);
	in_unzipper->title("Please select the unzip tool");
#ifdef _WIN32
	in_unzipper->pattern("Executable\t*.exe");
#else
	in_unzipper->pattern("");
#endif
	// Input switches for unzip command
	intl_input* in_switches = new intl_input(X + COL3, Y + ROW8_1, WSMEDIT, HTEXT, "Switches");
	in_switches->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in_switches->callback(cb_value<intl_input, string>, &unzip_switches_);
	in_switches->when(FL_WHEN_CHANGED);
	in_switches->value(unzip_switches_.c_str());
	in_switches->tooltip("Location of unzipper executable");

	grp_unzip->end();

	Fl_Group::end();

	enable_widgets();

}

// save values to the settings
void files_dialog::save_values() {
	// Settings groups for below
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences qsl_settings(settings, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	Fl_Preferences datapath_settings(settings, "Datapath");
	Fl_Preferences backup_settings(settings, "Backup");
	Fl_Preferences status_settings(settings, "Status");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	Fl_Preferences qsld_settings(settings, "QSL Design");
	Fl_Preferences call_settings(qsld_settings, station_callsign_.c_str());

	// TQSL Executable
	lotw_settings.set("Enable", enable_tqsl_);
	if (enable_tqsl_) {
		datapath_settings.set("TQSL Executable", tqsl_executable_.c_str());
	}

	// eQSL e-card directory
	qsl_settings.set("Enable Save", enable_card_);
	if (enable_card_) {
		datapath_settings.set("QSLs", card_directory_.c_str());
	}

	// Reference data directory
	datapath_settings.set("Reference", ref_data_directory_.c_str());

	// Backup 
	backup_settings.set("Path", backup_directory_.c_str());
	backup_settings.set("Enable", enable_backup_);

	// Status log file
	status_settings.set("Report File", status_log_file_.c_str());

	// WSJT-X directory
	datapath_settings.set("WSJT-X", wsjtx_directory_.c_str());

	// Clublog
	clublog_settings.set("Unzip Command", unzipper_.c_str());
	clublog_settings.set("Unzip Switches", unzip_switches_.c_str());

}

// Method provided as needed to overload the page_dialog version
void files_dialog::enable_widgets() {
}


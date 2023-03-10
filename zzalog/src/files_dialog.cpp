#include "files_dialog.h"

#include "callback.h"
#include "utils.h"
#include "import_data.h"
#include "intl_widgets.h"
#include "qsl_design.h"
#include "main_window.h"
#include "qso_manager.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>




extern Fl_Preferences* settings_;
extern import_data* import_data_;
extern main_window* main_window_;
extern qso_manager* qso_manager_;

// Constructor
files_dialog::files_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)
{
	// Auto-import enables
	for (int i = 0; i < AUTO_COUNT; i++) {
		enable_auto_[i] = false;
		auto_file_[i] = "";
		auto_src_[i] = "";
		auto_empty_[i] = false;
		auto_data_[i] = { "", "", nullptr, nullptr, nullptr, nullptr };
	}
	enable_tqsl_ = false;;
	enable_card_ = false;
	enable_backup_ = false;
	tqsl_executable_ = "";
	card_directory_ = "";
	ref_data_directory_ = "";
	backup_directory_ = "";
	status_log_file_ = "";
	web_browser_ = "";
	unzipper_ = "";
	qsl_template_ = "";
	tqsl_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	card_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	ref_data_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	backup_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	web_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	status_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	unzipper_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };
	template_data_ = { "", "", nullptr, nullptr, nullptr, nullptr };

	bn_params_ = nullptr;

	auto_poll_ = nan("");
	autos_changed_ = false;

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
	Fl_Preferences rtu_settings(settings_, "Real Time Update");
	Fl_Preferences files_settings(rtu_settings, "Files");
	// Get number of auto-import files
	int num_files = files_settings.groups();
	int temp_bool = false;
	char * temp_string;
	// For each auto-import file - prevent a manual edit creating more files than we can handle
	for (int i = 0; i < num_files && i < AUTO_COUNT; i++) {
		// Get name of app from the name of the settings group
		auto_src_[i] = files_settings.group(i);
		Fl_Preferences file_settings(files_settings, i);
		// Get the various information for this file
		file_settings.get("Filename", temp_string, "");
		auto_file_[i] = temp_string;
		free(temp_string);
		file_settings.get("Empty On Read", temp_bool, false);
		auto_empty_[i] = temp_bool;
		enable_auto_[i] = true;
		file_settings.get("Timestamp", temp_string, "20190601000000");
		auto_ts_[i] = temp_string;
		free(temp_string);
	}
	// If less files than the default dialog provision, add default data
	for (int i = num_files; i < AUTO_COUNT; i++) {
		auto_file_[i] = "";
		auto_src_[i] = "";
		auto_empty_[i] = false;
		enable_auto_[i] = false;
	}
	rtu_settings.get("Polling Interval", auto_poll_, AUTO_IP_DEF);

	// Settings groups required below
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	Fl_Preferences datapath_settings(settings_, "Datapath");
	Fl_Preferences backup_settings(settings_, "Backup");
	Fl_Preferences status_settings(settings_, "Status");
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

	// Web browser executable
	datapath_settings.get("Web Browser", temp_string, "");
	web_browser_ = temp_string;
	free(temp_string);

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

	// Callsign for QSL template
	station_callsign_ = qso_manager_->get_default(qso_manager::CALLSIGN);
	// QSL Template
	call_settings.get("Filename", temp_string, "");
	qsl_template_ = temp_string;
	free(temp_string);

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
	// Group 1, rows 1 to 4
	const int GRP1 = EDGE;
	const int ROW1_1 = GRP1 + HTEXT;
	const int ROW1_2 = ROW1_1 + max(HBUTTON, HTEXT);
	const int ROW1_3 = ROW1_2 + max(HBUTTON, HTEXT);
	const int* const ROW1[files_dialog::AUTO_COUNT + 1] =
	{ &ROW1_1, &ROW1_2, &ROW1_3 };
	const int HGRP1 = ROW1_3 + HTEXT;
	// Group 2, row 1
	const int GRP2 = GRP1 + HGRP1;
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
	// Group 6, row 1
	const int GRP6 = GRP5 + HGRP5;
	const int ROW6_1 = GRP6 + HTEXT;
	const int HGRP6 = ROW6_1 - GRP6 + max(HBUTTON, HTEXT) + GAP;
	// Group 7, row 1
	const int GRP7 = GRP6 + HGRP6;
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
	// Group 9, row1
	const int GRP9 = GRP8 + HGRP8;
	const int ROW9_1 = GRP9 + HTEXT;
	const int HGRP9 = ROW9_1 - GRP9 + max(HBUTTON, HTEXT) + GAP;

	// Bottom of required dialog
	const int YMAX = GRP8 + HGRP8 + EDGE;
	
	Fl_Group* grp_auto = new Fl_Group (X + XGRP, Y + GRP1, XMAX, HGRP1, "Auto-import files");
	grp_auto->labelsize(FONT_SIZE);
	grp_auto->box(FL_THIN_DOWN_BOX);
	grp_auto->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// widgets required for auto-import
	Fl_Check_Button* bn_auto_en[AUTO_COUNT];
	intl_input* in_auto_file[AUTO_COUNT];
	intl_input* in_auto_src[AUTO_COUNT];
	Fl_Check_Button* bn_auto_mt[AUTO_COUNT];
	Fl_Button* bn_browse_auto[AUTO_COUNT];

	for (int i = 0; i < AUTO_COUNT; i++) {
		// Check box - enable this file in auto-import
		bn_auto_en[i] = new Fl_Check_Button(X + COL1, Y + *ROW1[i], WRADIO, HBUTTON);
		bn_auto_en[i]->callback(cb_value_auto<Fl_Check_Button, bool>, &(enable_auto_[i]));
		bn_auto_en[i]->when(FL_WHEN_CHANGED);
		bn_auto_en[i]->value(enable_auto_[i]);
		bn_auto_en[i]->tooltip("Enable this auto-import");
		// Input - file name for auto-import
		in_auto_file[i] = new intl_input(X + COL2, Y + *ROW1[i], WEDIT, HTEXT);
		in_auto_file[i]->callback(cb_value_auto<intl_input, string>, &(auto_file_[i]));
		in_auto_file[i]->when(FL_WHEN_CHANGED);
		in_auto_file[i]->textsize(FONT_SIZE);
		in_auto_file[i]->value(auto_file_[i].c_str());
		in_auto_file[i]->tooltip("File name for auto-import");
		// Input - name of application generating the auto-import
		in_auto_src[i] = new intl_input(X + COL3, Y + *ROW1[i], WBUTTON, HTEXT);
		in_auto_src[i]->callback(cb_value_auto<intl_input, string>, &(auto_src_[i]));
		in_auto_src[i]->when(FL_WHEN_CHANGED);
		in_auto_src[i]->textsize(FONT_SIZE);
		in_auto_src[i]->value(auto_src_[i].c_str());
		in_auto_src[i]->tooltip("Application generating the auto-import");
		// Button - file will be cleared after auto-import
		bn_auto_mt[i] = new Fl_Check_Button(X + COL4, Y + *ROW1[i], WRADIO, HBUTTON, "Empty");
		bn_auto_mt[i]->align(FL_ALIGN_RIGHT);
		bn_auto_mt[i]->callback(cb_value_auto<Fl_Check_Button, bool>, &(auto_empty_[i]));
		bn_auto_mt[i]->when(FL_WHEN_CHANGED);
		bn_auto_mt[i]->labelsize(FONT_SIZE);
		bn_auto_mt[i]->value(auto_empty_[i]);
		bn_auto_mt[i]->tooltip("Empty the file after import");
		// Button - opens a file browser to get the filename
		bn_browse_auto[i] = new Fl_Button(X + COL5, Y + *ROW1[i], WBUTTON, HBUTTON, "Browse");
		bn_browse_auto[i]->align(FL_ALIGN_INSIDE);
		auto_data_[i] = { "Please enter a file to auto-import", "ADI Files\t*.adi\nADX Files\t*.adx", &auto_file_[i], &enable_auto_[i], in_auto_file[i], bn_auto_en[i] };
		bn_browse_auto[i]->callback(cb_bn_browsefile, &(auto_data_[i]));
		bn_browse_auto[i]->when(FL_WHEN_RELEASE);
		bn_browse_auto[i]->labelsize(FONT_SIZE);
		bn_browse_auto[i]->tooltip("Open file browser to locate auto-import file");
	}
	// Spinner - sets  the polling interval for auto-import
	Fl_Spinner* spin_auto = new Fl_Spinner(X + COL6, Y + ROW1_2, WBUTTON, HBUTTON, "Polling\nint. (secs)");
	spin_auto->labelsize(FONT_SIZE);
	spin_auto->textsize(FONT_SIZE);
	spin_auto->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	spin_auto->type(FL_FLOAT_INPUT);
	spin_auto->maximum(AUTO_IP_MAX);
	spin_auto->minimum(AUTO_IP_MIN);
	spin_auto->step(5.0);
	spin_auto->callback(cb_value<Fl_Spinner, double>, &auto_poll_);
	spin_auto->when(FL_WHEN_RELEASE);
	spin_auto->value(auto_poll_);
	char tip[128];
	sprintf(tip, "Select the required polling period (s). Min = %f, Max = %f", AUTO_IP_MIN, AUTO_IP_MAX);
	spin_auto->copy_tooltip(tip);

	grp_auto->end();

	Fl_Group* grp_tqsl = new Fl_Group(X + XGRP, Y + GRP2, XMAX, HGRP2, "TQSL Executable");
	grp_tqsl->labelsize(FONT_SIZE);
	grp_tqsl->box(FL_THIN_DOWN_BOX);
	grp_tqsl->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Check box - TQSL executable is valid
	Fl_Check_Button* bn_tqsl_en = new Fl_Check_Button(X + COL1, Y + ROW2_1, WRADIO, HBUTTON);
	bn_tqsl_en->callback(cb_value<Fl_Check_Button, bool>, &enable_tqsl_);
	bn_tqsl_en->when(FL_WHEN_CHANGED);
	bn_tqsl_en->value(enable_tqsl_);
	bn_tqsl_en->tooltip("TQSL Executable filename is valid");
	// Input - TQSL Executable filename
	intl_input* in_tqsl_file = new intl_input(X + COL2, Y + ROW2_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_tqsl_file->callback(cb_value<intl_input, string>, &tqsl_executable_);
	in_tqsl_file->when(FL_WHEN_CHANGED);
	in_tqsl_file->textsize(FONT_SIZE);
	in_tqsl_file->value(tqsl_executable_.c_str());
	in_tqsl_file->tooltip("Location of TQSL executable");
	// Button - Opens file browser to locate executable
	Fl_Button* bn_browse_tqsl = new Fl_Button(X + COL5, Y + ROW2_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_tqsl->align(FL_ALIGN_INSIDE);
#ifdef _WIN32
	tqsl_data_ = { "Please enter the TQSL executable", "Executable\t*.exe", &tqsl_executable_, &enable_tqsl_, in_tqsl_file, bn_tqsl_en };
#else
	// TODO: Change the file pattern for Posix 
	tqsl_data_ = { "Please enter the TQSL executable", "Executable\t*", &tqsl_executable_, &enable_tqsl_, in_tqsl_file, bn_tqsl_en };
#endif
	bn_browse_tqsl->callback(cb_bn_browsefile, &tqsl_data_);
	bn_browse_tqsl->when(FL_WHEN_RELEASE);
	bn_browse_tqsl->labelsize(FONT_SIZE);
	bn_browse_tqsl->tooltip("Opens a file browsewr to locate the TQSL executable");

	grp_tqsl->end();

	Fl_Group* grp_card = new Fl_Group(X + XGRP, Y + GRP3, XMAX, HGRP2, "QSL Card Directoty");
	grp_card->labelsize(FONT_SIZE);
	grp_card->box(FL_THIN_DOWN_BOX);
	grp_card->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Check box - Enable downloading eQSL.cc card images
	Fl_Check_Button* bn_card_en = new Fl_Check_Button(X + COL1, Y + ROW3_1, WRADIO, HBUTTON);
	bn_card_en->callback(cb_value<Fl_Check_Button, bool>, &enable_card_);
	bn_card_en->when(FL_WHEN_CHANGED);
	bn_card_en->value(enable_card_);
	bn_card_en->tooltip("Enable downloading of eQSL e-cards");
	// Input - Card image directory
	intl_input* in_card_file = new intl_input(X + COL2, Y + ROW3_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_card_file->callback(cb_value<intl_input, string>, &card_directory_);
	in_card_file->when(FL_WHEN_CHANGED);
	in_card_file->textsize(FONT_SIZE);
	in_card_file->value(card_directory_.c_str());
	in_card_file->tooltip("Directory to which to download eQSL cards");
	// Button - Opens directory browser to locate the directory
	Fl_Button* bn_browse_card = new Fl_Button(X + COL5, Y + ROW3_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_card->align(FL_ALIGN_INSIDE);
	card_data_ = { "Please enter the QSL card top-level directory", "", &card_directory_, &enable_card_, in_card_file, bn_card_en };
	bn_browse_card->callback(cb_bn_browsedir, &card_data_);
	bn_browse_card->when(FL_WHEN_RELEASE);
	bn_browse_card->labelsize(FONT_SIZE);
	bn_browse_card->tooltip("Opens directory for saving eQSL e-cards");

	grp_card->end();

	Fl_Group* grp_ref_data = new Fl_Group(X + XGRP, Y + GRP4, XMAX, HGRP2, "Reference Data Directory");
	grp_ref_data->labelsize(FONT_SIZE);
	grp_ref_data->box(FL_THIN_DOWN_BOX);
	grp_ref_data->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - directory name for the reference (ADIF spec, Prefix data and band plans)
	intl_input* in_ref_data_file = new intl_input(X + COL2, Y + ROW4_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_ref_data_file->callback(cb_value<intl_input, string>, &ref_data_directory_);
	in_ref_data_file->when(FL_WHEN_CHANGED);
	in_ref_data_file->textsize(FONT_SIZE);
	in_ref_data_file->value(ref_data_directory_.c_str());
	in_ref_data_file->tooltip("Directory containing reference data");
	// Button - Opens directory browse
	Fl_Button* bn_browse_ref_data = new Fl_Button(X + COL5, Y + ROW4_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_ref_data->align(FL_ALIGN_INSIDE);
	ref_data_data_ = { "Please enter the reference data directory", "", &ref_data_directory_, nullptr, in_ref_data_file, nullptr };
	bn_browse_ref_data->callback(cb_bn_browsedir, &ref_data_data_);
	bn_browse_ref_data->when(FL_WHEN_RELEASE);
	bn_browse_ref_data->labelsize(FONT_SIZE);
	bn_browse_ref_data->tooltip("Opens directory browser to locate reference data directory");

	grp_ref_data->end();

	Fl_Group* grp_backup = new Fl_Group(X + XGRP, Y + GRP5, XMAX, HGRP2, "Backup Directory");
	grp_backup->labelsize(FONT_SIZE);
	grp_backup->box(FL_THIN_DOWN_BOX);
	grp_backup->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Check box - Enable automatic back up of data
	Fl_Check_Button* bn_backup_en = new Fl_Check_Button(X + COL1, Y + ROW5_1, WRADIO, HBUTTON);
	bn_backup_en->callback(cb_value<Fl_Check_Button, bool>, &enable_backup_);
	bn_backup_en->when(FL_WHEN_CHANGED);
	bn_backup_en->value(enable_backup_);
	bn_backup_en->tooltip("Enable data back-up when closing logging session");
	// Input - Backup directory name
	intl_input* in_backup_file = new intl_input(X + COL2, Y + ROW5_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_backup_file->callback(cb_value<intl_input, string>, &backup_directory_);
	in_backup_file->when(FL_WHEN_CHANGED);
	in_backup_file->textsize(FONT_SIZE);
	in_backup_file->value(backup_directory_.c_str());
	in_backup_file->tooltip("Directory to which to back-up the log");
	// Button - opens directory browser
	Fl_Button* bn_browse_backup = new Fl_Button(X + COL5, Y + ROW5_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_backup->align(FL_ALIGN_INSIDE);
	backup_data_ = { "Please enter the backup directory", "", &backup_directory_, &enable_backup_, in_backup_file, bn_backup_en };
	bn_browse_backup->callback(cb_bn_browsedir, &backup_data_);
	bn_browse_backup->when(FL_WHEN_RELEASE);
	bn_browse_backup->labelsize(FONT_SIZE);
	bn_browse_backup->tooltip("Opens directory browser to locate back-up directory");

	grp_backup->end();

	Fl_Group* grp_web = new Fl_Group(X + XGRP, Y + GRP6, XMAX, HGRP2, "Web Browser");
	grp_web->labelsize(FONT_SIZE);
	grp_web->box(FL_THIN_DOWN_BOX);
	grp_web->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - preferred web browser
	intl_input* in_web_file = new intl_input(X + COL2, Y + ROW6_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_web_file->callback(cb_value<intl_input, string>, &web_browser_);
	in_web_file->when(FL_WHEN_CHANGED);
	in_web_file->textsize(FONT_SIZE);
	in_web_file->value(web_browser_.c_str());
	in_web_file->tooltip("Location of web browser executable");
	// Button - opens file browser
	Fl_Button* bn_browse_web = new Fl_Button(X + COL5, Y + ROW6_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_web->align(FL_ALIGN_INSIDE);
#ifdef _WIN32
	web_data_ = { "Please enter the Web browser", "Executable\t*.exe", &web_browser_, nullptr, in_web_file, nullptr };
#else
	// TODO: Change file pattern for Posix executables
	web_data_ = { "Please enter the Web browser", "Executable\t*.exe", &web_browser_, nullptr, in_web_file, nullptr };
#endif
	bn_browse_web->callback(cb_bn_browsefile, &web_data_);
	bn_browse_web->when(FL_WHEN_RELEASE);
	bn_browse_web->labelsize(FONT_SIZE);
	bn_browse_web->tooltip("Opens file browser to locate your preferred web browser");

	grp_web->end();

	Fl_Group* grp_status = new Fl_Group(X + XGRP, Y + GRP7, XMAX, HGRP7, "Status log file");
	grp_status->labelsize(FONT_SIZE);
	grp_status->box(FL_THIN_DOWN_BOX);
	grp_status->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - status log file name
	intl_input* in_status_file = new intl_input(X + COL2, Y + ROW7_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_status_file->callback(cb_value<intl_input, string>, &status_log_file_);
	in_status_file->when(FL_WHEN_CHANGED);
	in_status_file->textsize(FONT_SIZE);
	in_status_file->value(status_log_file_.c_str());
	in_status_file->tooltip("Location of status log file");
	// Button - opens file browser
	Fl_Button* bn_browse_status = new Fl_Button(X + COL5, Y + ROW7_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_status->align(FL_ALIGN_INSIDE);
	status_data_ = { "Please enter the status log file", "Text\t*.txt", &status_log_file_, nullptr, in_status_file, nullptr };
	bn_browse_status->callback(cb_bn_browsefile, &status_data_);
	bn_browse_status->when(FL_WHEN_RELEASE);
	bn_browse_status->labelsize(FONT_SIZE);
	bn_browse_status->tooltip("Opens a file browsewr to locate the status file");

	grp_status->end();

	Fl_Group* grp_wsjtx = new Fl_Group(X + XGRP, Y + GRP7A, XMAX, HGRP7A, "WSJT-X directory");
	grp_wsjtx->labelsize(FONT_SIZE);
	grp_wsjtx->box(FL_THIN_DOWN_BOX);
	grp_wsjtx->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - status log file name
	intl_input* in_wsjtx_file = new intl_input(X + COL2, Y + ROW7A_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	in_wsjtx_file->callback(cb_value<intl_input, string>, &wsjtx_directory_);
	in_wsjtx_file->when(FL_WHEN_CHANGED);
	in_wsjtx_file->textsize(FONT_SIZE);
	in_wsjtx_file->value(wsjtx_directory_.c_str());
	in_wsjtx_file->tooltip("Location of WSJT-X directory");
	// Button - opens file browser
	Fl_Button* bn_browse_wsjtx = new Fl_Button(X + COL5, Y + ROW7A_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_wsjtx->align(FL_ALIGN_INSIDE);
	wsjtx_data_ = { "Please enter the WSJT-X directory", "", &wsjtx_directory_, nullptr, in_wsjtx_file, nullptr };
	bn_browse_wsjtx->callback(cb_bn_browsedir, &wsjtx_data_);
	bn_browse_wsjtx->when(FL_WHEN_RELEASE);
	bn_browse_wsjtx->labelsize(FONT_SIZE);
	bn_browse_wsjtx->tooltip("Opens a file browsewr to locate the status file");

	grp_wsjtx->end();

	Fl_Group* grp_unzip = new Fl_Group(X + XGRP, Y + GRP8, XMAX, HGRP8, "Unzip executable");
	grp_unzip->labelsize(FONT_SIZE);
	grp_unzip->box(FL_THIN_DOWN_BOX);
	grp_unzip->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	// Input - preferred web browser
	intl_input* in_unzipper = new intl_input(X + COL2, Y + ROW8_1, WEDIT, HTEXT);
	in_unzipper->callback(cb_value<intl_input, string>, &unzipper_);
	in_unzipper->when(FL_WHEN_CHANGED);
	in_unzipper->textsize(FONT_SIZE);
	in_unzipper->value(unzipper_.c_str());
	in_unzipper->tooltip("Location of unzipper executable");
	// Input switches for unzip command
	intl_input* in_switches = new intl_input(X + COL3, Y + ROW8_1, WSMEDIT, HTEXT, "Switches");
	in_switches->labelfont(FONT);
	in_switches->labelsize(FONT_SIZE);
	in_switches->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in_switches->callback(cb_value<intl_input, string>, &unzip_switches_);
	in_switches->when(FL_WHEN_CHANGED);
	in_switches->textsize(FONT_SIZE);
	in_switches->value(unzip_switches_.c_str());
	in_switches->tooltip("Location of unzipper executable");

	// Button - opens file browser
	Fl_Button* bn_browse_unzip = new Fl_Button(X + COL5, Y + ROW8_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_unzip->align(FL_ALIGN_INSIDE);
#ifdef _WIN32
	unzipper_data_ = { "Please select the unzip tool", "Executable\t*.exe", &unzipper_, nullptr, in_unzipper, nullptr };
#else
	// TODO: Change file pattern for Posix executables
	unzipper_data_ = { "Please select the unzip tool", "Executable\t*.exe", &unzipper_, nullptr, in_web_file, nullptr };
#endif
	bn_browse_unzip->callback(cb_bn_browsefile, &unzipper_data_);
	bn_browse_unzip->when(FL_WHEN_RELEASE);
	bn_browse_unzip->labelsize(FONT_SIZE);
	bn_browse_unzip->tooltip("Opens file browser to locate your preferred web browser");

	grp_unzip->end();

	Fl_Group* grp_qsld = new Fl_Group(X + XGRP, Y + GRP9, XMAX, HGRP9, "QSL Template: - ");
	grp_qsld->labelsize(FONT_SIZE);
	grp_qsld->box(FL_THIN_DOWN_BOX);
	grp_qsld->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	ch_callsign_ = new field_input(grp_qsld->x() + WLABEL * 2, grp_qsld->y(), WSMEDIT, HBUTTON);
	ch_callsign_->value(station_callsign_.c_str());
	ch_callsign_->callback(cb_ch_callsign, &station_callsign_);
	ch_callsign_->field_name("STATION_CALLSIGN");
	ch_callsign_->tooltip("Select the callsign to change QSL template parameters for");

	// Input - QSL Template file name
	ip_qsl_template_ = new intl_input(X + COL2, Y + ROW9_1, WEDIT + GAP + WBUTTON + GAP + WBUTTON, HTEXT);
	ip_qsl_template_->callback(cb_value<intl_input, string>, &qsl_template_);
	ip_qsl_template_->when(FL_WHEN_CHANGED);
	ip_qsl_template_->textsize(FONT_SIZE);
	ip_qsl_template_->value(qsl_template_.c_str());
	ip_qsl_template_->tooltip("Location of QSL Template file");

	// Button - opens file browser
	Fl_Button* bn_browse_qsld = new Fl_Button(X + COL5, Y + ROW9_1, WBUTTON, HBUTTON, "Browse");
	bn_browse_qsld->align(FL_ALIGN_INSIDE);
	template_data_ = { "Please enter the QSL Template", "HTML\t*.{htm,html}", &qsl_template_, nullptr, ip_qsl_template_, nullptr };
	bn_browse_qsld->callback(cb_bn_browsefile, &template_data_);
	bn_browse_qsld->when(FL_WHEN_RELEASE);
	bn_browse_qsld->labelsize(FONT_SIZE);
	bn_browse_qsld->tooltip("Opens a file browser to locate the QSL template file");

	// Button - open dimansions dialog
	Fl_Button* bn_qsl_dim = new Fl_Button(X + COL6, Y + ROW9_1, WBUTTON, HBUTTON, "Params");
	bn_qsl_dim->align(FL_ALIGN_INSIDE);
	bn_qsl_dim->callback(cb_bn_qslt, nullptr);
	bn_qsl_dim->labelsize(FONT_SIZE);
	bn_qsl_dim->when(FL_WHEN_RELEASE);
	bn_qsl_dim->tooltip("Opens dialog to set label size and printer details");
	bn_params_ = bn_qsl_dim;

	grp_qsld->end();


	Fl_Group::end();

	enable_widgets();

}

// save values to the settings
void files_dialog::save_values() {
	Fl_Preferences rtu_settings(settings_, "Real Time Update");
	// Auto Import files
	Fl_Preferences files_settings(rtu_settings, "Files");
	// Delete existing files information
	files_settings.clear();
	int file_ix = 0;
	// For all the possible auto-import data
	for (int i = 0; i < AUTO_COUNT; i++) {
		if (enable_auto_[i]) {
			// Only set those enabled - note this will start afresh so gaps are closed up
			Fl_Preferences file_settings(files_settings, auto_src_[i].c_str());
			file_settings.set("Filename", auto_file_[i].c_str());
			file_settings.set("Empty On Read", auto_empty_[i]);
			file_settings.set("Timestamp", auto_ts_[i].c_str());
			file_ix++;
		}
	}
	rtu_settings.set("Polling Interval", auto_poll_);

	// Settings groups for below
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	Fl_Preferences datapath_settings(settings_, "Datapath");
	Fl_Preferences backup_settings(settings_, "Backup");
	Fl_Preferences status_settings(settings_, "Status");
	Fl_Preferences clublog_settings(qsl_settings, "ClubLog");
	Fl_Preferences qsld_settings(settings_, "QSL Design");
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

	// Web browser
	datapath_settings.set("Web Browser", web_browser_.c_str());

	// Status log file
	status_settings.set("Report File", status_log_file_.c_str());

	// WSJT-X directory
	datapath_settings.set("WSJT-X", wsjtx_directory_.c_str());

	// Clublog
	clublog_settings.set("Unzip Command", unzipper_.c_str());
	clublog_settings.set("Unzip Switches", unzip_switches_.c_str());


	// QSL Template
	call_settings.set("Filename", qsl_template_.c_str());

	// Restart any auto-update if any of the information may have changed
	if (autos_changed_ && import_data_->is_auto_update()) {
		import_data_->stop_update(false);
		while (!import_data_->update_complete()) Fl::check();
		import_data_->start_auto_update();
		// Clear ths flag in case "Save" not "OK" was clicked to get here
		autos_changed_ = false;
	}

}

// Method provided as needed to overload the page_dialog version
void files_dialog::enable_widgets() {
	Fl_Preferences qsld_settings(settings_, "QSL Design");
	Fl_Preferences call_settings(qsld_settings, station_callsign_.c_str());

	// Callsign for QSL template
	ch_callsign_->value(station_callsign_.c_str());
	// QSL Template
	char* temp_string;
	call_settings.get("Filename", temp_string, "");
	qsl_template_ = temp_string;
	((intl_input*)ip_qsl_template_)->value(temp_string);
	free(temp_string);

}

// Special version of cb_value callback that also sets auto_changed_
template<class WIDGET, class DATA>
void files_dialog::cb_value_auto(Fl_Widget* w, void* v) {
	cb_value<WIDGET, DATA>(w, v);
	files_dialog* that = ancestor_view<files_dialog>(w);
	that->autos_changed_ = true;
}

// Callback to open dialog
void files_dialog::cb_bn_qslt(Fl_Widget* w, void* v) {
	files_dialog* that = ancestor_view<files_dialog>(w);
	int target_x = main_window_->x_root() + that->bn_params_->x();
	int target_y = main_window_->y_root() + that->bn_params_->y();
	qsl_design* dialog = new qsl_design(target_x, target_y, 0, 0, that->station_callsign_.c_str());
}

// Callback for changing callsign
void files_dialog::cb_ch_callsign(Fl_Widget* w, void* v) {
	files_dialog* that = ancestor_view<files_dialog>(w);
	cb_value<field_input, string>(w, v);
	that->enable_widgets();
}

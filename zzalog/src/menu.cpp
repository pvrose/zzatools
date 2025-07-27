#include "menu.h"

#include "callback.h"
#include "utils.h"

#include "about_dialog.h"
#include "book.h"
#include "calendar.h"
#include "change_dialog.h"
#include "config.h"
#include "cty_data.h"
#include "eqsl_handler.h"
#include "extract_data.h"
#include "import_data.h"
#include "intl_dialog.h"
#include "main_window.h"
#include "page_dialog.h"
#include "printer.h"
#include "qrz_handler.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "report_tree.h"
#include "search_dialog.h"
#include "spec_data.h"
#include "spec_tree.h"
#include "status.h"
#include "tabbed_forms.h"
#include "ticker.h"
#include "toolbar.h"
#include "url_handler.h"
#include "wsjtx_handler.h"

#include <iostream>
#include <list>
#include <sstream>
#include <string>
// For launching PDF reader
#ifdef _WIN32
#include <windows.h>
#else 
#include <cstdlib>
#endif



#include <FL/fl_ask.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Tooltip.H>




extern book* book_;
extern book* navigation_book_;
extern config* config_;
extern cty_data* cty_data_;
extern eqsl_handler* eqsl_handler_;
extern extract_data* extract_records_;
extern import_data* import_data_;
extern intl_dialog* intl_dialog_;
extern main_window* main_window_;
extern qrz_handler* qrz_handler_;
extern qso_manager* qso_manager_;
extern spec_data* spec_data_;
extern status* status_;
extern tabbed_forms* tabbed_forms_;
extern ticker* ticker_;
extern toolbar* toolbar_;
extern url_handler* url_handler_;
extern wsjtx_handler* wsjtx_handler_;

extern bool READ_ONLY;
extern list<string> recent_files_;
extern string default_data_directory_;
extern string PROGRAM_ID;
extern string VENDOR;
extern time_t session_start_;
extern void open_html(const char*);

extern Fl_Preferences::Root prefs_mode_;



	// The default menu - set of menu items
	Fl_Menu_Item menu_items[] = {
		// File operations
	{ "&File", 0, 0, 0, FL_SUBMENU },
		{ "&New", 0, menu::cb_mi_file_new, 0 },
		{ "&Open", 0, menu::cb_mi_file_open, 0 },
		{ "Rea&d", 0, menu::cb_mi_file_open, (void*)-1L },
		{ "&Save", 0, menu::cb_mi_file_save, (void*)OT_MAIN },
		{ "Save &As", 0, menu::cb_mi_file_saveas, (void*)OT_MAIN },
		{ "&Close", 0, menu::cb_mi_file_close, 0 },
		{ "&Print Log", 0, menu::cb_mi_file_print, (void*)OT_MAIN, FL_MENU_DIVIDER },
		{ "&Recent", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
		// Extra menu items are dynamically inserted here 
			{ 0 },
		{ "&Backup", 0, menu::cb_mi_file_backup, (void*)false },
		{ "Res&tore", 0, menu::cb_mi_file_restore, 0 },
		{ 0 },

	// Settings operations
	{ "&Settings", 0, 0, 0, FL_SUBMENU },
		{ "&Web/Network", 0, menu::cb_mi_settings, (void*)config::DLG_WEB },
		{ "&Fields", 0, menu::cb_mi_settings, (void*)config::DLG_COLUMN },
		{ "&Station", 0, menu::cb_mi_settings, (void*)config::DLG_STATION },
		{ "&User config", 0, menu::cb_mi_settings, (void*)config::DLG_USER },
		{ "&QSL design", 0, menu::cb_mi_settings, (void*)config::DLG_QSLE },
		{ "&Contests", 0, menu::cb_mi_settings, (void*)config::DLG_CONTEST },
		{ "&All", 0, menu::cb_mi_settings, (void*)config::DLG_ALL },
		{ 0 },

	// Windows viewing
	{ "&Windows", 0, 0, 0, FL_SUBMENU },
		{ "&Show All", 0, menu::cb_mi_windows_all, (void*)true },
		{ "&Hide All", 0, menu::cb_mi_windows_all, (void*)false },
		// Extra items to be added here
		{ 0 },

	// Log navigation
	{ "&Navigate", 0, 0, 0, FL_SUBMENU },
		{ "&First", 0, menu::cb_mi_navigate, (void*)(NV_FIRST) },
		{ "&Previous", 0, menu::cb_mi_navigate, (void*)(NV_PREV) },
		{ "Ne&xt", 0, menu::cb_mi_navigate, (void*)(NV_NEXT) },
		{ "&Last", 0, menu::cb_mi_navigate, (void*)(NV_LAST) },
		{ "&Date", 0, menu::cb_mi_nav_date	, nullptr },
		{ "&Record No.", 0, menu::cb_mi_nav_recnum, nullptr },
		{ "F&ind", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
			{ "&New", 0, menu::cb_mi_nav_find, (void*)true },
			{ "Ne&xt", 0, menu::cb_mi_nav_find, (void*)false },
			{ 0 },
		{ 0 },

	// Log operation
	{ "&Log", 0, 0, 0, FL_SUBMENU },
		{ "&New record", 0, menu::cb_mi_log_new, nullptr },
		{ "&Save record", 0, menu::cb_mi_log_save, nullptr },
		{ "Re&time record", 0, menu::cb_mi_log_retime, nullptr },
		{ "&Cancel", 0, menu::cb_mi_log_del, (void*)false },
		{ "&Delete record", 0, menu::cb_mi_log_del, (void*)true, FL_MENU_DIVIDER },
		{ "&Parse record", 0, menu::cb_mi_parse_qso, 0 },
		{ "&Unparse record", 0, menu::cb_mi_unparse_qso, 0 },
		{ "R&eparse record", 0, menu::cb_mi_reparse_qso, 0 },
		{ "&Validate record", 0, menu::cb_mi_valid8_qso, 0, FL_MENU_DIVIDER },
		{ "Pa&rse log", 0, menu::cb_mi_parse_log, 0 },
		{ "Val&idate log", 0, menu::cb_mi_valid8_log, 0, FL_MENU_DIVIDER },
		{ "Suspend Save", 0, menu::cb_mi_log_ssave, 0, FL_MENU_TOGGLE },
		{ "&Bulk changes", 0, menu::cb_mi_log_bulk, 0 },
		{ "Chec&k Duplicates", 0, menu::cb_mi_log_dupes, 0 },
		{ "Edit &Header", 0, menu::cb_mi_log_edith, 0 },
		{ "S&ession", 0, 0, 0, FL_SUBMENU },
			{ "&Today", 0, menu::cb_mi_log_start, (void*)2L },
			{ "&Start Session", 0, menu::cb_mi_log_start, (void*)1L },
			{ "Sto&p Session", 0, menu::cb_mi_log_start, (void*)0L },

			{ 0 },
		{ 0 },

	// Log Extract and export operations
	{ "E&xtract", 0, 0, 0, FL_SUBMENU },
		{ "Clea&r", 0, menu::cb_mi_ext_clr, 0 },
		{ "&Criteria", 0, menu::cb_mi_ext_crit, 0 },
		{ "Re&do", 0, menu::cb_mi_ext_redo, 0 },
		{ "Special", 0, 0, 0, FL_SUBMENU },
			{ "No &Name", 0, menu::cb_mi_ext_special, (void*)extract_data::NO_NAME },
			{ "No &QTH", 0, menu::cb_mi_ext_special, (void*)extract_data::NO_QTH },
			{ "Small &Locator", 0, menu::cb_mi_ext_special, (void*)extract_data::LOCATOR },
			{ "No &Image", 0, menu::cb_mi_ext_no_image, 0 },
			{ 0 },
		{ "&Display", 0, menu::cb_mi_ext_disp, 0, FL_MENU_DIVIDER },
		{ "e&QSL", 0, menu::cb_mi_ext_qsl, (void*)extract_data::EQSL },
		{ "&LotW", 0, menu::cb_mi_ext_qsl, (void*)extract_data::LOTW },
		{ "Car&d", 0, menu::cb_mi_ext_qsl, (void*)extract_data::CARD },
		{ "e-&Mail", 0, menu::cb_mi_ext_qsl, (void*)extract_data::EMAIL },
		{ "Club&Log", 0, menu::cb_mi_ext_qsl, (void*)extract_data::CLUBLOG },
		{ "QR&Z.com", 0, menu::cb_mi_ext_qsl, (void*)extract_data::QRZCOM, FL_MENU_DIVIDER },
		{ "&Save", 0, menu::cb_mi_file_saveas, (void*)OT_EXTRACT },
		{ "&Upload", 0, menu::cb_mi_ext_upload, 0 },
		{ "&Print Cards", 0, menu::cb_mi_ext_print, 0 },
		{ "Send e-mail", 0, menu::cb_mi_ext_email, 0 },
		{ "Mar&k sent", 0, menu::cb_mi_ext_mark, 0 },
		{ "Download &Images", 0, menu::cb_mi_ext_dl_images, 0 },
		{ 0 },

	// Log import operations
	{ "&Import", 0, 0, 0, FL_SUBMENU },
		{ "&File", 0, menu::cb_mi_imp_file, (void*)(long)import_data::FILE_IMPORT },
		{ "File (Up&date QSOs)", 0, menu::cb_mi_imp_file, (void*)(long)import_data::FILE_UPDATE },
		{ "File (QR&Z.com)", 0, menu::cb_mi_imp_file, (void*)(long)import_data::QRZCOM_UPDATE },
		{ "Download e&QSL", 0, menu::cb_mi_download, (void*)(long)import_data::EQSL_UPDATE },
		{ "Download &LotW", 0, menu::cb_mi_download, (void*)(long)import_data::LOTW_UPDATE },
		{ "Download &QRZ.com", 0, menu::cb_mi_download, (void*)(long)import_data::QRZCOM_UPDATE, FL_MENU_DIVIDER },
		{ "Clip&board", 0, menu::cb_mi_imp_clipb, nullptr },
		{ "&WSJT-X UDP", 0, menu::cb_mi_imp_wsjtx, nullptr, FL_MENU_DIVIDER },
		{ "&Merge (New QSOs)", 0, menu::cb_mi_imp_merge, (void*)(long)import_data::FILE_IMPORT },
		{ "Merge (&Update QSOS)", 0, menu::cb_mi_imp_merge, (void*)(long)import_data::EXISTING },
		{ "&Cancel", 0, menu::cb_mi_imp_cancel, nullptr },
		{ 0 },

	// Log analysis reports
	{ "Re&port", 0, 0, 0, FL_SUBMENU },
		{ "&Clear", 0, menu::cb_mi_rep_filter, (void*)RF_NONE, FL_MENU_RADIO },
		{ "&All", 0, menu::cb_mi_rep_filter, (void*)RF_ALL, FL_MENU_RADIO | FL_MENU_VALUE },
		{ "A&ll Station", 0, menu::cb_mi_rep_filter, (void*)RF_ALL_CURRENT, FL_MENU_RADIO },
		{ "E&xtracted", 0, menu::cb_mi_rep_filter, (void*)RF_EXTRACTED, FL_MENU_RADIO },
		{ "&Selected record", 0, menu::cb_mi_rep_filter, (void*)RF_SELECTED, FL_MENU_RADIO | FL_MENU_DIVIDER },
		{ "Level &1", 0, 0, 0, FL_SUBMENU },
			{ "&Entities", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_DXCC), FL_MENU_RADIO },
			{ "Entities/&States", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_PAS), FL_MENU_RADIO | FL_MENU_VALUE },
			{ "&Bands", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_BAND), FL_MENU_RADIO },
			{ "&Modes", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_MODE), FL_MENU_RADIO },
			{ "&Callsigns", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_CALL), FL_MENU_RADIO },
			{ "Cus&tom", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_CUSTOM), FL_MENU_RADIO },
			{ 0 },
		{ "Level &2", 0, 0, 0, FL_SUBMENU },
			{ "&Entities", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_DXCC), FL_MENU_RADIO },
			{ "Entities/&States", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_PAS), FL_MENU_RADIO },
			{ "&Bands", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_BAND), FL_MENU_RADIO },
			{ "&Modes", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_MODE), FL_MENU_RADIO },
			{ "&Callsigns", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_CALL), FL_MENU_RADIO },
			{ "Cus&tom", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_CUSTOM), FL_MENU_RADIO },
			{ "&Nothing", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_EMPTY), FL_MENU_RADIO | FL_MENU_VALUE },
			{ 0 },
		{ "Level &3", 0, 0, 0, FL_SUBMENU },
			{ "&Entities", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_DXCC), FL_MENU_RADIO },
			{ "Entities/&States", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_PAS), FL_MENU_RADIO },
			{ "&Bands", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_BAND), FL_MENU_RADIO },
			{ "&Modes", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_MODE), FL_MENU_RADIO },
			{ "&Callsigns", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_CALL), FL_MENU_RADIO },
			{ "Cus&tom", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_CUSTOM), FL_MENU_RADIO },
			{ "&Nothing", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_EMPTY), FL_MENU_RADIO | FL_MENU_VALUE },
			{ 0 },
		{ 0 },

	// Web-based information
	{ "&Information", 0, 0, 0, FL_SUBMENU },
		{ "&QRZ.com", 0, menu::cb_mi_info_qrz },
		{ "Google &Maps", 0, menu::cb_mi_info_map },
		{ "QSO &Web-site", 0, menu::cb_mi_info_web },
		{ 0 },

	// Program help features
	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "&About", 0, menu::cb_mi_help_abt },
		{ "&User Guide", 0, menu::cb_mi_help_ug },
		{ 0 },
	{ 0 }
	};

extern void add_data();
extern void main_window_label(string text);
extern void backup_file();
extern void restore_backup();
extern void set_recent_file(string filename);


// Constructor
menu::menu(int X, int Y, int W, int H, const char* label) :
	Fl_Menu_Bar(X, Y, W, H, label)
{
	// Add the menu
	Fl_Menu_Bar::menu(menu_items);
	// Add the recent files list to it
	add_recent_files();
	// default text size - just larger than default font size
	textsize(FL_NORMAL_SIZE + 1);
	criteria_ = nullptr;
	
	show();
}

// Destructor
menu::~menu()
{
	clear();
}

// Handle
int menu::handle(int event) {
	int result = Fl_Menu_Bar::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("menu.html");
			return true;
		}
		break;
	}
	return result;
}

// File->New
// v is not used
void menu::cb_mi_file_new(Fl_Widget* w, void* v) {
	// Gracefully stop any import in progress - restart with ON_AIR logging - if no rig will drop to OFF_AIR
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();
	if (book_->is_dirty_record(book_->get_record()) || book_->new_record()) {
		fl_beep(FL_BEEP_QUESTION);
		switch (fl_choice("You are currently modifying a record? Save or Quit?", "Save?", "Quit?", nullptr)) {
		case 0:
			qso_manager_->end_qso();
			break;
		case 1:
			book_->delete_record(false);
			break;
		}
	}
	if (book_->is_dirty()) {
		fl_beep(FL_BEEP_QUESTION);
		if (fl_choice("Book has been modified, do you want to save?", "Yes", "No", nullptr) == 0) {
			book_->store_data();
		}
	}

	ticker_->stop_all();
	book_->delete_contents(true);
	qso_manager_->deactivate_all();

	// get book to delete contents by loading no data
	book_->load_data("");
	// Allow writes
	READ_ONLY = false;

	// Clear any extracted data as there are no records to reference
	extract_records_->clear_criteria();
	set_recent_file("");
	tabbed_forms_->activate_pane(OT_MAIN, true);
	book_->navigate(NV_FIRST);
}

// File->Open
// v is a long. -1 = open browser and open file read-only; 0 = open browser and open file; 1-4 = open specific recent file
void menu::cb_mi_file_open(Fl_Widget* w, void* v) {
	// Stop any import occurring
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();

	if (book_->is_dirty()) {
		fl_beep(FL_BEEP_QUESTION);
		if (fl_choice("Book has been modified, do you want to save?", "Yes", "No", nullptr) == 0) {
			// Do File->Save first
			cb_mi_file_save(w, v);
		}
	}

	ticker_->stop_all();
	book_->delete_contents(true);
	qso_manager_->deactivate_all();
	
	// Set to a recent file number (1 to 4) or 0 for file_chooser
	char file_id = (char)(intptr_t)v;
	string filename = "";
	// Set read_only flag
	if (file_id < 0) {
		READ_ONLY = true;
	}
	else {
		READ_ONLY = false;
	}
	// Open chooser and open read/write (0) or read only (-1)
	if (file_id <= 0) {
		// Open file chooser to get file to load
		Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
		Fl_Preferences datapath_settings(settings, "Datapath");
		char* directory;
		datapath_settings.get("Log Directory", directory, "");
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser->title("Select file name to load");
		chooser->directory(directory);
		chooser->filter("ADI Files\t*.adi\nADX Files\t*.adx");
		if (chooser->show() == 0) {
			filename = chooser->filename();
		}
		free(directory);
		delete chooser;
	}
	else {
		// Get the recent file with supplied ID
		Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
		Fl_Preferences recent_settings(settings, "Recent Files");
		char * temp;
		char path[6];
		sprintf(path, "File%c", file_id);
		recent_settings.get(path, temp, "");
		filename = temp;
		free(temp);
	}
	if (filename.length() > 0) {
		// Only open a file if it has a name
		extract_records_->clear_criteria();
		// get book to load it.
		if (book_->load_data(filename)) {
			tabbed_forms_->activate_pane(OT_MAIN, true);
			book_->navigate(NV_LAST);
			// Set the filename in the window title and recent file list
			main_window_label(filename);
			set_recent_file(filename);
		}

	}
}

// File->Save
// v is set to 0 to represent object_t = OT_MAIN
void menu::cb_mi_file_save(Fl_Widget* w, void* v) {
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();
	// Get the main book to store itself as long as it wasn't opened read-only
	if (READ_ONLY) {
		fl_beep(FL_BEEP_QUESTION);
		if (fl_choice("File was opened read-only - save as new file?", "OK", "No", nullptr) == 0) {
			cb_mi_file_saveas(w, v);
		}
	}
	else {
		// If we have a file loaded then save it
		if (book_->filename().length()) {
			book_->store_data();
			READ_ONLY = false;
		}
		// otherwise it's a new file with added records - ask for name
		else {
			cb_mi_file_saveas(w, v);
		}
	}
}

// File->SaveAs 
// v is set to the enum object_t. OT_MAIN = save main book, OT_EXTRACT = save extracted records
void menu::cb_mi_file_saveas(Fl_Widget* w, void* v) {
	string filename = book_->filename();
	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	chooser->title("Select file name to save");
	chooser->filter("ADI Files\t*.adi\nADX Files\t*.adx\nTSV Files\t*.{tsv,tab}");
	chooser->preset_file(filename.c_str());
	if (chooser->show() == 0) {
		filename = chooser->filename();
		// No file type - force it to .adi
		string suffix = filename.substr(filename.length() - 4);
		if (suffix != ".adi" && suffix != ".adx" && suffix != ".tsv" && suffix != ".tab") {
			filename += ".adi";
		}
		// Got a filename
		ifstream* check = new ifstream(filename.c_str());
		if (check->fail() || fl_choice("File exists, do you want to over-write", "OK", "No", nullptr) == 0) {
			// Stop any current import
			import_data_->stop_update(false);
			while (!import_data_->update_complete()) Fl::check();
			// Get the book to save
			book* b = nullptr;
			object_t type = (object_t)(intptr_t)v;
			switch (type) {
			case OT_MAIN:
				b = book_;
				break;
			case OT_EXTRACT:
				b = extract_records_;
				break;
			default:
				break;
			}
			// Save even if not modified
			b->store_data(filename, true);
			if (type == OT_MAIN) {
				// Change filename in title and top of recent file list
				main_window_label(filename);
				set_recent_file(filename);
			}
		}
		delete check;
		READ_ONLY = false;
	}
	delete chooser;
}

// File->Close
// v is not used
void menu::cb_mi_file_close(Fl_Widget* w, void* v) {
	// Gracefully stop import
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();

	ticker_->stop_all();
	qso_manager_->deactivate_all();

	main_window_->do_callback();
}

// File->Print
// v is not used
void menu::cb_mi_file_print(Fl_Widget* w, void* v) {
	// Print the current book
	object_t type = (object_t)(intptr_t)v;
	if (type == OT_MAIN) {
		type = navigation_book_->book_type();
	}
	printer* ptr = new printer(type);
	ptr->do_job();
	delete ptr;
}

// File->Backup
// v is not used
void menu::cb_mi_file_backup(Fl_Widget*, void* v) {
	// Back up
	backup_file();
}

// File->Restore
// v is not used
void menu::cb_mi_file_restore(Fl_Widget* w, void* v) {
	// Restore backup
	restore_backup();
}

// Config->any
// v is enum cfg_dialog_t indicating which config page to open at
void menu::cb_mi_settings(Fl_Widget* w, void* v) {
	// v provides that id of the page of the config dialogs to open with
	config::cfg_dialog_t active = (config::cfg_dialog_t)(intptr_t)v;
	config_->set_tab(active);
	config_->show();
}

// Windows->Show All|Hide All
// v is bool. false = hide all, true = show all
void menu::cb_mi_windows_all(Fl_Widget* w, void* v) {
	bool show_all = (bool)(intptr_t)v;
	menu* that = ancestor_view<menu>(w);
	if (show_all) {
		main_window_->show();
		qso_manager_->show();
		// status_->file_viewer()->show();
		intl_dialog_->show();
	}
	else {
		// Minimise the main window rather than hide it. When all windows are hidden we end the app
		main_window_->iconize();
		qso_manager_->hide();
		// status_->file_viewer()->hide();
		intl_dialog_->hide();
	}
	that->update_windows_items();
}

// Windows->Other
// v is Fl_Window* and points to the specified window
void menu::cb_mi_windows(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	Fl_Window* win = (Fl_Window*)v;
	if (win) {
		if (win == main_window_ && win->visible()) {
			win->iconize();
		}
		else if (win->visible()) {
			win->hide();
		}
		else {
			win->show();
		}
	}
	that->update_windows_items();
}

// Navigate->First,Previous,Next,Last callbacks
// v is enum navigate_t indicating which record to go to
void menu::cb_mi_navigate(Fl_Widget* w, void* v) {
	// v supplies that navigation target
	navigate_t target = (navigate_t)(intptr_t)v;
	// Navigate to the specified point in the specified book
	if (navigation_book_ != nullptr) {
		navigation_book_->navigate(target);
	}
}

// Navigate->Date: Opens a calendar window
// v is ignored
void menu::cb_mi_nav_date(Fl_Widget* w, void* v) {
	// Populate calendar with today's date
	record* qso = navigation_book_->get_record();
	string date;
	if (qso) date = qso->item("QSO_DATE");
	else date = now(false, "%Y%m%d");
	calendar* cal = new calendar(Fl::event_x_root(), Fl::event_y_root());
	cal->value(date.c_str());
	cal->callback(cb_value<calendar, string>, &date);
	cal->show();
	Fl_Widget_Tracker wt(cal);
	while (wt.exists()) Fl::check();
	// now fiind the selected date
	if (navigation_book_ != nullptr) {
		navigation_book_->go_date(date);
	}
}

// Navigate->Record Number
// v is ignored
void menu::cb_mi_nav_recnum(Fl_Widget* w, void* v) {
	int record_num;
	// get record number
	const char* reply = fl_input("Enter record number");
	if (reply) {
		// Valid reply - convert to integer and select item
		record_num = atoi(reply);
		if (record_num > 0) {
			item_num_t item_num = navigation_book_->item_number(record_num - 1, true);
			navigation_book_->selection(item_num);
		}
	}
}

// Navigate->Find
// v is a bool. false = find next; true = new search
void menu::cb_mi_nav_find(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// v supplies whether to change search criteria
	bool do_extract = (bool)(intptr_t)v;
	if (do_extract || that->criteria_ == nullptr) {
		// New search criteria - open dialog
		delete that->criteria_;
		search_dialog* dialog = new search_dialog;
		while (do_extract && dialog->display() == BN_OK) {
			that->criteria_ = dialog->criteria();
			// Find first record with those criteria
			int ix = navigation_book_->search(that->criteria_, do_extract);
			if (ix >= 0) {
				// Got criteria - force out of loop
				do_extract = false;
				// Navigate to found record
				navigation_book_->selection(ix);
			}
			else {
				// Tell user no records match
				dialog->fail("No matching record, try again or cancel?");
			}
		}
		Fl::delete_widget(dialog);
	}
	else {

		bool search = true;
		while (search) {
			// Find next record wth these criteria
			int ix = navigation_book_->search(that->criteria_, do_extract);
			if (ix >= 0) {
				// There is one select it
				navigation_book_->selection(ix);
				search = false;
			}
			else {
				// Reached the end of file, go back to the bein or cancel
				if (fl_choice("Reached the end of the file, start again or cancel", "Start again", "Cancel", 0) == 1) {
					search = false;
				}
			}
		}
	}
}

// Log->Parse QSO - parse the selectedrecord
// v is not used
void menu::cb_mi_parse_qso(Fl_Widget* w, void* v) {
	// Get selected record
	record* record = navigation_book_->get_record();
	// get parse mode
	if (true) {
		// command parsing enabled - parse record
		bool parse_result = cty_data_->update_qso(record);
		// update band
		bool changed = record->update_band(true);
		if (changed || parse_result) {
			// If modified - tell views to update
			navigation_book_->selection(-1, HT_CHANGED);
		}
	}
}

// Log->Unparse QSO - remove the parsed fields
// v is not used
void menu::cb_mi_unparse_qso(Fl_Widget* w, void* v) {
	// Get selected record
	record* record = navigation_book_->get_record();
	record->unparse();
	navigation_book_->selection(-1, HT_CHANGED);
}

// Log->Reparse QSO - Unparse followed by parse
// v is not used
void menu::cb_mi_reparse_qso(Fl_Widget* w, void* v) {
	cb_mi_unparse_qso(w, v);
	cb_mi_parse_qso(w, v);
}

// Log ->Parse Log - parse all records in selected book
// v is not used
void menu::cb_mi_parse_log(Fl_Widget* w, void* v) {
	// display hourglass while we validate the log
	fl_cursor(FL_CURSOR_WAIT);
	// Get parse mode
	int num_items = navigation_book_->size();
	book_->enable_save(false, "Starting parsing log");
	if (true) {	
		// If command parsing allowed
		bool abandon = false;
		int item_number = 0;
		// Initialise progress
		status_->misc_status(ST_NOTE, "LOG: Started parsing");
		status_->progress(num_items, navigation_book_->book_type(), "Parsing log", "records");
		// For all records in selected book
		for (int i = 0; i < num_items && !abandon;) {
			record* record = navigation_book_->get_record(i, false);
			// Parse each record in turn
			bool parse_result = cty_data_->update_qso(record);
			bool changed = record->update_band(true);
			item_number = i;
			// Update progress
			status_->progress(++i, navigation_book_->book_type());
			if (changed || parse_result) {
				// The parsing may have modified date and time
				book_->correct_record_position(navigation_book_->record_number(item_number));
			}
		}
		// update views with last record parsed selected
		status_->misc_status(ST_OK, "LOG: Parsing done!");
		navigation_book_->selection(item_number, HT_CHANGED);
	}
	book_->enable_save(true, "Ended parsing log");
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Log->Validate QSO - validate selected record
// v is not used
void menu::cb_mi_valid8_qso(Fl_Widget* w, void* v) {
	record* record = navigation_book_->get_record();
	qso_num_t record_num = navigation_book_->record_number(navigation_book_->selection());
	book_->enable_save(false, "Started validate QSO");
	// If command validation is enabled
	bool changed = spec_data_->validate(record, record_num);
	if (changed) {
		// Update views if record has changed
		item_num_t item_num = navigation_book_->item_number(record_num);
		navigation_book_->selection(item_num, HT_MINOR_CHANGE);
	}
	book_->enable_save(true, "Ended validate QSO");
}
 
// Log->Validate Log - validate all records in selected log
// v is not used
void menu::cb_mi_valid8_log(Fl_Widget* w, void* v) {
	fl_cursor(FL_CURSOR_WAIT);
		// Command validation is enabled
	int record_num = -1;
	int item_number = 0;
	int num_items = navigation_book_->size();
	bool changed = false;
	// Initialse progress
	book_->enable_save(false, "Started validate log");
	status_->misc_status(ST_NOTE, "VALIDATE: Started");
	status_->progress(num_items, navigation_book_->book_type(), "Validating log", "records");
	spec_data_->reset_continue();
	// For each record in selected book unless user has indicated to quit
	for (int i = 0; i < num_items && spec_data_->do_continue(); ) {
		record_num = navigation_book_->record_number(i);
		item_number = i;
		record* record = navigation_book_->get_record(i, false);
		// validate it
		if (spec_data_->validate(record, record_num)) {
			changed = true;
		}
		status_->progress(++i, navigation_book_->book_type());
	}
	status_->misc_status(ST_OK, "VALIDATE: Done!");
	if (changed) {
		navigation_book_->selection(item_number, HT_MINOR_CHANGE);
	}
	book_->enable_save(true, "Ended validate log");
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Log->New - start a new record
// v is not used
void menu::cb_mi_log_new(Fl_Widget* w, void* v) {
	// Force main log book
	tabbed_forms_->activate_pane(OT_MAIN, true);
	// Create a new record - on or off-air
	qso_manager_->start_qso();
}


// Log->Save - save current record
// v is not used
void menu::cb_mi_log_save(Fl_Widget* w, void* v) {
	qso_manager_->end_qso();
	qso_manager_->update_rig();
}

// Log->Delete/Cancel
// v is bool. false = cancel new record; true = delete record
void menu::cb_mi_log_del(Fl_Widget* w, void* v) {
	// delete_record(true) - deliberately deleting a record
	// delete_record(false) - only deletes if entering a new record (i.e. cancel)
	navigation_book_->delete_record((bool)(intptr_t)v);
	qso_manager_->update_rig();
}

// Log->Retime record - reset TIME_OFF to now
// v is not used
void menu::cb_mi_log_retime(Fl_Widget* w, void* v) {
	record* this_record = navigation_book_->get_record();
	if (!this_record) {
		status_->misc_status(ST_ERROR, "LOG: Do not have a current record to retime");
		return;
	}
	string old_time = this_record->item("TIME_OFF");
	string time = now(false, "%H%M%S");
	this_record->item("TIME_OFF", time);
	char message[200];
	snprintf(message, 200, "LOG: %s %s %s record changed %s from %s to %s",
		this_record->item("QSO_DATE").c_str(),
		this_record->item("TIME_ON").c_str(),
		this_record->item("CALL").c_str(),
		"TIME_OFF",
		old_time.c_str(),
		time.c_str());
	status_->misc_status(ST_NOTE, message);
	tabbed_forms_->update_views(nullptr, HT_CHANGED, navigation_book_->selection());
	menu* that = ancestor_view<menu>(w);
	that->update_items();
}

// Log->Bulk Change - Do the same mod on all extracted records (all if none extracted)
// v is not used
void menu::cb_mi_log_bulk(Fl_Widget* w, void* v) {
	// Open dialog to allow user to define change
	change_dialog* dialog = new change_dialog("Bulk change fields");
	bool repeat = true;
	while (repeat) {
		button_t result = dialog->display();
		if (result != BN_CANCEL) {
			change_action_t action = CHANGE_FIELD;
			string old_field_name = "";
			string new_field_name = "";
			string new_text = "";
			int num_changed = 0;
			fl_cursor(FL_CURSOR_WAIT);
			// get action
			dialog->get_data(action, old_field_name, new_field_name, new_text);
			status_->progress(navigation_book_->size(), navigation_book_->book_type(), "Changing data in bulk", "records");
			status_->misc_status(ST_NOTE, "LOG: Bulk change started");
			// For each record in selected book
			for (item_num_t i = 0; i < navigation_book_->size(); i++) {
				record* record = navigation_book_->get_record(i, false);

				switch (action) {
				case RENAME_FIELD:
					// Change the field_name from old to new
					record->change_field_name(old_field_name, new_field_name);
					num_changed++;
					break;
				case DELETE_FIELD:
					// Delete the field with this name
					record->item(old_field_name, string(""));
					num_changed++;
					break;
				case ADD_FIELD:
					// Add a field with this name and value
					if (record->item(old_field_name) == "") {
						record->item(old_field_name, new_text);
						num_changed++;
					}
					break;
				case CHANGE_FIELD:
					// Change the field to this value - adding the field if necessary
					record->item(old_field_name, new_text);
					num_changed++;
					break;
				}
				status_->progress(i + 1, navigation_book_->book_type());
			}
			// Generate message
			char message[256];
			switch (action) {
			case RENAME_FIELD:
				snprintf(message, 256, "LOG: Bulk Change done, %d records: Field %s renamed as %s",
					num_changed, old_field_name.c_str(), new_field_name.c_str());
				break;
			case DELETE_FIELD:
				snprintf(message, 256, "LOG: Bulk Change dome, %d records: Field %s deleted",
					num_changed, old_field_name.c_str());
				break;
			case ADD_FIELD:
				snprintf(message, 256, "LOG: Bulk_Change done: %d/%u records: Field %s changed from no value to %s",
					num_changed, (int)navigation_book_->size(), old_field_name.c_str(), new_text.c_str());
				break;
			case CHANGE_FIELD:
				snprintf(message, 256, "LOG: Bulk Change done, %d records: Field %s set to %s",
					num_changed, old_field_name.c_str(), new_text.c_str());
				break;
			}
			status_->misc_status(ST_OK, message);
			// Get all views to update
			book_->selection(book_->selection(), HT_ALL);
			fl_cursor(FL_CURSOR_DEFAULT);
		}
		// If OK or CANCEL we close the dialog
		if (result != BN_SPARE) {
			repeat = false;
		}
	}
	Fl::delete_widget(dialog);
}

// Log->Check Duplicates - call book's check duplicates
// v is not used
void menu::cb_mi_log_dupes(Fl_Widget* w, void* v) {
	navigation_book_->check_dupes(false);
}

// Log->Edit Header - open editor on header comment
// v is not used
void menu::cb_mi_log_edith(Fl_Widget* w, void* v) {
	navigation_book_->edit_header();
}

// Operate->Start Session
// v is long: 0 is now, 1 is selected QSO, 2 is mos recent QSO
void menu::cb_mi_log_start(Fl_Widget* w, void* v) {
	long mode = (intptr_t)v;
	item_num_t save_pos = book_->selection();
	switch (mode) {
	case 0:
		session_start_ = time(nullptr);
		break;
	case 1: {
		// Get selected record timestamp 
		record* start = book_->get_record();
		session_start_ = start->timestamp();
		break;
	}
	case 2: {
		// Get most recent QSO
		item_num_t pos = book_->size() - 1;
		record* start = book_->get_record(pos, false);
		string qso_date = start->item("QSO_DATE");
		pos--;
		while (book_->get_record(pos, false)->item("QSO_DATE") == qso_date) {
			start = book_->get_record(pos, false);
			pos--;
		}
		session_start_ = start->timestamp();
		break;
	}
	}
	// Display the start time in the status log
	char stime[100];
	tm* start_time = gmtime(&session_start_);
	strftime(stime, 100, "%Y/%m/%d %H:%M:%S", start_time);
	char message[256];
	snprintf(message, 256, "ZZALOG: Setting session start at %s", stime);
	status_->misc_status(ST_NOTE, message);
	tabbed_forms_->update_views(nullptr, HT_ALL, save_pos);
}

// Log->Suspend Save
// If suspeneded remove all bars on saving
// v is not used
void menu::cb_mi_log_ssave(Fl_Widget* w, void* v) {
	// Get the value of the checked menu item
	Fl_Menu_* m = (Fl_Menu_*)w;
	const Fl_Menu_Item* item = m->mvalue();
	bool value = item->value();
	if (value) {
		book_->enable_save(false, "Disabling from menu");
	} else {
		while(!book_->enable_save()) book_->enable_save(true, "Enabling from menu");
	}
}

// Import->File
// v is not used
void menu::cb_mi_imp_file(Fl_Widget* w, void* v) {
	// Cancel any existing update
	import_data_->stop_update(false);
	while (!import_data_->update_complete()) Fl::check();
	// Get data directory
	char* directory;
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences datapath_settings(settings, "Datapath");
	datapath_settings.get("Log Directory", directory, "");
	// Open file chooser
	string filename;
	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser->title("Select file name");
	chooser->directory(directory);
	chooser->filter("ADI files\t*.adi\nADX Files\t*.adx");
	import_data::update_mode_t mode = (import_data::update_mode_t)(intptr_t)v;
	if (chooser->show() == 0) {
		filename = chooser->filename();
		import_data_->load_data(filename, mode);
	}
	delete chooser;
	free(directory);
}

// Import->Download->eQSL/LotW - fetch the data from the server into the import_data_ book
// v is enum update_mode_t: EQSL_UPDATE or LOTW_UPDATE
void menu::cb_mi_download(Fl_Widget* w, void* v) {
	// v has QSL server
	import_data::update_mode_t server = (import_data::update_mode_t)(intptr_t)v;
	qso_manager_->qsl_download(server);
}

// Import->WSJT-X - start the WSJT-X listener for logging datagrams
// v is not used
void menu::cb_mi_imp_wsjtx(Fl_Widget* w, void* v) {
	if (!wsjtx_handler_) {
		// Create new listener
		wsjtx_handler_ = new wsjtx_handler;
	}
	if (wsjtx_handler_ && !wsjtx_handler_->has_server()) {
		// Restart wsjt-x listener
		wsjtx_handler_->run_server();
	}
}

// Import->Clipboard - send FL_PASTE eveny to main_window to handle paste
// v is not used
void menu::cb_mi_imp_clipb(Fl_Widget* w, void* v) {
	Fl::paste(*main_window_, 1);
}

// Import->Merge - merge what has just been downloaded
// v is update mode
void menu::cb_mi_imp_merge(Fl_Widget* w, void* v) {
	// Merge the data
	import_data::update_mode_t mode = (import_data::update_mode_t)(intptr_t)v;
	import_data_->merge_data(mode);
}

// Import->Cancel
// v is not used
void menu::cb_mi_imp_cancel(Fl_Widget* w, void* v) {
	// Delete the import records
	import_data_->delete_contents(false);
	tabbed_forms_->activate_pane(OT_MAIN, true);
	navigation_book_->selection(0, HT_ALL);
}

// Extract->Clear
// v is not used
// clear the criteria and display the log book
void menu::cb_mi_ext_clr(Fl_Widget* w, void* v) {
	extract_records_->clear_criteria();
	tabbed_forms_->activate_pane(OT_MAIN, true);
}

// Extract->Criteria
// v is not used
// Open the dialog to get the criteria, get the records accordingly and display the extract
void menu::cb_mi_ext_crit(Fl_Widget* w, void* v) {
	// Open search dialog
	search_dialog* dialog = new search_dialog;
	while (dialog->display() == BN_OK) {
		// Extract the records
		search_criteria_t criteria = *dialog->criteria();
		if (extract_records_->criteria(criteria)) {
			// Successful - clear fail message
			dialog->fail("");
			tabbed_forms_->activate_pane(OT_EXTRACT, true);
			navigation_book_->selection(0, HT_EXTRACTION);
		}
		else {
			// No records extracted
			dialog->fail("No records match condition - try again or cancel?");
		}
	}
	Fl::delete_widget(dialog);
}

// Extract->redo - repeat the extraction
// v is not used
void menu::cb_mi_ext_redo(Fl_Widget* w, void* v) {
	extract_records_->reextract();
	// Display the extraction
	tabbed_forms_->activate_pane(OT_EXTRACT, true);
	navigation_book_->selection(0, HT_EXTRACTION);
}

// Extract->display - display the extract criteria in a tooltip window
// v is not used
void menu::cb_mi_ext_disp(Fl_Widget* w, void* v) {
	string message = "There are no extracted records!\n";
	if (extract_records_->size()) {
		message = extract_records_->header()->header();
	}
	// Create a tooltip window at the explain button (in w) X and Y
	Fl_Window* tip_window = ::tip_window(message, main_window_->x_root() + w->x(), main_window_->y_root() + w->y());
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tip_window);

}

// Extract->eQSL/LotW/Card/ClubLog - specificall extract records for QSL use
// v is enum extract_mode_t: EQSL, LOTW, CARD or CLUBLOG
void menu::cb_mi_ext_qsl(Fl_Widget* w, void* v) {
	// v passes the particular option
	extract_data::extract_mode_t server = (extract_data::extract_mode_t)(intptr_t)v;
	qso_manager_->qsl_extract(server);
	}

// Extract->Quick->* - one-click for specific searches
// v is enum extract_mode_t: NO_NAME, NO_QTH or (inadequate) LOCATOR
void menu::cb_mi_ext_special(Fl_Widget* w, void* v) {
	// v passes the particular option
	extract_data::extract_mode_t reason = (extract_data::extract_mode_t)(intptr_t)v;
	extract_records_->extract_special(reason);
	tabbed_forms_->activate_pane(OT_EXTRACT, true);
}

// Extract->No Image-
// v is not used 
void menu::cb_mi_ext_no_image(Fl_Widget* w, void* v) {
	// v passes the particular option
	extract_records_->extract_no_image();
	tabbed_forms_->activate_pane(OT_EXTRACT, true);
}

// Extract->Upload - upload to the server the data was extracted for
// v is not used
void menu::cb_mi_ext_upload(Fl_Widget* w, void* v) {
	qso_manager_->qsl_upload();
}

// Extract->Print - print 
// v is not used
void menu::cb_mi_ext_print(Fl_Widget* w, void* v) {
	qso_manager_->qsl_print();
}

// Extract->eMail
void menu::cb_mi_ext_email(Fl_Widget* w, void* v) {
	qso_manager_->qsl_email();
}

// Extract->Mark sent
// Update the extracted records with the fct that a QSL has been sent to the appropriate place
// v is not used
void menu::cb_mi_ext_mark(Fl_Widget* w, void* v) {
	qso_manager_->qsl_print_done();
}

// Extract->Download Images
// v is not used
void menu::cb_mi_ext_dl_images(Fl_Widget* w, void* v) {
	for (item_num_t ix = 0; ix < extract_records_->size(); ix++) {
		qso_num_t qn = extract_records_->record_number(ix);
		eqsl_handler_->enqueue_request(qn, true);
	}
	eqsl_handler_->enable_fetch(eqsl_handler::EQ_START);
}

// Report->Clear etc. - set the report filter
// v is enum report_filter_t: RF_NONE, RF_ALL, RF_ALL_CURRENT, RF_EXTRACTED or RF_SELECTED
void menu::cb_mi_rep_filter(Fl_Widget* w, void* v) {
	// v has the filter
	report_filter_t filter = (report_filter_t)(intptr_t)v;
	((report_tree*)tabbed_forms_->get_view(OT_REPORT))->add_filter(filter);
	tabbed_forms_->activate_pane(OT_REPORT, true);
}

// Report->Levelx - set report level n to category
// v contains two bytes { level, category }
void menu::cb_mi_rep_level(Fl_Widget* w, void* v) {
	long params = (intptr_t)v;
	string custom_field = "";
	report_cat_t category = (report_cat_t)(params & 0xFF);
	int level = params >> 8;
	if (category == RC_CUSTOM) {
		// TODO - replace with custom dialog that uses field_choice
		const char* custom = fl_input("Select Custom field name");
		if (custom) {
			custom_field = custom;
		}
	}
	((report_tree*)tabbed_forms_->get_view(OT_REPORT))->add_category(level, category, custom_field);
	tabbed_forms_->activate_pane(OT_REPORT, true);
}

// Information->QRZ.com - query QRZ.com about the selected contact
// v is string*. nullptr = uses selected record else uses call sign in v.
void menu::cb_mi_info_qrz(Fl_Widget* w, void* v) {
	if (v == nullptr) {
		// Treat as button(s) within qso_manager_
		qso_manager_->merge_qrz_com();
	}
	else {
		qrz_handler_->open_web_page(*(string*)v);
	}
}

// Information->Google Maps - open browser with Google maps centered on contact's QTH (where calculable)
// v is not used
void menu::cb_mi_info_map(Fl_Widget* w, void* v) {
	// Get currently selected record
	record* record = book_->get_record();
	if (record != nullptr) {
		// If there is one
		// Get locator, QTH and callsign
		location_t source;
		lat_long_t location = record->location(false, source);
		string locator = record->item("GRIDSQUARE");
		string city = record->item("QTH");
		string label = record->item("CALL");
		// These are DOS specific, will need the Linux version
		string format_coords = "http://google.com/maps/@%f,%f,25000m";
		char message[128];
		char uri[256];
		if ((source == LOC_NONE || source == LOC_GRID2 || source == LOC_GRID4) && city != "") {
			// Location got from wide stuff, therefore use city location
			// ?q=City&z=10 - centre on city zoom level 10
			snprintf(uri, sizeof(uri), "http://google.com/maps/?q=%s&z=10", city.c_str());
			snprintf(message, 128, "INFO: Launching Google maps for %s", city.c_str());
		}
		else if (source == LOC_GRID6 || source == LOC_GRID8 || source == LOC_LATLONG) {
			// grid square is 6 or more characters 
			// @%f,%f,25000m display 25 km around long/lat
			snprintf(uri, sizeof(uri), "http://google.com/maps/@%f,%f,25000m", location.latitude, location.longitude);
			snprintf(message, 128, "INFO: Lunching Google maps for %s", locator.c_str());
		}
		else {
			// Any thing else could be a random location somewhere up to 1 degree away.
			status_->misc_status(ST_WARNING, "INFO: Insufficient QTH information in record to launch Google Maps");
			return;
		}
		status_->misc_status(ST_NOTE, message);
		fl_open_uri(uri);
	}
}

// Information->QSO Web-site
// v is not used
void menu::cb_mi_info_web(Fl_Widget* w, void* v) {
	// Get currently selected record
	record* record = book_->get_record();
	menu* that = ancestor_view<menu>(w);
	if (record != nullptr && record->item_exists("WEB")) {
		// Website logged for contact
		// Get browser from config
		string browser = that->get_browser();
		if (browser.length() == 0) {
			return;
		}
		// Open browser with URL from WEB field in record
		string website = record->item("WEB");
		char uri[256];
		snprintf(uri, sizeof(uri), "%s", website.c_str());
		char message[128];
		snprintf(message, 128, "INFO: Opening website %s", website.c_str());
		status_->misc_status(ST_NOTE, message);
		fl_open_uri(uri);
	}
	else {
		char message[128];
		snprintf(message, 128, "INFO: %s has no website logged", record->item("CALL").c_str());
		status_->misc_status(ST_WARNING, message);
	}
}

// Help->About - display about dialog
// v is not used
void menu::cb_mi_help_abt(Fl_Widget* w, void* v) {
	about_dialog* dialog = new about_dialog();
	//add_sub_window(dialog);
	dialog->display();
	//remove_sub_window(dialog);
	delete dialog;
}

// Help->User Guide
void menu::cb_mi_help_ug(Fl_Widget* w, void* v) {
	open_html("index.html");
}

// Enable/disable menu 
// active: false = disable all; true = enable those which can be enabled
void menu::enable(bool active) {
	active_enabled_ = active;
	// For all menu items
	for (int i = 0; i < size(); i++) {
		if (active) {
			// enable item, specific ones will be disabled later
			mode(i, mode(i) & ~FL_MENU_INACTIVE);
		}
		else {
			// disable item
			mode(i, mode(i) | FL_MENU_INACTIVE);
		}
	}
	if (active) {
		// Disable specific items that do not make sense in the current state of the app
		update_items();
	}
	// Enable/disable toolbar buttons that map onto menu items
	if (toolbar_) toolbar_->update_items();
}

// Add recent files - add most recent 4 filenames to menu
void menu::add_recent_files() {
	// Get position to add the items and delete any existing entries
	int index = find_index("&File/&Recent");
	if (index != -1) {
		clear_submenu(index);
	}
	// Get the filenames - start with File1
	char i = '1';

	for (auto it = recent_files_.begin(); i <= '4' && it != recent_files_.end(); i++, it++) {
		string& filename = *it;
		// We have a filename - Find the last slash in the filename
		int start = filename.find_last_of("/\\");
		char label[100];
		if (start == filename.npos) {
			// No slash in filename so use it
			sprintf(label, "&File/&Recent/%s", filename.c_str());
		} 
		else {
			// else use the end of the file name
			sprintf(label, "&File/&Recent/%s", filename.substr(start + 1).c_str());
		}
		add(label, FL_ALT + i, cb_mi_file_open, (void*)(intptr_t)i);
	}
}

// Set check marks on the menu to represent the actual report tree mode and filter
void menu::report_mode(vector<report_cat_t> report_mode, report_filter_t filter) {
	// Set the Report->Level N->* menu items
	for (int i = 1; i < 4; i++) {
		// Get the item indices to set modes
		char item_label[128];
		sprintf(item_label, "Re&port/Level &%d/&Entities", i);
		int index_entities = find_index(item_label);
		sprintf(item_label, "Re&port/Level &%d/Entities/&States", i);
		int index_states = find_index(item_label);
		sprintf(item_label, "Re&port/Level &%d/&Bands", i);
		int index_bands = find_index(item_label);
		sprintf(item_label, "Re&port/Level &%d/&Modes", i);
		int index_modes = find_index(item_label);
		sprintf(item_label, "Re&port/Level &%d/&Callsigns", i);
		int index_calls = find_index(item_label);
		sprintf(item_label, "Re&port/Level &%d/Cus&tom", i);
		int index_custom = find_index(item_label);
		int index_nothing = -1;
		// Nothing is not an option for Level1
		if (i > 1) {
			sprintf(item_label, "Re&port/Level &%d/&Nothing", i);
			index_nothing = find_index(item_label);
		}
		if (i <= report_mode.size()) {
			if (i > 1) {
				// As we have this level, it can't be nothing
				mode(index_nothing, mode(index_nothing) & ~FL_MENU_VALUE);
			}
			// The report mode at this level
			switch (report_mode[i - 1]) {
			case RC_DXCC:
				// Set Entities only
				mode(index_entities, mode(index_entities) | FL_MENU_VALUE);
				mode(index_states, mode(index_states) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_bands) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_modes) & ~FL_MENU_VALUE);
				mode(index_calls, mode(index_calls) & ~FL_MENU_VALUE);
				mode(index_custom, mode(index_custom) & ~FL_MENU_VALUE);
				break;
			case RC_PAS:
				// Set Entities/States only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_states) | FL_MENU_VALUE);
				mode(index_bands, mode(index_bands) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_modes) & ~FL_MENU_VALUE);
				mode(index_calls, mode(index_calls) & ~FL_MENU_VALUE);
				mode(index_custom, mode(index_custom) & ~FL_MENU_VALUE);
				break;
			case RC_BAND:
				// Set Bands only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_states) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_bands) | FL_MENU_VALUE);
				mode(index_modes, mode(index_modes) & ~FL_MENU_VALUE);
				mode(index_calls, mode(index_calls) & ~FL_MENU_VALUE);
				mode(index_custom, mode(index_custom) & ~FL_MENU_VALUE);
				break;
			case RC_MODE:
				// Set Modes only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_states) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_bands) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_modes) | FL_MENU_VALUE);
				mode(index_calls, mode(index_calls) & ~FL_MENU_VALUE);
				mode(index_custom, mode(index_custom) & ~FL_MENU_VALUE);
				break;
			case RC_CALL:
				// Set Callsigns only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_states) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_bands) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_modes) & ~FL_MENU_VALUE);
				mode(index_calls, mode(index_calls) | FL_MENU_VALUE);
				mode(index_custom, mode(index_custom) & ~FL_MENU_VALUE);
				break;
			case RC_CUSTOM:
				// Set CQ Zones only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_states) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_bands) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_modes) & ~FL_MENU_VALUE);
				mode(index_calls, mode(index_calls) & ~FL_MENU_VALUE);
				mode(index_custom, mode(index_custom) | FL_MENU_VALUE);
				break;
			default:
				break;
			}
		}
		else {
			// This level is not used - set Nothing
			mode(index_nothing, mode(index_nothing) | FL_MENU_VALUE);
			mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
			mode(index_states, mode(index_states) & ~FL_MENU_VALUE);
			mode(index_bands, mode(index_bands) & ~FL_MENU_VALUE);
			mode(index_modes, mode(index_entities) & ~FL_MENU_VALUE);
			mode(index_calls, mode(index_calls) & ~FL_MENU_VALUE);
			mode(index_custom, mode(index_custom) & ~FL_MENU_VALUE);

		}
	}
	// Set the Report->* menu items
	int index_clear = find_index("Re&port/&Clear");
	int index_all = find_index("Re&port/&All");
	int index_extracted = find_index("Re&port/E&xtracted");
	int index_selected = find_index("Re&port/&Selected record");
	switch (filter) {
	case RF_NONE:
		// Set Report->Clear only
		mode(index_clear, mode(index_clear) | FL_MENU_VALUE);
		mode(index_all, mode(index_all) & ~FL_MENU_VALUE);
		mode(index_extracted, mode(index_extracted) & ~FL_MENU_VALUE);
		mode(index_selected, mode(index_selected) & ~FL_MENU_VALUE);
		break;
	case RF_ALL:
		// Set Report->All only
		mode(index_clear, mode(index_clear) & ~FL_MENU_VALUE);
		mode(index_all, mode(index_all) | FL_MENU_VALUE);
		mode(index_extracted, mode(index_extracted) & ~FL_MENU_VALUE);
		mode(index_selected, mode(index_selected) & ~FL_MENU_VALUE);
		break;
	case RF_EXTRACTED:
		// Set Report->Extracted only
		mode(index_clear, mode(index_clear) & ~FL_MENU_VALUE);
		mode(index_all, mode(index_all) & ~FL_MENU_VALUE);
		mode(index_extracted, mode(index_extracted) | FL_MENU_VALUE);
		mode(index_selected, mode(index_selected) & ~FL_MENU_VALUE);
		break;
	case RF_SELECTED:
		// Set Report->Selected only
		mode(index_clear, mode(index_clear) & ~FL_MENU_VALUE);
		mode(index_all, mode(index_all) & ~FL_MENU_VALUE);
		mode(index_extracted, mode(index_extracted) & ~FL_MENU_VALUE);
		mode(index_selected, mode(index_selected) | FL_MENU_VALUE);
		break;
	default:
		break;
	}
}

// Update menu item Help->Status/Append File
void menu::append_file(bool append) {
	int index_append = find_index("&Help/&Status/&Append File");
	if (append) {
		mode(index_append, mode(index_append) | FL_MENU_VALUE);
	}
	else {
		mode(index_append, mode(index_append) & ~FL_MENU_VALUE);
	}
}

// Update menu items' active state depending on state of the book
void menu::update_items() {
	if (active_enabled_) {
		// Get conditions
		bool modified = book_->is_dirty();
		bool empty = book_->size() == 0;
		bool first = navigation_book_->selection() == 0;
		bool last = navigation_book_->selection() == navigation_book_->size() - 1;
		bool record_modified = book_->is_dirty_record(book_->get_record());
		bool new_record = book_->new_record();
		bool delete_enabled = book_->delete_enabled();
		bool save_enabled = book_->enable_save();
		bool web_enabled = book_ && book_->get_record() && book_->get_record()->item_exists("WEB");
		bool listening_wsjtx = wsjtx_handler_ && wsjtx_handler_->has_server();
		// Get all relevant menu item indices
		int index_save = find_index("&File/&Save");
		int index_saveas = find_index("&File/Save &As");
		int index_print = find_index("&File/&Print Log");
		int index_first = find_index("&Navigate/&First");
		int index_prev = find_index("&Navigate/&Previous");
		int index_next = find_index("&Navigate/Ne&xt");
		int index_last = find_index("&Navigate/&Last");
		int index_find = find_index("&Navigate/F&ind");
		int index_saver = find_index("&Log/&Save record");
		int index_cancel = find_index("&Log/&Cancel");
		int index_delete = find_index("&Log/&Delete record");
		int index_newr = find_index("&Log/&New record");
		int index_parser = find_index("&Log/&Parse record");
		int index_bulk = find_index("&Log/&Bulk changes");
		int index_ssave = find_index("&Log/Suspend Save");
		int index_extract = find_index("E&xtract");
		int index_wsjtx = find_index("&Import/&WSJT-X UDP");
		int index_rep = find_index("Re&port");
		int index_info = find_index("&Information");
		int index_web = find_index("&Information/QSO &Web-site");
		// Enable/Disable save 
		if (modified) {
			mode(index_save, mode(index_save) & ~FL_MENU_INACTIVE);
		}
		else {
			mode(index_save, mode(index_save) | FL_MENU_INACTIVE);
		}
		if (empty) {
			// Disallow log/record modification etc. if the file is empty
			mode(index_saveas, mode(index_saveas) | FL_MENU_INACTIVE);
			mode(index_print, mode(index_print) | FL_MENU_INACTIVE);
			mode(index_extract, mode(index_extract) | FL_MENU_INACTIVE);
			mode(index_rep, mode(index_rep) | FL_MENU_INACTIVE);
			mode(index_info, mode(index_info) | FL_MENU_INACTIVE);
			for (int i = index_parser; i <= index_bulk; i++) {
				mode(i, mode(i) | FL_MENU_INACTIVE);
			}
		}
		else {
			// Allow log/modification etc if the file has records
			mode(index_saveas, mode(index_saveas) & ~FL_MENU_INACTIVE);
			mode(index_print, mode(index_print) & ~FL_MENU_INACTIVE);
			mode(index_extract, mode(index_extract) & ~FL_MENU_INACTIVE);
			mode(index_rep, mode(index_rep) & ~FL_MENU_INACTIVE);
			mode(index_info, mode(index_info) & ~FL_MENU_INACTIVE);
			for (int i = index_parser; i <= index_bulk; i++) {
				mode(i, mode(i) & ~FL_MENU_INACTIVE);
			}
		}
		// Enable/disable navigation buttons depending on record selected
		if (first && !last) {
			// Disable Navigate->First and Navigate->Prev, enable others
			mode(index_first, mode(index_first) | FL_MENU_INACTIVE);
			mode(index_prev, mode(index_prev) | FL_MENU_INACTIVE);
			mode(index_next, mode(index_next) & ~FL_MENU_INACTIVE);
			mode(index_last, mode(index_last) & ~FL_MENU_INACTIVE);
			mode(index_find, mode(index_find) & ~FL_MENU_INACTIVE);
		}
		else if (last && !first) {
			// Disable NAvigate->Next and Navigate->Last and enable others
			mode(index_first, mode(index_first) & ~FL_MENU_INACTIVE);
			mode(index_prev, mode(index_prev) & ~FL_MENU_INACTIVE);
			mode(index_next, mode(index_next) | FL_MENU_INACTIVE);
			mode(index_last, mode(index_last) | FL_MENU_INACTIVE);
			mode(index_find, mode(index_find) & ~FL_MENU_INACTIVE);
		}
		else if (first && last) {
			// Only 1 record, disable all Navigate->* menu items
			mode(index_first, mode(index_first) | FL_MENU_INACTIVE);
			mode(index_prev, mode(index_prev) | FL_MENU_INACTIVE);
			mode(index_next, mode(index_next) | FL_MENU_INACTIVE);
			mode(index_last, mode(index_last) | FL_MENU_INACTIVE);
			mode(index_find, mode(index_find) | FL_MENU_INACTIVE);
		}
		else {
			// Enable all Navigate->* menu items
			mode(index_first, mode(index_first) & ~FL_MENU_INACTIVE);
			mode(index_prev, mode(index_prev) & ~FL_MENU_INACTIVE);
			mode(index_next, mode(index_next) & ~FL_MENU_INACTIVE);
			mode(index_last, mode(index_last) & ~FL_MENU_INACTIVE);
			mode(index_find, mode(index_find) & ~FL_MENU_INACTIVE);
		}
		if (record_modified || new_record) {
			// entering or modifyimng a record - enable save and cancel and disable new record 
			mode(index_saver, mode(index_saver) & ~FL_MENU_INACTIVE);
			mode(index_cancel, mode(index_cancel) & ~FL_MENU_INACTIVE);
			mode(index_newr, mode(index_newr) | FL_MENU_INACTIVE);
		}
		else {
			// Disable Save and Cancel and enable new record
			mode(index_saver, mode(index_saver) | FL_MENU_INACTIVE);
			mode(index_cancel, mode(index_cancel) | FL_MENU_INACTIVE);
			mode(index_newr, mode(index_newr) & ~FL_MENU_INACTIVE);
		}
		// Suspend save
		if (save_enabled) {
			mode(index_ssave, mode(index_ssave) & ~FL_MENU_VALUE);
		} else {
			mode(index_ssave, mode(index_ssave) | FL_MENU_VALUE);
		}
		// Delete enabled
		if (delete_enabled) {
			mode(index_delete, mode(index_delete) & ~FL_MENU_INACTIVE);
		}
		else {
			mode(index_delete, mode(index_delete) | FL_MENU_INACTIVE);
		}
		// Info->QSO WEb-site
		if (web_enabled) {
			mode(index_web, mode(index_web) & ~FL_MENU_INACTIVE);
		}
		else {
			mode(index_web, mode(index_web) | FL_MENU_INACTIVE);
		}
		// Update Import->WSJT-X UDP
		if (listening_wsjtx) {
			mode(index_wsjtx, mode(index_wsjtx) | FL_MENU_INACTIVE);
		}
		else {
			mode(index_wsjtx, mode(index_wsjtx) & ~FL_MENU_INACTIVE);
		}
	}
	redraw();
	update_windows_items();
	update_qsl_items();
	// Update toolbar to reflect active state of menu items
	toolbar_->update_items();
}

// Add windows user data
void menu::add_windows_items() {
	// Get indices to &Windows
	int index = find_index("&Windows/&Hide All");
	insert(index, "&Windows/&Main", 0, cb_mi_windows, main_window_, FL_MENU_TOGGLE);
	insert(index, "&Windows/Das&hboard", 0, cb_mi_windows, qso_manager_, FL_MENU_TOGGLE);
	// insert(index, "&Windows/S&tatus Viewer", 0, cb_mi_windows, status_->file_viewer(), FL_MENU_TOGGLE);
	insert(index, "&Windows/&International Chars", 0, cb_mi_windows, intl_dialog_, FL_MENU_TOGGLE);
}

// Update Windows sub-menu
void menu::update_windows_items() {
	// Get indices to menu items
	int index_main = find_index("&Windows/&Main");
	int index_oper = find_index("&Windows/Das&hboard");
	//int index_band = find_index("&Windows/&Band View");
	int index_intl = find_index("&Windows/&International Chars");

	if (main_window_ && index_main != -1) {
		if (main_window_->visible()) {
			mode(index_main, mode(index_main) | FL_MENU_VALUE);
		}
		else {
			mode(index_main, mode(index_main) & ~FL_MENU_VALUE);
		}
	}

	if (qso_manager_ && index_oper != -1) {
		if (qso_manager_->visible()) {
			mode(index_oper, mode(index_oper) | FL_MENU_VALUE);
		}
		else {
			mode(index_oper, mode(index_oper) & ~FL_MENU_VALUE);
		}
	}

	if (intl_dialog_ && index_intl != -1) {
		if (intl_dialog_->visible()) {
			mode(index_intl, mode(index_intl) | FL_MENU_VALUE);
		}
		else {
			mode(index_intl, mode(index_intl) & ~FL_MENU_VALUE);
		}
	}

	redraw();
}

// Get the browser form config or if not ask user
string menu::get_browser() {
	// Get browser from config
	char* temp;
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences datapath_settings(settings, "Datapath");
	datapath_settings.get("Web Browser", temp, "");
	string browser = temp;
	free(temp);
	if (!browser.length()) {
		// User hasn't defined browser yet - open dialog to get it
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser->title("Please select your favoured web browser");
		chooser->filter("Applications\t*.exe");
		if (chooser->show() != 0) {
			status_->misc_status(ST_WARNING, "INFO: No browser selected, abandoning");
			return "";
		}
		else {
			browser = chooser->filename();
		}
		delete chooser;
	}
	datapath_settings.set("Web Browser", browser.c_str());
	return browser;
}

// Do not allow a second upload to take place until the first has completed
void menu::update_qsl_items() {
	int index_eqsl = find_index("E&xtract/e&QSL");
	int index_lotw = find_index("E&xtract/&LotW");
	int index_card = find_index("E&xtract/Car&d");
	int index_clog = find_index("E&xtract/Club&Log");
	int index_upload = find_index("E&xtract/&Upload");
	int index_print = find_index("E&xtract/&Print Cards");
	int index_dn_eqsl = find_index("&Import/Download e&QSL");
	int index_dn_lotw = find_index("&Import/Download &LotW");

	bool inactive = qso_manager_ ? qso_manager_->data()->inactive() : false;

	if (inactive) {
		mode(index_dn_eqsl, mode(index_dn_eqsl) & ~FL_MENU_INACTIVE);
		mode(index_dn_lotw, mode(index_dn_lotw) & ~FL_MENU_INACTIVE);
		mode(index_card, mode(index_card) & ~FL_MENU_INACTIVE);
		mode(index_print, mode(index_print) & ~FL_MENU_INACTIVE);

	} else {
		mode(index_dn_eqsl, mode(index_dn_eqsl) | FL_MENU_INACTIVE);
		mode(index_dn_lotw, mode(index_dn_lotw) | FL_MENU_INACTIVE);
		mode(index_card, mode(index_card) | FL_MENU_INACTIVE);
		mode(index_print, mode(index_print) | FL_MENU_INACTIVE);
	}

	if (extract_records_->upload_in_progress() || !inactive) {
		mode(index_eqsl, mode(index_eqsl) | FL_MENU_INACTIVE);
		mode(index_lotw, mode(index_lotw) | FL_MENU_INACTIVE);
		mode(index_clog, mode(index_clog) | FL_MENU_INACTIVE);
	}
	else {
		mode(index_eqsl, mode(index_eqsl) & ~FL_MENU_INACTIVE);
		mode(index_lotw, mode(index_lotw) & ~FL_MENU_INACTIVE);
		mode(index_clog, mode(index_clog) & ~FL_MENU_INACTIVE);
	}

	if (extract_records_->upload_in_progress() && inactive) {
		mode(index_upload, mode(index_upload) & ~FL_MENU_INACTIVE);
	} else {
		mode(index_upload, mode(index_upload) | FL_MENU_INACTIVE);
	}
}

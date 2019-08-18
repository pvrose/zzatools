#include "menu.h"
#include "book.h"
#include "record.h"
#include "pfx_data.h"
#include "spec_data.h"
#include "settings.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "rig_if.h"
#include "utils.h"
#include "status.h"


#include "printer.h"
#include "extract_data.h"
#include "search_dialog.h"
#include "change_dialog.h"
#include "use_dialog.h"
#include "about_dialog.h"
#include "pfx_tree.h"
#include "spec_tree.h"
#include "report_tree.h"
#include "url_handler.h"
#include "callback.h"
#include "page_dialog.h"
#include "intl_dialog.h"
#include "toolbar.h"
#include "scratchpad.h"
#include "calendar.h"

#include <sstream>
#include <list>
#include <string>

#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Tooltip.H>

namespace zzalog {
	// The default menu - set of menu items
	Fl_Menu_Item menu_items[] = {
		{ "&File", 0, 0, 0, FL_SUBMENU },
	{ "&New", 0, menu::cb_mi_file_new, 0 },
	{ "&Open", 0, menu::cb_mi_file_open, 0 },
	{ "Rea&d", 0, menu::cb_mi_file_open, (void*)-1L },
	{ "&Save", 0, menu::cb_mi_file_save, (void*)OT_MAIN },
	{ "Save &As", 0, menu::cb_mi_file_saveas, (void*)OT_MAIN },
	{ "&Close", 0, menu::cb_mi_file_close, 0 },
	{ "Au&to Save", 0, menu::cb_mi_file_auto, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
	{ "&Print", 0, menu::cb_mi_file_print, 0, FL_MENU_DIVIDER },
	{ "&Recent", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
	// Extra menu items are dynamically inserted here 
	{ 0 },
	{ "&Backup", 0, menu::cb_mi_file_backup, (void*)false },
	{ "Retrie&ve", 0, menu::cb_mi_file_backup, (void*)true },
	{ 0 },
	{ "&Settings", 0, 0, 0, FL_SUBMENU },
	{ "&Rig", 0, menu::cb_mi_settings, (void*)DLG_RIG },
	{ "Fi&les", 0, menu::cb_mi_settings, (void*)DLG_FILES },
	{ "&Web", 0, menu::cb_mi_settings, (void*)DLG_WEB },
	{ "&Station", 0, menu::cb_mi_settings, (void*)DLG_STATION },
	{ "&Fields", 0, menu::cb_mi_settings, (void*)DLG_COLUMN },
	{ "&QSL Design", 0, menu::cb_mi_settings, (void*)DLG_QSL },
	{ "All", 0, menu::cb_mi_settings, (void*)DLG_ALL },
	{ 0 },
	{ "&Navigate", 0, 0, 0, FL_SUBMENU },
	{ "&First", 0, menu::cb_mi_navigate, (void*)(NV_FIRST) },
	{ "&Previous", 0, menu::cb_mi_navigate, (void*)(NV_PREV) },
	{ "Ne&xt", 0, menu::cb_mi_navigate, (void*)(NV_NEXT) },
	{ "&Last", 0, menu::cb_mi_navigate, (void*)(NV_LAST) },
	{ "&Date", 0, menu::cb_mi_nav_date	, nullptr },
	{ "F&ind", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
	{ "&New", 0, menu::cb_mi_nav_find, (void*)true },
	{ "Ne&xt", 0, menu::cb_mi_nav_find, (void*)false },
	{ 0 },
	{ 0 },
	{ "&Log", 0, 0, 0, FL_SUBMENU },
	{ "&Mode", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
	{ "O&ff-air", 0, menu::cb_mi_log_mode, (void*)(LM_OFF_AIR) , FL_MENU_RADIO | FL_MENU_VALUE },
	{ "&Radio Connected", 0, menu::cb_mi_log_mode, (void*)(LM_RADIO_CONN), FL_MENU_RADIO },
	{ "Radio &Disconnected", 0, menu::cb_mi_log_mode, (void*)(LM_RADIO_DISC), FL_MENU_RADIO },
	{ "&Import", 0, menu::cb_mi_log_mode, (void*)(LM_IMPORTED), FL_MENU_RADIO },
	{ 0 },
	{ "&Use View", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER },
	{ "Main &Log", 0, menu::cb_mi_log_view, (void*)(OT_MAIN), FL_MENU_RADIO | FL_MENU_VALUE },
	{ "&Report View", 0, menu::cb_mi_log_view, (void*)(OT_REPORT), FL_MENU_RADIO  },
	{ "&Scratchpad", 0, menu::cb_mi_log_view, (void*)(OT_SCRATCH), FL_MENU_RADIO  },
	{ 0 },
	{ "&New record", 0, menu::cb_mi_log_new, nullptr },
	{ "&Save record", 0, menu::cb_mi_log_save, nullptr },
	{ "&Cancel", 0, menu::cb_mi_log_del, (void*)false },
	{ "&Delete record", 0, menu::cb_mi_log_del, (void*)true, FL_MENU_DIVIDER },
	{ "&Parse record", 0, menu::cb_mi_parse_qso, 0 },
	{ "&Validate record", 0, menu::cb_mi_valid8_qso, 0, FL_MENU_DIVIDER },
	{ "Pa&rse log", 0, menu::cb_mi_parse_log, 0 },
	{ "Val&idate log", 0, menu::cb_mi_valid8_log, 0, FL_MENU_DIVIDER },
	{ "&Bulk changes", 0, menu::cb_mi_log_bulk, 0 },
	{ "Chec&k Duplicates", 0, menu::cb_mi_log_dupes, 0 },
	{ "Edit &Header", 0, menu::cb_mi_log_edith, 0 },
	{ "Scratc&hpad", 0, menu::cb_mi_log_spad, 0, FL_MENU_TOGGLE },
	{ 0 },
	{ "&Operating", 0, 0, 0, FL_SUBMENU },
	{ "C&hange", 0, 0, 0, FL_SUBMENU },
	{ "Ri&g", 0, menu::cb_mi_oper_change, (void*)UD_RIG },
	{ "&Aerial", 0, menu::cb_mi_oper_change, (void*)UD_AERIAL },
	{ "&QTH", 0, menu::cb_mi_oper_change, (void*)UD_QTH },
	{ "&Prop mode", 0, menu::cb_mi_oper_change, (void*)UD_PROP },
	{ 0 },
	{ "Se&t", 0, 0, 0, FL_SUBMENU },
	{ "Ri&g", 0, menu::cb_mi_oper_set, (void*)UD_RIG },
	{ "&Aerial", 0, menu::cb_mi_oper_set, (void*)UD_AERIAL },
	{ "&QTH", 0, menu::cb_mi_oper_set, (void*)UD_QTH },
	{ "&Prop mode", 0, menu::cb_mi_oper_set, (void*)UD_PROP },
	{ 0 },
	{ 0 },
	{ "E&xtract", 0, 0, 0, FL_SUBMENU },
	{ "Clea&r", 0, menu::cb_mi_ext_clr, 0 },
	{ "&Criteria", 0, menu::cb_mi_ext_crit, 0 },
	{ "Re&do", 0, menu::cb_mi_ext_redo, 0 },
	{ "&Display", 0, menu::cb_mi_ext_disp, 0, FL_MENU_DIVIDER },
	{ "e&QSL", 0, menu::cb_mi_ext_qsl, (void*)extract_data::EQSL },
	{ "&LotW", 0, menu::cb_mi_ext_qsl, (void*)extract_data::LOTW },
	{ "Car&d", 0, menu::cb_mi_ext_qsl, (void*)extract_data::CARD },
	{ "Ca&brillo", 0, menu::cb_mi_ext_qsl, (void*)extract_data::CABRILLO, FL_MENU_DIVIDER },
	{ "&Save", 0, menu::cb_mi_file_saveas, (void*)OT_EXTRACT },
	{ "&Upload", 0, menu::cb_mi_ext_upload, 0 },
	{ "&Print", 0, menu::cb_mi_ext_print, 0 },
	{ 0 },
	{ "&Import", 0, 0, 0, FL_SUBMENU },
	{ "&File", 0, menu::cb_mi_imp_file, nullptr },
	{ "Download e&QSL", 0, menu::cb_mi_download, (void*)(long)import_data::EQSL_UPDATE },
	{ "Download &LotW", 0, menu::cb_mi_download, (void*)(long)import_data::LOTW_UPDATE, FL_MENU_DIVIDER },
	{ "&Merge", 0, menu::cb_mi_imp_merge, nullptr, FL_MENU_DIVIDER },
	{ "&Cancel", 0, menu::cb_mi_imp_cancel, nullptr, FL_MENU_DIVIDER },
	{ "&Enable Auto-Update", 0, menu::cb_mi_imp_enauto, nullptr, FL_MENU_TOGGLE },
	{ 0 },
	{ "&Reference", 0, 0, 0, FL_SUBMENU },
	{ "&Prefix", 0, 0, 0, FL_SUBMENU },
	{ "&Clear", 0, menu::cb_mi_ref_filter, (void*)RF_NONE, FL_MENU_RADIO },
	{ "&All", 0, menu::cb_mi_ref_filter, (void*)RF_ALL, FL_MENU_RADIO | FL_MENU_VALUE },
	{ "E&xtracted", 0, menu::cb_mi_ref_filter, (void*)RF_EXTRACTED, FL_MENU_RADIO },
	{ "&Selected record", 0, menu::cb_mi_ref_filter, (void*)RF_SELECTED, FL_MENU_RADIO | FL_MENU_DIVIDER },
	{ "&Code", 0, menu::cb_mi_ref_items, (void*)RI_CODE,  FL_MENU_RADIO | FL_MENU_VALUE },
	{ "&Prefix", 0, menu::cb_mi_ref_items, (void*)RI_NICK, FL_MENU_RADIO },
	{ "&Name", 0, menu::cb_mi_ref_items, (void*)RI_NAME, FL_MENU_RADIO | FL_MENU_DIVIDER },
	{ "Add &details", 0, menu::cb_mi_ref_details, nullptr, FL_MENU_TOGGLE },
	// TODO: implement a real callback for this menu item
	{ "&Reload data", 0, menu::cb_mi_ref_reload, 0},
	{ 0 },
	{ 0 },
	{ "Re&port", 0, 0, 0, FL_SUBMENU },
	{ "&Clear", 0, menu::cb_mi_rep_filter, (void*)RF_NONE, FL_MENU_RADIO },
	{ "&All", 0, menu::cb_mi_rep_filter, (void*)RF_ALL, FL_MENU_RADIO | FL_MENU_VALUE },
	{ "E&xtracted", 0, menu::cb_mi_rep_filter, (void*)RF_EXTRACTED, FL_MENU_RADIO },
	{ "&Selected record", 0, menu::cb_mi_rep_filter, (void*)RF_SELECTED, FL_MENU_RADIO | FL_MENU_DIVIDER },
	{ "Level &1", 0, 0, 0, FL_SUBMENU },
	{ "&Entities", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_DXCC), FL_MENU_RADIO },
	{ "Entities/&States", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_PAS), FL_MENU_RADIO | FL_MENU_VALUE },
	{ "&Bands", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_BAND), FL_MENU_RADIO },
	{ "&Modes", 0, menu::cb_mi_rep_level, (void*)((1 << 8) + RC_MODE), FL_MENU_RADIO },
	{ 0 },
	{ "Level &2", 0, 0, 0, FL_SUBMENU },
	{ "&Entities", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_DXCC), FL_MENU_RADIO },
	{ "Entities/&States", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_PAS), FL_MENU_RADIO },
	{ "&Bands", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_BAND), FL_MENU_RADIO },
	{ "&Modes", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_MODE), FL_MENU_RADIO },
	{ "&Nothing", 0, menu::cb_mi_rep_level, (void*)((2 << 8) + RC_EMPTY), FL_MENU_RADIO | FL_MENU_VALUE },
	{ 0 },
	{ "Level &3", 0, 0, 0, FL_SUBMENU },
	{ "&Entities", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_DXCC), FL_MENU_RADIO },
	{ "Entities/&States", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_PAS), FL_MENU_RADIO },
	{ "&Bands", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_BAND), FL_MENU_RADIO },
	{ "&Modes", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_MODE), FL_MENU_RADIO },
	{ "&Nothing", 0, menu::cb_mi_rep_level, (void*)((3 << 8) + RC_EMPTY), FL_MENU_RADIO | FL_MENU_VALUE },
	{ 0 },
	{ 0 },
	{ "&Information", 0, 0, 0, FL_SUBMENU },
	{ "&QRZ.com", 0, menu::cb_mi_info_qrz },
	{ "Google &Maps", 0, menu::cb_mi_info_map },
	{ "QSO &Web-site", 0, menu::cb_mi_info_web },
	{ 0 },
	{ "&Help", 0, 0, 0, FL_SUBMENU },
	{ "&About", 0, menu::cb_mi_help_abt },
	{ "&Status", 0, 0, 0, FL_SUBMENU },
	{ "View &Status", 0, menu::cb_mi_help_view, nullptr, FL_MENU_DIVIDER },
	{ "&Note", 0, menu::cb_mi_help_level, (void*)ST_NOTE, FL_MENU_RADIO | FL_MENU_VALUE },
	{ "&Done", 0, menu::cb_mi_help_level, (void*)ST_OK, FL_MENU_RADIO },
	{ "&Warning", 0, menu::cb_mi_help_level, (void*)ST_WARNING, FL_MENU_RADIO },
	{ "&Error", 0, menu::cb_mi_help_level, (void*)ST_ERROR, FL_MENU_RADIO },
	{ "Se&vere", 0, menu::cb_mi_help_level, (void*)ST_SEVERE, FL_MENU_RADIO },
	{ "&Fatal", 0, menu::cb_mi_help_level, (void*)ST_FATAL, FL_MENU_RADIO },
	{ 0 },
	{ "&Intl", 0, menu::cb_mi_help_intl, nullptr, FL_MENU_TOGGLE },
	{ 0 },
	{ 0 }
	};
}

using namespace zzalog;

extern book* book_;
extern import_data* import_data_;
extern extract_data* extract_records_;
extern book* navigation_book_;
extern pfx_data* pfx_data_;
extern spec_data* spec_data_;
extern rig_if *rig_if_;
extern status* status_;
extern tabbed_forms* tabbed_view_;
extern url_handler* url_handler_;
extern Fl_Single_Window* main_window_;
extern Fl_Preferences* settings_;
extern bool read_only_;
extern list<string> recent_files_;
extern intl_dialog* intl_dialog_;
extern toolbar* toolbar_;
extern scratchpad* scratchpad_;

extern void add_rig_if();
extern void main_window_label(string text);
extern void backup_file(bool force, bool retrieve = false);
extern void set_recent_file(string filename);
extern void add_scratchpad();
extern void remove_sub_window(Fl_Window* win);
extern void add_sub_window(Fl_Window* win);


// Constructor
menu::menu(int X, int Y, int W, int H, const char* label) :
	Fl_Menu_Bar(X, Y, W, H, label)
{
	// Add the menu
	Fl_Menu_Bar::menu(menu_items);
	// Add the recent files list to it
	add_recent_files();
	// default text size - just larger than default font size
	textsize(FONT_SIZE + 1);
	// This will get set to either ON_AIR or IMPORT if a rig gets connected depending on whether the rig is in a digital mode.
	logging(LM_OFF_AIR);
	criteria_ = nullptr;
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.get("Edit view", (int&)editting_view_, OT_REPORT);
	show();
}

// Destructor
menu::~menu()
{
	clear();
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Edit view", editting_view_);
}

// File->New
// v is not used
void menu::cb_mi_file_new(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// Gracefully stop any import in progress - restart with ON_AIR logging - if no rig will drop to OFF_AIR
	if (that->logging() == LM_IMPORTED) {
		import_data_->stop_update(LM_RADIO_CONN, false);
	}
	while (!import_data_->update_complete()) Fl::wait();
	if (book_->modified_record() | book_->new_record()) {
		switch (fl_choice("You are currently modifying a record? Save or Quit?", "Save?", "Quit?", nullptr)) {
		case 0:
			book_->save_record();
			break;
		case 1:
			book_->delete_record(false);
			break;
		}
	}
	if (book_->modified()) {
		if (fl_choice("Book has been modified, do you want to save?", "Yes", "No", nullptr) == 0) {
			book_->store_data();
		}
	}
	// get book to delete contents by loading no data
	book_->load_data("");
	// Allow writes
	read_only_ = false;

	// Clear any extracted data as there are no records to reference
	extract_records_->clear_criteria();
	set_recent_file("");
	navigation_book_ = book_;
	book_->navigate(NV_FIRST);
}

// File->Open
// v is a long. -1 = open browser and open file read-only; 0 = open browser and open file; 1-4 = open specific recent file
void menu::cb_mi_file_open(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// Stop any import occurring
	import_data_->stop_update(LM_OFF_AIR, false);
	while (!import_data_->update_complete()) Fl::wait();

	if (book_->modified()) {
		if (fl_choice("Book has been modified, do you want to save?", "Yes", "No", nullptr) == 0) {
			// Do File->Save first
			cb_mi_file_save(w, v);
		}
	}
	
	// Set to a recent file number (1 to 4) or 0 for file_chooser
	char file_id = (char)(long)v;
	string filename = "";
	// Set read_only flag
	if (file_id < 0) {
		read_only_ = true;
	}
	else {
		read_only_ = false;
	}
	// Open chooser and open read/write (0) or read only (-1)
	if (file_id <= 0) {
		// Open file chooser to get file to load
		Fl_Preferences datapath_settings(settings_, "Datapath");
		char* directory;
		datapath_settings.get("Log Directory", directory, "");
		Fl_File_Chooser* chooser = new Fl_File_Chooser(directory, "ADI Files(*.adi)\tADX Files (*.adx)", Fl_File_Chooser::SINGLE, "Select file name to load");
		free(directory);
		chooser->callback(cb_chooser, &filename);
		chooser->textfont(FONT);
		chooser->textsize(FONT_SIZE);
		chooser->show();
		// Wait while the dialog is active (visible)
		while (chooser->visible()) Fl::wait();
		if (!chooser->count()) {
			filename = "";
		}
		delete chooser;
	}
	else {
		// Get the recent file with supplied ID
		Fl_Preferences recent_settings(settings_, "Recent Files");
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
			// Set off-air logging, clear any search results, select last entry
			that->logging(LM_OFF_AIR);
			navigation_book_ = book_;
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
	import_data_->stop_update(LM_OFF_AIR, false);
	while (!import_data_->update_complete()) Fl::wait();
	// Get the main book to store itself as long as it wasn't opened read-only
	if (read_only_) {
		if (fl_choice("File was opened read-only - save as new file?", "OK", "No", nullptr) == 0) {
			cb_mi_file_saveas(w, v);
		}
	}
	else {
		// If we have a file loaded then save it
		if (book_->filename().length()) {
			book_->store_data();
			read_only_ = false;
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
	Fl_File_Chooser* chooser = new Fl_File_Chooser(filename.c_str(), "ADI Files(*.adi)\tADX Files (*.adx)\tTSV files (*.{tsv,tab})", Fl_File_Chooser::CREATE, "Select file name to save");
	chooser->callback(cb_chooser, &filename);
	chooser->textfont(FONT);
	chooser->textsize(FONT_SIZE);
	chooser->show();
	// Wait while the dialog is active (visible)
	while (chooser->visible()) Fl::wait();
	// Trial and error indicated that if Cancel is clicked, no files are selected.
	if (chooser->count()) {
		// Got a filename
		ifstream* check = new ifstream(filename.c_str());
		if (check->fail() || fl_choice("File exists, do you want to over-write", "OK", "No", nullptr) == 0) {
			// Stop any current import
			import_data_->stop_update(LM_OFF_AIR, false);
			while (!import_data_->update_complete()) Fl::wait();
			// Get the book to save
			book* b = nullptr;
			object_t type = (object_t)(long)v;
			switch (type) {
			case OT_MAIN:
				b = book_;
				break;
			case OT_EXTRACT:
				b = extract_records_;
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
		read_only_ = false;
	}
	delete chooser;
}

// File->Close
// v is not used
void menu::cb_mi_file_close(Fl_Widget* w, void* v) {
	// Gracefully stop import
	import_data_->stop_update(LM_OFF_AIR, false);
	while (!import_data_->update_complete()) Fl::wait();
	main_window_->do_callback();
}

// File ->Auto Save
// v is not used
void menu::cb_mi_file_auto(Fl_Widget* w, void* v) {
	// Get the value of the checked menu item
	Fl_Menu_* menu = (Fl_Menu_*)w;
	const Fl_Menu_Item* item = menu->mvalue();
	bool value = item->value();
	book_->enable_save(value);
}

// File->Print
// v is not used
void menu::cb_mi_file_print(Fl_Widget* w, void* v) {
	// Print the current book
	object_t type = (object_t)(long)v;
	if (type == OT_MAIN) {
		type = navigation_book_->book_type();
	}
	printer* ptr = new printer(type);
	delete ptr;
}

// File->Backup
// v is bool. false = backup; true = retrieve
void menu::cb_mi_file_backup(Fl_Widget*, void* v) {
	bool retrieve = (bool)(long)v;
	// Back up
	backup_file(true, retrieve);
}

// Config->any
// v is enum cfg_dialog_t indicating which settings page to open at
void menu::cb_mi_settings(Fl_Widget* w, void* v) {
	// v provides that id of the page of the settings dialogs to open with
	cfg_dialog_t active = (cfg_dialog_t)(long)v;
	// Open the config and wait for it to close
	settings* config = new settings(WCONFIG, HCONFIG, "Configuration", active);
}

// Navigate->First,Previous,Next,Last callbacks
// v is enum navigate_t indicating which record to go to
void menu::cb_mi_navigate(Fl_Widget* w, void* v) {
	// v supplies that navigation target
	navigate_t target = (navigate_t)(long)v;
	// Navigate to the specified point in the specified book
	if (navigation_book_ != nullptr) {
		navigation_book_->navigate(target);
	}
}

// Navigate->Date: OPens a cendar window
// v is ignored
void menu::cb_mi_nav_date(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// Populate calendar with today's date
	string date = now(false, "%Y%m%d");
	cal_cb_data_t cb_data(&date, nullptr);
	calendar* cal = new calendar(Fl::event_x_root(), Fl::event_y_root());
	cal->value(date.c_str());
	cal->callback(calendar::cb_cal_close, &cb_data);
	cal->show();
	while (cal->active()) Fl::wait();
	// Now fiind the selected date
	if (navigation_book_ != nullptr) {
		navigation_book_->go_date(date);
	}
}
// Navigate->Find
// v is a bool. false = find next; true = new search
void menu::cb_mi_nav_find(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// v supplies whether to change search criteria
	bool do_extract = (bool)(long)v;
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
		parse_result_t parse_result = pfx_data_->parse(record);
		// update band
		bool changed = record->update_band(true);
		if (changed || parse_result == PR_CHANGED) {
			// If modified - tell views to update
			navigation_book_->selection(-1, HT_CHANGED);
			book_->modified(true);
		}
	}
}

// Log ->Parse Log - parse all records in selected book
// v is not used
void menu::cb_mi_parse_log(Fl_Widget* w, void* v) {
	// display hourglass while we validate the log
	fl_cursor(FL_CURSOR_WAIT);
	// Get parse mode
	int num_items = navigation_book_->size();
	if (true) {	
		// If command parsing allowed
		bool abandon = false;
		int record_num = -1;
		// Initialise progress
		status_->misc_status(ST_NOTE, "PARSE LOG: Started");
		status_->progress(num_items, navigation_book_->book_type(), "records");
		// For all records in selected book
		for (int i = 0; i < num_items && !abandon; i++) {
			record* record = navigation_book_->get_record(i, false);
			// Parse each record in turn
			parse_result_t parse_result = pfx_data_->parse(record);
			bool changed = record->update_band(true);
			record_num = navigation_book_->record_number(i);
			// Update progress
			status_->progress(i);
			if (changed || parse_result == PR_CHANGED) {
				book_->modified(true);
				// The parsing may have modified date and time
				book_->correct_record_position(navigation_book_->record_number(i));
			}
			else if (parse_result == PR_ABANDONED) {
				// User has the opportunity to abandon this if too many records have issues
				abandon = true;
				status_->misc_status(ST_WARNING, "PARSE LOG: Abandoned");
				status_->progress(0);
			}
		}
		// update views with last record parsed selected
		status_->misc_status(ST_OK, "PARSE LOG: Done!");
		navigation_book_->selection(record_num, HT_CHANGED);
	}
#ifndef _DEBUG
	// Save document
	if (book_->modified()) {
		book_->store_data();
	}
#endif
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Log->Validate QSO - validate selected record
// v is not used
void menu::cb_mi_valid8_qso(Fl_Widget* w, void* v) {
	record* record = navigation_book_->get_record();
	record_num_t record_num = navigation_book_->record_number(navigation_book_->selection());
	// If command validation is enabled
	bool changed = spec_data_->validate(record, record_num);
	if (changed) {
		// Update views if record has changed
		navigation_book_->selection(record_num, HT_MINOR_CHANGE);
		book_->modified(true);
	}
#ifndef _DEBUG
	// Save document
	if (book_->modified()) {
		book_->store_data();
	}
#endif
}
 
// Log->Validate Log - validate all records in selected log
// v is not used
void menu::cb_mi_valid8_log(Fl_Widget* w, void* v) {
	fl_cursor(FL_CURSOR_WAIT);
		// Command validation is enabled
	int record_num = -1;
	int num_items = navigation_book_->size();
	bool changed = false;
	// Initialse progress
	status_->misc_status(ST_NOTE, "VALIDATE LOG: Started");
	status_->progress(num_items, navigation_book_->book_type(), "records");
	spec_data_->reset_continue();
	// For each record in selected book unless user has indicated to quit
	for (int i = 0; i < num_items && spec_data_->do_continue(); i++) {
		record_num = navigation_book_->record_number(i);
		record* record = navigation_book_->get_record(i, false);
		// validate it
		if (spec_data_->validate(record, record_num)) {
			changed = true;
		}
		status_->progress(i);
	}
	status_->misc_status(ST_OK, "VALIDATE LOG: Done!");
	if (changed) {
		navigation_book_->selection(record_num, HT_MINOR_CHANGE);
	}
#ifndef _DEBUG
	// Save document
	if (book_->modified()) {
		book_->store_data();
	}
#endif
	fl_cursor(FL_CURSOR_DEFAULT);
}

// Log->Mode->Off-air,On-air,Import - change logging mode
// v is enum logging_mode_t to select logging mode
void menu::cb_mi_log_mode(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// v has new logging mode
	logging_mode_t next_mode = (logging_mode_t)(long)v;

	// Stop current logging mode
	switch (that->logging_mode_) {
	case LM_OFF_AIR:
	case LM_RADIO_DISC:
		// Currently entering a QSO - ask Save or Quit
		if (book_->modified_record() || book_->new_record()) {
			switch (fl_choice("You are currently modifying a record? Save or Quit?", "Save?", "Quit?", nullptr)) {
			case 0:
				book_->save_record();
				break;
			case 1:
				book_->delete_record(false);
				break;
			}
		}
		break;
	case LM_RADIO_CONN:
		if (rig_if_) {
			status_->misc_status(ST_NOTE, "RIG: Closing rig connection by user");
			// Close rig
			rig_if_->close();
			delete rig_if_;
			rig_if_ = nullptr;
		}
		break;
	case LM_IMPORTED:
		// Stop importing - and wait for it to finish
		import_data_->stop_update(next_mode, false);
		while (!import_data_->update_complete()) Fl::wait();
		break;
	}

	// Start new logging mode
	that->logging(next_mode);
	switch (that->logging_mode_) {
	case LM_OFF_AIR:
		// Let user have mode and stop auto-save
		status_->misc_status(ST_NOTE, "RIG: Off-air logging initiated by user");
		book_->enable_save(false);
		// Update enabled menu items
		that->update_items();
		break;
	case LM_RADIO_DISC:
		// Let user have mode and stop auto-save
		status_->misc_status(ST_NOTE, "RIG: Real-time, radio disconnected logging");
		book_->enable_save(false);
		// Update enabled menu items
		that->update_items();
		break;

	case LM_RADIO_CONN:
		// attempt to open rig
		add_rig_if();
		break;
	case LM_IMPORTED:
		// Reset navigation mode
		navigation_book_ = book_;
		// start import_process
		import_data_->start_auto_update();
		break;
	}
}

// Log->Use View->
// v is which view
void menu::cb_mi_log_view(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	that->editting_view_ = (object_t)(long)v;
}

// Log->New - start a new record
// v is not used
void menu::cb_mi_log_new(Fl_Widget* w, void* v) {
	menu* that = ancestor_view<menu>(w);
	// Force main log book
	navigation_book_ = book_;
	// Create a new record - on or off-air
	book_->new_record(that->logging());
	// Open log view
	switch (that->editting_view_) {
	case OT_MAIN:
	case OT_RECORD:
		tabbed_view_->activate_pane(that->editting_view_, true);
		break;
	case OT_SCRATCH:
		if (!scratchpad_) {
			add_scratchpad();
		}
		scratchpad_->show();
		break;
	}
}


// Log->Save - save current record
// v is not used
void menu::cb_mi_log_save(Fl_Widget* w, void* v) {
	navigation_book_->save_record();
}

// Log->Delete/Cancel - v = true/false defines which
// v is bool. false = cancel entry; true = delete record
void menu::cb_mi_log_del(Fl_Widget* w, void* v) {
	// delete_record(true) - deliberately deleting a record
	// delete_record(false) - only deletes if entering a new record (i.e. cancel)
	navigation_book_->delete_record((bool)(long)v);
}

// Log->Bulk Change - Do the same mod on all records
// v is not used
void menu::cb_mi_log_bulk(Fl_Widget* w, void* v) {
	// Open dialog to allow user to define change
	change_dialog* dialog = new change_dialog("Bulk change fields");
	if (dialog->display() == BN_OK) {
		change_action_t action = CHANGE_FIELD;
		string old_field_name = "";
		string new_field_name = "";
		string new_text = "";
		fl_cursor(FL_CURSOR_WAIT);
		// get action
		dialog->get_data(action, old_field_name, new_field_name, new_text);
		status_->progress(navigation_book_->size(), navigation_book_->book_type(), "records");
		status_->misc_status(ST_NOTE, "LOG: Bulk change started");
		// For each record in selected book
		for (record_num_t i = 0; i < navigation_book_->size(); i++) {
			record* record = navigation_book_->get_record(i, false);

			switch (action) {
			case RENAME_FIELD:
				// Change the field_name from old to new
				record->change_field_name(old_field_name, new_field_name);
				book_->modified(true);
				break;
			case DELETE_FIELD:
				// Delete the field with this name
				record->item(old_field_name, string(""));
				book_->modified(true);
				break;
			case ADD_FIELD:
				// Add a field with this name and value
				if (record->item(old_field_name) == "") {
					record->item(old_field_name, new_text);
					book_->modified(true);
				}
				break;
			case CHANGE_FIELD:
				// Change the field to this value - adding the field if necessary
				record->item(old_field_name, new_text);
				book_->modified(true);
				break;
			}
			status_->progress(i);
		}
		// Get all views to update
		book_->selection(book_->selection(), HT_ALL);
		status_->misc_status(ST_OK, "LOG: Bulk change done!");
		fl_cursor(FL_CURSOR_DEFAULT);
	}
	Fl::delete_widget(dialog);
}

// Log->Check Duplicates - call books check duplicates
// v is not used
void menu::cb_mi_log_dupes(Fl_Widget* w, void* v) {
	navigation_book_->check_dupes(true);
}

// Log->Edit Header - open editor on header comment
// v is not used
void menu::cb_mi_log_edith(Fl_Widget* w, void* v) {
	navigation_book_->edit_header();
}

// Log->Scratchpad
// v is unsued
void menu::cb_mi_log_spad(Fl_Widget* w, void* v) {
	// Get the value of the checked menu item
	Fl_Menu_* menu = (Fl_Menu_*)w;
	const Fl_Menu_Item* item = menu->mvalue();
	bool value = item->value();
	// Remeber it
	Fl_Preferences spad_settings(settings_, "Scratchpad");
	spad_settings.set("Enabled", (int)value);
	add_scratchpad();
}

// Log->Change->Rig/Aerial/QTH - open a dialog to specify new rig/aerial/QTH to use
// v is enum use_dialog_t: UD_RIG, UD_AERIAL, UD_QTH, UD_PROP
void menu::cb_mi_oper_change(Fl_Widget* w, void* v) {
	use_dialog_t type = (use_dialog_t)(long)v;
	// Open dialog for the type - dialog changes settings
	use_dialog* dialog = new use_dialog(type);
	if (dialog->display() == BN_OK) {
		string path;
		switch (type) {
		case UD_RIG:
			path = "Rigs";
			break;
		case UD_AERIAL:
			path = "Aerials";
			break;
		case UD_QTH:
			path = "QTHs";
			break;
		case UD_PROP:
			path = "Propagation Mode";
			break;
		}
		// get the modified settings
		Fl_Preferences station_settings(settings_, "Stations");
		Fl_Preferences use_settings(station_settings, path.c_str());
		char * temp;
		use_settings.get("Current", temp, "");
		// Change the field in the current record 
		switch (type)
		{
		case UD_RIG:
			book_->get_record()->item("MY_RIG", string(temp));
			break;
		case UD_AERIAL:
			book_->get_record()->item("MY_ANTENNA", string(temp));
			break;
		case UD_QTH:
			book_->get_record()->item("APP_ZZA_QTH", string(temp));
			break;
		case UD_PROP:
			book_->get_record()->item("PROP_MODE", string(temp));
		}
		free(temp);
	}
	book_->modified(true);
	// Update views with the mdoified record
	book_->selection(-1, HT_MINOR_CHANGE);

	Fl::delete_widget(dialog);
}

// Log->Change->Rig/Aerial/QTH - open a dialog to specify new rig/aerial/QTH to use
// v is enum use_dialog_t: UD_RIG, UD_AERIAL, UD_QTH or UD_PROP
void menu::cb_mi_oper_set(Fl_Widget* w, void* v) {
	use_dialog_t type = (use_dialog_t)(long)v;
	// Open dialog for the type - dialog changes settings
	use_dialog* dialog = new use_dialog(type);
	if (dialog->display() != BN_OK) {
		status_->misc_status(ST_WARNING, "Set rig or aerail or QTH cancelled by user");
	}
	book_->modified(true);
	Fl::delete_widget(dialog);
}

// Import->File
// v is not used
void menu::cb_mi_imp_file(Fl_Widget* w, void* v) {
	// Cancel any existing update
	import_data_->stop_update(LM_OFF_AIR, false);
	while (!import_data_->update_complete()) Fl::wait();
	// Get data directory
	char* directory;
	Fl_Preferences datapath_settings(settings_, "Datapath");
	datapath_settings.get("Log Directory", directory, "");
		// Open file chooser
	string filename;
	Fl_File_Chooser* chooser = new Fl_File_Chooser(directory, "ADI Files(*.adi)\tADX Files (*.adx)", Fl_File_Chooser::SINGLE, "Select file name");
	chooser->callback(cb_chooser, &filename);
	chooser->textfont(FONT);
	chooser->textsize(FONT_SIZE);
	chooser->show();
	// Wait while the dialog is active (visible)
	while (chooser->visible()) Fl::wait();
	if (chooser->count()) {
		import_data_->load_data(filename);
	}
	delete chooser;
	free(directory);
}

// Import->Download->eQSL/LotW - fetch the data from the server into the import_data_ book
// v is enum update_mode_t: EQSL_UPDATE or LOTW_UPDATE
void menu::cb_mi_download(Fl_Widget* w, void* v) {
	// v has QSL server
	import_data_->download_data((import_data::update_mode_t)(long)v);
}

// Import->Merge - merge what has just been downloaded
// v is not used
void menu::cb_mi_imp_merge(Fl_Widget* w, void* v) {
	// Merge the data
	import_data_->merge_data();
}

// Import->Cancel
// v is not used
void menu::cb_mi_imp_cancel(Fl_Widget* w, void* v) {
	// Delete the import records
	import_data_->delete_contents(false);
	navigation_book_ = book_;
	tabbed_view_->activate_pane(OT_MAIN, true);
	navigation_book_->selection(0, HT_ALL);
}

// Import->Enable Auto Update
// v is not used
void menu::cb_mi_imp_enauto(Fl_Widget* w, void* v) {
	// Get the value of the checked menu item
	Fl_Menu_* menu = (Fl_Menu_*)w;
	const Fl_Menu_Item* item = menu->mvalue();
	bool value = item->value();
	import_data_->auto_enable(value);
}

// Extract->Clear
// v is not used
// clear the criteria and display the log book
void menu::cb_mi_ext_clr(Fl_Widget* w, void* v) {
	extract_records_->clear_criteria();
	navigation_book_ = book_;
	tabbed_view_->activate_pane(OT_MAIN, true);
	navigation_book_->selection(0, HT_EXTRACTION);
}

// Extract->Criteria
// v is not used
// Open the dialog to get the criteria, get the records accordingly and display the extract
void menu::cb_mi_ext_crit(Fl_Widget* w, void* v) {
	// Open search dialog
	search_dialog* dialog = new search_dialog;
	bool do_extract = true;
	while (do_extract && dialog->display() == BN_OK) {
		// Extract the records
		search_criteria_t criteria = *dialog->criteria();
		if (extract_records_->criteria(criteria)) {
			// Successful
			do_extract = false;
			navigation_book_ = extract_records_;
			tabbed_view_->activate_pane(OT_EXTRACT, true);
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
	navigation_book_ = extract_records_;
	tabbed_view_->activate_pane(OT_EXTRACT, true);
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

// Extract->eQSL/LotW/Card - specificall extract records for QSL use
// v is enum extract_mode_t: EQSL, LOTW or CARD
void menu::cb_mi_ext_qsl(Fl_Widget* w, void* v) {
	menu* that = (menu*)w;
	// v passes the particular option
	that->qsl_type_ = (extract_data::extract_mode_t)(long)v;
	extract_records_->extract_qsl(that->qsl_type_);
}

// Extract->Upload - upload to the server the data was extracted for
// v is not used
void menu::cb_mi_ext_upload(Fl_Widget* w, void* v) {
	extract_records_->upload();
}

// Extract->Print - print 
// v is not used
void menu::cb_mi_ext_print(Fl_Widget* w, void* v) {
	menu* that = (menu*)w;
	navigation_book_ = extract_records_;
	printer* ptr;
	switch (that->qsl_type_) {
	case extract_data::EQSL:
	case extract_data::LOTW:
		ptr = new printer(OT_EXTRACT);
		delete ptr;
		break;
	case extract_data::CARD:
		ptr = new printer(OT_CARD);
		delete ptr;
		break;
	}
}

// Reference->Prefix->Filter - used to set the filter for displaying prefix data
// v is enum report_filter_t: RF_NONE, RF_ALL, RF_EXTRACTED or RF_SELECTED
void menu::cb_mi_ref_filter(Fl_Widget* w, void* v) {
	// v contains the particular filter
	report_filter_t filter = (report_filter_t)(long)v;
	((pfx_tree*)tabbed_view_->get_view(OT_PREFIX))->set_filter(filter);
	tabbed_view_->activate_pane(OT_PREFIX, true);
}

// Reference->Prefix->Items - used to set the items  to display
// v is enum report_item_t: RI_CODE, RI_NICK, RI_NAME, RI_CQ_ZONE, RI_ITU_ZONE or RI_ADIF
void menu::cb_mi_ref_items(Fl_Widget* w, void* v) {
	// v defineds the items to dispaly
	report_item_t items = (report_item_t)(long)v;
	((pfx_tree*)tabbed_view_->get_view(OT_PREFIX))->set_items(items);
	tabbed_view_->activate_pane(OT_PREFIX, true);
}

// Reference->Prefix->Add details - Add or remove the details from the prefix record
// v is not used
void menu::cb_mi_ref_details(Fl_Widget* w, void* v) {
	// Get the value of the checked menu item
	Fl_Menu_* menu = (Fl_Menu_*)w;
	const Fl_Menu_Item* item = menu->mvalue();
	bool value = item->value();
	((pfx_tree*)tabbed_view_->get_view(OT_PREFIX))->add_details(value);
	tabbed_view_->activate_pane(OT_PREFIX, true);
}

// Reference->Reload Data - reload the specification data
// v is not used
void menu::cb_mi_ref_reload(Fl_Widget* w, void* v) {
	// Get spec_data_ to reload itself
	spec_data_->load_data(true);
}

// Report->Clear etc. - set the report filter
// v is enum report_filter_t: RF_NONE, RF_ALL, RF_ALL_CURRENT, RF_EXTRACTED or RF_SELECTED
void menu::cb_mi_rep_filter(Fl_Widget* w, void* v) {
	// v has the filter
	report_filter_t filter = (report_filter_t)(long)v;
	((report_tree*)tabbed_view_->get_view(OT_REPORT))->add_filter(filter);
	tabbed_view_->activate_pane(OT_REPORT, true);
}

// Report->Levelx - set report level n to category
// v contains two bytes { level, category }
void menu::cb_mi_rep_level(Fl_Widget* w, void* v) {
	long params = (long)v;
	report_cat_t category = (report_cat_t)(params & 0xFF);
	int level = params >> 8;
	((report_tree*)tabbed_view_->get_view(OT_REPORT))->add_category(level, category);
	tabbed_view_->activate_pane(OT_REPORT, true);
}

// Information->QRZ.com - query QRZ.com about the selected contact
// v is string*. nullptr = uses selected record else uses call sign in v.
void menu::cb_mi_info_qrz(Fl_Widget* w, void* v) {
	record* record = book_->get_record();
	menu* that = ancestor_view<menu>(w);
	if (record != nullptr || v != nullptr) {
		// If we have a selected record - get browser executable from settings
		string browser = that->get_browser();
		if (browser.length() == 0) {
			return;
		}
		// Open browser with QRZ.com URL 
#ifdef _WIN32
		char format[] = "start \"%s\" http://www.qrz.com/lookup?callsign=%s";
#else
		char format[] = "\"%s\" http://www.qrz.com/lookup?callsign=%s &";
#endif
		string call = v == nullptr ? record->item("CALL") : *((string*)v);
		char * url = new char[strlen(format) + call.length() + browser.length() + 10];
		sprintf(url, format, browser.c_str(), call.c_str());
		status_->misc_status(ST_NOTE, "MENU: Launching QRZ.com");
		int result = system(url);
		delete[] url;
	}
}

// Information->Google Maps - open browser with Google maps centered on contact's QTH (where calculable)
// v is not used
void menu::cb_mi_info_map(Fl_Widget* w, void* v) {
	// Get currently selected record
	record* record = book_->get_record();
	menu* that = ancestor_view<menu>(w);
	if (record != nullptr) {
		// If there is one
		// Get locator, QTH and callsign
		location_t source;
		lat_long_t location = record->location(false, source);
		string locator = record->item("GRIDSQUARE");
		string city = record->item("QTH");
		string label = record->item("CALL");
		// These are DOS specific, will need the Linux version
#ifdef _WIN32
		string format_coords = "start \"%s\" http://google.com/maps/@%f,%f,25000m";
		string format_city = "start \"%s\" http://google.com/maps/?q=%s&z=10";
#else
		string format_coords = "\"%s\" http://google.com/maps/@%f,%f,25000m &";
		string format_city = "\"%s\" http://google.com/maps/?q=%s&z=10 &";
#endif
		// Get browser from settings
		string browser = that->get_browser();
		if (browser.length() == 0) {
			return;
		}
		char * url = nullptr;
		if ((source == LOC_NONE || source == LOC_GRID2 || source == LOC_GRID4) && city != "") {
			// Location got from wide stuff, therefore use city location
			// ?q=City&z=10 - centre on city zoom level 10
			url = new char[format_city.length() + city.length() + browser.length() + 10];
			sprintf(url, format_city.c_str(), browser.c_str(), city.c_str());
		}
		else if (source == LOC_GRID6 || source == LOC_GRID8 || source == LOC_LATLONG) {
			// grid square is 6 or more characters 
			// @%f,%f,25000m display 25 km around long/lat
			url = new char[format_coords.length() + browser.length() + 30];
			sprintf(url,format_coords.c_str(), browser.c_str(), location.latitude, location.longitude);
		}
		else {
			// Any thing else could be a random location somewhere up to 1 degree away.
			status_->misc_status(ST_WARNING, "MENU: Insufficient QTH information in record to launch Google Maps");
			return;
		}
		status_->misc_status(ST_NOTE, "MENU: Launching google maps");
		int result = system(url);
		delete[] url;
	}
}

// Information->QSO Web-site
// v is not used
void menu::cb_mi_info_web(Fl_Widget* w, void* v) {
	// Get currently selected record
	record* record = book_->get_record();
	menu* that = ancestor_view<menu>(w);
	if (record != nullptr && record->item_exists("WEB")) {
		// Get browser from settings
		string browser = that->get_browser();
		if (browser.length() == 0) {
			return;
		}
		// Open browser with QRZ.com URL 
#ifdef _WIN32
		char format[] = "start \"%s\" %s";
#else
		char format[] = "\"%s\" %s &";
#endif
		string website = record->item("WEB");
		char* url = new char[website.length() + browser.length() + strlen(format) + 10];
		sprintf(url, format, browser.c_str(), website.c_str());
		status_->misc_status(ST_NOTE, "MENU: Opening QSO web-site");
		int result = system(url);
		delete[] url;
	}
}

// Help->About - display about dialog
// v is not used
void menu::cb_mi_help_abt(Fl_Widget* w, void* v) {
	about_dialog* dialog = new about_dialog();
	dialog->display();
}

// Help->Status->View - display status log file
// v is not used
void menu::cb_mi_help_view(Fl_Widget* w, void* v) {
	status_->misc_status()->do_callback();
}

// Help->Status->Note/Done/Warning/Error/Fatal
// v is enum status_t: minimum display level
void menu::cb_mi_help_level(Fl_Widget* w, void* v) {
	status_->min_level((status_t)(long)v);
}

// Hell->Intl - show/hide international character set
// v is not used
void menu::cb_mi_help_intl(Fl_Widget* w, void* v) {
	if (intl_dialog_->visible()) {
		intl_dialog_->hide();
	}
	else {
		intl_dialog_->show();
	}
}

// Enable/disable menu 
// active: false = disable all; true = enable those which can be enabled
void menu::enable(bool active) {
	active_enabled_ = active;
	// For all menu items
	for (int i = 0; i < size(); i++) {
		if (active) {
			// enable item
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
	else if (toolbar_) {
		// Disable buttons in the tool bar that map onto menu items
		toolbar_->update_items();
	}
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
		add(label, FL_ALT + i, cb_mi_file_open, (void*)i);
	}
}

// set the menu to the state of the logging mode - check mark the appropriate menu item
void menu::logging(logging_mode_t mode) {
	int index_conn = find_index("&Log/&Mode/&Radio Connected");
	int index_disc = find_index("&Log/&Mode/Radio &Disconnected");
	int index_offair = find_index("&Log/&Mode/O&ff-air");
	int index_import = find_index("&Log/&Mode/&Import");
	// Change the mode 
	switch (mode) {
	case LM_RADIO_CONN:
		// Tick On-Air only
		this->mode(index_conn, this->mode(index_conn) | FL_MENU_VALUE);
		this->mode(index_disc, this->mode(index_disc) & ~FL_MENU_VALUE);
		this->mode(index_offair, this->mode(index_offair) & ~FL_MENU_VALUE);
		this->mode(index_import, this->mode(index_import) & ~FL_MENU_VALUE);
		break;
	case LM_RADIO_DISC:
		// Tick On-Air only
		this->mode(index_conn, this->mode(index_conn) & ~FL_MENU_VALUE);
		this->mode(index_disc, this->mode(index_disc) | FL_MENU_VALUE);
		this->mode(index_offair, this->mode(index_offair) & ~FL_MENU_VALUE);
		this->mode(index_import, this->mode(index_import) & ~FL_MENU_VALUE);
		if (status_) status_->rig_status(ST_WARNING, "Real-time, no rig logging");
		break;
	case LM_OFF_AIR:
		// Tick Off-air only
		this->mode(index_conn, this->mode(index_conn) & ~FL_MENU_VALUE);
		this->mode(index_disc, this->mode(index_disc) & ~FL_MENU_VALUE);
		this->mode(index_offair, this->mode(index_offair) | FL_MENU_VALUE);
		this->mode(index_import, this->mode(index_import) & ~FL_MENU_VALUE);
		if (status_) status_->rig_status(ST_WARNING, "Off-air logging");
		break;
	case LM_IMPORTED:
		// Tick Import only
		this->mode(index_conn, this->mode(index_conn) & ~FL_MENU_VALUE);
		this->mode(index_disc, this->mode(index_disc) & ~FL_MENU_VALUE);
		this->mode(index_offair, this->mode(index_offair) & ~FL_MENU_VALUE);
		this->mode(index_import, this->mode(index_import) | FL_MENU_VALUE);
		if (status_) status_->rig_status(ST_WARNING, "Automatic import from data modem");
		break;
	}
	// Set logging mode for every-one to access
	logging_mode_ = mode;
}

// returns the logging mode
logging_mode_t menu::logging() {
	return logging_mode_;
}

// Set check marks on the menu to represent the actual report tree mode and filter
void menu::report_mode(vector<report_cat_t> report_mode, report_filter_t filter) {
	// Set the Report->Level N->* menu items
	for (size_t i = 1; i < 4; i++) {
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
				mode(index_states, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_entities) & ~FL_MENU_VALUE);
				break;
			case RC_PAS:
				// Set Entities/States only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_entities) | FL_MENU_VALUE);
				mode(index_bands, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_entities) & ~FL_MENU_VALUE);
				break;
			case RC_BAND:
				// Set Bands only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_entities) | FL_MENU_VALUE);
				mode(index_modes, mode(index_entities) & ~FL_MENU_VALUE);
				break;
			case RC_MODE:
				// Set Modes only
				mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_states, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_bands, mode(index_entities) & ~FL_MENU_VALUE);
				mode(index_modes, mode(index_entities) | FL_MENU_VALUE);
				break;
			}
		}
		else {
			// This level is not used - set Nothing
			mode(index_nothing, mode(index_nothing) | FL_MENU_VALUE);
			mode(index_entities, mode(index_entities) & ~FL_MENU_VALUE);
			mode(index_states, mode(index_entities) & ~FL_MENU_VALUE);
			mode(index_bands, mode(index_entities) & ~FL_MENU_VALUE);
			mode(index_modes, mode(index_entities) & ~FL_MENU_VALUE);

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
	}
}

// Set check marks on the menu items Help->Status->* to reflect current display level
void menu::status_level(status_t level) {
	// Set menu items Help->Status->*
	int index_note = find_index("&Help/&Status/&Note");
	int index_done = find_index("&Help/&Status/&Done");
	int index_warning = find_index("&Help/&Status/&Warning");
	int index_error = find_index("&Help/&Status/&Error");
	int index_severe = find_index("&Help/&Status/Se&vere");
	int index_fatal = find_index("&Help/&Status/&Fatal");
	switch (level) {
	case ST_NOTE:
		// Set Help->Status->Note only
		mode(index_note, mode(index_note) | FL_MENU_VALUE);
		mode(index_done, mode(index_done) & ~FL_MENU_VALUE);
		mode(index_warning, mode(index_warning) & ~FL_MENU_VALUE);
		mode(index_error, mode(index_error) & ~FL_MENU_VALUE);
		mode(index_severe, mode(index_severe) & ~FL_MENU_VALUE);
		mode(index_fatal, mode(index_fatal) & ~FL_MENU_VALUE);
		break;
	case ST_OK:
		// Set Help->Status->Done only
		mode(index_note, mode(index_note) & ~FL_MENU_VALUE);
		mode(index_done, mode(index_done)  | FL_MENU_VALUE);
		mode(index_warning, mode(index_warning) & ~FL_MENU_VALUE);
		mode(index_error, mode(index_error) & ~FL_MENU_VALUE);
		mode(index_severe, mode(index_severe) & ~FL_MENU_VALUE);
		mode(index_fatal, mode(index_fatal) & ~FL_MENU_VALUE);
		break;
	case ST_WARNING:
		// Set Help->Status->Warning only
		mode(index_note, mode(index_note) & ~FL_MENU_VALUE);
		mode(index_done, mode(index_done) & ~FL_MENU_VALUE);
		mode(index_warning, mode(index_warning) | FL_MENU_VALUE);
		mode(index_error, mode(index_error) & ~FL_MENU_VALUE);
		mode(index_severe, mode(index_severe) & ~FL_MENU_VALUE);
		mode(index_fatal, mode(index_fatal) & ~FL_MENU_VALUE);
		break;
	case ST_ERROR:
		// Set Help->Status->Error only
		mode(index_note, mode(index_note) & ~FL_MENU_VALUE);
		mode(index_done, mode(index_done) & ~FL_MENU_VALUE);
		mode(index_warning, mode(index_warning) & ~FL_MENU_VALUE);
		mode(index_error, mode(index_error) | FL_MENU_VALUE);
		mode(index_severe, mode(index_severe) & ~FL_MENU_VALUE);
		mode(index_fatal, mode(index_fatal) & ~FL_MENU_VALUE);
		break;
	case ST_SEVERE:
		// Set Help->Status->Error only
		mode(index_note, mode(index_note) & ~FL_MENU_VALUE);
		mode(index_done, mode(index_done) & ~FL_MENU_VALUE);
		mode(index_warning, mode(index_warning) & ~FL_MENU_VALUE);
		mode(index_error, mode(index_error) & ~FL_MENU_VALUE);
		mode(index_severe, mode(index_severe) | FL_MENU_VALUE);
		mode(index_fatal, mode(index_fatal) & ~FL_MENU_VALUE);
		break;
	case ST_FATAL:
		// Set Help->Status->Fatal only
		mode(index_note, mode(index_note) & ~FL_MENU_VALUE);
		mode(index_done, mode(index_done) & ~FL_MENU_VALUE);
		mode(index_warning, mode(index_warning) & ~FL_MENU_VALUE);
		mode(index_error, mode(index_error) & ~FL_MENU_VALUE);
		mode(index_severe, mode(index_severe) & ~FL_MENU_VALUE);
		mode(index_fatal, mode(index_fatal) | FL_MENU_VALUE);
		break;
	}
}

// Update menu items' active state depending on state of the book
void menu::update_items() {
	if (active_enabled_) {
		// Get conditions
		bool modified = book_->modified();
		bool empty = book_->size() == 0;
		bool first = navigation_book_->selection() == 0;
		bool last = navigation_book_->selection() == navigation_book_->size() - 1;
		bool record_modified = book_->modified_record();
		bool new_record = book_->new_record();
		bool save_enabled = book_->save_enabled();
		bool delete_enabled = book_->delete_enabled();
		bool web_enabled = book_ && book_->get_record() && book_->get_record()->item_exists("WEB");
		bool auto_enable = import_data_ && import_data_->auto_enable();
		// Get all relevant menu item indices
		int index_save = find_index("&File/&Save");
		int index_saveas = find_index("&File/Save &As");
		int index_auto = find_index("&File/Au&to Save");
		int index_print = find_index("&File/&Print");
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
		int index_spad = find_index("&Log/Scratc&hpad");
		int index_change = find_index("&Operating/C&hange");
		int index_extract = find_index("E&xtract");
		int index_ref = find_index("&Reference");
		int index_rep = find_index("Re&port");
		int index_info = find_index("&Information");
		int index_web = find_index("&Information/QSO &Web-site");
		int index_use_log = find_index("&Log/&Use View/Main &Log");
		int index_use_form = find_index("&Log/&Use View/&Report View");
		int index_use_spad = find_index("&Log/&Use View/&Scratchpad");
		int index_auto_enable = find_index("&Import/&Enable Auto-Update");
		// Enable/Disable save 
		if (modified) {
			mode(index_save, mode(index_save) & ~FL_MENU_INACTIVE);
		}
		else {
			mode(index_save, mode(index_save) | FL_MENU_INACTIVE);
		}
		// Auto save enabled
		if (save_enabled) {
			mode(index_auto, mode(index_auto) | FL_MENU_VALUE);
		}
		else {
			mode(index_auto, mode(index_auto) & ~FL_MENU_VALUE);
		}
		if (empty) {
			// Disallow log/record modification etc. if the file is empty
			mode(index_saveas, mode(index_saveas) | FL_MENU_INACTIVE);
			mode(index_print, mode(index_print) | FL_MENU_INACTIVE);
			mode(index_change, mode(index_change) | FL_MENU_INACTIVE);
			mode(index_extract, mode(index_extract) | FL_MENU_INACTIVE);
			mode(index_ref, mode(index_ref) | FL_MENU_INACTIVE);
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
			mode(index_change, mode(index_change) & ~FL_MENU_INACTIVE);
			mode(index_extract, mode(index_extract) & ~FL_MENU_INACTIVE);
			mode(index_ref, mode(index_ref) & ~FL_MENU_INACTIVE);
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
		else if (last & !first) {
			// Disable NAvigate->Next and Navigate->Last and enable others
			mode(index_first, mode(index_first) & ~FL_MENU_INACTIVE);
			mode(index_prev, mode(index_prev) & ~FL_MENU_INACTIVE);
			mode(index_next, mode(index_next) | FL_MENU_INACTIVE);
			mode(index_last, mode(index_last) | FL_MENU_INACTIVE);
			mode(index_find, mode(index_find) & ~FL_MENU_INACTIVE);
		}
		else if (first & last) {
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
		// Delete enabled
		if (delete_enabled) {
			mode(index_delete, mode(index_delete) & ~FL_MENU_INACTIVE);
		}
		else {
			mode(index_delete, mode(index_delete) | FL_MENU_INACTIVE);
		}
		// Scratchpad
		Fl_Preferences spad_settings(settings_, "Scratchpad");
		int spad_enabled;
		spad_settings.get("Enabled", spad_enabled, (int)false);
		if (spad_enabled) {
			mode(index_spad, mode(index_spad) | FL_MENU_VALUE);
		}
		else {
			mode(index_spad, mode(index_spad) & ~FL_MENU_VALUE);
		}
		// Editting view
		switch (editting_view_) {
		case OT_MAIN:
			mode(index_use_log, mode(index_use_log) | FL_MENU_VALUE);
			mode(index_use_form, mode(index_use_form) & ~FL_MENU_VALUE);
			mode(index_use_spad, mode(index_use_spad) & ~FL_MENU_VALUE);
			break;
		case OT_REPORT:
			mode(index_use_log, mode(index_use_log) & ~FL_MENU_VALUE);
			mode(index_use_form, mode(index_use_form) | FL_MENU_VALUE);
			mode(index_use_spad, mode(index_use_spad) & ~FL_MENU_VALUE);
			break;
		case OT_SCRATCH:
			mode(index_use_log, mode(index_use_log) & ~FL_MENU_VALUE);
			mode(index_use_form, mode(index_use_form) & ~FL_MENU_VALUE);
			mode(index_use_spad, mode(index_use_spad) | FL_MENU_VALUE);
			break;
		}
		// Info->QSO WEb-site
		if (web_enabled) {
			mode(index_web, mode(index_web) & ~FL_MENU_INACTIVE);
		}
		else {
			mode(index_web, mode(index_web) | FL_MENU_INACTIVE);
		}
		// Import->Enable Auto-Update
		if (auto_enable) {
			mode(index_auto_enable, mode(index_auto_enable) | FL_MENU_VALUE);
		}
		else {
			mode(index_auto_enable, mode(index_auto_enable) & ~FL_MENU_VALUE);
		}
		// Update logging commands and rig status
		logging(logging());
	}
	redraw();
	// Update toolbar to reflect active state of menu items
	toolbar_->update_items();
}

// Get the browser form settings or if not ask user
string menu::get_browser() {
	// Get browser from settings
	char* temp;
	Fl_Preferences datapath_settings(settings_, "Datapath");
	datapath_settings.get("Web Browser", temp, "");
	string browser = temp;
	free(temp);
	if (!browser.length()) {
		// User hasn't defined browser yet - open dialog to get it
		Fl_File_Chooser* chooser = new Fl_File_Chooser("", "Applications(*.exe)", Fl_File_Chooser::SINGLE, "Please select your favoured web browser");
		chooser->callback(cb_chooser, &browser);
		chooser->textfont(FONT);
		chooser->textsize(FONT_SIZE);
		chooser->show();
		// Wait while the dialog is active (visible)
		while (chooser->visible()) Fl::wait();
		if (!chooser->count()) {
			status_->misc_status(ST_WARNING, "MENU: No browser selected, abandoning");
			return "";
		}
		delete chooser;
	}
	datapath_settings.set("Web Browser", browser.c_str());
	return browser;
}

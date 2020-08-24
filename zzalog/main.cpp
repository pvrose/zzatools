/*
ZZALOG - Amateur radio log
© - Copyright 2018, Philip Rose, GM3ZZA
All Rights Reserved

main.cpp - application entry point
*/

// local header files

#include "icons.h"
#include "../zzalib/utils.h"
#include "record.h"
#include "settings.h"
#include "menu.h"
#include "book.h"
#include "pfx_data.h"
#include "toolbar.h"
#include "spec_data.h"
#include "../zzalib/rig_if.h"
#include "../zzalib/ic7300.h"
#include "status.h"
#include "tabbed_forms.h"
#include "import_data.h"
#include "extract_data.h"
#include "lotw_handler.h"
#include "eqsl_handler.h"
#include "../zzalib/url_handler.h"
#include "pfx_tree.h"
#include "spec_tree.h"
#include "report_tree.h"
#include "../zzalib/callback.h"
#include "version.h"
#include "drawing.h"
#include "intl_dialog.h"
#include "band_view.h"
#include "scratchpad.h"
#include "dxa_if.h"
#include "qrz_handler.h"
#include "club_handler.h"
#include "ic7300_table.h"
#include "wsjtx_handler.h"

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

using namespace std;
using namespace zzalog;
using namespace zzalib;

// Global data items instanced in zzalib
extern rig_if* rig_if_;
extern ic7300* ic7300_;
extern url_handler* url_handler_;

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
pfx_data* pfx_data_ = nullptr;
spec_data* spec_data_ = nullptr;
eqsl_handler* eqsl_handler_ = nullptr;
lotw_handler* lotw_handler_ = nullptr;
qrz_handler* qrz_handler_ = nullptr;
Fl_RGB_Image* main_icon_ = nullptr;
Fl_Single_Window* main_window_ = nullptr;
intl_dialog* intl_dialog_ = nullptr;
band_view* band_view_ = nullptr;
scratchpad* scratchpad_ = nullptr;
club_handler* club_handler_ = nullptr;
wsjtx_handler* wsjtx_handler_ = nullptr;
#ifdef _WIN32
dxa_if* dxatlas_ = nullptr;
#endif
// Readonly flag on command-line
bool read_only_ = false;
// Recent files opened
list<string> recent_files_;

// Forward declarations
void backup_file(bool force, bool retrieve = false);
void set_recent_file(string filename);

// Display the time in the local timezone rather than UTC
bool use_local_ = false;
// Flag to prevent more than closure process at the same time
bool closing_ = false;
// Flag to mark everything loaded
bool initialised_ = false;
// Time loaded
time_t session_start_  = (time_t)0;

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
		status_->misc_status(ST_NOTE, "ZZALOG: Closing...");
		// If rig connected close it - this will close the timer as well. Tell the scorebaord there
		// is no longer a rig to read.
		if (rig_if_) {
			rig_if_->close();
			if (scratchpad_) {
				scratchpad_->update();
			}
		}
		// Delete band view
		if (band_view_) {
			Fl::delete_widget(band_view_);
			band_view_ = nullptr;
		}
		if (scratchpad_) {
			delete scratchpad_;
			scratchpad_ = nullptr;
		}
		if (dxatlas_) {
			delete dxatlas_;
			dxatlas_ = nullptr;
		}
		// Currently modifying a (potentially new) record
		if (book_ && (book_->modified_record() || book_->new_record()) ) {
			switch (fl_choice("You are currently modifying a record? Save or Quit?", "Save?", "Quit?", nullptr)) {
			case 0:
				// Save
				book_->save_record();
				break;
			case 1:
				// Quit - delete any new record
				book_->delete_record(book_->new_record());
				break;
			}
		}
		// Wait for auto-import of files to complete
		if (import_data_) {
			if (!import_data_->update_complete()) {
				switch (fl_choice("There is an import in process. Do you want to let it finish or abandon it?", "Finish?", "Abandon", nullptr)) {
				case 0:
					// Gracefully wait for import to complete
					import_data_->stop_update(LM_OFF_AIR, false);
					while (!import_data_->update_complete()) Fl::wait();
					break;
				case 1:
					// Immediately stop the import
					import_data_->stop_update(LM_OFF_AIR, true);
					break;
				}
			}
		}
		// Wait for eQSL card downloads - user can cancel
		if (eqsl_handler_ && eqsl_handler_->requests_queued()) {
			switch (fl_choice("There are outstanding eQSL card image requests. Do you want to cancel download, wait or cancel exit?", "Cancel download", "Wait", "Cancel exit")) {
			case 0:
				// Cancel the download immediately
				eqsl_handler_->enable_fetch(eqsl_handler::EQ_ABANDON);
				break;
			case 1:
				// Wait for the request queue to empty
				while (eqsl_handler_->requests_queued()) Fl::wait();
				break;
			case 2:
				// Cancel Exit - don't doing anything else
				closing_ = false;
				return;
			}
		}

		// Check the book needs saving
		if (book_ && book_->modified()) {
			switch (fl_choice("Book has been modified. Do you want to save and exit, exit or cancel exit?", "Exit", "Save && Exit", "Cancel Exit")) {
			case 0:
				// Exit
				break;
			case 1:
				// Save and Exit
				if (read_only_) {
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
				closing_ = false;
				return;
			}
		}

		// Shutdown the WSJT-X server
		if (wsjtx_handler_) {
			wsjtx_handler_->close_socket();
		}

		// Back up the book
		if (book_ && initialised_) {
			backup_file(false);
		}

		// Save the window position
		Fl_Preferences window_settings(settings_, "Window");
		window_settings.set("Left", main_window_->x_root());
		window_settings.set("Top", main_window_->y_root());
		window_settings.set("Width", main_window_->w());
		window_settings.set("Height", main_window_->h());

		// Hide all the open windows - this will allow Fl to close the app.
		for (Fl_Window* w = Fl::first_window(); w; w = Fl::first_window()) w->hide();
		status_->null_file_viewer();

		// Exit and close application
		Fl_Single_Window::default_callback((Fl_Window*)w, v);
	}
}

// Callback to parse arguments
// -r | --read_only : marks the file read-only
int cb_args(int argc, char** argv, int& i) {
	// Look for read_only (-r or --read_only)
	if (strcmp("-r", argv[i]) == 0 || strcmp("--read_only", argv[i]) == 0) {
		read_only_ = true;
		i += 1;
		return 1;
	}
	return 0;
}

// Use supplied argument, or read the latest file from settings or open file chooser if that's an empty string
string get_file(char * arg_filename) {
	string result = "";
	if (!arg_filename || !(*arg_filename)) {
		// null argument or empty string - get the recent file settings group.
		Fl_Preferences recent_settings(settings_, "Recent Files");
		char *filename;
		// get the most recent file
		if (recent_settings.get("File1", filename, "")) {
			// return the obtained filename
			result = string(filename);
			free(filename);
		}
		else {
			// No recent file - open the file chooser
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
	// Add the selected file to the front of the recent file list (and update menu)
	set_recent_file(result);
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
	tip_settings.get("Duration", duration, (float)TIP_SHOW);
	tip_settings.get("Font Name", font, FONT);
	tip_settings.get("Font Size", size, FONT_SIZE);
	// Default tooltip properties
	Fl_Tooltip::size(size);
	Fl_Tooltip::font(font);
	Fl_Tooltip::delay(duration);
	// Default message properties
	fl_message_size_ = FONT_SIZE;
	fl_message_font_ = FONT;
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
		pfx_data_ = new pfx_data;
		// Draw the prefix data
		((pfx_tree*)tabbed_forms_->get_view(OT_PREFIX))->populate_tree(false);
	}
	if (!closing_) {
		// add ADIF data.
		spec_data_ = new spec_data;
		// Draw the ADIF data
		((spec_tree*)tabbed_forms_->get_view(OT_ADIF))->populate_tree(false);
	}
	// Add intl dialog
	if (!closing_) {
		intl_dialog_ = new intl_dialog;
		// Don't show here - add a menu item to show it.
	}
}

// read in the log data
void add_book(char* arg) {
	if (!closing_) {
		// Check this is a new session
		void* p_last = &session_start_;
		time_t today = time(nullptr);
		settings_->get("Session End", p_last, (const void*)&p_last, sizeof(time_t));
		if (difftime(today, session_start_) > 1800.0) {
			// It is > 30 minutes since we last saved a record - new session
			session_start_ = today;
			status_->misc_status(ST_NOTE, "ZZALOG: Starting new session");
		}
		else {
			status_->misc_status(ST_NOTE, "ZZALOG: Less than 30 minutes since last session - resuming");
		}
		// Create the book options and set them in the forms
		book_ = new book;
		navigation_book_ = book_;
		import_data_ = new import_data;
		extract_records_ = new extract_data;
		// Tell the views that a book now exists
		tabbed_forms_->books();
		// Get filename and load the data
		string log_file = get_file(arg);

		if (book_->load_data(log_file)) {
			// All actions now performed in book_
		}
	}
}

// Update rig information in the various views - this is controlled by rig_if_.
void cb_rig_timer() {
	// There may be a race hazard involving flrig and zzalib when I try and close zzalib
	if (!closing_) {
		// Band view may not have been created yet
		if (band_view_) {
			band_view_->update(rig_if_->tx_frequency() / 1000.0);
		}
		// Update scratchpad
		string freq = rig_if_->get_frequency(true);
		if (scratchpad_) {
			string mode;
			string submode;
			rig_if_->get_string_mode(mode, submode);
			string power = rig_if_->get_tx_power();
			if (submode.length()) {
				scratchpad_->rig_update(freq, submode, power);
			}
			else {
				scratchpad_->rig_update(freq, mode, power);
			}
		}
		// Update rig status
		if (!band_view_ || band_view_->in_band(rig_if_->tx_frequency()/1000.0)) {
			if (rig_if_->get_tx() == true) {
				status_->rig_status(RS_TX, rig_if_->rig_info().c_str());
			}
			else {
				status_->rig_status(RS_RX, rig_if_->rig_info().c_str());
			}
		}
		else {
			status_->rig_status(RS_ERROR, rig_if_->rig_info().c_str());
		}
	}
}

// Callback for rig_if_ to use to access spec_data_
string cb_freq_to_band(double frequency) {
	return spec_data_->band_for_freq(frequency);
}

// Callback for rig_if_ to use to display messages in statues_
void cb_error_message(bool ok, const char* message) {
	status_->misc_status(ok ? ST_NOTE : ST_ERROR, message);
}

// Create the rig interface handler and connect to the rig.
void add_rig_if() {
	if (!closing_) {
		fl_cursor(FL_CURSOR_WAIT);
		delete rig_if_;
		// Get the handler from the settings
		rig_handler_t handler;
		Fl_Preferences rig_settings(settings_, "Rig");
		rig_settings.get("Handler", (int&)handler, RIG_FLRIG);
		Fl_Preferences stations_settings(settings_, "Stations");
		Fl_Preferences rigs_settings(stations_settings, "Rigs");
		char* rig_name;
		rigs_settings.get("Current", rig_name, "");
		char temp[256];
		char handler_name[8];
		switch (handler) {
		case RIG_HAMLIB:
			strcpy(handler_name, "hamlib");
			break;
		case RIG_FLRIG:
			strcpy(handler_name, "FlRig");
			break;
		}
		if (handler != RIG_NONE) {
			sprintf(temp, "RIG: Connecting %s (%s)", rig_name, handler_name);
			status_->misc_status(ST_NOTE, temp);
		}
		// Create the appropriate interface handler
		switch (handler) {
		case RIG_HAMLIB:
			rig_if_ = new rig_hamlib;
			break;
		case RIG_FLRIG:
			// If the HTTP URL handler hasn't yet been created do so.
			if (url_handler_ == nullptr) url_handler_ = new url_handler;
			rig_if_ = new rig_flrig;
			break;
		case RIG_NONE:
			status_->misc_status(ST_NOTE, "RIG: Not connecting one");
			rig_if_ = nullptr;
			break;
		default:
			rig_if_ = nullptr;
			break;
		}
		if (rig_if_ == nullptr) {
			// No handler defined - assume manual logging in real-time
			status_->misc_status(ST_WARNING, "RIG: No handler - assume real-time logging, no rig");
			menu_->logging(LM_ON_AIR);
		}
		else {
			// Set callbacks
			rig_if_->callback(cb_rig_timer, cb_freq_to_band, cb_error_message);
			// Try and open the connection to the rig
			bool done = false;
			while (!done) {
				if (rig_if_->open()) {
					if (!rig_if_->is_good()) {
						// No rig handler or rig didn't open - assume manual logging
						delete rig_if_;
						rig_if_ = nullptr;
						status_->misc_status(ST_ERROR, "RIG: No handler - assume real-time logging, no rig");
						done = true;
						// Change logging mode from IMPORTED to ON_AIR. otherwise leave as was
						if (menu_->logging() == LM_IMPORTED) menu_->logging(LM_ON_AIR);
					}
					else {
						// Connect to rig OK - see if we are a digital mode
						if ((rig_if_->mode() == GM_DIGL || rig_if_->mode() == GM_DIGU) && import_data_->start_auto_update()) {
							char message[256];
							snprintf(message, 256, "RIG: %s", rig_if_->success_message().c_str());
							status_->misc_status(ST_OK, message);
							// start auto-data mode so we import the log written by the mode app
							status_->misc_status(ST_WARNING, "RIG: Data mode - assume logging by data modem app");
							// Change logging mode to IMPORTED as will be using a data-modem
							done = true;
							menu_->logging(LM_IMPORTED);
						}
						// The first access to read the mode may fail
						else if (!rig_if_->is_good()) {
							char message[512];
							sprintf(message, "RIG: Bad access - %s. Assume real-time logging, no rig", rig_if_->error_message().c_str());
							// Problem with rig_if when making first access, close it to stop timer and clean up 
							rig_if_->close();
							string error_message = rig_if_->error_message();
							delete rig_if_;
							rig_if_ = nullptr;
							if (scratchpad_) {
								scratchpad_->update();
							}
							status_->misc_status(ST_ERROR, message);
							// Put the error message from the rig in the rig status box
							status_->rig_status(RS_ERROR, error_message.c_str());
							done = true;
							// Change logging mode from IMPORTED to ON_AIR. otherwise leave as was
							if (menu_->logging() == LM_IMPORTED) menu_->logging(LM_ON_AIR);
						}
						else {
							// Rig seems OK - timer will have been started by rig_if_->open()
							// Set rig timer callnack
							char message[256];
							snprintf(message, 256, "RIG: %s", rig_if_->success_message().c_str());
							status_->misc_status(ST_OK, message);
							// Get the operating condition from the rig
							if (!band_view_ || band_view_->in_band(rig_if_->tx_frequency())) {
								status_->rig_status(rig_if_->get_tx() ? RS_TX : RS_RX, rig_if_->rig_info().c_str());
							}
							else {
								status_->rig_status(RS_ERROR, rig_if_->rig_info().c_str());
							}
							done = true;
							// Change logging mode to ON_AIR
							menu_->logging(LM_ON_AIR);
						}
						// Note this is IC-7300 specific code
						ic7300_table* mem_table = (ic7300_table*)(tabbed_forms_->get_view(OT_MEMORY));
						if (rig_if_ && rig_if_->rig_name() == "IC-7300") {
							ic7300_ = new ic7300;
							rig_if_->update_clock();
							mem_table->type(mem_table->type());
						}
						else {
							mem_table->type(VT_NONE);
						}
					}
				}
				else {
					// Rig hadn't opened - set off-air logging
					char temp[128] = "RIG: failed to open - no information";
					if (rig_if_) {
						snprintf(temp, sizeof(temp), "RIG: failed to open - %s", rig_if_->error_message().c_str());
					}
					delete rig_if_;
					rig_if_ = nullptr;
					status_->misc_status(ST_ERROR, temp);
					done = true;
				}
			}
		}
		// Tell menu to update its items
		menu_->update_items();
		if (rig_if_ == nullptr) {
			status_->rig_status(RS_OFF, "No rig present");
		}
		fl_cursor(FL_CURSOR_DEFAULT);
	}
}

// Add the band plan window
void add_band_view() {
	if (!closing_) {
		if (rig_if_) {
			// Use actual rig frequency 
			band_view_ = new band_view(rig_if_->tx_frequency() / 1000.0, 400, 100, "Band plan");
		}
		else {
			// Use frequency of selected record if the book is not empty, else use 14.1 MHz
			double frequency;
			if (book_->size()) {
				frequency = stod(book_->get_record()->item("FREQ")) * 1000.0;
			}
			else {
				frequency = 14100.0;
			}
			band_view_ = new band_view(frequency, 400, 100, "Band plan");
		}
		if (!band_view_->valid()) {
			Fl::delete_widget(band_view_);
			band_view_ = nullptr;
		}
	}
}

// Add the various HTML handlers
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
	}
}

// Add a scratchpad
void add_scratchpad() {
	if (!closing_) {
		if (!scratchpad_) {
			scratchpad_ = new scratchpad;
		}
		int enabled;
		int top;
		int left;
		Fl_Preferences spad_settings(settings_, "Scratchpad");
		spad_settings.get("Enabled", enabled, (int)false);
		spad_settings.get("Top", top, 100);
		spad_settings.get("Left", left, 100);
		if (top < 0) top = 100;
		if (left < 0) left = 100;
		if (enabled) {
			// Show the scratchpad at the saved position
			scratchpad_->show();
			scratchpad_->position(left, top);
			status_->misc_status(ST_NOTE, "SCRATCHPAD: Opened");
		}
		else {
			// Hide it
			scratchpad_->hide();
			status_->misc_status(ST_NOTE, "SCRATCHPAD: Closed");
		}
	}
}

// Add DxAtlas control window
void add_dxatlas() {
	if (!closing_ && dxatlas_ == nullptr) {
		dxatlas_ = new dxa_if();
	}
}

// Set the text in the main window label
void main_window_label(string text) {
	// e.g. ZZALOG 3.0.0: <filename>
	string label = PROGRAM_ID + " " + PROGRAM_VERSION;
	if (text.length()) {
		label += ": " + text;
	}
	main_window_->copy_label(label.c_str());
}

// Create the main window
void create_window() {
	// Create the main window
	main_window_ = new Fl_Single_Window(WIDTH, HEIGHT);
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
	// Status bar - tracks progress and reports any messages and errors.
	status_ = new status(0, curr_y, WIDTH, TOOL_HEIGHT);
	main_window_->add(status_);
	curr_y += status_->h();
	// Toolbar - image buttons representing a number of menu items - disable all menu related items
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
	Fl_Preferences window_settings(settings_, "Window");
	window_settings.get("Left", left, -1);
	window_settings.get("Top", top, -1);
	window_settings.get("Width", width, WIDTH);
	window_settings.get("Height", height, HEIGHT);
	// Only allow the views to resize fully - the bars will resize horizontally
	main_window_->resizable(tabbed_forms_);
	// Get minimum resizing from all the children - horizontal limited by views and toolbar
	int min_w = max(tabbed_forms_->min_w(), toolbar_->min_w());
	// Vertical limited by view, the bars remain a fixed height
	int min_h = tabbed_forms_->min_h() + status_->h() + toolbar_->h() + menu_->h();
	main_window_->size_range(min_w, min_h);
	// Set the size to the setting or minimum specified by the view + bars if that's larger
	main_window_->resize(left, top, max(min_w, width), max(min_h, height));
}

// Tidy memory
void tidy() {
	// Tidy memory - this is not perfect
	// From inspection of the code - calling this a second time frees the memory
	fl_message_title_default(nullptr);
	delete scratchpad_;
	delete dxatlas_;
	delete wsjtx_handler_;
	delete club_handler_;
	delete qrz_handler_;
	delete lotw_handler_;
	delete eqsl_handler_;
	delete url_handler_;
	delete band_view_;
	delete rig_if_;
	delete extract_records_;
	delete import_data_;
	delete book_;
	delete intl_dialog_;
	delete spec_data_;
	delete pfx_data_;
	delete tabbed_forms_;
	delete toolbar_;
	delete status_;
	delete menu_;
	delete main_window_;
	delete settings_;
	delete main_icon_;
}

// Add the icon
void add_icon(const char* arg0) {
#ifndef _WIN32
	// set the default Icon
	main_icon_ = new Fl_RGB_Image(ICON_MAIN, 16, 16, 4);
#else
	// TODO: On windows we have a separate icon file - never worked out how to get one into the file
	// Find the directory the app is loaded from and add the icon filename
	const char* last_slash = strrchr(arg0, '\\');
	int pos = last_slash - arg0;
	char* path = new char[pos + 16];
	strncpy(path, arg0, pos);
	*(path + pos) = 0;
	strcat(path, "\\zzalog.png");
	main_icon_ = new Fl_PNG_Image(path);
#endif
	// Use the icon as the default for all windows
	Fl_Window::default_icon(main_icon_);
}

// Display the arguments in the status log
void print_args(int argc, char** argv) {
	// Create a string to hold all the info
	int length = 20;
	for (int i = 0; i < argc; i++) {
		length += strlen(argv[i]);
	}
	char* message = new char[length + 10];
	// Generate the string
	strcpy(message, "ZZALOG: ");
	for (int i = 0; i < argc; i++) {
		strcat(message, argv[i]);
		strcat(message, " ");
	}
	strcat(message, "Started");
	status_->misc_status(ST_LOG, message);
}

// The main app entry point
int main(int argc, char** argv)
{
	// Parse command-line arguments - accept FLTK standard arguments and custom ones (in cb_args)
	int i = 1;
	Fl::args(argc, argv, i, cb_args);
	// Create the settings before anything else 
	settings_ = new Fl_Preferences(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str());
	// Create window
	add_icon(argv[0]);
	create_window();
	add_properties();
	recent_files();

	// add the various drawn items
	int curr_y = 0;
	add_widgets(curr_y);
	print_args(argc, argv);
	// Resize the window
	resize_window();
	// now show the window
	main_window_->show(argc, argv);
	// Read in reference data - uses progress
	add_data();
	// Read in log book data - uses progress - use supplied argument for filename
	add_book(argc == 1 ? nullptr : argv[argc - 1]);
	// Connect to the rig
	rig_if_ = nullptr;
	add_rig_if();
	// Add band-plan 
	add_band_view();
	// Add qsl_handlers - note add_rig_if() may have added URL handler
	add_qsl_handlers();
	// Add scratchpad
	add_scratchpad();
	// Add DxAtlas interface
	add_dxatlas();
	// enable menu
	int code = 0;
	// We are now initialised
	initialised_ = true;
	if (!closing_) {
		// Only do this if we haven't tried to close
		menu_->enable(true);
		menu_->redraw();
		// Start WSJT-X server
		wsjtx_handler_->run_server();
		// Run the application until it is closed
		code = Fl::run();
	}
	// Delete everything we've created
	tidy();
	return code;
}

// Copy existing data to back up file. force = true used by menu command, 
void backup_file(bool force, bool retrieve) {
	// This needs to be int and not bool as the settings.get() would corrupt the stack.
	int settings_force;
	Fl_Preferences backup_settings(settings_, "Backup");
	backup_settings.get("Enable", settings_force, false);
	if (!force && !retrieve) {
		if (!settings_force) {
			if (fl_choice("Backup is currently disabled, do you want to enable it?", "Yes", "No", nullptr) == 0) {
				settings_force = true;
			}
		}
		backup_settings.set("Enable", (int)settings_force);
	}
	if (force || settings_force) {
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

		// ensure a '\' is appendded
		if (backup.back() != '/' && backup.back() != '\\') {
			backup += '\\';
		}
		// Get source filename
		string source = book_->filename(true);
		// Create backup filename - use back-up directory and current file-name
		size_t last_slash = source.find_last_of("/\\");
		if (last_slash == string::npos) {
			backup += source;
		}
		else {
			backup += source.substr(last_slash + 1);
		}
		char* message = new char[backup.length() + 25];
		if (retrieve) {
			sprintf(message, "RETRIEVE: Reading %s", backup.c_str());
		}
		else {
			sprintf(message, "BACKUP: Writing %s", backup.c_str());
		}
		status_->misc_status(ST_NOTE, message);
		delete[] message;
		// In and out streams
		ifstream in(retrieve ? backup :source);
		in.seekg(0, in.end);
		int length = (int)in.tellg();
		const int increment = 8000;
		in.seekg(0, in.beg);
		status_->progress(length, OT_MAIN, "bytes");
		ofstream out(retrieve ? source : backup);
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
		status_->progress(length, OT_MAIN);
		in.close();
		out.close();
		if (!ok) {
			// Report error
			if (retrieve) {
				status_->misc_status(ST_ERROR, "RETRIEVE: failed");
			}
			else {
				status_->misc_status(ST_ERROR, "BACKUP: failed");
			}
		}
		else if (retrieve) {
			status_->misc_status(ST_OK, "RETRIEVE: Done");
		} else {
			status_->misc_status(ST_OK, "BACKUP: Done");
		}
	}
	else if (retrieve) {
		status_->misc_status(ST_WARNING, "RETRIEVE: Disabled");
	} else {
		status_->misc_status(ST_WARNING, "BACKUP: Disabled");
	}
}

// Add the current file to the recent files list
void set_recent_file(string filename) {

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

	menu_->add_recent_files();

}
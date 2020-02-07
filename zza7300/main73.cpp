#include "menu73.h"
#include "view73.h"
#include "../zzalib/rig_if.h"
#include "../zzalib/drawing.h"
#include "../zzalib/url_handler.h"

#include <FL/Fl_Output.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Tooltip.H>

using namespace zza7300;
using namespace zzalib;

Fl_Preferences* settings_ = nullptr;
menu73* menu_ = nullptr;
view73* view_ = nullptr;
rig_if* rig_if_ = nullptr;
Fl_Output* status_ = nullptr;
url_handler* url_handler_ = nullptr;

Fl_Single_Window* main_window_;

bool closing_ = false;


const string VENDOR = "GM3ZZA";
const string PROGRAM_ID = "ZZA7300";
const string PROGRAM_VERSION = "1.0.0";
const unsigned WIDTH = 650;
const unsigned HEIGHT = 450;

// This callback intercepts the close command and performs chacks and tidies up
// Updates recent files settings
static void cb_bn_close(Fl_Widget* w, void* v) {
	if (closing_) {
		status_->value("ZZALOG: Already closing!");
	}
	else {
		closing_ = true;
		status_->value("ZZALOG: Closing...");
		// If rig connected close it - this will close the timer as well
		if (rig_if_) {
			rig_if_->close();
		}

		// Save the window position
		Fl_Preferences window_settings(settings_, "Window");
		window_settings.set("Left", main_window_->x_root());
		window_settings.set("Top", main_window_->y_root());
		window_settings.set("Width", main_window_->w());
		window_settings.set("Height", main_window_->h());

		// Exit and close application
		Fl_Single_Window::default_callback((Fl_Window*)w, v);
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
	menu_ = new menu73(0, curr_y, WIDTH, MENU_HEIGHT);
	main_window_->add(menu_);
	curr_y += menu_->h();
	curr_y += status_->h();
	// The main views - this is a set of tabs with each view
	view_ = new view73(0, curr_y, WIDTH, HEIGHT - curr_y - TOOL_HEIGHT);
	main_window_->add(view_);
	// Status bar - tracks progress.
	status_ = new Fl_Output(0, curr_y, WIDTH, TOOL_HEIGHT);
	main_window_->add(status_);
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
	main_window_->resizable(view_);
	main_window_->size_range(100, 100);
	// Set the size to the setting or minimum
	main_window_->resize(left, top, max(100, width), max(100, height));
}


// Add some global properties
void add_properties() {
	// Default tooltip properties
	Fl_Tooltip::size(FONT_SIZE);
	Fl_Tooltip::font(FONT);
	Fl_Tooltip::delay((float)TIP_SHOW);
	// Default message properties
	fl_message_size_ = FONT_SIZE;
	fl_message_font_ = FONT;
	fl_message_title_default(PROGRAM_ID.c_str());
	// Default scrollbar
	Fl::scrollbar_size(10);
}

void cb_rig_timer() {
	status_->value(rig_if_->rig_info().c_str());
}

void cb_error_message(bool ok, const char* message) {
	status_->value(message);
}


// Create the rig interface handler and connect to the rig.
void add_rig_if() {
	if (!closing_) {
		fl_cursor(FL_CURSOR_WAIT);
		delete rig_if_;
			status_->value("Connecting to transceiver");
		// Create the appropriate interface handler
		// If the HTTP URL handler hasn't yet been created do so.
		if (url_handler_ == nullptr) url_handler_ = new url_handler;
		rig_if_ = new rig_flrig;
		// Set callbacks
		rig_if_->callback(cb_rig_timer, nullptr, cb_error_message);
		// Trya and open the connection to the rig
		bool done = false;
		while (!done) {
			if (rig_if_->open()) {
				if (!rig_if_->is_good()) {
					// No rig handler or rig didn't open - assume manual logging
					delete rig_if_;
					rig_if_ = nullptr;
				}
				else {
					// Access rig - timer will have been started by rig_if_->open()
					// Set rig timer callnack
					status_->value(rig_if_->success_message().c_str());
					done = true;
					// Change logging mode to ON_AIR
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
				status_->value(temp);
				done = true;
			}
		}
		fl_cursor(FL_CURSOR_DEFAULT);
	}
}


void tidy() {
	delete rig_if_;
	delete url_handler_;
	delete status_;
	delete view_;
	delete menu_;
	delete main_window_;
}

// The main app entry point
int main(int argc, char** argv)
{
	// Create the settings before anything else 
	settings_ = new Fl_Preferences(Fl_Preferences::USER, VENDOR.c_str(), PROGRAM_ID.c_str());
	// Create window
	// add_icon(argv[0]);
	create_window();
	add_properties();

	// add the various drawn items
	int curr_y = 0;
	add_widgets(curr_y);
	// Resize the window
	resize_window();
	// now show the window
	main_window_->show(argc, argv);

	// Connect to the rig
	rig_if_ = nullptr;
	add_rig_if();

	int code = 0;

	if (!closing_) {
		// Only do this if we haven't tried to close
		menu_->enable(true);
		menu_->redraw();
		// Run the application until it is closed
		code = Fl::run();
	}
	// Delete everything we've created
	tidy();
	return code;
}

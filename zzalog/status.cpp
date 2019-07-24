#include "status.h"

#include "utils.h"
#include "rig_if.h"
#include "callback.h"
#include "menu.h"

#include <FL/Fl_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>

using namespace zzalog;

extern Fl_Preferences* settings_;
extern rig_if* rig_if_;
extern menu* menu_;
extern void add_rig_if();
extern Fl_Single_Window* main_window_;
extern bool read_only_;
extern void add_sub_window(Fl_Window* w);
extern void remove_sub_window(Fl_Window* w);

// Default colours for file status
const map<file_status_t, Fl_Color> FILE_STATUS_COLOURS = {
	{ FS_EMPTY, FL_LIGHT2 },
	{ FS_SAVED, FL_GREEN },
	{ FS_MODIFIED, FL_RED },
	{ FS_LOADING, FL_YELLOW },
	{ FS_SAVING, FL_CYAN }
};


// Constructor
status::status(int X, int Y, int W, int H, const char * label) :
	Fl_Group(X, Y, W, H, label)
	, clock_bn_(nullptr)
	, progress_(nullptr)
	, rig_status_(nullptr)
	, misc_status_(nullptr)
	, file_status_(nullptr)
	, status_file_viewer_(nullptr)
	, use_local_(false)
	, max_progress_(0)
	,report_filename_("")
	, report_file_(nullptr)
	, progress_suffix_("")
	, min_level_(ST_NONE)

{
	// Initialise attributes
	max_progress_ = 0;

	// Current position for next widget
	int curr_x = X;
	// Add modified indicator
	const int file_w = H;
	// BUTTON - colour by file status
	file_status_ = new Fl_Button(curr_x, Y, file_w, H);
	file_status_->box(FL_DOWN_BOX);
	file_status_->color(FILE_STATUS_COLOURS.at(FS_EMPTY), FILE_STATUS_COLOURS.at(FS_EMPTY));
	file_status_->tooltip("File status:\n\tOff white - log empty\n\tYellow - loading file\n"
		"\tGreen - log saved to file\n\tRed - log modified\n\tCyan - saving file"
		"\n\tDark Green - read-only unmodified\n\tDark Red - read-only modified");
	add(file_status_);
	curr_x += file_w;
	const int rem_w = W - curr_x;

	// Add clock widget - use a button which when clicked switches between UTC and local time
	const int clock_w = rem_w / 4;
	// Button - display the clock
	clock_bn_ = new Fl_Button(curr_x, Y, clock_w, H);
	clock_bn_->labelsize(FONT_SIZE);
	clock_bn_->box(FL_DOWN_BOX);
	clock_bn_->color(FL_BLACK);
	clock_bn_->selection_color(FL_BLACK);
	clock_bn_->labelcolor(use_local_ ? FL_YELLOW : FL_GREEN);
	clock_bn_->labelfont(FL_HELVETICA_BOLD);
	// Get initial time 2018/12/02 14:13:00 UTC +0000, default to UTC at start-up.
	use_local_ = false;
	string clock_text = now(use_local_, "%F %T UTC %z");
	clock_bn_->copy_label(clock_text.c_str());
	// Set callback to switch between UTC and local timezone when clicked
	clock_bn_->callback(cb_bn_clock);
	clock_bn_->when(FL_WHEN_RELEASE);
	clock_bn_->tooltip("Current time (UTC)");
	// Set callback to receive an update every UTC_TIMER (1) second.
	Fl::add_timeout(UTC_TIMER, cb_timer_clock, (void*)clock_bn_);
	add(clock_bn_);

	// Add a progress bar widget - updated by all the processes which take a while
	curr_x += clock_w;
	const int progress_w = rem_w * 2 / 10;
	// Progress bar
	progress_ = new Fl_Progress(curr_x, Y, progress_w, H, "Initialising...");
	progress_->align(FL_ALIGN_INSIDE);
	progress_->labelsize(FONT_SIZE);
	progress_->tooltip("Activity progress");
	progress_->color(FL_WHITE, OBJECT_COLOURS.at(OT_NONE));
	add(progress_);
	curr_x += progress_w;

	// Add rig status
	const int rig_status_w = rem_w / 4;
	// Button - rig status
	rig_status_ = new Fl_Button(curr_x, Y, rig_status_w, H, "Rig connection not yet established");
	rig_status_->labelsize(FONT_SIZE);
	rig_status_->box(FL_DOWN_BOX);
	rig_status_->color(status_colours.at(ST_NONE));
	rig_status_->labelcolor(FL_BLACK);
	rig_status_->labelfont(FONT);
	rig_status_->copy_tooltip(rig_status_->label());
	// Add callback to try and reconnect the rig
	rig_status_->callback(cb_bn_rig);
	rig_status_->when(FL_WHEN_RELEASE);
	add(rig_status_);
	curr_x += rig_status_w;

	// Miscellaneous status - used for general status and error information
	const int misc_w = rem_w * 3 / 10;
	// Button - status message 
	misc_status_ = new Fl_Button(curr_x, Y, misc_w, H, "Miscellaneous information");
	misc_status_->labelsize(FONT_SIZE);
	misc_status_->box(FL_DOWN_BOX);
	misc_status_->color(status_colours.at(ST_NONE));
	misc_status_->labelcolor(FL_BLACK);
	misc_status_->labelfont(FONT);
	misc_status_->copy_tooltip(misc_status_->label());
	// Callback will open the status log report
	misc_status_->callback(cb_bn_misc);
	misc_status_->when(FL_WHEN_RELEASE);
	add(misc_status_);
	curr_x += misc_w;

	end();

	// Get report filename from the settings
	char * filename;
	Fl_Preferences status_settings(settings_, "Status");
	status_settings.get("Report File", filename, "");
	report_filename_ = filename;
	free(filename);
	// If it's not in the settings, open file dialog, get it and set it.
	while (report_filename_.length() == 0) {
		// Create an Open dialog; the default file name extension is ".txt".
		Fl_File_Chooser* chooser = new Fl_File_Chooser(report_filename_.c_str(), "Text Files(*.txt)\tAll Files (*.*)", Fl_File_Chooser::CREATE, "Select file name for status report");
		chooser->callback(cb_chooser, &report_filename_);
		chooser->textfont(FONT);
		chooser->textsize(FONT_SIZE);
		chooser->show();
		// Wait while the dialog is active (visible)
		while (chooser->visible()) Fl::wait();
		if (chooser->count()) {
			status_settings.set("Report File", report_filename_.c_str());
		}
		delete chooser;
	}

	// Try to open the file.
	report_file_ = new ofstream(report_filename_, ios::out | ios::app);
	if (!report_file_->good()) {
		delete report_file_;
		report_file_ = nullptr;
		char * message = new char[report_filename_.length() + 50];
		sprintf(message, "Failed to open status report file %s", report_filename_.c_str());
		misc_status(ST_ERROR, message);
	}

	status_settings.get("Minimum Level", (int&)min_level_, ST_NOTE);
	menu_->status_level(min_level_);

	status_file_viewer_ = nullptr;
}

// Destructor
status::~status()
{
	clear();
	report_file_->close();
}

// Callbacks
// Clock button callback - toggles between UTC and local-time
void status::cb_bn_clock(Fl_Widget* bn, void* v) {
	status* that = ancestor_view<status>(bn);
	// Toggle the flag 
	that->use_local_ = that->use_local_ ? false : true;
	// Set label-colour to indicate local or UTC
	bn->labelcolor(that->use_local_ ? FL_YELLOW : FL_GREEN);
	// Redisplay the time
	string clock_text = now(that->use_local_, "%F %T UTC %z");
	bn->copy_label(clock_text.c_str());
	if (that->use_local_) {
		string tz = now(true, "Current time (UTC %z)");
		bn->copy_tooltip(tz.c_str());
	}
	else {
		bn->tooltip("Current time (UTC)");
	}
	bn->redraw();
}

// Clock timer callback - received every UTC_TIMER seconds
void status::cb_timer_clock(void * v) {
	// Update the label in the clock button which is passed as the parameter
	Fl_Button* bn = (Fl_Button*)v;
	status* that = ancestor_view<status>(bn);
	string clock_text = now(that->use_local_, "%F %T UTC %z");
	bn->copy_label(clock_text.c_str());
	bn->redraw();
	// repeat the timer
	Fl::repeat_timeout(UTC_TIMER, cb_timer_clock, v);
}

// Rig status bn callback - attempts to toggle rig connection state
void status::cb_bn_rig(Fl_Widget* bn, void* v) {
	status* that = ancestor_view<status>(bn);

	if (rig_if_ == nullptr || !rig_if_->is_good() || !rig_if_->is_open()) {
		// Rig wasn't present or hadn't connected or had crashed, so try again
		add_rig_if();
	}
	else {
		// Close the rig
		rig_if_->close();
		that->rig_status(ST_WARNING, "Rig closed - assume off-air logging");
		that->misc_status(ST_WARNING, "RIG: Closing reig connection");
	}
	bn->redraw();
}

// Intialise progress bar - 
// maximum value, purpose (used to set colour) and items counted
void status::progress(int max_value, object_t object, const char* suffix) {
	// Initialise it
	max_progress_ = max_value;
	progress_suffix_ = suffix;
	Fl_Color bar_colour = OBJECT_COLOURS.at(object);
	// Set colours
	progress_->color(OBJECT_COLOURS.at(OT_NONE), bar_colour);
	// display text in a colour that contrasts the bar colour
	progress_->labelcolor(fl_contrast(FL_BLACK, bar_colour));
	char label[100];
	sprintf(label, "0/%d %s", max_progress_, progress_suffix_.c_str());
	progress_->copy_label(label);
	// Set range (0:max_value)
	progress_->minimum(0.0);
	progress_->maximum((float)max_value);
	// redraw and allow scheduler to effect the redrawing
	progress_->redraw();
	Fl::wait();
}

// Update progress bar
void status::progress(int value) {
	// Update progress 
	progress_->value((float)value);
	// Add label "N/M things"
	char label[100];
	sprintf(label, "%d/%d %s", value, max_progress_, progress_suffix_.c_str());
	progress_->copy_label(label);
	// redraw and allow scheduler to effect the redrawing
	progress_->redraw();
	Fl::wait();
}

// Update rig_status - set text and colour
void status::rig_status(status_t status, const char* label) {
	// Set the fixed label and tool tip value. Set the text colour to contrast the button colour
	Fl_Color colour = status_colours.at(status);
	rig_status_->copy_label(label);
	rig_status_->color(colour);
	rig_status_->labelcolor(fl_contrast(FL_BLACK, colour));
	rig_status_->copy_tooltip(label);
	rig_status_->redraw();
	Fl::wait();
}

const map<status_t, string> STATUS_CODES = {
	{ ST_NONE, "   "},
	{ ST_LOG, " L "},
	{ ST_NOTE, " N "},
	{ ST_OK, " D "},
	{ ST_WARNING, "+W+"},
	{ ST_ERROR, "*E*"},
	{ ST_SEVERE, "*S*"},
	{ ST_FATAL, "*F*"}
};

// Update miscellaneous status - set text and colour
void status::misc_status(status_t status, const char* label) {
	// If we are displaying the message
	if ((int)status >= (int)min_level_ ) {
		// Set the fixed label and tool tip value. Set the text colour to contrast the button colour
		Fl_Color colour = status_colours.at(status);
		misc_status_->copy_label(label);
		misc_status_->color(colour);
		misc_status_->labelcolor(fl_contrast(FL_BLACK, colour));
		misc_status_->copy_tooltip(label);
		misc_status_->redraw();

		Fl::wait();
	}

#ifdef _DEBUG
	// Add a marker to the memory leak report to indicate where we are - deliberately
	// create a leak with the first 15 chars of label in
	char* marker = new char[16];
	strncpy(marker, label, 15);
#endif


	// Append the status to the file
	if (report_file_) {
		string timestamp = now(true, "%Y/%m/%d %H:%M:%S");
		char* message = new char[timestamp.length() + 10 + strlen(label)];
		sprintf(message, "%s %s %s\n", timestamp.c_str(), STATUS_CODES.at(status).c_str(), label);
		*report_file_ << message;
		delete[] message;
	}

	// Fatal error - ask user to continue
	switch(status) {
	case ST_SEVERE:
		if (fl_choice("An error that resulted in reduced functionality occurred:\n%s\n\nDo you want to try to continue or quit?", "Continue", "Quit", nullptr, label, report_filename_.c_str()) == 1) {
			// Call the exit handler
			Fl::wait();
			// Open status file viewer and update it.
			cb_bn_misc(misc_status_, nullptr);
			main_window_->do_callback();
		}
	case ST_FATAL:
		fl_message("An unrecoverable error has occurred, closing down - check status log");
		Fl::wait();
		// Open status file viewer and update it : remove from list to be deleted when closing
		cb_bn_misc(misc_status_, nullptr);
		remove_sub_window(status_file_viewer_->top_window());

		main_window_->do_callback();
	}
}

// Update file status
void status::file_status(file_status_t status) {
	Fl_Color colour;
	if (read_only_) {
		colour = fl_darker(FILE_STATUS_COLOURS.at(status));
	}
	else {
		colour = FILE_STATUS_COLOURS.at(status);
	}
	file_status_->color(colour, colour);
	file_status_->redraw();
	Fl::wait();
}

// Text display constructor
text_display::text_display(int X, int Y, int W, int H) :
	Fl_Text_Display(X, Y, W, H)
{
}

// Reload the file keeping the scroll position fixed - default was to start at line 0 again.
void text_display::reload(const char* filename) {
	// Find current scroll position
	int scroll_pos = mVScrollBar->value();
	// Reload the file
	buffer()->loadfile(filename);
	// Restore original scroll position
	scroll(scroll_pos, 0);
}

// Callback opens a text browser
void status::cb_bn_misc(Fl_Widget* w, void* v) {
	status* that = ancestor_view<status>(w);
	// Ensure any outstanding writes are flushed to disc
	that->report_file_->flush();
	if (that->status_file_viewer_) {
		// Reload the viewer and force the window to be shown - it may have been closed
		that->status_file_viewer_->reload(that->report_filename_.c_str());
		that->status_file_viewer_->window()->show();
	}
	else {
		// Now read it into the text buffer
		Fl_Text_Buffer* buffer = new Fl_Text_Buffer;
		// And display it
		char * title = new char[that->report_filename_.length() + 30];
		sprintf(title, "Status report file: %s", that->report_filename_.c_str());
		Fl_Window* win = new Fl_Window(640, 480);
		win->copy_label(title);
		delete[] title;
		// Create the text display and set its parameters
		that->status_file_viewer_ = new text_display(10, 10, 640 - 20, 480 - 20);
		that->status_file_viewer_->buffer(buffer);
		that->status_file_viewer_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
		that->status_file_viewer_->textfont(FL_COURIER);
		that->status_file_viewer_->textsize(12);
		that->status_file_viewer_->buffer()->loadfile(that->report_filename_.c_str());
		win->resizable(that->status_file_viewer_);
		win->show();
		win->end();
		// Add the display to the main window to delete it if the main window is first.
		add_sub_window(win);
	}
}

Fl_Widget* status::misc_status() {
	return misc_status_;
}

void status::min_level(status_t level) {
	min_level_ = level;
	Fl_Preferences status_settings(settings_, "Status");
	status_settings.set("Minimum Level", min_level_);
}

// Status log display is closed
void status::cb_text(Fl_Widget* w, void* v) {
	remove_sub_window((Fl_Window*)w);
}

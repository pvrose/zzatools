#ifndef __STATUS__
#define __STATUS__

#include "drawing.h"
#include "utils.h"

#include <map>
#include <string>
#include <fstream>
#include <list>
#include <vector>

using namespace std;


	// // The status of the file
	// enum file_status_t {
	// 	FS_EMPTY,            // The log is empty
	// 	FS_SAVED,            // The log is the same as the file
	// 	FS_MODIFIED,         // The log differs from the file
	// 	FS_LOADING,          // The log is being loaded
	// 	FS_SAVING            // The log is being saved
	// };


	// Default colours for status bars
	struct colours_t {
		Fl_Color fg;
		Fl_Color bg;
	};
	const map<status_t, colours_t> STATUS_COLOURS = {
		{ ST_NONE, { FL_LIGHT2, FL_BLACK } },
		{ ST_LOG, { FL_WHITE, FL_BLACK } },
		{ ST_DEBUG, { fl_lighter(FL_MAGENTA), FL_BLACK } },
		{ ST_NOTE, { fl_lighter(FL_CYAN), FL_BLACK } },
		{ ST_OK, { fl_lighter(FL_GREEN), FL_BLACK } },
		{ ST_WARNING, { FL_YELLOW, FL_BLACK } },
		{ ST_NOTIFY, { fl_darker(FL_YELLOW), FL_BLACK } },
		{ ST_ERROR, { FL_RED, FL_BLACK } },
		{ ST_SEVERE, { FL_RED, FL_WHITE } },
		{ ST_FATAL, { FL_BLACK, FL_RED } }
	};

	// Code - letters witten to log file to indicate severity of the logged status
	const map<status_t, char> STATUS_CODES = {
		{ ST_NONE, ' '},
		{ ST_LOG, 'L'},
		{ ST_DEBUG, 'B' },
		{ ST_NOTE, 'N'},
		{ ST_OK, 'D'},
		{ ST_WARNING, 'W'},
		{ ST_NOTIFY, 'Y'},
		{ ST_ERROR, 'E'},
		{ ST_SEVERE, 'S'},
		{ ST_FATAL, 'F'}
	};

	// Description for filter
	const map<status_t, string> STATUS_TEXTS = {
		{ ST_NONE, "All"},
		{ ST_LOG, "Information"},
		{ ST_DEBUG, "Debug info"},
		{ ST_NOTE, "Action starting"},
		{ ST_OK, "Action complete"},
		{ ST_WARNING, "Warnings"},
		{ ST_NOTIFY, "Notifications"},
		{ ST_ERROR, "Recoverable errors"},
		{ ST_SEVERE, "Severe errors"},
		{ ST_FATAL, "Irrecoverable errors"}
	};
	// // Styles used by Fl_Text_Display to control font and colour 
	// const Fl_Text_Display::Style_Table_Entry STATUS_STYLES[] = {        // Style label and status
	// 	{ COLOUR_GREY, FL_COURIER_ITALIC, 12, 0 },                  // A - ST_NONE
	// 	{ COLOUR_GREY, FL_COURIER, 12, 0 },                         // B - ST_LOG
	// 	{ FL_MAGENTA, FL_COURIER, 12, 0 },                              // C - ST_DEBUG
	// 	{ FL_FOREGROUND_COLOR, FL_COURIER, 12, 0 },                                // D - ST_NOTE
	// 	{ fl_darker(FL_GREEN), FL_COURIER, 12, 0},                      // E - ST_OK
	// 	{ fl_color_average(FL_RED, FL_YELLOW, 0.5), FL_COURIER, 12, 0}, // G - ST_WARNING
	// 	{ fl_darker(FL_YELLOW), FL_COURIER, 12, 0},                     // H - ST_NOTIFY
	// 	{ FL_RED, FL_COURIER, 12, 0} ,                                  // I - ST_ERROR
	// 	{ fl_darker(FL_RED), FL_COURIER, 12, 0 },                       // J - ST_SEVERE
	// 	{ fl_darker(FL_RED), FL_COURIER_BOLD, 12, 0}                    // K - ST_FATAL
	// };

	// // Default colours for file status
	// const map<file_status_t, Fl_Color> FILE_STATUS_COLOURS = {
	// 	{ FS_EMPTY, FL_LIGHT2 },
	// 	{ FS_SAVED, FL_GREEN },
	// 	{ FS_MODIFIED, FL_RED },
	// 	{ FS_LOADING, FL_YELLOW },
	// 	{ FS_SAVING, FL_CYAN }
	// };

	// 
	enum object_t;

	// // This class extends Fl_Text_Buffer by switching off UTF-8 warnin
	// class text_buffer : public Fl_Text_Buffer {
	// public:
	// 	text_buffer(int requestedSize = 0, int preferredGapSize = 1024);
	// 	~text_buffer();
	// };

	// // This class provides an extension to the Text Display widget to allow the buffer to be reloaded
	// class text_display : public Fl_Text_Display {
	// public:
	// 	text_display(int X, int Y, int W, int H, const char* label = nullptr);
	// 	~text_display();

	// 	void append(const char* line);

	// 	// Text filter
	// 	string filter_;
	// 	// Status filter
	// 	status_t status_filter_;


	// };

	// // This class provides the window in which to display the status log
	// class viewer_window : public Fl_Window {
	// public:
	// 	viewer_window(int W, int Y, const char* label = nullptr);
	// 	~viewer_window();

	// 	virtual int handle(int event);
	// 	// Add a line to the window
	// 	void append(const char* line);
	// 	// Draw the window
	// 	void draw_window();
	// 	// Apply colours to the various levels of message
	// 	void colour_buffer();

	// protected:
	// 	// Find text in Text Buffer
	// 	static void cb_find(Fl_Widget* w, void* v);
	// 	// Filter choice button
	// 	static void cb_ch_filter(Fl_Widget* w, void* v);
	// 	// Status filter 
	// 	static void cb_ch_status(Fl_Widget* w, void* v);

	// 	// Display widget
	// 	text_display* display_;
	// 	// Find search string
	// 	string search_;
	// 	// Search direction
	// 	int direction_;
	// 	// Whether to match case
	// 	int match_case_;
	// 	// The raw data - for restoring to buffer
	// 	vector<const char*> original_lines_;
	// };

	// A progress stack entry
	struct progress_item {
		int max_value;
		int value;
		char* description;
		char* suffix;
		bool countdown;
		int prev_value;
	};

	// This class provides the status bar and manages access to the various status information
	class status 
	{
	public:

		status();
		~status();

		// Initialise progress
		void progress(int max_value, object_t object, const char* description, const char* suffix, bool countdown = false);
		// UPdate progress
		void progress(int value, object_t object);
		// Update progress with a text message and mark 100%
		void progress(const char* message, object_t object);
		// Update miscellaneous status
		void misc_status(status_t status, const char* label);
		// // Update file status
		// void file_status(file_status_t status);
		// return misc_status widget
		// Fl_Widget* misc_status();
		// Set minimum status reporting level
		// void min_level(status_t);
		// status_t min_level();
		// Append or overwrite status log
		// void append_log(bool append);
		// Set file viewer to nullptr
		// void null_file_viewer();
		// Get status_file_viewer
		// Fl_Window* file_viewer();
		// Debug display
		// void display_debug(bool value);
		// bool display_debug();
		// One second ticker
		void ticker();

		// Callbacks
		// Rig button callback
		// static void cb_bn_rig(Fl_Widget* bn, void* v);
		// Misc button callback
		// static void cb_bn_misc(Fl_Widget* bn, void* v);
		// File viewer close callback
		// static void cb_fv_close(Fl_Widget* w, void* v);


	protected:
		// Re-initialise progress bar
		void update_progress(object_t object);

		// Colour code
		string colour_code(status_t status, bool fg); 


	protected:
		// Progress bar - shows progress during activities that take noticeable time
		// Fl_Progress* progress_;
		// Rig status - when a rig connected regularly update from rig
		// Fl_Button* rig_status_;
		// Miscellaneous status - used to display messages from the aplication
		// Fl_Button* misc_status_;
		// File status
		// Fl_Button* file_status_;
		// Status file viewer
		// viewer_window* status_file_viewer_;
		// Containing window

		// Status report file
		string report_filename_;
		ofstream* report_file_;
		// // Minimum reporting level
		// status_t min_level_;
		// // Append log
		// bool append_log_;
		// // Flag to prevent double clicking of rig button
		// bool rig_in_progress_;
		// The progress item stack
		list<object_t> progress_stack_;
		// The progress items in the stack
		map<object_t, progress_item*> progress_items_;
		// Report file unusable
		bool file_unusable_;
		// // Do not update file viewer
		// bool no_update_viewer;
		// // Display debug status lines
		// bool display_debug_messages_;

	};
#endif


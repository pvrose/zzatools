#ifndef __STATUS__
#define __STATUS__

#include "drawing.h"
#include "../zzalib/utils.h"

#include <map>
#include <string>
#include <fstream>
#include <list>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// The status of the file
	enum file_status_t {
		FS_EMPTY,            // The log is empty
		FS_SAVED,            // The log is the same as the file
		FS_MODIFIED,         // The log differs from the file
		FS_LOADING,          // The log is being loaded
		FS_SAVING            // The log is being saved
	};

	// The status of the rig
	enum rig_status_t {
		RS_OFF,              // The rig is off or disconnected
		RS_ERROR,            // An error with the rig occured
		RS_RX,               // The rig is connected and in RX mode
		RS_TX                // The rig is connected and in TX mode
	};

	// Default colours for status bars
	const map<status_t, Fl_Color> STATUS_COLOURS = {
		{ ST_NONE, FL_LIGHT2 },
		{ ST_LOG, FL_GRAY },
		{ ST_NOTE, fl_lighter(FL_CYAN) },
		{ ST_OK, fl_lighter(FL_GREEN) },
		{ ST_WARNING, FL_YELLOW },
		{ ST_ERROR, fl_lighter(FL_RED) },
		{ ST_SEVERE, fl_darker(FL_RED) },
		{ ST_FATAL, FL_BLACK }
	};

	// Code - letters witten to log file to indicate severity of the logged status
	const map<status_t, char> STATUS_CODES = {
	{ ST_NONE, ' '},
	{ ST_LOG, 'L'},
	{ ST_NOTE, 'N'},
	{ ST_OK, 'D'},
	{ ST_WARNING, 'W'},
	{ ST_ERROR, 'E'},
	{ ST_SEVERE, 'S'},
	{ ST_FATAL, 'F'}
	};

	// Styles used by Fl_Text_Display to control font and colour 
	const Fl_Text_Display::Style_Table_Entry STATUS_STYLES[] = {        // Style label and status
		{ fl_gray_ramp(4), FL_COURIER_ITALIC, 12, 0 },                  // A - ST_NONE
		{ fl_gray_ramp(4), FL_COURIER, 12, 0 },                         // B - ST_LOG
		{ FL_BLACK, FL_COURIER, 12, 0 },                                // C - ST_NOTE
		{ fl_darker(FL_GREEN), FL_COURIER, 12, 0},                      // D - ST_OK
		{ fl_color_average(FL_RED, FL_YELLOW, 0.5), FL_COURIER, 12, 0}, // E - ST_WARNING
		{ FL_RED, FL_COURIER, 12, 0} ,                                  // F - ST_ERROR
		{ fl_darker(FL_RED), FL_COURIER, 12, 0 },                       // G - ST_SEVERE
		{ fl_darker(FL_RED), FL_COURIER_BOLD, 12, 0}                    // H - ST_FATAL
	};

	// Default colours for file status
	const map<file_status_t, Fl_Color> FILE_STATUS_COLOURS = {
		{ FS_EMPTY, FL_LIGHT2 },
		{ FS_SAVED, FL_GREEN },
		{ FS_MODIFIED, FL_RED },
		{ FS_LOADING, FL_YELLOW },
		{ FS_SAVING, FL_CYAN }
	};

	// Default colours for rig status
	const map<rig_status_t, Fl_Color> RIG_STATUS_COLOURS = {
		{ RS_OFF, FL_YELLOW },
		{ RS_ERROR, fl_color_average(FL_RED, FL_YELLOW, 0.5)},
		{ RS_RX, FL_GREEN },
		{ RS_TX, FL_RED }
	};

	// 
	enum object_t;

	// clock display - 1s.
	const double UTC_TIMER = 1.0;

	// This class extends Fl_Text_Buffer by switching off UTF-8 warnin
	class text_buffer : public Fl_Text_Buffer {
	public:
		text_buffer(int requestedSize = 0, int preferredGapSize = 1024);
		~text_buffer();
	};

	// This class provides an extension to the Text Display widget to allow the buffer to be reloaded
	class text_display : public Fl_Text_Display {
	public:
		text_display(int X, int Y, int W, int H, const char* label = nullptr);
		~text_display();

		void append(const char* line);

		// Text filter
		string filter_;


	};

	class viewer_window : public Fl_Window {
	public:
		viewer_window(int W, int Y, const char* label);
		~viewer_window();

		void append(const char* line);

		void draw_window();

		void colour_buffer();

	protected:
		// Find text in Text Buffer
		static void cb_find(Fl_Widget* w, void* v);
		// Filter choice button
		static void cb_ch_filter(Fl_Widget* w, void* v);

		// Display widget
		text_display* display_;
		// Find search string
		string search_;
		// Search direction
		int direction_;
		// Whether to match case
		int match_case_;
		// The raw data - for restoring to buffer
		vector<const char*> original_lines_;
	};

	// A progress stack entry
	struct progress_item {
		int max_value;
		int value;
		char* suffix;
		bool countdown;
	};

	// This class provides the status bar and manages access to the various status information
	class status : public Fl_Group
	{
	public:

		status(int X, int Y, int W, int H, const char * label = 0);
		~status();

		// Initialise progress
		void progress(int max_value, object_t object, const char* suffix, bool countdown = false);
		// UPdate progress
		void progress(int value, object_t object);
		// Update progress with a text message and mark 100%
		void progress(const char* message, object_t object);
		// Update rig_status
		void rig_status(rig_status_t status, const char* label);
		// Update miscellaneous status
		void misc_status(status_t status, const char* label);
		// Update file status
		void file_status(file_status_t status);
		// return misc_status widget
		Fl_Widget* misc_status();
		// Set minimum status reporting level
		void min_level(status_t);
		status_t min_level();
		// Append or overwrite status log
		void append_log(bool append);
		// Set file viewer to nullptr
		void null_file_viewer();
		// Get status_file_viewer
		Fl_Window* file_viewer();

		// Callbacks
		// Clock button callback
		static void cb_bn_clock(Fl_Widget* bn, void* v);
		// Clock timer callback
		static void cb_timer_clock(void* v);
		// Rig button callback
		static void cb_bn_rig(Fl_Widget* bn, void* v);
		// Misc button callback
		static void cb_bn_misc(Fl_Widget* bn, void* v);
		// File viewer close callback
		static void cb_fv_close(Fl_Widget* w, void* v);


	protected:
		// Re-initialise progress bar
		void update_progress(object_t object);

	protected:
		// The clock display
		Fl_Button * clock_bn_;
		// Progress bar - shows progress during activities that take noticeable time
		Fl_Progress* progress_;
		// Rig status - when a rig connected regularly update from rig
		Fl_Button* rig_status_;
		// Miscellaneous status - used to display messages from the aplication
		Fl_Button* misc_status_;
		// File status
		Fl_Button* file_status_;
		// Status file viewer
		viewer_window* status_file_viewer_;
		// Containing window

		// local or UTC
		bool use_local_;
		// Status report file
		string report_filename_;
		ofstream* report_file_;
		// Minimum reporting level
		status_t min_level_;
		// Append log
		bool append_log_;
		// Flag to prevent double clicking of rig button
		bool rig_in_progress_;
		// The progress item stack
		list<object_t> progress_stack_;
		// The progress items in the stack
		map<object_t, progress_item*> progress_items_;
		// Report file unusable
		bool file_unusable_;
		// Previous reported progress
		int prev_progress_;
		// Do not update file viewer
		bool no_update_viewer;

	};

}
#endif


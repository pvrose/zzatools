#ifndef __STATUS__
#define __STATUS__

#include "drawing.h"

#include <map>
#include <string>
#include <fstream>

#include <FL/Fl_Group.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>

using namespace std;

namespace zzalog {

	// The status of the various messages
	enum status_t {
		ST_NONE,             // Uninitialised
		ST_LOG,              // Only log the message, do not display it in status
		ST_NOTE,             // An information message
		ST_OK,               // Task successful
		ST_WARNING,          // A warning message
		ST_ERROR,            // An error has been signaled
		ST_SEVERE,           // A sever error that will result in reduced capability
		ST_FATAL             // A fatal (non-recoverable) error has been signaled
	};

	// The status of the file
	enum file_status_t {
		FS_EMPTY,            // The log is empty
		FS_SAVED,            // The log is the same as the file
		FS_MODIFIED,         // The log differs from the file
		FS_LOADING,          // The log is being loaded
		FS_SAVING            // The log is being saved
	};

	// Default colours for status bars
	const map<status_t, Fl_Color> status_colours = {
		{ ST_NONE, FL_LIGHT2 },
		{ ST_NOTE, fl_lighter(FL_CYAN) },
		{ ST_OK, fl_lighter(FL_GREEN) },
		{ ST_WARNING, FL_YELLOW },
		{ ST_ERROR, fl_lighter(FL_RED) },
		{ ST_SEVERE, fl_darker(FL_RED) },
		{ ST_FATAL, FL_BLACK }
	};


	enum object_t;

	// clock display - 1s.
	const double UTC_TIMER = 1.0;

	// This class provides an extension to the Text Display widget to allow the buffer to be reloaded
	class text_display : public Fl_Text_Display {
	public:
		text_display(int X, int Y, int W, int H);

		void reload(const char* filename);
	};

	// This class provides the status bar and manages access to the various status information
	class status : public Fl_Group
	{
	public:

		status(int X, int Y, int W, int H, const char * label = 0);
		~status();

		// Initialise progress
		void progress(int max_value, object_t object, const char* suffix);
		// UPdate progress
		void progress(int value);
		// Update rig_status
		void rig_status(status_t status, const char* label);
		// Update miscellaneous status
		void misc_status(status_t status, const char* label);
		// Update file status
		void file_status(file_status_t status);
		// return misc_status widget
		Fl_Widget* misc_status();
		// Set minimum status reporting level
		void min_level(status_t);

		// Callbacks
		// Clock button callback
		static void cb_bn_clock(Fl_Widget* bn, void* v);
		// Clock timer callback
		static void cb_timer_clock(void* v);
		// Rig button callback
		static void cb_bn_rig(Fl_Widget* bn, void* v);
		// Misc button callback
		static void cb_bn_misc(Fl_Widget* bn, void* v);
		// Status log display is closed
		static void cb_text(Fl_Widget* w, void* v);

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
		text_display* status_file_viewer_;
		// local or UTC
		bool use_local_;
		// Max progres value
		int max_progress_;
		// Status report file
		string report_filename_;
		ofstream* report_file_;
		// Progress "label"
		string progress_suffix_;
		// Minimum reporting level
		status_t min_level_;

	};

}
#endif


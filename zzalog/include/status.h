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

	// Default colours for status bars
	struct colours_t {
		Fl_Color fg;
		Fl_Color bg;
	};
	const map<status_t, colours_t> STATUS_COLOURS = {
		{ ST_NONE, { FL_BLUE, FL_BLACK } },
		{ ST_LOG, { fl_lighter(FL_BLUE), FL_BLACK } },
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

	// 
	enum object_t;

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
		// 200 ms ticker
		void ticker();

	protected:
		// Re-initialise progress bar
		void update_progress(object_t object);

		// Colour code
		string colour_code(status_t status, bool fg); 


	protected:
		// Status report file
		string report_filename_;
		ofstream* report_file_;
		// The progress item stack
		list<object_t> progress_stack_;
		// The progress items in the stack
		map<object_t, progress_item*> progress_items_;
		// Report file unusable
		bool file_unusable_;
		// Previous progress value
		int previous_value_;
	};
#endif


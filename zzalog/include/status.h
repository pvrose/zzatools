#ifndef __STATUS__
#define __STATUS__

#include "drawing.h"
#include "utils.h"

#include <map>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <cstdint>

using namespace std;

class banner;

	// Default colours for status bars
	struct colours_t {
		Fl_Color fg;
		Fl_Color bg;
	};

	// The status of the various messages
	enum status_t : char {
		ST_NONE,             // Uninitialised
		ST_LOG,              // Only log the message, do not display it in status
		ST_DEBUG,            // Debug message
		ST_NOTE,             // An information message
		ST_PROGRESS,         // A progress note
		ST_OK,               // Task successful
		ST_WARNING,          // A warning message
		ST_ERROR,            // An error has been signaled
		ST_SEVERE,           // A sever error that will result in reduced capability
		ST_FATAL             // A fatal (non-recoverable) error has been signaled
	};

	const map<status_t, colours_t> STATUS_COLOURS = {
		{ ST_NONE, { FL_BLUE, FL_BLACK } },
		{ ST_LOG, { fl_lighter(FL_BLUE), FL_BLACK } },
		{ ST_DEBUG, { fl_lighter(FL_MAGENTA), FL_BLACK } },
		{ ST_NOTE, { fl_lighter(FL_CYAN), FL_BLACK } },
		{ ST_PROGRESS, { fl_darker(FL_WHITE), FL_BLACK } },
		{ ST_OK, { fl_lighter(FL_GREEN), FL_BLACK } },
		{ ST_WARNING, { FL_YELLOW, FL_BLACK } },
		{ ST_ERROR, { FL_RED, FL_BLACK } },
		{ ST_SEVERE, { FL_RED, FL_WHITE } },
		{ ST_FATAL, { FL_BLACK, FL_RED } },
	};

	// Code - letters witten to log file to indicate severity of the logged status
	const map<status_t, char> STATUS_CODES = {
		{ ST_NONE, ' '},
		{ ST_LOG, 'L'},
		{ ST_DEBUG, 'B' },
		{ ST_NOTE, 'N'},
		{ ST_PROGRESS, 'P'},
		{ ST_OK, 'D'},
		{ ST_WARNING, 'W'},
		{ ST_ERROR, 'E'},
		{ ST_SEVERE, 'S'},
		{ ST_FATAL, 'F'}
	};

	// Abbreviations
	const map < status_t, const char* > STATUS_ABBREV = {
		{ ST_NONE, "    "},
		{ ST_LOG, " LOG "},
		{ ST_DEBUG, " DBUG"},
		{ ST_NOTE, " NOTE"},
		{ ST_PROGRESS, " PROG"},
		{ ST_OK, " OK  "},
		{ ST_WARNING, "?WARN"},
		{ ST_ERROR, "*ERR*"},
		{ ST_SEVERE, "!SVR!"},
		{ ST_FATAL, "!FTL!"}
	};

	// 
	enum object_t : char;

	// This class provides the status bar and manages access to the various status information
	class status 
	{
	public:

		status();
		~status();

		// Initialise progress
		void progress(uint64_t max_value, object_t object, const char* description, const char* suffix);
		// UPdate progress
		void progress(uint64_t value, object_t object);
		// Update progress with a text message and mark 100%
		void progress(const char* message, object_t object);
		// Update miscellaneous status
		void misc_status(status_t status, const char* label);

	protected:
		// Colour code
		string colour_code(status_t status, bool fg); 


	protected:
		// Status report file
		string report_filename_;
		ofstream* report_file_;
		// Report file unusable
		bool file_unusable_;
	};
#endif


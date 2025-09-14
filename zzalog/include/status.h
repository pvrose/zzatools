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



class banner;

	//! The colours used for a particular status_t value.
	struct colours_t {
		Fl_Color fg;
		Fl_Color bg;
	};

	//! The status of the various messages
	enum status_t : char {
		ST_NONE,             //!< Uninitialised
		ST_LOG,              //!< Only log the message, do not display it in status
		ST_DEBUG,            //!< Debug message
		ST_NOTE,             //!< An information message
		ST_PROGRESS,         //!< A progress note
		ST_OK,               //!< Task successful
		ST_WARNING,          //!< A warning message
		ST_ERROR,            //!, An error has been signaled
		ST_SEVERE,           //!< A sever error that will result in reduced capability
		ST_FATAL             //!< A fatal (non-recoverable) error has been signaled
	};

	//! MAp the values of status_t to the colours used to display them.
	const std::map<status_t, colours_t> STATUS_COLOURS = {
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

	//! Code - letters witten to log file to indicate severity of the logged status
	const std::map<status_t, char> STATUS_CODES = {
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

	// 
	enum object_t : char;

	//! This class provides the means of managing status and progress for ZZALOG.
	class status 
	{
	public:

		//! Constructor.
		status();
		//! Destructor.
		~status();

		//! Initialise progress
		
		//! \param max_value Maximum value of items being used to monitor progress.
		//! \param object An identifier for the what is being measured
		//! \param description Textual description of what is being measured.
		//! \param suffix Indicates units of items being measured.
		void progress(uint64_t max_value, object_t object, const char* description, const char* suffix);
		//! Update progress
		
		//! \param value Current number of items being measured.
		//! \param object Identifier.
		void progress(uint64_t value, object_t object);
		//! Update progress with a text message and mark complete.
		
		//! \param message Message indicating why it is complete.
		//! \param object Identifier.
		void progress(const char* message, object_t object);
		//! Output message \p label with \p status.
		void misc_status(status_t status, const char* label);

	protected:
		//! Returns the terminal characters used for the specific \p fg or bg colour for \p status.
		std::string colour_code(status_t status, bool fg); 


	protected:
		//! Status report file
		std::string report_filename_;
		//! Output stream for report file.
		std::ofstream* report_file_;
		//! Report file unusable
		bool file_unusable_;
	};
#endif


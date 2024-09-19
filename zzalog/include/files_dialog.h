#ifndef __FILES_DIALOG__
#define __FILES_DIALOG__

#include "page_dialog.h"
#include "callback.h"

#include <string>

using namespace std;

	// This class provides the dialog used to enable the user to select file locations
	// for the various operations and save them in the settings.
	class files_dialog : public page_dialog
	{
	public:
		files_dialog(int X, int Y, int W, int H, const char* label);
		~files_dialog();

	protected:
		// Load the settings data
		virtual void load_values();
		// create the form
		virtual void create_form(int X, int Y);
		// update or initialise the form
		virtual void save_values();
		// Enable widgets
		virtual void enable_widgets();

		// attributes
		// Enable TQSL executable
		bool enable_tqsl_;
		// Enable eQSL cards
		bool enable_card_;
		// Enable auto-backup
		bool enable_backup_;
		// The TQSL executable
		string tqsl_executable_;
		// eQSL e-card directory
		string card_directory_;
		// Reference data (prefix database and ADIF specification database)
		string ref_data_directory_;
		// Back up directory
		string backup_directory_;
		// Status log file
		string status_log_file_;
		// All.txt file
		string wsjtx_directory_;
		// Unzipper location
		string unzipper_;
		// Unzipper switches
		string unzip_switches_;
		// Stations callsign
		string station_callsign_;
	};
#endif
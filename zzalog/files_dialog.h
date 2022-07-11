#ifndef __FILES_DIALOG__
#define __FILES_DIALOG__

#include "../zzalib/page_dialog.h"
#include "intl_widgets.h"

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Check_Button.H>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// This class provides the dialog used to enable the user to select file locations
	// for the various operations and save them in the settings.
	class files_dialog : public page_dialog
	{
	public:
		// Default number of auto-import files
		static const int AUTO_COUNT = 3;

	public:
		files_dialog(int X, int Y, int W, int H, const char* label);
		~files_dialog();

	protected:
		// Special version of cb_value to set auto_changed_
		template <class WIDGET, class DATA>
		static void cb_value_auto(Fl_Widget* w, void* v);
		// Callback to displat QSL label dimensions dialog
		static void cb_bn_qslt(Fl_Widget* w, void* v);
		// Load the settings data
		virtual void load_values();
		// create the form
		virtual void create_form(int X, int Y);
		// update or initialise the form
		virtual void save_values();
		// Enable widgets
		virtual void enable_widgets();

		// attributes
		// Auto-import enables
		bool enable_auto_[AUTO_COUNT];
		// Enable TQSL executable
		bool enable_tqsl_;
		// Enable eQSL cards
		bool enable_card_;
		// Enable auto-backup
		bool enable_backup_;
		// Auto-import files
		string auto_file_[AUTO_COUNT];
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
		// Names of the modem applications providiing auto-import data
		string auto_src_[AUTO_COUNT];
		// Default web-browser location
		string web_browser_;
		// Unzipper location
		string unzipper_;
		// Unzipper switches
		string unzip_switches_;
		// QSL Template 
		string qsl_template_;
		// Empty auto-import files after importing them
		bool auto_empty_[AUTO_COUNT];
		// callback data for the auto-import browse buttons
		browser_data_t auto_data_[AUTO_COUNT];
		// Timestamps for files - not editable here
		string auto_ts_[AUTO_COUNT];
		// callback data for the TQSL executable browse button
		browser_data_t tqsl_data_;
		// callback data for the eQSL card directory browse button
		browser_data_t card_data_;
		// callback data for the reference directory brose button
		browser_data_t ref_data_data_;
		// callback data for the Backup directory browse button
		browser_data_t backup_data_;
		// callback data for the web browser executable browse button
		browser_data_t web_data_;
		// callback data for the status log file browse button
		browser_data_t status_data_;
		// callback data for all.txt file
		browser_data_t wsjtx_data_;
		// callback data for the unzipper command
		browser_data_t unzipper_data_;
		// callback data for QSL template
		browser_data_t template_data_;
		// Poll interval (in seconds) for auto-importing
		double auto_poll_;
		// The specification of auto-import has changed
		bool autos_changed_;
		// Stations callsign
		string station_callsign_;

		// QSL Params widget
		Fl_Widget* bn_params_;


	};

}
#endif
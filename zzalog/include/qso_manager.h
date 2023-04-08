#ifndef __STN_DIALOG__
#define __STN_DIALOG__

#include "qso_rig.h"
#include "qso_clock.h"
#include "qso_data.h"

#include "callback.h"
#include "record.h"
#include "intl_widgets.h"
#include "alarm_dial.h"
#include "field_choice.h"
#include "qsl_viewer.h"
#include "record_table.h"
#include "book.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <array>

#include <FL/Fl_Window.H>

using namespace std;

	// This class provides the dialog to chage the current station settings: rig, antenna and QTH
	class qso_manager :
		public Fl_Window
	{
		// Logging mode - used when initialising a record
	public:

		// A bit of a misnomer - but the category of settings
		enum stn_item_t {
			RIG = 1,
			ANTENNA = 2,
			CALLSIGN = 4,
			QTH = 8,
			NONE = 0
		};


	// qso_manager
	public:
		qso_manager(int W, int H, const char* label);
		virtual ~qso_manager();

		// Override to tell menu when it opens and closes
		virtual int handle(int event);

		// get settings - rely on individual groups to do it
		virtual void load_values();
		// create the form
		virtual void create_form(int X, int Y);
		// enable/disable widgets - rely on individual groups to do it
		virtual void enable_widgets();
		// save values
		virtual void save_values();
		// Update record with MY_RIG etc. For use when importing 
		void update_import_qso(record* import);
		// Query/update
		void update(hint_t hint, qso_num_t log_num, qso_num_t query_num);
		// Update time
		void update_time(time_t when);

		// Callback - close button
		static void cb_close(Fl_Widget* w, void* v);

		// Actions attached to the various buttons and keyboard shortcuts
		enum actions {
			WRITE_CALL,
			WRITE_NAME,
			WRITE_QTH,
			WRITE_GRID,
			WRITE_RST_SENT,
			WRITE_RST_RCVD,
			WRITE_FIELD
		};

		// Get logging mode
		qso_data::logging_mode_t logging_mode();
		// Set logging mode
		void logging_mode(qso_data::logging_mode_t mode);
		// Get logging state
		qso_data::logging_state_t logging_state();
		// Called when rig is read or changed
		void update_rig();
		// Switch rig status
		void switch_rig();
		// Present query (uses view update mechanism)
		void update_qso(hint_t hint, qso_num_t match_num, qso_num_t query_num);
		// QSO i n progress
		bool qso_in_progress();
		// Start QSO
		void start_qso();
		// Create a dummy qso - eg for parsing a callsign
		record* dummy_qso();
		// End QSO
		void end_qso();
		// Edit QSO
		void edit_qso();
		// Get default value
		string get_default(stn_item_t item);
		// Get current MY_RIG value
		string get_my_rig();
		// 1 second ticker
		void ticker();
		// Change rig 
		void change_rig(string rig_name);


	protected:

		// Set of bands in frequency order
		list<string> ordered_bands_;
		// Widgets have been xcreated
		bool created_;
		// Typing into choice - prevent it being overwritten
		bool items_changed_;
		// Ticker action in progress - prevent continual ticker access
		bool ticker_in_progress_;

		// Groups
		qso_data* data_group_;
		qso_rig* rig_group_;
		qso_clock* clock_group_;
		// widgets

		const static int WEDITOR = 400;

	};

#endif


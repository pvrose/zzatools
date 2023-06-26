#ifndef __STN_DIALOG__
#define __STN_DIALOG__

#include "qso_clock.h"
#include "qso_data.h"
#include "qso_tabbed_rigs.h"
#include "qso_log_info.h"

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

#include <FL/Fl_Double_Window.H>

using namespace std;

	// This class provides the dialog to chage the current station settings: rig, antenna and QTH
	class qso_manager :
		public Fl_Double_Window
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

		// In a state that does not allow interference
		bool editing();
		// The QSO number is inwith that being edited
		bool outwith_edit(qso_num_t number);
		// Called when rig is read or changed
		void update_rig();
		// Switch rig status
		void switch_rig();
		// Present query (uses view update mechanism)
		void update_qso(hint_t hint, qso_num_t match_num, qso_num_t query_num);
		// Update modem query with a QSO
		void update_modem_qso(record* qso);
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
		// 1 second ticker
		void ticker();
		// Change rig 
		void change_rig(string rig_name);
		// Get rig
		rig_if* rig();
		// Stop the 1s timer in qso_clock
		void stop_ticker();
		// Get current displayed QSO
		qso_data* data();


	protected:

		// Set of bands in frequency order
		list<string> ordered_bands_;
		// Widgets have been xcreated
		bool created_;
		// Typing into choice - prevent it being overwritten
		bool items_changed_;
		// Ticker action in progress - prevent continual ticker access
		bool ticker_in_progress_;
		// Closing down app from here
		bool close_by_dash_;

		// Groups
		qso_data* data_group_;
		qso_tabbed_rigs* rig_group_;
		qso_clock* clock_group_;
		qso_log_info* info_group_;
		// widgets

		const static int WEDITOR = 400;

	};

#endif


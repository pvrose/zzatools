#ifndef __STN_DIALOG__
#define __STN_DIALOG__

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <array>

#include <FL/Fl_Double_Window.H>

using namespace std;

// Forward declarations
class qso_clocks;
class qso_data;
class qso_tabbed_rigs;
class qso_log_info;
class qso_log;
class qso_rig;
class qso_qsl;
class rig_if;
class import_data;
class record;

typedef size_t qso_num_t;


	// This class provides the main dashboard functionality of the app.
	// It contains all the objects used during a QSO in a hierarchy
	// of Fl_Group and Fl_Tabss
	class qso_manager :
		public Fl_Double_Window
	{
		// Logging mode - used when initialising a record
	public:

		// Used in get_default to identify the object
		enum stn_item_t {
			RIG = 1,        // MY_RIG
			ANTENNA = 2,    // MY_ANTENNA
			CALLSIGN = 4,   // STATION_CALLSIGN
			QTH = 8,        // APP_ZZA_QTH
			OP = 16,        // APP_ZZA_OP
			NONE = 0
		};


	// qso_manager
	public:
		qso_manager(int W, int H, const char* label = nullptr);
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
		// Update time
		void update_time(time_t when);

		// Callback - close button
		static void cb_close(Fl_Widget* w, void* v);

		// In a state that does not allow interference
		bool editing();
		// The QSO number is inwith that being edited
		bool outwith_edit(qso_num_t number);
		// Called when rig is read or changed
		void update_rig();
		// Switch rig status
		void switch_rig();
		// Present query (uses view update mechanism)
		void update_qso(uchar hint, qso_num_t match_num, qso_num_t query_num);
		// Start modem - returns a new record
		record* start_modem_qso(string call, uchar source);
		// Log modem record
		void update_modem_qso(bool log_it);
		// Log modem record
		void enter_modem_qso(record* qso);
		// Cancel mode QSO
		void cancel_modem_qso();
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
		// Change rig 
		void change_rig(string rig_name);
		// Get rig
		rig_if* rig();
		// Get rig control
		qso_rig* rig_control();
		// Stop the 1s timer in qso_clock
		void stop_ticker();
		// Get current displayed QSO
		qso_data* data();
		// QSL handler
		qso_qsl* qsl_control();
		// Log info
		qso_log_info* log_info();

		// Deactivate all QSOs and Rigs
		void deactivate_all();

		// Shared QSL methods
		void qsl_download(uchar server);
		void qsl_extract(uchar server);
		void qsl_upload();
		void qsl_print();
		void qsl_print_done();

		// Widgets have been xcreated
		bool created_;

	protected:

		// Set of bands in frequency order
		list<string> ordered_bands_;
		// Typing into choice - prevent it being overwritten
		bool items_changed_;
		// Closing down app from here
		bool close_by_dash_;

		// Groups
		qso_data* data_group_;
		qso_tabbed_rigs* rig_group_;
		qso_clocks* clock_group_;
		qso_log* info_group_;
		// widgets

		const static int WEDITOR = 400;

	};

#endif


#ifndef __STN_DIALOG__
#define __STN_DIALOG__

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <array>

#include <FL/Fl_Double_Window.H>



// Forward declarations
class qso_apps;
class qso_clocks;
class qso_data;
class qso_tabbed_rigs;
class qso_log_info;
class qso_log;
class qso_rig;
class qso_qsl;
class qso_wx;
class rig_if;
class import_data;
class record;

typedef size_t qso_num_t;
enum hint_t : uchar;


	//! This class provides the main dashboard functionality of the app.
	
	//! It contains all the objects used during a QSO in a hierarchy
	//! of Fl_Group and Fl_Tabss
	class qso_manager :
		public Fl_Double_Window
	{
	public:

		//! Station item: used in get_default to identify the object
		enum stn_item_t : char {
			RIG = 1,        //!< MY_RIG
			ANTENNA = 2,    //!< MY_ANTENNA
			CALLSIGN = 4,   //!< STATION_CALLSIGN
			QTH = 8,        //!< MY_CITY etc.
			OP = 16,        //!< MY_NAME etc.
			NONE = 0
		};


	// qso_manager
	public:
		//! Constuctor.

		//! \param W width 
		//! \param H height
		//! \param L label
		qso_manager(int W, int H, const char* L = nullptr);
		//! Destructor.
		virtual ~qso_manager();

		//! Inherited from Fl_Double_Window::handle.
		 
		//! It tells menu_ when it opens and closes.
		//! It takes focus to enable keyboard F1 to open userguide.
		virtual int handle(int event);

		//! Load previous window position from settings.
		void load_values();
		//! Instantiate component widgets.
		void create_form(int X, int Y);
		//! Configure component widgets after data change.
		void enable_widgets();
		//! Save window position to settings.
		void save_values();
		//! Update record with MY_RIG etc from imported QSO record \p import. 
		void update_import_qso(record* import);

		//! Callback from system close button: closes ZZALOG.
		static void cb_close(Fl_Widget* w, void* v);

		//! Returns that the QSO record is being edited.
		bool editing();
		//! Returns true if the QSO record indexed \p number is outwith those being edited.
		bool outwith_edit(qso_num_t number);
		//! Called whenever rig is read or changed
		void update_rig();
		//! Called whenever the active rig is changed.
		void switch_rig();
		//! Present the user with a merge or duplicate query (uses view update mechanism)
		
		//! \param hint Indication of what has changed.
		//! \param match_num Index of QSO record to be matched against.
		//! \param query_num Index of QSO record being query.
		void update_qso(hint_t hint, qso_num_t match_num, qso_num_t query_num);
		//! Returns a new QSO record for a modem \p source QSO with \p call.
		record* start_modem_qso(std::string call, uchar source);
		//! Update the modem QSO record: Save it to the log if \p log_it is true.
		void update_modem_qso(bool log_it);
		//! Enter the QSO record \p qso into the log.
		void enter_modem_qso(record* qso);
		//! Cancel the current modem QSO.
		void cancel_modem_qso();
		//! Return true of a QSO is in progress.
		bool qso_in_progress();
		//! Start a new QSO.
		void start_qso();
		//! Returns a dummy qso - eg for parsing a callsign
		record* dummy_qso();
		//! End the current QSO.
		void end_qso();
		//! Edit the selected QSO record.
		void edit_qso();
		//! Returns the default value for \p item.
		std::string get_default(stn_item_t item);
		//! Change rig to \p rig_name. 
		void change_rig(std::string rig_name);
		//! Returns the rig_if structure for the currently attached rig.
		rig_if* rig();
		//! Returns the rig control widget qso_rig.
		qso_rig* rig_control();
		//! Returns the QSO control widget qso_data.
		qso_data* data();
		//! Returns the QSL upload/download control widget qso_qsl.
		qso_qsl* qsl_control();
		//! Retunrs the log status widget qso_log_info.
		qso_log_info* log_info();
		//! Returns the associated app control widget qso_apps.
		qso_apps* apps();
		//! Returns the weather control
		qso_wx* wx();

		//! Deactivate all controls.
		void deactivate_all();

		// Shared QSL methods
		//! Download std::list of QSLs from \p server.
		void qsl_download(uchar server);
		//! Extract data for uploading to \p server.
		void qsl_extract(uchar server);
		//! Upload extracted data to remembered server.
		void qsl_upload();
		//! Print labels for extracted data.
		void qsl_print();
		//! Mark all extracted QSO records as with QSL sent flag and date. 
		void qsl_print_done();
		//! Send e-mail QSLs for all extracted QSO records.
		void qsl_email();

		//! Merge data downloaded from QRZ.com for current QSO record.
		void merge_qrz_com();

		//! Flag to indicate that all widgets have been created so that pointers are valid.
		bool created_;

	protected:

		//!  List of bands in frequency order
		std::list<std::string> ordered_bands_;
		//! Flag indicates that ZZALOG is being closed because this window is being closed.
		bool close_by_dash_;

		// Groups
		qso_data* data_group_;         //!< QSO editing and viewing. 
		qso_tabbed_rigs* rig_group_;   //!< Rig access and controls.
		qso_clocks* clock_group_;      //!< Clock and weather displays.
		qso_log* info_group_;          //!< General information.

		//! Fixed width of QSO editing pane.
		const static int WEDITOR = 400;

	};

#endif


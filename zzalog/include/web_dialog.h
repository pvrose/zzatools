#ifndef __WEB_DIALOG__
#define __WEB_DIALOG__

#include "page_dialog.h"
#include "callback.h"

#include <string>
#include <ctime>
#include <set>
#include <map>



class Fl_Group;
class Fl_Select_Browser;
class Fl_Widget;
struct qsl_call_data;
struct server_data_t;

	//! This class provides a dialog to let the user supply web addresses, usernames and passwords.
	class web_dialog :
		public page_dialog
	{
	public:
		//! Constructor.

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		web_dialog(int X, int Y, int W, int H, const char* L = nullptr);
		//! Destructor
		virtual ~web_dialog();

		//! Inherited from page_dialog allows keyboard F1 to open userguide
		virtual int handle(int event);

		// inherited methods
		// Standard methods - need to be written for each
		//! Load values from settings and internal databases loaded from files
		virtual void load_values();
		//! Instantiate component widgets.
		virtual void create_form(int X, int Y);
		//! Save values back to settings.
		virtual void save_values();
		//! Configure component widgets after data change.
		virtual void enable_widgets();

	protected:
		// Callbacks
		//! Callback when switching tabs: reformats labels.
		static void cb_tab(Fl_Widget* w, void* v);
		//! Callback adding data to No QSL list.
		
		//! \p v points to input choice
		static void cb_add_noqsl(Fl_Widget* w, void* v);
		//! Callback deleting data from No QSL list

		//! \p v points to input choice
		static void cb_del_noqsl(Fl_Widget* w, void* v);

		//! Instantiate the widgets to configure eQSL interface.
		void create_eqsl(int rx, int ry, int rw, int rh);
		//! Instantiate the widgets to configure the LotW interface.
		void create_lotw(int rx, int ry, int rw, int rh);
		//! Instantiate the widgets to configure the QRZ.com interface.
		void create_qrz(int rx, int ry, int rw, int rh);
		//! Instatntiate the widgets to configure the Clublog.org interface.
		void create_club(int rx, int ry, int rw, int rh);
		//! Instantiate the widgets to configure the e-Mail interface. 
		void create_email(int rx, int ry, int rw, int rh);
		//! Instantiate the widgets to define No QSL list
		void create_noqsl(int rx, int ry, int rw, int rh);

		//! Returns the server data
		server_data_t* get_server(std::string name);

		//! Populate No QSL list
		void populate_noqsl(Fl_Select_Browser* b);
		

		// Widgets for eQSL
		Fl_Group* grp_eqsl_;            //!< eQSL widget group
		Fl_Group* grp_eqsl_calls_;      //!< Group for eQSL logbook credentials
		// Widgets for LotW
		Fl_Group* grp_lotw_;            //!< LotW widget group
		// Widgets for QRZ.com
		Fl_Group* grp_qrz_;             //!< QRZ.com widget group
		Fl_Group* grp_qrz_xml_;         //!< Group for QRZ.com XML subscription
		Fl_Group* grp_qrz_api_;         //!< Group for QRZ.com API configuration
		// Widgets for ClubLog
		Fl_Group* grp_club_;            //!< Clublog.org widget group
		// Widgets for networking   
		Fl_Group* grp_server_;
		// Widgets for e-mail
		Fl_Group* grp_email_;           //!< e-Mail widget group/#
		// Widgets for NoQSL
		Fl_Group* grp_noqsl_;           //!< No QSL widget group

		//! Mapping of callsign to widget groups per QRZ.com logbook
		std::map<std::string, Fl_Group*> grp_api_books_;
		//! Mapping of callsign to eQSL last download date widgets.
		std::map<std::string, Fl_Widget*> w_eqsl_lupds_;

		server_data_t* eqsl_data_;       //! eQSL.cc configuration data
		server_data_t* lotw_data_;       //! LotW configuration data
		server_data_t* club_data_;       //! Clublog.org configuration data
		server_data_t* qrz_data_;        //! QRZ.com configuration data
		server_data_t* email_data_;      //! e-Mail configuration data
		std::set<std::string>* noqsl_data_;   //!< No QSL list data

		std::string add_call_;           //!< Callsign to add

	};
#endif

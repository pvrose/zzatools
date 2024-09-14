#ifndef __WEB_DIALOG__
#define __WEB_DIALOG__

#include "page_dialog.h"
#include "callback.h"

#include <string>
#include <ctime>
#include <set>

using namespace std;

class Fl_Widget;
class Fl_Group;

	// This class provides a dialog to let the user supply web addresses, usernames and passwords
	class web_dialog :
		public page_dialog
	{
	public:
		web_dialog(int X, int Y, int W, int H, const char* label = nullptr);
		virtual ~web_dialog();

		// inherited methods
		// Standard methods - need to be written for each
		// Load values from settings_
		virtual void load_values();
		// Used to create the form
		virtual void create_form(int X, int Y);
		// Used to write settings back
		virtual void save_values();
		// Used to enable/disable specific widget - any widgets enabled musr be attributes
		virtual void enable_widgets();

	protected:
		// Callbacks
		// Enable WSJT-X
		static void cb_bn_wsjtx(Fl_Widget* w, void* v);
		// Enable FLDIGI
		static void cb_bn_fldigi(Fl_Widget* w, void* v);
		// Switch tabs
		static void cb_tab(Fl_Widget* w, void* v);

		// Create the eQSL group
		void create_eqsl(int rx, int ry, int rw, int rh);
		// Create the LotW group
		void create_lotw(int rx, int ry, int rw, int rh);
		// Create the QRZ.com group
		void create_qrz(int rx, int ry, int rw, int rh);
		// Create the Club log group
		void create_club(int rx, int ry, int rw, int rh);
		// Create the server group
		void create_server(int rx, int ry, int rw, int rh);
		

		// Widgets for eQSL
		Fl_Group* grp_eqsl_;
		// Widgets for LotW
		Fl_Group* grp_lotw_;
		// Widgets for QRZ.com
		Fl_Group* grp_qrz_;
		// Widgets for ClubLog
		Fl_Group* grp_club_;
		// Widgets for networking
		Fl_Group* grp_server_;
		// Widgets containing images
		set<Fl_Widget*> image_widgets_;

		// 
		Fl_Group* grp_wsjtx_;
		Fl_Group* grp_fldigi_;


		// eQSL attributes

		// enable eQSL access
		bool eqsl_enable_;
		// download starting at this date
		string eqsl_last_got_;
		// user name
		string eqsl_username_;
		// password
		string eqsl_password_;
		// use QSO message
		bool eqsl_use_qso_msg_;
		string eqsl_qso_msg_;
		// SWL message
		bool eqsl_use_swl_msg_;
		string eqsl_swl_msg_;
		// Upload QSOs as logged
		bool eqsl_upload_qso_;
		// Download confirmed as well
		bool eqsl_confirmed_too_;

		// LotW attributes

		// enable
		bool lotw_enable_;
		// download start date
		string lotw_last_got_;
		// user name/password
		string lotw_username_;
		string lotw_password_;
		// Upload QSOs as logged
		bool lotw_upload_qso_;

		// QRZ.com attributes

		// enable
		bool qrz_enable_;
		// username/password
		string qrz_username_;
		string qrz_password_;
		// Use XML database and merge
		bool qrz_xml_merge_;

		// ClubLog attributes

		// enable
		bool club_enable_;
		// username (e-mail)
		string club_password_;
		string club_username_;
		// Number of days between downloads
		int club_interval_;
		// Upload QSOs as logged
		bool club_upload_qso_;

		// wsjt-x udp PORT CONNECTION
		int wsjtx_udp_port_;
		string wsjtx_udp_addr_;
		bool wsjtx_enable_;
		// Fl-digi log xmlrpc server
		int fldigi_rpc_port_;
		string fldigi_rpc_addr_;
		bool fldigi_enable_;

	};
#endif

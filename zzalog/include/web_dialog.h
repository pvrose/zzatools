#ifndef __WEB_DIALOG__
#define __WEB_DIALOG__

#include "page_dialog.h"
#include "callback.h"

#include <string>
#include <ctime>
#include <set>
#include <map>

using namespace std;

class Fl_Widget;
class Fl_Group;
struct qrz_api_data;
struct server_data_t;

	// This class provides a dialog to let the user supply web addresses, usernames and passwords
	class web_dialog :
		public page_dialog
	{
	public:
		web_dialog(int X, int Y, int W, int H, const char* label = nullptr);
		virtual ~web_dialog();

		// inherited methods
		// Standard methods - need to be written for each
		// Load values from settings
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
		// Create the e-mail group
		void create_email(int rx, int ry, int rw, int rh);

		// Get the server
		server_data_t* get_server(string name);
		

		// Widgets for eQSL
		Fl_Group* grp_eqsl_;
		// Widgets for LotW
		Fl_Group* grp_lotw_;
		// Widgets for QRZ.com
		Fl_Group* grp_qrz_;
		Fl_Group* grp_qrz_xml_;
		Fl_Group* grp_qrz_api_;
		// Widgets for ClubLog
		Fl_Group* grp_club_;
		// Widgets for networking
		Fl_Group* grp_server_;
		// Widgets for e-mail
		Fl_Group* grp_email_;
		// Widgets containing images
		set<Fl_Widget*> image_widgets_;

		// 
		Fl_Group* grp_wsjtx_;
		Fl_Group* grp_fldigi_;

		map<string, Fl_Group*> grp_api_books_;

		server_data_t* eqsl_data_;
		server_data_t* lotw_data_;
		server_data_t* club_data_;
		server_data_t* qrz_data_;
		server_data_t* email_data_;

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

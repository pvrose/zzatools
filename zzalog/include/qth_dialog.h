#pragma once
#ifndef __QTH_DIALOG__
#define __QTH_DIALOG__

#include "win_dialog.h"
#include "record.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>

using namespace std;

	// This class provides the dialog to chage the current station settings: rig, aerial and QTH
	class qth_dialog :
		public win_dialog
	{

		// Station location details - ADIF field name
		struct qth_info_t {
			string name;           // MY_NAME
			string street;         // MY_STREET
			string city;           // MY_CITY
			string postcode;       // MY_POSTAL_CODE
			string locator;        // MY_GRIDSQUARE
			string dxcc_name;      // MY_COUNTRY
			string dxcc_id;        // MY_DXCC
			string state;          // MY_STATE
			string county;         // MY_CNTY
			string cq_zone;        // MY_CQ_ZONE
			string itu_zone;       // MY_ITU_ZONE
			string continent;      // MY_CONT
			string iota;           // MY_IOTA
			string description;    // not ADIF - APP_ZZA_QTH_DESCR

			bool operator==(qth_info_t rhs) {
				return name == rhs.name &&
					street == rhs.name &&
					city == rhs.city &&
					postcode == rhs.postcode &&
					locator == rhs.locator &&
					dxcc_name == rhs.dxcc_name &&
					dxcc_id == rhs.dxcc_id &&
					state == rhs.state &&
					county == rhs.county &&
					cq_zone == rhs.cq_zone &&
					itu_zone == rhs.itu_zone &&
					continent == rhs.continent &&
					iota == rhs.iota &&
					description == rhs.description;
			}
		};

	public:

		qth_dialog(string qth_name);
		virtual ~qth_dialog();

		// get settings
		void load_values();
		// create the form
		void create_form(int X, int Y);
		// enable/disable widgets
		void enable_widgets();
		// save values
		void save_values();
		// button callback - OK
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// button callback - cancel
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		// input callback - country input
		static void cb_ip_cty(Fl_Widget* w, void* v);
		// upper case callback
		static void cb_ip_upper(Fl_Widget* w, void* v);
		// Convert lat/long to gridsquare
		static void cb_bn_convert(Fl_Widget* w, void* v);
		// 

	protected:

		// Current QTH
		qth_info_t current_qth_;
		qth_info_t original_qth_;
		// QTH macro expansion
		record* qth_details_;
		// QTH name
		string qth_name_;
		// Any field changed
		bool changed_;

		// All widgets
		Fl_Input* ip_name_;
		Fl_Input* ip_address1_;
		Fl_Input* ip_address3_;
		Fl_Input* ip_address4_;
		Fl_Input* ip_locator_;
		Fl_Input* ip_dxcc_;
		Fl_Int_Input* ip_dxcc_adif_;
		Fl_Input* ip_admin1_;
		Fl_Input* ip_admin2_;
		Fl_Int_Input* ip_cq_zone_;
		Fl_Int_Input* ip_itu_zone_;
		Fl_Input* ip_cont_;
		Fl_Input* ip_iota_;
		Fl_Input* ip_description_;

		Fl_Float_Input* ip_latitude_;
		Fl_Float_Input* ip_longitude_;

		// Parameters for each of the QTH input widgets
		enum input_type {
			INTEGER,
			MIXED,
			UPPER
		};
	};

#endif

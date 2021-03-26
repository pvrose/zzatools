#ifndef __STN_DIALOG__
#define __STN_DIALOG__


#include "../zzalib/page_dialog.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Int_Input.H>

using namespace std;
using namespace zzalib;

namespace zzalog {
	// This class provides the dialog to chage the current station settings: rig, aerial and QTH
	class stn_dialog :
		public page_dialog
	{
		// A bit of a misnomer - but the category of settings
		enum equipment_t {
			RIG,
			AERIAL,
			QTH,
			NONE
		};

		// Station location details - ADIF field name
		struct qth_info_t {
			string callsign;       // STATION_CALLSIGN
			string name;           // MY_NAME
			string street;         // MY_STREET
			string town;           // MY_CITY
			string county;         // MY_CNTY
			string country;        // MY_COUNTRY
			string postcode;       // MY_POSTAL_CODE
			string locator;        // MY_GRIDSQUARE
			string dxcc_id;        // MY_DXCC
			string dxcc_name;      // MY_COUNTRY
			string state;          // MY_STATE
			string cq_zone;        // MY_CQ_ZONE
			string itu_zone;       // MY_ITU_ZONE
			string continent;      // MY_CONT
			string iota;           // MY_IOTA
		};

		// This class provides individual grouping for rig, aerial and QTH.
		// It is itself inherited for particular uses (rig and QTH)
		class common_grp :
			public Fl_Group
		{

		public:

			common_grp(int X, int Y, int W, int H, const char* label);
			virtual ~common_grp();

			// get settings
			virtual void load_values();
			// create the form
			virtual void create_form(int X, int Y);
			// enable/disable widgets
			virtual void enable_widgets() {};
			// save values
			virtual void save_values();
			// button callback - add
			static void cb_bn_add(Fl_Widget* w, void* v);
			// button callback - delete
			static void cb_bn_del(Fl_Widget* w, void* v);
			// button callback - all/active items
			static void cb_bn_all(Fl_Widget* w, void* v);
			// button callback - active/deactive
			static void cb_bn_activ8(Fl_Widget* w, void* v);
			// choice callback
			static void cb_ch_stn(Fl_Widget* w, void* v);


		protected:
			// (re)populate the choice widget
			void populate_choice();
			// Add an item
			virtual void add_item();
			// Delete item
			virtual void delete_item(string item);
			// Update derived fields
			virtual void update_item();
			// Save derived fields
			virtual void save_item();
			// Choice widget
			Fl_Widget* choice_;
			// Active widget
			Fl_Widget* active_;
			// type
			equipment_t type_;
			// in text for Fl_Preference
			string settings_name_;
			// selected item
			int selected_item_;
			// selected item's name
			string selected_name_;
			// all items - active/inactive
			map<string, bool> all_items_;
			// current item active
			bool item_active_;
			// display all items
			bool display_all_items_;
			// The settings
			Fl_Preferences* my_settings_;
		};

		// This class provides individual QTH. It inherits from common group
		// some of the features
		class qth_group :
			public common_grp
		{

		public:

			qth_group(int X, int Y, int W, int H, const char* label);
			virtual ~qth_group();

			// get settings
			virtual void load_values();
			// create the form
			virtual void create_form(int X, int Y);
			// enable/disable widgets
			virtual void enable_widgets() {};
			// save values
			virtual void save_values();
			// callsign callback - update other fields
			static void cb_ip_call(Fl_Widget* w, void* v);
			// upper case callback
			static void cb_ip_upper(Fl_Widget* w, void* v);


		protected:
			// Update QTH fields
			virtual void update_item();
			// Add a new item
			virtual void add_item();
			// Delete an item
			virtual void delete_item(string item);
			// Save item
			virtual void save_item();
			// QTH info
			Fl_Widget* qth_info_[15];
			// QTH info
			map<string, qth_info_t*> all_qths_;
			// Current QTH
			qth_info_t current_qth_;
			// Parameters for each of the QTH input widgets
			enum input_type {
				CALL,
				INTEGER,
				MIXED,
				UPPER
			};
			struct { const char* label; void* v; input_type type; int row; int col; } qth_params_[15] = {
				{ "Callsign", (void*)&(current_qth_.callsign), CALL, 0, 0 },
				{ "Name", (void*)&(current_qth_.name), MIXED, 1, 0 },
				{ "Street", (void*)&(current_qth_.street), MIXED, 2, 0 },
				{ "Town", (void*)&(current_qth_.town), MIXED, 3, 0 },
				{ "County", (void*)&(current_qth_.county), MIXED, 4, 0 },
				{ "Country", (void*)&(current_qth_.country), MIXED, 0, 1 },
				{ "Postcode", (void*)&(current_qth_.postcode), MIXED, 1, 1 },
				{ "Locator", (void*)&(current_qth_.locator), UPPER, 2, 1 },
				{ "DXCC Id", (void*)&(current_qth_.dxcc_id), INTEGER, 3, 1 },
				{ "DXCC Name", (void*)&(current_qth_.dxcc_name), MIXED, 4, 1 },
				{ "State", (void*)&(current_qth_.state), UPPER, 0, 2 },
				{ "CQ Zone", (void*)&(current_qth_.cq_zone), INTEGER, 1, 2 },
				{ "ITU Zone", (void*)&(current_qth_.itu_zone), INTEGER, 2, 2 },
				{ "Continent", (void*)&(current_qth_.continent), UPPER, 3, 2 },
				{ "IOTA", (void*)&(current_qth_.iota), UPPER, 4, 2 }
			};
		};

		// This class provides individual grouping for rig. It inherits from common group
		// But with additional controls to input power matrix
		class rig_group :
			public common_grp
		{

		public:

			rig_group(int X, int Y, int W, int H, const char* label);
			virtual ~rig_group();

			// get settings
			virtual void load_values();
			// create the form
			virtual void create_form(int X, int Y);
			// enable/disable widgets
			virtual void enable_widgets() {};
			// save values
			virtual void save_values();

		};

		// This class provides individual grouping for aerials. It inherits from common group
		class aerial_group :
			public common_grp
		{
		public:

			aerial_group(int X, int Y, int W, int H, const char* label);
			virtual ~aerial_group();

			// create the form
			virtual void create_form(int X, int Y);
		};

	// stn_dialog
	public:
		stn_dialog(int X, int Y, int W, int H, const char* label);
		virtual ~stn_dialog();

		// get settings - rely on individual groups to do it
		virtual void load_values() {};
		// create the form
		virtual void create_form(int X, int Y);
		// enable/disable widgets - rely on individual groups to do it
		virtual void enable_widgets() {};
		// save values
		virtual void save_values();

	protected:
		// Rig widgets
		common_grp * rig_grp_;
		// Aerial widgets
		common_grp* aerial_grp_;
		// QTH widgets
		common_grp* qth_grp_;

	};

}
#endif


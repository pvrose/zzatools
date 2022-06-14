#pragma once
#ifndef __QTH_DIALOG__
#define __QTH_DIALOG__


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
	class qth_dialog :
		public page_dialog
	{

		// Station location details - ADIF field name
		struct qth_info_t {
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

	public:

		qth_dialog(int X, int Y, int W, int H, const char* label);
		virtual ~qth_dialog();

		// get settings
		void load_values();
		// create the form
		void create_form(int X, int Y);
		// enable/disable widgets
		void enable_widgets() {};
		// save values
		void save_values();
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
		// upper case callback
		static void cb_ip_upper(Fl_Widget* w, void* v);


	protected:
		// (re)populate the choice widget
		void populate_choice();
		// Add an item
		void add_item();
		// Delete item
		void delete_item(string item);
		// Update derived fields
		void update_item();
		// Save derived fields
		void save_item();
		// Choice widget
		Fl_Widget* choice_;
		// Active widget
		Fl_Widget* active_;
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

		// QTH info
		Fl_Widget* qth_info_[15];
		// QTH info
		map<string, qth_info_t*> all_qths_;
		// Current QTH
		qth_info_t current_qth_;

		// Parameters for each of the QTH input widgets
		enum input_type {
			INTEGER,
			MIXED,
			UPPER
		};
#define NUM_QTH_PARAMS 14
		struct { const char* label; void* v; input_type type; int row; int col; } qth_params_[NUM_QTH_PARAMS] = {
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

};

#endif

#ifndef __COLUMN_DIALOG__
#define __COLUMN_DIALOG__

#include "page_dialog.h"
#include "view.h"
#include "fields.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Widget.H>

using namespace std;



	// This class allows the fields to be displayed in a table together with parameters 
	// it is only used in fields_dialog
	class fields_table :
		public Fl_Table_Row
	{
	public:
		fields_table(int X, int Y, int W, int H, const char*label = 0);
		virtual ~fields_table();


		// public methods
	public:
		// inherited from Fl_Table_Row
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);
		// Set the fields to display
		void fields(vector<field_info_t>* fields);
		// get current row
		int row();

		// Protected methods
	protected:
		// The fields being displayed
		vector<field_info_t>* fields_;
	};

	// This class displays a dialog that allows the user to select and order fields that are to be used
	// in the various different views of the log
	class fields_dialog :
		public page_dialog

	{
	public:

		// Constructor
		fields_dialog(int X, int Y, int W, int H, const char* label);
		virtual ~fields_dialog();

		// Load values from settings_
		virtual void load_values();
		// Used to create the form
		virtual void create_form(int X, int Y);
		// Used to write settings back
		virtual void save_values();
		// Used to enable/disable specific widget - any widgets enabled musr be attributes
		virtual void enable_widgets();

		// Callbacks
		// Select collection closed
		static void cb_ch_sel_col(Fl_Widget* w, void* v);
		// Select collection by application
		static void cb_ch_sel_app(Fl_Widget* w, void* v);
		// Copy an existing collection
		static void cb_bn_new(Fl_Widget* w, void* v);
		// Use collection in selected application
		static void cb_bn_use_app(Fl_Widget* w, void* v);
		// left-hand table clicked (fields in use)
		static void cb_tab_inuse(Fl_Widget* w, void* v);
		// right table clicked (fields available)
		static void cb_tab_avail(Fl_Widget* w, void* v);
		// move field from available to use
		static void cb_bn_use(Fl_Widget* w, void* v);
		// move field from use to available
		static void cb_bn_disuse(Fl_Widget* w, void* v);
		// move field up in use list
		static void cb_bn_up(Fl_Widget* w, void* v);
		// move field down in use list
		static void cb_bn_down(Fl_Widget* w, void * v);
		// header input changed
		static void cb_ip_header(Fl_Widget* w, void* v);
		// width input changed
		static void cb_ip_width(Fl_Widget* w, void* v);

	protected:
		// get ;list of unused fields
		void unused_fields();
		// Update tables
		void update_widgets(bool update_name = true);

	protected:
		// used or available
		bool selection_in_used_;
		// Collection name
		string field_set_name_;
		// Columns are arraigned in collections - i.e. a set of fields to use for a particular logging activity
		map<string, vector<field_info_t>* > field_sets_;
		// And for the specific collection
		vector<field_info_t> unused_fields_;
		// application 
		field_ordering_t application_;
		// which collections are used for which applications
		map <field_ordering_t, string> field_set_by_app_;
		// widgets that need accessing
		Fl_Widget* name_input_;
		Fl_Widget* name_choice_;
		Fl_Widget* app_choice_;
		Fl_Widget* app_buttons_[FO_LAST];
		Fl_Widget* used_table_;
		Fl_Widget* avail_table_;
		Fl_Widget* header_input_;
		Fl_Widget* width_input_;
		Fl_Widget* use_button_;
		Fl_Widget* disuse_button_;
		Fl_Widget* up_button_;
		Fl_Widget* down_button_;
		// Groups that need accessing
		Fl_Group* table_group_;
		Fl_Group* app_group_;

		const string APP_NAMES[FO_LAST] = { "Log", "Extract", "Record", "Import", "TSV", "Choice" };

	};
#endif

#ifndef __CORR_DIALOG__
#define __CORR_DIALOG__

#include "record.h"
#include "win_dialog.h"

#include <string>
#include <set>

#include <FL/Fl_Widget.H>

using namespace std;



	// This class implements a dialog to allow user correction in response to valdation errors
	class corr_dialog : public win_dialog
	{
	public:
		corr_dialog(record* record, const string& field, const string& message);
		virtual ~corr_dialog();

		// return the correction message
		string correction_message();

	protected:
		// call backs OK/Cancel
		static void cb_bn_ok(Fl_Widget* w, void* v);
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		static void cb_bn_quit(Fl_Widget* w, void* v);

		// attributes

		// Change the value
		bool change_value_;
		// Change the field name
		bool change_field_;
		// Add a field
		bool add_field_;
		// Name to change field
		string change_field_name_;
		// Name to add field
		string add_field_name_;
		// Dat to which to change field
		string change_value_data_;
		// data for an added field
		string add_value_data_;

		// the record to be updated
		record* record_;
		// the field being queried
		string query_field_;
		// Waiting OK or CANCEL
		bool pending_button_;
		// button pressed
		button_t button_;
		// correction message
		string correction_message_;

		// widgets to clear
		set<Fl_Widget*> field_choices_;

	};
#endif
#ifndef __CHANGE_DIALOG__
#define __CHANGE_DIALOG__

#include "win_dialog.h"
#include "callback.h"

class Fl_Widget;

	// This enum is tied to a set of radio buttons in the dialog
	enum change_action_t {
		RENAME_FIELD = 0,   // Rename the field
		DELETE_FIELD = 1,   // Delete the field
		ADD_FIELD = 2,      // Add a field without changing any already there
		CHANGE_FIELD = 3    // Add, and change existing fields with this name
	};

	// This class implements a dialog that asks a user how to make similar changes to all records in the log
	class change_dialog :
		public win_dialog
	{
	public:
		change_dialog(const char* label = 0);
		virtual ~change_dialog();

		virtual int handle(int event);

		// Create the dialog
		void create_form();
		// Get the user action
		void get_data(change_action_t& action, string& old_field_name, string& new_field_name, string& new_text);

		// OK button callback
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// Cancel button call back
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		// Action button callback
		static void cb_bn_action(Fl_Widget* w, void* v);
		// Action radio buttons callback
		static void cb_radio_action(Fl_Widget* w, void* v);
		// Old field name choice
		static void cb_ch_old_field(Fl_Widget* w, void* v);

	protected:
		// Enable widgets
		void enable_widgets();
		// Update enumeration choice
		void populate_enum(string name);
		// Original field name
		string old_field_name_;
		// Action selected
		change_action_t action_;
		// New field name
		string new_field_name_;
		// New value
		string new_text_;
		// Radio button callback data
		const radio_param_t radio_params_[4] =
		{
			{ RENAME_FIELD, (int*)&action_ },
			{ DELETE_FIELD, (int*)&action_ },
			{ ADD_FIELD, (int*)&action_ },
			{ CHANGE_FIELD, (int*)&action_ }
		};
		// Widgets that need to be accessed
		Fl_Widget* w_field_name_;
		Fl_Widget* w_text_;
		Fl_Widget* w_enum_;

	};

#endif

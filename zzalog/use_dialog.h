#ifndef __USE_DIALOG__
#define __USE_DIALOG__

#include "win_dialog.h"

#include <string>
#include <map>

#include <FL/Fl_Widget.H>

using namespace std;

namespace zzalog {

	// The item to display in the dialog
	enum use_dialog_t {
		UD_RIG,          // Obtain rigs to use
		UD_AERIAL,       // Obtain aerial to use
		UD_QTH,          // Obtain QTH to use.
		UD_PROP          // Prpagation mode to use
	};

	// This class provides a dialog by which the user can select rig, aerial or QTH currently needs 
	// to be logged
	class use_dialog :
		public win_dialog
	{
	public:
		use_dialog(use_dialog_t type);
		virtual ~use_dialog();

		// callback for OK button
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// callback for cancel button
		static void cb_bn_cancel(Fl_Widget* w, void* v);
		// callback to reload choice on check display all
		static void cb_bn_all(Fl_Widget* w, void* v);

	protected:
		// Load data from settings
		void load_data();
		// Create dialog
		void create_form();
		// Save data to settings
		void save_data();
		// Load the item choice widget
		void load_choice();
		// Item choice widget
		Fl_Widget* choice_;
		// The use of the dialog
		use_dialog_t type_;
		// The path to the item settings
		string setting_path_;
		// The singular item
		string item_name_;
		// The current value of the item
		string current_value_;
		// Display both active and inactive items
		bool display_all_;
		// Identifies whether items are active or inactive
		map<string, bool> all_items_;

	};

}
#endif

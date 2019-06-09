#ifndef __EDIT_DIALOG__
#define __EDIT_DIALOG__

#include "win_dialog.h"

#include <string>

#include <FL/Fl_Input.H>

using namespace std;

namespace zzalog {

	// This provides a means to edit a table cell by displaying an input exactly above the cell
	class edit_dialog :
		public win_dialog
	{
	protected:
		// Create a custom version of Fl_Input so that we can handle keyboard events
		class edit_input :
			public Fl_Input
		{
		public:
			edit_input(int X, int Y, int W, int H);
			virtual int handle(int event);
		};

	public:
		edit_dialog(int X, int Y, int W, int H);
		~edit_dialog();

		// Menu button
		static void cb_menu(Fl_Widget* w, void* v);
		// Menu button select values
		enum menu_item_t {
			UPPER,          // Convert to upper-case
			LOWER,          // Convert to lower-case
			MIXED,          // Convert to mixed case: first letter upper, rest lower
			SAVE,           // Save data
			CANCEL,         // Cancel edit
			SAVE_NEXT,      // Save data and open next item
			SAVE_PREV,      // Save data and open previous
			SAVE_UP,        // Save data and open item in above record
			SAVE_DOWN       // Save data and open item in below record
		};

		// open edit
		button_t display(string value);
		// Get value
		string value();

	protected:
		// Data value
		string value_;
		// Input widget
		Fl_Widget* ip_;

	};

}
#endif
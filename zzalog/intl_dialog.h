#ifndef __INTL_DIALOG__
#define __INTL_DIALOG__

#include "../zzalib/win_dialog.h"

#include <string>
#include <set>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

using namespace std;
using namespace zzalib;

namespace zzalog {

	// This class provides a window that can be used to paste non-ASCII characters into the specified editor widget.
	// The editor widget tells this dialog when it opens.
	// It comprises a number of buttons each of which copies a character to clipboard and pastes it into
	// the current input or editor widget.
	// The available characters for pasting can be added to.
	class intl_dialog : public win_dialog
	{
	public:
		intl_dialog();
		virtual ~intl_dialog();

		virtual int handle(int event);

		// Set the widget that receives the paste command
		void editor(Fl_Widget* w);
		// REturns the widget that recives the paste command
		Fl_Widget* editor();

		// Save the data
		static void cb_bn_save(Fl_Widget* w, void* v);
		// Restore saved data
		static void cb_bn_restore(Fl_Widget* w, void* v);
		// Add a button
		static void cb_bn_add(Fl_Widget* w, void* v);
		// Copy and paste button
		static void cb_bn_use(Fl_Widget* w, void* v);

	protected:

		// Get the pathname to the file
		string get_path();
		// Load data
		bool load_data();
		// Save data
		bool save_data();
		// Create dialog
		void create_form();
		// Add button group
		void add_buttons(int width);
		// Add symbols
		void add_symbols(string text);
		// file name
		string filename_;
		// The set of UCSs to draw buttons for
		set<unsigned int> symbols_;
		// New character
		string new_char_;
		// The button group
		Fl_Group* buttons_;
		// The edit widget
		Fl_Widget* editor_;

	};

}
#endif

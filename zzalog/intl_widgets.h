#ifndef __INTL_WIDGETS__
#define __INTL_WIDGETS__

#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Editor.H>

/* 
This file provides versions of standard input widgets that allow pastes from
the international character dialog
*/
namespace zzalog {

	// An editor for modifying various string items - it accepts pastes from intl_dialog
	class intl_editor : public Fl_Text_Editor
	{
		// Whether the editor is inserting or overwriting
		bool insert_mode_;

	public:
		// Constructor
		intl_editor(int X, int Y, int W, int H, const char* label = "");
		// Event handler - handle event as normal then set the cursor depending on current insert mode
		int handle(int event);
	};


	// Version of Fl_Input that accepts pastes from intl_dialog
	class intl_input : public Fl_Input
	{
	public:
		// Constructor
		intl_input(int X, int Y, int W, int H, const char* label = nullptr);

		// Event handler
		int handle(int event);
	};

}

#endif
#ifndef __INTL_WIDGETS__
#define __INTL_WIDGETS__

#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Editor.H>

/* 
This file provides versions of standard input widgets that allow pastes from
the international character dialog
*/


	//! Vesrion of Fl_Text_Editor that accepts pastes from intl_dialog
	class intl_editor : public Fl_Text_Editor
	{
		//! Whether the editor is inserting or overwriting
		bool insert_mode_;

	public:
		//! Constructor

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		intl_editor(int X, int Y, int W, int H, const char* L = "");
		//! Destructor.
		virtual ~intl_editor();

		//! Event handler - handle event as normal then set the cursor depending on current insert mode.
		int handle(int event);
	};


	//! Version of Fl_Input that accepts pastes from intl_dialog
	class intl_input : public Fl_Input
	{
	public:
		//! Constructor

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param label label
		intl_input(int X, int Y, int W, int H, const char* label = nullptr);
		//! Descriptor.
		virtual ~intl_input();

		//! Event handler - tells intl_dialog that this widget has focus.
		int handle(int event);
	};

#endif
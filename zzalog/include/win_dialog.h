#ifndef __DIALOG__
#define __DIALOG__

#include "drawing.h"

#include <FL/Fl_Window.H>



	// Used for general button responses
	enum button_t {
		BN_OK,   // OK
		BN_CANCEL,  // Cancel
		BN_SPARE    // Any third and subsequent commands - use BN_SPARE+n
	};

	// This class is the base class for free-standing dialogs - it provides a standard way of waiting for the dialog to be closed
	class win_dialog : public Fl_Window
	{
	public:
		win_dialog(int W, int H, const char* label = 0);
		virtual ~win_dialog();

		// Display the dialog until OK or CANCEL Is clicked (pending_button_ is cleared)
		button_t display();
		// Call at end of any button call-back to clear pending_button_
		void do_button(button_t button);

	protected:
		// Which button
		button_t button_;

	};
#endif
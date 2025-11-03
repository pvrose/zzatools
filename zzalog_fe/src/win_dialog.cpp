#include "win_dialog.h"

#include <FL/Fl.H>



// Constructor - derived classes will extend this to build the dialog
win_dialog::win_dialog(int W, int H, const char * label) :
	Fl_Window(W, H, label)
	, button_(BN_CANCEL)
{
}

// Destructor
win_dialog::~win_dialog()
{
	//clear();
}

// Show the dialog and wait for OK or Cancel (or any bespoke buttons) to be clicked
button_t win_dialog::display() {
	show();
	// Default to CANCEL otherwise if another event hides the dialog something nasty happens
	button_ = BN_CANCEL;
	// now wait for OK or cancel to be clicked - using the FLTK scheduler ensures
	// other tasks get a look-in
	while (shown()) { Fl::check(); }
	return button_;
}

// This stops the wait and allows the wanted response to be sent
void win_dialog::do_button(button_t button) {
	button_ = button;
	hide();
}


#include "win_dialog.h"

#include <FL/Fl.H>

using namespace zzalib;

// Constructor - derived classes will extend this to build the dialog
win_dialog::win_dialog(int W, int H, const char * label) :
	Fl_Window(W, H, label)
	, pending_button_(false)
	, button_(BN_OK)
{
}

// Destructor
win_dialog::~win_dialog()
{
	clear();
}

// Show the dialog and wait for OK or Cancel to be clicked
button_t win_dialog::display() {
	pending_button_ = true;
	show();
	// now wait for OK or cancel to be clicked - using the FLTK scheduler ensures
	// other tasks get a look-in
	while (pending_button_) { Fl::wait(); }
	hide();
	return button_;
}

// This stops the wait and allows the wamnted response to be sent
void win_dialog::do_button(button_t button) {
	pending_button_ = false;
	button_ = button;
}


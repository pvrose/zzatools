#include "edit_input.h"
#include "../zzalib/utils.h"
#include "intl_dialog.h"
#include "log_table.h"

#include "../zzalib/callback.h"

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Input.H>

using namespace zzalog;
using namespace zzalib;

extern intl_dialog* intl_dialog_;

// Edit input - create the input to the supplied size
edit_input::edit_input(int X, int Y, int W, int H) :
	intl_input(X, Y, W, H) 
{

}

// Handler for mouse and keyboard events
int edit_input::handle(int event) {
	// Tell the international character dialog this input will accept pastes
	// Get the parent table
	log_table* table = ancestor_view<log_table>(this);
	switch (event) {
	case FL_FOCUS:
		if (intl_dialog_) intl_dialog_->editor(this);
		return true;
	case FL_UNFOCUS:
		// mouse coming in or going out of focus on this view
		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_Tab:
			// Tab key - save and open new edit 
			if (Fl::event_state(FL_SHIFT)) {
				// SHIFT-TAB - save and select cell to the left
				table->edit_save(SAVE_PREV);
			}
			else {
				// TAB - save and select cell to right
				table->edit_save(SAVE_NEXT);
			}
			return true;
		case FL_Up:
			// Up arrow - save and select record above
			table->edit_save(SAVE_UP);
			return true;
		case FL_Down:
			// Down arrow - save and select record above
			table->edit_save(SAVE_DOWN);
			return true;
		case FL_Shift_L:
		case FL_Shift_R:
			return true;

		}
	}
	return Fl_Input::handle(event);
}


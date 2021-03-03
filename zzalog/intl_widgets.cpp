#include "intl_dialog.h"
#include "intl_widgets.h"

using namespace zzalog;

extern intl_dialog* intl_dialog_;

// Constructor
intl_editor::intl_editor(int X, int Y, int W, int H, const char* label) :
	Fl_Text_Editor(X, Y, W, H, label),
	insert_mode_(true)
{
	// Define standard cursor
	insert_mode(insert_mode_);
	if (insert_mode_) {
		cursor_style(Fl_Text_Display::NORMAL_CURSOR);
	}
	else {
		cursor_style(Fl_Text_Display::BLOCK_CURSOR);
	}
};

intl_editor::~intl_editor() {
	if (intl_dialog_) {
		intl_dialog_->editor(nullptr);
	}
}

// Event handler - handle event as normal then set the cursor depending on current insert mode
int intl_editor::handle(int event) {
	// Tell international character dialog to paste to this widget as this is the most recent one to get focus
	switch (event) {
	case FL_FOCUS:
		// Something has tried to give the editor the focus: accept it.
		if (intl_dialog_) {
			intl_dialog_->editor(this);
		}
		return true;
	default:
		// Default handling of all events
		int result = Fl_Text_Editor::handle(event);
		// Change cursor depending on insert_mode()
		insert_mode_ = insert_mode();
		if (insert_mode_) {
			// INS
			cursor_style(Fl_Text_Display::NORMAL_CURSOR);
		}
		else {
			// OVR
			cursor_style(Fl_Text_Display::BLOCK_CURSOR);
		}
		return result;
	}
}

// Version of Fl_Input - constructor
intl_input::intl_input(int X, int Y, int W, int H, const char* label) :
	Fl_Input(X, Y, W, H, label) {};

intl_input::~intl_input() {
	if (intl_dialog_) {
		intl_dialog_->editor(nullptr);
	}
}

// Event handler
int intl_input::handle(int event) {
	// Tell international character dialog to paste to this widget
	switch (event) {
	case FL_FOCUS:
		if (intl_dialog_) {
			intl_dialog_->editor(this);
		}
		return true;
	default:
		// Do normal handling
		return Fl_Input::handle(event);
	}
}

#include "intl_dialog.h"
#include "intl_widgets.h"

using namespace zzalog;

extern intl_dialog* intl_dialog_;

// Constructor
intl_editor::intl_editor(int X, int Y, int W, int H, const char* label) :
	Fl_Text_Editor(X, Y, W, H, label),
	insert_mode_(true)
{
	insert_mode(insert_mode_);
	if (insert_mode_) {
		cursor_style(Fl_Text_Display::NORMAL_CURSOR);
	}
	else {
		cursor_style(Fl_Text_Display::BLOCK_CURSOR);
	}
};
// Event handler - handle event as normal then set the cursor depending on current insert mode
int intl_editor::handle(int event) {
	// Tell international character dialog to paste to this widget
	if (event == FL_FOCUS && intl_dialog_) {
		intl_dialog_->editor(this);
	}
	int result = Fl_Text_Editor::handle(event);
	insert_mode_ = insert_mode();
	if (insert_mode_) {
		cursor_style(Fl_Text_Display::NORMAL_CURSOR);
	}
	else {
		cursor_style(Fl_Text_Display::BLOCK_CURSOR);
	}
	return result;
}

intl_input::intl_input(int X, int Y, int W, int H, const char* label) :
	Fl_Input(X, Y, W, H, label) {};

// Event handler
int intl_input::handle(int event) {
	// Tell international character dialog to paste to this widget
	if (event == FL_FOCUS && intl_dialog_) {
		intl_dialog_->editor(this);
	}
	// Do normal handling
	return Fl_Input::handle(event);
}

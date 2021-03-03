#include "main_window.h"
#include "menu.h"

using namespace zzalog;

extern menu* menu_;

main_window::main_window(int W, int H, const char* label) :
	Fl_Single_Window(W, H, label)
{
}

main_window::~main_window() {}

// Handle FL_HIDE and FL_SHOW to get menu to update itself
int main_window::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		break;
	}

	return Fl_Single_Window::handle(event);
}

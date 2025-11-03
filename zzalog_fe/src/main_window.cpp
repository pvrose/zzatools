#include "main_window.h"

#include "import_data.h"
#include "main.h"
#include "menu.h"

#include <sstream>

// Create the window
main_window::main_window(int W, int H, const char* label) :
	Fl_Double_Window(W, H, label)
{
}

main_window::~main_window() {}

// Handle FL_HIDE and FL_SHOW to get menu to update itself
// Handle FL_PASTE to import QSOs from the clipboard
int main_window::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		if(menu_) menu_->update_windows_items();
		break;
	case FL_PASTE:
		// Get data from paste
		std::string data = Fl::event_text();
		std::stringstream adif;
		adif.str(data);
		// Stop any extant update and wait for it to complete
		import_data_->stop_update(false);
		while (!import_data_->update_complete()) Fl::check();
		import_data_->load_stream(adif, import_data::update_mode_t::CLIPBOARD);
		// Wait for the import to finish
		while (import_data_->size()) Fl::check();
		return true;
	}

	return Fl_Double_Window::handle(event);
}

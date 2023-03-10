#include "main_window.h"
#include "menu.h"
#include "import_data.h"
#include <sstream>



extern menu* menu_;
extern import_data* import_data_;
extern status* status_;

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
		if(menu_) menu_->update_windows_items();
		return true;
	case FL_PASTE:
		// Get data from paste
		string data = Fl::event_text();
		stringstream adif;
		adif.str(data);
		// Stop any extant update and wait for it to complete
		import_data_->stop_update(false);
		while (!import_data_->update_complete()) Fl::check();
		import_data_->load_stream(adif, import_data::update_mode_t::DATAGRAM);
		int num_records = import_data_->size();
		// Wait for the import to finish
		while (import_data_->size()) Fl::check();
		char message[100];
		snprintf(message, 100, "LOG: %d records copied from clipboard", num_records);
		status_->misc_status(ST_NOTE, message);
		return true;
	}

	return Fl_Single_Window::handle(event);
}

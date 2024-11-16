#include "file_viewer.h"
#include "status.h"

#include "utils.h"
#include "drawing.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

extern status* status_;

// Constructor
file_viewer::file_viewer(int W, int H, const char* L) :
	Fl_Window(W, H, L)
{
	create();
	callback(cb_close);
}

// Desctructor
file_viewer::~ file_viewer() {}

// Create
void file_viewer::create() {
	buffer_ = new Fl_Text_Buffer();
	display_ = new Fl_Text_Display(0, 0, w(), h());
	display_->box(FL_BORDER_BOX);
	display_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	display_->textfont(FL_COURIER);
	display_->textsize(12);

	display_->buffer(buffer_);

	resizable(display_);

	end();
}

// Callback 
void file_viewer::cb_close(Fl_Widget* w, void* v) {
	file_viewer* win = ancestor_view<file_viewer>(w);
	win->hide();
}

// Set text
void file_viewer::load_file(string name) {
	filename_ = name;
	char msg[128];
	switch (buffer_->loadfile(filename_.c_str())) {
		case 1:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to open %s", filename_.c_str(), strerror(errno));
		status_->misc_status(ST_ERROR, msg);
		break;
		case 2:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to read completely", filename_.c_str());
		status_->misc_status(ST_WARNING, msg);
		break;
	}
	copy_label(filename_.c_str());
	show();
}


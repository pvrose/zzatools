#include "file_viewer.h"

#include "utils.h"

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>


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
	display_->buffer(buffer_);

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
	buffer_->loadfile(filename_.c_str());
	copy_label(filename_.c_str());
	show();
}


#include "file_viewer.h"
#include "status.h"

#include "utils.h"
#include "drawing.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>

extern status* status_;

// Constructor
file_viewer::file_viewer(int W, int H, const char* L) :
	Fl_Window(W, H, L)
{
	dirty_ = false;
	filename_ = "";
	create();
	enable_widgets();
	callback(cb_close);
}

// Desctructor
file_viewer::~ file_viewer() {}

// Create
void file_viewer::create() {
	buffer_ = new Fl_Text_Buffer();
	buffer_->add_modify_callback(cb_modified, this);

	int avail_height = h() - HBUTTON;
	display_ = new Fl_Text_Editor(0, 0, w(), avail_height);
	display_->box(FL_BORDER_BOX);
	display_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	display_->linenumber_width(WLABEL);
	display_->textfont(FL_COURIER);
	display_->linenumber_font(FL_COURIER);
	display_->textsize(12);

	display_->buffer(buffer_);

	bn_reload_ = new Fl_Button(w() - (2 * WBUTTON), avail_height, WBUTTON, HBUTTON, "Reload");
	bn_reload_->callback(cb_reload, nullptr);
	bn_reload_->tooltip("Restore original file");

	bn_save_ = new Fl_Button(w() - WBUTTON, avail_height, WBUTTON, HBUTTON, "Save");
	bn_save_->callback(cb_save, nullptr);
	bn_save_->tooltip("SAve the file");

	resizable(display_);

	end();
}

// Enable widgets
void file_viewer::enable_widgets() {
	char l[128];
	if (is_dirty()) {
		bn_reload_->activate();
		bn_save_->activate();
		snprintf(l, sizeof(l), "%s %s", filename_.c_str(), "[Modified]");
		copy_label(l);
	} else {
		bn_reload_->deactivate();
		bn_save_->deactivate();
		copy_label(filename_.c_str());
	}
}

// Callback 
void file_viewer::cb_close(Fl_Widget* w, void* v) {
	file_viewer* win = ancestor_view<file_viewer>(w);
	win->hide();
}

// Reload file
void file_viewer::cb_reload(Fl_Widget* w, void* v) {
	file_viewer* that = ancestor_view<file_viewer>(w);
	int len = that->buffer_->length();
	that->buffer_->remove(0, len);
	that->load_file(that->filename_);
}

// Save file
void file_viewer::cb_save(Fl_Widget* w, void* v) {
	file_viewer* that = ancestor_view<file_viewer>(w);
	that->save_file();
}

// Buffer modified
void file_viewer::cb_modified(int pos, int inserted, int deleted, int restyled, const char* deleteion, void* arg) {
	file_viewer* that = (file_viewer*)arg;
	if (inserted || deleted) {
		that->dirty_ = true;
		that->enable_widgets();
	}
}

// Set text
void file_viewer::load_file(string name) {
	if (is_dirty()) {
		switch(fl_choice("Existing file has been modified - Save or cancel?", "Save", "Cancel", nullptr)) {
			case 0: {
				save_file();
				break;
			}
			case 1: {
				break;
			}
		}
	}
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
	dirty_ = false;
	enable_widgets();
	show();
}

string file_viewer::file() {
	return filename_;
}

// Save file
void file_viewer::save_file() {
	char msg[128];
	switch(buffer_->savefile(filename_.c_str())) {
		case 1:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to open %s", filename_.c_str(), strerror(errno));
		status_->misc_status(ST_ERROR, msg);
		break;
		case 2:
		snprintf(msg, sizeof(msg), "APPS: File %s failed to write completely", filename_.c_str());
		status_->misc_status(ST_WARNING, msg);
		break;
		case 0:
		snprintf(msg, sizeof(msg), "APPS: File %s written OK", filename_.c_str());
		status_->misc_status(ST_NOTE, msg);
		dirty_ = false;
		enable_widgets();
		break; 
	}
}

// Is dirty
bool file_viewer::is_dirty() {
	return dirty_;
}


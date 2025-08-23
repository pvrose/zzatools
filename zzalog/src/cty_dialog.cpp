#include "cty_dialog.h"

#include "cty_data.h"

#include "drawing.h"
#include "utils.h"

#include <chrono>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Output.H>

extern cty_data* cty_data_;
extern string default_data_directory_;
extern void open_html(const char*);

// Check age button clicked
map< cty_data::cty_type_t, chrono::hours > OLD_AGE = {
	{ cty_data::CLUBLOG, chrono::hours(7 * 24) },
	{ cty_data::COUNTRY_FILES, chrono::hours(7 * 24) },
	{ cty_data::DXATLAS, chrono::hours(365 * 24) } };


cty_dialog::cty_dialog(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L)
{
	create_form();
}

cty_dialog::~cty_dialog() {

}

int cty_dialog::handle(int event) {
	int result = Fl_Double_Window::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		if (!result) take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("cty_dialog.html");
			return true;
		}
		break;
	}
	return result;

}

void cty_dialog::create_form() {
	int curr_x = GAP;
	int curr_y = GAP;
	curr_x += WLABEL;

	Fl_Box* b1 = new Fl_Box(curr_x, curr_y, WSMEDIT, HBUTTON, "Timestamp");
	b1->box(FL_FLAT_BOX);

	curr_x += WSMEDIT;
	Fl_Box* b2 = new Fl_Box(curr_x, curr_y, WSMEDIT, HBUTTON, "Version");
	b2->box(FL_FLAT_BOX);

	curr_y += HBUTTON;
	curr_x = GAP;
	Fl_Box* b3 = new Fl_Box(curr_x, curr_y, WLABEL, HBUTTON, "ClubLog");
	b3->box(FL_FLAT_BOX);

	curr_x += WLABEL;
	Fl_Output* o1 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	w_timestamps_[cty_data::CLUBLOG] = o1;

	curr_x += WSMEDIT;
	Fl_Output* o2 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	w_versions_[cty_data::CLUBLOG] = o2;

	curr_x += WSMEDIT;
	Fl_Button* bn_update = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Fetch");
	bn_update->callback(cb_update, (void*)(intptr_t)cty_data::CLUBLOG);
	bn_update->tooltip("Download latest vesrion of Clublog country date");

	curr_y += HBUTTON;
	curr_x = GAP;
	Fl_Box* b4 = new Fl_Box(curr_x, curr_y, WLABEL, HBUTTON, "Country-files");
	b4->box(FL_FLAT_BOX);

	curr_x += WLABEL;
	Fl_Output* o3 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	w_timestamps_[cty_data::COUNTRY_FILES] = o3;

	curr_x += WSMEDIT;
	Fl_Output* o4 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	w_versions_[cty_data::COUNTRY_FILES] = o4;

	curr_x += WSMEDIT;
	Fl_Button* bn_update2 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Update");
	bn_update2->callback(cb_update, (void*)(intptr_t)cty_data::COUNTRY_FILES);
	bn_update2->tooltip("Download latest vesrion of Clublog country date");

	curr_y += HBUTTON;
	curr_x = GAP;
	Fl_Box* b5 = new Fl_Box(curr_x, curr_y, WLABEL, HBUTTON, "DxAtlas");
	b5->box(FL_FLAT_BOX);

	curr_x += WLABEL;
	Fl_Output* o5 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	w_timestamps_[cty_data::DXATLAS] = o5;

	curr_x += WSMEDIT;
	Fl_Output* o6 = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	w_versions_[cty_data::DXATLAS] = o6;

	curr_x += WSMEDIT;
	Fl_Button* bn_update3 = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Update");
	bn_update3->callback(cb_update, (void*)(intptr_t)cty_data::DXATLAS);
	bn_update3->tooltip("Download latest vesrion of Clublog country date");

	curr_y += HBUTTON + GAP;
	curr_x = bn_update->x() - WBUTTON - WBUTTON;

	Fl_Button* bn_browser = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Folder");
	bn_browser->callback(cb_browser);
	bn_browser->tooltip("Open data directory for cty_data files");

	curr_x += WBUTTON;
	Fl_Button* bn_reload = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Reload");
	bn_reload->callback(cb_reload);
	bn_reload->tooltip("Reload updated cty_data files");

	curr_x += WBUTTON;
	Fl_Button* bn_close = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Close");
	bn_close->callback(cb_close);
	bn_close->tooltip("Close this dialog");

	curr_x += WBUTTON + GAP;
	curr_y += HBUTTON + GAP;

	resizable(nullptr);
	size(curr_x, curr_y);
	end();
	update_widgets();
	show();
}

void cty_dialog::update_widgets() {
	chrono::system_clock::time_point now = chrono::system_clock::now();
	
	// Update the timestamp and version
	for (auto it : OLD_AGE) {
		chrono::system_clock::time_point t = cty_data_->timestamp(it.first);
		time_t tt = chrono::system_clock::to_time_t(t);
		tm* ttm = gmtime(&tt);
		char ts[64];
		strftime(ts, sizeof(ts), "%Y-%m-%d", ttm);
		w_timestamps_[it.first]->value(ts);
		if (now - t > it.second) {
			w_timestamps_[it.first]->textcolor(FL_RED);
		}
		else {
			w_timestamps_[it.first]->textcolor(FL_FOREGROUND_COLOR);
		}
		w_versions_[it.first]->value(cty_data_->version(it.first).c_str());
	}
	redraw();
}

// where possible download new data and update the info about them
void cty_dialog::cb_update(Fl_Widget* w, void* v) {
	cty_data::cty_type_t t = (cty_data::cty_type_t)(intptr_t)v;
	cty_dialog* that = ancestor_view<cty_dialog>(w);
	switch (t) {
	case cty_data::CLUBLOG:
		cty_data_->fetch_data(t);
		// Fall through
	default:
		that->update_widgets();
	}
}

// Reload cty_data
void cty_dialog::cb_reload(Fl_Widget* w, void* v) {
	delete cty_data_;
	cty_data_ = new cty_data;
}

// Close the dialog
void cty_dialog::cb_close(Fl_Widget* w, void* v) {
	cty_dialog* that = ancestor_view<cty_dialog>(w);
	default_callback(that, v);
}

// Open directory browser
void cty_dialog::cb_browser(Fl_Widget* w, void* v) {
	Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser->title("ZZALOG Application Data");
	chooser->directory(default_data_directory_.c_str());
	chooser->show();
	delete chooser;
}
#include "stn_dialog.h"

#include "cty_data.h"
#include "init_dialog.h"
#include "main.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "spec_data.h"
#include "stn_call_dlg.h"
#include "stn_data.h"
#include "stn_oper_dlg.h"
#include "stn_qth_dlg.h"

#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tabs.H>

stn_dialog::stn_dialog(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L),
	g_defs_(nullptr),
	g_qth_(nullptr),
	g_oper_(nullptr),
	g_call_(nullptr)
{
	create_form(x(), y());
	load_values();
	enable_widgets();
	redraw();
}

stn_dialog::~stn_dialog() {}

// Handle
int stn_dialog::handle(int event) {
	int result = Fl_Group::handle(event);
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
			open_html("stn_dialog.html");
			return true;
		}
		break;
	}
	return result;
}

// inherited methods
// Standard methods - need to be written for each
// Load values
void stn_dialog::load_values() {
	if (qso_manager_ && qso_manager_->data()) {
		qso_ = qso_manager_->data()->current_qso();
	}
	else {
		qso_ = nullptr;
	}
}

// Used to create the form
void stn_dialog::create_form(int X, int Y) {
	int cx = x() + GAP;
	int cy = y() + GAP;
	int cw = w() - GAP - GAP;

	message_ = new Fl_Box(cx, cy, cw, HBUTTON * 3 / 2);
	message_->box(FL_FLAT_BOX);
	message_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	message_->labelsize(FL_NORMAL_SIZE + 2);

	cy += message_->h();
	cx += (cw / 2) - (WBUTTON / 2);
	Fl_Button* bn_done = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Done!");
	bn_done->callback(cb_done);
	bn_done->tooltip("Close the window to accept the changes");

	cy += HBUTTON;
	cx = x() + GAP;

	int ch = y() + h() - cy;

	// Create an Fl_Tabs
	tabs_ = new Fl_Tabs(cx, cy, cw, ch);
	tabs_->callback(cb_tab);
	tabs_->box(FL_FLAT_BOX);

	int rx = 0, ry = 0, rw = 0, rh = 0;
	// Get individual area
	tabs_->client_area(rx, ry, rw, rh);

	g_defs_ = new init_dialog(rx, ry, rw, rh, "Defaults");
	g_defs_->tooltip("Specify the default station callsign, location and club or operator");

	g_qth_ = new stn_qth_dlg(rx, ry, rw, rh, "QTHs");
	g_qth_->tooltip("Allows the editing of location (QTH) data");

	g_oper_ = new stn_oper_dlg(rx, ry, rw, rh, "Operators");
	g_oper_->tooltip("Specify the operator");

	g_call_ = new stn_call_dlg(rx, ry, rw, rh, "Callsigns");
	g_call_->tooltip("Specify additional callsigns");

	tabs_->end();

	end();
	show();
}

// Used to write settings back
void stn_dialog::save_values() {
	stn_default current;
	current.type = NOT_USED;
	current.callsign = g_call_->get_callsign();
	current.location = g_qth_->get_location();
	current.name = g_oper_->get_operator();
	stn_data_->set_current(current);

	// Make the changes availble in qso_data
	if (qso_manager_) qso_manager_->data()->update_station_choices();
}

// Used to enable/disable specific widget - any widgets enabled musr be attributes
void stn_dialog::enable_widgets() {
	// Standard tab formats
// value() returns the selected widget. We need to test which widget it is.
	Fl_Widget* tab = tabs_->value();
	for (int ix = 0; ix < tabs_->children(); ix++) {
		Fl_Widget* wx = tabs_->child(ix);
		if (wx == tab) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
			wx->activate();
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
			wx->deactivate();
		}
	}
	g_defs_->enable_widgets();
	g_qth_->enable_widgets();
	g_oper_->enable_widgets();
	g_call_->enable_widgets();
}

// Switch tabs
void stn_dialog::cb_tab(Fl_Widget* w, void* v) {
	stn_dialog* that = ancestor_view<stn_dialog>(w);
	that->enable_widgets();
}

// Done
void stn_dialog::cb_done(Fl_Widget* w, void* v) {
	stn_dialog* that = ancestor_view<stn_dialog>(w);
	stn_window* win = ancestor_view<stn_window>(that);
	that->save_values();
	win->hide();
}

// Set tab
void stn_dialog::set_tab(tab_type t, std::string id, std::string message) {
	switch (t) {
	case DEFAULTS:
		g_defs_->activate();
		tabs_->value(g_defs_);
		break;
	case QTH:
		g_qth_->activate();
		g_qth_->set_location(id);
		tabs_->value(g_qth_);
		break;
	case OPERATOR:
		g_oper_->activate();
		g_oper_->set_operator(id);
		tabs_->value(g_oper_);
		break;
	case CALLSIGN:
		g_call_->activate();
		g_call_->set_callsign(id);
		tabs_->value(g_call_);
		break;
	}
	message_->copy_label(message.c_str());
	enable_widgets();
}

//! Update other widgets
void stn_dialog::update() {
	qso_manager_->enable_widgets();
}
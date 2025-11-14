#include "qso_clocks.h"

#include "condx_view.h"
#include "main.h"
#include "qso_clock.h"
#include "qso_manager.h"
#include "qso_wx.h"
#include "settings.h"
#include "status.h"
#include "wsjtx_handler.h"

#include "utils.h"

#include <algorithm>

#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Tabs.H>

// Constructor
qso_clocks::qso_clocks(int X, int Y, int W, int H, const char* L) :
    Fl_Group(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	box(FL_BORDER_BOX);
	load_values();
	create_form();
    enable_widgets();
}

// Destructor
qso_clocks::~qso_clocks() {
	save_values();
}

// Handle
int qso_clocks::handle(int event) {
	int result = Fl_Group::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		Fl_Group::take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("qso_clocks.html");
			return true;
		}
		break;
	}
	return result;
}


void qso_clocks::load_values() {
	// Load default tab value
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings dash_settings(&view_settings, "Dashboard");
	settings condx_settings(&dash_settings, "Conditions");
	condx_settings.get("Local Timezone", local_, false);
}

// Create two tabs - one each for UTC and local timezone
void qso_clocks::create_form() {

	int cx = x() + GAP;
	int cy = y() + HTEXT;

	clock_ = new qso_clock(cx, cy, 50, 50);
	clock_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	// clock_->callback(cb_clock, nullptr);
	// clock_->when(FL_WHEN_RELEASE);
	clock_->local(local_);

	cx += clock_->w();

	int max_w = clock_->x() + clock_->w() - x();
	int max_h = clock_->y() + clock_->h() - y();

	cy += clock_->h() + GAP;
	cx = x() + GAP;
	int cw = max_w - GAP;

	tabs_ = new Fl_Tabs(cx, cy, cw, 50);
	tabs_->callback(cb_tabs);
	tabs_->box(FL_BORDER_BOX);
	tabs_->labeltype(FL_NO_LABEL);
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	tabs_->client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;

	qso_weather_ = new qso_wx(rx, ry, rw, rh, "Weather");
	qso_weather_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	rw = std::max<int>(rw, qso_weather_->w());
	rh = std::max<int>(rh, qso_weather_->h());

	condx_ = new condx_view(rx, ry, rw, rh, "Conditions");
	condx_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	rw = std::max<int>(rw, condx_->w());
	rh = std::max<int>(rh, condx_->h());

	tabs_->resizable(nullptr);
	tabs_->size(tabs_->w() + rw - saved_rw, tabs_->h() + rh - saved_rh);
	tabs_->end();

	for (int ix = 0; ix < tabs_->children(); ix++) {
		tabs_->child(ix)->size(rw, rh);
	}

	max_h = tabs_->y() + tabs_->h() - y();

	enable_widgets();
	resizable(nullptr);
	size(max_w + GAP, max_h + GAP);
	end();
}

void qso_clocks::save_values() {
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings dash_settings(&view_settings, "Dashboard");
	settings condx_settings(&dash_settings, "Conditions");
	// Find the current selected tab and save its index
	condx_settings.set("Local Timezone", local_);
}

// Enable the widgets
void qso_clocks::enable_widgets() {
	// Set standard tab label formats
	for (int ix = 0; ix < tabs_->children(); ix++) {
		Fl_Widget* wx = tabs_->child(ix);
		if (wx == tabs_->value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}
	local_ = clock_->local();
	// Enable the two qso_clock widgets
	clock_->enable_widgets();
	qso_weather_->enable_widgets();
	redraw();
}

bool qso_clocks::is_local() { return local_; }

qso_wx* qso_clocks::wx() {
	return qso_weather_;
}

void qso_clocks::cb_tabs(Fl_Widget* w, void* v) {
	Fl_Tabs* that = (Fl_Tabs*)w;
	that->label(that->value()->label());
	((qso_clocks*)that->parent())->enable_widgets();

}
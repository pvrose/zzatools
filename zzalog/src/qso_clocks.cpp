#include "qso_clocks.h"

#include "qso_clock.h"
#include "qso_manager.h"
#include "qso_wx.h"
#include "status.h"
#include "wsjtx_handler.h"

#include "utils.h"

#include <algorithm>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Radio_Round_Button.H>

using namespace std;

// basic tick is 200 ms 
extern status* status_;
extern wsjtx_handler* wsjtx_handler_;
extern bool closing_;
extern string VENDOR;
extern string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;
extern void open_html(const char*);

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
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	int tempi;
	tab_settings.get("Clocks", tempi, 0);
	local_ = (bool)tempi;
}

// Create two tabs - one each for UTC and local timezone
void qso_clocks::create_form() {

	int cx = x() + GAP;
	int cy = y() + HTEXT;

	utc_clock_ = new qso_clock(cx, cy, 50, 50, false);
	utc_clock_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	local_clock_ = new qso_clock(cx, cy, 50, 50, true);
	local_clock_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

	cx += utc_clock_->w();

	Fl_Group* g1 = new Fl_Group(cx, cy, HBUTTON, HBUTTON * 2);
	Fl_Radio_Round_Button* r1 = new Fl_Radio_Round_Button(cx, cy, HBUTTON, HBUTTON);
	r1->value(!local_);
	r1->callback(cb_radio, (void*)(intptr_t)false);
	r1->tooltip("Click to select UTC time");

	cy += HBUTTON;
	Fl_Radio_Round_Button* r2 = new Fl_Radio_Round_Button(cx, cy, HBUTTON, HBUTTON);
	r2->value(local_);
	r2->callback(cb_radio, (void*)(intptr_t)true);
	r2->tooltip("Click to select local time");

	g1->end();


	int max_w = g1->x() + g1->w() - x();
	int max_h = g1->y() + g1->h() - y();

	cy += utc_clock_->h();
	cx = x() + GAP;
	int cw = max_w - GAP;

	qso_weather_ = new qso_wx(cx, cy, cw, 50, "Weather");
	qso_weather_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

	max_w = max(max_w, qso_weather_->x() + qso_weather_->w() - x());
	max_h = max(max_h, qso_weather_->y() + qso_weather_->h() - y());

	enable_widgets();
	resizable(nullptr);
	size(max_w + GAP, max_h + GAP);
	end();
}

void qso_clocks::save_values() {
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	// Find the current selected tab and save its index
	tab_settings.set("Clocks", (int)local_);
}

// Enable the widgets
void qso_clocks::enable_widgets() {
	// Enable the two qso_clock widgets
	utc_clock_->enable_widgets();
	local_clock_->enable_widgets();	
	if (local_) {
		utc_clock_->hide();
		local_clock_->show();
	} else {
		utc_clock_->show();
		local_clock_->hide();
	}
	qso_weather_->enable_widgets();
}

void qso_clocks::cb_radio(Fl_Widget* w, void* v) {
	qso_clocks* that = ancestor_view<qso_clocks>(w);
	that->local_ = (bool)(uintptr_t)v;
	that->enable_widgets();
}

bool qso_clocks::is_local() { return local_; }
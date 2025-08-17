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

	clock_ = new qso_clock(cx, cy, 50, 50);
	clock_->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
	// clock_->callback(cb_clock, nullptr);
	// clock_->when(FL_WHEN_RELEASE);
	clock_->local(local_);

	cx += clock_->w();

	int max_w = clock_->x() + clock_->w() - x();
	int max_h = clock_->y() + clock_->h() - y();

	cy += clock_->h() + HTEXT;
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
	clock_->enable_widgets();
	qso_weather_->enable_widgets();
}

void qso_clocks::cb_clock(Fl_Widget* w, void* v) {
	qso_clock* clock = (qso_clock*)w;
	qso_clocks* that = ancestor_view<qso_clocks>(w);
	that->local_ = !clock->local();
	clock->local(that->local_);
	that->enable_widgets();
}

bool qso_clocks::is_local() { return local_; }
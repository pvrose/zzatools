#include "qso_clocks.h"
#include "qso_manager.h"
#include "utils.h"
#include "status.h"
#include "wsjtx_handler.h"

#include <algorithm>

using namespace std;

// basic tick is 200 ms 
extern status* status_;
extern wsjtx_handler* wsjtx_handler_;
extern bool closing_;

// Constructor
qso_clocks::qso_clocks(int X, int Y, int W, int H, const char* L) :
    Fl_Tabs(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	create_form();
	callback(cb_tabs);
    enable_widgets();
}

// Destructor
qso_clocks::~qso_clocks() {
}

// Create two tabs - one each for UTC and local timezone
void qso_clocks::create_form() {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;
	utc_clock_ = new qso_clock(rx, ry, rw, rh, false);
	// All versions of qso_clock should be the same size, but...
	rw = max(rw, utc_clock_->w());
	rh = max(rh, utc_clock_->h());

	local_clock_ = new qso_clock(rx, ry, rw, rh, true);
	rw = max(rw, utc_clock_->w());
	rh = max(rh, utc_clock_->h());

	resizable(nullptr);
	size(w() + rw - saved_rw, h() + rh - saved_rh);
	end();
}

// Enable the widgets
void qso_clocks::enable_widgets() {
	// Set the label of the selected tab to BOLD, others to ITALIC
	for (int ix = 0; ix < children(); ix++) {
		Fl_Widget* wx = child(ix);
		if (wx == value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}
	// Enable the two qso_clock widgets
	utc_clock_->enable_widgets();
	local_clock_->enable_widgets();	
}

// 1 Hz ticker - enables both clocks
void qso_clocks::ticker() {
	enable_widgets();
}

// Selecting either tab - redraws with the new tab selected
void qso_clocks::cb_tabs(Fl_Widget* w, void* v) {
	qso_clocks* that = ancestor_view<qso_clocks>(w);
	that->enable_widgets();
}
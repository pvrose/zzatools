#include "qso_clocks.h"
#include "qso_manager.h"
#include "utils.h"
#include "status.h"

#include <algorithm>

using namespace std;

// basic tick is 200 ms 
const double BASIC_TICK = 0.1;
unsigned int qso_clocks::tick_count_ = 0;
extern status* status_;
extern bool closing_;

qso_clocks::qso_clocks(int X, int Y, int W, int H, const char* L) :
    Fl_Tabs(X, Y, W, H, L)
{
	labeltype(FL_NO_LABEL);
	create_form();
	callback(cb_tabs);
    enable_widgets();
	Fl::add_timeout(0, cb_ticker, this);
}

qso_clocks::~qso_clocks() {
    Fl::remove_timeout(cb_ticker);
}

void qso_clocks::create_form() {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;
	utc_clock_ = new qso_clock(rx, ry, rw, rh, false);
	// All versions of qso_rig should be the same size, but...
	rw = max(rw, utc_clock_->w());
	rh = max(rh, utc_clock_->h());

	local_clock_ = new qso_clock(rx, ry, rw, rh, true);
	rw = max(rw, utc_clock_->w());
	rh = max(rh, utc_clock_->h());

	resizable(nullptr);
	size(w() + rw - saved_rw, h() + rh - saved_rh);
	end();
}

void qso_clocks::enable_widgets() {
	// Set the label of the selected tab to BOLD, others to ITALIC
	for (int ix = 0; ix < children(); ix++) {
		Fl_Widget* wx = child(ix);
		if (wx == value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
		}
	}
	utc_clock_->enable_widgets();
	local_clock_->enable_widgets();	
}

void qso_clocks::cb_ticker(void* v) {
	if (!closing_) {
		qso_clocks* that = (qso_clocks*)v;
		tick_count_++;
		if (tick_count_ % 10 == 0) {
			that->enable_widgets();
			((qso_manager*)that->parent())->ticker();
		}
		if (tick_count_ % 2 == 0) {
			status_->ticker();
		}

		Fl::repeat_timeout(BASIC_TICK, cb_ticker, v);
	}
}

void qso_clocks::stop_ticker() {
	Fl::remove_timeout(cb_ticker);
}

void qso_clocks::cb_tabs(Fl_Widget* w, void* v) {
	qso_clocks* that = ancestor_view<qso_clocks>(w);
	that->enable_widgets();
}
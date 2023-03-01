#include "QBS_reporter.h"

QBS_reporter::QBS_reporter(int W, int H, const char* L) :
	Fl_Window(W, H, L) {
	callback(cb_hide);
	// Add display
	display_ = new Fl_Help_View(0, 0, w(), h());
	display_->box(FL_FLAT_BOX);

	resizable(display_);
	end();
}

QBS_reporter::~QBS_reporter() {
	delete display_;
}

void QBS_reporter::text(const char* value) {
	display_->value(value);
}

void QBS_reporter::cb_hide(Fl_Widget* w) {
	w->hide();
}
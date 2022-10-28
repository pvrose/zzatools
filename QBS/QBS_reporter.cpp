#include "QBS_reporter.h"

QBS_reporter::QBS_reporter(int W, int H, const char* L) :
	Fl_Window(W, H, L) {
	// No border, always on top
	set_non_modal();
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
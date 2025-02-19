#include "qso_bands.h"

#include "band_widget.h"
#include "band_window.h"
#include "qso_manager.h"
#include "rig_if.h"

#include "drawing.h"
#include "utils.h"

qso_bands::qso_bands(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	load_values();
	create_form();
	enable_widgets();
}

qso_bands::~qso_bands() {
	// Hide the window so that it will be closed 
	if (full_window_) full_window_->hide();
}

// LLoad settings
void qso_bands::load_values() {
	// TODO: Add previous window size (& position)
}
// Create widgets
void qso_bands::create_form() {
	// CReate the window
	Fl_Group::current(nullptr);
	full_window_ = new band_window(300, 400, "Band Plan");
	full_window_->hide();

	begin();
	int cx = x() + GAP;
	int cy = y() + GAP;
	int cw = w() - GAP - GAP;
	int ch = h() - GAP - GAP;
	summary_ = new band_widget(cx, cy, cw, ch);
	summary_->box(FL_BORDER_FRAME);
	summary_->callback(cb_band);
	summary_->tooltip("Band plan display - click to open larger view");
	end();
	resizable(summary_);
}

// Save settimngs
void qso_bands::save_values() {
	// TODO:
}

// Configure widgets
void qso_bands::enable_widgets() {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	rig_if* rig = mgr->rig();
	double f = rig->get_dfrequency(true);
	summary_->value(f);
	full_window_->set_frequency(f);
}


void qso_bands::cb_band(Fl_Widget* w, void* v) {
	qso_bands* that = ancestor_view<qso_bands>(w);
	if (that->full_window_->visible()) {
		that->full_window_->hide();
	}
	else {
		that->full_window_->show();
	}
}

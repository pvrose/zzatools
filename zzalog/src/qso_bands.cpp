#include "qso_bands.h"

#include "band_widget.h"
#include "band_window.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "rig_if.h"
#include "ticker.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Preferences.H>

extern ticker* ticker_;
extern Fl_Preferences* settings_;
extern string PROGRAM_ID;
extern string PROGRAM_VERSION;
extern bool DARK;

qso_bands::qso_bands(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	load_values();
	create_form();
	enable_widgets();
	// Set up to read rig every 1 s.
	ticker_->add_ticker(this, cb_ticker, 10);
}

qso_bands::~qso_bands() {
	save_values();
	// Hide the window so that it will be closed 
	if (full_window_) full_window_->hide();
}

// LLoad settings
void qso_bands::load_values() {
	Fl_Preferences my_settings(settings_, "Windows/Bandplan");
	int temp;
	my_settings.get("Open Automatically", temp, (int)false);
	open_window_ = (bool)temp;
	my_settings.get("Left", left_, 0);
	my_settings.get("Top", top_, 0);
	my_settings.get("Width", width_, 300);
	my_settings.get("Height", height_, 400);
}

// Create widgets
void qso_bands::create_form() {
	// CReate the window
	Fl_Group::current(nullptr);
	char l[128];
	snprintf(l,sizeof(l), "%s %s: Bandplan", PROGRAM_ID.c_str(), PROGRAM_VERSION.c_str());
	full_window_ = new band_window(left_, top_, width_, height_);
	full_window_->copy_label(l);
	full_window_->color(DARK ? COLOUR_ORANGE : FL_RED, DARK ? FL_GREEN : FL_DARK_GREEN);
	if (open_window_) full_window_->show();
	else full_window_->hide();

	begin();
	int cx = x() + GAP;
	int cy = y() + GAP;
	int cw = w() - GAP - GAP;
	int ch = h() - GAP - GAP;
	summary_ = new band_widget(cx, cy, cw, ch);
	summary_->type(band_widget::BAND_SUMMARY);
	summary_->color(DARK ? COLOUR_ORANGE : FL_RED, DARK ? FL_GREEN : FL_DARK_GREEN);
	summary_->box(FL_BORDER_FRAME);
	summary_->callback(cb_band);
	summary_->tooltip("Band plan display - click to open larger view");
	end();
	resizable(summary_);
}

// Save settimngs
void qso_bands::save_values() {
	Fl_Preferences my_settings(settings_, "Windows/Bandplan");
	my_settings.set("Open Automatically", (int)open_window_);
	my_settings.set("Left", full_window_->x_root());
	my_settings.set("Top", full_window_->y_root());
	my_settings.set("Width", full_window_->w());
	my_settings.set("Height", full_window_->h());
}

// Configure widgets
void qso_bands::enable_widgets() {
}


void qso_bands::cb_band(Fl_Widget* w, void* v) {
	qso_bands* that = ancestor_view<qso_bands>(w);
	if (that->full_window_->visible()) {
		that->full_window_->hide();
		that->open_window_ = false;
	}
	else {
		that->full_window_->show();
		that->open_window_ = true;
	}
}

void qso_bands::cb_ticker(void* v) {
	qso_bands* that = (qso_bands*)v;
	qso_manager* mgr = ancestor_view<qso_manager>(that);
	rig_if* rig = mgr->rig();
	if (rig && rig->is_good()) {
		double tx = rig->get_dfrequency(true);
		double rx = rig->get_dfrequency(false);
		that->summary_->value(tx, rx);
		that->full_window_->set_frequency(tx, rx);
	} else {
		record* qso = mgr->data()->current_qso();
		if (qso) {
			double tx;
			qso->item("FREQ", tx);
			that->summary_->value(tx, 0.0);
			that->full_window_->set_frequency(tx, 0.0);
		} else {
			that->summary_->value(0.0, 0.0);
			that->full_window_->set_frequency(0.0, 0.0);
		}
	}

}
#include "band_window.h"

#include "band_widget.h"
#include "qso_manager.h"
#include "rig_if.h"
#include "status.h"

#include "drawing.h"

#include <FL/Fl_Box.H>

extern std::string CONTACT;
extern std::string COPYRIGHT;
extern qso_manager* qso_manager_;
extern status* status_;

band_window::band_window(int X, int Y, int W, int H, const char* L) :
	Fl_Double_Window(X, Y, W, H, L)
{
	bw_ = new band_widget(0, 0, W, H - FOOT_HEIGHT);
	bw_->type(band_widget::BAND_FULL | band_widget::ZOOMABLE);
	bw_->selection_color(selection_color());
	bw_->callback(cb_widget);
	bw_->when(FL_WHEN_RELEASE);

	Fl_Box* b_cr = new Fl_Box(0, H - FOOT_HEIGHT, W, FOOT_HEIGHT);
	b_cr->box(FL_FLAT_BOX);
	b_cr->copy_label(std::string(COPYRIGHT + " " + CONTACT + "     ").c_str());
	b_cr->labelsize(FL_NORMAL_SIZE - 1);
	b_cr->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	bw_->show();

	end();
	resizable(bw_);
	show();

}

band_window::~band_window() {}

// Overload draw
void band_window::draw() {
	// Pass on selection colour to the widget
	bw_->color(color(), selection_color());
	Fl_Double_Window::draw();
}

void band_window::set_frequency(double tx, double rx) {
	bw_->value(tx, rx);
}

// Callback from 
void band_window::cb_widget(Fl_Widget* w, void* v) {
	band_widget* bw = (band_widget*)w;
	double f = bw->frequency(Fl::event_x(), Fl::event_y());
	if (!isnan(f)) {
		rig_if* rig = qso_manager_->rig();
		if (rig) {
			if (!rig->set_frequency(f)) {
				char msg[128];
				snprintf(msg, sizeof(msg), "BAND: Unable to std::set frequency %g", f);
				status_->misc_status(ST_WARNING, msg);
			}
		}
	}
}
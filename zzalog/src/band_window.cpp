#include "band_window.h"

#include "band_widget.h"

band_window::band_window(int X, int Y, int W, int H, const char* L) :
	Fl_Double_Window(X, Y, W, H, L)
{
	bw_ = new band_widget(0, 0, W, H);
	bw_->type(band_widget::BAND_FULL);
	bw_->selection_color(FL_RED);
	bw_->show();

	end();
	resizable(bw_);
	show();

}

band_window::~band_window() {}

void band_window::set_frequency(double f) {
	bw_->value(f);
}
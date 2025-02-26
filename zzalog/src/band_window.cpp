#include "band_window.h"

#include "band_widget.h"

band_window::band_window(int X, int Y, int W, int H, const char* L) :
	Fl_Double_Window(X, Y, W, H, L)
{
	bw_ = new band_widget(0, 0, W, H);
	bw_->type(band_widget::BAND_FULL);
	bw_->selection_color(selection_color());
	bw_->show();

	end();
	resizable(bw_);
	show();

}

band_window::~band_window() {}

// Overload draw
void band_window::draw() {
	// Pass on selection colour to the widget
	bw_->selection_color(selection_color());
	Fl_Double_Window::draw();
}

void band_window::set_frequency(double f) {
	bw_->value(f);
}
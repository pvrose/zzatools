#include "QBS_charth.h"
#include "QBS_data.h"

#include <set>

#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/names.h>


using namespace std;

const Fl_Color COLOUR_RECEIVED = FL_YELLOW;
const Fl_Color COLOUR_RECYCLED = FL_RED;
const Fl_Color COLOUR_SENT = FL_GREEN;
const int AXIS_WIDTH = 16;
const int LEGEND_HEIGHT = 7;

QBS_charth::QBS_charth(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	data_ = nullptr;
	max_ = 15;
	number_boxes_ = 0;
	start_box_ = 0;
	win_tip_ = nullptr;
	create_form();
}

QBS_charth::~QBS_charth() {

}

void QBS_charth::create_form() {
	box(FL_FLAT_BOX);
	color(FL_BACKGROUND2_COLOR);

	// Allow space foe axis
	int curr_x = x() + AXIS_WIDTH;
	int curr_w = w() - AXIS_WIDTH;

	chart_ = new Fl_Chart(curr_x, y(), curr_w, h() - LEGEND_HEIGHT - HBUTTON);
	chart_->box(FL_NO_BOX);
	chart_->type(FL_BAR_CHART);
	chart_->autosize(true);

	scroll_ = new Fl_Scrollbar(x(), chart_->y() + chart_->h() + LEGEND_HEIGHT, w(), HBUTTON);
	scroll_->type(FL_HORIZONTAL);
	scroll_->callback(cb_scroll, nullptr);

	end();
}

void QBS_charth::data(QBS_data* d) {
	data_ = d;
}

void QBS_charth::update(string call) {
	call_ = call;
	stop_box_= data_->get_current();
	start_box_ = stop_box_ > 11 ? stop_box_ - 11 : 0;
	head_box_ = data_->get_head();
	number_boxes_ = stop_box_ - start_box_ + 1;

	if (number_boxes_ > 11) {
		scroll_->activate();
		scroll_->range(12, stop_box_);
		scroll_->value(stop_box_);
	} else {
		scroll_->deactivate();
	}
	draw_chart();

}

void QBS_charth::draw_chart() {

	max_ = 0;

	chart_counts_.resize(number_boxes_ * 4);

	chart_->clear();

	char err_msg[64];

	for (int b = start_box_, ix = 0; b <= stop_box_; b++, ix++) {
		string box_name = data_->get_batch(b);
		string label = "";
		if (box_name.substr(5, 2) == "Q1") {
			label = box_name.substr(2, 2);
		}
		box_data* box = data_->get_box(b);
		// Add received data
		int rcvd = 0;
		if (box->received->find(call_) != box->received->end()) {
			rcvd = box->received->at(call_);
			if (rcvd < 0) {
				snprintf(err_msg, sizeof(err_msg), "Negative value %d received Box %s\n", rcvd, box_name.c_str());
				clog << err_msg;
				rcvd = 0;
			}
			max_ = max(max_, rcvd);
		}
		chart_counts_[ix] = rcvd;
		chart_->add((double)rcvd, label.c_str(), COLOUR_RECEIVED);
		// Add recycled and sent data
		int rcyc = 0;
		int sent = 0;
		if (box->sent->find(call_) != box->sent->end()) {
			sent = box->sent->at(call_);
			if (sent < 0) {
				snprintf(err_msg, sizeof(err_msg), "Negative value %d sent Box %s\n", sent, box_name.c_str());
				clog << err_msg;
				sent = 0;
			}
		}
		ix++;
		chart_counts_[ix] = sent;
		chart_->add((double)sent, "", COLOUR_SENT);
		if (b < head_box_ && box->counts->find(call_) != box->counts->end()) {
			rcyc = box->counts->at(call_);
			if (rcyc < 0) {
				snprintf(err_msg, sizeof(err_msg), "Negative value %d recycled Box %s\n", rcyc, box_name.c_str());
				clog << err_msg;
				rcyc = 0;
			}
		}
		ix++;
		chart_counts_[ix] = rcyc;
		max_ = max(max_, rcyc);
		chart_->add((double)(rcyc), "", COLOUR_RECYCLED);

		ix++;
		chart_counts_[ix] = 0;
		chart_->add(0, "", color());
	}

	if (max_ > 50) max_ = 100;
	else if(max_ > 25) max_ = 50;
	else if (max_ > 15) max_ = 25;
	else max_ = 15;

	// Set boiunds of recycled and sent to those of received
	chart_->bounds(0, max_);
	redraw();
}

void QBS_charth::draw_y_axis() {
	// Draw the SWR axis
	fl_color(FL_FOREGROUND_COLOR);
	int ax = x() + AXIS_WIDTH;
	int dh = fl_height();
	int ay = y();
	int ah = chart_->h() - dh - 1;

	fl_line(ax, ay, ax, ay + ah);

	// Now add the ticks - generate values
	set<double> ticks;
	ticks.clear();
	double tick = 0.0;
	double gap;
	if (max_ >= 100) {
		gap = 50.0;
	}
	else if (max_ >= 50) {
		gap = 20.0;
	}
	else if (max_ >= 20) {
		gap = 10.0;
	}
	else if (max_ >= 10) {
		gap = 5.0;
	}
	else {
		gap = 2.0;
	}
	while (tick <= max_) {
		ticks.insert(tick);
		tick += gap;
	}
	double range = max_;
	double pixel_per_unit = ah / range;
	// For each tick
	for (auto it = ticks.begin(); it != ticks.end(); it++) {
		int ty = ay + ah - (int)round(*it * pixel_per_unit);
		// Draw the tick
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(x(), ty, ax, ty);
		// Label the tick
		char l[10];
		snprintf(l, sizeof(l), "%.0f", *it);
		fl_draw(l, x() + 1, ty - 1);
	}
}

void QBS_charth::draw_legend() {
	int x = Fl_Group::x();
	int y = Fl_Group::y() + chart_->h();

	fl_color(COLOUR_RECEIVED);
	fl_rectf(x + 1, y + 1, LEGEND_HEIGHT -1, LEGEND_HEIGHT - 1);
	fl_color(FL_FOREGROUND_COLOR);
	x += LEGEND_HEIGHT;
	fl_draw("RCVD", x, y + LEGEND_HEIGHT - 1);
	x += w()/3 - LEGEND_HEIGHT;

	fl_color(COLOUR_SENT);
	fl_rectf(x + 1, y + 1, LEGEND_HEIGHT -1, LEGEND_HEIGHT - 1);
	fl_color(FL_FOREGROUND_COLOR);
	x += LEGEND_HEIGHT;
	fl_draw("SENT", x, y + LEGEND_HEIGHT - 1);
	x += w()/3 - LEGEND_HEIGHT;

	fl_color(COLOUR_RECYCLED);
	fl_rectf(x + 1, y + 1, LEGEND_HEIGHT -1, LEGEND_HEIGHT - 1);
	fl_color(FL_FOREGROUND_COLOR);
	x += LEGEND_HEIGHT;
	fl_draw("RCYC", x, y + LEGEND_HEIGHT - 1);
}

void QBS_charth::draw() {
	Fl_Group::draw();
	draw_y_axis();
	draw_legend();
}

//Place holder for now
int QBS_charth::handle(int event) {
	switch(event) {
		case FL_PUSH: {
			int x = Fl::event_x() - chart_->x();
			int y = Fl::event_y() - chart_->y();
			if (x > 0 && x < chart_->w() &&
				y > 0 && y < chart_->h()) {
				chart_tip();
				return true;
			}
		}
		case FL_RELEASE: {
			if (win_tip_)  {
				Fl::delete_widget(win_tip_);
				win_tip_ = nullptr;
				return true;
			}
		}
	}
	return Fl_Group::handle(event);
}

void QBS_charth::chart_tip() {
	int x = Fl::event_x() - chart_->x();
	float bar_width = (float)chart_->w() / (float)number_boxes_;
	int bar = (int)trunc((float)x / bar_width);
	int rcvd = chart_counts_[bar * 4];
	int rcyc = chart_counts_[bar * 4 + 1];
	int sent = chart_counts_[bar * 4 + 2];
	int i_box = start_box_ + bar;
	string id = data_->get_box(i_box)->id;

	// Create text for tooltip
	char text[256];
	snprintf(text, sizeof(text), "Box %d %s\nReceived %d\nSent %d\nRecycled %d",
		i_box, id.c_str(), rcvd, sent, rcyc); 
	// get the size of the text, set the font, default width
	fl_font(Fl_Tooltip::font(), Fl_Tooltip::size());
	int width = Fl_Tooltip::wrap_width();
	int height = 0;
	// Now get the actual width and height
	fl_measure(text, width, height, 0);
	// adjust sizes to allow for margins - use Fl_Tooltip margins.
	width += (2 * Fl_Tooltip::margin_width());
	height += (2 * Fl_Tooltip::margin_height());
	// Create the window
	win_tip_ = new Fl_Window(Fl::event_x_root(), Fl::event_y_root(), width, height, 0);
	win_tip_->clear_border();
	
	// Create the output widget.
	Fl_Multiline_Output* op = new Fl_Multiline_Output(0, 0, width, height, 0);
	// Copy the attributes of tool-tips
	op->color(Fl_Tooltip::color());
	op->textcolor(Fl_Tooltip::textcolor());
	op->textfont(Fl_Tooltip::font());
	op->textsize(Fl_Tooltip::size());
	op->wrap(true);
	op->box(FL_BORDER_BOX);
	op->value(text);
	win_tip_->add(op);
	win_tip_->end();
	// set the window parameters: always on top, tooltip
	win_tip_->set_non_modal();
	win_tip_->set_tooltip_window();
	// Must be after set_tooltip_window.
	win_tip_->show();

}

void QBS_charth::cb_scroll(Fl_Widget* w, void* v) {
	QBS_charth* that = ancestor_view<QBS_charth>(w);
	Fl_Scrollbar* bar = (Fl_Scrollbar*)w;
	that->stop_box_ = bar->value();
	that->start_box_ = that->stop_box_ - that->number_boxes_ + 1;
	that->draw_chart();
}
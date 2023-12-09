#include "QBS_charth.h"
#include "QBS_data.h"

#include <set>

using namespace std;

const Fl_Color COLOUR_RECEIVED = FL_YELLOW;
const Fl_Color COLOUR_RECYCLED = FL_RED;
const Fl_Color COLOUR_SENT = FL_GREEN;
const int AXIS_WIDTH = 16;

QBS_charth::QBS_charth(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	data_ = nullptr;
	max_ = 15.0;
	create_form();
}

QBS_charth::~QBS_charth() {

}

void QBS_charth::create_form() {
	begin();
	box(FL_FLAT_BOX);
	color(FL_BACKGROUND2_COLOR);

	// Allow space foe axis
	int curr_x = x() + AXIS_WIDTH;
	int curr_w = w() - AXIS_WIDTH;

	// Create the charts on top of each other - assumes draw order will remain
	ct_received_ = new Fl_Chart(curr_x, y(), curr_w, h());
	ct_received_->box(FL_NO_BOX);
	ct_received_->type(FL_BAR_CHART);
	ct_received_->autosize(true);

	ct_recycled_ = new Fl_Chart(curr_x, y(), curr_w, h());
	ct_recycled_->box(FL_NO_BOX);
	ct_recycled_->type(FL_BAR_CHART);
	ct_recycled_->autosize(true);

	ct_sent_ = new Fl_Chart(curr_x, y(), curr_w, h());
	ct_sent_->box(FL_NO_BOX);
	ct_sent_->type(FL_BAR_CHART);
	ct_sent_->autosize(true);

	end();
}

void QBS_charth::data(QBS_data* d) {
	data_ = d;
}

void QBS_charth::update(string call) {
	int stop_box = data_->get_current();
	int start_box = stop_box > 11 ? stop_box - 11 : 0;
	int head_box = data_->get_head();

	ct_received_->clear();
	ct_recycled_->clear();
	ct_sent_->clear();

	char err_msg[64];

	for (int b = start_box; b <= stop_box; b++) {
		string box_name = data_->get_batch(b);
		string label = "";
		if (box_name.substr(5, 2) == "Q1") {
			label = box_name.substr(2, 2);
		}
		//else {
		//	label = box_name.substr(5, 2);
		//}
		box_data* box = data_->get_box(b);
		// Add received data
		int rcvd = 0;
		if (box->received->find(call) != box->received->end()) {
			rcvd = box->received->at(call);
			if (rcvd < 0) {
				snprintf(err_msg, sizeof(err_msg), "Negative value %d received Box %s\n", rcvd, box_name.c_str());
				clog << err_msg;
				rcvd = 0;
			}
		}
		ct_received_->add((double)rcvd, label.c_str(), COLOUR_RECEIVED);
		// Add recycled and sent data
		int rcyc = 0;
		int sent = 0;
		if (box->sent->find(call) != box->sent->end()) {
			sent = box->sent->at(call);
			if (sent < 0) {
				snprintf(err_msg, sizeof(err_msg), "Negative value %d sent Box %s\n", sent, box_name.c_str());
				clog << err_msg;
				sent = 0;
			}
		}
		if (b < head_box && box->counts->find(call) != box->counts->end()) {
			rcyc = box->counts->at(call);
			if (rcyc < 0) {
				snprintf(err_msg, sizeof(err_msg), "Negative value %d recycled Box %s\n", rcyc, box_name.c_str());
				clog << err_msg;
				rcyc = 0;
			}
		}
		ct_recycled_->add((double)(rcyc + sent), "", COLOUR_RECYCLED);
		ct_sent_->add((double)sent, "", COLOUR_SENT);
	}

	// Set boiunds of recycled and sent to those of received
	ct_received_->bounds(0, max_);
	ct_recycled_->bounds(0, max_);
	ct_sent_->bounds(0, max_);
	
	redraw();
}

void QBS_charth::draw_y_axis() {
	// Draw the SWR axis
	fl_color(FL_FOREGROUND_COLOR);
	int ax = x() + AXIS_WIDTH;
	int dh = fl_height();
	int ay = y();
	int ah = h() - dh - 1;

	fl_line(ax, ay, ax, ay + ah);
	// Now add the ticks - generate values
	set<double> ticks;
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

void QBS_charth::draw() {
	Fl_Group::draw();
	draw_y_axis();
}
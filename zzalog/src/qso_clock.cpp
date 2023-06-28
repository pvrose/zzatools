#include "qso_clock.h"
#include "qso_manager.h"
#include "drawing.h"

#include<ctime>

const double UTC_TIMER = 1.0;
extern bool closing_;

// Clock group - constructor
qso_clock::qso_clock
(int X, int Y, int W, int H, bool local) :
	Fl_Group(X, Y, W, H)
	, display_local_(local)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	box(FL_BORDER_BOX);
	load_values();
	create_form(X, Y);
}

// Clock group destructor
qso_clock::~qso_clock()
{
	save_values();
}

// get settings
void qso_clock::load_values() {
	// No code
}

// Create form
void qso_clock::create_form(int X, int Y) {

	int curr_x = X + GAP;
	int curr_y = Y + GAP;

	const int WCLOCKS = 200;

	bn_time_ = new Fl_Button(curr_x, curr_y, WCLOCKS, 3 * HTEXT);
	bn_time_->color(FL_BLACK);
	bn_time_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	//	bn_time_->labelfont(FL_BOLD);
	bn_time_->labelsize(4 * FL_NORMAL_SIZE);
	bn_time_->box(FL_FLAT_BOX);
	bn_time_->when(FL_WHEN_RELEASE);

	curr_y += bn_time_->h();

	bn_date_ = new Fl_Button(curr_x, curr_y, WCLOCKS, HTEXT * 3 / 2);
	bn_date_->color(FL_BLACK);
	bn_date_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_date_->labelsize(FL_NORMAL_SIZE * 3 / 2);
	bn_date_->box(FL_FLAT_BOX);
	bn_date_->when(FL_WHEN_RELEASE);

	curr_y += bn_date_->h() + GAP;
	curr_x += GAP + WCLOCKS;

	resizable(nullptr);
	size(curr_x - X, curr_y - Y);
	end();
}

// Enable/disab;e widgets
void qso_clock::enable_widgets() {
	time_t now = time(nullptr);
	tm* value;
	char result[100];
	if (display_local_) {
		value = localtime(&now);
		strftime(result, 99, "%Z", value);
		copy_label(result);
		bn_time_->labelcolor(FL_RED);
		strftime(result, 99, "%H:%M:%S", value);
		bn_time_->copy_label(result);
		bn_date_->labelcolor(FL_RED);
		strftime(result, 99, "%A %d %B %Y", value);
		bn_date_->copy_label(result);
	}
	else {
		value = gmtime(&now);
		label("UTC");
		bn_time_->labelcolor(FL_YELLOW);
		strftime(result, 99, "%H:%M:%S", value);
		bn_time_->copy_label(result);
		bn_date_->labelcolor(FL_YELLOW);
		strftime(result, 99, "%A %d %B %Y", value);
		bn_date_->copy_label(result);
	}
}

// save value
void qso_clock::save_values() {
	// No code
}

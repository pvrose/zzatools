#include "qso_clock.h"

#include "qso_manager.h"
#include "ticker.h"

#include "utils.h"
#include "drawing.h"

#include<ctime>

#include <FL/Fl_Image.H>
#include <FL/Fl_Button.H>

extern ticker* ticker_;

// Clock group - constructor
qso_clock::qso_clock
(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, previous_time_(0)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	box(FL_BORDER_BOX);
	tooltip("Displays current clock value in the appropriate timezone");
	create_form(X, Y);

	// Add 1s clock
	ticker_->add_ticker(this, cb_ticker, 1);

}

// Clock group destructor
qso_clock::~qso_clock()
{
}

// Create form
void qso_clock::create_form(int X, int Y) {

	int curr_x = X;
	int curr_y = Y;

	const int WCLOCKS = 200;

	const int TIME_SZ = 3 * FL_NORMAL_SIZE;
	const int DATE_SZ = 3 * FL_NORMAL_SIZE / 2;

	// Button in which to display time
	bn_time_ = new Fl_Button(curr_x, curr_y, WCLOCKS, TIME_SZ + 2);
	bn_time_->color(FL_BLACK);
	bn_time_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	//	bn_time_->labelfont(FL_BOLD);
	bn_time_->labelsize(TIME_SZ);
	bn_time_->box(FL_FLAT_BOX);
	bn_time_->down_box(FL_FLAT_BOX);
	bn_time_->callback(cb_clock, nullptr);
	bn_time_->when(FL_WHEN_RELEASE);
	bn_time_->clear_visible_focus();

	curr_y += bn_time_->h();

	// Button in which to display date
	bn_date_ = new Fl_Button(curr_x, curr_y, WCLOCKS, DATE_SZ + 2 + GAP);
	bn_date_->color(FL_BLACK);
	bn_date_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_date_->labelsize(DATE_SZ);
	bn_date_->box(FL_FLAT_BOX);
	bn_date_->down_box(FL_FLAT_BOX);
	bn_date_->callback(cb_clock, nullptr);
	bn_date_->when(FL_WHEN_RELEASE);
	bn_date_->clear_visible_focus();

	curr_x = X + WCLOCKS;
	curr_y += bn_date_->h();

	resizable(nullptr);
	size(curr_x - X, curr_y - Y);
	end();
}

// Enable/disab;e widgets
void qso_clock::enable_widgets() {
	// Get the current time
	time_t now = time(nullptr);

	if (now != previous_time_) {
		previous_time_ = now;

		tm value;
		char result[100];
		if (display_local_) {
			// Display in local timezone
			value = *localtime(&now);
			// Copy timezone to tab's label
			strftime(result, 99, "Time: %Z", &value);
			copy_label(result);
			bn_time_->labelcolor(fl_lighter(FL_RED));
			strftime(result, 99, "%H:%M:%S", &value);
			bn_time_->copy_label(result);
			bn_date_->labelcolor(fl_lighter(FL_RED));
			strftime(result, 99, "%a %d %b %Y", &value);
			bn_date_->copy_label(result);
		}
		else {
			// Display in UTC (aka GMT)
			value = *gmtime(&now);
			label("Time: UTC");
			bn_time_->labelcolor(FL_YELLOW);
			strftime(result, 99, "%H:%M:%S", &value);
			bn_time_->copy_label(result);
			bn_date_->labelcolor(FL_YELLOW);
			strftime(result, 99, "%a %d %b %Y", &value);
			bn_date_->copy_label(result);
		}
	}
}

// Clock
void qso_clock::cb_ticker(void* v) {
	((qso_clock*)v)->enable_widgets();
}

// Click date or ime
void qso_clock::cb_clock(Fl_Widget* w, void* v) {
	qso_clock* that= ancestor_view<qso_clock>(w);
	that->display_local_ = !that->display_local_;
	qso_manager* mgr = ancestor_view<qso_manager>(that);
	mgr->enable_widgets();
}

// Set local
void qso_clock::local(bool value) {
	display_local_ = value;
	enable_widgets();
}

// Get local
bool qso_clock::local() {
	return display_local_;
}

#include "qso_clock.h"
#include "qso_manager.h"
#include "drawing.h"

#include<ctime>

const double UTC_TIMER = 1.0;

// Clock group - constructor
qso_clock::qso_clock
(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
	, display_local_(false)
{
	load_values();
}

// Clock group destructor
qso_clock::~qso_clock() {
	Fl::remove_timeout(cb_timer_clock, nullptr);
	save_values();
}

// get settings
void qso_clock::load_values() {
	// No code
}

// Create form
void qso_clock::create_form(int X, int Y) {

	g_clock_ = new Fl_Group(X, Y, 10, 10);
	g_clock_->labelfont(FL_BOLD);
	g_clock_->labelsize(FL_NORMAL_SIZE + 2);
	g_clock_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g_clock_->box(FL_BORDER_BOX);

	int curr_x = g_clock_->x() + GAP;
	int curr_y = g_clock_->y() + HTEXT;

	const int WCLOCKS = 250;

	bn_time_ = new Fl_Button(curr_x, curr_y, WCLOCKS, 3 * HTEXT);
	bn_time_->color(FL_BLACK);
	bn_time_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_time_->labelfont(FL_BOLD);
	bn_time_->labelsize(5 * FL_NORMAL_SIZE);
	bn_time_->labelcolor(FL_YELLOW);
	bn_time_->box(FL_FLAT_BOX);
	bn_time_->when(FL_WHEN_RELEASE);
	bn_time_->callback(cb_click, nullptr);

	curr_y += bn_time_->h();

	bn_date_ = new Fl_Button(curr_x, curr_y, WCLOCKS, HTEXT * 3 / 2);
	bn_date_->color(FL_BLACK);
	bn_date_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_date_->labelsize(FL_NORMAL_SIZE * 3 / 2);
	bn_date_->labelcolor(FL_YELLOW);
	bn_date_->box(FL_FLAT_BOX);
	bn_date_->when(FL_WHEN_RELEASE);
	bn_date_->callback(cb_click, nullptr);

	curr_y += bn_date_->h();

	bn_local_ = new Fl_Button(curr_x, curr_y, WCLOCKS, HTEXT * 3 / 2);
	bn_local_->color(FL_BLACK);
	bn_local_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_local_->labelsize(FL_NORMAL_SIZE * 3 / 2);
	bn_local_->labelcolor(FL_RED);
	bn_local_->box(FL_FLAT_BOX);
	bn_local_->when(FL_WHEN_RELEASE);
	bn_local_->callback(cb_click, nullptr);

	curr_x += GAP + WCLOCKS;
	curr_y += bn_local_->h() + GAP;

	g_clock_->resizable(nullptr);
	g_clock_->size(curr_x - g_clock_->x(), curr_y - g_clock_->y());
	g_clock_->end();

	resizable(nullptr);
	size(g_clock_->w(), g_clock_->h());
	show();
	end();

	// Start clock timer
	Fl::add_timeout(0, cb_timer_clock, this);
}

// Enable/disab;e widgets
void qso_clock::enable_widgets() {
	time_t now = time(nullptr);
	tm* value;
	tm* other_value;
	char result[100];
	if (display_local_) {
		value = localtime(&now);
		strftime(result, 99, "Clock - %Z", value);
		g_clock_->copy_label(result);
		bn_time_->labelcolor(FL_RED);
		strftime(result, 99, "%H:%M:%S", value);
		bn_time_->copy_label(result);
		bn_date_->labelcolor(FL_RED);
		strftime(result, 99, "%A %d %B %Y", value);
		bn_date_->copy_label(result);
		// Convert other time
		other_value = gmtime(&now);
		strftime(result, 99, "%T UTC", other_value);
		bn_local_->labelcolor(FL_YELLOW);
		bn_local_->copy_label(result);
	}
	else {
		value = gmtime(&now);
		g_clock_->label("Clock - UTC");
		bn_time_->labelcolor(FL_YELLOW);
		strftime(result, 99, "%H:%M:%S", value);
		bn_time_->copy_label(result);
		bn_date_->labelcolor(FL_YELLOW);
		strftime(result, 99, "%A %d %B %Y", value);
		bn_date_->copy_label(result);
		// Convert other time
		other_value = localtime(&now);
		strftime(result, 99, "%T %Z", other_value);
		bn_local_->labelcolor(FL_RED);
		bn_local_->copy_label(result);
	}
}

// save value
void qso_clock::save_values() {
	// No code
}

// Callback - 1s timer
void qso_clock::cb_timer_clock(void* v) {
	// Update the label in the clock button which is passed as the parameter
	qso_clock* that = (qso_clock*)v;
	that->enable_widgets();
	((qso_manager*)that->parent())->ticker();

	Fl::repeat_timeout(UTC_TIMER, cb_timer_clock, v);
}

// Callback click group
void qso_clock::cb_click(Fl_Widget* w, void* v) {
	qso_clock* that = ancestor_view<qso_clock>(w);
	that->display_local_ = !that->display_local_;
	that->enable_widgets();
}

// Stop 1s ticker
void qso_clock::stop_ticker() {
	Fl::remove_timeout(cb_timer_clock);
}
#include "qso_clock.h"
#include "qso_manager.h"
#include "drawing.h"
#include "wx_handler.h"
#include "drawing.h"

#include<ctime>

#include <FL/Fl_Image.H>

extern bool closing_;
extern wx_handler* wx_handler_;

// Clock group - constructor
qso_clock::qso_clock
(int X, int Y, int W, int H, bool local) :
	Fl_Group(X, Y, W, H)
	, display_local_(local)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	box(FL_BORDER_BOX);
	display_speed_ = MILE_PER_HOUR;
	display_temperature_ = CELSIUS;
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

	const int TIME_SZ = 4 * FL_NORMAL_SIZE;
	const int DATE_SZ = 3 * FL_NORMAL_SIZE / 2;

	bn_time_ = new Fl_Button(curr_x, curr_y, WCLOCKS, TIME_SZ + 2);
	bn_time_->color(FL_BLACK);
	bn_time_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	//	bn_time_->labelfont(FL_BOLD);
	bn_time_->labelsize(TIME_SZ);
	bn_time_->box(FL_FLAT_BOX);
	bn_time_->when(FL_WHEN_RELEASE);

	curr_y += bn_time_->h();

	bn_date_ = new Fl_Button(curr_x, curr_y, WCLOCKS, DATE_SZ + 2 + GAP);
	bn_date_->color(FL_BLACK);
	bn_date_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_date_->labelsize(DATE_SZ);
	bn_date_->box(FL_FLAT_BOX);
	bn_date_->when(FL_WHEN_RELEASE);

	curr_y += bn_date_->h() + GAP;
	const int WICON = 2 * HBUTTON;
	const int WTEXT = WCLOCKS - WICON - GAP;
	const int WX_SIZE = FL_NORMAL_SIZE + 2;

	bn_wx_icon_ = new Fl_Button(curr_x, curr_y, WICON, WICON);
	bn_wx_icon_->color(COLOUR_GREY);
	bn_wx_icon_->box(FL_FLAT_BOX);
	bn_wx_icon_->callback(cb_bn_icon, nullptr);

	curr_x += WICON + GAP;
	bn_location_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_location_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_location_->box(FL_FLAT_BOX);
	bn_location_->labelsize(WX_SIZE);

	curr_y += WX_SIZE;

	bn_wx_description_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_wx_description_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_wx_description_->box(FL_FLAT_BOX);
	bn_wx_description_->labelsize(WX_SIZE);

	curr_y += WX_SIZE;
	bn_wx_detail_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_wx_detail_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_wx_detail_->box(FL_FLAT_BOX);
	bn_wx_detail_->labelsize(WX_SIZE);

	curr_y += WX_SIZE;

	bn_sun_times_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_sun_times_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_sun_times_->box(FL_FLAT_BOX);
	bn_sun_times_->labelsize(WX_SIZE);

	curr_y += WX_SIZE;

	bn_updated_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_updated_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_updated_->box(FL_FLAT_BOX);
	bn_updated_->labelsize(WX_SIZE);

	curr_x = X + WCLOCKS + GAP + GAP;
	curr_y += WX_SIZE + GAP;

	resizable(nullptr);
	size(curr_x - X, curr_y - Y);
	end();
}

// Enable/disab;e widgets
void qso_clock::enable_widgets() {
	time_t now = time(nullptr);
	time_t sunrise = wx_handler_ ? wx_handler_->sun_rise() : 0;
	time_t sunset = wx_handler_ ? wx_handler_->sun_set() : 0;
	string wx_descr = wx_handler_ ? wx_handler_->description() : "";
	float temperature = wx_handler_ ? wx_handler_->temperature() : 0.0; 
	float wind_speed = wx_handler_ ? wx_handler_->wind_speed() : 0.0;
	string wind_dirn = wx_handler_ ? wx_handler_->wind_direction() : "";
	Fl_Image* icon = wx_handler_ ? wx_handler_->icon() : nullptr;
	time_t updated = wx_handler_ ? wx_handler_->last_updated() : 0;
	string wx_location = wx_handler_ ? wx_handler_->location() : "";

	tm value;
	tm sunup_time;
	tm sundown_time;
	tm updated_time;
	char result[100];
	if (display_local_) {
		value = *localtime(&now);
		strftime(result, 99, "%Z", &value);
		copy_label(result);
		bn_time_->labelcolor(FL_RED);
		strftime(result, 99, "%H:%M:%S", &value);
		bn_time_->copy_label(result);
		bn_date_->labelcolor(FL_RED);
		strftime(result, 99, "%a %d %b %Y", &value);
		bn_date_->copy_label(result);
		// Sunrise and sunset
		sunup_time = *localtime(&sunrise);
		sundown_time = *localtime(&sunset);
		updated_time = *localtime(&updated);
	}
	else {
		value = *gmtime(&now);
		label("UTC");
		bn_time_->labelcolor(FL_YELLOW);
		strftime(result, 99, "%H:%M:%S", &value);
		bn_time_->copy_label(result);
		bn_date_->labelcolor(FL_YELLOW);
		strftime(result, 99, "%a %d %b %Y", &value);
		bn_date_->copy_label(result);
		// Sunrise and sunset
		sunup_time = *gmtime(&sunrise);
		sundown_time = *gmtime(&sunset);
		updated_time = *gmtime(&updated);
	}
	bn_location_->copy_label(wx_location.c_str());
	bn_wx_description_->copy_label(wx_descr.c_str());
	char label[128];
	snprintf(label, sizeof(label), "%0.0f \302\260C %0.0f MPH %s", temperature, wind_speed, wind_dirn.c_str());
	bn_wx_detail_->copy_label(label);
	char sunup[16];
	char sundown[16];
	char update_value[16];
	strftime(sunup, sizeof(sunup), "%H:%M", &sunup_time);
	strftime(sundown, sizeof(sundown), "%H:%M", &sundown_time);
	snprintf(label, sizeof(label), "Sun rise %s set %s", sunup, sundown);
	bn_sun_times_->copy_label(label);
	strftime(label, sizeof(label), "Updated %H:%M:%S", &updated_time);
	bn_updated_->copy_label(label);
	bn_wx_icon_->image(icon);
	bn_wx_icon_->deimage(icon);
	bn_wx_icon_->redraw();
}

// save value
void qso_clock::save_values() {
	// No code
}

// Icon clicked'
void qso_clock::cb_bn_icon(Fl_Widget* w, void* v) {
	qso_clock* that = ancestor_view<qso_clock>(w);
	wx_handler_->update();
	that->enable_widgets();
}

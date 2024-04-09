#include "qso_wx.h"

#include "qso_wx.h"
#include "qso_manager.h"
#include "drawing.h"
#include "wx_handler.h"
#include "utils.h"

#include<ctime>

#include <FL/Fl_Image.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_RGB_Image.H>

extern wx_handler* wx_handler_;
extern Fl_Preferences* settings_;

// Clock group - constructor
qso_wx::qso_wx
(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	box(FL_BORDER_BOX);
	display_speed_ = MILE_PER_HOUR;
	display_temperature_ = CELSIUS;
	display_pressure_ = HECTOPASCAL;
	display_direction_ = CARDINAL;
	load_values();
	create_form(X, Y);
}

// Clock group destructor
qso_wx::~qso_wx()
{
	save_values();
	// Delete any created images
	bn_direction_->image(nullptr);
}

// get settings
void qso_wx::load_values() {
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences wx_settings(user_settings, "Weather");
	wx_settings.get("Speed", (int&)display_speed_, MILE_PER_HOUR);
	wx_settings.get("Direction", (int&)display_direction_, CARDINAL);
	wx_settings.get("Temperature", (int&)display_temperature_, CELSIUS);
	wx_settings.get("Pressure", (int&)display_pressure_, HECTOPASCAL);
	wx_settings.get("Cloud", (int&)display_cloud_, PERCENT);
}

// Create form
void qso_wx::create_form(int X, int Y) {

	int curr_x = X + GAP;
	int curr_y = Y + GAP;

	const int WCLOCKS = 200;

	const int TIME_SZ = 4 * FL_NORMAL_SIZE;
	const int DATE_SZ = 3 * FL_NORMAL_SIZE / 2;

	const int WICON = 2 * HBUTTON;
	const int WTEXT = WCLOCKS - WICON - GAP;
	const int WWX = WCLOCKS - GAP;
    const int WT4 = WWX / 5;
    const int WT2 = WWX / 2;
	const int WX_SIZE = FL_NORMAL_SIZE + 2;

	bn_location_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_location_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_location_->box(FL_FLAT_BOX);
	bn_location_->labelsize(WX_SIZE);
    bn_location_->labelfont(FL_BOLD);

	curr_y += WX_SIZE;

	bn_latlong_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_latlong_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_latlong_->box(FL_FLAT_BOX);
	bn_latlong_->labelsize(WX_SIZE);

	curr_y += WX_SIZE + GAP;
	
	bn_wx_icon_ = new Fl_Button(curr_x, curr_y, WICON, WICON);
	bn_wx_icon_->color(COLOUR_GREY);
	bn_wx_icon_->box(FL_FLAT_BOX);
	bn_wx_icon_->callback(cb_bn_icon, nullptr);

	curr_x += WICON + GAP;

	bn_wx_description_ = new Fl_Button(curr_x, curr_y, WTEXT, WICON);
	bn_wx_description_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_wx_description_->box(FL_FLAT_BOX);
	bn_wx_description_->labelsize(WX_SIZE);

	curr_y += WICON + GAP;
	curr_x = X + GAP;
	bn_temperature_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_temperature_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_temperature_->box(FL_FLAT_BOX);
	bn_temperature_->labelsize(WX_SIZE);
	bn_temperature_->callback(cb_bn_temperature, nullptr);

	curr_x += WT4;
	bn_speed_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_speed_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_speed_->box(FL_FLAT_BOX);
	bn_speed_->labelsize(WX_SIZE);
	bn_speed_->callback(cb_bn_speed, nullptr);

	curr_x += WT4;
	bn_direction_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_direction_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_direction_->box(FL_FLAT_BOX);
	bn_direction_->labelsize(WX_SIZE);
	bn_direction_->callback(cb_bn_direction, nullptr);

	curr_x += WT4;
	bn_pressure_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_pressure_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_pressure_->box(FL_FLAT_BOX);
	bn_pressure_->labelsize(WX_SIZE);
	bn_pressure_->callback(cb_bn_pressure, nullptr);

	curr_x += WT4;
	bn_cloud_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_cloud_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_cloud_->box(FL_FLAT_BOX);
	bn_cloud_->labelsize(WX_SIZE);
	bn_cloud_->callback(cb_bn_cloud, nullptr);

	curr_x = X + GAP;
	curr_y += WICON;

	bn_sunrise_ = new Fl_Button(curr_x, curr_y, WT2, WX_SIZE);
	bn_sunrise_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_sunrise_->box(FL_FLAT_BOX);
	bn_sunrise_->labelsize(WX_SIZE);

    curr_x += WT2;

	bn_sunset_ = new Fl_Button(curr_x, curr_y, WT2, WX_SIZE);
	bn_sunset_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_sunset_->box(FL_FLAT_BOX);
	bn_sunset_->labelsize(WX_SIZE);

	curr_y += HTEXT;
	curr_x = X + GAP;

	bn_updated_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_updated_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_updated_->box(FL_FLAT_BOX);
	bn_updated_->labelsize(WX_SIZE);

	curr_x = X + WCLOCKS + GAP + GAP;
	curr_y += WX_SIZE + GAP;

	resizable(nullptr);
	size(curr_x - X, curr_y - Y);
	end();
}

// Enable/disab;e widgets
void qso_wx::enable_widgets() {
	time_t sunrise = wx_handler_ ? wx_handler_->sun_rise() : 0;
	time_t sunset = wx_handler_ ? wx_handler_->sun_set() : 0;
	string wx_descr = wx_handler_ ? wx_handler_->description() : "";
	float temperature = wx_handler_ ? wx_handler_->temperature() : 0.0; 
	float wind_speed = wx_handler_ ? wx_handler_->wind_speed() : 0.0;
	string wind_descr = wx_handler_ ? wx_handler_->wind_name() : "";
	string wind_dirn = wx_handler_ ? wx_handler_->wind_direction() : "";
	unsigned int wind_degree = wx_handler_ ? wx_handler_->wind_degrees() : 0;
	float pressure = wx_handler_ ? wx_handler_->pressure() : 0.0;
	float cloud_cover = wx_handler_ ? wx_handler_->cloud() : 0.0;
	string cloud_descr = wx_handler_ ? wx_handler_->cloud_name() : "";
	Fl_Image* icon = wx_handler_ ? wx_handler_->icon() : nullptr;
	time_t updated = wx_handler_ ? wx_handler_->last_updated() : 0;
	string wx_location = wx_handler_ ? wx_handler_->location() : "";
	string wx_latlong = wx_handler_ ? wx_handler_->latlong() : "";

	tm value;
	tm sunup_time;
	tm sundown_time;
	tm updated_time;
	char result[100];
	// Sunrise and sunset
	sunup_time = *localtime(&sunrise);
	sundown_time = *localtime(&sunset);
	updated_time = *localtime(&updated);
	bn_location_->copy_label(wx_location.c_str());
	bn_latlong_->copy_label(wx_latlong.c_str());
	char label[128];
	strcpy(label, wx_descr.c_str());
	strcat(label, "\n");
	strcat(label, to_lower(wind_descr).c_str());
	strcat(label, "\n");
	strcat(label, cloud_descr.c_str());
	bn_wx_description_->copy_label(label);
	switch(display_temperature_) {
		case CELSIUS: {
			snprintf(label, sizeof(label), "%0.0f\n\302\260C", temperature);
			break;
		}
		case FAHRENHEIT: {
			snprintf(label, sizeof(label), "%0.0f\n\302\260F", (temperature * 9 / 5) + 32);
			break;
		}
		default:
			strcpy(label, "");
			break;
	}
	bn_temperature_->copy_label(label);
	switch(display_speed_) {
		case MILE_PER_HOUR: {
			snprintf(label, sizeof(label), "%0.0f\nMPH", wind_speed);
			break;
		}
		case METRE_PER_SECOND: {
			snprintf(label, sizeof(label), "%0.0f\nm/s", wind_speed * 1760 * 36 *25.4 / 3600000);
			break;
		}
		case KM_PER_HOUR: {
			snprintf(label, sizeof(label), "%0.0f\nkm/h", wind_speed * 1760 * 36 *25.4 / 1000000);
			break;
		}
		case KNOTS: {
			snprintf(label, sizeof(label), "%0.0f\nknot", wind_speed * 0.868976);
			break;
		}
		default:
			strcpy(label, "");
			break;

	}
	bn_speed_->copy_label(label);

	bn_direction_->label(nullptr);
	bn_direction_->image(nullptr);
	switch(display_direction_) {
		case CARDINAL: {
			snprintf(label, sizeof(label), "%s", wind_dirn.c_str());
			bn_direction_->copy_label(label);
			break;
		}
		case DEGREES: {
			if (wind_degree == -1) strcpy(label, "---"); 
			else snprintf(label, sizeof(label), "%03d\302\260", wind_degree);
			bn_direction_->copy_label(label);
			break;
		}
		case ARROW: {
			// if (wind_degree == -1) strcpy(label, "@line");
			// else if (wind_degree < 23) strcpy(label, "@2arrow");
			// else if (wind_degree < 68) strcpy(label, "@1arrow");
			// else if (wind_degree < 113) strcpy(label, "@4arrow");
			// else if (wind_degree < 158) strcpy(label, "@7arrow");
			// else if (wind_degree < 203) strcpy(label, "@8arrow");
			// else if (wind_degree < 248) strcpy(label, "@9arrow");
			// else if (wind_degree < 293) strcpy(label, "@6arrow");
			// else if (wind_degree < 338) strcpy(label, "@3arrow");
			// else strcpy(label, "@2arrow");
			draw_wind_dirn(bn_direction_, wind_degree);
			break;
		}
		default:
			break;
	}
	switch(display_pressure_) {
		case HECTOPASCAL: {
			snprintf(label, sizeof(label), "%0.0f\nhPa", pressure);
			break;
		}
		case MILLIBARS: {
			snprintf(label, sizeof(label), "%0.0f\nmbar", pressure);
			break;
		}
		case MM_MERCURY: {
			snprintf(label, sizeof(label), "%0.0f\nmmHg", pressure * 0.75006375541921);
			break;
		}
		case IN_MERCURY: {
			snprintf(label, sizeof(label), "%0.1f\ninHg", pressure * 0.02952998057228486);
			break;
		}
		default:
			strcpy(label, "");
			break;
	}
	bn_pressure_->copy_label(label);
	switch(display_cloud_) {
		case PERCENT: {
			snprintf(label, sizeof(label), "%d%%\ncloud", (int)(cloud_cover * 100));
			break;
		}
		case OKTA: {
			if (cloud_cover == 0.0) strcpy(label, "0\nokta");
			else if (cloud_cover == 1.0) strcpy(label, "8\nokta");
			else {
				int okta = (int)(cloud_cover * 6.0) + 1;
				snprintf(label, sizeof(label), "%d\nokta", okta);
			}
			break;
		}
	}
	bn_cloud_->copy_label(label);

	char sunup[16];
	char sundown[16];
	char update_value[16];
	strftime(sunup, sizeof(sunup), "%H:%M", &sunup_time);
	strftime(sundown, sizeof(sundown), "%H:%M", &sundown_time);
	snprintf(label, sizeof(label), "\360\237\214\236Rise %s", sunup);
	bn_sunrise_->copy_label(label);
	snprintf(label, sizeof(label), "\360\237\214\236Set %s", sundown);
	bn_sunset_->copy_label(label);
	strftime(label, sizeof(label), "Updated %H:%M:%S", &updated_time);
	bn_updated_->copy_label(label);
	bn_wx_icon_->image(icon);
	bn_wx_icon_->deimage(icon);
	bn_wx_icon_->redraw();
}

// save value
void qso_wx::save_values() {
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences wx_settings(user_settings, "Weather");
	wx_settings.set("Speed", (int&)display_speed_);
	wx_settings.set("Direction", (int&)display_direction_);
	wx_settings.set("Temperature", (int&)display_temperature_);
	wx_settings.set("Pressure", (int&)display_pressure_);
	wx_settings.set("Cloud", (int&)display_cloud_);
}

// Icon clicked'
void qso_wx::cb_bn_icon(Fl_Widget* w, void* v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	wx_handler_->update();
	that->enable_widgets();
}

// Temperetaure clicked
void qso_wx::cb_bn_temperature(Fl_Widget* w, void * v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	switch (that->display_temperature_) {
		case CELSIUS: {
			that->display_temperature_ = FAHRENHEIT;
			break;
		}
		case FAHRENHEIT: {
			that->display_temperature_ = CELSIUS;
			break;
		}
	}
	that->enable_widgets();
}

// Speed clicked
void qso_wx::cb_bn_speed(Fl_Widget* w, void * v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	switch (that->display_speed_) {
		case MILE_PER_HOUR: {
			that->display_speed_ = METRE_PER_SECOND;
			break;
		}
		case METRE_PER_SECOND: {
			that->display_speed_ = KM_PER_HOUR;
			break;
		}
		case KM_PER_HOUR: {
			that->display_speed_ = KNOTS;
			break;
		}
		case KNOTS: {
			that->display_speed_ = MILE_PER_HOUR;
			break;
		}
	}
	that->enable_widgets();
}

// Pressure clicked
void qso_wx::cb_bn_pressure(Fl_Widget* w, void * v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	switch (that->display_pressure_) {
		case HECTOPASCAL: {
			that->display_pressure_ = MM_MERCURY;
			break;
		}
		case MM_MERCURY: {
			that->display_pressure_ = IN_MERCURY;
			break;
		}
		case IN_MERCURY: {
			that->display_pressure_ = MILLIBARS;
			break;
		}
		case MILLIBARS: {
			that->display_pressure_ = HECTOPASCAL;
			break;
		}
	}
	that->enable_widgets();
}

// Direction clicked
void qso_wx::cb_bn_direction(Fl_Widget* w, void * v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	switch (that->display_direction_) {
		case CARDINAL: {
			that->display_direction_ = DEGREES;
			break;
		}
		case DEGREES: {
			that->display_direction_ = ARROW;
			break;
		}
		case ARROW: {
			that->display_direction_ = CARDINAL;
			break;
		}
	}
	that->enable_widgets();
}

// Cloud clicked
void qso_wx::cb_bn_cloud(Fl_Widget* w, void* v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	switch (that->display_cloud_) {
		case PERCENT: {
			that->display_cloud_ = OKTA;
			break;
		}
		case OKTA: {
			that->display_cloud_ = PERCENT;
			break;
		}
	}
	that->enable_widgets();
}

// Draw wind direction arrow in the supplied widget
void qso_wx::draw_wind_dirn(Fl_Widget* w, unsigned int dirn) {
	// Find central position
	int x_zero = w->w() / 2;
	int y_zero = w->h() / 2;
	// arrow will fill 80% widget
	int radius = min(x_zero, y_zero) * 7 / 10;
	float angle = dirn * DEGREE_RADIAN;
	int x_disp = radius * sin(angle);
	int y_disp = radius * cos(angle);
	int x_start = x_zero + x_disp;
	int y_start = y_zero - y_disp;
	int x_end = x_zero - x_disp;
	int y_end = y_zero + y_disp;
	// Create the drawing surface
	Fl_Image_Surface* image_surface = new Fl_Image_Surface(w->w(), w->h());
	Fl_Surface_Device::push_current(image_surface);
	// Draw the background
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, w->w(), w->h());
	// Draw the line
	fl_color(FL_FOREGROUND_COLOR);
	if  (dirn == -1) {
		// No wind draw a circle
		fl_arc(x_zero-(radius/2), y_zero-(radius/2), radius, radius	, 0, 360);
	} else {
		fl_line(x_start, y_start, x_end, y_end);
		float rad45 = 15 * DEGREE_RADIAN;
		int arrow_len = radius;
		float arrow_l = angle + rad45;
		int x_arrow_l = x_end + (arrow_len * sin(arrow_l));
		int y_arrow_l = y_end - (arrow_len * cos(arrow_l));
		// fl_line(x_end, y_end, x_arrow_l, y_arrow_l);
		float arrow_r = angle - rad45;
		int x_arrow_r = x_end + (arrow_len * sin(arrow_r));
		int y_arrow_r = y_end - (arrow_len * cos(arrow_r));
		// fl_line(x_end, y_end, x_arrow_r, y_arrow_r);
		fl_polygon(x_end, y_end, x_arrow_l, y_arrow_l, x_arrow_r, y_arrow_r);
	}

	Fl_RGB_Image* image = image_surface->image();
	// Restore window before drawing widget
	Fl_Surface_Device::pop_current();

	// Now put the image into the widget
	w->label(nullptr);
	w->image(image);
}
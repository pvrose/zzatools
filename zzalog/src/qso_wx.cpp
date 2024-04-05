#include "qso_wx.h"

#include "qso_wx.h"
#include "qso_manager.h"
#include "drawing.h"
#include "wx_handler.h"

#include<ctime>

#include <FL/Fl_Image.H>
#include <FL/Fl_Preferences.H>

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
}

// get settings
void qso_wx::load_values() {
	Fl_Preferences user_settings(settings_, "User Settings");
	Fl_Preferences wx_settings(user_settings, "Weather");
	wx_settings.get("Speed", (int&)display_speed_, MILE_PER_HOUR);
	wx_settings.get("Direction", (int&)display_direction_, CARDINAL);
	wx_settings.get("Temperature", (int&)display_temperature_, CELSIUS);
	wx_settings.get("Pressure", (int&)display_pressure_, HECTOPASCAL);
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
    const int WT4 = WWX / 4;
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

	curr_y += WICON;
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
	string wind_dirn = wx_handler_ ? wx_handler_->wind_direction() : "";
	unsigned int wind_degree = wx_handler_ ? wx_handler_->wind_degrees() : 0;
	float pressure = wx_handler_ ? wx_handler_->pressure() : 0.0;
	Fl_Image* icon = wx_handler_ ? wx_handler_->icon() : nullptr;
	time_t updated = wx_handler_ ? wx_handler_->last_updated() : 0;
	string wx_location = wx_handler_ ? wx_handler_->location() : "";
	string wx_latlong = wx_handler_ ? wx_handler_->latlong() : "";

	tm value;
	tm sunup_time;
	tm sundown_time;
	tm updated_time;
	char result[100];
	if (display_local_) {
		// Sunrise and sunset
		sunup_time = *localtime(&sunrise);
		sundown_time = *localtime(&sunset);
		updated_time = *localtime(&updated);
	}
	else {
		// Sunrise and sunset
		sunup_time = *gmtime(&sunrise);
		sundown_time = *gmtime(&sunset);
		updated_time = *gmtime(&updated);
	}
	bn_location_->copy_label(wx_location.c_str());
	bn_latlong_->copy_label(wx_latlong.c_str());
	bn_wx_description_->copy_label(wx_descr.c_str());
	char label[128];
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
	switch(display_direction_) {
		case CARDINAL: {
			snprintf(label, sizeof(label), "%s", wind_dirn.c_str());
			break;
		}
		case DEGREES: {
			if (wind_degree == -1) strcpy(label, "---"); 
			else snprintf(label, sizeof(label), "%03d\302\260", wind_degree);
			break;
		}
		case ARROW: {
			if (wind_degree == -1) strcpy(label, "@line");
			else if (wind_degree < 23) strcpy(label, "@2arrow");
			else if (wind_degree < 68) strcpy(label, "@1arrow");
			else if (wind_degree < 113) strcpy(label, "@4arrow");
			else if (wind_degree < 158) strcpy(label, "@7arrow");
			else if (wind_degree < 203) strcpy(label, "@8arrow");
			else if (wind_degree < 248) strcpy(label, "@9arrow");
			else if (wind_degree < 293) strcpy(label, "@6arrow");
			else if (wind_degree < 338) strcpy(label, "@3arrow");
			else strcpy(label, "@2arrow");
			break;
		}
		default:
			strcpy(label, "");
			break;
	}
	bn_direction_->copy_label(label);
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
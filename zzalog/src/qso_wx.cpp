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
#include <FL/Fl_Button.H>

extern wx_handler* wx_handler_;
extern string VENDOR;
extern string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;
extern void open_html(const char*);

// Weather group - constructor
qso_wx::qso_wx
(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	box(FL_BORDER_BOX);
	tooltip("Displays the current weather - as supplied by openweather.com");
	display_speed_ = MILE_PER_HOUR;
	display_temperature_ = CELSIUS;
	display_pressure_ = HECTOPASCAL;
	display_direction_ = CARDINAL;
	load_values();
	create_form(X, Y);
	enable_widgets();
}

// Weather group destructor
qso_wx::~qso_wx()
{
	save_values();
	// Delete any created images
	bn_direction_->image(nullptr);
}

// Handle
int qso_wx::handle(int event) {
	int result = Fl_Group::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("qso_wx.html");
			return true;
		}
		break;
	}
	return result;
}

// get settings
void qso_wx::load_values() {
	// Get last used display formats
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences user_settings(settings, "User Settings");
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

	const int WICON = 2 * HBUTTON;
	const int WTEXT = WCLOCKS - WICON - GAP;
	const int WWX = WCLOCKS - GAP;
    const int WT4 = WWX / 5;
    const int WT2 = WWX / 2;
	const int WX_SIZE = FL_NORMAL_SIZE + 2;

	// Button that displays the received location
	bn_location_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_location_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_location_->box(FL_FLAT_BOX);
	bn_location_->labelsize(WX_SIZE);
    bn_location_->labelfont(FL_BOLD);

	curr_y += WX_SIZE;

	// Button that displays the longitude and latitude
	bn_latlong_ = new Fl_Button(curr_x, curr_y, WTEXT, WX_SIZE);
	bn_latlong_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_latlong_->box(FL_FLAT_BOX);
	bn_latlong_->labelsize(WX_SIZE);

	curr_y += WX_SIZE + GAP;
	
	// Button that displays the received weather icon
	bn_wx_icon_ = new Fl_Button(curr_x, curr_y, WICON, WICON);
	bn_wx_icon_->color(COLOUR_GREY);
	bn_wx_icon_->box(FL_FLAT_BOX);
	bn_wx_icon_->callback(cb_bn_icon, nullptr);

	curr_x += WICON + GAP;

	// Button that displays the description of the weather
	bn_wx_description_ = new Fl_Button(curr_x, curr_y, WTEXT, WICON);
	bn_wx_description_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_wx_description_->box(FL_FLAT_BOX);
	bn_wx_description_->labelsize(WX_SIZE);

	curr_y += WICON + GAP;
	curr_x = X + GAP;
	// Button that displays the temperature
	bn_temperature_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_temperature_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_temperature_->box(FL_FLAT_BOX);
	bn_temperature_->labelsize(WX_SIZE);
	bn_temperature_->callback(cb_bn_temperature, nullptr);

	curr_x += WT4;
	// Button that displays the wind speed
	bn_speed_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_speed_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_speed_->box(FL_FLAT_BOX);
	bn_speed_->labelsize(WX_SIZE);
	bn_speed_->callback(cb_bn_speed, nullptr);

	curr_x += WT4;
	// Button that displays the wind direction
	bn_direction_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_direction_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_direction_->box(FL_FLAT_BOX);
	bn_direction_->labelsize(WX_SIZE);
	bn_direction_->callback(cb_bn_direction, nullptr);

	curr_x += WT4;
	// Button that displays the air pressure
	bn_pressure_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_pressure_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_pressure_->box(FL_FLAT_BOX);
	bn_pressure_->labelsize(WX_SIZE);
	bn_pressure_->callback(cb_bn_pressure, nullptr);

	curr_x += WT4;
	// Button that displays the cloud cover
	bn_cloud_ = new Fl_Button(curr_x, curr_y, WT4, WICON);
	bn_cloud_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_cloud_->box(FL_FLAT_BOX);
	bn_cloud_->labelsize(WX_SIZE);
	bn_cloud_->callback(cb_bn_cloud, nullptr);

	curr_x = X + GAP;
	curr_y += WICON;

	// Button that displays the sunrise time
	bn_sunrise_ = new Fl_Button(curr_x, curr_y, WT2, WX_SIZE);
	bn_sunrise_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_sunrise_->box(FL_FLAT_BOX);
	bn_sunrise_->labelsize(WX_SIZE);

    curr_x += WT2;

	// Button thst displays the sunset time
	bn_sunset_ = new Fl_Button(curr_x, curr_y, WT2, WX_SIZE);
	bn_sunset_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	bn_sunset_->box(FL_FLAT_BOX);
	bn_sunset_->labelsize(WX_SIZE);

	curr_y += HTEXT;
	curr_x = X + GAP;

	// Button that displays when the data was last updated
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
	// Get the data from the WX Handler object
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

	tm sunup_time;
	tm sundown_time;
	tm updated_time;
	// Sunrise and sunset
	sunup_time = *localtime(&sunrise);
	sundown_time = *localtime(&sunset);
	updated_time = *localtime(&updated);
	// Latitude and Longitude 
	bn_location_->copy_label(wx_location.c_str());
	bn_latlong_->copy_label(wx_latlong.c_str());
	// Set the weather description
	char label[128];
	memset(label, '\0', sizeof(label));
	if (wx_descr != cloud_descr) {
		strcat(label, wx_descr.c_str());
		strcat(label, "\n");
	}
	strcat(label, to_lower(wind_descr).c_str());
	strcat(label, "\n");
	strcat(label, cloud_descr.c_str());
	bn_wx_description_->copy_label(label);
	// Set temperature
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
	// Set wind-speed
	switch(display_speed_) {
		case MILE_PER_HOUR: {
			snprintf(label, sizeof(label), "%0.0f\nMPH", wind_speed);
			break;
		}
		case METRE_PER_SECOND: {
			snprintf(label, sizeof(label), "%0.0f\nm s\342\201\273\302\271", wind_speed * 1760 * 36 *25.4 / 3600000);
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

	// Set wind direction - either text or an image
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
			draw_wind_dirn(bn_direction_, wind_degree);
			break;
		}
		default:
			break;
	}
	// Set air pressure
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
	// Set cloud cover - either text or an image
	unsigned int okta;
	if (cloud_cover == 0.0) okta = 0;
	else if (cloud_cover == 1.0) okta = 8;
	else okta = floor(cloud_cover * 7.0) + 1;
	bn_cloud_->label(nullptr);
	bn_cloud_->image(nullptr);
	switch(display_cloud_) {
		case PERCENT: {
			snprintf(label, sizeof(label), "%d%%\ncloud", (int)(cloud_cover * 100));
			bn_cloud_->copy_label(label);
			break;
		}
		case OKTA: {
			snprintf(label, sizeof(label), "%d\nokta", okta);
			bn_cloud_->copy_label(label);
			break;
		}
		case PIE: {
			draw_cloud_okta(bn_cloud_, okta);
			break;
		}
	}

	char sunup[16];
	char sundown[16];
	// Set sunrise and sunset times and last updated time (local)
	strftime(sunup, sizeof(sunup), "%H:%M", &sunup_time);
	strftime(sundown, sizeof(sundown), "%H:%M", &sundown_time);
	snprintf(label, sizeof(label), "\360\237\214\236Rise %s", sunup);
	bn_sunrise_->copy_label(label);
	snprintf(label, sizeof(label), "\360\237\214\236Set %s", sundown);
	bn_sunset_->copy_label(label);
	strftime(label, sizeof(label), "Updated %H:%M:%S %Z", &updated_time);
	bn_updated_->copy_label(label);
	bn_wx_icon_->image(icon);
	bn_wx_icon_->deimage(icon);
	bn_wx_icon_->redraw();
}

// save value
void qso_wx::save_values() {
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences user_settings(settings, "User Settings");
	Fl_Preferences wx_settings(user_settings, "Weather");
	wx_settings.set("Speed", (int&)display_speed_);
	wx_settings.set("Direction", (int&)display_direction_);
	wx_settings.set("Temperature", (int&)display_temperature_);
	wx_settings.set("Pressure", (int&)display_pressure_);
	wx_settings.set("Cloud", (int&)display_cloud_);
}

// Icon clicked' - reload weather
// v is not used
void qso_wx::cb_bn_icon(Fl_Widget* w, void* v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	wx_handler_->update();
	that->enable_widgets();
}

// Temperetaure clicked - cycle units
// v is not used
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

// Speed clicked - cycled units
// v is not used
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

// Pressure clicked - cycle units
// v is not used
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

// Direction clicked - cycle display format
// v is not used
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

// Cloud clicked - cycle formats
// v is not used
void qso_wx::cb_bn_cloud(Fl_Widget* w, void* v) {
	qso_wx* that = ancestor_view<qso_wx>(w);
	switch (that->display_cloud_) {
		case PERCENT: {
			that->display_cloud_ = OKTA;
			break;
		}
		case OKTA: {
			that->display_cloud_ = PIE;
			break;
		}
		case PIE: {
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
	// Create the drawing surface - origin will be top-left of the widget
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
		fl_push_matrix();
		// Set the drawing to be centred in the widget and rotated by the wind direction
		fl_translate(x_zero, y_zero);
		fl_rotate(-(double)dirn);
		fl_begin_complex_polygon();
		// Draw an arrow down from North
		fl_vertex(1, -radius);
		fl_vertex(1, radius/2);
		fl_vertex(5, radius/2);
		fl_vertex(0, radius);
		fl_vertex(-5, radius/2);
		fl_vertex(-1, radius/2);
		fl_vertex(-1, -radius);
		//
		fl_end_complex_polygon();
		
		fl_pop_matrix();
	}

	Fl_RGB_Image* image = image_surface->image();
	// Restore window before drawing widget
	Fl_Surface_Device::pop_current();

	// Now put the image into the widget
	w->label(nullptr);
	w->image(image);
}

// Draw the cloud cover okta pictogram
void qso_wx::draw_cloud_okta(Fl_Widget* w, unsigned int okta) {

	// Create the drawing surface
	Fl_Image_Surface* image_surface = new Fl_Image_Surface(w->w(), w->h());
	Fl_Surface_Device::push_current(image_surface);
	// Draw the background
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, w->w(), w->h());
	// Draw the line
	fl_color(FL_FOREGROUND_COLOR);
	fl_push_matrix();
	fl_translate(w->w() / 2.0, w->h() / 2.0);
	double radius = min(w->h(), w->w()) * 0.35;
	switch(okta) {
		case 0: {
			// 0 okta - empty circle
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			break;
		}
		case 1: {
			// 1 okta - circle plus single line at from 6 o'clock to 12 o'clock
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_line();
			fl_vertex(0, -radius);
			fl_vertex(0, radius);
			fl_end_line();
			break;
		}
		case 2: {
			// 2 okta - circle plus filled pie from 3 o'clock to 12 o'clock
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_polygon();
			fl_vertex(0,0);
			fl_arc(0, 0, radius, 0, 90);
			fl_end_polygon();
			break;
		}
		case 3: {
			// 3 okta - as 2 okta plus line down to 6 o'clock
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_polygon();
			fl_vertex(0,0);
			fl_arc(0, 0, radius, 0, 90);
			fl_end_polygon();
			fl_begin_line();
			fl_vertex(0, 0);
			fl_vertex(0, radius);
			fl_end_line();
			break;
		}
		case 4: {
			// 4 okta - circle plus filled pie in right half
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_polygon();
			fl_arc(0, 0, radius, 270, 450);
			fl_end_polygon();
			break;
		}
		case 5: {
			// 5 okta - as 4 okta plus line to 9 o'clock
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_polygon();
			fl_arc(0, 0, radius, 270, 450);
			fl_end_polygon();
			fl_begin_line();
			fl_vertex(0, 0);
			fl_vertex(-radius, 0);
			fl_end_line();
			break;
		}
		case 6: {
			// 6 okta - circle plus filled pie from 12 o'clock to 9 o'clock
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_complex_polygon();
			fl_vertex(0,0);
			fl_arc(0, 0, radius, 180, 450);
			fl_end_complex_polygon();
			break;
		}
		case 7: {
			// 7 okta - circle plus to sectors filled on left and right with a small gap
			fl_begin_line();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_line();
			fl_begin_polygon();
			fl_arc(0, 0, radius, 280, 440);
			fl_end_polygon();
			fl_begin_polygon();
			fl_arc(0, 0, radius, 100, 260);
			fl_end_polygon();
			break;
		}
		case 8: {
			// 8 okta - filled  in circle
			fl_begin_polygon();
			fl_arc(0, 0, radius, 0, 360);
			fl_end_polygon();
			break;
		}
	}
	fl_pop_matrix();

	Fl_RGB_Image* image = image_surface->image();
	// Restore window before drawing widget
	Fl_Surface_Device::pop_current();

	// Now put the image into the widget
	w->image(image);

}
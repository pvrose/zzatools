#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>

// Displays the current weather report; summary, temperature, wind speed & direction,
// Cloud cover and pressure
class qso_wx :
    public Fl_Group 
    
{
public:
    qso_wx(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_wx();

	// LLoad settings
    void load_values();
	// Create widgets
    void create_form(int X, int Y);
	// Save settimngs
    void save_values();
	// Configure widgets
    void enable_widgets();

protected:
	// Callback on the WX icon clicked - fetches report again
	static void cb_bn_icon(Fl_Widget* w, void *v);
	// Callback - changes the temperature between C and F
	static void cb_bn_temperature(Fl_Widget* w, void* v);
	// Callback - changes the windspeed MPH, km/h, m/s, knots
	static void cb_bn_speed(Fl_Widget* w, void* v);
	// Calback - changes the direction cardinal, degree, arrow
	static void cb_bn_direction(Fl_Widget* w, void* v);
	// Callback - changes the pressure - hPa, mbar, in Hg, mm Hg
	static void cb_bn_pressure(Fl_Widget* w, void* v);
	// Callback - changes the cloud - %, okta, pictogram
	static void cb_bn_cloud(Fl_Widget* w, void* v);

	// Draw the wind direction arrow 
	void draw_wind_dirn(Fl_Widget* w, unsigned int dirn); 
	// Draw the cloud cover okta pictogram
	void draw_cloud_okta(Fl_Widget* w, unsigned int okta);

	// Display local time rather than UTC
	bool display_local_;
	// Value display units
	enum {
		CELSIUS,
		FAHRENHEIT,
	} display_temperature_;
	enum {
		METRE_PER_SECOND,
		MILE_PER_HOUR,
		KM_PER_HOUR,
        KNOTS
	} display_speed_;
	enum {
		CARDINAL,
		DEGREES,
		ARROW
	} display_direction_;
    enum {
        HECTOPASCAL,
        MM_MERCURY,
        IN_MERCURY,
        MILLIBARS
    } display_pressure_;
	enum {
		PERCENT,
		OKTA,
		PIE
	} display_cloud_;

    // Widgets
	Fl_Button* bn_wx_icon_;
	Fl_Button* bn_wx_description_;
	Fl_Button* bn_temperature_;
	Fl_Button* bn_speed_;
	Fl_Button* bn_direction_;
	Fl_Button* bn_sunrise_;
    Fl_Button* bn_sunset_;
	Fl_Button* bn_updated_;
	Fl_Button* bn_location_;
	Fl_Button* bn_latlong_;
    Fl_Button* bn_pressure_;
	Fl_Button* bn_cloud_;

};
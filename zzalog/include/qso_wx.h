#pragma once

#include <FL/Fl_Group.H>

class Fl_Button;

//! \brief This class displays the current weather report; summary, temperature, wind speed & direction,
//! Cloud cover and pressure
class qso_wx :
    public Fl_Group 
    
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_wx(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
    ~qso_wx();

	//! Inherited from Fl_Group::handle: allows keyboard F1 to open userguide.
	virtual int handle(int event);

	//! Load display formats from settings.
    void load_values();
	//! Implement component widgets.
    void create_form(int X, int Y);
	//! Save display formats to settings.
    void save_values();
	//! Configure components widgets after data changes.
    void enable_widgets();

	//! Returns in daytime - defaults true if no data
	bool is_day(time_t when);

	//! Returns in nighttime - defaults true if no data
	bool is_night(time_t when);

protected:
	//! Callback on the WX icon clicked - fetches report again
	static void cb_bn_icon(Fl_Widget* w, void *v);
	//! Callback - changes the temperature between C and F
	static void cb_bn_temperature(Fl_Widget* w, void* v);
	//! Callback - changes the windspeed MPH, km/h, m/s, knots
	static void cb_bn_speed(Fl_Widget* w, void* v);
	//! Calback - changes the direction cardinal, degree, arrow
	static void cb_bn_direction(Fl_Widget* w, void* v);
	//! Callback - changes the pressure - hPa, mbar, in Hg, mm Hg
	static void cb_bn_pressure(Fl_Widget* w, void* v);
	//! Callback - changes the cloud - %, okta, pictogram
	static void cb_bn_cloud(Fl_Widget* w, void* v);

	//! Draw the wind direction arrow 
	void draw_wind_dirn(Fl_Widget* w, unsigned int dirn); 
	//! Draw the cloud cover okta pictogram
	void draw_cloud_okta(Fl_Widget* w, unsigned int okta);

	//! Display local time rather than UTC
	bool display_local_;
	//! Temperature units
	enum {
		CELSIUS,           //!< degrees Celsius 
		FAHRENHEIT,        //!< degrees Fahrenheit
	} display_temperature_;
	//! Wind speed
	enum {
		METRE_PER_SECOND,  //!< metres per second
		MILE_PER_HOUR,     //!< miles per hour
		KM_PER_HOUR,       //!< kilometres per hour
        KNOTS              //!< knots
	} display_speed_;
	//! Wind direction.
	enum {
		CARDINAL,          //!< one of the 16 cardinal points (eg WNW)
		DEGREES,           //!< degrees
		ARROW              //!< iconic representation as an arrow pointing from wind direction.
	} display_direction_;
	//! Atmospheric pressure
    enum {
        HECTOPASCAL,       //!< hectopascals
        MM_MERCURY,        //!< millimetres of mercury.
        IN_MERCURY,        //!< inches of mercury
        MILLIBARS          //!< mullibars
    } display_pressure_;
	//! Clous coverage
	enum {
		PERCENT,           //!< percentage 
		OKTA,              //!< okta
		PIE                //!< iconic representation
	} display_cloud_;

    // Widgets
	Fl_Button* bn_wx_icon_;          //!< Button: Iconic description of weather
	Fl_Button* bn_wx_description_;   //!< Button: Textual description of weather.
	Fl_Button* bn_temperature_;      //!< Button: Air Temperature.
	Fl_Button* bn_speed_;            //!< Button: Wind speed.
	Fl_Button* bn_direction_;        //!< Button: Wind direction.
	Fl_Button* bn_sunrise_;          //!< Button: Sunrise (in displayed timezone)
    Fl_Button* bn_sunset_;           //!< Button: Sunset (in displayed timezone)
	Fl_Button* bn_updated_;          //!< Button: Time last updated.
	Fl_Button* bn_location_;         //!< Button: Location name returned from server.
	Fl_Button* bn_latlong_;          //!< Button: Geographic coordinates returned from server.
    Fl_Button* bn_pressure_;         //!< Button: Air pressure.
	Fl_Button* bn_cloud_;            //!< Button: Cloud cover.

};
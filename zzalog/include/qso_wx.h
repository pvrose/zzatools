#pragma once

#include <nlohmann/json.hpp>

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

	//! Temperature units
	enum temp_t : uchar {
		CELSIUS,           //!< degrees Celsius 
		FAHRENHEIT,        //!< degrees Fahrenheit
	};
	//! Wind speed
	enum speed_t : uchar {
		METRE_PER_SECOND,  //!< metres per second
		MILE_PER_HOUR,     //!< miles per hour
		KM_PER_HOUR,       //!< kilometres per hour
		KNOTS              //!< knots
	};
	//! Wind direction.
	enum dirn_t : uchar {
		CARDINAL,          //!< one of the 16 cardinal points (eg WNW)
		DEGREES,           //!< degrees
		ARROW              //!< iconic representation as an arrow pointing from wind direction.
	};
	//! Atmospheric pressure
	enum press_t : uchar {
		HECTOPASCAL,       //!< hectopascals
		MM_MERCURY,        //!< millimetres of mercury.
		IN_MERCURY,        //!< inches of mercury
		MILLIBARS          //!< mullibars
	};
	//! Clous coverage
	enum cloud_t : uchar {
		PERCENT,           //!< percentage 
		OKTA,              //!< okta
		PIE                //!< iconic representation
	};

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
	temp_t display_temperature_;
	//! Wind speed
	speed_t display_speed_;
	//! Wind direction.
	dirn_t display_direction_;
	//! Atmospheric pressure
    press_t display_pressure_;
	//! Clous coverage
	cloud_t display_cloud_;

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

//! JSON serialisation for qso_wx::temp_t
NLOHMANN_JSON_SERIALIZE_ENUM(qso_wx::temp_t, {
	{ qso_wx::CELSIUS, "Celsius" },
	{ qso_wx::FAHRENHEIT, "Fahrenheight" }
	}
)

//! JSON serialisation for qso_wx::speed_t
NLOHMANN_JSON_SERIALIZE_ENUM(qso_wx::speed_t, {
	{ qso_wx::METRE_PER_SECOND, "m/s" },
	{ qso_wx::MILE_PER_HOUR, "mph" },
	{ qso_wx::KM_PER_HOUR, "km/h" },
	{ qso_wx::KNOTS, "knot" }
	}
)

//! JSON serialisation for qso_wx::dirn_t
NLOHMANN_JSON_SERIALIZE_ENUM(qso_wx::dirn_t, {
	{ qso_wx::CARDINAL, "Cardinal Point" },
	{ qso_wx::DEGREES, "Degress" },
	{ qso_wx::ARROW, "Pointer" }
	}
)

//! JSON serialisation for qso_wx::press_t
NLOHMANN_JSON_SERIALIZE_ENUM(qso_wx::press_t, {
	{ qso_wx::HECTOPASCAL, "Hectopascal" },
	{ qso_wx::MILLIBARS, "Millibar" },
	{ qso_wx::MM_MERCURY, "Millimetre Mercury" },
	{ qso_wx::IN_MERCURY, "Inch Mercury" }
	}
)

//! JSON serialisation for contest_scorer::ct_status
NLOHMANN_JSON_SERIALIZE_ENUM(qso_wx::cloud_t, {
	{ qso_wx::PERCENT, "Percentage Cover" },
	{ qso_wx::OKTA, "Okta" },
	{ qso_wx::PIE, "Icon" }
	}
)

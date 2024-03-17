#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>


class qso_wx :
    public Fl_Group 
    
{
public:
    qso_wx(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_wx();


    void load_values();
    void create_form(int X, int Y);
    void save_values();
    void enable_widgets();

protected:

	static void cb_bn_icon(Fl_Widget* w, void *v);
	static void cb_bn_temperature(Fl_Widget* w, void* v);
	static void cb_bn_speed(Fl_Widget* w, void* v);
	static void cb_bn_direction(Fl_Widget* w, void* v);
	static void cb_bn_pressure(Fl_Widget* w, void* v);

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

};
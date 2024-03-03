#pragma once
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

class qso_clock :
    public Fl_Group
{
public:
	qso_clock(int X, int Y, int W, int H, bool local);
	~qso_clock();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

protected:

	static void cb_bn_icon(Fl_Widget* w, void *v);
	static void cb_bn_temperature(Fl_Widget* w, void* v);
	static void cb_bn_speed(Fl_Widget* w, void* v);
	static void cb_bn_direction(Fl_Widget* w, void* v);

	// Display local time rather than UTC
	bool display_local_;
	// Value display units
	enum {
		CELSIUS,
		FAHRENHEIT,
	} display_temperature_;
	enum {
		METRE_PER_SECOND,
		MILE_PER_HOUR
	} display_speed_;
	enum {
		CARDINAL,
		DEGREES
	} display_direction_;

	Fl_Button* bn_time_;
	Fl_Button* bn_date_;
	Fl_Button* bn_wx_icon_;
	Fl_Button* bn_wx_description_;
	Fl_Button* bn_temperature_;
	Fl_Button* bn_speed_;
	Fl_Button* bn_direction_;
	Fl_Button* bn_sun_times_;
	Fl_Button* bn_updated_;
	Fl_Button* bn_location_;
	Fl_Button* bn_latlong_;
};


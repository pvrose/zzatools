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
	// Stop te 1 second ticker
	void stop_ticker();

protected:

	// Display local time rather than UTC
	bool display_local_;

	Fl_Button* bn_time_;
	Fl_Button* bn_date_;
};


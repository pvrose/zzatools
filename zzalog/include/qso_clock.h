#pragma once
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

class qso_clock :
    public Fl_Group
{
public:
	qso_clock(int X, int Y, int W, int H, const char* l);
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

	// Callback - 1s timer
	static void cb_timer_clock(void* v);
	// Callback - click on group
	static void cb_click(Fl_Widget* w, void* v);

	// Display local time rather than UTC
	bool display_local_;

	Fl_Group* g_clock_;
	Fl_Button* bn_time_;
	Fl_Button* bn_date_;
	Fl_Button* bn_local_;

};


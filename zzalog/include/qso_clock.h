#pragma once
#include <FL/Fl_Group.H>

class Fl_Button;

// This calss provides a ticking display of the current time
// It cna be configured for UTC or local time 
class qso_clock :
    public Fl_Group
{
public:
	qso_clock(int X, int Y, int W, int H, const char* L = nullptr);
	~qso_clock();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

	// Clock callback
	static void cb_ticker(void* v);
	// Click clock
	static void cb_clock(Fl_Widget* w, void* v);

	// Set/get local
	void local(bool value);
	bool local();

protected:

	// Display local time rather than UTC
	bool display_local_;

	// The date and time displays
	Fl_Button* bn_time_;
	Fl_Button* bn_date_;
};


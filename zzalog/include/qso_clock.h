#pragma once
#include <FL/Fl_Group.H>

class Fl_Button;

//! This class provides a ticking display of the current time.

//! It can be configured for UTC or local time 
class qso_clock :
    public Fl_Group
{
public:
	// Constructor

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_clock(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
	~qso_clock();

	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets on data change.
	void enable_widgets();

	//! Callback every 100 ms: the clock is updated when the system time ticks over one second.
	static void cb_ticker(void* v);
	//! Callback when clicking clock.
	static void cb_clock(Fl_Widget* w, void* v);

	//! Set the timezone.
	
	//! \param value true sets the timezone according to system locale.
	//! false sets the timezone to UTC.
	void local(bool value);
	//! Returns true if sstem locale, false if UTC.
	bool local();

protected:

	//! Display local time rather than UTC
	bool display_local_;
	//! Previous displayed time: used to detect second roll-over when ticking every 100 ms.
	time_t previous_time_;

	// The date and time displays
	Fl_Button* bn_time_;    //!< Button displaying current time.
	Fl_Button* bn_date_;    //!< Button displayimg current date.
};


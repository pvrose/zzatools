#pragma once

#include "button_input.h"

//! \brief This class is a widget that combines a button that opens a calendar browser 
//! and input widget.

//! The button opens a calendar widget to select a date.
//! The input allows the date to be typed in (and displays that selected by the calendar).
class calendar_input :
    public button_input
{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    calendar_input(int X, int Y, int W, int H, const char* L = nullptr);
    //! Desctructor.
    ~calendar_input();

    //! Override button_input::callback to intercept any value returned by it.
    virtual void callback(Fl_Callback* cb, void* v);
    //! Override button_input::user_data(void*) to forward user daat to components.
    virtual void user_data(void* v);

    //! Callback for the button.
    
    //! When the button is pressed, any data in the input is passed to the
    //! calendar with a default of the current date. The calendar is shown 
    //! at the position of the button
    //! pending any interaction with it.
    static void cb_button(Fl_Widget* w, void* v);

    //! Callback for the calendar
    static void cb_calendar(Fl_Widget* w, void* v);
};


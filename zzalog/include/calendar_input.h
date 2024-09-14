#pragma once

#include "button_input.h"


class calendar_input :
    public button_input
{
public:
    calendar_input(int X, int Y, int W, int H, const char* L = nullptr);
    ~calendar_input();

    // Overload callback
    virtual void callback(Fl_Callback* cb, void* v);
    // Overload user data
    virtual void user_data(void* v);

    // Call back for the button
    static void cb_button(Fl_Widget* w, void* v);

    // Call back for calendar
    static void cb_calendar(Fl_Widget* w, void* v);
};


#pragma once

#include "qso_clock.h"

#include <FL/Fl_Tabs.H>

// This class provides a tabbed group of instances of qso_clock (UTC and Local time)
class qso_clocks :
    public Fl_Tabs
{
public:
    qso_clocks(int X, int Y, int W, int H, const char* L);
    ~qso_clocks();

    void create_form();

    void enable_widgets();

protected:
    // Callback when the tab is changed
    static void cb_tabs(Fl_Widget* w, void* v);
    // The two instances
    qso_clock* utc_clock_;
    qso_clock* local_clock_;

};


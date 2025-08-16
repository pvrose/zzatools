#pragma once

#include <FL/Fl_Group.H>

class qso_clock;
class qso_wx;

// This class provides a tabbed group of instances of qso_clock (UTC and Local time)
class qso_clocks :
    public Fl_Group
{
public:
    qso_clocks(int X, int Y, int W, int H, const char* L);
    ~qso_clocks();

    virtual int handle(int event);

    void load_values();
    void create_form();
    void save_values();

    void enable_widgets();

    bool is_local();

protected:
    // Callback when the radio button is changed
    static void cb_radio(Fl_Widget* w, void* v);

    // The two instances
    qso_clock* utc_clock_;
    qso_clock* local_clock_;
    qso_wx* qso_weather_;


    bool local_;

};


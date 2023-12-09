#pragma once

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Chart.H>

using namespace std;

class QBS_data;

class QBS_charth :
    public Fl_Group
{

public:
    QBS_charth(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_charth();

    void create_form();

    void data(QBS_data* d);

    void update(string call);

    virtual void draw();

protected:

    QBS_data* data_;

    // Widgets
    Fl_Chart* ct_received_;
    Fl_Chart* ct_recycled_;
    Fl_Chart* ct_sent_;

    void draw_y_axis();

    double max_;

};


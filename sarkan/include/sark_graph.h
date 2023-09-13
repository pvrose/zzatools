#pragma once

#include "sark_data.h"

#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Chart.H>

class sark_graph : public Fl_Group {

public:
    sark_graph(int X, int Y, int W, int H, const char* L = nullptr);
    ~sark_graph();

    void data(sark_data* d);

    void clear();

    void swr_bounds(double max, double min = 1.0);
    void ohm_bounds(double max, double min = 0.0);
    void MHz_bounds(double max, double min)
;
    void colours(Fl_Color swr, Fl_Color x, Fl_Color r, Fl_Color z);

protected:
    // Overloaded methods
    virtual void draw();

    // Attributes
   
    sark_data* data_;

    // Max and min values
    double max_swr_;
    double min_swr_;
    double max_ohm_;
    double min_ohm_;
    double max_MHz_;
    double min_MHz_;

    // Chart line colours
    Fl_Color swr_colour_;
    Fl_Color r_colour_;
    Fl_Color x_colour_;
    Fl_Color z_colour_;
    
    // Widgets
    Fl_Group* charts_;
    Fl_Chart* swr_chart_;
    Fl_Chart* r_chart_;
    Fl_Chart* x_chart_;
    Fl_Chart* z_chart_;

    // Internal methods
    void draw_yl_axis();
    void draw_yr_axis();
    void draw_x_axis();

};
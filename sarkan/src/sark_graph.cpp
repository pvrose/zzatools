#include "sark_graph.h"
#include "sark_drawing.h"

#include <set>
#include <cmath>

using namespace std;

sark_graph::sark_graph(int X, int Y, int W, int H, const char*L) 
    : Fl_Group(X, Y, W, H, L) 
    , swr_colour_(FL_BLUE)
    , r_colour_(FL_GREEN)
    , x_colour_(FL_RED)
    , z_colour_(FL_BLACK)
    , min_swr_(1.0)
    , max_swr_(1.0)
    , min_ohm_(50.0)
    , max_ohm_(50.0)
{

    box(FL_BORDER_BOX);

    // Add gap for axis ticks
    int chart_x = X + GAP + AXIS_GAP;
    int chart_y = Y + GAP;
    int chart_w = W - AXIS_GAP - AXIS_GAP - GAP;
    int chart_h = H - AXIS_GAP - GAP;

    charts_ = new Fl_Group(chart_x, chart_y, chart_w, chart_h);
    charts_->box(FL_BORDER_BOX);
    charts_->color(FL_WHITE);
    charts_->begin();    

    swr_chart_ = new Fl_Chart(charts_->x(), charts_->y(), charts_->w(), charts_->h());
    swr_chart_->box(FL_NO_BOX);
    swr_chart_->type(FL_LINE_CHART);

    x_chart_ = new Fl_Chart(charts_->x(), charts_->y(), charts_->w(), charts_->h());
    x_chart_->box(FL_NO_BOX);
    x_chart_->type(FL_LINE_CHART);

    r_chart_ = new Fl_Chart(charts_->x(), charts_->y(), charts_->w(), charts_->h());
    r_chart_->box(FL_NO_BOX);
    r_chart_->type(FL_LINE_CHART);

    z_chart_ = new Fl_Chart(charts_->x(), charts_->y(), charts_->w(), charts_->h());
    z_chart_->box(FL_NO_BOX);
    z_chart_->type(FL_LINE_CHART);

    charts_->end();

    end();
    show();
    
}

sark_graph::~sark_graph() {}

void sark_graph::data(sark_data* d) {

    // Clear current data in charts
    clear();
    data_ = d;

    if (data_) {
        
        sark_data::comp_data p = data_->get_data(0);

        if (min_swr_ == max_swr_) {
            // Scan data for maximum and minimum
            for (size_t ix = 0; ix < data_->size(); ix++) {
                p = data_->get_data(ix);
                min_swr_ = min(min_swr_, p.SWR);
                max_swr_ = max(max_swr_, p.SWR);
            }
            // Limit max_swr_ to 25.0
            max_swr_ = min(25.0, max_swr_);
        }
        if (min_ohm_ == max_ohm_) {
            for (size_t ix = 1; ix < data_->size(); ix++) {
                min_ohm_ = min(min_ohm_, p.R);
                min_ohm_ = min(min_ohm_, p.X);
                min_ohm_ = min(min_ohm_, p.Z);
                max_ohm_ = max(max_ohm_, p.R);
                max_ohm_ = max(max_ohm_, p.X);
                max_ohm_ = max(max_ohm_, p.Z);
            }
            // Limit max/min ohmage to +/-10 k ohm 
            min_ohm_ = max(min_ohm_, -10000.0);
            max_ohm_ = min(max_ohm_, 10000.0);
            // If min_ohm is +ve make  it zero
            min_ohm_ = min(min_ohm_, 0.0);
         }
        min_MHz_ = (double)data_->get_params().start / 1000000;
        max_MHz_ = (double)data_->get_params().end / 1000000; 

        // Configure the charts - Y-axis limits
        swr_chart_->bounds(min_swr_, max_swr_);
        x_chart_->bounds(min_ohm_, max_ohm_);
        r_chart_->bounds(min_ohm_, max_ohm_);
        z_chart_->bounds(min_ohm_, max_ohm_);
        // Number of X-axis points
        swr_chart_->maxsize((int)data_->size());
        x_chart_->maxsize((int)data_->size());
        r_chart_->maxsize((int)data_->size());
        z_chart_->maxsize((int)data_->size());
        
        // Copy the data to the charts
        for (size_t ix = 0; ix < data_->size(); ix++) {
            p = data_->get_data(ix);
            swr_chart_->add(p.SWR, 0, swr_colour_);
            x_chart_->add(p.X, 0, x_colour_);
            r_chart_->add(p.R, 0, r_colour_);
            z_chart_->add(p.Z, 0, z_colour_);
        }
    }
}

void sark_graph::swr_bounds(double max, double min) {
    min_swr_ = min;
    max_swr_ = max;
    redraw();
}

void sark_graph::ohm_bounds(double max, double min) {
    min_ohm_ = min;
    max_ohm_ = max;
    redraw();
}

void sark_graph::MHz_bounds(double max, double min) {
    min_MHz_ = min;
    max_MHz_ = max;
    redraw();
}

void sark_graph::draw() {
    // Draw the charts
    Fl_Group::draw();

    // Now add the axes
    draw_yl_axis();
    draw_yr_axis();
    draw_x_axis();
}

void sark_graph::draw_yl_axis() {
    // Draw the SWR axis
    fl_color(FL_BLACK);
    int ax = charts_->x();
    int ay = charts_->y();
    int ah = charts_->h();

    fl_line(ax, ay, ax, ay + ah);
    // Now add the ticks - generate values
    set<double> ticks;
    double tick = 1.0;
    double gap;
    if (max_swr_ > 10) {
        gap = 2.0;
    } else if (max_swr_ > 3) {
        gap = 1.0;
    } else if (max_swr_ > 2) {
        gap = 0.25;
    } else if (max_swr_ > 1.1) {
        gap = 0.1;
    } else {
        gap = 0.01;
    }
    while (tick < max_swr_) {
        ticks.insert(tick);
        tick += gap;
    }
    double range = max_swr_ - min_swr_;
    double pixel_per_swr = (double)charts_->h() / range;
    // For each tick
    for (auto it = ticks.begin(); it != ticks.end(); it++) {
        int ty = ay + round((max_swr_ - *it) * pixel_per_swr);
        // Draw the tick
        fl_color(FL_BLACK);
        fl_line(x(), ty, ax, ty);
        // Label the tick
        char l[10];
        if (gap >= 1.0) {
            snprintf(l, sizeof(l), "%.0f:1", *it);
        } else {
            snprintf(l, sizeof(l), "%.2f:1", *it);
        }
        fl_draw(l, x(), ty - 1);
    }
}

void sark_graph::draw_yr_axis() {
       // Draw the SWR axis
    fl_color(FL_BLACK);
    int ax = charts_->x() + charts_->w();
    int ay = charts_->y();
    int ah = charts_->h();
    int axw = x() + w();

    fl_line(ax, ay, ax, ay + ah);
    // Now add the ticks - generate values
    set<double> ticks;
    double tick = 0.0;
    double range = max_ohm_ - min_ohm_;
    double gap;
    if (range > 5000.0) {
        gap = 500.0;
    } else if (range > 2000.0) {
        gap = 200.0;
    } else if (range > 1000.0) {
        gap = 100.0;
    } else if (range > 500.0) {
        gap = 50.0;
    } else if (range > 200.0) {
        gap = 20.0;
    } else if (range > 100.0) {
        gap = 10.0;
    } else if (range > 50.0) {
        gap = 5.0;
    } else if (range > 20.0) {
        gap = 2.0;
    } else {
        gap = 1.0;
    }
    tick = trunc(max_ohm_ / gap) * gap;
    while (tick > min_ohm_) {
        ticks.insert(tick);
        tick -= gap;
    }
    double pixel_per_ohm = (double)charts_->h() / range;
    // For each tick
    for (auto it = ticks.begin(); it != ticks.end(); it++) {
        int ty = ay + round((max_ohm_ - *it) * pixel_per_ohm);
        // Draw the tick
        fl_color(FL_BLACK);
        fl_line(ax, ty, axw, ty);
        // Label the tick
        char l[10];
        if (gap >= 100.0) {
            snprintf(l, sizeof(l), "%.1fk\316\251", (*it)/1000.0);
        } else {
            snprintf(l, sizeof(l), "%.0f\316\251", *it);
        }
        fl_draw(l, ax, ty - 1);
    }
}
void sark_graph::draw_x_axis() {
           // Draw the SWR axis
    fl_color(FL_BLACK);
    int ax = charts_->x();
    int ay = charts_->y() + charts_->h();
    int aw = charts_->w();

    fl_line(ax, ay, aw, ay);
    // Now add the ticks - generate values
    set<double> ticks;
    double tick = 0.0;
    double range = max_MHz_ - min_MHz_;
    double gap;
    if (range > 50.0) {
        gap = 5.0;
    } else if (range > 20.0) {
        gap = 2.0;
    } else if (range > 10.0) {
        gap = 1.0;
    } else if (range > 5.0) {
        gap = 0.5;
    } else if (range > 2.0) {
        gap = 0.2;
    } else if (range > 1.0) {
        gap = 0.1;
    } else if (range > 0.5) {
        gap = 0.5;
    } else if (range > 0.2) {
        gap = 0.2;
    } else {
        gap = 0.1;
    }
    double pixel_per_MHz = (double)charts_->w() / range;
    tick = trunc(max_MHz_ / gap) * gap;
    while (tick > min_MHz_) {
        ticks.insert(tick);
        tick -= gap;
    }
    // For each tick
    for (auto it = ticks.begin(); it != ticks.end(); it++) {
        int tx = ax + round((*it - min_MHz_) * pixel_per_MHz);
        // Draw the tick
        fl_color(FL_BLACK);
        fl_line(tx, ay, tx, y() + h());
        // Label the tick
        char l[10];
        if (gap >= 1.0) {
            snprintf(l, sizeof(l), "%.0fMHz", *it);
        } else {
            snprintf(l, sizeof(l), "%.1fMHz", *it);
        }
        fl_draw(270, l, tx + 1, ay);
    }

}

void sark_graph::colours(Fl_Color swr, Fl_Color x, Fl_Color r, Fl_Color z) {
    swr_colour_ = swr;
    x_colour_ = x;
    r_colour_ = r;
    z_colour_ = z;
    redraw();
}

void sark_graph::clear() {
    swr_chart_->clear();
    x_chart_->clear();
    r_chart_->clear();
    z_chart_->clear();
}

#include "xy_chart.h"

#include <cmath>

#include <FL/fl_draw.H>
#include <FL/Fl_Rect.H>

xy_chart::xy_chart(int X, int Y, int W, int H, const char* L) :
    Fl_Widget(X, Y, W, H, L)
{
    data_.clear();
    units_per_pixel_x_ = 0.0;
    units_per_pixel_y_ = 0.0;
    minimum_ = { DBL_MAX, DBL_MAX };
    maximum_ = { -DBL_MAX, -DBL_MIN };
    scale_width_ = 30;
}

xy_chart::~xy_chart() {
    data_.clear();
}

void xy_chart::draw() {
    // Draw the outlying box
    draw_box();
    // And the associated label
    draw_label();

    // Get the drawing rectangle - and restrict drawing inwith it
    Fl_Rect r(x(), y(), w(), h(), box());
    fl_push_clip(r.x(), r.y(), r.w(), r.h());
    // Main chart ares
    Fl_Rect chart(r.x() + scale_width_, r.y(), r.w() - scale_width_, r.h() - scale_width_);
    // Y scale
    Fl_Rect yscale(r.x(), r.y(), scale_width_, chart.h());
    // X scale
    Fl_Rect xscale(chart.x(), chart.b(), chart.w(), scale_width_);
    // Reserve area on left for Y scale
    Fl_Rect yscale(r.x(), r.y(), scale_width_, r.h() - scale_width_);
    // Reserve area at bottom for X scale
    Fl_Rect xscale(r.x() + scale_width_, r.b() - scale_width_, r.w() - scale_width_, scale_width_);

    // calculate units per pixel
    calc_uppx(chart);
    draw_chart(chart);
    draw_x_scale(xscale);
    draw_y_scale(yscale);

    fl_pop_clip();

}

// Data input method - add point
int xy_chart::add(unsigned int l, xy_chart::point p) {
    if (l >= data_.size()) {
        return -1;
    } else {
        data_[l].points.push_back(p);
        return data_[l].points.size() - 1;
    }
}

// Define line
int xy_chart::add_line(Fl_Color colour, unsigned int t) {
    line l;
    l.points.clear();
    l.colour = colour;
    l.thickness = t;
    data_.push_back(l);
    return data_.size() - 1;
}

// Set bounds
void xy_chart::bounds(xy_chart::point minimum, xy_chart::point maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
}

// Get bounds
void xy_chart::bounds(xy_chart::point* minimum, xy_chart::point* maximum) {
    *minimum = minimum_;
    *maximum = maximum_;
}

// Set automatic bounds
void xy_chart::auto_bounds() {
    // initialise 
    minimum_ = { nan(""), nan("") };
    maximum_ = { nan(""), nan("") };
    for (auto l = data_.begin(); l != data_.end(); l++) {
        for (auto p = (*l).points.begin(); p != (*l).points.end(); p++) {
            minimum_.X = fmin(minimum_.X, (*p).X);
            minimum_.Y = fmin(minimum_.Y, (*p).Y);
            maximum_.X = fmax(maximum_.X, (*p).X);
            maximum_.Y = fmax(maximum_.Y, (*p).Y);
        }
    }
}

// Drawing methods
void xy_chart::draw_y_scale(Fl_Rect& r) {
    // Draw base line
    fl_color(FL_BLACK);
    fl_line(r.r(), r.y(), r.r(), r.b());
    // Get smallest gap between ticks (in pixels)
    int pixel_gap = fl_height() + 2;
    double units_gap = pixel_gap * units_per_pixel_y_;
    double ten_power = 0;
    // Round up to 1, 2 or 5*10^N
    // First find decade
    while (units_gap <= 1.0) {
        units_gap *= 10.0;
        ten_power -= 1.0;
    }
    while (units_gap > 10.0) {
        units_gap /= 10.0;
        ten_power += 1.0;
    }
    // Now is it 2, 5 or 10 times
    if (units_gap <= 2.0) units_gap = 2.0;
    else if (units_gap <= 5.0) units_gap = 5.0;
    else units_gap = 10.0;
    // And add the power of ten
    units_gap = units_gap + pow(10, ten_power);
    // Now find the smallest tick vale above the minumum
    double tick_value = ceil(minimum_.Y / units_gap) * units_gap;
    // Now draw the ticks and label them
    char l[10];
    while (tick_value < maximum_.Y) {
        int yx = r.y() + y_pixel(tick_value);
        fl_line(r.x(), yx, r.r(), yx);
        if (abs(tick_value) > 1000.0) snprintf(l, sizeof(l), "%.0e", tick_value);
        else snprintf(l, sizeof(l), "%.0f", tick_value);
        fl_draw(l, r.x(), yx);
    }
}

void xy_chart::draw_x_scale(Fl_Rect& r) {
    // Draw base line
    fl_color(FL_BLACK);
    fl_line(r.x(), r.y(), r.r(), r.y());
    // Get smallest gap between ticks (in pixels)
    int pixel_gap = fl_height() + 2;
    double units_gap = pixel_gap * units_per_pixel_x_;
    double ten_power = 0;
    // Round up to 1, 2 or 5*10^N
    // First find decade
    while (units_gap <= 1.0) {
        units_gap *= 10.0;
        ten_power -= 1.0;
    }
    while (units_gap > 10.0) {
        units_gap /= 10.0;
        ten_power += 1.0;
    }
    // Now is it 2, 5 or 10 times
    if (units_gap <= 2.0) units_gap = 2.0;
    else if (units_gap <= 5.0) units_gap = 5.0;
    else units_gap = 10.0;
    // And add the power of ten
    units_gap = units_gap + pow(10, ten_power);
    // Now find the smallest tick vale above the minumum
    double tick_value = ceil(minimum_.X / units_gap) * units_gap;
    // Now draw the ticks and label them
    char l[10];
    while (tick_value < maximum_.X) {
        int yx = r.x() + x_pixel(tick_value);
        fl_line(yx, r.y(), yx, r.b());
        snprintf(l, sizeof(l), "%.0e", tick_value);
        fl_draw(90, l, yx, r.y());
    }
}

// Get pixel position for value
int xy_chart::y_pixel(double d) {
    // Get number of pixels down from the top
    int px = (maximum_.Y - d) / units_per_pixel_y_;
    return px;
}

int xy_chart::x_pixel(double d) {
    // Get the number of pixels to the right of the edge
    int px = (d - minimum_.X) / units_per_pixel_x_;
    return px;
}

// Draw chart
void xy_chart::draw_chart(Fl_Rect& r) {
    for (auto l = data_.begin(); l != data_.end(); l++) {
        if ((*l).points.size() < 2) {
            printf("Cannot draw a line with %d points\n", (*l).points.size());
        } else {
            fl_color((*l).colour);
            point a = (*l).points[0];
            for (size_t ix = 1; ix < (*l).points.size(); ix++) {
                point b = (*l).points[ix];
                draw_line(r, a, b, (*l).thickness);
                a = b;
            }
        }
    }
}

// Draw a line between a and b thickness t
void xy_chart::draw_line(Fl_Rect& r, xy_chart::point a, xy_chart::point b, unsigned int t) {
    int x0 = r.x() + x_pixel(a.X);
    int x1 = r.x() + x_pixel(b.X);
    int y0 = r.y() + y_pixel(a.Y);
    int y1 = r.y() + y_pixel(b.Y);
    if (t == 1) {
        // draw a line from a to b
        fl_line(x0, y0, x1, y1);
    } else {
        int x2, y2, x3, y3;
        // Draw a filled polygon centred on a straight line from a to b
        // Effectively smear the line up and down the x-axis
        x0 = x0;
        x1 = x1;
        x2 = x1;
        x3 = x0;
        y0 = y0 - (t / 2);
        y1 = y1 - (t / 2);
        y2 = y1 + t;
        y3 = y0 + t;
        fl_polygon(x0, y0, x1, y1, x2, y2, x3, y3);
    }

}

// Calculate units per pixel
void xy_chart::calc_uppx(Fl_Rect& r) {
    units_per_pixel_x_ = (maximum_.X - minimum_.X) / (double)r.w();
    units_per_pixel_y_ = (maximum_.Y - minimum_.Y) / (double)r.h();
}

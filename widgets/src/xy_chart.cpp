#include "xy_chart.h"

#include <cstdlib>
#include <cstdio>
#include <cfloat>
#include <cmath>

#include <Fl_Rect.H>
#include <Fl/fl_draw.H>

xy_chart::xy_chart(int X, int Y, int W, int H, const char* L) :
    Fl_Widget(X, Y, W, H, L)
{
    data_.clear();
    // Set bounds
    minimum_ = { nan(""), nan("") };
    maximum_ = { nan(""), nan("") };
    scale_width_ = 30;

}

xy_chart::~xy_chart() {
    data_.clear();
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
    line l = { {}, colour, t };
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

// Set automatic bounds to the minum and maximum of all points
void xy_chart::auto_bounds() {
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
void xy_chart::draw_scale(Fl_Rect& r, double min, double max, bool mirrored)
{
    bool horiz = r.w() > r.h();
    double units_per_pixel = (max - min) / (horiz ? r.w() : r.h());
    int minimum_tag = fl_height() * 2;
    double tag_step = units_per_pixel * minimum_tag;
    // Now adjust tag_step to 1eN, 2eN or 5eN 
    // find N by normalising 1.0 < S <= 10.0
    double ten_power = 0;
    while (tag_step <= 1.0) {
        tag_step *= 10.0;
        ten_power -= 1.0;
    }
    while (tag_step > 10) {
        tag_step /= 10.0;
        ten_power += 1.0;
    }
    // Now see if it's 2, 5 or 10
    if (tag_step <= 2.0) {
        tag_step = 2.0;
    } else if (tag_step <= 5.0) {
        tag_step = 5.0;
    } else {
        tag_step = 10.0;
    }
    // And multiply by the power of 10 again
    tag_step *= pow(10.0, ten_power);

    // Draw the base line
    if (horiz && mirrored) {
        // along bottom of rectangle
        fl_line(r.x(), r.b(), r.r(), r.b());
    } else if (horiz && !mirrored) {
        fl_line(r.x(), r.y(), r.r(), r.y());
    } else if (!horiz && mirrored) {
        fl_line(r.x(), r.y(), r.x(), r.b());
    } else {
        fl_line(r.r(), r.y(), r.r(), r.b());
    }
    // Find the first tag
    double tag = ceil(min / tag_step) * tag_step;
    while (tag < max) {
        // Draw each tag
        int pix = (tag - min) / units_per_pixel;
        char lab[10];
        if (tag >= 10000.0) {
            snprintf(lab, sizeof(lab), "%0.e", tag);
        } else {
            snprintf(lab, sizeof(lab), "%,0f", tag);
        }
        if (horiz) {
            fl_line(pix, r.y(), pix, r.b());
            fl_draw(90.0, lab, pix, r.b());
        } else {
            fl_line(r.x(), pix, r.r(), pix);
            fl_draw(lab, r.x(), pix);
        }
    }
}

void xy_chart::draw_chart(Fl_Rect& r) {
    // For each line
    for (auto l = data_.begin(); l != data_.end(); l++) {
        // Draw each segment
        if ((*l).points.size() < 2) {
            printf("Line has no segmants\n");
        } else {
            fl_color((*l).colour);
            fl_line_style(FL_SOLID, (*l).thickness);
            point a = (*l).points[0];
            for (size_t ix = 1; ix < (*l).points.size(); ix++) {
                point b = (*l).points[ix];
                draw_line(r, a, b, (*l).thickness);
                a = b;
            }
            fl_line_style(0);
        }
    }
}

// Draw a line between a and b thickness t
void xy_chart::draw_line(Fl_Rect& r, point a, point b, unsigned int t) {
    double x_per_px = (maximum_.X - minimum_.X) / r.w();
    double y_per_px = (maximum_.Y - minimum_.Y) / r.h();
    int x0 = round((a.X - minimum_.X) / x_per_px) + r.x();
    int x1 = round((b.X - minimum_.X) / x_per_px) + r.x();
    int y0 = round((maximum_.Y - a.Y) / y_per_px) + r.y();
    int y1 = round((maximum_.Y - b.Y) / y_per_px) + r.y();
    if (crosses(r, x0, y0, x1, y1)) {
        fl_line(x0, y0, x1, y1);
    }
}

// Check if line crosses or enters the rectangle
bool xy_chart::crosses(Fl_Rect& r, int x0, int y0, int x1, int y1) {
    if (x0 >= r.x() && x0 <= r.r() && y0 >= r.y() && y0 <= r.b()) {
        // Point 0 is inside the rectangle
        return true;
    } 
    if (x1 >= r.x() && x1 <= r.r() && y1 >= r.y() && y1 <= r.b()) {
        // Point 1 is inside the rectangle
        return true;
    } 
    if (x0 < r.x() && x1 < r.x()) {
        // both to the left
        return false;
    }
    if (x0 > r.r() && x1 > r.r()) {
        // both to the right
        return false;
    }
    if (y0 < r.y() && y1 < r.y()) {
        // both above
        return false;
    }
    if (y0 > r.b() && y1 > r.b()) {
        return false;
    }
    // The remaining cases require more complex analysis
    // Err on the side of caution and draw them for now
    return true;
}

// Overloaded draw function 
void xy_chart::draw() {
    // Use standard widget box and label drawing functions
    draw_box();
    draw_label();
    // Find drawable area
    Fl_Rect area(this);
    // Get chart area - TODO: add parameters for L/R T/B scales
    switch(type()) {
        case XYC_NORMAL: {
            // Y-scale on left, X-scale below
            int x0 = area.x();
            int x1 = area.x() + scale_width_;
            int x2 = area.r();
            int y0 = area.y();
            int y1 = area.b() - scale_width_;
            int y2 = area.b();
            Fl_Rect chart(x1, y0, x2 - x1, y1 - y0);
            Fl_Rect yscale(x0, y0, x1 - x0, y1 - y0);
            Fl_Rect xscale(x1, y1, x2 - x1, y2 - y1);
            draw_chart(chart);
            draw_scale(yscale, minimum_.Y, maximum_.Y, false);
            draw_scale(xscale, minimum_.X, maximum_.X, false);
            break;
        }
    }
}

// Set and get scale widths
void xy_chart::scale_width(int w) { 
    scale_width_ = w;
}
int xy_chart::scale_width() {
    return scale_width_;
}

#pragma once

#include <vector>
#include <cmath>
#include <cfloat>

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>

using namespace std;

class Fl_Rect;

class xy_chart : public Fl_Widget {
public:
    xy_chart(int X, int Y, int W, int H, const char* L = nullptr);
    ~xy_chart();

    // Overloaded methods
    void draw();

    // data structure - basic point
    struct point { 
        double X;
        double Y;
    };
    // Line definition
    struct line {
        vector<point> points;
        Fl_Color colour;
        unsigned int thickness;
    };

    // Data input method - add point
    int add(unsigned int l, point p);
    // Define line
    int add_line(Fl_Color colour, unsigned int t);

    // Set bounds
    void bounds(point minimum, point maximum);
    // Get bounds
    void bounds(point* minimum, point* maximum);
    // Set automatic bounds
    void auto_bounds();

protected:
    // The main data
    vector<line> data_;

    // units per pixel - x or y
    double units_per_pixel_x_;
    double units_per_pixel_y_;

    // drawing bounds
    point minimum_;
    point maximum_;

    // Scale width
    int scale_width_;

    // Drawing methods
    void draw_y_scale(Fl_Rect& r);
    void draw_x_scale(Fl_Rect& r);
    void draw_chart(Fl_Rect& r);

    // Draw a line between a and b thickness t
    void draw_line(Fl_Rect& r, point a, point b, unsigned int t);

    // Calculate units per pixel
    void calc_uppx(Fl_Rect& r);

    // Convert unit value to pixel position
    int y_pixel(double d);
    int x_pixel(double d);


};


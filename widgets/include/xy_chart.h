#pragma once

#include <vector>
#include <cmath>
#include <cfloat>

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>

using namespace std;

class Fl_Rect;

const uchar XYC_NORMAL = 0;
// TODO extend other chart types

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
    // Set/get scalewidth
    void scale_width(int w);
    int scale_width();
  
protected:
// The main data
    vector<line> data_;

    // drawing bounds
    point minimum_;
    point maximum_;

    // Scale width
    int scale_width_;

    // Drawing methods
    void draw_scale(Fl_Rect& r, double min, double max, bool mirrored);
    void draw_chart(Fl_Rect& r);

    // Draw a line between a and b thickness t
    void draw_line(Fl_Rect& r, point a, point b, unsigned int t);

     // Line segment crosses the rectangle
    bool crosses(Fl_Rect& r, int x0, int y0, int x1, int y1);
  
 };
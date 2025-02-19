#pragma once

#include "band_data.h"

#include <string>
#include <set>
#include <vector>
#include <map>

#include <FL/Fl_Widget.H>
#include <FL/Enumerations.H>

using namespace std;

// This widget includes a chart showing the band plan in graphic format
// It has two types: FULL, SUMMARY

class band_widget :
    public Fl_Widget
{
public:
    band_widget(int X, int Y, int W, int H, const char* L = nullptr);
    ~band_widget();

    virtual void draw();

    virtual int handle(int event);

    // Value - frequency
    void value(double f);
    double value();

    // Types
    static const uchar BAND_FULL = 0;     // Full display 
    static const uchar BAND_SUMMARY = 1;  // Summary display

protected:
    // data structures
    struct range_t {
        double lower;
        double upper;
    };                   // Frqeuency range
  


    // Draw the scale (from lower to upper +/- a bit)
    void draw_scale(range_t range);
    // Draw the current frequency
    void draw_current(double f);
    // Draw mode text
    void draw_subband(range_t range, double bw, string text);
    // Draw subband mode bars
    void draw_modebars(range_t range, set<string> modes);
    // Draw spot frequenct text
    void draw_spot_text(double f, string text);
    // Draw legend
    void draw_legend();
    // Generate data for band associated with frequency
    void generate_data(double f);
    // Rescale drawing
    void rescale();
    // Get Y-position for frequency
    int y_for_f(double f);
    // Get nearest text position to y
    int text_pos(int yt);

    // The data
    // The current value
    double value_; 
    // The band
    string band_;
    // The band limits
    range_t band_range_;
    // the data
    set<band_data::band_entry_t*> data_;
    // Defined modes
    map<string, int> modes_;
    // Major drawing positions
    int x_scale_;      // Position of scale axis
    int y_upper_;      // Upper position of scale
    int y_lower_;      // Lower position of scale
    int bar_width_;    // Width of mode bar
    int x_sub_;        // Position of sub band inclusion arrow
    int x_text_;       // Position of comments and subband text
    int x_kink1_;      // Positions to bend line between spot on scale and the text
    int x_kink2_;      // do.
    int x_freq_;       // Start position pf frequency text
    int h_offset_;     // Offset to centre text on a frequency
    int x_major_;      // Left end of major tick
    int x_minor_;      // Left end of minor tick
    int w_freq_;       // Width of frequency text
    range_t scale_range_;    // Scale range
    // Pixels per MHz on scale
    double px_per_MHz_;
    // Spacing for major tick (with frequency text)
    double major_tick_;
    // Spacing for minor tick 
    double minor_tick_;
    // Possible text positions - used
    vector<bool> text_in_use_;

};


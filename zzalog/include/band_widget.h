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
    static const uchar ZOOMABLE = 2;      // Allow zooming 
    static const uchar BAND_MASK = 1;  

protected:
    // data structures
    struct range_t {
        double lower;
        double upper;
    };                   // Frqeuency range
  
    enum marker_t : char {
        SUBBAND_UPPER,
        SUBBAND_LOWER,
        SUBBAND_LOCUM,       // If lower is outwith the frequency range
        SPOTGROUP_UPPER,
        SPOTGROUP_LOWER,
        CURRENT,
        CURRENT_LOCUM,
        SPOT
    };

    struct marker {
        double f;          // Frequency in MHz
        marker_t type;     // Type
        int y_scale;       // Position on scale
        int y_text;        // Position of text
        char* text;        // Text to display
    };

    struct mode_bar {
        std::string mode;       // Mode
        range_t range;     // Frequency range of mode bars
    };

    // Draw the scale (from lower to upper +/- a bit)
    void draw_scale(range_t range);
    // Draw legend
    void draw_legend();
    // Draw markers
    void draw_markers();
    // Draw modebars
    void draw_modebars();
    // Draw a single line
    void draw_line(int yl, int yr, int style);
    // Generate data for band associated with frequency
    void generate_data(double f);
    // Rescale drawing
    void rescale();
    // Add a marker in its correct position
    void add_marker(marker m);
    // Adjust markers
    void adjust_markers();
    // Reset markers
    void reset_markers();
    // Get Y-position for frequency
    int y_for_f(double f);
    // IS textual marker
    bool is_text_marker(marker m);
    // Generate markers and mode_bars
    void generate_items();
    // Format for drawing scale label
    const char* label_format();

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

    // Frequency markers
    vector<marker> markers_;
    // Mode bars
    vector<mode_bar> mode_bars_;
    // Don't Display spots
    bool ignore_spots_;
    // Already warned that we have removed spots once
    bool size_warned_;
    // Zoom value - 1.0 default scaling to fit available space
    double zoom_value_; 
    // Scroll delta - ie frequency off set of top from default
    double scroll_offset_;
    // Label format
    char label_format_[16];
};


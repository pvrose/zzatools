#pragma once

#include "band_data.h"

#include <string>
#include <set>
#include <vector>
#include <map>
#include <list>

#include <FL/Fl_Widget.H>
#include <FL/Enumerations.H>



//! This widget displays chart showing the band plan in graphic format

//! It has two types: BAND_FULL, BAND_SUMMARY plus a ZOOMABLE attribute std::set by type().
class band_widget :
    public Fl_Widget
{
public:
    //! Constructor 

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    band_widget(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor
    ~band_widget();
    //! Overload of Fl_Widget::draw()
    
    //! This method draws the various components of the bandplan schematic.
    virtual void draw();

    //! Overload of Fl_Widget::handle
    
    //! It responds to the follwoing actions
    //! - <B>Left Mouse</B> Invokes callback.
    //! - <B>Middle Mouse</B> Resets the frequency display to the current band.
    //! - <B>Mouse Wheel</B> Adjust zoom (horizontal or shift single wheel) and scroll.
    virtual int handle(int event);

    //! Overload of Fl_Widget::type(uchar t).
    
    //! Also calls default_mode.
    virtual void type(uchar t) {
        Fl_Widget::type(t);
        default_mode();
    }

    //! Overload of Fl_Widget::type().
    
    //! \return Fl_Widget::type()
    virtual uchar type() {
        return Fl_Widget::type();
    }

    //! Sets the frequency markers
    
    //! \param tx Set transmit frequency.
    //! \param rx Set receive frequency.
    void value(double tx, double rx);

    //! Get frequency at position
    
    //! \param x horizontal position
    //! \param y vertical position
    //! \return frequency shown by scale or marker depending where clicked.
    double frequency(int x, int y);

    // Type decodes
    static const uchar BAND_FULL = 0;     //!< Full display 
    static const uchar BAND_SUMMARY = 1;  //!< Summary display
    static const uchar ZOOMABLE = 2;      //!< Allow zooming 
    static const uchar BAND_MASK = 1;     //!< Mask for testing BAND_FULL or BAND_SUMMARY

protected:
    //! Type of marker - controls how the marker is handled and displayed
    enum marker_t : char {
        SUBBAND_UPPER,    //!< Upper limit of a sub-band.
        SUBBAND_LOWER,    //!< Lower limit of a sub-band.
        SUBBAND_LOCUM,    //!< Used if lower is outwith the displayed frequency range.
        SPOTGROUP_UPPER,  //!< Upper limit of a group of spot frequencies.
        SPOTGROUP_LOWER,  //!< Upper limit of a group of spot frequencies.
        CURRENT,          //!< Current transmit frequency.
        CURRENT_LOCUM,    //!< Used for transmit frequency if ooutwith the displayed range.
        CURRENT_RX,       //!< Current receive frequency.
        CURRENT_QSO,      //!< Frequency of log QSO if tig not connected.
        SPOT,             //!< Single spot frequency.
        USER,             //!< User Frequency
        BAND_UPPER,       //!< Upper limit of a band.
        BAND_LOWER,       //!< Lower limit of a band.
        BAND_LOCUM        //!< Used if lower is outwith the displayed frequency range.
    };
    //! Parameters describing a marker.
    struct marker {
        double f;          //!< Frequency in MHz
        marker_t type;     //!< Type
        int y_scale;       //!< y position on scale
        int y_text;        //!< y position of text
        char* text;        //!< Text to display
    };
    //! Parameters describing a mode bar.
    struct mode_bar {
        std::string mode;  //!< Mode the bar represents
        range_t range;     //!< Frequency range of mode bars
    };

    //! Draw the frequency scale (from lower to upper +/- a bit).
    void draw_scale();
    //! Draw legend.
    void draw_legend();
    //! Draw markers
    void draw_markers();
    //! Draw modebars
    void draw_modebars();
    //! Draw a single line
    
    //! \param yl y position of left end of line.
    //! \param yr y position of right end of line.
    //! \param style of the line per FLTK drawing.
    void draw_line(int yl, int yr, int style);
    //! Draw bands.
    void draw_bands();
    //! Generate data for band associated with frequency
    void process_data();
    //! Add a marker in its correct position
    
    //! \param m description of the marker.
    void add_marker(marker m);
    //! Adjust markers.
    
    //! Scan the std::list of markers iteratively adjusting the y position of the
    //! marker text until they all fit without overlapping. If necessary
    //! remove spot markers.
    void adjust_markers();
    //! Get Y-position for frequency
    
    //! \param f Frequency in MHz.
    //! \return y position on scale.
    int y_for_f(double f);
    //! Does the marker include text.
    
    //! \param m description of marker.
    //! \return true if the marker has text, false if not.
    bool is_text_marker(marker m);
    //! Generate markers and mode_bars
    void generate_items();
    //! Format for drawing scale label
    
    //! \return the print format to ensure at least 1 significant digit after the decimal point.
    const char* label_format();
    //! Set default modes
 
    //! Sets the various drawing modes depending on type() std::set.
    void default_mode();
    //! Calculate major and minor labels and display options
    
    //! Adjust the frequency scale to fit the height of the widgets. Adjust the
    //! major and minor marker intervals to keep minor markers close to 10 pixels.
    void set_verticals();
    //! Calculate horizontal positions
    
    //! Adjust the positions of the mode bars and text positions to fit width.
    void set_horizontals();
    //! Set scale range
    
    //! Set the frequency range of the display in response to zoom and scroll 
    //! mouse commands, or if \a restore_default is std::set restore to fit
    //! the band.
    //! \param restore_default std::set the frequency range of the display to 
    //! the full range of the band.
    void set_range(bool restore_default = false);

 
    // The data
    //! Current transmit or QSO frequency.
    double value_tx_;
    //! Current receive frequency.
    double value_rx_; 
    //! Band name.
    std::string band_;
    //! Frequency range of the band.
    range_t band_limits_;
    //! Complete std::set of band_entry_t items.
    std::set<band_data::band_entry_t*, band_data::ptr_lt> data_;
    //! Used to std::map mode name to mode bar number.
    std::map<std::string, int> modes_;
    //! Used to std::map modes that are within the range.
    std::map<std::string, int> used_modes_;
    // Major drawing positions
    int x_scale_;      //!< Position of scale axis.
    int y_upper_;      //!< Upper position of scale.
    int y_lower_;      //!< Lower position of scale.
    int bar_width_;    //!< Width of mode bar.
    int x_sub_;        //!< Position of sub band inclusion arrow.
    int x_text_;       //!< Position of comments and subband text.
    int x_kink1_;      //!< Positions to bend line between spot on scale and the text.
    int x_kink2_;      //!< Positions to bend line between spot on scale and the text.
    int x_freq_;       //!< Start position pf frequency text.
    int h_offset_;     //!< Offset to centre text on a frequency.
    int x_major_;      //!< Left end of major tick.
    int x_minor_;      //!< Left end of minor tick.
    int w_freq_;       //!< Width of frequency text.
    range_t scale_range_;    //!< Frequency range for the scale.
    //! Pixels per MHz on scale.
    double px_per_MHz_;
    //! Frequency spacing of a major tick.
    double major_tick_;
    //! Frequency spacing of a minor tick.
    double minor_tick_;

    //! All the markers to be displayed.
    std::list<marker> markers_;
    //! All the mode bars to be displayed.
    std::vector<mode_bar> mode_bars_;
    //! Format used to display frequency in widget label.
    char label_format_[16];
    // Modes
    bool display_bands_;    //!< Display bands as a different background
    bool display_spots_;    //!< Display spot and spot-band markers
    bool display_subbands_; //!< Display sub-band markers
    bool auto_bw_;          //!< automatically std::set bandwidth if value is within a band
    bool display_band_label_; 
                            //!< Add the label for the band
    bool verbose_;          //!< text displays in full mode
    bool zoomable_;         //!< The view is zoomable
    // Attributes
    double bandwidth_;      //!< Displayed bandwidth in MHz - actual
    // Median
    double median_;         //!< Median frequency in MHz - actual
    // Number of markers
    int num_spots_;         //!< Number of spot markers
    int num_subbands_;      //!< Number of subband markers
};


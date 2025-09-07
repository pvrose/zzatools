#pragma once
#include <FL/Fl_Group.H>

class band_window;
class band_widget;

//! This class displays the bandplan.
class qso_bands :
    public Fl_Group
{
public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_bands(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qso_bands();

    //! Overrife Fl_Group::handle() it accepts focus so that keyboard F1 launches the userguide.
    virtual int handle(int event);

    //! Load data from bands.tsv.
    void load_values();
    //! Instantiate component widgets.
    void create_form();
    //! Save data to bands.tsv.
    void save_values();
    //! Configure component widgets after data changes.
    void enable_widgets();

    //! Callback on clicking bandplan viewing widget.
    static void cb_band(Fl_Widget* w, void* v);

    //! Callback from ticker: every 1s update frequency displayed from rig or qso.
    static void cb_ticker(void* v);

protected:
    //! Bandplan widget.
    band_widget* summary_;
    //! Window to display larger version of bandplan.
    band_window* full_window_;

    // Saved windows coordinates
    int left_;       //!< X coordinate
    int top_;        //!< Y coordinate
    int width_;      //!< Width
    int height_;     //!< Height
    //! Flag, if true opens the larger view on start-up.
    bool open_window_; 

};


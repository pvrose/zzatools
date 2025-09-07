#pragma once

#include <FL/Fl_Group.H>

class qso_clock;
class qso_wx;

//! This class contains the clock and weather displays.
class qso_clocks :
    public Fl_Group
{
public:

    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_clocks(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qso_clocks();

    //! Override Fl_Group::handle accepts focue to enable keyboard F1 to open userguide.
    virtual int handle(int event);

    //! Loads previous timezone from settings
    void load_values();
    //! Instatntiate component widgets.
    void create_form();
    //! Save current timezone to settings.
    void save_values();
    //! Configure widgets after data changes.
    void enable_widgets();
    // Returns whether clock is displaying system locale's timezone.
    bool is_local();

protected:
    // The two instances
    qso_clock* clock_;         //!< Clock display.
    qso_wx* qso_weather_;      //!< Weather display.

    //! Clock is displaying timezone from system locale.
    bool local_;

};


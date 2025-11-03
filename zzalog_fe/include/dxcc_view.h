#pragma once

#include "view.h"
#include "dxcc_table.h"
#include "fields.h"

#include <FL/Fl_Group.H>

class Fl_Check_Button;

//! Container for dxcc_table. 
class dxcc_view :
    //! Inherits from Fl_Group and view.
    public Fl_Group, public view
{
public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    //! \param f ignored (required by view).
    dxcc_view(int X, int Y, int W, int H, const char* L = nullptr, field_app_t f = FO_LAST);
    // Destructor.
    ~dxcc_view();

    //! Overload of Fl_Group::handle().
    
    //! Clicking sets focus to enable F1 to launch userguide reader.
    virtual int handle(int event);

    //! Instantiate component widgets.
    void create_form();
    //! Configure widgets depending on data.
    void enable_widgets();
    //! Load configuration data from settinsg.
    void load_data();
    //! Store configuration settings.
    void store_data();

    //! something has changed in the book - reevaluates all the data.
    virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);

    //! Callback when a display mode has changed.
    static void cb_display(Fl_Widget* w, void* v);
    //! Callback ehen a confirmation mode has changed.
    static void cb_confirm(Fl_Widget* w, void* v);

protected:
    //! Display type.
    dxcc_table::display_t display_type_;
    //! Confirmation mode.
    dxcc_table::confirm_t confirm_type_;

    dxcc_table* table_;             //!< Table view of data
    Fl_Group* rg_display_;          //!< To group display mode radio buttons.
    Fl_Check_Button* bn_total_;     //!< Button to display only totals.
    Fl_Check_Button* bn_bands_;     //!< Button to display by bands.
    Fl_Check_Button* bn_modes_;     //!< Button to display by modes.
    Fl_Check_Button* bn_dmodes_;    //!< Button to display by DXCC modes.
    Fl_Group* bg_confirm_;          //!< Group confirmation mode buttons.
    Fl_Check_Button* bn_eqsl_;      //!< Button for confirmed by eQSL.cc
    Fl_Check_Button* bn_lotw_;      //!< Button for confirmed by Logbook of the World.
    Fl_Check_Button* bn_card_;      //!< Button for confirmed by physical card.

};


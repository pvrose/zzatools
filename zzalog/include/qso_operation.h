#pragma once

#include "utils.h"

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Input_Choice.H>



class qso_data;
class record;


//! This class is a special version of Fl_Input_Choice.

//! It allows annoated menu items. The real value represented by a menu item
//! is kept in that item's user data.
class annotated_choice :
    public Fl_Input_Choice 
{
public:

    //! Callback when menu item is selected: copies userdata into the input widget.
    static void cb_menu(Fl_Widget* w, void* v) {
        annotated_choice* that = ancestor_view<annotated_choice>(w);
        const char* val = that->menubutton()->text();
        const char* pos = strstr(val, ":--->");
        if (pos == nullptr) {
            that->input()->value(val);
        } else {
            char* nval = new char[pos - val + 1];
            memset(nval, 0, pos - val + 1);
            strncpy(nval, val, pos - val);
            that->input()->value(nval);
        }
        // Pretend that the enter key was pressed for the outer when
        that->do_callback(FL_REASON_ENTER_KEY);
    }

    //! Callback when input widget is changed: emulates the enter ket being pressed.
    static void cb_inp(Fl_Widget* w, void* v) {
        annotated_choice* that = ancestor_view<annotated_choice>(w);
        that->do_callback(FL_REASON_ENTER_KEY);
    }

    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    annotated_choice(int X, int Y, int W, int H, const char* L = nullptr) :
    Fl_Input_Choice(X, Y, W, H, L) 
    {
        menubutton()->callback(cb_menu);
        input()->callback(cb_inp);
        input()->when(FL_WHEN_ENTER_KEY);
    }
};

//! This class provides the means of changing station credentials.
//! 
//! It allows the station location, station operator and station to be selected.
class qso_operation :
    public Fl_Group
{
public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_operation(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qso_operation();

    //! Inherited from Fl_Group::handle(): it allows F1 to open userguide.
    virtual int handle(int event);

    //! Instantiate component widgets.
    void create_form();
    //! Configure component widgets after data chaange.
    void enable_widgets();
    //! Load previous session's values from settings.
    void load_data();
    //! SAve current settings to settings.
    void store_data();

    //! Callback from "QTH" input.
    static void cb_qth(Fl_Widget* w, void* v);
    //! Callback from "Operator" input. 
    static void cb_oper(Fl_Widget* w, void* v);
    //! Callback from "Station Callsign" input.
    static void cb_call(Fl_Widget* w, void* v);

    //! Callback from the "Show" buttons: \p v specifes the specific button.
    static void cb_show(Fl_Widget*w, void* v);

    //! std::set the QSO record \p qso.
    void qso(record* qso);

    //! Returns the current QTH. 
    std::string current_qth();
    //! Returns the current operator.
    std::string current_oper();
    //! Returns the current station callsign.
    std::string current_call();

    //! Update QSO record \p qso from current selected values.
    void update_qso(record* qso);
    //! Reload data and configure widgets.
    void update_details();
    //! Populate the choices
    void populate_choices();

protected:
    //! A new QTH has been entered, open edit dialog.
    void new_qth();
    //! A new Operator has been entered, open edit dialog.
    void new_oper();
    //! A new station callsign has been entered, open edit dialog.
    void new_call();

    // Widgets
    Fl_Input_Choice* ch_qth_;      //!< Input for specifyinh station location
    Fl_Input_Choice* ch_oper_;     //!< Input for specifying station operator
    Fl_Input_Choice* ch_call_;     //!< Input for specifying station callsign.

    // Attributes
    std::string current_qth_;      //!< Current station location.
    std::string current_oper_;     //!< Current station operator.
    std::string current_call_;     //!< Current station callsign.
    record* current_qso_;     //!< Current QSO record.


};


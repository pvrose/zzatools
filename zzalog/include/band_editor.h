#pragma once

#include "band_data.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Scroll.H>

class Fl_Button;
class Fl_Check_Button;
class Fl_Choice;
class Fl_Float_Input;
class Fl_Input;

class band_row;

//! This class provides the editor widget for band_data.
class band_table :
    public Fl_Scroll{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    band_table(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor
    ~band_table();

    //! Set the selected entry
    void selected(band_row* r);

    //! Get the selected row
    band_row* selected();

    //! Select the row with \p entry
    void select_with_entry(band_data::band_entry_t* entry);

    //! Draw all the internal widgets
    void draw_widgets();

protected:

    //! Selected entry
    band_row* selected_row_;

 };

//! This class provides the ability to edit band_data.
class band_editor :
    public Fl_Group
{
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    band_editor(int X, int Y, int W, int H, const char* L = nullptr);
    
    //! Destructor
    ~band_editor();

    //!Callback from add row button
    static void cb_add(Fl_Widget* w, void* v);

    //! Callback from delete row button
    static void cb_delete(Fl_Widget* w, void* v);

    //! CAllback from re-order button
    static void cb_reorder(Fl_Widget* w, void* v);

    //! Set the \p frequency
    void value(double frequency);


protected:

    //! Frequency a scroll is made to
    double scroll_freq_;

    Fl_Button* bn_add_;          //!< Add an entry
    Fl_Button* bn_delete_;       //!< Delete an entry
    Fl_Button* bn_reorder_;      //!< Correct the row order
    band_table* table_;          //!< Table of band entries for editing

};

//! This class uses Fl_Input_Choice to provide a custom choice.

//! The Fl_input is configured as output only and reflects the
//! menu ptions that are selected
class band_modechoice :
    public Fl_Input_Choice {
public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    band_modechoice(int X, int Y, int, int H, const char* L = nullptr);

    //! Desctructor
    ~band_modechoice();

    //! Menu callback
    static void cb_menu(Fl_Widget* w, void* v);

    //! Value - returns all selected modes
    std::set < std::string > value();

    //! Set value
    void value(std::set<std::string>);

};

//! This class provides a line of the band_table
class band_row :
    public Fl_Group {

public:
    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    band_row(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor
    ~band_row();

    //! Set data entry
    void entry(band_data::band_entry_t* e);

    //! Get entry
    band_data::band_entry_t* entry();

    //! Set selected
    void selected(bool value);

    //! Callback from the select check buttom
    static void cb_select(Fl_Widget* w, void* v);
    //! Callback from the Type entry field.
    static void cb_type(Fl_Widget* w, void* v);
    //! Callback from the Upper entry field.
    static void cb_upper(Fl_Widget* w, void* v);
    //! Callback from the Lower entry field.
    static void cb_lower(Fl_Widget* w, void* v);
    //! Callback from the bandwidth entry field.
    static void cb_width(Fl_Widget* w, void* v);
    //! Callback from the Modes entry field.
    static void cb_modes(Fl_Widget* w, void* v);
    //! Callback from the Description entry field.
    static void cb_description(Fl_Widget* w, void* v);

protected:

    //! Populate the type choices
    void populate_type(Fl_Choice* ch);

    //! Populate the mode choices
    void populate_mode(band_modechoice* ch);
    
    //! Enable widgets - depends on band_data::entry_t type.
    void enable_widgets();

    band_data::band_entry_t* entry_;

    Fl_Check_Button* w_select_;    //!< Row selected
    Fl_Choice* w_type_;            //!< Type of entry choice.
    Fl_Float_Input* w_lower_;      //!< Lower frequency input.
    Fl_Float_Input* w_upper_;      //!< Upper frequency input.
    Fl_Float_Input* w_width_;      //!< Bandwidth.
    band_modechoice* w_modes_;     //!< Modes choice.
    Fl_Input* w_description_;      //!< Description.

};
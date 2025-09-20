#pragma once

#include <string>

#include <FL/Fl_Double_Window.H>

class Fl_Button;
class Fl_Input;
class Fl_Radio_Round_Button;

enum stn_type : uchar;
struct stn_default;

//! This class provides a dialog to enter data for a new installation
class init_dialog :
    public Fl_Double_Window
{
public:

    //! Constructor - sizes and labels itself
    init_dialog();

    //! Destructor
    ~init_dialog();

    //! Instantiate the component widgets
    void create_form();

    //! Copy initial values from staion_defaults_ if the exist
    void load_values();

    //! Remember details in settings to set club login
    void save_values();

    //! Returns the station details
    stn_default get_default();

protected:
    //! Callback when "Accept" button clicked
    static void cb_accept(Fl_Widget* w, void* v);

    //! Callback when radio button is selected
    
    //! \param w: widget clicked
    //! \param v: object of type stn_type indicates the new station type
    static void cb_type(Fl_Widget* w, void* v);
    
    // Widgets
    Fl_Radio_Round_Button* bn_club_;        //!< Club station
    Fl_Radio_Round_Button* bn_indiv_;       //!< Individual station
    Fl_Input* ip_call_;                     //!< Station callsign
    Fl_Input* ip_location_;                 //!< Location name
    Fl_Input* ip_grid_;                     //!< Gridsquare locator
    Fl_Input* ip_name_;                     //!< Club or individual's name
    Fl_Button* bn_accept_;                  //!< Accept these data items

    stn_type station_type_;                 //!< Club or individual
    stn_default* default_station_;          //!< Default details


};


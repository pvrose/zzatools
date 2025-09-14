#pragma once

#include "win_dialog.h"

#include <string>


class Fl_Button;
class Fl_Input;
class Fl_Input_Choice;



//! This class provides the log-in dialog for a club station operator.
class club_stn_dlg :
    public win_dialog
{
public:
    //! Constructor.
    club_stn_dlg();
    //! Destructor.
    ~club_stn_dlg();

    //! Instantiate all the components of the dialog.
    void create_form();

    //! Callback to accept login details: updates operator database and exits.
    static void cb_bn_login(Fl_Widget* w, void* v);
    //! Callback after enytering or selecting the operator: updates the dialog.
    static void cb_operator(Fl_Widget* w, void* v);
    //! Callback after entering or selecting QTH: updatesthe dialog/
    static void cb_edit_qth(Fl_Widget* w, void* v);

protected:
    //! Reads the operator data from settings into the internal database.
    void load_data();
    //! Writes the internal database back to settings.
    void store_data();
    //! Updates the dialog after changing data.
    void enable_widgets();
    //! Populates the operator choice menu with known operators from the database.
    void populate_login();
    //! Add the entered login of not known and update nickname and callsign if changed.
    void add_login();
    //! Add the entered station callsign if not known.
    void add_callsign();

    std::string club_name_;       //!< Name of the club.
    std::string club_call_;       //!< Station callsign of the club.
    std::string club_location_;   //!< Location of the club station.
    std::string nickname_;        //!< Nickname of the operator.

    Fl_Input* w_club_name_;       //!< input for club name.
    Fl_Input* w_club_call_;       //!< input for club station callsign.
    Fl_Input* w_club_location_;   //!< input for club station location.
    Fl_Button* w_edit_qth_;       //!< button to open QTH edit dialog. 
    Fl_Input_Choice* w_operator_; //!< choice and input for operator.
    Fl_Input* w_name_;            //!< input for operator's name.
    Fl_Input* w_call_;            //!< input for oeprator's callsign.
    Fl_Button* w_login_;          //!< button to action the login.

};


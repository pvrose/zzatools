#pragma once

#include "win_dialog.h"

#include <string>


class Fl_Button;
class Fl_Input;
class Fl_Input_Choice;

using namespace std;

class club_stn_dlg :
    public win_dialog
{
public:
    club_stn_dlg();
    ~club_stn_dlg();

    void create_form();

    static void cb_bn_login(Fl_Widget* w, void* v);
    static void cb_operator(Fl_Widget* w, void* v);
    static void cb_edit_qth(Fl_Widget* w, void* v);

protected:
    void load_data();
    void store_data();
    void enable_widgets();
    void populate_login();
    void add_login();
    void add_callsign();

    string club_name_;
    string club_call_;
    string club_location_;
    string nickname_;

    Fl_Input* w_club_name_;
    Fl_Input* w_club_call_;
    Fl_Input* w_club_location_;
    Fl_Button* w_edit_qth_;
    Fl_Input_Choice* w_operator_;
    Fl_Input* w_name_;
    Fl_Input* w_call_;
    Fl_Button* w_login_;

};


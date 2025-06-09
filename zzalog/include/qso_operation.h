#pragma once

#include "utils.h"

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Input_Choice.H>

using namespace std;

class qso_data;
class record;


// Special version of Fl_Input_Choice that allows annoated menu items - value is user_data
class annotated_choice :
    public Fl_Input_Choice 
{
public:

    // Copy user data rather than value to input
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

    // Override input callback to pick up 
    static void cb_inp(Fl_Widget* w, void* v) {
        annotated_choice* that = ancestor_view<annotated_choice>(w);
        that->do_callback(FL_REASON_ENTER_KEY);
    }

    annotated_choice(int X, int Y, int W, int H, const char* L = nullptr) :
    Fl_Input_Choice(X, Y, W, H, L) 
    {
        menubutton()->callback(cb_menu);
        input()->callback(cb_inp);
        input()->when(FL_WHEN_ENTER_KEY);
    }
};

class qso_operation :
    public Fl_Group
{
public:
    qso_operation(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_operation();

    void create_form();
    void enable_widgets();

    static void cb_qth(Fl_Widget* w, void* v);
    static void cb_oper(Fl_Widget* w, void* v);
    static void cb_call(Fl_Widget* w, void* v);

    static void cb_show(Fl_Widget*w, void* v);

    // set the QSO
    void qso(record* qso);

    // get the current QTH 
    string current_qth();
    string current_oper();
    string current_call();

    // Update QSO from current values
    void update_qso(record* qso);

protected:
    // Populate the choices
    void populate_choices();
    // Try and evaluate the current operation from QSO
    bool evaluate_qso();
    // Handle new QTH 
    void new_qth();
    // handle new Operator
    void new_oper();
    // Handle new call
    void new_call();

    // Widgets
    Fl_Input_Choice* ch_qth_;
    Fl_Input_Choice* ch_oper_;
    Fl_Input_Choice* ch_call_;

    // Attributes
    string current_qth_;
    string current_oper_;
    string current_call_;
    record* current_qso_;


};


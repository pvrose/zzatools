#pragma once

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class qso_data;
class Fl_Input_Choice;
class record;

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


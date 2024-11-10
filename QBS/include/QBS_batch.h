#pragma once

#include <string>

#include <FL/Fl_Group.H>

class QBS_data;
class QBS_top20;
class QBS_window;
class Fl_Button;
class Fl_Float_Input;
class Fl_Input_Choice;
class Fl_Output;


using namespace std;

class QBS_batch :
    public Fl_Group
{
public:

    QBS_batch(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_batch();

    void create_form();
    void enable_widgets();

    void initialise();
    void populate_batch_choice();


protected:

    static void cb_back(Fl_Widget* w, void* v);
    static void cb_execute(Fl_Widget* w, void* v);
    static void cb_next(Fl_Widget* w, void* v);
    static void cb_batch(Fl_Widget* w, void* v);

    void execute_new();
    void execute_recycle();
    void execute_post();
    void execute_finish();


    QBS_window* win_;
    QBS_data* data_;

    Fl_Input_Choice* ch_batch_;

    Fl_Output* op_rcvd_calls_;
    Fl_Output* op_sent_calls_;
    Fl_Output* op_rcyc_calls_;
    Fl_Output* op_held_calls_;

    Fl_Output* op_rcvd_cards_;
    Fl_Output* op_sent_cards_;
    Fl_Output* op_rcyc_cards_;
    Fl_Output* op_held_cards_;

    Fl_Output* op_rcvd_date_;
    Fl_Output* op_sent_date_;
    Fl_Output* op_rcyc_date_;

    Fl_Float_Input* ip_weight_;

    QBS_top20* tab_top20_;

    Fl_Button* bn_back_;
    Fl_Button* bn_execute_;
    Fl_Button* bn_next_;

    float weight_;
    string date_;
    // The box number to be/have been processed
    int box_;

    // Indicates that this has been executed
    bool executed_;
};


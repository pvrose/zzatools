#pragma once

#include <string>

#include <FL/Fl_Group.H>

class QBS_data;
class QBS_window;
class Fl_Button;
class Fl_Float_Input;
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

protected:

    static void cb_back(Fl_Widget* w, void* v);
    static void cb_execute(Fl_Widget* w, void* v);
    static void cb_done(Fl_Widget* w, void* v);

    void execute_new();
    void execute_recycle();
    void execute_post();
    void execute_finish();


    QBS_window* win_;
    QBS_data* data_;

    Fl_Output* op_rcvd_calls_;
    Fl_Output* op_sent_calls_;
    Fl_Output* op_rcyc_calls_;

    Fl_Output* op_rcvd_cards_;
    Fl_Output* op_sent_cards_;
    Fl_Output* op_rcyc_cards_;

    Fl_Float_Input* ip_weight_;

    Fl_Button* bn_back_;
    Fl_Button* bn_execute_;
    Fl_Button* bn_done_;

    float weight_;
    string date_;
};


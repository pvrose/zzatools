#pragma once
#include <FL/Fl_Group.H>

#include <string>

class QBS_window;
class QBS_data;
class Fl_Widget;
class Fl_Button;
class Fl_Output;
class Fl_Int_Input;

using namespace std;

class QBS_dormant :
    public Fl_Group
{
public:
    QBS_dormant(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_dormant();

    void create_form();
    void enable_widgets();

    // Callback - Receive ad-hoc cards
    static void cb_action(Fl_Widget* w, void* v);

    QBS_window* win_;
    QBS_data* data_;
    string date_;

    string current_;
    string next_;
    string head_;

protected:

    // callsign for "Receive QSL" and "Receive SASEs"
    string callsign_;
    // Number of cards or envelopes received
    int num_received_;

    // Buttons that can be disabled
    Fl_Button* bn_receive_card_;
    Fl_Button* bn_receive_sase_;
    Fl_Button* bn_new_batch_;
    Fl_Output* op_new_batch_;
    Fl_Button* bn_recycle_;
    Fl_Output* op_recycle_;
    Fl_Button* bn_edit_;
    Fl_Button* bn_reports_;



};


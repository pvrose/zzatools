#pragma once
#include <FL\Fl_Group.H>

#include <string>

class QBS_window;
class QBS_data;
class Fl_Widget;
class Fl_Button;

using namespace std;

class QBS_dormant :
    public Fl_Group
{
public:
    QBS_dormant(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_dormant();

    void create_form();
    void enable_widgets();

    // Callback - Receive SASEs
    static void cb_receive_sases(Fl_Widget* w, void* v);
    // Callback - New Batch
    static void cb_new_batch(Fl_Widget* w, void* v);
    // Callback - Recycle
    static void cb_recycle(Fl_Widget* w, void* v);
    // Callback - Edit
    static void cb_edit(Fl_Widget* w, void* v);
    // Callback - Reports
    static void cb_reports(Fl_Widget* w, void* v);
    // callback - callsign input
    static void cb_callsign(Fl_Widget* w, void* v);
    // callback - numeric input

    QBS_window* win_;
    QBS_data* data_;

protected:
    // callsign for "Receive QSL" and "Receive SASEs"
    string callsign_;
    // Number of cards or envelopes received
    int num_received_;

    // Buttons that can be disabled
    Fl_Button* bn_receive_sase_;


};


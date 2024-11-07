#pragma once

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class QBS_card_table;
class QBS_charth;
class QBS_data;
class QBS_window;
class Fl_Button;
class Fl_Input;
class Fl_Int_Input;

class QBS_call :
    public Fl_Group
{
public:
    QBS_call(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_call();

    void create_form();
    void enable_widgets();

    static void cb_back(Fl_Widget* w, void* v);
    static void cb_execute(Fl_Widget* w, void* v);
    static void cb_done(Fl_Widget* w, void* v);
    static void cb_call(Fl_Widget* w, void* v);

protected:
    // Process specific executes
    void execute_log_card();
    void execute_log_sase();
    void execute_sort();
    void execute_process();

    void stuff_cards(int box, int& cards);

    // Fixed widgets
    Fl_Input* ip_call_;
    QBS_card_table* tb_holding_;
    QBS_charth* ch_history_;
    Fl_Button* bn_back_;
    Fl_Button* bn_execute_;
    Fl_Button* bn_done_;
    // Group 1 - Sorting, log card, log SASE
    Fl_Group* gp_add_item_;
    Fl_Int_Input* ip_add_qty_;
    // Group 2 - Processing
    Fl_Group* gp_process_;
    Fl_Int_Input* ip_stuff_;
    Fl_Int_Input* ip_keep_;
    Fl_Int_Input* ip_sases_;

    // References
    QBS_window* win_;
    QBS_data* data_;

    // Attributes
    // Callsign being processed
    string call_;
    // Number of cards/SASEs toadd
    int add_qty_;
    // Number of cards going to SASEs
    int stuff_qty_;
    // Number of cards to keep
    int keep_qty_;
    // Number of SASEs used
    int sases_qty_;
    // Current date
    string date_;



};


#pragma once
/******************************************************************
 * QBS_process
 * Main processing interface

*/
#include "QBS_data.h"

#include "FL/Fl_Group.H"
#include "FL/Fl_Widget.H"

class QBS_process :
    public Fl_Group
{

public:
    QBS_process(int X, int Y, int W, int H, const char* L);
    virtual ~QBS_process();

    void update();

protected:
    enum action_t {
        RECEIVE_BATCH,
        ADD_CARD,
        ADD_SASE,
        SEND_CARD,
        KEEP_CARD,
        DISPOSE_BATCH,
        DISPOSE_SASE
    };

    // Call input - select existing or new call
    static void cb_ip_call(Fl_Widget* w, void* v);
    // Button - new batch
    static void cb_bn_add(Fl_Widget* w, void* v);
    // Button - dispose batch
    static void cb_bn_dispose(Fl_Widget* w, void* v);
    // Button - Exceute action
    static void cb_bn_execute(Fl_Widget* w, void* v);
    // Radio button - add card, add SASE, Send batch, dispose SASE
    static void cb_bn_action(Fl_Widget* w, void* v);


    void load_values();
    void create_form();
    void save_values();
    void update_batches();
    void update_counts();
    void update_actions();
    void execute_action();

    action_t action_;
    string callsign_;
    int num_cards_;
    int num_sases_;
    float weight_disposed_;

    QBS_data* data_;

    // Widgets to read or need configuration
    // - inputs
    Fl_Widget* w_num_cards_;
    Fl_Widget* w_num_sases_;
    // - radio buttons
    Fl_Widget* w_add_card_;
    Fl_Widget* w_add_sase_;
    Fl_Widget* w_send_card_;
    Fl_Widget* w_keep_card_;
    Fl_Widget* w_disp_sase_;
    // - outputs
    Fl_Widget* w_current_;
    Fl_Widget* w_disposal_;
    Fl_Widget* w_count_[QD_NUM_DIRECTION][QI_NUM_ITEM_TYPE];

};


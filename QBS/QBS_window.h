#pragma once

#include "QBS_consts.h"
#include "QBS_notes.h"
#include "../zzalib/utils.h"
#include "../zzalib/callback.h"

#include <map>
#include <string>

#include "FL/Fl_Single_Window.H"
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Table.H>


using namespace std;

// SEverity level used for message
enum severity_t {
    NOT_ERROR = 0,
    WARNING,
    ERROR,
    SEVERE
};

// Forward instantiation otherwise we have circular use of .h files
class QBS_data;
class QBS_notes;

/****************************************************
*  QBS_window
* Top-level window that can be switched to differnt functional views
*****************************************************/
class QBS_window :
    public Fl_Single_Window
{
public:
    QBS_window(int W, int H, const char* L, const char* filename);
    virtual ~QBS_window();
    // Respond to QBS_data mode
    void update_actions();
    void update_import(float loaded);
    void update_qbs(int loaded);
    // Update filename
    void filename(const char* value);
    void directory(const char* value);

protected:

    // Close -
    static void cb_close(Fl_Widget* w, void* v);
    // Button callback - import files
    static void cb_import(Fl_Widget* w, void* v);
    // Read QBS
    static void cb_read_qbs(Fl_Widget* w, void* v);
    // Process selection buttons
    static void cb_process(Fl_Widget* w, void* v);
    // Exceute batch action
    static void cb_exec_batch(Fl_Widget* w, void* v);
    // Execute call action
    static void cb_exec_call(Fl_Widget* w, void* v);
    // Call input 
    static void cb_input_call(Fl_Widget* w, void* v);
    // navigation buttons - v used to pass user data of type navigate_t
    static void cb_nav_batch(Fl_Widget* w, void* v);
    static void cb_nav_call(Fl_Widget* w, void* v);
    // Reset button
    static void cb_reset(Fl_Widget* w, void* v);
 
    // Specify the widgets
    void create_form();

    // Configure widgets
    void update_new_batch();
    void update_sort_cards();
    void update_rcv_card();
    void update_rcv_sase();
    void update_stuff_cards();
    void update_dispose_cards();
    void update_post_cards();
    void update_recycle_cards();
    void update_dispose_sase();
    void update_batches(bool enable_nav);
    void update_calls(int box_num);
    void update_batch_summary();
    void update_call_summary();
    void update_call_history();
    void update_edit_notes();
    void hide_edit_notes();
    void update_action_values();
    void update_action_bn(Fl_Button* b, action_t a);
    // Call whatever update is needed per action
    void update_whatever();
 
    // Various data items accessible by most classes
    Fl_Preferences settings_;
    QBS_data* data_;

    // widgets - input group
    Fl_Group* g_input_;
    Fl_Button* bn_import_;
    Fl_Button* bn_read_qbs_;
    Fl_Input* ip_csv_dir_;
    Fl_Input* ip_qbs_file_;
    Fl_Button* bn_brf_qbs_;
    browser_data_t qbs_browser_;
    Fl_Button* bn_brf_csv_;
    browser_data_t csv_browser_;

    // widgets - process group
    Fl_Group* g_process_;
    Fl_Group* g_proc_bns_;

    Fl_Radio_Light_Button* bn_rcv_card_;
    radio_param_t rp_rcv_card_;
    Fl_Radio_Light_Button* bn_rcv_sase_;
    radio_param_t rp_rcv_sase_;
 
    Fl_Radio_Light_Button* bn_new_batch_;
    radio_param_t rp_new_batch_;
    Fl_Radio_Light_Button* bn_sort_cards_;
    radio_param_t rp_sort_cards_;
    Fl_Radio_Light_Button* bn_stuff_cards_;
    radio_param_t rp_stuff_cards_;
    Fl_Radio_Light_Button* bn_dispose_cards_;
    radio_param_t rp_dispose_cards_;
    Fl_Radio_Light_Button* bn_post_cards_;
    radio_param_t rp_post_cards_;
    Fl_Radio_Light_Button* bn_recycle_cards_;
    radio_param_t rp_recycle_cards_;
    Fl_Radio_Light_Button* bn_dispose_sase_;
    radio_param_t rp_dispose_sase_;

    Fl_Radio_Light_Button* bn_edit_notes_;
    radio_param_t rp_edit_notes_;

    Fl_Radio_Light_Button* bn_summ_batch_;
    radio_param_t rp_summ_batch_;
    Fl_Radio_Light_Button* bn_summ_call_;
    radio_param_t rp_summ_call_;
    Fl_Radio_Light_Button* bn_hist_call_;
    radio_param_t rp_hist_call_;

    Fl_Output* op_batch_;
    Fl_Button* bn_b_navbb_;
    Fl_Button* bn_b_navb_;
    Fl_Button* bn_b_navf_;
    Fl_Button* bn_b_navff_;
    Fl_Button* bn_b_action_;
    Fl_Input* ip_call_;
    Fl_Button* bn_c_navbbb_;
    Fl_Button* bn_c_navbb_;
    Fl_Button* bn_c_navb_;
    Fl_Button* bn_c_navf_;
    Fl_Button* bn_c_navff_;
    Fl_Button* bn_c_navfff_;
    Fl_Button* bn_c_action_;
    Fl_Button* bn_nav_n_do_;
    Fl_Box* bx_current_;
    Fl_Box* bx_change_;
    static const int NUM_COUNTS = 10;
    Fl_Value_Output* op_value_[NUM_COUNTS];
    Fl_Value_Input* ip_delta_[NUM_COUNTS];
    int index_inbox_;
    int index_sase_;
    int index_curr_;
    int index_head_;
    int index_keep_;
    int index_outbox_;
    int index_weight_;
    Fl_Button* bn_reset_;
    Fl_Button* bn_close_;


    QBS_notes* tab_old_notes_;
    Fl_Output* op_note_date_;
    Fl_Input* ip_note_name_;
    Fl_Input* ip_note_value_;

    // Command selection
    action_t action_;
    // CSV file directory
    string csv_directory_;
    // QBS filename
    string qbs_filename_;
    // Selected box number
    int selected_box_;
    // Current callsign
    string call_;
    // Batch operation finished - disables batch execute button
    bool batch_op_done_;
};


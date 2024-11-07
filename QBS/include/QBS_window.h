#pragma once

#include "QBS_consts.h"
//#include "QBS_notes.h"
#include "utils.h"
#include "callback.h"
//#include "QBS_charth.h"

#include <map>
#include <stack>
#include <string>
#include <vector>

#include "FL/Fl_Single_Window.H"

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
class QBS_file;
class QBS_dormant;
class QBS_call;
class QBS_batch;
class input_hierch;
class Fl_Wizard;

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
 /*   void update_import(float loaded);
    void update_qbs(int loaded);*/
    // Update filename
    void filename(const char* value);
    void directory(const char* value);
    // Open batch log
    void open_batch_log(string batch_name);

    // Get the current process
    process_mode_t process();
    // Push the proces
    void process(process_mode_t p);
    // Pop the process - and return new process
    process_mode_t pop_process();
    // Populate call choice with extant callsigns
    // TODO implement it in QBS_window.cpp
    void populate_call_choice(input_hierch* ch);
    // Write batch log
    void append_batch_log(const char* text);

protected:

    bool read_qbs();

    // Close -
    static void cb_close(Fl_Widget* w, void* v);
    //// Button callback - import files
    //static void cb_import(Fl_Widget* w, void* v);
    //// Read QBS
    //static void cb_read_qbs(Fl_Widget* w, void* v);
    //// Process selection buttons
    //static void cb_process(Fl_Widget* w, void* v);
    //// Exceute batch action
    //static void cb_exec_batch(Fl_Widget* w, void* v);
    //// Execute call action
    //static void cb_exec_call(Fl_Widget* w, void* v);
    //// Call input 
    //static void cb_input_call(Fl_Widget* w, void* v);
    // // Reset button
    //static void cb_reset(Fl_Widget* w, void* v);
    //// Enter in any input value widget
    //static void cb_ip_enter(Fl_Widget* w, void* v);
 
    // Specify the widgets
    void create_form();
    // Configure the widgets
    void enable_widgets();

    //// Configure widgets
    //void update_new_batch();
    //void update_sort_cards();
    //void update_rcv_card();
    //void update_rcv_sase();
    //void update_stuff_cards();
    //void update_keep_cards();
    //void update_dispose_cards();
    //void update_post_cards();
    //void update_recycle_cards();
    //void update_dispose_sase();
    //void update_batches(bool enable_nav, bool add = false);
    //void update_calls(int box_num);
    //void update_batch_summary();
    //void update_batch_listing();
    //void update_call_summary();
    //void update_call_history();
    //void update_edit_notes();
    //void update_correct_data();
    //void update_history(bool enable);
    //void hide_edit_notes(bool info);
    //void update_action_values();
    //void update_action_bn(Fl_Button* b, action_t a);
    //// Call whatever update is needed per action
    //void update_whatever();
    //// Populate batch choice
    //void populate_batch(bool enable_change, bool add = false);

    void show_process();

public:
    // Public variables

    // Various data items accessible by most classes
    //Fl_Preferences settings_;
    QBS_data* data_;
    // Batch log
    ofstream* blog_file_;
    // CSV file directory
    string csv_directory_;
    // QBS filename
    string qbs_filename_;
    // Selected box number
    int selected_box_;
    // Current callsign
    string call_;   

protected:
    //// Log display scroll position
    //int pos_batch_log_;
    //// Pre-populate inputs with defaults
    //bool default_inputs_;

    //// widgets - input group
    //Fl_Group* g_input_;
    //Fl_Button* bn_import_;
    //Fl_Button* bn_read_qbs_;
    //Fl_Input* ip_csv_dir_;
    //Fl_Input* ip_qbs_file_;
    //Fl_Button* bn_brf_qbs_;
    //browser_data_t qbs_browser_;
    //Fl_Button* bn_brf_csv_;
    //browser_data_t csv_browser_;

    //// widgets - process group
    //Fl_Group* g_process_;
    //Fl_Group* g_proc_bns_;

    //Fl_Radio_Light_Button* bn_rcv_card_;
    //radio_param_t rp_rcv_card_;
    //Fl_Radio_Light_Button* bn_rcv_sase_;
    //radio_param_t rp_rcv_sase_;
 
    //Fl_Radio_Light_Button* bn_new_batch_;
    //radio_param_t rp_new_batch_;
    //Fl_Radio_Light_Button* bn_sort_cards_;
    //radio_param_t rp_sort_cards_;
    //Fl_Radio_Light_Button* bn_stuff_cards_;
    //radio_param_t rp_stuff_cards_;
    //Fl_Radio_Light_Button* bn_keep_cards_;
    //radio_param_t rp_keep_cards_;
    //Fl_Radio_Light_Button* bn_dispose_cards_;
    //radio_param_t rp_dispose_cards_;
    //Fl_Radio_Light_Button* bn_post_cards_;
    //radio_param_t rp_post_cards_;
    //Fl_Radio_Light_Button* bn_recycle_cards_;
    //radio_param_t rp_recycle_cards_;
    //Fl_Radio_Light_Button* bn_dispose_sase_;
    //radio_param_t rp_dispose_sase_;
    //Fl_Radio_Light_Button* bn_correction_;
    //radio_param_t rp_correction_;

    //Fl_Radio_Light_Button* bn_edit_notes_;
    //radio_param_t rp_edit_notes_;

    //Fl_Radio_Light_Button* bn_summ_batch_;
    //radio_param_t rp_summ_batch_;
    //Fl_Radio_Light_Button* bn_list_batch_;
    //radio_param_t rp_list_batch_;
    //Fl_Radio_Light_Button* bn_summ_call_;
    //radio_param_t rp_summ_call_;
    //Fl_Radio_Light_Button* bn_hist_call_;
    //radio_param_t rp_hist_call_;

    //Fl_Choice* ch_batch_;
    //Fl_Button* bn_b_action_;
    //Fl_Input* ip_call_;
    //Fl_Button* bn_c_action_;
    //Fl_Button* bn_use_defaults_;
    //Fl_Box* bx_current_;
    //Fl_Box* bx_change_;
    //static const int NUM_COUNTS = 10;
    //Fl_Output* op_value_[NUM_COUNTS];
    //Fl_Input* ip_delta_[NUM_COUNTS];
    //Fl_Box* bx_label_[NUM_COUNTS];
    //int index_inbox_;
    //int index_sase_;
    //int index_curr_;
    //int index_head_;
    //int index_keep_;
    //int index_outbox_;
    //int index_weight_;
    //Fl_Button* bn_reset_;
    //Fl_Button* bn_close_;


    //QBS_notes* tab_old_notes_;
    //Fl_Output* op_note_date_;
    //Fl_Input* ip_note_name_;
    //Fl_Input* ip_note_value_;

    //Fl_Text_Display* td_log_;

    //QBS_charth* g_charts_;

    Fl_Wizard* wiz_;
    QBS_file* g_file_;
    QBS_dormant* g_dormant_;
    QBS_call* g_call_;
    QBS_batch* g_batch_;

    // The groups within the wizard that perform specific acts (hence spells).
    map<process_mode_t, Fl_Group*> spells_;

    // The stack of operations
    stack<process_mode_t, vector<process_mode_t> > stack_;
 
    //// Command selection
    //action_t action_;

    //// Batch operation finished - disables batch execute button
    //bool batch_op_done_;

    // Reading - ignore update_action
    bool reading_;
};


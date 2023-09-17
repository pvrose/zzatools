#pragma once

#include <cinttypes>
#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Output.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Radio_Button.H>

using namespace std;

class sark_control : public Fl_Group {

public:
    sark_control(int X, int Y, int W, int H, const char* L = nullptr);
    ~sark_control();
 
    void update_sark_graph();

protected:

    // Callbacks
    static void cb_bn_connect(Fl_Widget* w, void* v);
    static void cb_ch_device(Fl_Widget* w, void* v);
    static void cb_ch_preset(Fl_Widget* w, void* v);
    static void cb_vl(Fl_Widget* w, void* v);
    static void cb_ch_step(Fl_Widget* w, void* v);
    static void cb_bn_scan(Fl_Widget* w, void* v);
    static void cb_ch_max_swr(Fl_Widget* w, void* v);
    static void cb_ch_max_ohm(Fl_Widget* w, void* v);
    static void cb_bn_colour(Fl_Widget* w, void* v);
    static void cb_bn_scantype(Fl_Widget* w, void* v);

    // Timer
    static void cb_ticker(void* v);

    // Create
    void create();

    // Widget updates
    void update_device();
    void update_scan_params();
    void update_scan_progress();
    void scan_sark();
    void clear_sark_graph();
    void update_scan_type();
    
    // Load/Save settings
    void load_settings();
    void save_settings();

    // Choice population
    void populate_device();
    void populate_preset();
    void populate_step();
    void populate_max_swr();
    void populate_max_ohm();

    // Attributes
    Fl_Preferences* settings_;
    // Device is connected
    bool connected_;
    // Device [port name]
    string device_;
    // Default band
    string band_;
    // Frequency of start of scan (in Hz)
    int64_t start_;
    // Frequency of end of scan (in Hz)
    int64_t end_;
    // Frequency step (in Hz)
    int64_t step_;
    // Scan data is valid
    bool data_valid_;
    // Scan progress value
    float scan_progress_;
    // Maximu SWR displayed
    double max_swr_level_;
    // Maximum ohmage displayed
    double max_ohm_level_;
    // sark_graph colours
    Fl_Color swr_colour_;
    Fl_Color x_colour_;
    Fl_Color r_colour_;
    Fl_Color z_colour_;
    // Scan tyype
    enum scan_type { RAW_ABS, RAW_SIGN, COMPUTED, RAW_ABS_I, RAW_SIGN_I, COMPUTED_I } scan_type_;


    // Widgets
    Fl_Light_Button* bn_connect_;
    Fl_Choice* ch_device_;
    Fl_Choice* ch_preset_;
    Fl_Spinner* vl_start_;
    Fl_Spinner* vl_end_;
    Fl_Choice* ch_step_;
    Fl_Output* op_samples_;
    Fl_Light_Button* bn_scan_;
    Fl_Progress* pr_scan_;
    Fl_Choice* ch_max_swr_;
    Fl_Choice* ch_max_ohm_;
    Fl_Button* bn_colour_swr_;
    Fl_Button* bn_colour_x_;
    Fl_Button* bn_colour_r_;
    Fl_Button* bn_colour_z_;
    Fl_Group* gp_scan_type_;
    Fl_Radio_Button* bn_raw_abs_;
    Fl_Radio_Button* bn_raw_sign_;
    Fl_Radio_Button* bn_computed_;
    Fl_Radio_Button* bn_raw_absi_;
    Fl_Radio_Button* bn_raw_signi_;
    Fl_Radio_Button* bn_computedi_;

};
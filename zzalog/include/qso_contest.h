#pragma once

#include "fields.h"
#include "field_choice.h"
#include "calendar.h"

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Radio_Light_Button.H>

using namespace std;

class qso_data;

const field_list CONTEST_FIELDS = {
    "QSO_DATE",
    "TIME_ON",
    "TIME_OFF",
    "FREQ",
    "MODE",
    "TX_PWR",
    "CALL",
    "RST_SENT",
    "STX",
    "RST_RCVD",
    "SRX"
};

// This class provides the interface to configure for a contest
class qso_contest :
    public Fl_Group
{
public:
    qso_contest(int X, int Y, int W, int H, const char* L);
    ~qso_contest();

    // Build the widget
    void create_form();
    // Configure all the component widgets
    void enable_widgets();

    // Get the contest ID
    string contest_id();
    // Get the logging field colelcttion
    string collection();
    // Get the current serial number
    string serial();
    // Increment the serial number
    void increment_serial();
    // Return the fixed exchange
    string exchange();
    // Contest active
    bool contest_active();

protected:
    // Contest ID
    static void cb_contest_id(Fl_Widget* w, void* v);
    // Log data
    static void cb_collection(Fl_Widget* w, void* v);
    // Decrement serial
    static void cb_dec_ser(Fl_Widget* w, void* v);
    // Increment serial
    static void cb_inc_ser(Fl_Widget* w, void* v);
    // Reset serial
    static void cb_rst_ser(Fl_Widget* w, void* v);
    // Active button
    static void cb_active(Fl_Widget* w, void* v);
    // Finish button
    static void cb_finish(Fl_Widget* w, void* v);
    // Save button
    static void cb_save(Fl_Widget* w, void* v);
    // Write data button
    static void cb_write(Fl_Widget* w, void* v);
    // Write radio buttons
    static void cb_wradio(Fl_Widget* w, void* v);

    // Load settings for the contest ID
    void load_settings();
    // SAve settings for the conest ID
    void save_settings();
    // Populate collection
    void populate_collection();
    // Check collection exists and if necessary open fields dialog
    void check_collection();
    // Write ADIF
    void write_adif();
    // Write cabrillo
    void write_cabrillo();

    // Contest ID
    string contest_id_;
    // Collection name
    string collection_;
    // Notes
    string notes_;
    // My exchange
    string exchange_;
    // Start date
    string start_date_;
    string start_time_;
    // End date
    string end_date_;
    string end_time_;
    // Serial number
    int serial_;
    // Active
    bool active_;

    enum write_t : uchar  {
        ADIF,
        CABRILLO
    } write_format_;

    // Extract collection
    string saved_collection_;

    // Callback data for the calendars
    cal_cb_data_t start_data_;
    cal_cb_data_t end_data_;

    // Contest ID input
    field_input* ip_contest_id_;
    // Collection input
    Fl_Input_Choice* ip_collection_;
    // Notes
    Fl_Input* ip_notes_;
    // Exchange
    Fl_Input* ip_exchange_;
    // Start date ip
    Fl_Input* ip_start_date_;
    Fl_Button* bn_start_date_;
    Fl_Input* ip_start_time_;
    // End date ip
    Fl_Input* ip_end_date_;
    Fl_Button* bn_end_date_;
    Fl_Input* ip_end_time_;
    // Serial number
    Fl_Output* op_serial_;
    Fl_Button* bn_dec_ser_;
    Fl_Button* bn_inc_ser_;
    Fl_Button* bn_rst_ser_;
    // Active
    Fl_Light_Button* bn_active_;
    // Finish
    Fl_Button* bn_finish_;
    // SAve
    Fl_Button* bn_save_;
    // Generate logs
    Fl_Button* bn_write_;
    Fl_Radio_Light_Button* bn_adif_;
    Fl_Radio_Light_Button* bn_cabrillo_;





};


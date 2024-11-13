#pragma once

#include "QBS_consts.h"
#include "utils.h"
#include "callback.h"

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
  
    // Specify the widgets
    void create_form();
    // Configure the widgets
    void enable_widgets();

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

    Fl_Wizard* wiz_;
    QBS_file* g_file_;
    QBS_dormant* g_dormant_;
    QBS_call* g_call_;
    QBS_batch* g_batch_;

    // The groups within the wizard that perform specific acts (hence spells).
    map<process_mode_t, Fl_Group*> spells_;

    // The stack of operations
    stack<process_mode_t, vector<process_mode_t> > stack_;
 
    // Reading - ignore update_action
    bool reading_;
    // Last screen was DORMANT
    bool last_dormant_;
};


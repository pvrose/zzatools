#pragma once
/*****************************************************************
*   QBS_start
* Provides the start-up screen.
*/
#include "QBS_window.h"

#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>

using namespace std;

class QBS_start :
    public Fl_Group
{
public:
    QBS_start(int X, int Y, int W, int H, const char* L);
    virtual ~QBS_start();
    // Put filename
    void filename(string value);

protected:
    // Callbacks
    ////////////
    // Import Spreadshhet
    static void cb_bn_import(Fl_Widget* w, void* v);
    // Load File (XML)
    static void cb_bn_loadxml(Fl_Widget* w, void* v);
    // Save File (XML)
    static void cb_bn_savexml(Fl_Widget* w, void* v);
    // Process cards
    static void cb_bn_process(Fl_Widget* w, void* v);
    // View/edit databse
    static void cb_bn_review(Fl_Widget* w, void* v);

    void load_values();
    void create_form();
    void configure_widgets();
    void save_values();

    string filename_;
    Fl_Output* op_filename_;
    QBS_window* window_;

public:
};


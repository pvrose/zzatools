#pragma once

#include "fields.h"
#include "page_dialog.h"
#include "field_choice.h"
#include "intl_widgets.h"

#include <vector>
#include <string>
#include <map>

#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>

using namespace std;


class fields_table:
public Fl_Table_Row {

public:

    fields_table(int X, int Y, int W, int H, const char* L = nullptr);
    ~fields_table();

    // Inheritedfrom Fl_Table_Row
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);
    // Set the contents - by reference
    void data(collection_t* d);
    // Selected row
    int selected_row();
    // Set selection - single row
    void select_row(int row);

protected:
    // Callbacks - field choice
    static void cb_field(Fl_Widget* w, void* v);
    // Header input
    static void cb_header(Fl_Widget* w, void* v);
    // Width input
    static void cb_width(Fl_Widget* w, void* v);
    // Table callback
    static void cb_table(Fl_Widget* w, void* v);
    // The data
    collection_t* data_;
    // The cell being edited
    int edit_row_;
    int edit_col_;
    // The selected row
    int selected_row_;

    // Editing widgets
    field_choice* ch_field_;
    intl_input* ip_header_;
    Fl_Int_Input* ip_width_;
    
};

// This class displays a dialog that allows the user to select and order fields that are to be used
// in the various different views of the log
class fields_dialog :
    public page_dialog

{
public:

    // Constructor
    fields_dialog(int X, int Y, int W, int H, const char* L = nullptr);
    virtual ~fields_dialog();

    // Load values from settings_
    virtual void load_values();
    // Used to create the form
    virtual void create_form(int X, int Y);
    // Used to write settings back
    virtual void save_values();
    // Used to enable/disable specific widget - any widgets enabled musr be attributes
    virtual void enable_widgets();

    // callbacks:-

    // Application choice
    static void cb_application(Fl_Widget* w, void* v);
    // Field collection choice
    static void cb_collection(Fl_Widget* w, void* v);
    // Move selected row up or down
    static void cb_move(Fl_Widget* w, void* v);
    // Delete collection
    static void cb_del_coll(Fl_Widget* w, void* v);

protected:

    // Populate the application choice
    void populate_app(Fl_Choice* w);
    // Populate the Collection choice
    void populate_coll(Fl_Input_Choice* w);

    void navigate_table(bool up);

    // Current application
    field_app_t application_;
    // Current collection
    string collection_;
    // Application and collection are linked
    bool linked_;
    // The application choice widget
    Fl_Choice* ch_app_;
    // The collection choice widget
    Fl_Input_Choice* ch_coll_;
    // Table
    fields_table* table_;
    // Up button
    Fl_Button* bn_up_;
    // Down button
    Fl_Button* bn_down_;

};
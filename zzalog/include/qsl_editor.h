#pragma once

#include "page_dialog.h"
#include "qsl_display.h"
#include "utils.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Scroll.H>



// This class provides the dialog to allow the design of a QSL card 
// and the parameters used for printing

class qsl_editor : public page_dialog
{

    public:

    qsl_editor(int x, int Y, int W, int H, const char* L = nullptr);
    ~qsl_editor();
 
    protected:
    void load_values();
    void save_values();
    void create_form(int X, int Y);
    void enable_widgets();

    virtual int handle(int event);

    // Create item groups
    void create_items();
    void create_labels();
    void create_fparams(int& x, int& y, qsl_display::field_def* params);
    void create_tparams(int& x, int& y, qsl_display::text_def* params);
    void create_iparams(int& x, int& y, qsl_display::image_def* params);
    // Resize the group after adding or deleting an item
    void resize();
    // Redraw dislay
    void redraw_display();
    // Create display
    void create_display();
    // Update size
    void update_size();

    // Callbacks
    // Callsign
    static void cb_callsign(Fl_Widget* w, void* v);
    // Filename
    static void cb_filename(Fl_Widget* w, void* v);
    // Dimension radio 
    static void cb_radio_dim(Fl_Widget* w, void* v);
    // Size parameter changed
    static void cb_size_double(Fl_Widget* w, void* v);
    // Design edited
    static void cb_design(Fl_Widget* w, void* v);
    // Item field name
    static void cb_ch_field(Fl_Widget* w, void* v);
    // Style edited
    static void cb_bn_style(Fl_Widget* w, void* v);
    // String value updated
    static void cb_ip_string(Fl_Widget* w, void* v);
    static void cb_ip_int(Fl_Widget* w, void* v);
    static void cb_ip_bool(Fl_Widget* w, void* v);
 
    static void cb_new_item(Fl_Widget* w, void* v);
    // Change an item type
    static void cb_ch_type(Fl_Widget* w, void* v);
    // Image
    static void cb_image(Fl_Widget* w, void* v);
    // Date and time format
    template<class ENUM> 
    static void cb_datetime(Fl_Widget* w, void* v);
    // Example QSO
    static void cb_example(Fl_Widget* w, void* v);

    void populate_type(Fl_Choice* ch);
    void populate_date(Fl_Choice* ch);
    void populate_time(Fl_Choice* ch);

    // Number of rows in print
    int num_rows_;
    // Number of columns to print
    int num_cols_;
    // dimension usnits
    qsl_display::dim_unit unit_;
    // Label width
    double width_;
    // Label height
    double height_;
    // Column width
    double col_width_;
    // Row height
    double row_height_;
    // First column position
    double col_left_;
    // First row position
    double row_top_;
    // Number of QSOs per card
    int number_qsos_;
    // Callsign to read parameters
    string callsign_;
    // String filename
    string filename_;
    // Data for file browser
    browser_data_t filedata_;
    // display window coordinates
    int win_x_;
    int win_y_;
    // Date/Time formats
    qsl_display::date_format date_format_;
    qsl_display::time_format time_format_;
    // Use current QSO
    bool show_example_;
    // Current QSO to use
    record* example_qso_;

    //Widgets to reference
    Fl_Group* g_1_;
    Fl_Group* g_2_;
    // Contains the item editing buttons
    Fl_Group* g_4_;
    // Card display
    qsl_display* display_;
    Fl_Window* w_display_;
    Fl_Output* op_size_;
};


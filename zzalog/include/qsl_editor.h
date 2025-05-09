#pragma once

#include "page_dialog.h"
#include "qsl_data.h"
#include "utils.h"
#include "callback.h"

class qsl_display;
class qsl_widget;
class record;
class filename_input;
class field_input;
class Fl_Choice;
class Fl_Widget;
class Fl_Group;
class Fl_Window;
class Fl_Output;
class Fl_Value_Input;
class Fl_Check_Button;

// This class provides the dialog to edit the card label parameters
// and define the items used in drawing the card label

class qsl_editor : public page_dialog
{

public:

    qsl_editor(int x, int Y, int W, int H, const char* L = nullptr);
    ~qsl_editor();
 
protected:
    // Load the settings
    void load_values();
    // Save settings
    void save_values();
    // Create the dialog
    void create_form(int X, int Y);
    // Update widget values etc
    void enable_widgets();

    // Handle the ACTIVATE/DEACTIVATE events to show/hide the display window
    virtual int handle(int event);

    // Update - change callsign
    virtual void update();

    // Create all the item definition widgets
    void create_items();
    // Boxes to act as column headers
    void create_labels(int curr_y);
    // Create the widgets to edit one set of field parameters
    void create_fparams(int& x, int& y, qsl_data::field_def* params);
    // Crreate the widgets to edit one set of text parameters
    void create_tparams(int& x, int& y, qsl_data::text_def* params);
    // Create the widgets to edit one set of image parameters
    void create_iparams(int& x, int& y, qsl_data::image_def* params);
    // Resize the group after adding or deleting an item
    void resize();
    // Redraw dislay
    void redraw_display(bool dirty);
    // Create display
    void create_display();
    // Update size
    void update_size();
    // Update vale user data
    void update_dimensions();

    // Callbacks
    // Callsign
    static void cb_callsign(Fl_Widget* w, void* v);
    // Filename
    static void cb_filename(Fl_Widget* w, void* v);
    // Load TSV
    static void cb_bn_loadtsv(Fl_Widget* w, void* v);
    // Dimension radio 
    static void cb_radio_dim(Fl_Widget* w, void* v);
    // Size parameter changed
    static void cb_size_double(Fl_Widget* w, void* v);
    // Item field name
    static void cb_ch_field(Fl_Widget* w, void* v);
    // Style edited
    static void cb_bn_style(Fl_Widget* w, void* v);
    // String value updated
    static void cb_ip_string(Fl_Widget* w, void* v);
    // Integer value updated
    static void cb_ip_int(Fl_Widget* w, void* v);
    // Boolean value updated
    static void cb_ip_bool(Fl_Widget* w, void* v);
    // New iteh selected
    static void cb_new_item(Fl_Widget* w, void* v);
    // Change an item type
    static void cb_ch_type(Fl_Widget* w, void* v);
    // Image
    static void cb_image(Fl_Widget* w, void* v);
    // Date and time format choice widget
    template<class ENUM> 
    static void cb_datetime(Fl_Widget* w, void* v);
    // Use Example QSO
    static void cb_example(Fl_Widget* w, void* v);
    // Generate QSL type
    static void cb_qsl_type(Fl_Widget* w, void* v);
    // Reset image to unscaled
    static void cb_descale(Fl_Widget* w, void* v);
    
    // Populate the item type choice widget
    void populate_type(Fl_Choice* ch);
    // Populate the date format choice widget
    void populate_date(Fl_Choice* ch);
    // Populate the time format choice widget
    void populate_time(Fl_Choice* ch);
    // Populate the QSL type choice
    void populate_qsl_type(Fl_Choice* ch);

     // Callsign to read parameters
    string callsign_;
    // Data for file browser
    browser_data_t filedata_;
    // display window coordinates
    int win_x_;
    int win_y_;
    // Use current QSO
    bool show_example_;
    // Current QSO to use
    record* example_qso_;
    // Card display
    qsl_display* qsl_;
    // Directory of filename
    string dir_name_;
    // Pointer to card data item
    qsl_data* data_;
    // QSL type
    qsl_data::qsl_type qsl_type_;
    // Load data from TSV file
    bool load_tsv_;


    //Widgets to reference:-

    // Filename input and browse button
    Fl_Group* g_1_;
    field_input* ip_callsign_;
    filename_input* ip_filename_;
    Fl_Check_Button* bn_loadtsv_;

    // Inputs to define size and formats
    Fl_Group* g_2_;
    Fl_Value_Input* ip_cols_;
    Fl_Value_Input* ip_width_;
    Fl_Value_Input* ip_cpos_;
    Fl_Value_Input* ip_cspace_;
    Fl_Value_Input* ip_rows_;
    Fl_Value_Input* ip_height_;
    Fl_Value_Input* ip_rpos_;
    Fl_Value_Input* ip_rspace_;
    Fl_Value_Input* ip_qsos_;
    Fl_Choice* ch_data_;
    Fl_Choice* ch_time_;
    // Contains the item editing buttons
    Fl_Group* g_4_;
    // Window in which to show the display
    Fl_Window* w_display_;
    // Display widget
    qsl_widget* display_;
    // Output contains calculated size of display (in pixels)
    Fl_Output* op_size_;
};


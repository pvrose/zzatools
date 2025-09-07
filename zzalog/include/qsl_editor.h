#pragma once

#include "page_dialog.h"
#include "qsl_data.h"

#include "callback.h"
#include "utils.h"

class field_input;
class filename_input;
class qsl_display;
class qsl_widget;
class record;

class Fl_Check_Button;
class Fl_Choice;
class Fl_Group;
class Fl_Output;
class Fl_Radio_Round_Button;
class Fl_Value_Input;
class Fl_Widget;
class Fl_Window;

//! This class provides the dialog to edit the card label parameters
//! and define the items used in drawing the card label
class qsl_editor : public page_dialog
{

public:

    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qsl_editor(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qsl_editor();

protected:
    //! Load the QSL configration data.
    void load_values();
    //! Save the qSL configuration data.
    void save_values();
    //! Instantiate the component widgets.
    void create_form(int X, int Y);
    //! Configure and update component widgets when data changes.
    void enable_widgets();

    //! Override page_dialog::handle().
    
    //! Handle the ACTIVATE/DEACTIVATE events to show/hide the display window.
    //! Handle foxus events to allow keyboard F1 to open userguide.
    virtual int handle(int event);

    //! Update - change callsign
    virtual void update();

    //! Create all the item definition widgets
    void create_items();
    //! Create column header at y position \p curr_y.
    void create_labels(int curr_y);
    //! Create the widgets to edit one set of field parameters \p params at (\p x, \p y).
    void create_fparams(int& x, int& y, qsl_data::field_def* params);
    //! Create the widgets to edit one set of text parameters \p params at (\p x, \p y).
    void create_tparams(int& x, int& y, qsl_data::text_def* params);
    //! Create the widgets to edit one set of image parameters \p params at (\p x, \p y).
    void create_iparams(int& x, int& y, qsl_data::image_def* params);
    //! Resize the group after adding or deleting an item
    void resize();
    //! Redraw display: \p dirty indicates that a display needs resizing.
    void redraw_display(bool dirty);
    //! Create display
    void create_display();
    //! Update size
    void update_size();
    //! Update vale user data
    void update_dimensions();

    // Callbacks
    //! Callback indicating station callsign has changed: reload all data.
    static void cb_callsign(Fl_Widget* w, void* v);
    //! Callback indicating filename has changed: relead data from filename. 
    static void cb_filename(Fl_Widget* w, void* v);
    //! Callback indicating "Load TSV" clicked: reload from TSV file.
    
    //! \todo is this still needed now that ZZALOG has ligrated to XML format.
    static void cb_bn_loadtsv(Fl_Widget* w, void* v);
    //! Callback on one of the dimension radio button: \p v as dim_unit indicates which button. 
    static void cb_radio_dim(Fl_Widget* w, void* v);
    //! Callback when a size parameter has changed.
    static void cb_size_double(Fl_Widget* w, void* v);
    //! Callback when a field selected from the choice drop-down menu.
    static void cb_ch_field(Fl_Widget* w, void* v);
    //! Callback to edit a style.
    static void cb_bn_style(Fl_Widget* w, void* v);
    //! Callback when a string value has been updated.
    static void cb_ip_string(Fl_Widget* w, void* v);
    //! Callback when an integer value has been updated.
    static void cb_ip_int(Fl_Widget* w, void* v);
    //! Callback when a Boolean value has been updated.
    static void cb_ip_bool(Fl_Widget* w, void* v);
    //! Callback when the "New" button is clicked to add a drawing item.
    static void cb_new_item(Fl_Widget* w, void* v);
    //! Callback when the item choice menu is selected.
    static void cb_ch_type(Fl_Widget* w, void* v);
    //! Callback when the fillename for an image has changed.
    static void cb_image(Fl_Widget* w, void* v);
    //! Callback when either the date or time choice has been selected.
    template<class ENUM> 
    static void cb_datetime(Fl_Widget* w, void* v);
    //! Callback when the "Show Example QSO" button has been clicked.
    static void cb_example(Fl_Widget* w, void* v);
    //! Callback when the QSL type choice has been selected.
    static void cb_qsl_type(Fl_Widget* w, void* v);
    //! Callback when the "Actual Size" button is clicked: restores the displayed image to being unscaled.
    static void cb_descale(Fl_Widget* w, void* v);
    
    //! Populate the item type choice widget
    void populate_type(Fl_Choice* ch);
    //! Populate the date format choice widget
    void populate_date(Fl_Choice* ch);
    //! Populate the time format choice widget
    void populate_time(Fl_Choice* ch);
    //! Populate the QSL type choice
    void populate_qsl_type(Fl_Choice* ch);

    //! Remove filepath from filename - returns false if unsuccesful
    bool relative_filename(string& filename);

    //! Station callsign to read parameters.
    string callsign_;
    //! Data for file browser
    browser_data_t filedata_;
    // display window coordinates
    int win_x_;     //!< X coordinate of display window.
    int win_y_;     //!< Y coordinate of display window.
    //! Use current QSO as an example image.
    bool show_example_;
    //! Current QSO to use
    record* example_qso_;
    //! Card display
    qsl_display* qsl_;
    //! Directory of filename
    string dir_name_;
    //! Pointer to card data item
    qsl_data* data_;
    //! QSL type
    qsl_data::qsl_type qsl_type_;
    //! Load data from TSV file
    bool load_tsv_;


    //Widgets to reference:-

    // Filename input and browse button
    Fl_Group* g_1_;
    field_input* ip_callsign_;            //!< Station callsign.
    filename_input* ip_filename_;         //!< Input filename.
    Fl_Check_Button* bn_loadtsv_;         //!< "Load TSV" button.

    // Inputs to define size and formats
    Fl_Group* g_dim_;        
    Fl_Radio_Round_Button* bn_inch_;      //!< Radio button: Dimensions in inches.
    Fl_Radio_Round_Button* bn_mm_;        //!< Radio button: Dimensions in millimetres.
    Fl_Radio_Round_Button* bn_point_;     //!< Radio button: Dimensions in (print) points.
    Fl_Group* g_2_;
    Fl_Value_Input* ip_cols_;             //!< Input: Number of columns in  label sheet.
    Fl_Value_Input* ip_width_;            //!< Input: Width of label.
    Fl_Value_Input* ip_cpos_;             //!< Input: Position of first columns.
    Fl_Value_Input* ip_cspace_;           //!< Input: Spacing between columns.
    Fl_Value_Input* ip_rows_;             //!< Input: Number of rows in label sheet.
    Fl_Value_Input* ip_height_;           //!< Input: Height of label.
    Fl_Value_Input* ip_rpos_;             //!< Input: Position of first row.
    Fl_Value_Input* ip_rspace_;           //!< Input: Spacing between rowss.
    Fl_Value_Input* ip_qsos_;             //!< Input: Number of QSOs to print per label.
    Fl_Choice* ch_data_;                  //!< Choice: Date format.
    Fl_Choice* ch_time_;                  //!< Choice: Time format.
    // Contains the item editing buttons
    Fl_Group* g_4_;
    // Window in which to show the display
    Fl_Window* w_display_;                //!< Window to show full-size image.
    // Display widget 
    qsl_widget* display_;                 //!< Image display within w_display_.
    // Output contains calculated size of display (in pixels)
    Fl_Output* op_size_;                  //!< Output: size of display in pixels.
};


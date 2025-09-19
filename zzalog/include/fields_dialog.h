#pragma once

#include "fields.h"
#include "page_dialog.h"

#include <vector>
#include <string>
#include <map>

#include <FL/Fl_Table_Row.H>



class field_choice;
class intl_input;
class Fl_Light_Button;
class Fl_Input_Choice;
class Fl_Int_Input;
class Fl_Choice;
class Fl_Button;

//! This class displays a tabular version for editing field collections.
class fields_table:
public Fl_Table_Row {

public:

    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    fields_table(int X, int Y, int W, int H, const char* L = nullptr);
    //! Desctructor.
    ~fields_table();

    //! Inherited from Fl_Table_Row. Required to populate the table cells.
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);
    //! Set the contents - by reference
    void data(collection_t* d);
    //! Returns the selected row.
    int selected_row();
    //! Set the selected row.
    void select_row(int row);

protected:
    //! Callback from field choice, that either adds, deletes or renames a field.
    static void cb_field(Fl_Widget* w, void* v);
    //! Callback from header text input that changes the header text for the selected field.
    static void cb_header(Fl_Widget* w, void* v);
    //! Callback from the width input that changes the width value for the selected field.
    static void cb_width(Fl_Widget* w, void* v);
    //! Callback from table that opens an edit widget for the data item clicked.
    static void cb_table(Fl_Widget* w, void* v);
    //! The data being displayed.
    collection_t* data_;
    //! The row containing the cell being edited.
    int edit_row_;
    //! The column containing the cell being edited.
    int edit_col_;
    //! The selected row
    int selected_row_;

    // Editing widgets
    field_choice* ch_field_;    //!< Choice to select a field.
    intl_input* ip_header_;     //!< Input to provide text for the header in log_table views
    Fl_Int_Input* ip_width_;    //!< Input to provide the width of the field in log_table views.
    
    
};

//! \brief This class displays a dialog that allows the user to select and order fields that are to be used
//! in the various different views of the log
class fields_dialog :
    public page_dialog

{
public:

    //! Constructor

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    fields_dialog(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    virtual ~fields_dialog();

    //! Override of page_dialog::handle() takes focus to enable keyboard F1 to open userguide.
    virtual int handle(int event);

    //! Null method.
    virtual void load_values();
    //! Instantiates component widgets at (\p X,\p Y) in the window.
    virtual void create_form(int X, int Y);
    //! Updates views to indicate that the format may have changed.
    virtual void save_values();
    //! Configure component widgets after data has changed.
    virtual void enable_widgets();

    // callbacks:-

    //! Callback when usage being edited has changed: updates dialog.
    static void cb_application(Fl_Widget* w, void* v);
    //! Callback when collection name has changed: updates dialog.
    static void cb_collection(Fl_Widget* w, void* v);
    //! Callback o n up or down arrows: moves the selected row up or down.
    static void cb_move(Fl_Widget* w, void* v);
    //! Callback on "Delete" button: deletes collection
    static void cb_del_coll(Fl_Widget* w, void* v);
    //! Callback on "Linked" button: associates collection to usage.
    static void cb_linked(Fl_Widget* w, void* v);

protected:

    //! Populate the usage choice.
    void populate_app(Fl_Choice* w);
    //! Populate the Collection choice
    void populate_coll(Fl_Input_Choice* w);
    //! Implementation of the Up/Down actions.
    void navigate_table(bool up);

    //! Current usage.
    field_app_t application_;
    //! Current collection name.
    std::string collection_;
    //! Usage and collection are linked
    bool linked_;
    //! The usage choice widget
    Fl_Choice* ch_app_;
    //! The collection choice widget
    Fl_Input_Choice* ch_coll_;
    //! Table
    fields_table* table_;
    //! Up button
    Fl_Button* bn_up_;
    //! Down button
    Fl_Button* bn_down_;
    //! Linked button
    Fl_Light_Button* bn_linked_;

};
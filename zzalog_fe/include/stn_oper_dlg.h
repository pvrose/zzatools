#pragma once
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

#include <map>
#include <string>


enum oper_value_t : char;
class Fl_Button;
class Fl_Input;
class Fl_Input_Choice;
class Fl_Output;
struct oper_info_t;

//! This class represents a single line in the stn_qth_dlg editing pane.
class stn_oper_widget :
    public Fl_Group
{
public:
    //! Constructor 

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    stn_oper_widget(int X, int Y, int W, int H, const char* L = nullptr);

    //! Destructor
    ~stn_oper_widget();

    //! Set the values into the inputs
    void enable_widgets();

    //! Callback - entering value into an input

    //! \param w widget that raised the callback
    //! \param v oper_value_t 
    static void cb_ip_data(Fl_Widget* w, void* v);

protected:
    //! Instantiate the component widgets
    void create_form();

    // Widgets
    std::map<oper_value_t, Fl_Input*> ip_data_;
    //!< Data items
};



//! Class  provides a container to hold several stn_oper_widget items
class stn_oper_cntnr :
    public Fl_Scroll
{
public:
    //! Constructor 

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    stn_oper_cntnr(int X, int Y, int W, int H, const char* L = "");

    //! Destructor
    ~stn_oper_cntnr();

    //! Get selected widget
    stn_oper_widget* value();

    //! Get selected ID
    std::string get_selected();

    //! Set selected widget
    void set_selected(std::string id);

    //! Redraw widgets
    void redraw_widgets();

protected:

    //! Contained widgets
    std::map<std::string, stn_oper_widget*> widgets_;

    //! Selected widget
    std::string selected_;

};


//! \brief This class provides the oper pane of the stn_dialog
class stn_oper_dlg :
    public Fl_Group
{
public:
    //! Constructor 

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    stn_oper_dlg(int X, int Y, int W, int H, const char* L);

    //! Desctructor
    ~stn_oper_dlg();

    //! Enable widgets
    void enable_widgets();

    //! Set selected
    void set_operator(std::string s);

    //! Return current operator
    std::string get_operator();

    //! Callback from "Add" button
    static void cb_add(Fl_Widget* w, void* v);

    //! Callback from "Delete" button
    static void cb_delete(Fl_Widget* w, void* v);

    //! Callback from "Clear" button to clear all values for specific identifier.
    static void cb_clear(Fl_Widget* w, void* v);

    //! callback from choice
    static void cb_choice(Fl_Widget* w, void* v);

    //! Callback from "Check log" button
    static void cb_check(Fl_Widget* w, void* v);

    //! Callback from "Set Default" button
    static void cb_default(Fl_Widget* w, void* v);

protected:
    //! Load data
    void load_data();

    //! Instantiate component widgets
    void create_form();

    //! Populate location choice
    void populate_operators();

    // Widgets
    Fl_Button* bn_clear_;            //!< Clear all fields
    Fl_Button* bn_add_;              //!< Add an entry to oper data
    Fl_Input_Choice* ip_new_;        //!< Select location name
    Fl_Button* bn_delete_;           //!< Selete selected entry
    Fl_Button* bn_default_;          //!< Set as default
    Fl_Button* bn_check_;            //!< Check log
    stn_oper_cntnr* table_;           //!< Table displaying oper details

    bool selected_new_;              //!< A new location has been enetered (for Add or REname)

    std::string new_operator_;       //!< New location entered
    std::string operator_;          //!< Current selected location
};

#pragma once

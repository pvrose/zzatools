#pragma once

#include "cty_data.h"

#include <map>

#include <FL\Fl_Double_Window.H>

class Fl_Output;

using namespace std;

class cty_dialog :
    public Fl_Double_Window
{
public:
    cty_dialog(int W, int H, const char* L = nullptr);
    ~cty_dialog();

    virtual int handle(int event);

    void create_form();

    void update_widgets();

    // Fetch new data
    static void cb_update(Fl_Widget* w, void* v);
    // Reload cty_data
    static void cb_reload(Fl_Widget* w, void* v);
    // Close the dialog
    static void cb_close(Fl_Widget* w, void* v);
    // Open directory browser
    static void cb_browser(Fl_Widget* w, void* v);

protected:

    // Updateable widgets
    map<cty_data::cty_type_t, Fl_Output*> w_timestamps_;
    map<cty_data::cty_type_t, Fl_Output*> w_versions_;
    
};


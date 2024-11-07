#pragma once
#include <FL/Fl_Group.H>

// Forward declarations
class QBS_window;
class Fl_Button;

class QBS_file :
    public Fl_Group
{
public:
    QBS_file(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_file();

    void create_form();
    void enable_widgets();

    // Main app
    QBS_window* win_;

protected:

    // Callbacks - import CVS files
    static void cb_import(Fl_Widget* w, void* v);
    // Callback - read QBS file
    static void cb_read(Fl_Widget* w, void* v);


    // Configurable buttons
    Fl_Button* bn_import_;
    Fl_Button* bn_read_;
 
};


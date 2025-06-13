#pragma once
#include <FL\Fl_Double_Window.H>

enum status_t : char;
enum object_t : char;

// FLTK classes
class Fl_Box;
class Fl_Button;
class Fl_Fill_Dial;
class Fl_Multiline_Output;
class Fl_Output;
class Fl_Text_Display;

class banner :
    public Fl_Double_Window
{
public:
    banner(int W, int H, const char* L = nullptr);
    ~banner();

    void create_form();
    void enable_widgets();

    // Add a message to the banner
    void add_message(status_t type, const char* msg, object_t object, const char* prg_unit = nullptr);
    // Add progress
    void add_progress(uint64_t value, uint64_t max_value = -1);
    // Update progress
    void ticker();

    // Callback - ticker
    static void cb_ticker(void* v);
    // Callback - drop down viewer
    static void cb_bn_pulldown(Fl_Widget* w, void* v);
    // Callback - close button - ignore
    static void cb_close(Fl_Widget* w, void* v);

protected:

    // Add message to display (with colour)
    void copy_msg_display(status_t type, const char* msg);

    // Widget
    Fl_Box* bx_icon_;
    Fl_Button* bn_pulldown_;
    Fl_Fill_Dial* fd_progress_;
    Fl_Multiline_Output* op_msg_low_;
    Fl_Multiline_Output* op_msg_high_;
    Fl_Output* op_prog_title_;
    Fl_Output* op_prog_value_;
    Fl_Text_Display* display_;

    // Overall heights - small banner
    int h_small_;
    // large banner (with display)
    int h_large_;
    // Large banner
    bool large_;
    // Progress maximum value
    uint64_t max_value_;
    // Progress unit
    const char* prg_unit_;

};


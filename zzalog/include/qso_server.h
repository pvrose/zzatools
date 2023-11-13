#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

class qso_server:
    public Fl_Group

{

public:
    qso_server(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_server();

    void load_values();
    void create_form();
    void save_values();
    void enable_widgets();

protected:
    enum server_t {
        WSJTX,
        FLDIGI
    };
    // Button callback
    static void cb_bn_change(Fl_Widget* w, void* v);

    // Widgets
    Fl_Check_Button* bn_wsjtx_on_;
    Fl_Check_Button* bn_fldigi_on_;
    Fl_Button* bn_wsjtx_;
    Fl_Button* bn_fldigi_;

};
#pragma once

#include "view.h"
#include "dxcc_table.h"
#include "fields.h"

#include <FL/Fl_Group.H>

class Fl_Check_Button;

class dxcc_view :
    public Fl_Group, public view
{
public:
    dxcc_view(int X, int Y, int W, int H, const char* L = nullptr, field_app_t f = FO_LAST);
    ~dxcc_view();

    void create_form();
    void enable_widgets();
    void load_data();
    void store_data();

    // something has changed in the book - usually record 1 is to be selected, record_2 usage per view
    virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0);

    // Display choice
    static void cb_display(Fl_Widget* w, void* v);
    // Confirmation choice
    static void cb_confirm(Fl_Widget* w, void* v);

protected:
    dxcc_table::display_t display_type_;
    dxcc_table::confirm_t confirm_type_;

    dxcc_table* table_;
    Fl_Group* rg_display_;
    Fl_Check_Button* bn_total_;
    Fl_Check_Button* bn_bands_;
    Fl_Check_Button* bn_modes_;
    Fl_Check_Button* bn_dmodes_;
    Fl_Group* bg_confirm_;
    Fl_Check_Button* bn_eqsl_;
    Fl_Check_Button* bn_lotw_;
    Fl_Check_Button* bn_card_;

};


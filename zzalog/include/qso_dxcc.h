#pragma once

#include "prefix.h"
#include "cty_data.h"

#include <set>
#include <vector>
#include <map>

#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>

class qso_dxcc :
    public Fl_Group
{

 
    class wb4_buttons : public Fl_Scroll
    {
    public:
        wb4_buttons(int X, int Y, int W, int H, const char* L = nullptr);
        ~wb4_buttons();

        static void cb_bn_mode(Fl_Widget* w, void* v);
        static void cb_bn_band(Fl_Widget* w, void* v);

        void create_form();
        void enable_widgets();

        // widget maps
        map<string, Fl_Widget*> map_;

 
    protected:
        set<string>* dxcc_bands_;
        set<string>* dxcc_modes_;
        set<string>* dxcc_submodes_;
        set<string>* all_bands_;
        set<string>* all_modes_;
        set<string>* all_submodes_;
    };

public:
    qso_dxcc(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_dxcc();

    void create_form();
    void enable_widgets();

    void set_data();

protected:

    // Button for QRZ.com
    static void cb_bn_qrz(Fl_Widget* w, void* v);
    

    // Widgets:
    // Tree
    //tree* tree_;
    // Callsign label
    Fl_Output* op_call_;
    // Prefix data
    Fl_Output* op_source_;
    Fl_Output* op_prefix_;
    Fl_Output* op_geo_;
    // 
    // Buttons showing worked before
    wb4_buttons* g_wb4_;
    // Call QRZ.com
    Fl_Button* bn_qrz_;

    // Callsign
    string callsign_;
    string nickname_;
    string name_;
    int cq_zone_;
    lat_long_t location_;
    cty_data::parse_source_t source_;

    // BAnd worked
    set<string>* bands_worked_;
    // Modes worked
    set<string>* modes_worked_;
    // Source of prefix parsing
    

};


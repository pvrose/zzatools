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

    //enum source_t {
    //    NONE,
    //    EXCEPTION,
    //    RECORD_DXCC,
    //    RECORD_PFX,
    //    PREFIX_LIST
    //};

    //class tree : public Fl_Tree
    //{
    //public:
    //    tree(int X, int Y, int W, int H, const char* L = nullptr);
    //    ~tree();

    //    void enable_widgets();

    //    void set_data(vector<prefix*>* items);

    //protected:

    //    // Hang an item on the tree
    //    Fl_Tree_Item* hang_item(prefix* pfx);

    //    vector<prefix*>* prefixes_;
    //};

    class wb4_buttons : public Fl_Scroll
    {
    public:
        wb4_buttons(int X, int Y, int W, int H, const char* L = nullptr);
        ~wb4_buttons();

        void create_form();
        void enable_widgets();

        // widget maps
        map<string, Fl_Widget*> map_;

        void set_data(set<string>* bands, set<string>* modes);

    protected:
        set<string>* bands_;
        set<string>* modes_;
    };

public:
    qso_dxcc(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_dxcc();

    void create_form();
    void enable_widgets();

    void set_data();

protected:

    //// Tree callback
    //static void cb_tree(Fl_Widget* w, void* v);
    // Button for QRZ.com
    static void cb_bn_qrz(Fl_Widget* w, void* v);
    

    //source_t source();

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


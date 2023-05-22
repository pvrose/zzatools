#pragma once

#include "prefix.h"

#include <set>
#include <vector>
#include <map>

#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Output.H>

class qso_dxcc :
    public Fl_Group
{

    class tree : public Fl_Tree
    {
    public:
        tree(int X, int Y, int W, int H, const char* L = nullptr);
        ~tree();

        void enable_widgets();

        void set_data(vector<prefix*>* items);

    protected:

        // Hang an item on the tree
        void hang_item(Fl_Tree_Item*& child, prefix* pfx);

        vector<prefix*>* prefixes_;
    };

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

    static void cb_tree(Fl_Widget* w, void* v);

    // Widgets:
    // Tree
    tree* tree_;
    // Callsign label
    Fl_Output* op_call_;
    // Buttons showing worked before
    wb4_buttons* g_wb4_;

    // Callsign
    string callsign_;
    // Prefix decode
    vector<prefix*>* prefixes_;
    // BAnd worked
    set<string>* bands_worked_;
    // Modes worked
    set<string>* modes_worked_;

};


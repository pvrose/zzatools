#pragma once

#include "record.h"

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Output.H>

class qso_qth :
    public Fl_Group
{
public:
    qso_qth(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_qth();

    void load_values();
    void create_form(int X, int Y);
    void save_values();
    void enable_widgets();

    void set_qth(string name);

protected:

    static void cb_bn_edit(Fl_Widget* w, void* v);

    class table : public Fl_Table_Row
    {
    public:
         table(int X, int Y, int W, int H, const char* L = nullptr);
        ~table();
        // public methods
       // inherited from Fl_Table_Row
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);
        void set_data(record* macro);

    protected:
        // The macro definition
        record* macro_;
        // Sorted list of field names
        vector<string> fields_;


    };
    // Name of QTH
    string qth_name_;
    // Macro expansion
    record* qth_details_;
    // Widgets
    Fl_Output* op_name_;
    Fl_Output* op_descr_;
    table* table_;
    Fl_Button* bn_edit_;

};


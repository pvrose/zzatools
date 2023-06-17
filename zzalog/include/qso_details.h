#pragma once

#include "record.h"

#include <string>
#include <set>
#include <vector>
#include <map>

#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table.H>

using namespace std;

class qso_details :
    public Fl_Group
{
public:
    qso_details(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_details();

    void create_form();
    void enable_widgets();

    void set_call(string name);

protected:
    class table_d : public Fl_Table
    {
    public:
        table_d(int X, int Y, int W, int H, const char* L = nullptr);
        ~table_d();
        // public methods
       // inherited from Fl_Table_Row
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);
        // Set data
        void set_data(set<string> names, set<string> qths, set<string> locators);
        // Use button
        static void cb_table(Fl_Widget* w, void* v);


    protected:

        enum item_type {
            NAME,
            QTH,
            LOCATOR
        };

        struct item_details {
            item_type type;
            string value;
        };

        vector<item_details> items_;

        struct item_names {
            const char* heading;
            const char* field;
        };

        const map<item_type, item_names> name_map_ = {
            {NAME, {"NAME ", "NAME"}},
            {QTH, {"QTH ", "QTH"}},
            {LOCATOR, {"GRID ", "GRIDSQUARE" }}
        };

    };

    class table_q : Fl_Table
    {
    public:
        table_q(int X, int Y, int W, int H, const char* L = nullptr);
        ~table_q();
        // public methods
        // inherited from Fl_Table_Row
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);
        // Click on row to peek 
        static void cb_table(Fl_Widget* w, void* v);
        // Set data
        void set_data(set<qso_num_t> items);
            
    protected:
        vector<qso_num_t> items_;

    };

protected:

    void get_qsos();

    string callsign_;

    //widgets
    Fl_Output* op_call_;
    table_d* table_details_;
    table_q* table_qsos_;


};


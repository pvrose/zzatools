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

// A class which displays information about previous QSOs with this contact
class qso_details :
    public Fl_Group
{
public:
    qso_details(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_details();

    void create_form();
    void enable_widgets();

    // Set the call to display details for
    void set_call(string name);

protected:
    // A table to show collected details about the contacted station 
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

        // Item to show in the row
        enum item_type {
            NAME,
            QTH,
            LOCATOR
        };

        // The row contents
        struct item_details {
            item_type type;
            string value;
        };

        // The items to display
        vector<item_details> items_;

        // The headers
        struct item_names {
            const char* heading;
            const char* field;
        };
        // Relates the item type, its header text and the record field
        const map<item_type, item_names> name_map_ = {
            {NAME, {"NAME ", "NAME"}},
            {QTH, {"QTH ", "QTH"}},
            {LOCATOR, {"GRID ", "GRIDSQUARE" }}
        };

    };

    // Table to display the previous QSOs with this contact
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
        // The previous QSOs
        vector<qso_num_t> items_;

    };

protected:
    // Search the book for the previous qSOs
    void get_qsos();
    // Callsign of contacted station
    string callsign_;

    //widgets
    Fl_Output* op_call_;
    table_d* table_details_;
    table_q* table_qsos_;


};


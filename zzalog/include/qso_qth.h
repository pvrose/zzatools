#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Output.H>

using namespace std;

class record;

// This displays the definition of the QTH defined by APP_ZZA_QTH
// and allowd it to be edited
class qso_qth :
    public Fl_Group
{
public:
    qso_qth(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_qth();

    // Load settings
    void load_values();
    // Craete the widgets
    void create_form(int X, int Y);
    // Save settings
    void save_values();
    // Configure the widgets
    void enable_widgets();

    // Set the Qth name
    void set_qth(string name);

protected:

    // Edit button clicked - opens qth_dialog
    static void cb_bn_edit(Fl_Widget* w, void* v);

    // Editing table 
    class table : public Fl_Table_Row
    {
    public:
         table(int X, int Y, int W, int H, const char* L = nullptr);
        ~table();
        // public methods
       // inherited from Fl_Table_Row
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);
        // Display the QTH values for this qSO
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


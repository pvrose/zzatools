#pragma once

#include "cty_data.h"
#include "utils.h"

#include <set>
#include <vector>
#include <map>
#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Table.H>

using namespace std;

class band_set;
class book;
class record;

class Fl_Button;
class Fl_Light_Button;
class Fl_Output;

enum location_t : uchar;
enum worked_t : uchar;

// This class displays the "worked before" status for the DX - band and mode
class qso_dxcc :
    public Fl_Group
{

    // A table 
    class wb4_table : public Fl_Table {

    public:

        wb4_table(int X, int Y, int W, int H, const char* L = nullptr);
        ~wb4_table();
        // inherited from Fl_Table
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);

        void set_data();

    protected:

        // Worked data
        struct wkd_line {
            string text;
            bool any;
            bool band;
            bool mode;
        };
        // worked matrix
        map < worked_t, wkd_line > wkd_matrix_;
     };

public:
    qso_dxcc(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_dxcc();

    virtual int handle(int event);

    // Create the widgets
    void create_form();
    // Configure them
    void enable_widgets();

    // Set the data - parse the callsign in the current qso
    void set_data(record* qso);

protected:

    // Button for QRZ.com
    static void cb_bn_qrz(Fl_Widget* w, void* v);
 
    // Widgets:
    // Callsign label
    Fl_Output* op_call_;
    // Prefix data
    Fl_Output* op_source_;
    Fl_Output* op_prefix_;
    Fl_Output* op_geography_;
    Fl_Output* op_usage_;
    Fl_Output* op_zones_;
    Fl_Output* op_coords_;
    Fl_Output* op_dist_bear_;
    // 
    // Buttons showing worked before
    wb4_table* g_wb4_;
    // Call QRZ.com
    Fl_Button* bn_qrz_;

    // Callsign
    string callsign_;
    // DXCC nickname - eg GM for Scotland
    string nickname_;
    // DXCC full name
    string name_;
    // The CQ zone
    int cq_zone_;
    int itu_zone_;
    // The DXCC number per ADIF
    int dxcc_;
    // The latitude and longitude of the DXCC centre
    lat_long_t location_;
    location_t loc_source_;
    // The latitude and longitude of the user station
    lat_long_t my_location_;
    // How has the callsign been parsed to get the DXCC - decoded or exception
    cty_data::parse_source_t source_;
    // Current geography
    string geography_;
    // Current usage
    string usage_;
    // Current record
    record* qso_;
    // The continent
    string continent_;
    // Station callsign
    string station_;

    // BAnd worked
    band_set* bands_worked_;
    // Modes worked
    set<string>* modes_worked_;
    // Source of prefix parsing
    

};


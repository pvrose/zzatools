#pragma once

#include "cty_data.h"
#include "utils.h"

#include <set>
#include <vector>
#include <map>
#include <string>

#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

using namespace std;

class book;
class band_set;
class Fl_Output;
class Fl_Button;
class Fl_Light_Button;

enum location_t : uchar;


// This class displays the "worked before" status for the DX - band and mode
class qso_dxcc :
    public Fl_Group
{

    // A display of all the bands and modes worked with the ones for this DXCC
    // indicated by button colour
    class wb4_buttons : public Fl_Scroll
    {
    public:
        wb4_buttons(int X, int Y, int W, int H, const char* L = nullptr);
        ~wb4_buttons();

        // Callback when a mode button is clicked
        static void cb_bn_mode(Fl_Widget* w, void* v);
        // Callback when a band button is clicked
        static void cb_bn_band(Fl_Widget* w, void* v);

        // Create the buttons
        void create_form();
        // Update their status
        void enable_widgets();

        // widget maps
        map<string, Fl_Widget*> map_;

 
    protected:
        // A set of bands worked (ordered by frequency)
        band_set* dxcc_bands_;
        // A set of submodes worked
        set<string>* dxcc_submodes_;
        // A set of all bands in the log (ordered by frequency)
        band_set* all_bands_;
        // a set of all submodes in the log
        set<string>* all_submodes_;
    };

public:
    qso_dxcc(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_dxcc();

    // Create the widgets
    void create_form();
    // Configure them
    void enable_widgets();

    // Set the data - parse the callsign in the current qso
    void set_data(record* qso);

protected:

    // Button for QRZ.com
    static void cb_bn_qrz(Fl_Widget* w, void* v);
    // Button for show extract
    static void cb_bn_show_xt(Fl_Widget* w, void* v);
    

    // Widgets:
    // Callsign label
    Fl_Output* op_call_;
    // Prefix data
    Fl_Output* op_source_;
    Fl_Output* op_prefix_;
    Fl_Output* op_geo_;
    Fl_Output* op_dist_bear_;
    // 
    // Buttons showing worked before
    wb4_buttons* g_wb4_;
    // Call QRZ.com
    Fl_Button* bn_qrz_;
    // Show all/extracted
    Fl_Light_Button* bn_show_extract_;

    // Callsign
    string callsign_;
    // DXCC nickname - eg GM for Scotland
    string nickname_;
    // DXCC full name
    string name_;
    // The CQ zone
    int cq_zone_;
    // The DXCC number per ADIF
    int dxcc_;
    // The latitude and longitude of the DXCC centre
    lat_long_t location_;
    location_t loc_source_;
    // The latitude and longitude of the user station
    lat_long_t my_location_;
    // How has the callsign been parsed to get the DXCC - decoded or exception
    cty_data::parse_source_t source_;
    // Current record
    record* qso_;
    // Use extracted data for worked b4
    bool show_extract_;

    // BAnd worked
    band_set* bands_worked_;
    // Modes worked
    set<string>* modes_worked_;
    // Source of prefix parsing
    

};


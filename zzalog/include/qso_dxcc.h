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

//! This class displays the "worked before" status for the DX - band and mode
class qso_dxcc :
    public Fl_Group
{

    // This class displays the "worked before" status in tabular form.
    class wb4_table : public Fl_Table {

    public:

        //! Constructor.

        //! \param X horizontal position within host window
        //! \param Y vertical position with hosr window
        //! \param W width 
        //! \param H height
        //! \param L label
        wb4_table(int X, int Y, int W, int H, const char* L = nullptr);
        //! Destructor.
        ~wb4_table();
        //! Inherited from Fl_Table to draw the contents of each cell.
        virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
            int W = 0, int H = 0);

        //! Gathers the data required for the table from the current QSO and date held in book_.
        void set_data();

    protected:

        //! The data for one line of the "worked before" table.
        struct wkd_line {
            string text;     //!< The name of the tested item.
            bool any;        //!< true: worked on any band or mode.
            bool band;       //!< true: worked on the same band as current QSO.
            bool mode;       //!< true: worked on the same mode as current QSO.
        };
        //! The data for the table mapped by "worked before" item against status.
        map < worked_t, wkd_line > wkd_matrix_;
     };

public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    qso_dxcc(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~qso_dxcc();

    //! Inherited from Fl_Group::handle() - accepts focus to let keyboard F1 open userguide.
    virtual int handle(int event);

    //! Instantiate component widgets.
    void create_form();
    //! Configure component widgets after data change.
    void enable_widgets();

    //! Set the data - parse the callsign in the current \p qso.
    void set_data(record* qso);

protected:

    //! Callback from "QRZ.com" button: opens QRZ.com page for this callsign.
    static void cb_bn_qrz(Fl_Widget* w, void* v);
    //! Callback from "Check Data" button.
    
    //! Opens a dialog showing the status of the files used
    //! and buttons to guide the user through updating them.
    static void cb_check_age(Fl_Widget * w, void* v);
 
    // Widgets:
    // Callsign label
    Fl_Output* op_call_;         //!< Output displaying callsign.
    // Prefix data
    Fl_Output* op_source_;       //!< Output showing how the callsign was parsed.
    Fl_Output* op_prefix_;       //!< Output showing parsed prefix and name.
    Fl_Output* op_geography_;    //!< Output showing secondary geography.
    Fl_Output* op_usage_;        //!< Output showing special usage.
    Fl_Output* op_zones_;        //!< Output showing CQ and ITU zones.
    Fl_Output* op_coords_;       //!< Output showing latitude and longitude.
    Fl_Output* op_dist_bear_;    //!< Output showing distance and bearing from user's station.
    wb4_table* g_wb4_;           //!< Table showing "Worked before" status. 
    Fl_Button* bn_qrz_;          //!< Button "QRZ.com".
    Fl_Button* bn_check_age_;    //!< Button "Check Data".

    //! Current callsign.
    string callsign_;
    //! DXCC nickname - eg GM for Scotland.
    string nickname_;
    //! DXCC full name.
    string name_;
    //! The CQ zone.
    int cq_zone_;
    //! The ITU Zone.
    int itu_zone_;
    //! The DXCC number per ADIF.
    int dxcc_;
    //! The latitude and longitude of the DXCC centre
    lat_long_t location_;
    //! The source of geographic coordinates.
    location_t loc_source_;
    //! The latitude and longitude of the user station
    lat_long_t my_location_;
    //! How has the callsign been parsed to get the DXCC - decoded or exception
    cty_data::parse_source_t source_;
    //! Current geographic subdivision.
    string geography_;
    //! Current usage
    string usage_;
    //! Current record
    record* qso_;
    //! Current continent
    string continent_;
    //! Station callsign
    string station_;

    //! The set of bands worked.
    band_set* bands_worked_;
    //! The set of modes worked.
    set<string>* modes_worked_;

};


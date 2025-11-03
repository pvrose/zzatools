#pragma once

#include <string>
#include <map>
#include <vector>

#include <nlohmann/json.hpp>

#include <FL/Fl_Table.H>



//! Tabular display of DXCC worked status.
class dxcc_table :
    public Fl_Table
{
public:
    //! Constructor.

    //! \param X horizontal position within host window
    //! \param Y vertical position with hosr window
    //! \param W width 
    //! \param H height
    //! \param L label
    dxcc_table(int X, int Y, int W, int H, const char* L = nullptr);
    //! Destructor.
    ~dxcc_table();

    //! Display mode.
    enum display_t : char {
        TOTAL,            //!< Display regardess of bands or modes
        BANDS,            //!< Display on a per-band basis.
        DXCC_MODES,       //!< Display on a per DXCC mode basis.
        MODES             //!< Display on a per mode basis.
    };
    //! Filter on confirmation status.
    enum confirm_t : char {
        WORKED = 0,       //!< Display even if not confirmed.
        EQSL = 1,         //!< Display if confirmed by eQSL.cc
        LOTW = 2,         //!< Display if confirmed by Logbook of the World.
        CARD = 4          //!< Display if confirmed by a physical card.
    };

    //! Set the display type
    void display_type(display_t t);
    //! Set the confirmation mode
    void confirm_type(confirm_t t);
    //! Get the display type
    display_t display_type();
    //! Get the confirmation mode
    confirm_t confirm_type();
    //! Inherited from Fl_Table, used to draw the table.
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);

    //! Minimum height
    int min_h();

protected:

    //! Scan the full logbook, collecting data into the various bins.
    void scan_book();
    //! Configure the table
    void configure_table();

    // Main data structure
    //! Count QSOs per item
    typedef std::map<std::string, int> count_t;
    //! Count QSOs by item and DXCC entity identification
    std::map<int, count_t> data_;
    //! Total counts.
    count_t total_counts_;
    //! Count QSOs per DXCC.
    std::map<int, int> qsos_dxcc_;
    //! Count DXCC entities per band.
    count_t dxccs_band_;
    //! Total QSOs
    int total_qsos_;

    //! Column header std::list, based on type of count.
    std::vector<std::string> column_names_;
    //! Row header std::list, uses DXCC entity identification.
    std::vector<int> row_ids_;

    //! Current display type.
    display_t display_type_;
    //! Current confirmation mode.
    confirm_t confirm_type_;
    //! The size of the page.
    int rows_in_page_;

 
};

//! JSON serialisation for dxcc_table::display_t
NLOHMANN_JSON_SERIALIZE_ENUM(dxcc_table::display_t, {
    { dxcc_table::TOTAL, "Total" },
    { dxcc_table::BANDS, "By Bands" },
    { dxcc_table::DXCC_MODES, "By DXCC Modes" },
    { dxcc_table::MODES, "By ADIF Modes" }
    }
)

//! JSON serialisation for dxcc_table::display_t
NLOHMANN_JSON_SERIALIZE_ENUM(dxcc_table::confirm_t, {
    { dxcc_table::WORKED, "Worked" },
    { dxcc_table::EQSL, "eQSL.cc" },
    { dxcc_table::LOTW, "LotW" },
    { dxcc_table::CARD, "Paper Card" }
    }
)

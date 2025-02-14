#pragma once

#include <string>
#include <map>
#include <vector>

#include <FL/Fl_Table.H>

using namespace std;

class dxcc_table :
    public Fl_Table
{
public:
    dxcc_table(int X, int Y, int W, int H, const char* L = nullptr);
    ~dxcc_table();

    enum display_t : char {
        TOTAL,
        BANDS,
        DXCC_MODES,
        MODES
    };
    enum confirm_t : char {
        WORKED = 0,
        EQSL = 1,
        LOTW = 2,
        CARD = 4
    };

    // Set the display type
    void display_type(display_t t);
    // Set the conformed mode
    void confirm_type(confirm_t t);
    // Get the display type
    display_t display_type();
    // Get the confirmation mode
    confirm_t confirm_type();
    // inherited from Fl_Table
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);

    // Minimum height
    int min_h();

protected:

    // Get the data
    void scan_book();
    // Configure the table
    void configure_table();

    // Main data structure
    // Count QSOs per band or mode
    typedef map<string, int> count_t;
    // Order QSOs by entity
    map<int, count_t> data_;
    // Total counts
    count_t total_counts_;
    // QSOs per DXCC
    map<int, int> qsos_dxcc_;
    // DXCCs per band
    count_t dxccs_band_;
    // Total QSOs
    int total_qsos_;

    // Column map
    vector<string> column_names_;
    // Row map
    vector<int> row_ids_;

    // Current display type
    display_t display_type_;
    // Current confirmation status
    confirm_t confirm_type_;
    // The size of the page
    int rows_in_page_;

 
};


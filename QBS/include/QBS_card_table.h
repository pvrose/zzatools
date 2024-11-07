#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Table.H>

using namespace std;

class QBS_data;
class QBS_window;

class QBS_card_table :
    public Fl_Table
{
public:
    QBS_card_table(int X, int Y, int W, int H, const char* L);
    ~QBS_card_table();

    // inherited from Fl_Table
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);

    // Set the callsign
    void call(string call);

    // Redraw the table - size of data may have changed
    virtual void draw();

protected:

    // Get the number of rows
    int num_rows();

    string call_;
    QBS_data* data_;
    QBS_window* win_;

    // Map of table row to box number
    vector<int> boxes_;

};


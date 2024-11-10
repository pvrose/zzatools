#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Table.H>

using namespace std;

class QBS_data;

class QBS_top20 :
    public Fl_Table
{
public:

    QBS_top20(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_top20();

    void box(int box);

    void data(QBS_data* d);

protected:
    // inherited from Fl_Table
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);

    int box_;

    QBS_data* data_;

    vector<string> top20_;

};


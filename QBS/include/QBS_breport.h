#pragma once

#include <vector>
#include <string>

#include <FL/Fl_Table.H>

class QBS_data;

using namespace std;

class QBS_breport :
    public Fl_Table
{
public:
    QBS_breport(int X, int Y, int W, int H, const char* L = nullptr);
    ~QBS_breport();

    void box(int b);

    void data(QBS_data* d);

protected:
    // inherited from Fl_Table
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
        int W = 0, int H = 0);

    int box_;

    QBS_data* data_;

    struct count_data {
        string callsign;
        int received{ 0 };
        int sent{ 0 };
        int recycled{ 0 };
    };
     
    vector<count_data*> counts_;
};


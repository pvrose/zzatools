#pragma once

#include "QBS_data.h"

#include <list>

#include <FL/Fl_Table.H>

using namespace std;
class QBS_data;

class QBS_notes :
    public Fl_Table
{
public:
    QBS_notes(int X, int Y, int W, int H, const char* L);
    ~QBS_notes();

    // inherited from Fl_Table_Row
    virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
		int W = 0, int H = 0);

    void set_data(void* data);

protected:
    void* data_;
 };


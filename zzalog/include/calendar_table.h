#pragma once

#include <FL/Fl_Table.H>


// text is column header
const char WEEKDAY[7][2] = { "S", "M", "T", "W", "T", "F", "S" };

// This class provides the Fl_Table object to be used in calendar
class calendar_table : public Fl_Table
{
public:
	calendar_table(int X, int Y, int W, int H, tm date);
	virtual ~calendar_table();

	// inherited from Fl_Table
	virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
		int W = 0, int H = 0);

	// set date
	void value(tm date);
	// get date
	tm& value();
	// get the date for the current cell
	tm* get_date(int R, int C);

protected:

	// attributes

	// The date selected by user
	tm selected_date_;
	// The date of the 1st of displayed month
	tm month_start_;
	// Today's date
	tm today_;


};


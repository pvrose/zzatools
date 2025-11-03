#pragma once

#include <FL/Fl_Table.H>


//! Array of days of the week used as column headers.
const char WEEKDAY[7][2] = { "S", "M", "T", "W", "T", "F", "S" };

//! This class provides the Fl_Table object to be used in calendar
class calendar_table : public Fl_Table
{
public:
	//! Constructor

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param date date to std::set.
	calendar_table(int X, int Y, int W, int H, tm date);
	//! Destructor.
	virtual ~calendar_table();

	//! Overload of Fl_Table::draw_cell is called to draw the contents of the cell.
	
	//! Adds column headers as days of the week. Adds individual days of the month in
	//! the cells with differnt colours:
	//! - Grey: outwith the month.
	//! - Red: today's date.
	//! - Selected colour: the selected date.
	virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
		int W = 0, int H = 0);

	//! std::set date using tm structure.
	void value(tm date);
	//! Get date as a tm structure.
	tm& value();
	//! Get the date for the indicated cell
	
	//! \param R row in the table.
	//! \param C column in the table.
	//! \return date as a tm structure.
	tm* get_date(int R, int C);

protected:

	// attributes

	//! The date selected by user
	tm selected_date_;
	//! The date of the 1st of displayed month
	tm month_start_;
	//! Today's date
	tm today_;


};


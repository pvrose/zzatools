#include "calendar_table.h"
#include "drawing.h"
#include "utils.h"


#include <FL/fl_draw.H>

// Constructor for the calendar table
calendar_table::calendar_table(int X, int Y, int W, int H, tm date) :
	Fl_Table(X, Y, W, H, nullptr)
{
	// Set table parameters
	col_header(true);
	col_header_color(FL_GRAY);
	selection_color(FL_YELLOW);
	cols(7);
	rows(5);
	value(date);
	end();
}

// Destructor
calendar_table::~calendar_table()
{
}

// inherited from Fl_Table_Row - called when a cell is being drawn
void calendar_table::draw_cell(TableContext context, int R, int C, int X, int Y,
	int W, int H) {

	switch (context) {
	case CONTEXT_STARTPAGE:
		// Starting to draw the table - define font
		fl_font(0, FL_NORMAL_SIZE);
		return;

	case CONTEXT_ENDPAGE:
	case CONTEXT_ROW_HEADER:
		// do nothing
		return;

	case CONTEXT_COL_HEADER:
		// put column header text into header - one of S M T W T F S
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, col_header_color());
			fl_color(FL_BLACK);
			fl_draw(WEEKDAY[C], X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Fill in the day from the date
		fl_push_clip(X, Y, W, H);
		{
			// Get date for cell
			tm* date = get_date(R, C);
			Fl_Color bg_color;

			// BG COLOR
			if (date == nullptr) {
				// Outwith this month
				bg_color = COLOUR_GREY;
			}
			else if (mktime(date) == mktime(&today_)) {
				// Today
				bg_color = FL_RED;
			}
			else if (mktime(date) == mktime(&selected_date_)) {
				// Selected date
				bg_color = selection_color();
			}
			else {
				// Other days
				bg_color = FL_BACKGROUND_COLOR;
			}
			// Draw the blackground
			fl_color(bg_color);
			fl_rectf(X, Y, W, H);

			// TEXT
			fl_color(fl_contrast(FL_FOREGROUND_COLOR, bg_color));
			if (date != nullptr) {
				// Get day of the month and write it in cell
				char mday[3];
				strftime(mday, 3, "%e", date);
				fl_draw(mday, X + 1, Y, W - 1, H, FL_ALIGN_CENTER);
			}

			// BORDER
			fl_color(FL_LIGHT1);
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
		}
		fl_pop_clip();
		return;
	default:
		return;
	}
}

// get the selected date
tm& calendar_table::value() {
	return selected_date_;
}

// Set the value
void calendar_table::value(tm date) {

	// Get today's date (in UTC) and normalise it to midnight.
	time_t now = time(nullptr);
	today_ = *(gmtime(&now));
	today_.tm_hour = 0;
	today_.tm_min = 0;
	today_.tm_sec = 0;

	if (mktime(&date) == -1) {
		// The date cannot be represented - std::set to today
		selected_date_ = today_;
	}
	else {
		// use value
		selected_date_ = date;
	}

	// get the first of the month
	month_start_ = selected_date_;
	month_start_.tm_mday = 1;
	// now convert to time_t and back to correct wday
	refresh_tm(&month_start_);
	// redraw so that today is highlighted again
	redraw();
}

// returns the date represented by row and column in the calendar. NULL if outwith the month
tm* calendar_table::get_date(int R, int C) {
	// Get the column containing 1st of the month
	int col_first = month_start_.tm_wday;
	// Number of days in month
	int last_day = days_in_month(&selected_date_);
	tm* date = nullptr;
	if (R == 0 && C < col_first) {
		// special case where last days are printed in row 0.

		// Date for the last Sunday
		int sunday_mday = 36 - col_first;
		if (C <= last_day - sunday_mday) {
			// The cell represents a date between last Sunday and end of month
			date = new tm(month_start_);
			date->tm_mday = sunday_mday + C;
			refresh_tm(date);
		}
		else {
			// Cell is outwith the month
			date = nullptr;
		}
	}
	else {
		// Date for the Sunday in that row
		int sunday_mday = (R * 7) + 1 - col_first;
		if (C <= last_day - sunday_mday || C + 1 > sunday_mday) {
			// day is not before first or after last of month
			date = new tm(month_start_);
			date->tm_mday = sunday_mday + C;
			refresh_tm(date);
		}
		else {
			// Cell is outwith the month
			date = nullptr;
		}
	}
	return date;
}

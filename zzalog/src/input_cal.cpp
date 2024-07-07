#include "input_cal.h"
#include "utils.h"
#include "drawing.h"

#include <FL/fl_draw.H>


// Constructor for the calendar table
input_cal::cal_table::cal_table(int X, int Y, int W, int H, const char* L) :
	Fl_Table(X, Y, W, H, L)
{
	// Set table parameters
	col_header(true);
	col_header_color(FL_GRAY);
	selection_color(FL_YELLOW);
	cols(7);
	rows(5);
	end();
}

// Destructor
input_cal::cal_table::~cal_table()
{
}

// inherited from Fl_Table_Row - called when a cell is being drawn
void input_cal::cal_table::draw_cell(TableContext context, int R, int C, int X, int Y,
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
			fl_color(FL_FOREGROUND_COLOR);
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
	}
}

// get the selected date
tm input_cal::cal_table::value() {
	return selected_date_;
}

// Set the value
void input_cal::cal_table::value(tm date) {

	// Get today's date (in UTC) and normalise it to midnight.
	time_t now = time(nullptr);
	today_ = *(gmtime(&now));
	today_.tm_hour = 0;
	today_.tm_min = 0;
	today_.tm_sec = 0;

	if (mktime(&date) == -1) {
		// The date cannot be represented - set to today
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
tm* input_cal::cal_table::get_date(int R, int C) {
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

// Constructor
input_cal::cal_popup::cal_popup(int X, int Y, int W, int H, const char* L) :
	Fl_Window(X, Y, W, Y, L),
	display_date_(tm()),
	value_("")
{
	// Position calculations
	const int WSMALLBN = HBUTTON * 3 / 2;
	const int ROWHEIGHT = HBUTTON;
	const int COLWIDTH = WSMALLBN;
	const int R0 = EDGE;
	const int H0 = HBUTTON;
	const int R1 = R0 + GAP + H0;
	const int H1 = 6 * ROWHEIGHT;
	const int R2 = R1 + GAP + H1;
	const int H2 = HBUTTON;
	const int HALL = R2 + H2 + EDGE;
	const int C0 = EDGE;
	const int C1 = C0 + COLWIDTH;
	const int C2 = C1 + COLWIDTH;
	const int W2 = 2 * COLWIDTH;
	const int C3 = C2 + W2;
	const int C4 = C3 + COLWIDTH;
	const int C5 = C4 + COLWIDTH;
	const int WALL = C5 + COLWIDTH + EDGE;
	const int WTABLE = 7 * COLWIDTH;

	// Remove standard window border and delineate the window
	size(WALL, HALL);
	box(FL_BORDER_BOX);
	clear_border();
	
	// Create row 1 buttons

	// previous year
	Fl_Button* bn_0_0 = new Fl_Button(C0, R0, COLWIDTH, H0, "@<<");
	bn_0_0->box(FL_UP_BOX);
	bn_0_0->callback(cb_bn_cal, (void*)BN_PREV_Y);
	bn_0_0->when(FL_WHEN_RELEASE);
	bn_0_0->tooltip("Previous year");
	// previous month
	Fl_Button* bn_0_1 = new Fl_Button(C1, R0, COLWIDTH, H0, "@<");
	bn_0_1->box(FL_UP_BOX);
	bn_0_1->callback(cb_bn_cal, (void*)BN_PREV_M);
	bn_0_1->when(FL_WHEN_RELEASE);
	bn_0_1->tooltip("Previous month");
	// Current date
	month_bn_ = new Fl_Button(C2, R0, W2, H0);
	month_bn_->box(FL_FLAT_BOX);
	month_bn_->tooltip("The current month");
	// next month
	Fl_Button* bn_0_3 = new Fl_Button(C3, R0, COLWIDTH, H0, "@>");
	bn_0_3->box(FL_UP_BOX);
	bn_0_3->callback(cb_bn_cal, (void*)BN_NEXT_M);
	bn_0_3->when(FL_WHEN_RELEASE);
	bn_0_3->tooltip("Next month");
	// next year
	Fl_Button* bn_0_4 = new Fl_Button(C4, R0, COLWIDTH, H0, "@>>");
	bn_0_4->box(FL_UP_BOX);
	bn_0_4->callback(cb_bn_cal, (void*)BN_NEXT_Y);
	bn_0_4->when(FL_WHEN_RELEASE);
	bn_0_4->tooltip("Next year");
	// today
	Fl_Button* bn_0_5 = new Fl_Button(C5, R0, COLWIDTH, H0, "@>|");
	bn_0_5->box(FL_UP_BOX);
	bn_0_5->callback(cb_bn_cal, (void*)BN_TODAY);
	bn_0_5->when(FL_WHEN_RELEASE);
	bn_0_5->tooltip("Today");


	// Row 2 - the calendar
	table_ = new cal_table(C0, R1, WTABLE, H1);
	table_->callback(cb_cal_cal, nullptr);
	table_->when(FL_WHEN_RELEASE);
	table_->row_height_all(ROWHEIGHT - 1);
	table_->col_width_all(COLWIDTH - 1);

	// Row 3 - OK/Cancel buttons
	Fl_Button* bn_2_3 = new Fl_Button(C4, R2, COLWIDTH, H2, "@circle");
	bn_2_3->labelcolor(FL_GREEN);
	bn_2_3->callback(cb_bn_cal, (void*)BN_OK);
	bn_2_3->when(FL_WHEN_RELEASE);
	bn_2_3->tooltip("Set selected date and close");

	Fl_Button* bn_2_4 = new Fl_Button(C5, R2, COLWIDTH, H2, "@square");
	bn_2_4->labelcolor(FL_RED);
	bn_2_4->callback(cb_bn_cal, (void*)BN_CANCEL);
	bn_2_4->when(FL_WHEN_RELEASE);
	bn_2_4->tooltip("Close without changing date");

	end();
}

// Destructor
input_cal::cal_popup::~cal_popup() {
	clear();
}

// Callback when clicking a button
void input_cal::cal_popup::cb_bn_cal(Fl_Widget* w, void* v) {
	// Indicates the button clicked
	button_t action = (button_t)(intptr_t)v;
	cal_popup* that = ancestor_view<cal_popup>(w);
	switch (action) {
	case BN_OK:
		// Get the display date from the calendar and convert it to text
		char temp[25];
		strftime(temp, 25, ADIF_DATEFORMAT, &that->table_->value());
		that->value_ = temp;
		that->display_date_ = that->table_->value();
		// Tell instancing view and delete this
		that->do_callback();
		Fl::delete_widget(that);
		break;
	case BN_CANCEL:
		// Restore to original value set.
		string_to_tm(that->value_, that->display_date_, ADIF_DATEFORMAT);
		// Tell instancing view and delete this
		that->do_callback();
		Fl::delete_widget(that);
		break;
	case BN_PREV_Y:
		// if year > 0 (1900) decrement it
		if (that->display_date_.tm_year > 0) {
			that->display_date_.tm_year--;
			that->change_date();
		}
		break;
	case BN_PREV_M:
		// Go to previous month
		if (that->display_date_.tm_mon > 0) {
			// month != Jan, decrement it
			that->display_date_.tm_mon--;
			that->change_date();
		}
		else if (that->display_date_.tm_year > 0) {
			// if year > 1900 - change to Dec the previous year
			that->display_date_.tm_mon = 11;
			that->display_date_.tm_year--;
			that->change_date();
		}
		break;
	case BN_NEXT_Y:
		// increment year (assume far enough in the future is available
		that->display_date_.tm_year++;
		that->change_date();
		break;
	case BN_NEXT_M:
		// Next month
		if (that->display_date_.tm_mon < 11) {
			// month != Dec, increment it
			that->display_date_.tm_mon++;
			that->change_date();
		}
		else {
			// month == Dec, Jan the next year
			that->display_date_.tm_mon = 0;
			that->display_date_.tm_year++;
			that->change_date();
		}
		break;
	case BN_TODAY:
		// Go to today (UTC)
		time_t now = time(nullptr);
		that->display_date_ = *gmtime(&now);
		that->change_date();
	}
}

// handle click in table
void input_cal::cal_popup::cb_cal_cal(Fl_Widget* w, void* v) {
	// Get the calendar and this
	cal_table* table = (cal_table*)w;
	cal_popup* that = ancestor_view<cal_popup>(w);
	// Get the row and column clicked
	int row = table->callback_row();
	int col = table->callback_col();
	// Why is it clicked
	switch (table->callback_context()) {
	case Fl_Table::CONTEXT_CELL:
		// Clicked on a cell
		// Get the new date from the calendar table - 
		// nullptr returned if an invalid cell is clicked
		tm* new_date = table->get_date(row, col);
		if (new_date != nullptr) {
			that->display_date_ = *new_date;
			that->change_date();
		}
		break;
	}
}

// calendar close call back - called when the calendar closes
void input_cal::cal_popup::cb_cal_close(Fl_Widget* w, void* v) {
	cal_popup* that = ancestor_view<cal_popup>(w);
	that->do_callback();
	that->hide();
}



// Set the date into the cal_popup
void input_cal::cal_popup::value(string date) {
	// Set the date
	value_ = date;
	// Update associated 
	string_to_tm(date, display_date_, ADIF_DATEFORMAT);
	// now try and set the date into calendar - it may fail so read back defaulted
	change_date();
	display_date_ = table_->value();
}

// Returns the date
string input_cal::cal_popup::value() {
	return value_;
}

// Update the various representations of the date
void input_cal::cal_popup::change_date() {
	// First make the date valid
	if (display_date_.tm_mday > days_in_month(&display_date_)) {
		// day in month is too great make it the end of the month
		display_date_.tm_mday = days_in_month(&display_date_);
	}
	// Update the date in the widgets that need it
	char month[9];
	strftime(month, 9, "%h %Y", &display_date_);
	month_bn_->copy_label(month);
	table_->value(display_date_);
	redraw();
}

input_cal::input_cal(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	input_ = new Fl_Input(X, Y, W - H, H);
	input_->callback(cb_input);
	input_->when(FL_WHEN_ENTER_KEY);

	button_ = new Fl_Button(X + W - H, Y, H, H, "@2|>");
	button_->callback(cb_button);
	button_->when(FL_WHEN_RELEASE);

	end();
}

input_cal::~input_cal() {

}


// Overloaded value methods - date in ADIF format
void input_cal::value(string v) {
	value_ = v;
	input_->value(value_.c_str());
}

string input_cal::value() {

	return value_;
}

// Data input through Fl_Input
void input_cal::cb_input(Fl_Widget* w, void* v) {
	input_cal* that = ancestor_view<input_cal>(w);
	that->value_ = ((Fl_Input*)w)->value();
	that->do_callback();
}

// Data input through cal_popup
void input_cal::cb_popup(Fl_Widget* w, void* v) {
	input_cal* that = (input_cal*)v;
	that->value_ = ((cal_popup*)w)->value();
	that->input_->value(that->value_.c_str());
}

// Popup button pressed
void input_cal::cb_button(Fl_Widget* w, void* v) {
	input_cal* that = ancestor_view<input_cal>(w);
	that->popup_ = new cal_popup(w->x(), w->y(), 10, 10);
	that->popup_->callback(cb_popup, that);
	that->popup_->value(that->value_);
	that->popup_->show();
	that->popup_->take_focus();
	that->parent()->add(that->popup_);
}


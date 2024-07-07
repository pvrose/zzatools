#pragma once

#include <time.h>
#include <string>

#include <FL/Fl_Table.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>

using namespace std;

// text is column header
const char WEEKDAY[7][2] = { "S", "M", "T", "W", "T", "F", "S" };
// Format date is represented in ADIF
const char ADIF_DATEFORMAT[] = "%Y%m%d";

class input_cal : public Fl_Group {
	// The calendar
	// 
	// This class provides the Fl_Table object to be used in cal_input
	class cal_table : public Fl_Table
	{
	public:
		cal_table(int X, int Y, int W, int H, const char* L = nullptr);
		virtual ~cal_table();

		// inherited from Fl_Table
		virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0,
			int W = 0, int H = 0);

		// set date
		void value(tm date);
		// get date
		tm value();
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

	// The popup widget
	class cal_popup : public Fl_Window
	{
	public:
		cal_popup(int X, int Y, int W, int H, const char* L = nullptr);
		~cal_popup();

		// re-implement these
		void value(string data);
		string value();

		// calendar close call back - v points to data item
		static void cb_cal_close(Fl_Widget* w, void* v);

	protected:
		// callbacks
		enum button_t {
			BN_OK,
			BN_CANCEL,
			BN_PREV_Y,
			BN_PREV_M,
			BN_NEXT_Y,
			BN_NEXT_M,
			BN_TODAY
		};
		// Generic callback for buttons used in this widget
		static void cb_bn_cal(Fl_Widget* w, void* v);
		// Callback when calendar is clicked
		static void cb_cal_cal(Fl_Widget* w, void* v);
		// Update the various representations of the date
		void change_date();

		// attributes
		// the original value
		string value_;
		// the date being displayed
		tm display_date_;

		// accessible widgets
		// The calendar itself
		cal_table* table_;
		// The date label
		Fl_Button* month_bn_;
	};

public:

	input_cal(int X, int Y, int W, int H, const char* L = nullptr);
	~input_cal();

	// Overloaded value methods - date in ADIF format
	void value(string v);
	string value();

protected:

	// Data input through Fl_Input
	static void cb_input(Fl_Widget* w, void* v);
	// Data input through cal_popup
	static void cb_popup(Fl_Widget* w, void* v);
	// Popup button pressed
	static void cb_button(Fl_Widget* w, void* v);

	// value
	string value_;

	// The widgets
	Fl_Input* input_;
	Fl_Button* button_;

	cal_popup* popup_;
};




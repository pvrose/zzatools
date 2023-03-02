#ifndef __CALENDAR__
#define __CALENDAR__

#include "intl_widgets.h"

#include <ctime>
#include <string>
#include <FL/Fl_Table.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

using namespace std;

// TODO: Why can't this be in zzalib?



	// text is column header
	const char WEEKDAY[7][2] = { "S", "M", "T", "W", "T", "F", "S" };
	// Format date is represented in ADIF
	const char ADIF_DATEFORMAT[] = "%Y%m%d";

	// Datatype to pass to a calendar call back
	struct cal_cb_data_t {
		// Date value in ADIF format YYYYMMDD
		string* date_value;
		// The input widget to receive the date
		intl_input* date_input;
		// Default constructor
		cal_cb_data_t() {
			date_value = nullptr;
			date_input = nullptr;
		}
		// Initialising constructor
		cal_cb_data_t(string* dv, intl_input* di) {
			date_value = dv;
			date_input = di;
		}
	};

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

	// This class provides a widget to be used for date selection
	class calendar : public Fl_Window
	{
	public:
		calendar(int X, int Y);
		virtual ~calendar();

		// re-implement these
		void value(const char * data);
		const char * value();

		// calendar close call back - v points to data item
		static void cb_cal_close(Fl_Widget* w, void* v);
		// open calendar drop-down call back - v points to calendar
		static void cb_cal_open(Fl_Widget* w, void *v);

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
		calendar_table* table_;
		// The date label
		Fl_Button* month_bn_;
	};
#endif

#ifndef __CALENDAR__
#define __CALENDAR__

#include <ctime>
#include <string>
#include <FL/Fl_Table.H>
#include <FL/Fl_Window.H>

using namespace std;

class Fl_Button;
class Fl_Input;
class calendar_table;

	// Format date is represented in ADIF
	const char ADIF_DATEFORMAT[] = "%Y%m%d";
	const char ADIF_HOURFORMAT[] = "%H%M";

	// This class provides a widget to be used for date selection
	class calendar : public Fl_Window
	{

	public:
		calendar(int X, int Y);
		virtual ~calendar();

		// re-implement these
		void value(const char * data);
		const char * value();

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
		const char* value_;
		// the date being displayed
		tm display_date_;

		// accessible widgets
		// The calendar itself
		calendar_table* table_;
		// The date label
		Fl_Button* month_bn_;
	};
#endif

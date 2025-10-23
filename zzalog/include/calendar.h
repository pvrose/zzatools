#ifndef __CALENDAR__
#define __CALENDAR__

#include <ctime>
#include <string>
#include <FL/Fl_Table.H>
#include <FL/Fl_Window.H>



class Fl_Button;
class Fl_Input;
class calendar_table;

	//! Format date is represented in ADIF
	const char ADIF_DATEFORMAT[] = "%Y%m%d";
	//! Format time is represented in ADIF
	const char ADIF_HOURFORMAT[] = "%H%M";

	//! This class provides a widget to be used for date selection
	
	//! It displays a month at a time and can be navigated by month or year.
	class calendar : public Fl_Window
	{

	public:
		//! Constructor
		
		//! \param X preferred location to position left of window.
		//! \param Y preferred location to position top of window.
		calendar(int X, int Y);
		//! Destructor
		virtual ~calendar();

		//! Set the date value of the calendar in ADIF_DATEFORMAT.
		void value(const char * data);
		//! Get the date value of the calendar in ADIF_DATEFORMAT.
		const char * value();

	protected:
		//! Identifiers for action buttons in the widget.
		enum button_t {
			BN_OK,        //!< OK
			BN_CANCEL,    //!< CANCEL
			BN_PREV_Y,    //!< Go back 1 year.
			BN_PREV_M,    //!< Go back 1 month.
			BN_NEXT_Y,    //!< Go forward 1 year.
			BN_NEXT_M,    //!< Go forward 1 month.
			BN_TODAY      //!< Go to today's date.
		};
		//! Callback when an action button is clicked.
		
		//! \param w clicked button.
		//! \param v as button_t indicates the action to do.
		static void cb_bn_cal(Fl_Widget* w, void* v);
		//! Callback when calendar is clicked. Selects the date under the click position. 
		static void cb_cal_cal(Fl_Widget* w, void* v);
		//! Update the calendar after a date has been changed.
		void change_date();

		// attributes
		//! the original date value
		const char* value_;
		//! the date being displayed as a tm structure.
		tm display_date_;

		// accessible widgets
		//! The calendar_table widget displays the calendar.
		calendar_table* table_;
		//! The date label. Used to show the current selected month by name.s
		Fl_Button* month_bn_;
	};
#endif

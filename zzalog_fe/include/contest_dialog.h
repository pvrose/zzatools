#pragma once

#include "page_dialog.h"

#include <string>



class calendar_input;
class field_input;
class Fl_Button;
class Fl_Choice;
class Fl_Input;
class Fl_Input_Choice;
class Fl_Int_Input;
class Fl_Window;

struct ct_data_t;
struct ct_date_t;

//! This class provides the dialog to configure a specific contest.
class contest_dialog :
    public page_dialog
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	contest_dialog(int X, int Y, int W, int H, const char* L = nullptr);
	//! Desctructor.
	virtual ~contest_dialog();

	//! Allow F1 to open user guide.
	virtual int handle(int event);

	// inherited methods
	// Standard methods - need to be written for each
	//! Load values from settings ino internal database.
	virtual void load_values();
	//! Instantiates all the component widgets.
	
	//! \param X position within window.
	//! \param Y position within window.
	virtual void create_form(int X, int Y);
	//! Write data back to settings.
	virtual void save_values();
	//! Configure the component widgets after any data change.
	virtual void enable_widgets();

	// Callbacks
	//! Callback on entering contest identifier.
	
	//! Clears instance identifier and redraws all components.
	static void cb_id(Fl_Widget* w, void* v);
	//! Callback on entering instance identifier.
	
	//! Redraws all components after data change.
	static void cb_index(Fl_Widget* w, void* v);

protected:

	//! Populate contest instance identifier choice for the specified contest.
	void populate_ct_index();
	//! Populate algorithm choice from the std::list of coded algorithms.
	void populate_algorithm();
	//! Update all data values from the database for specified contest instance.
	void update_contest();
	//! Update timeframe component values.
	void update_timeframe();
	//! Update algorithm component values.
	void update_algorithm();

	//! Current contest identifier.
	std::string contest_id_;
	//! Current contest instance identifier.
	std::string contest_index_;
	//! Data relating to current contest instance.
	ct_data_t* contest_;

	// Widgets
	// Contest ID
	field_input* w_contest_id_;       //!< Choice for contest identifier: loaded from ADIF specification.
	Fl_Input_Choice* w_contest_ix_;   //!< Choice for conyest instance identifier.
	// Fields
	Fl_Input_Choice* w_algorithm_;    //!< Choice for algorithm.
	// Timeframe
	calendar_input* w_start_date_;    //!< Calendar selector for start date.
	Fl_Int_Input* w_start_time_;      //!< Input for start time.
	calendar_input* w_finish_date_;   //!< Calendar selector for finish date.
	Fl_Int_Input* w_finish_time_;     //!< Input for finish time.
};


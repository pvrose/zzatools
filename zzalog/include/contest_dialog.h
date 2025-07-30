#pragma once

#include "page_dialog.h"

#include <string>

using namespace std;

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

class contest_dialog :
    public page_dialog
{
public:
	contest_dialog(int X, int Y, int W, int H, const char* label = nullptr);
	virtual ~contest_dialog();

	virtual int handle(int event);

	// inherited methods
	// Standard methods - need to be written for each
	// Load values from settings
	virtual void load_values();
	// Used to create the form
	virtual void create_form(int X, int Y);
	// Used to write settings back
	virtual void save_values();
	// Used to enable/disable specific widget - any widgets enabled musr be attributes
	virtual void enable_widgets();

	// Callbacks
	// Contest ID field_input
	static void cb_id(Fl_Widget* w, void* v);
	// Contest index
	static void cb_index(Fl_Widget* w, void* v);

protected:

	// Populate contest index choice
	void populate_ct_index();
	// Populate logging
	void populate_algorithm();
	// Update contest
	void update_contest();
	// Update timeframe
	void update_timeframe();
	// Update algorithm
	void update_algorithm();

	// Current contest if any.
	string contest_id_;
	string contest_index_;
	ct_data_t* contest_;

	// Widgets
	// Contest ID
	field_input* w_contest_id_;
	Fl_Input_Choice* w_contest_ix_;
	// Fields
	Fl_Input_Choice* w_algorithm_;
	// Timeframe
	calendar_input* w_start_date_;
	Fl_Int_Input* w_start_time_;
	calendar_input* w_finish_date_;
	Fl_Int_Input* w_finish_time_;
};


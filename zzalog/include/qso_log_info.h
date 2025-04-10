#pragma once

#include <FL/Fl_Group.H>

using namespace std;

class Fl_Output;
class Fl_Fill_Dial;
class Fl_Button;
class Fl_Check_Button;

// Displays the current state of the log, whether it is "dirty"
// and controls around this
class qso_log_info :
	public Fl_Group
{


public:

	qso_log_info(int X, int Y, int W, int H, const char* l = nullptr);
	~qso_log_info();

	// get settings 
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable widgets
	void enable_widgets();
	// Save changes
	void save_values();
	// Enable timer -dependent widgets
	void enable_timer_widgets();

	static void cb_ticker(void* v);

	// Call back for enable check box
	static void cb_bn_enable(Fl_Widget* w, void* v);
	// Callback to force save
	static void cb_bn_save(Fl_Widget* w, void* v);

protected:
	// Log status
	Fl_Output* op_status_;
	// Load/Save progress bar
	Fl_Fill_Dial* pr_loadsave_;
	// Saved after every QSO?
	Fl_Check_Button* bn_save_enable_;
	// Save it!
	Fl_Button* bn_save_;

};
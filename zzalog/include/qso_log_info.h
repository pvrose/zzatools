#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>

using namespace std;

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

	static void cb_ticker(void* v);

	// Call back for enable check box
	static void cb_bn_enable(Fl_Widget* w, void* v);
	// Callback to force save
	static void cb_bn_save(Fl_Widget* w, void* v);

protected:
	// Log status
	Fl_Output* op_status_;
	// Load/Save progress bar
	Fl_Progress* pr_loadsave_;
	// Saved after every QSO?
	Fl_Check_Button* bn_save_enable_;
	// Save it!
	Fl_Button* bn_save_;

};
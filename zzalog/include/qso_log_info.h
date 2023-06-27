#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>

using namespace std;

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

	void ticker();

	static void cb_bn_enable(Fl_Widget* w, void* v);

protected:
	Fl_Output* op_status_;
	Fl_Progress* pr_loadsave_;
	Fl_Check_Button* bn_save_enable_;

};
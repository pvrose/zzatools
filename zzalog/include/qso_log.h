#pragma once

#include "qso_log_info.h"
#include "qso_qsl.h"

#include <FL\Fl_Tabs.H>

class qso_log :
    public Fl_Tabs
{
public:
	qso_log(int X, int Y, int W, int H, const char* l);
	~qso_log();

	// get settings 
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable widgets
	void enable_widgets();
	// Save changes
	void save_values();
	// 1s clock interface
	void ticker();

protected:

	// Callback
	static void cb_tabs(Fl_Widget* w, void* v);
	//// Redesign tab_positions
	//virtual int tab_positions();

	qso_log_info* log_info_;
	qso_qsl* qsl_ctrl_;

};


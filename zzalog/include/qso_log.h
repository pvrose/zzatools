#pragma once

#include "qso_log_info.h"
#include "qso_qsl.h"
#include "qso_server.h"
#include "qso_wx.h"

#include <FL/Fl_Tabs.H>

// This calss collects the qso_log_info, qso_qsl, qso_server and qso_wx objects
// into a tabbed object 
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

	// Return a refernece to the qso_qsl object
	qso_qsl* qsl_control();
	// Returns a reference to the qso_log-info object
	qso_log_info* log_info();
	

protected:

	// Callback
	static void cb_tabs(Fl_Widget* w, void* v);

	// The four objects
	qso_log_info* log_info_;
	qso_qsl* qsl_ctrl_;
	qso_server* server_ctrl_;
	qso_wx* wx_ctrl_;

};


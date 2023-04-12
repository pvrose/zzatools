#pragma once

#include "rig_if.h"

#include <map>
#include <string>

#include <FL/Fl_Tabs.H>

using namespace std;

class qso_tabbed_rigs :
    public Fl_Tabs
{
public:
	qso_tabbed_rigs(int X, int Y, int W, int H, const char* l);
	~qso_tabbed_rigs();

	// get settings 
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable widgets
	void enable_widgets();
	// Save changes
	void save_values();
	// Switch the rig on or off
	void switch_rig();
	// 1s clock interface
	void ticker();
	// Get the rig
	rig_if* rig();

protected:

	// Callback
	static void cb_tabs(Fl_Widget* w, void* v);
	//// Redesign tab_positions
	//virtual int tab_positions();

	// Map the labels to the widgets
	map<string, Fl_Widget*> label_map_;

};


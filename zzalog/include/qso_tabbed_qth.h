#pragma once

#include <string>
#include <map>

using namespace std;

#include <FL/Fl_Tabs.H>
class qso_tabbed_qth :
    public Fl_Tabs
{
public:
	qso_tabbed_qth(int X, int Y, int W, int H, const char* l);
	~qso_tabbed_qth();

	// get settings 
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable widgets
	void enable_widgets();
	// Save changes
	void save_values();

	void switch_qth();

protected:

	// Callback
	static void cb_tabs(Fl_Widget* w, void* v);
	//// Redesign tab_positions
	//virtual int tab_positions();

	// Map the labels to the widgets
	map<string, Fl_Widget*> label_map_;

};


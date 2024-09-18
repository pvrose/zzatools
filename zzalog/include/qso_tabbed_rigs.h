#pragma once

#include <map>
#include <string>

#include <FL/Fl_Tabs.H>

using namespace std;

class rig_if;

// A tabbed display of qso_rig showing all the rigs recently refered to in QSOs
// Initially only rigs actively connectable are shown, but if a QSO refers to another
// that will get shown
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
	// Get the rig
	rig_if* rig();

	// Deactivate all rigs
	void deactivate_rigs();

protected:

	// Callback
	static void cb_tabs(Fl_Widget* w, void* v);

	// Map the labels to the widgets
	map<string, Fl_Widget*> label_map_;

	// Defauklt tab
	int default_tab_;
};


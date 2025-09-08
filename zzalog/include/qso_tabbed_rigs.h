#pragma once

#include <map>
#include <string>

#include <FL/Fl_Tabs.H>

using namespace std;

class rig_if;

//! This class presents a set of tabs indicating the usable rigs.

//! Initially only rigs actively connectable are shown, but if a QSO refers to another
//! that will tehn get shown.
class qso_tabbed_rigs :
    public Fl_Tabs
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_tabbed_rigs(int X, int Y, int W, int H, const char* L);
	//! Destructor.
	~qso_tabbed_rigs();

	//! Get active rigs and previous values from settings.
	void load_values();
	//! Instatntiate component widgets. 
	void create_form(int X, int Y);
	//! Configure component widgets after data change.
	void enable_widgets();
	//! Save configuration back to settings.
	void save_values();
	//! Connect/Disconnecy the active rig.
	void switch_rig();
	//! Returns the active rig interface.
	rig_if* rig();

	//! Deactivate all rigs
	void deactivate_rigs();

protected:

	//! Callback from switvhing tabs.
	static void cb_tabs(Fl_Widget* w, void* v);

	//! Map the labels to the widgets
	map<string, Fl_Widget*> label_map_;

	//! Default tab
	int default_tab_;
};


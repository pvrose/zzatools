#pragma once

#include "record.h"
#include "qso_qth.h"

#include <FL/Fl_Tabs.H>
class qso_misc :
    public Fl_Tabs
{
public:
    qso_misc(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_misc();

	// get settings
	void load_values();
	// Create form
	void create_form();
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

	void qso(record* qso);

protected:
	// The QSO to base all displays on
	record* qso_;

	// Widgets
	qso_qth* qth_;

};


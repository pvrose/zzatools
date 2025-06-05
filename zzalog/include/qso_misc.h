#pragma once

#include <FL/Fl_Tabs.H>

class record;
class qso_details;
class qso_dxcc;
class qso_qsl_vwr;
class qso_contest;
typedef size_t qso_num_t;

// This displays a set of tabs for qso_qth, qso_details, qso_dxcc and qso_qsl_vwr
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

	// Set the QSO and QSO number to all objects
	void qso(record* qso, qso_num_t qso_number);
	// Return the contest dialog
	qso_contest* contest();

protected:
	// callback when t he tab is changed
	static void cb_tabs(Fl_Widget* w, void* v);

	// The QSO to base all displays on
	record* qso_;

	// Widgets
	qso_details* details_;
	qso_dxcc* dxcc_;
	qso_qsl_vwr* qsl_;
	qso_contest* contest_;

	// Used to reopen the tab that was opened when the app was shut down
	int default_tab_;
};


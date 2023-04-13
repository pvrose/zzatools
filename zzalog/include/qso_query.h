#pragma once

#include "record_table.h"

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class qso_data;

class qso_query :
    public Fl_Group
{
public:
	qso_query(int X, int Y, int W, int H, const char* l = nullptr);
	~qso_query();

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();
	// Set query message
	void set_message(string message);


protected:
	// Record table
	static void cb_tab_qso(Fl_Widget* w, void* v);

	// Query message
	string query_message_;

	// Record table
	record_table* tab_query_;
	// Parent
	qso_data* qso_data_;

};


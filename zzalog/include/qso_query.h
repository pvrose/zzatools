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
	// Set QSOs
	void set_query(string message, qso_num_t log_number, record* query_qso = nullptr, bool save_original = true);
	// Return QSO
	record* qso();
	// Return Query QSO
	record* query_qso();
	// Return record number
	qso_num_t qso_number();
	// Clear query
	void clear_query();
	// Get query message
	string query_message();

	// 1 second ticker
	void ticker_1s();


protected:
	// Record table
	static void cb_tab_qso(Fl_Widget* w, void* v);
	// Action handle d-click
	void action_handle_dclick(int col, string field);

	// Query message
	string query_message_;

	// Record table
	record_table* tab_query_;
	// Parent
	qso_data* qso_data_;
	// QSOs
	record* log_qso_;
	qso_num_t log_number_;
	record* query_qso_;
	record* original_qso_;

};


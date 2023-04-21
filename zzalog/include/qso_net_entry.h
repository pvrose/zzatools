#pragma once

#include "record.h"

#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>

using namespace std;

class qso_net_entry : 
	public Fl_Group
{
public:
    qso_net_entry(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_net_entry();

	// Call back for entries
	static void cb_entries(Fl_Widget* w, void* v);

	// get settings
	void load_values();
	// Create form
	void create_form(int X, int Y);
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();
	// Return QSO
	record* qso();
	// Return record number
	qso_num_t qso_number();
	// Return original QSO
	record* original_qso();
	// Add QSO
	void add_entry();
	// Save and remove selected qso
	void remove_entry();
	// Set the QSO
	void set_qso(qso_num_t qso_number);
	// Copy fields from record
	void copy_qso_to_display(int flags);
	// QSO number in net
	bool qso_in_net(qso_num_t qso_number);
	// Select QSO number
	void select_qso(qso_num_t qso_number);
	// Append the created QSO to the book
	void append_qso();
	// Ticker
	void ticker();
	// SElected entry
	Fl_Widget* entry();
	// Number of entries
	int entries();

protected:
	Fl_Tabs* entries_;

	


protected:

};


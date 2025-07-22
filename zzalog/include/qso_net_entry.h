#pragma once

#include <FL/Fl_Group.H>

using namespace std;

enum navigate_t : uchar;
typedef size_t qso_num_t;
class record;
class qso_entry;
class Fl_Tabs;

// This provides a tabbed collection of qso_entry used when contacting more than 
// 1 station at once
class qso_net_entry : 
	public Fl_Group
{
public:
    qso_net_entry(int X, int Y, int W, int H, const char* L = nullptr);
    ~qso_net_entry();

	virtual int handle(int event);

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
	// SElected entry
	qso_entry* entry();
	// Most recent entry
	qso_entry* last_entry();
	// First entry
	qso_entry* first_entry();
	// Set entry
	void entry(qso_entry* w);
	// Number of entries
	int entries();
	// Navigate entry
	void navigate(navigate_t target);
	// Ask if can navigate
	bool can_navigate(navigate_t target);
	// Set focus on the call of the displayed entry
	void set_focus_call();

protected:
	// The tabbed qso_entry
	Fl_Tabs* entries_;

	


protected:

};


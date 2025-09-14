#pragma once

#include <FL/Fl_Group.H>



enum navigate_t : uchar;
typedef size_t qso_num_t;
class record;
class qso_entry;
class Fl_Tabs;

//! This class provides a tabbed collection of qso_entry widgets

//! used when contacting more than 1 station at once
class qso_net_entry : 
	public Fl_Group
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_net_entry(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
    ~qso_net_entry();

	//! Callback when selecting a tab and therefore changing current QSO record.
	static void cb_entries(Fl_Widget* w, void* v);

	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets after data changes.
	void enable_widgets();
	//! Returns the current QSO record.
	record* qso();
	//! Returns index of current QSO record
	qso_num_t qso_number();
	//! Returns a copy of the original QSO record. 
	record* original_qso();
	//! Add a QSO record to the net.
	void add_entry();
	//! Save and remove a QSO record from the net.
	void remove_entry();
	//! Set the QSO with index \p qso_number as the QSO record for the selected tab. 
	void set_qso(qso_num_t qso_number);
	//! Update the \p fields of the current QSO record in the display.
	void copy_qso_to_display(int flags);
	//! Returns true if the QSO indexed by \p qso_number is in the net.
	bool qso_in_net(qso_num_t qso_number);
	//! Set the QSO with the index \p qso_number as the current QSO.
	void select_qso(qso_num_t qso_number);
	//! Append the created QSO to the book
	void append_qso();
	//! Returns the selected qso_entry instance.
	qso_entry* entry();
	//! Returns the last entry added to the net.
	qso_entry* last_entry();
	//! Returns the first entry added to the net.
	qso_entry* first_entry();
	//! Set the qso_entry \p w as the selected entry.
	void entry(qso_entry* w);
	//! Returns the number of entries
	int entries();
	//! Set the selected entry as indicated by the navigate_t \p target.
	void navigate(navigate_t target);
	//! Returns true if it is possible to navigate to \p target.
	
	//! This is used to indicate that, for example, the user can't navigate to
	//! the previous entry if this is the first entry.
	bool can_navigate(navigate_t target);
	//! Set focus on the call of the displayed entry
	void set_focus_call();

protected:
	//! The tabbed qso_entry widgets.
	Fl_Tabs* entries_;

	


protected:

};


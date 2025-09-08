#pragma once

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class qso_data;
class record;
class record_table;
typedef size_t qso_num_t;

//! This class presents a query for the user to resolve.

//! It will display 1, 2 or 3 records depending on query to enable the
//! user to select one or the other or merge data from any.
class qso_query :
    public Fl_Group
{
public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_query(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
	~qso_query();

	//! Inherited from Fl_Grou::handle to allow keyboard F1 to open userguide.
	virtual int handle(int event);

	//! Instantiate component widgets.
	void create_form(int X, int Y);
	//! Configure component widgets after data change. 
	void enable_widgets();
	//! Set the query.
	
	//! \param message Instructions to user.
	//! \param log_number Index of QSO record in full log to check against.
	//! \param query_qso QSO record containing queried QSO.
	//! \param save_original Keep a copy of the original record so that changes can be undone if necessary.
	void set_query(string message, qso_num_t log_number, record* query_qso = nullptr, bool save_original = true);
	//! Set query to merge with new QSO. \see set_query.
	
	//! \param message Instructions to user.
	//! \param new_qso New QSO to check against.
	//! \param query_qso QSO record containing queried QSO.
	//! \param save_original Keep a copy of the original record so that changes can be undone if necessary.
	void set_query(string message, record* new_qso, record* query_qso = nullptr, bool save_original = true);
	//! Returns the logged QSO record.
	record* qso();
	//! Returns the queried QSO record.
	record* query_qso();
	//! Returns the index of the logged qSO record.
	qso_num_t qso_number();
	//! Clear the query.
	void clear_query();
	//! Returns the messae to the user.
	string query_message();

protected:
	//! Callback from clicking the query table.
	static void cb_tab_qso(Fl_Widget* w, void* v);
	//! Handle a double-click action in cb_tab_qso: usually updates logged record from query or original.
	void action_handle_dclick(int col, string field);

	//! Query message
	string query_message_;

	//! Record table
	record_table* tab_query_;
	//! Parent
	qso_data* qso_data_;
	//! Logged QSO record.
	record* log_qso_;
	//! Index of logged QSO record.
	qso_num_t log_number_;
	//! Query record.
	record* query_qso_;
	//! Copy of original logged record.
	record* original_qso_;

};


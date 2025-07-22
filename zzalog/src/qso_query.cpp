#include "qso_query.h"
#include "drawing.h"
#include "record_table.h"
#include "qso_data.h"
#include "book.h"
#include "fields.h"
#include "utils.h"
#include "record.h"

extern book* book_;
extern fields* fields_;
extern void open_html(const char*);

/** \page qso_query Querying QSOs

\section description Description

The QSO Query pane provides a means for checking imported records against
a record in the log. 

\section examples Examples

The first example is from the duplicate record check. Two records have been added to the 
query view. They 
might be duplicates. They probably are as the two records only differ in the one 
callsign is the /P version of the other and most other fields agree. The control buttons
offer the choice for keeping one, the other or both as well as merging the two records.

<IMG SRC="../images/qso_query_1.png">

The second example is from a putative import of records from an on-line QSL site.
The imported record appears not to match a record in the log.
In this case, no potential match was found, so only the imported records is added
to the query. The control buttons have changed to offer 
a choice of accepting or rejecting the QSO as well as a number of search functions.

<IMG SRC="../images/qso_query_2.png">

The third example is the result of clicking the "search log" button. Now it has found a
record that differs in a few fields (but a critical one - CALL, the other station's
callsign). Now three records have been added to the query. In the first column, the 
potential match found in the log. In the second column, the import record. The third 
column contains a copy of the record from the log.

<IMG SRC="../images/qso_query_3.png">

On the right hand of the pane is a scroll bar. This enables the user to look at all the
fields in the records. As well as the CALL fields differing another is seen to do so.
Highlighted below is the field EQSL_QSL_RDATE - a housekeeping field for QSL checking.

<IMG SRC="../images/qso_query_4.png">

A feature of this use of the QSO Query pane is the ability to move data from one record
to another. In this case, double-clicking on the field value in the import record
has copied the value to the log record.

<IMG SRC="../images/qso_query_5.png">

Note that it will be possible to restore the original log entry value for this field
by double-clicking on the entry in the third column.

*/

// Constrctor
qso_query::qso_query(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L),
	qso_data_((qso_data*)parent()),
	log_qso_(nullptr),
	log_number_(-1),
	query_message_(""),
	query_qso_(nullptr)
{
	create_form(X, Y);
	load_values();
	enable_widgets();
}

// Destructor
qso_query::~qso_query() {
	save_values();
}

// Handle
int qso_query::handle(int event) {
	int result = Fl_Group::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("qso_query.html");
			return true;
		}
		break;
	}
	return result;
}

// Load Settings
void qso_query::load_values() {
}

// Sabe settings
void qso_query::save_values() {
}

// Create the widgets
void qso_query::create_form(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);

	const int WTABLE = w() - GAP - GAP;
	const int HTABLE = h() - HTEXT - GAP;

	curr_x += GAP;
	curr_y += HTEXT;

	// Tabular display to allow all fields in the record to be displayed
	tab_query_ = new record_table(curr_x, curr_y, WTABLE, HTABLE);
	tab_query_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	tab_query_->callback(cb_tab_qso, nullptr);

	resizable(nullptr);
	end();
}

// Configure the widgets
void qso_query::enable_widgets() {
	switch (qso_data_->logging_state()) {
	case qso_data::QSO_BROWSE:
		tab_query_->activate();
		tab_query_->set_records(log_qso_, query_qso_, original_qso_);
		tab_query_->redraw();
		break;
	case qso_data::QUERY_DUPE:
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW:
	case qso_data::QUERY_WSJTX:
	case qso_data::QRZ_MERGE:
	case qso_data::QRZ_COPY:
		tab_query_->activate();
		tab_query_->set_records(log_qso_, query_qso_, original_qso_);
		tab_query_->redraw();
		break;
	default:
		hide();
	}
}

// Return query message
string qso_query::query_message() {
	return query_message_;
}

// Callback - table 
// v is not used
void qso_query::cb_tab_qso(Fl_Widget* w, void* v) {
	qso_query* that = ancestor_view<qso_query>(w);
	record_table* table = (record_table*)w;
	int row = table->callback_row();
	int col = table->callback_col();
	int button = Fl::event_button();
	bool double_click = Fl::event_clicks();
	string field = table->field(row);
	// Handle double left click in both the row header and each column 
	switch (table->callback_context()) {
	case Fl_Table::CONTEXT_ROW_HEADER:
		if (button == FL_LEFT_MOUSE && double_click) {
			that->action_handle_dclick(0, field);
		}
		break;
	case Fl_Table::CONTEXT_CELL:
		if (button == FL_LEFT_MOUSE && double_click) {
			that->action_handle_dclick(col, field);
		}
		break;
	default:
		break;
	}
}

//Set QSOs into query
void qso_query::set_query(string message, qso_num_t log_number, record* query_qso, bool save_original) {
	query_message_ = message;
	log_number_ = log_number;
	if (log_number != -1) {
		log_qso_ = book_->get_record(book_->item_number(log_number_), false);
		if (save_original) {
			if (log_qso_) original_qso_ = new record(*log_qso_);
			else original_qso_ = nullptr;
		}
		else original_qso_ = nullptr;
	}
	else {
		log_qso_ = nullptr;
		original_qso_ = nullptr;
	}
	query_qso_ = query_qso;
}

//Set QSOs into query
void qso_query::set_query(string message, record* new_qso, record* query_qso, bool save_original) {
	query_message_ = message;
	log_number_ = -1;
	log_qso_ = new_qso;
	if (save_original) {
		if (log_qso_) original_qso_ = new record(*log_qso_);
		else original_qso_ = nullptr;
	}
	else original_qso_ = nullptr;
	query_qso_ = query_qso;
}

// Get QSO
record* qso_query::qso() {
	return log_qso_;
}

// Get QSO number
qso_num_t qso_query::qso_number() {
	return log_number_;
}

// Return Query QSO
record* qso_query::query_qso() {
	return query_qso_;
}

// Clear query
void qso_query::clear_query() {
	query_message_ = "";
	log_number_ = -1;
	log_qso_ = nullptr;
	delete original_qso_;
	original_qso_ = nullptr;
	query_qso_ = nullptr;
}

// Action table double click
void qso_query::action_handle_dclick(int col, string field) {
	switch (qso_data_->logging_state()) {
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_DUPE:
	case qso_data::QRZ_MERGE:
	case qso_data::QRZ_COPY:
		switch (col) {
		case 0:
			//// Treat as if clicking row header
			//g_entry_->action_add_field(-1, field);
			break;
		case 1:
			// Copy log field
			log_qso_->item(field, query_qso_->item(field));
			break;
		case 2:
			// Copy original record
			log_qso_->item(field, original_qso_->item(field));
			break;
		}
	default:
		break;
	}
	enable_widgets();
}

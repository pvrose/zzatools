#include "qso_query.h"
#include "drawing.h"
#include "record_table.h"
#include "qso_data.h"
#include "book.h"

extern book* book_;

Fl_Color label_colour = fl_darker(FL_RED);

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

qso_query::~qso_query() {
	save_values();
}

void qso_query::load_values() {
}

void qso_query::save_values() {
}

void qso_query::create_form(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	labelcolor(label_colour);

	const int WTABLE = 420;
	const int HTABLE = 250;

	curr_x += GAP;
	curr_y += HTEXT;

	tab_query_ = new record_table(curr_x, curr_y, WTABLE, HTABLE);
	tab_query_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	tab_query_->callback(cb_tab_qso, nullptr);

	curr_x += WTABLE + GAP;
	curr_y += HTABLE + GAP;

	resizable(nullptr);
	size(curr_x - X, curr_y - Y);
	end();
}

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
void qso_query::cb_tab_qso(Fl_Widget* w, void* v) {
	qso_query* that = ancestor_view<qso_query>(w);
	record_table* table = (record_table*)w;
	int row = table->callback_row();
	int col = table->callback_col();
	int button = Fl::event_button();
	bool double_click = Fl::event_clicks();
	string field = table->field(row);
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
	}
}

//Set QSOs
void qso_query::set_query(string message, qso_num_t log_number, record* query_qso, bool save_original) {
	query_message_ = message;
	log_number_ = log_number;
	if (log_number != -1) {
		log_qso_ = book_->get_record(book_->item_number(log_number_), false);
		if (save_original) original_qso_ = new record(*log_qso_);
		else original_qso_ = nullptr;
	}
	else {
		log_qso_ = nullptr;
		original_qso_ = nullptr;
	}
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
	}
	enable_widgets();
}

// 1s ticker
void qso_query::ticker_1s() {
	if (labelcolor() == FL_BACKGROUND_COLOR) {
		labelcolor(label_colour);
	} else {
		labelcolor(FL_BACKGROUND_COLOR);
	}
	redraw();
}
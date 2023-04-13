#include "qso_query.h"
#include "drawing.h"
#include "record_table.h"
#include "qso_data.h"

// Constrctor
qso_query::qso_query(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L),
	qso_data_((qso_data*)parent())
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
	labelcolor(fl_darker(FL_RED));

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
	char l[128];
	switch (qso_data_->logging_state()) {
	case qso_data::QSO_BROWSE:
		snprintf(l, sizeof(l), "QSO Query - %s - %s", qso_data_->current_qso()->item("CALL").c_str(), query_message_.c_str());
		copy_label(l);
		show();
		tab_query_->activate();
		tab_query_->set_records(qso_data_->current_qso(), qso_data_->query_qso(), qso_data_->original_qso());
		break;
	case qso_data::QUERY_DUPE:
	case qso_data::QUERY_MATCH:
	case qso_data::QUERY_NEW:
		snprintf(l, sizeof(l), "QSO Query - %s - %s", qso_data_->query_qso()->item("CALL").c_str(), query_message_.c_str());
		copy_label(l);
		show();
		tab_query_->activate();
		tab_query_->set_records(qso_data_->current_qso(), qso_data_->query_qso(), qso_data_->original_qso());
		break;
	default:
		hide();
	}
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
			that->qso_data_->action_handle_dclick(0, field);
		}
		break;
	case Fl_Table::CONTEXT_CELL:
		if (button == FL_LEFT_MOUSE && double_click) {
			that->qso_data_->action_handle_dclick(col, field);
		}
		break;
	}
}

void qso_query::set_message(string message) {
	query_message_ = message;
}

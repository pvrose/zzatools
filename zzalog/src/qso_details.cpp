#include "qso_details.h"
#include "qso_entry.h"
#include "drawing.h"
#include "book.h"
#include "status.h"

extern book* book_;
extern status* status_;

qso_details::qso_details(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, callsign_("")
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	create_form();
	enable_widgets();
}

qso_details::~qso_details() {}

void qso_details::create_form() {
	int avail_width = w() - GAP - GAP;
	int avail_height = h() - GAP;
	int curr_x = x() + GAP;
	int curr_y = y() + 1;
	
	// "Title" is callsign
	op_call_ = new Fl_Output(curr_x, curr_y, avail_width, HBUTTON);
	op_call_->box(FL_FLAT_BOX);
	op_call_->color(FL_BACKGROUND_COLOR);
	op_call_->textfont(FL_BOLD);
	op_call_->textsize(FL_NORMAL_SIZE + 2);
	op_call_->textcolor(COLOUR_CLARET);

	curr_y += op_call_->h() + GAP;
	// Add table
	table_details_ = new table_d(curr_x, curr_y, avail_width, 5 * ROW_HEIGHT);

	curr_y += table_details_->h() + GAP;

	table_qsos_ = new table_q(curr_x, curr_y, avail_width, y() + avail_height - curr_y);

	end();
	enable_widgets();
}

void qso_details::enable_widgets() {
	op_call_->value(callsign_.c_str());
	get_qsos();
}

void qso_details::get_qsos() {
	set<string> names;
	set<string> qths;
	set<string> locators;
	set<record*>* records = new set<record*>;
	qso_entry* qe = ancestor_view<qso_entry>(this);
	record* qso = qe->qso();
	for (auto it = book_->begin(); it != book_->end(); it++) {
		if ((*it)->item("CALL") == callsign_) {
			string name = (*it)->item("NAME");
			if (name.length()) {
				names.insert(name);
			}
			string qth = (*it)->item("QTH");
			if (qth.length()) {
				qths.insert(qth);
			}
			string locator = (*it)->item("GRIDSQUARE");
			if (locator.length()) {
				locators.insert(locator);
			}
			if (qso->timestamp() != (*it)->timestamp()) {
				records->insert((*it));
			}
		}
	}
	table_details_->set_data(names, qths, locators);
	table_qsos_->set_data(records);
}

void qso_details::set_call(string callsign) {
	callsign_ = callsign;
	enable_widgets();
}

qso_details::table_d::table_d(int X, int Y, int W, int H, const char* L) :
	Fl_Table(X, Y, W, H, L)
{
	row_header(true);
	cols(1);
	col_width(0, tiw);
	end();
	callback(cb_table);
	when(FL_WHEN_RELEASE);
}

qso_details::table_d::~table_d() {
}

void qso_details::table_d::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
	string text;

	switch (context) {

	case CONTEXT_STARTPAGE:
		// Set the table font
		fl_font(0, FL_NORMAL_SIZE);
		return;

	case CONTEXT_ENDPAGE:
		// Do nothing
		return;

	case CONTEXT_ROW_HEADER:
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
			fl_color(active_r() ? FL_BLACK : fl_inactive(FL_BLACK));
			fl_font(FL_ITALIC | FL_BOLD, FL_NORMAL_SIZE);
			const char* heading = name_map_.at(items_[R].type).heading;
			if (R == 0) fl_draw(heading, X, Y, W, H, FL_ALIGN_LEFT);
			else if (items_[R].type != items_[R-1].type) fl_draw(heading, X, Y, W, H, FL_ALIGN_LEFT);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Column indicates which record, R the field
		fl_push_clip(X, Y, W, H);
		{
			// Get the details of the item
			qso_entry* qe = ancestor_view<qso_entry>(this);
			record* qso = qe->qso();
			string s_value = "";
			string field = name_map_.at(items_[R].type).field;
			text = items_[R].value;
			bool used = false;
			if (qso) {
				s_value = qso->item(field);
				if (s_value.length() && s_value == text) used = true;
			}
			Fl_Color bg_colour = used ? fl_lighter(COLOUR_CLARET) : FL_BACKGROUND_COLOR;
			if (!active_r()) bg_colour = fl_inactive(bg_colour);
			// BOX
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, bg_colour);
			// TEXT
			Fl_Color fg_colour = fl_contrast(FL_BLACK, bg_colour);
			if (!active_r()) fg_colour = fl_inactive(fg_colour);
			fl_color(fg_colour);
			fl_font(0, FL_NORMAL_SIZE);
			fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

			// BORDER
			fl_color(FL_LIGHT1);
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
			fl_pop_clip();
			return;
		}
	}
}

void qso_details::table_d::set_data(set<string>names, set < string>qths, set<string> locators) {
	items_.clear();
	for (auto it = names.begin(); it != names.end(); it++) {
		items_.push_back({ NAME, (*it) });
	}
	if (names.size() == 0) items_.push_back({ NAME, "" });
	for (auto it = qths.begin(); it != qths.end(); it++) {
		items_.push_back({ QTH, (*it) });
	}
	if (qths.size() == 0) items_.push_back({ QTH, "" });
	for (auto it = locators.begin(); it != locators.end(); it++) {
		items_.push_back({ LOCATOR, (*it) });
	}
	if (locators.size() == 0) items_.push_back({ LOCATOR, "" });
	cols(1);
	rows(items_.size());
	row_height_all(ROW_HEIGHT);
	redraw();
}

// Table callback 
// Set the selected value in the appropriate record item
void qso_details::table_d::cb_table(Fl_Widget* w, void* v) {
	table_d* that = ancestor_view<table_d>(w);
	if (that->callback_context() == CONTEXT_CELL) {
		int row = that->callback_row();
		string field = that->name_map_.at(that->items_[row].type).field;
		string value = that->items_[row].value;
		qso_entry* qe = ancestor_view<qso_entry>(that);
		record* qso = qe->qso();
		qso->item(field, value);
		book_->selection(-1, HT_CHANGED);
	}
}

qso_details::table_q::table_q(int X, int Y, int W, int H, const char* L) :
	Fl_Table(X, Y, W, H, L)
{
	row_header(false);
	col_header(true);
	cols(3);
	col_width(0, tiw / 2 );
	col_width(1, tiw / 4);
	col_width(2, tiw / 4);
	end();
}

qso_details::table_q::~table_q() {
}

void qso_details::table_q::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
	string text;

	switch (context) {

	case CONTEXT_STARTPAGE:
		// Set the table font
		fl_font(0, FL_NORMAL_SIZE);
		return;

	case CONTEXT_ENDPAGE:
		// Do nothing
		return;

	case CONTEXT_COL_HEADER:
	{
		fl_push_clip(X, Y, W, H);
		switch (C) {
		case 0:
			text = "QSO_DATE";
			break;
		case 1:
			text = "BAND";
			break;
		case 2:
			text = "MODE";
			break;
		}
		// TEXT
		Fl_Color fg_colour = FL_BLACK;
		if (!active_r()) fg_colour = fl_inactive(fg_colour);
		fl_color(fg_colour);
		fl_font(FL_BOLD | FL_ITALIC, FL_NORMAL_SIZE);
		fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);
		// BORDER
		fl_color(FL_LIGHT1);
		// draw top and right edges only
		fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
		fl_pop_clip();
		return;
	}
	case CONTEXT_CELL:
		// Column indicates which record, R the field
		fl_push_clip(X, Y, W, H);
		{
			// Get the details of the item
			qso_entry* qe = ancestor_view<qso_entry>(this);
			record* qso = qe->qso();
			string s_value = "";
			string field;
			switch (C) {
			case 0:
				field = "QSO_DATE";
				break;
			case 1:
				field = "BAND";
				break;
			case 2:
				field = "MODE";
				break;
			}
			text = items_[R]->item(field);
			bool used = false;
			bool same_call = false;
			if (qso) {
				s_value = qso->item(field);
				if (s_value.length() && s_value == text) used = true;
				if (qso->item("STATION_CALLSIGN") == items_[R]->item("STATION_CALLSIGN")) same_call = true;
			}
			Fl_Color bg_colour = used ? (same_call ? fl_lighter(COLOUR_CLARET) : COLOUR_APPLE) : FL_BACKGROUND_COLOR;
			if (!active_r()) bg_colour = fl_inactive(bg_colour);
			// BOX
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, bg_colour);
			// TEXT
			Fl_Color fg_colour = fl_contrast(FL_BLACK, bg_colour);
			if (!active_r()) fg_colour = fl_inactive(fg_colour);
			fl_color(fg_colour);
			fl_font(0, FL_NORMAL_SIZE);
			fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

			// BORDER
			fl_color(FL_LIGHT1);
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
			fl_pop_clip();
			return;
		}
	}
}

void qso_details::table_q::set_data(set<record*>* items) {
	items_.clear();
	for (auto it = items->begin(); it != items->end(); it++) {
		items_.insert(items_.begin(), (*it));
	}
	rows(items_.size());
	row_height_all(ROW_HEIGHT);
	redraw();
}

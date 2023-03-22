#include "pfx_dialog.h"

#include "menu.h"
#include "utils.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>




// Table constructor
pfx_dlg_table::pfx_dlg_table(pfx_dialog* parent, int X, int Y, int W, int H, const char* label) 
: Fl_Table_Row(X, Y, W, H, label)
{
	parent_ = parent;
	last_event_ = 0;
	last_button_ = 0;
	last_clicks_ = 0;
	// Set the various fixed attributes of the table
	type(SELECT_SINGLE);
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	col_header_height(ROW_HEIGHT);
	selection_color(FL_YELLOW);
	when(FL_WHEN_RELEASE);
	end();
}

// Destructor
pfx_dlg_table::~pfx_dlg_table() {}

// Overloaded to provide cell contents and formatting
void pfx_dlg_table::draw_cell(TableContext context, int R, int C, int X, int Y,	int W, int H) {
	string text;
	switch (context) {
	case CONTEXT_STARTPAGE:
		// Set the table font
		fl_font(0, FL_NORMAL_SIZE);
		return;

	case CONTEXT_ENDPAGE:
		return;

	case CONTEXT_ROW_HEADER:
		// Put row number into header
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, row_header_color());
			fl_color(FL_BLACK);
			text = to_string(R);
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_LEFT);
		}
		fl_pop_clip();
		return;

	case CONTEXT_COL_HEADER:
		// put column header text into header
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, col_header_color());
			fl_color(FL_BLACK);
			// text is column header
			switch (C) {
			case 0: text = "Entity";
				break;
			case 1: text = "Name";
				break;
			case 2: text = "Sub-entity";
				break;
			case 3: text = "Name";
			}
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Get content from record R, 
		fl_push_clip(X, Y, W, H);
		{
			// BG COLOR
			fl_color(row_selected(R) ? selection_color() : FL_WHITE);
			fl_rectf(X, Y, W, H);

			// TEXT
			// Get the prefix to be displayed in the row and its root prefix
			prefix* pfx = parent_->get_prefix(R);
			prefix* root_prefix = pfx;
			while (root_prefix->parent_ != 0) {
				root_prefix = root_prefix->parent_;
			}
			// Get the particular field for each column
			switch (C) {
			case 0:
				// entity nickname
				text = root_prefix->nickname_;
				break;
			case 1:
				// entity name
				text = root_prefix->name_;
				break;
			case 2:
				// sub-entity nickname_
				text = pfx->nickname_;
				break;
			case 3:
				// sub-entity name
				text = pfx->name_;
				break;
			}
			fl_color(FL_BLACK);
			fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

			// BORDER
			fl_color(FL_LIGHT1);
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
		}
		fl_pop_clip();
		return;
	}

}

const int PFXD_ROWS = 4;

// Dialog constructor
pfx_dialog::pfx_dialog() :
	win_dialog(500, 200, "Prefix Selection")
	, button_(BN_OK)
	, selection_(-1)
{
	prefixes_ = nullptr;
	rep_widgets_.clear();

	// output field to display callsign
	int curr_x = EDGE;
	int curr_y = EDGE;
	call_out_ = new Fl_Output(curr_x + WLABEL, curr_y, 100, HTEXT, "Call");

	// table to list the prefixes to be chosen from
	curr_y += HTEXT + GAP;
	table_ = new pfx_dlg_table(this, curr_x, curr_y, 400, (PFXD_ROWS + 2) * (ROW_HEIGHT));
	table_->callback((Fl_Callback*)pfx_dialog::cb_tab_pfx, (void*)this);

	int wall = table_->x() + table_->w() + EDGE;

	// button "QRZ.com"
	curr_y += table_->h() + GAP;
	Fl_Button* qrz_bn = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "QRZ.com");
	qrz_bn->callback(cb_bn_qrz);
	qrz_bn->align(FL_ALIGN_INSIDE);
	rep_widgets_.push_back(qrz_bn);

	// button "OK" 
	curr_x += WBUTTON;
	Fl_Button* ok_bn = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "OK");
	ok_bn->callback(cb_bn_ok);
	ok_bn->align(FL_ALIGN_INSIDE);
	ok_bn->tooltip("Accept selected prefix");
	rep_widgets_.push_back(ok_bn);

	// button "Cancel" 
	curr_x += WBUTTON;
	Fl_Button* cancel_bn = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Cancel");
	cancel_bn->callback(cb_bn_cancel);
	cancel_bn->align(FL_ALIGN_INSIDE);
	cancel_bn->tooltip("Cancel parsing");
	rep_widgets_.push_back(cancel_bn);

	curr_y += HBUTTON + EDGE;

	// stop creating - only show it when called to
	end();
	pending_button_ = false;

	resizable(nullptr);
	size(table_->w() + XLEFT + GAP, curr_y);

	// Set callback for close button
	callback(cb_bn_cancel);
}

// Destructor
pfx_dialog::~pfx_dialog()
{
	// delete all child widgets and then the window
	rep_widgets_.clear();
	clear();
}

// Table clicked
void pfx_dialog::cb_tab_pfx(Fl_Widget* w, void* v) {
	// get pointers to the dialog and the calling control
	pfx_dialog* that = (pfx_dialog*)v;
	pfx_dlg_table* table = (pfx_dlg_table*)w;
	switch (table->callback_context()) {

	case Fl_Table::CONTEXT_CELL:
		// Clicked on a cell - remember the row 
		that->selection_ = table->callback_row();
		if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks()) {
			// double left click - treat as OK button on selected row.
			that->do_button(BN_OK);
		}
		break;
	}
}

// QRZ.com button clicked
void pfx_dialog::cb_bn_qrz(Fl_Widget* w, void* v) {
	pfx_dialog* that = ancestor_view<pfx_dialog>(w);
	menu::cb_mi_info_qrz(w, (void*)&that->callsign_);
}

// OK Button clicked
void pfx_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	pfx_dialog* that = ancestor_view<pfx_dialog>(w);
	that->do_button(BN_OK);
}

void pfx_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	pfx_dialog* that = ancestor_view<pfx_dialog>(w);
	that->do_button(BN_CANCEL);
}

// Set the list of possible options
void pfx_dialog::set_data(vector<prefix*>* prefixes, string callsign) {
	// Delete current table contents
	table_->clear();
	// Remove any existing prefixes
	if (this->prefixes_ != nullptr) this->prefixes_->clear();
	// Copy prefixes by reference
	this->prefixes_ = prefixes;
	// Set callsign into display 
	callsign_ = callsign;
	call_out_->value(callsign.c_str());
	// Set table row parameters
	table_->rows(this->prefixes_->size());
	table_->row_height_all(ROW_HEIGHT);
	table_->cols(4);
	// Adjust height to include all rows
	int num_rows = table_->rows();
	int diff_height = ((table_->rows() - PFXD_ROWS) * ROW_HEIGHT);
	table_->size(table_->w(), table_->h() + diff_height);
	// Reposition all widgets below table_
	for (auto it = rep_widgets_.begin(); it != rep_widgets_.end(); it++) {
		(*it)->position((*it)->x(), (*it)->y() + diff_height);
	}
	size(w(), h() + diff_height);
	redraw();
}

// Get the selected prefix
prefix* pfx_dialog::get_prefix() {
	if (selection_ < 0 || selection_ >= prefixes_->size()) {
		return nullptr;
	}
	else {
		return (*prefixes_)[selection_];
	}
}

// Get specific prefix
prefix* pfx_dialog::get_prefix(size_t prefix_num) {
	if (prefix_num < prefixes_->size()) {
		return (*prefixes_)[prefix_num];
	}
	else {
		return nullptr;
	}
}

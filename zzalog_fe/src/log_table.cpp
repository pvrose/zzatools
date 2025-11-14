
#include "log_table.h"

#include "book.h"
#include "cty_data.h"
#include "extract_data.h"
#include "field_choice.h"
#include "fields.h"
#include "intl_dialog.h"
#include "intl_widgets.h"
#include "main.h"
#include "main_window.h"
#include "menu.h"
#include "qso_manager.h"
#include "record.h"
#include "settings.h"
#include "spec_data.h"
#include "status.h"
#include "toolbar.h"
#include "utils.h"

#include "callback.h"

#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/fl_ask.H>

Fl_Font log_table::font_;
Fl_Fontsize log_table::fontsize_;

// constructor - passes parameters  to the two base classes
log_table::log_table(int X, int Y, int W, int H, const char* label, field_app_t app) :
  Fl_Table_Row(X, Y, W, H, label)
, view()
{
	log_fields_ = nullptr;
	current_item_num_ = 0;
	last_event_ = 0;
	last_button_ = 0;
	last_clicks_ = 0;
	last_rootx_ = 0;
	last_rooty_ = 0;
	application_ = FO_MAINLOG;
	rows_per_page_ = 0;
	order_ = FIRST_TO_LAST;
	tip_window_ = nullptr;
	tip_root_x_ = 0;
	tip_root_y_ = 0;

	// These are static, but will get to the same value each time
	settings top_settings;
	settings view_settings(&top_settings, "Views");
	settings log_settings(&view_settings, "Log Table");
	log_settings.get("Font Name", font_, (Fl_Font)0);
	log_settings.get("Font Size", fontsize_, FL_NORMAL_SIZE);
	begin();
	// Create cell input widget, zero size, hide it
	edit_input_ = new field_input(x() + w() / 2, y() + h() / 2, 100, 20);
	edit_input_->box(FL_DOWN_BOX);
	edit_input_->hide();
	edit_input_->callback(cb_input, nullptr);
	edit_input_->input()->when(FL_WHEN_RELEASE);
	edit_input_->textfont(font_);
	edit_input_->textsize(fontsize_);
	add(edit_input_);
	end();
	alt_gr_ = false;

	// Set the various fixed attributes of the table
	type(SELECT_SINGLE);
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(COLOUR_GREY);
	row_header(true);
	row_header_color(COLOUR_GREY);
	selection_color(FL_BLUE);
	fl_font(font_, fontsize_);
	int row_height = fl_height() + 2;
	adjust_row_sizes();
	when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	callback((Fl_Callback*)log_table::cb_tab_log, (void*)nullptr);
	application_ = app;
	// Get the fields to display
	get_fields();
	// std::set number of fields
	cols(log_fields_->size());
	// For each field std::set the column width
	for (unsigned int i = 0; i < log_fields_->size(); i++) {
		col_width(i, (*log_fields_)[i].width);
	}
	// number of rows - available internal height / default row height
	rows_per_page_ = Fl_Table_Row::tih / (row_height);
	// Reverse order
	log_settings.get("Recent First", order_, FIRST_TO_LAST);

	// Set minimum resizing - width 1/3
	min_w_ = w() / 3;
	// Set minimum resizing - height 6 rows (plus col. header)
	min_h_ = (row_height) * 7;
	resizable(this);
}

// Destructor
log_table::~log_table()
{
	clear();
	// The destructor for this uses itself
	edit_input_ = nullptr;
}

// callback - called when mouse button is released but also at other times.
void log_table::cb_tab_log(Fl_Widget* w, void* v) {
	// Get the item number and field clicked
	log_table* that = (log_table*)w;
	int row = that->callback_row();
	int col = that->callback_col();
#ifdef _WIN32
	// There seems to be differnet behaviour in Windows - retutns -1 if click outside the table
	if (row < 0) row = 0;
	if (col < 0) col = 0;
#endif
	item_num_t item_num;
	qso_num_t qso_number;
	if (that->my_book_) {
		item_num = that->order_ == LAST_TO_FIRST ? that->my_book_->size() - 1 - row : row;
		qso_number = that->my_book_->record_number(item_num);
	}
	else {
		item_num = -1;
		qso_number = -1;
	}
	if (col < 0) col = 0;
	item_num_t save_num = that->current_item_num_;
	switch (that->callback_context()) {
	case Fl_Table::CONTEXT_CELL:
		// Mouse clicked within the cell
		switch (Fl::event()) {
		case FL_RELEASE:
			switch (that->last_button_) {
			case FL_LEFT_MOUSE:
				// Select the item at the row that was clicked if it can be selected
				if (qso_manager_ && qso_manager_->editing() && qso_manager_->outwith_edit(qso_number)) {
					// It cannot be selected - keep original selection
					char msg[128];
					snprintf(msg, sizeof(msg), "LOG: Attempt to select record %zu when another record is being edited", qso_number);
					status_->misc_status(ST_WARNING, msg);
					// and select the row in the table
					int orig_row = that->order_ == LAST_TO_FIRST ? that->my_book_->size() - 1 - save_num : save_num;
					that->select_row(orig_row);
					that->redraw();
				}
				else {
					// Tidy up any edit in progress
					that->done_edit(false);
					// Left button - double click edit cell
					if (Fl::event_clicks()) {
						that->edit_cell(row, col);
					}
					else {
						that->my_book_->selection(item_num, HT_SELECTED, that);
					}
					that->current_item_num_ = item_num;
				}
			break;
			case FL_RIGHT_MOUSE:
				// Right button - display tooltip explaining the field
				that->done_edit(false);
				that->describe_cell(row, col);
				break;
			}
		break;
		case FL_PUSH:
			// Keep the focus
			that->take_focus();
			break;
		default:
			break;
		}
		break;
	case Fl_Table::CONTEXT_COL_HEADER:
		// Column header clicked - conditionally sorts the display
		that->done_edit(false);
		if (Fl::event() == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks()) {
			// Left button double click on column header
			that->dbl_click_column(col);
		}
		break;
	case Fl_Table::CONTEXT_RC_RESIZE:
		// Row or column resized
		that->done_edit(false);
		if (Fl::event() == FL_DRAG && Fl::event_button() == FL_LEFT_MOUSE && that->is_interactive_resize()) {
			// A row or column has been resized
			that->drag_column(col);
		}
		break;
	default:
		break;
	}
}

// Callback from the edit input - Enter key has been typed
void log_table::cb_input(Fl_Widget* w, void* v) {
	log_table* that = ancestor_view<log_table>(w);
	field_input* fi = (field_input*)w;
	that->edit_save(fi->reason());
}

// Copy the data from the edit input, and start a new edit input to the left, right, above or below
void log_table::edit_save(field_input::exit_reason_t exit_type) {
	// Deselect row being edited
	select_row(edit_row_, 0);
	switch (exit_type) {
	case field_input::exit_reason_t::IR_ENTER:
		done_edit(false);
		break;
	case field_input::exit_reason_t::IR_LEFT:
		// Save and edit previous column
		done_edit(true);
		if (edit_col_ > 0) {
			edit_cell(edit_row_, --edit_col_);
		}
		else {
			status_->misc_status(ST_WARNING, "LOG: There is no cell to the left to edit.");
		}
		break;
	case field_input::exit_reason_t::IR_RIGHT:
		// Save and edit next column
		done_edit(true);
		if (edit_col_ < log_fields_->size() - 1) {
			edit_cell(edit_row_, ++edit_col_);
		}
		else {
			status_->misc_status(ST_WARNING, "LOG: There is no cell to the right to edit.");
		}
		break;
	case field_input::exit_reason_t::IR_UP:
		// Save and edit same column in previous record
		done_edit(false);
		if (edit_row_ > 0) {
			edit_cell(--edit_row_, edit_col_);
		}
		else {
			status_->misc_status(ST_WARNING, "LOG: There is no cell upwards to edit.");
		}
		break;
	case field_input::exit_reason_t::IR_DOWN:
		// Save and edit smae column in next record
		done_edit(false);
		if (edit_row_ < my_book_->size() - 1) {
			edit_cell(++edit_row_, edit_col_);
		}
		else {
			status_->misc_status(ST_WARNING, "LOG: There is no cell downwards to edit.");
		}
		break;
	default:
		break;
	}
	// Select new row to edit
	select_row(edit_row_, 1);
	// get the visible rows - and move the current row to the top if it is outwith them
	// note this assumes the top and bottom rows may be partially hidden
	int r1 = 0, r2 = 0, c1 = 0, c2 = 0;
	visible_cells(r1, r2, c1, c2);
	if ((unsigned)r1 >= edit_row_ || (unsigned)r2 <= edit_row_) {
		top_row(edit_row_);
	}
}

// event handler - remember the event and call widget's handle
int log_table::handle(int event) {
	last_event_ = event;
	last_rootx_ = Fl::event_x_root();
	last_rooty_ = Fl::event_y_root();
	last_button_ = Fl::event_button();
	last_clicks_ = Fl::event_clicks();
	int real_event;
	// Some keyboards share arrow keys and Home/End/PgUp/PgDn - use AltGr to differentiate
	if (alt_gr_) {
		switch (Fl::event_key()) {
		case FL_Left:
			real_event = FL_Home;
			break;
		case FL_Right:
			real_event = FL_End;
			break;
		case FL_Up:
			real_event = FL_Page_Up;
			break;
		case FL_Down:
			real_event = FL_Page_Down;
			break;
		default: 
			real_event = Fl::event_key();
			break;
		}
	}
	else {
		real_event = Fl::event_key();
	}
	switch (event) {
	case FL_PUSH:
		take_focus();
		break;
	case FL_FOCUS:
		// Mouse coming into focus 
		if (edit_input_->visible()) {
			// restore focus to edit input and reject focus oursleves
			edit_input_->take_focus();
		}
		return true;
	case FL_UNFOCUS:
		// mouse going in and out of focus on this view
		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
		return true;
	case FL_KEYBOARD:
		// Keyboard event - used for keyboard navigation
		switch (real_event) {
		case FL_Home:
			// HOME - Go->first
			my_book_->selection(0);
			return true;
		case FL_End:
			// END - Go->Last
			my_book_->selection(my_book_->size() - 1);
			return true;
		case FL_Up:
			// UP - Go->Prev/Next (depending on sort order
			if (order_ == LAST_TO_FIRST) {
				if (my_book_->selection() != my_book_->size() - 1) {
					my_book_->selection(my_book_->selection() + 1);
				}
			}
			else {
				if (my_book_->selection() != 0) {
					my_book_->selection(my_book_->selection() - 1);
				}
			}
			return true;
		case FL_Down:
			// DOWN - Go->Next/Prev (depending on sort order)
			if (order_ == LAST_TO_FIRST) {
				if (my_book_->selection() != 0) {
					my_book_->selection(my_book_->selection() - 1);
				}
			}
			else {
				if (my_book_->selection() != my_book_->size() - 1) {
					my_book_->selection(my_book_->selection() + 1);
				}
			}
			return true;
		case FL_Page_Up:
			// PGUP -  Go up one page (find out number of records displayed)
			if (order_ == LAST_TO_FIRST) {
				// The lower of the one page above where we are or last record
				my_book_->selection(std::min<size_t>(my_book_->selection() + rows_per_page_, my_book_->size() - 1));
			}
			else {
				// The greater of record 0 or one page above where we are (record numbers are unsigned (size_t))
				my_book_->selection(std::max<size_t>(0, (signed)my_book_->selection() - rows_per_page_));
			}
			return true;
		case FL_Page_Down:
			// PGDN - Go down one page
			if (order_ == LAST_TO_FIRST) {
				// The greater of record 0 or one page above where we are (record numbers are unsigned (size_t))
				my_book_->selection(std::max<size_t>(0, (signed)my_book_->selection() - rows_per_page_));
			}
			else {
				// The lower of the one page above where we are or last record
				my_book_->selection(std::min<size_t>(my_book_->selection() + rows_per_page_, my_book_->size() - 1));
			}
			return true;
		case FL_Alt_R:
			// Remember AltGR is pressed
			alt_gr_ = true;
			return true;
		case 'v':
			// CTRL-V
			if (Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R)) {
				// Treat as paste clipboard
				Fl::paste(*main_window_, 1);
				return true;
			}
		case FL_F + 1:
			// F1
			open_html("log_table.html");
			return true;
		}
		break;
	case FL_KEYUP:
		if (Fl::event_key() == FL_Alt_R) {
			// Forget AltGr is pressed
			alt_gr_ = false;
			return true;
		}
		break;
	case FL_MOVE:
		// If we have moved more than 10 pixels away from where the tip windows is displayed - remove it
		// Note we will only see this once the mouse has left the tip window
		// There is a race hazard between seeing this event and showing the tip window, so we do need to 
		// check how far the mouse has moved
		if (tip_window_) {
			if (abs(last_rootx_ - tip_root_x_) > 10 || abs(last_rooty_ - tip_root_y_) > 10) {
				Fl::delete_widget(tip_window_);
				tip_window_ = nullptr;
				return true;
			}
		}
		break;
	}
	// We haven't handled the event
	return Fl_Table_Row::handle(event);
}

// override of view::update(). view-specific actions on update
void log_table::update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2) {
	// Hide any edit input 
	if (edit_input_->visible()) {
		record* old_record = my_book_->get_record(edit_row_, false);
		char message[256];
		snprintf(message, 256, "LOG: Editing record %s %s %s - field %s: Abandoned due to external update",
			old_record->item("QSO_DATE").c_str(),
			old_record->item("TIME_ON").c_str(),
			old_record->item("CALL").c_str(),
			(*log_fields_)[edit_col_].field.c_str());
		status_->misc_status(ST_WARNING, message);
		edit_input_->hide();
	}
	// std::set the number of rows
	rows(my_book_->get_count());
	switch (hint) {
	case HT_FORMAT:
		// format has changed - it may be fields so update them
		get_fields();
		// std::set number of fields
		cols(log_fields_->size());
		// and each one's width
		for (unsigned int i = 0; i < log_fields_->size(); i++) {
			col_width(i, (*log_fields_)[i].width);
		}
		// font size may have changed
		adjust_row_sizes();
		redraw();
		break;
	case HT_RESET_ORDER:
		// Reset order to FIRST_TO_LAST
		order_ = FIRST_TO_LAST;
		redraw();
		break;
	default:
		// Rows must exist before the following has an effect
		adjust_row_sizes();
		// select specified record
		if (record_num_1 != -1) {
			current_item_num_ = my_book_->item_number(record_num_1);
			if (order_ == LAST_TO_FIRST) {
				current_item_num_ = my_book_->size() - 1 - current_item_num_;
			}
		}
		display_current();
		redraw();
		break;
	}
}

// Adjust the row height and row header width to match the font used
void log_table::adjust_row_sizes() {
	// Assume that italic may be larger than roman - add 2 pixels margin around the text
	fl_font(font_ | FL_ITALIC, fontsize_);
	int height = fl_height() + 2;
	row_height_all(height);
	int w1 = 0;
	int w2 = 0;
	int sz = 1;
	// Get the largest number record - and find its size
	if (book_) sz = book_->size() + 1;
	std::string max_number = std::to_string(sz) + ' ';
	fl_measure(max_number.c_str(), w1, height);
	// Get the size of the row header "column header" and std::set the width to the larger of the two
	fl_measure("QSO No.", w2, height);
	row_header_width(std::max<int>(w1, w2));
}
 
// Override of Fl_Table_Row method to provide data and formats for each cell
void log_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {

	Fl_Color line_colour = fl_color_average(FL_BACKGROUND_COLOR, selection_color(), 0.5);
	std::string text;
	switch (context) {

	case CONTEXT_STARTPAGE:
		// std::set the default font for the page
		fl_font(font_, fontsize_);
		return;

	case CONTEXT_ENDPAGE: 
	{
		// Code trying to write text into the top-left corner
		// Set the font for the header header
		fl_font(fl_font() ^ FL_ITALIC, fontsize_);
		// Set the col header row header crossing point
		fl_color(col_header_color());
		int X1 = Fl_Table_Row::wix;
		int Y1 = Fl_Table_Row::wiy;
		int W1 = row_header_width();
		int H1 = col_header_height();
		fl_color(col_header_color());
		fl_rectf(X1, Y1, W1, H1);
		fl_color(line_colour);
		fl_yxline(X1, Y1, Y1 + H1 - 1, X1 + W1);
		// Text color
		fl_color(FL_BLACK);
		fl_draw("QSO No.", X1, Y1, W1, H1, FL_ALIGN_CENTER);

		return;
	}

	case CONTEXT_ROW_HEADER:
		// Put record number into header (left-most column)
		fl_push_clip(X, Y, W, H);
		{
			item_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - R : R;
			record* this_record = my_book_->get_record(item_number, false);
			// If the row is selected include the row header in the colouring
			Fl_Color bg_colour = row_selected(R) ? selection_color() : row_header_color();
			if (this_record && my_book_->is_dirty_record(this_record)) bg_colour = fl_lighter(bg_colour);
			fl_color(bg_colour);
			fl_rectf(X, Y, W, H);
			fl_color(line_colour);
			fl_yxline(X, Y, Y + H - 1, X + W);

			// TEXT - contrast its colour to the bg colour.
			if (this_record && my_book_->is_dirty_record(this_record)) {
				fl_color(FL_RED);
			}
			else {
				fl_color(FL_BLACK);
			}
			// Make this italic version of default font
			Fl_Font save = fl_font();
			fl_font(font_ ^ FL_BOLD_ITALIC, fontsize_);
			// Display record number (starting at 1) in the row header
			text = std::to_string(my_book_->record_number(item_number) + 1);
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_LEFT);
			fl_font(save, fontsize_);
		}
		fl_pop_clip();
		return;

	case CONTEXT_COL_HEADER:
		// put field header text into header (top-most row)
		fl_push_clip(X, Y, W, H);
		{
			// Colour cell fill and edges (left and bottom only)
			fl_color(col_header_color());
			fl_rectf(X, Y, W, H);
			fl_color(line_colour);
			fl_yxline(X, Y, Y + H - 1, X + W);

			fl_color(FL_BLACK);
			// text is field header
			text = (*log_fields_)[C].header;
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Get content from record R, field given by fields[C]
		// Don't draw the cell under the Fl_INput
		if (edit_input_->visible() && R == edit_row_ && C == edit_col_) {
			return;
		}
		// Note we may be redrawing before we've updated the number of rows
		if ((size_t)R < my_book_->size() &&
			!(edit_input_->visible() && R == edit_row_ && C == edit_col_)) {
			fl_push_clip(X, Y, W, H);
			{
				item_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - R : R;
				record* this_record = my_book_->get_record(item_number, false);
				// Selected rows will have table specific colour, others in current sesson grey, rest white
				Fl_Color default_bg_colour = in_current_session(this_record) ? COLOUR_GREY : (DARK ? FL_BACKGROUND2_COLOR : FL_WHITE);
				Fl_Color bg_colour = row_selected(R) ? selection_color() : default_bg_colour;
				if (this_record && my_book_->is_dirty_record(this_record)) bg_colour = fl_lighter(bg_colour);
				fl_color(bg_colour);
				fl_rectf(X, Y, W, H);
				// Add a cell border
				fl_color(line_colour);
				fl_yxline(X, Y, Y + H - 1, X + W);

				// TEXT - contrast its colour to the bg colour.
				if (this_record && my_book_->is_dirty_record(this_record)) {
					fl_color(FL_RED);
				}
				else {
					fl_color(fl_contrast(FL_FOREGROUND_COLOR, bg_colour));
				}
				// get the formatted data from the field of the record
				std::string direct = this_record->item((*log_fields_)[C].field, true);
				text = this_record->item((*log_fields_)[C].field, true);
				Fl_Font font = font_;
				if (direct == text) font = font;
				else font ^= FL_ITALIC;
				bool swl = this_record->item("SWL") == "Y";
				if (swl) { 
					font ^= FL_ITALIC;
					fl_color(fl_color_average(fl_color(), bg_colour, 2.F/3.F));
				}
				else font = font;
				// if (DARK) font |= FL_BOLD;
				// else font &= ~FL_BOLD;
				fl_font(font, fontsize_);
				fl_draw(text.c_str(), X + 2, Y, W - 2, H, FL_ALIGN_LEFT, nullptr, false);
				fl_font(font_, fontsize_);

			}
			fl_pop_clip();
		}
		return;
		
	case CONTEXT_RC_RESIZE:
		// Resize the edit input if the cell it's above is being resized
		if (edit_input_->visible()) {
			find_cell(CONTEXT_TABLE, edit_row_, edit_col_, X, Y, W, H);
			edit_input_->resize(X, Y, W, H);
			init_sizes();
		}
		return;
	default:
		break;
	}
}

// Get the appropriate field std::set for the application using this view
void log_table::get_fields() {
	log_fields_ = fields_->collection(application_);
}

// Returns the field std::set
collection_t& log_table::fields() {
	return *log_fields_;
}

// Open an input dialog to allow user to edit the cell 
void log_table::edit_cell(int row, int col) {
	// get the field item under the mouse
	item_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - row : row;
	record* record = my_book_->get_record(item_number, true);
	field_info_t field_info = (*log_fields_)[col];
	std::string text = record->item(field_info.field, true);
	//intl_input* input = (intl_input*)edit_input_;
	edit_row_ = row;
	edit_col_ = col;
	// Get cell location
	int X, Y, W, H;
	find_cell(CONTEXT_CELL, row, col, X, Y, W, H);
	// Open edit dialog exactly the size and position of the cell
	edit_input_->field_name(field_info.field.c_str());
	edit_input_->resize(X, Y, W, H);
	edit_input_->value(text.c_str());
	edit_input_->textfont(font_);
	edit_input_->textsize(fontsize_);
	// Select all the contents of the input
	edit_input_->input()->insert_position(0, text.length());
	// Make the widget visible and let it take focus even if the mouse isn't over it
	edit_input_->show();
	edit_input_->input()->take_focus();
	edit_input_->redraw();
	damage(FL_DAMAGE_ALL, X, Y, W, H);
	redraw();
}

// Copy data from the edit input into the record
void log_table::done_edit(bool keep_row) {
	if (edit_input_->visible()) {
		// Get the record and field
		item_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - edit_row_ : edit_row_;
		record* record = my_book_->get_record(item_number, true);
		field_info_t field_info = (*log_fields_)[edit_col_];
		std::string old_text = record->item(field_info.field);
		std::string text = edit_input_->value();
		if (old_text != text) {
			// Set the record item to the edit input value
			record->item(field_info.field, text, true);
			// Now implemnt book-specific actions
			switch (my_book_->book_type()) {
			case OT_MAIN:
			case OT_EXTRACT:
				redraw();
				menu_->update_items();
				// Update all views with the change - fields that change location are major changes that require DxAtlas to be redrawn, date/time the book to be reordered
				if (field_info.field == "QSO_DATE" || field_info.field == "TIME_ON") {
					// The book will tell all views
					// the changed start time may cause the book to get reordered - move the edit point accordingly
					item_num_t new_item_num = my_book_->item_number(book_->selection(my_book_->record_number(item_number), HT_START_CHANGED));
					if (keep_row && my_book_->book_type() == OT_MAIN) {
						switch (order_) {
						case FIRST_TO_LAST:
							edit_row_ = new_item_num;
							break;
						case LAST_TO_FIRST:
							edit_row_ = my_book_->size() - 1 - new_item_num;
							break;
						default:
							// Do not std::set edit_row_
							break;
						}
					}
				}
				else if (field_info.field == "GRIDSQUARE" || field_info.field == "DXCC" || field_info.field == "STATE" || field_info.field == "APP_ZZA_PFX") {
					// Location may have changed
					book_->selection(my_book_->record_number(item_number), HT_CHANGED, this);
				}
				else {
					// Minor change - not important for DxAtlas
					book_->selection(my_book_->record_number(item_number), HT_MINOR_CHANGE, this);
				}
				if (field_info.field == "CALL") {
					toolbar_->search_text(my_book_->record_number(item_number));
				}
				char message[200];
				// Log the fact that a record has been interactively changed
				snprintf(message, 200, "LOG: %s %s %s record changed %s from %s to %s",
					record->item("QSO_DATE").c_str(),
					record->item("TIME_ON").c_str(),
					record->item("CALL").c_str(),
					field_info.field.c_str(),
					old_text.c_str(),
					record->item(field_info.field).c_str());
				status_->misc_status(ST_LOG, message);
				break;
			case OT_IMPORT:
				// Otherwise just redraw this log
				redraw();
				break;
			default:
				break;
			}
		}
		// Make these invivible as done with them
		edit_input_->hide();
	}
}

// Display a tooltip describing the field item and validating the value
void log_table::describe_cell(int item, int col) {
	int item_number = item;
	std::string tip = "";
	switch (order_) {
	case FIRST_TO_LAST:
		item_number = item;
		break;
	case LAST_TO_FIRST:
		item_number = my_book_->size() - 1 - item;
		break;
	default:
		// NB. We do not std::map items numbers to record numbers for sorted displays
		item_number = item;
		break;
	}
	if (tip.length() == 0) {
		// Get the field item under the mouse
		record* record = my_book_->get_record(item_number, true);
		field_info_t field_info = (*log_fields_)[col];
		// get the tip - for Call parse the call and provide prefix info, otherwise the ADIF description
		if (field_info.field == "CALL") {
			tip = cty_data_->get_tip(record);
		}
		else {
			tip = spec_data_->get_tip(field_info.field, record);
		}
	}
	// display it in a window. Position the window where the mouse clicked
	// The window will close when the mouse is moved far enough off it.
	if (tip_window_) {
		// Delete existing tip window
		Fl::delete_widget(tip_window_);
		tip_window_ = nullptr;
	}
	// Remember tip position
	tip_root_x_ = last_rootx_ - 5;
	tip_root_y_ = last_rooty_ - 5;
	tip_window_ = ::tip_window(tip, tip_root_x_, tip_root_y_);
	tip_window_->show();
}

// Column header was double clicked - if it was for a date/time field reverse direction of display
void log_table::dbl_click_column(int col) {
	field_info_t field_info = (*log_fields_)[col];
	if (field_info.field == "QSO_DATE" ||
		field_info.field == "TIME_ON" ||
		field_info.field == "QSO_DATE_OFF" ||
		field_info.field == "TIME_OFF") {
		// A time-related field - swap order
		switch (order_) {
		case FIRST_TO_LAST:
			order_ = LAST_TO_FIRST;
			break;
		case LAST_TO_FIRST:
			order_ = FIRST_TO_LAST;
			break;
		case SORTED_UP:
		case SORTED_DOWN:
			// Revert to FIRST_TO_LAST
			((extract_data*)my_book_)->correct_record_order();
			order_ = FIRST_TO_LAST;
			break;
		}
		// Keep selected record the same
		select_row(current_item_num_, 0);
		current_item_num_ = my_book_->size() - 1 - current_item_num_;
		display_current();
		// Save value in settings
		settings top_settings;
		settings view_settings(&top_settings, "Views");
		settings log_settings(&view_settings, "Log Table");
		log_settings.set("Recent First", order_);
		// Redraw the window
		redraw();
	}
	else if (my_book_->book_type() == OT_EXTRACT) {
		// Sorting on any other column is only available in extracted records
		// Remember the record number
		qso_num_t selected_record = my_book_->record_number(my_book_->selection());
		switch (order_) {
		case FIRST_TO_LAST:
		case LAST_TO_FIRST:
			// Sort up the way
			((extract_data*)my_book_)->sort_records(field_info.field, false);
			order_ = SORTED_UP;
			break;
		case SORTED_DOWN:
			if (field_info.field == sorted_field_) {
				// If sorting on same field, sort up the way
				((extract_data*)my_book_)->correct_record_order();
				((extract_data*)my_book_)->sort_records(field_info.field, false);
				order_ = SORTED_UP;
			}
			else {
				// Sort up the way on selected column
				//((extract_data*)my_book_)->reextract();
				((extract_data*)my_book_)->sort_records(field_info.field, false);
				order_ = SORTED_UP;
			}
			break;
		case SORTED_UP:
			if (field_info.field == sorted_field_) {
				// If sorting on same field, sort down the way
				((extract_data*)my_book_)->correct_record_order();
				((extract_data*)my_book_)->sort_records(field_info.field, true);
				order_ = SORTED_DOWN;
			}
			else {
				// Sort on new field only (up the way)
				//((extract_data*)my_book_)->reextract();
				((extract_data*)my_book_)->sort_records(field_info.field, false);
				order_ = SORTED_UP;
			}
			break;
		}
		sorted_field_ = field_info.field;
		// Restore selection to record
		my_book_->selection(my_book_->item_number(selected_record));
		redraw();
	}
	else {
		char message[128];
		snprintf(message, 128, "LOG: Sort on %s not available with this log type. Sort ignored", field_info.field.c_str());
		status_->misc_status(ST_WARNING, message);
	}
}

// Select and go to item 
void log_table::display_current() {
	select_row(current_item_num_, 1);
	// get the visible rows - and move the current row to the top if it is outwith them
	// note this assumes the top and bottom rows may be partially hidden
	int r1 = 0, r2 = 0, c1 = 0, c2 = 0;
	visible_cells(r1, r2, c1, c2);
	if ((unsigned)r1 >= current_item_num_ || (unsigned)r2 <= current_item_num_) {
		if (current_item_num_ == 0)
			top_row(current_item_num_);
		else top_row(current_item_num_ - 1);
	}
}

// Column width changed
void log_table::drag_column(int C) {
	// save the new width - note this gets called for both columns either side of the drag point
	// So ignore if it tries to do this for the "column" tp the right of the last one.
	if ((unsigned)C < log_fields_->size()) {
		int new_width = col_width(C);
		(*log_fields_)[C].width = new_width;
		collection_t* coll = fields_->collection(application_);
		field_info_t* item = &(*coll)[C];
		item->width = new_width;
		fields_->save_update();
	}
}

// Set log font values
void log_table::set_font(Fl_Font font, Fl_Fontsize size) {
	font_ = font;
	fontsize_ = size;
}

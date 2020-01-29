
#include "log_table.h"
#include "book.h"
#include "spec_data.h"
#include "../zzalib/utils.h"
#include "../zzalib/callback.h"
#include "edit_dialog.h"
#include "fields.h"
#include "menu.h"
#include "toolbar.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>
#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Single_Window.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern spec_data* spec_data_;
extern book* book_;
extern Fl_Single_Window* main_window_;
extern menu* menu_;
extern toolbar* toolbar_;
extern void add_sub_window(Fl_Window* w);
extern void remove_sub_window(Fl_Window* w);

// constructor - passes parameters  to the two base classes
log_table::log_table(int X, int Y, int W, int H, const char* label, field_ordering_t app) :
  Fl_Table_Row(X, Y, W, H, label)
, view()
{
	fields_.clear();
	current_item_num_ = 0;
	last_event_ = 0;
	last_button_ = 0;
	last_clicks_ = 0;
	last_rootx_ = 0;
	last_rooty_ = 0;
	application_ = FO_MAINLOG;
	rows_per_page_ = 0;
	order_ = FIRST_TO_LAST;
	edit_dialog_ = nullptr;

	// Set the various fixed attributes of the table
	type(SELECT_SINGLE);
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_GRAY);
	row_header(true);
	row_header_color(FL_GRAY);
	when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	callback((Fl_Callback*)log_table::cb_tab_log, (void*)nullptr);
	application_ = app;
	// Get the fields to display
	get_fields();
	// set number of fields
	cols(fields_.size());
	// For each field set the column width
	for (unsigned int i = 0; i < fields_.size(); i++) {
		col_width(i, fields_[i].width);
	}
	// number of rows - available internal height / default row height
	rows_per_page_ = Fl_Table_Row::tih / (ROW_HEIGHT);
	// Reverse order
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.get("Recent First", (int&)order_, FIRST_TO_LAST);

	// Set minimum resizing - width 1/3
	min_w_ = w() / 3;
	// Set minimum resizing - height 6 rows (plus col. header)
	min_h_ = (ROW_HEIGHT) * 7;
	resizable(this);

}

// Destructor
log_table::~log_table()
{
	// Remove data items
	fields_.clear();
	clear();
}

// callback - called when mouse button is released but also at other times. The handle method
// will have been called when the evnt happend allowing us to get full details of the event
void log_table::cb_tab_log(Fl_Widget* w, void* v) {
	// Get the row and field clicked
	log_table* that = (log_table*)w;
	int row = that->callback_row();
	record_num_t item_num = that->order_ == LAST_TO_FIRST ? that->my_book_->size() - 1 - row : row;
	int col = that->callback_col();
	switch (that->callback_context()) {
	case Fl_Table::CONTEXT_CELL:
		// Confirm that it is the button being released
		// tell all views that selection has changed
		if (that->last_event_ == FL_PUSH) {
			// Select the row clicked
			that->my_book_->selection(item_num, HT_SELECTED, that);
			switch (that->last_button_) {
			case FL_LEFT_MOUSE:
				// Delete an existing edit cell
				if (that->edit_dialog_) {
					// Remove the dialog after asking it to quit
					((edit_dialog*)that->edit_dialog_)->do_button(BN_CANCEL);
				}
				// Left button - double click edit cell
				if (that->last_clicks_) {
					that->edit_cell(row, col);
				}
				break;
			case FL_RIGHT_MOUSE:
				// Right button - display tooltip explaining the field
				that->describe_cell(row, col);
				break;
			}
		}
		break;
	case Fl_Table::CONTEXT_COL_HEADER:
		if (that->last_event_ == FL_PUSH && that->last_button_ == FL_LEFT_MOUSE && that->last_clicks_) {
			// Left button double click on column header
			that->dbl_click_column(col);
		}
		break;
	case Fl_Table::CONTEXT_RC_RESIZE:
		if (that->last_event_ == FL_DRAG && that->last_button_ == FL_LEFT_MOUSE && that->is_interactive_resize()) {
			// A row or column has been resized
			that->drag_column(col);
		}
		break;
	}
}

// event handler - remember the event and call widget's handle
int log_table::handle(int event) {
	last_event_ = event;
	last_rootx_ = Fl::event_x_root();
	last_rooty_ = Fl::event_y_root();
	last_button_ = Fl::event_button();
	last_clicks_ = Fl::event_clicks();
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		// mouse going in and out of focus on this view
		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
		return true;
	case FL_KEYBOARD:
		// Keyboard event - used for keyboard navigation
		switch (Fl::event_key()) {
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
				my_book_->selection(min(my_book_->selection() + rows_per_page_, my_book_->size() - 1));
			}
			else {
				// The greater of record 0 or one page above where we are (record numbers are unsigned (size_t))
				my_book_->selection(max(0, (signed)my_book_->selection() - rows_per_page_));
			}
			return true;
		case FL_Page_Down:
			// PGDN - Go down one page (--- do. ---) - NB the bang
			if (order_ == LAST_TO_FIRST) {
				// The greater of record 0 or one page above where we are (record numbers are unsigned (size_t))
				my_book_->selection(max(0, (signed)my_book_->selection() - rows_per_page_));
			}
			else {
				// The lower of the one page above where we are or last record
				my_book_->selection(min(my_book_->selection() + rows_per_page_, my_book_->size() - 1));
			}
			return true;
		}
		break;
	}
	// We haven't handled the event
	return Fl_Table_Row::handle(event);
}

// override of view::update(). view-specific actions on update
void log_table::update(hint_t hint, unsigned int record_num_1, unsigned int record_num_2) {
	switch (hint) {
	case HT_FORMAT:
		// format has changed - it may be fields so update them
		get_fields();
		// set number of fields
		cols(fields_.size());
		// and each one's width
		for (unsigned int i = 0; i < fields_.size(); i++) {
			col_width(i, fields_[i].width);
		}
		break;
	default:
		// set the number of rows
		rows(my_book_->get_count());
		// Rows must exist before the following has an effect
		row_height_all(ROW_HEIGHT);
		// select specified record
		current_item_num_ = my_book_->item_number(record_num_1);
		if (order_ == LAST_TO_FIRST) {
			current_item_num_ = my_book_->size() - 1 - current_item_num_;
		}
		display_current();
		redraw();
		break;
	}
}
 
// Override of Fl_Table_Row method to provide data and formats for each cell
void log_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	string text;
	switch (context) {

	case CONTEXT_STARTPAGE:
		// Set the table font
		fl_font(FONT, FONT_SIZE);
		return;

	case CONTEXT_ENDPAGE:
		// Do nothing
		return;

	case CONTEXT_ROW_HEADER:
		// Put record number into header (left-most column)
		fl_push_clip(X, Y, W, H);
		{

			Fl_Color bg_colour = row_selected(R) ? selection_color() : row_header_color();
			fl_color(bg_colour);
			fl_rectf(X, Y, W, H);
			// TEXT - contrast its colour to the bg colour.
			fl_color(fl_contrast(FL_BLACK, bg_colour));
			// Make this italic version of default font
			Fl_Font save = fl_font();
			fl_font(FONT | FL_ITALIC, FONT_SIZE);
			record_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - R : R;
			text = to_string(my_book_->record_number(item_number) + 1);
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_LEFT);
			fl_font(save, FONT_SIZE);
			// BORDER - mid grey
			fl_color(color());
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
		}
		fl_pop_clip();
		return;

	case CONTEXT_COL_HEADER:
		// put field header text into header (top-most row)
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_BORDER_BOX, X, Y, W, H, col_header_color());
			fl_color(FL_BLACK);
			// text is field header
			text = fields_[C].header;
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Get content from record R, field given by fields[C]
		// Note we may be redrawing before we've updated the number of rows
		if ((size_t)R < my_book_->size()) {
			fl_push_clip(X, Y, W, H);
			{
				// BG COLOR - fill the cell with a colour
				Fl_Color bg_colour = row_selected(R) ? selection_color() : FL_WHITE;
				fl_color(bg_colour);
				fl_rectf(X, Y, W, H);

				// TEXT - contrast its colour to the bg colour.
				fl_color(fl_contrast(FL_BLACK, bg_colour));
				record_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - R : R;
				// get the formatted data from the field of the record
				text = my_book_->get_record(item_number, false)->item(fields_[C].field, true);
				fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

				// BORDER - unselected colour
				fl_color(color());
				// draw top and right edges only
				fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
			}
			fl_pop_clip();
		}
		return;
	}
}

// Get the appropriate field set for the application using this view
void log_table::get_fields() {
	// Read the field data from the registry
	Fl_Preferences display_settings(settings_, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	int num_field_sets = fields_settings.groups();
	// Then get the field_sets for the applications
	char app_path[128];
	char* field_set_name;
	int num_fields = 0;
	sprintf(app_path, "App%d", (int)application_);
	if (fields_settings.groups() > 0) {
		fields_settings.get(app_path, field_set_name, "Default");
		// Read all the field field_sets
		fields_.clear();
		Fl_Preferences field_set_settings(fields_settings, field_set_name);
		num_fields = field_set_settings.groups();
		// now get the field data for the field_set
		for (int j = 0; j < num_fields; j++) {
			Fl_Preferences field_settings(field_set_settings, field_set_settings.group(j));
			field_info_t field;
			field_settings.get("Width", (int&)field.width, 50);
			char * temp;
			field_settings.get("Header", temp, "");
			field.header = temp;
			free(temp);
			field_settings.get("Name", temp, "");
			field.field = temp;
			free(temp);
			fields_.push_back(field);
		}
		free(field_set_name);
	}
	// If the field_set is empty
	if (num_fields == 0) {
		// For each field in the default field set
		for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
			// Copy the field information
			field_info_t field = DEFAULT_FIELDS[i];
			fields_.push_back(field);
		}
	}
}

// Returns the field set
vector<field_info_t>& log_table::fields() {
	return fields_;
}

// Open an input dialog to allow user to edit the cell 
void log_table::edit_cell(int row, int col) {
	// get the field item under the mouse
	record_num_t item_number = (order_ == LAST_TO_FIRST) ? my_book_->size() - 1 - row : row;
	record* record = my_book_->get_record(item_number, true);
	field_info_t field_info = fields_[col];
	string text = record->item(field_info.field, true);
	// Get cell location
	int X, Y, W, H;
	find_cell(CONTEXT_CELL, row, col, X, Y, W, H);
	// Open edit dialog - on top of cell
	edit_dialog_ = new edit_dialog(main_window_->x_root() + X, main_window_->y_root() + Y, W, H);
	add_sub_window(edit_dialog_);
	// Get the dialog action
	button_t action = ((edit_dialog*)edit_dialog_)->display(text);
	switch (action) {
	case BN_OK:
	case BN_SPARE:
	case BN_SPARE + 1:
	case BN_SPARE + 2:
	case BN_SPARE + 3:
		// Save, Save and edit previous, Save and edit next - implement the save
		text = ((edit_dialog*)edit_dialog_)->value();
		record->item(field_info.field, text, true);
		switch (my_book_->book_type()) {
		case OT_MAIN:
		case OT_EXTRACT:
			// Update all views - including this
			if (!my_book_->modified_record() && !my_book_->new_record()) {
				// Let menu know to re-enable save and/or cancel
				if (!my_book_->new_record()) {
					my_book_->modified_record(true);
				}
				if (fields_[col].field == "GRIDSQUARE" || fields_[col].field == "DXCC") {
					book_->selection(my_book_->record_number(item_number), HT_CHANGED);
				}
				else {
					book_->selection(my_book_->record_number(item_number), HT_MINOR_CHANGE);
				}
			}
			else {
				redraw();
				book_->modified(true);
				menu_->update_items();
			}
			if (fields_[col].field == "CALL") {
				toolbar_->search_text(my_book_->record_number(item_number));
			}
			break;
		case OT_IMPORT:
			// Otherwise just redraw this log
			redraw();
		}
		break;
	}
	// Remove the dialog
	remove_sub_window(edit_dialog_);
	Fl::delete_widget(edit_dialog_);
	edit_dialog_ = nullptr;
	switch (action) {
	case BN_SPARE:
		// Save and edit next - open new edit
		if ((unsigned)col < fields_.size() - 1) {
			// Only step forward if there is a column to step to
			edit_cell(row, col + 1);
		}
		break;
	case BN_SPARE + 1:
		// Save and edit previous - open new edit
		if (col > 0) {
			// Only step back if there is a column to step into.
			edit_cell(row, col - 1);
		}
		break;
	case BN_SPARE + 2:
		// Save and edit previous record - open new edit
		if (row > 0) {
			// Only step back if there is a row to step into.
			book_->navigate(NV_PREV);
			edit_cell(row - 1, col);
		}
		break;
	case BN_SPARE + 3:
		// Save and edit next record - open new edit
		if ((unsigned)row < my_book_->size() - 1) {
			// Only step forward if there is a row to step into.
			book_->navigate(NV_NEXT);
			edit_cell(row + 1, col);
		}
		break;
	}
}

// Display a tooltip describing the field item and validating the value
void log_table::describe_cell(int item, int col) {
	// Get the field item under the mouse
	record* record = my_book_->get_record(item, true);
	field_info_t field_info = fields_[col];
	// get the tip
	string tip = spec_data_->get_tip(field_info.field, record);
	// display it in a window that will time-out. Position the window where the mouse clicked
	Fl_Window* tw = ::tip_window(tip, last_rootx_, last_rooty_);
	// Set a timeout to remove the tip window
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
}

// Column header was double clicked - if it was for a date/time field reverse direction of display
void log_table::dbl_click_column(int col) {
	field_info_t field_info = fields_[col];
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
			if (my_book_->book_type() == OT_EXTRACT) {
				((extract_data*)my_book_)->reextract();
			}
			order_ = FIRST_TO_LAST;
			break;
		}
		// Keep selected record the same
		select_row(current_item_num_, 0);
		current_item_num_ = my_book_->size() - 1 - current_item_num_;
		display_current();
		// Save value in settings
		Fl_Preferences display_settings(settings_, "Display");
		display_settings.set("Recent First", order_);
		// Redraw the window
		redraw();
	}
	else if (my_book_->book_type() == OT_EXTRACT) {
		// Save record number of selected item
		record_num_t selected_record = my_book_->record_number(my_book_->selection());
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
				((extract_data*)my_book_)->sort_records(field_info.field, false);
				order_ = SORTED_UP;
			}
			else {
				((extract_data*)my_book_)->reextract();
				((extract_data*)my_book_)->sort_records(field_info.field, false);
				order_ = SORTED_UP;
			}
			break;
		case SORTED_UP:
			if (field_info.field == sorted_field_) {
				// If sorting on same field, sort down the way
				((extract_data*)my_book_)->sort_records(field_info.field, true);
				order_ = SORTED_DOWN;
			}
			else {
				// Sort on new field only (up the way)
				((extract_data*)my_book_)->reextract();
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
}

// Select and go to item 
void log_table::display_current() {
	select_row(current_item_num_, 1);
	// get the visible rows - and move the current row to the top if it is outwith them
	// note this assumes the top and bottom rows may be partially hidden
	int r1 = 0, r2 = 0, c1 = 0, c2 = 0;
	visible_cells(r1, r2, c1, c2);
	if ((unsigned)r1 >= current_item_num_ || (unsigned)r2 <= current_item_num_) {
		top_row(current_item_num_);
	}
}

// Column width changed
void log_table::drag_column(int C) {
	// save the new width - note this gets called for both columns either side of the drag point
	// So ignore if it tries to do this for the "column" tp the right of the last one.
	if ((unsigned)C < fields_.size()) {
		int new_width = col_width(C);
		fields_[C].width = new_width;
		// Write the new value to settings Display/Fields/AppN/Field N
		Fl_Preferences display_settings(settings_, "Display");
		Fl_Preferences fields_settings(display_settings, "Fields");
		char app_path[128];
		char* field_set_name;
		sprintf(app_path, "App%d", (int)application_);
		fields_settings.get(app_path, field_set_name, "Default");
		Fl_Preferences field_set_settings(fields_settings, field_set_name);
		// This is the Cth group.
		Fl_Preferences field_settings(field_set_settings, field_set_settings.group(C));
		field_settings.set("Width", new_width);
	}
}


#include "record_table.h"
#include "spec_data.h"
#include "drawing.h"
#include "fields.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>




extern Fl_Preferences* settings_;
extern spec_data* spec_data_;

// Constructor
record_table::record_table(int X, int Y, int W, int H, const char* label) :
	Fl_Table_Row(X, Y, W, H, label)
, log_record_(nullptr)
, query_record_(nullptr)
, saved_record_(nullptr)
, display_mode_(NO_RECORD)

{	// Set the various fixed attributes of the table
	fields_.clear();
	type(SELECT_SINGLE);
	col_header(true);
	col_resize(true);
	col_resize_min(10);
	col_header_color(FL_BACKGROUND2_COLOR);
	row_header(true);
	row_header_color(FL_BACKGROUND2_COLOR);
	row_header_width(150);
	selection_color(FL_YELLOW);

}

// Destructor
record_table::~record_table()
{
}

// Virtual method to provide cell formatting and contents
void record_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
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
		// Put row number into header
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, row_header_color());
			// Use field name as header
			string field_name = fields_[R];
			if (display_mode_ == LOG_AND_QUERY && 
				log_record_->item(field_name) != "" &&
				query_record_->item(field_name) != "") {
				if (log_record_->item(field_name) == query_record_->item(field_name)) {
					// Set text colour black if match exactly
					fl_color(FL_FOREGROUND_COLOR);
				}
				else if (log_record_->items_match(query_record_, field_name)) {
					// Set text color dark yellow if they almost match - e.g. 4 vs 6 character grid-square
					fl_color(FL_DARK_YELLOW);
				}
				else if ((field_name == "TIME_ON" || field_name == "QSO_DATE") &&
					abs(difftime(log_record_->timestamp(), query_record_->timestamp())) < 300.0) {
					// Time on matches within 5 minutes
					fl_color(FL_DARK_YELLOW);
				}
				else if ((field_name == "TIME_OFF" || field_name == "QSO_DATE_OFF") &&
					abs(difftime(log_record_->timestamp(true), query_record_->timestamp(true))) < 300.0) {
					// Time off matches within 5 minutes
					fl_color(FL_DARK_YELLOW);
				}
				else if (field_name == "MODE" &&
					query_record_->item(field_name) == spec_data_->dxcc_mode(log_record_->item(field_name))) {
					// Mode supplied is the DXCC version of the mode
					fl_color(FL_DARK_YELLOW);
				}
				else if (field_name == "MODE" &&
					query_record_->item(field_name) == log_record_->item("SUBMODE")) {
					// Mode supplied is the submode
					fl_color(FL_DARK_YELLOW);
				}
				else {
					// Set text colour red to indicate significant mismatch
					fl_color(FL_RED);
				}
			}
			else {
				// Set it black
				fl_color(FL_FOREGROUND_COLOR);
			}
			fl_draw(field_name.c_str(), X, Y, W, H, FL_ALIGN_LEFT);
		}
		fl_pop_clip();
		return;

	case CONTEXT_COL_HEADER:
		// put field header text into header
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_FLAT_BOX, X, Y, W, H, col_header_color());
			fl_color(FL_FOREGROUND_COLOR);
			// text is field header
			switch (display_mode_) {
			case LOG_ONLY:
				// Should only be a single column - the record being displayed
				switch (C) {
				case 0:
					text = "LOG";
					break;
				default:
					text = "**INVALID**";
					break;
				}
				break;
			case QUERY_ONLY:
				// Should only be a single column - the record being queried
				switch (C) {
				case 0:
					text = "QUERY";
					break;
				default:
					text = "**INVALID**";
					break;
				}
				break;
			case LOG_AND_QUERY:
				// Three columns - the (modified) entry in the log, the record being queried
				// The original log entry
				switch (C) {
				case 0:
					text = "CURRENT LOG";
					break;
				case 1:
					text = "QUERY";
					break;
				case 2:
					text = "ORIGINAL LOG";
					break;
				}
				break;
			case DUPE_QUERY:
				// Three columns - the (modified) entry in the log, the record being queried
				// The original log entry
				switch (C) {
				case 0:
					text = "RECORD 1";
					break;
				case 1:
					text = "RECORD 2";
					break;
				default:
					text = "**INVALID**";
					break;
				}
				break;
			}
			fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
		// Column indicates which record, row the field
		fl_push_clip(X, Y, W, H);
		{
			// BG COLOR
			fl_color(row_selected(R) ? selection_color() : FL_BACKGROUND_COLOR);
			fl_rectf(X, Y, W, H);

			// TEXT
			fl_color(FL_FOREGROUND_COLOR);
			string field_name = fields_[R];
			switch (C) {
			case 0:
				switch(display_mode_) {
				case LOG_ONLY:
				case LOG_AND_QUERY:
				case DUPE_QUERY:
					// Log record
					text = log_record_->item(field_name);
					break;
				case QUERY_ONLY:
					// Query record
					text = query_record_->item(field_name);
					break;
				}
				break;
			case 1:
				switch (display_mode_) {
				case LOG_ONLY:
				case QUERY_ONLY:
					// Should not get here
					text = "";
					break;
				case LOG_AND_QUERY:
				case DUPE_QUERY:
					// Query record
					text = query_record_->item(field_name);
					break;
				}
				break;
			case 2:
				switch (display_mode_) {
				case LOG_ONLY:
				case QUERY_ONLY:
				case DUPE_QUERY:
					// Should not get here
					text = "";
					break;
				case LOG_AND_QUERY:
					// Original log entry
					text = saved_record_->item(field_name);
					break;
				}
				break;
			}
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

// set the records to display
void record_table::set_records(record* log_record, record* query_record, record* saved_record) {
	log_record_ = log_record;
	query_record_ = query_record;
	saved_record_ = saved_record;
	assess_fields();
	rows(fields_.size());
	row_height_all(ROW_HEIGHT);
	if (log_record_ == NULL && query_record_ == NULL) {
		// No record to display
		display_mode_ = NO_RECORD;
		cols(0);
	}
	else if (log_record_ != NULL && query_record_ == NULL) {
		// Only display log entry
		display_mode_ = LOG_ONLY;
		cols(1);
	}
	else if (log_record_ == NULL && query_record_ != NULL) {
		// Only display query record
		display_mode_ = QUERY_ONLY;
		cols(1);
	}
	else if (log_record_ != NULL && query_record_ != NULL && saved_record != NULL) {
		// Display modified log, query and original log
		display_mode_ = LOG_AND_QUERY;
		cols(3);
	}
	else if (log_record_ != NULL && query_record_ != NULL && saved_record == NULL) {
		// Display both log entries
		display_mode_ = DUPE_QUERY;
		cols(2);
	}
}

// Get the fields that will be displayed - those used in records being displayed plus the minimum set of records specified in settings
void record_table::assess_fields() {
	// Clear any existing fields
	fields_.clear();

	// Get the field_set for this view - this is the set of fields that will be
	// displayed at the top of the table, followed by the remainder in alphabetical order
	Fl_Preferences display_settings(settings_, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	char view_name[20];
	// Get the name of the field_set for this application
	sprintf(view_name, "App%d", FO_QSOVIEW);
	char* field_set_name;
	fields_settings.get(view_name, field_set_name, "Default");

	// now get all the fields in the field_set
	if (fields_settings.groups() > 0) {
		// Settings has one or more field sets - get this one
		Fl_Preferences field_set_settings(fields_settings, field_set_name);
		int num_fields = field_set_settings.groups();
		// For each field named in the field set
		for (int i = 0; i < num_fields; i++) {
			// Get the id of the field - numerical order
			string field_id = field_set_settings.group(i);
			// Get its settings - only interested in name
			Fl_Preferences field_settings(field_set_settings, field_id.c_str());
			char * temp;
			field_settings.get("Name", temp, "");
			string field_name = temp;
			free(temp);
			// if ((log_record_ != NULL && log_record_->item_exists(field_name)) ||
			// 	(query_record_ != NULL && query_record_->item_exists(field_name))) {
				// If the field has a valid entry in either record add it to the list to display
				fields_.push_back(field_name);
			// }
		}
		if (num_fields == 0) {
			// No fields provided in the settings - use default field set
			for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
				fields_.push_back(DEFAULT_FIELDS[i].field);
			}
		}
	}
	else {
		// No field sets provided in the settings - use te default field set
		for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
			fields_.push_back(DEFAULT_FIELDS[i].field);
		}
	}
	free(field_set_name);

	// now add all the remaining fields from the log record
	if (log_record_ != NULL) {
		// For each field in the log record
		for (auto it = log_record_->begin(); it != log_record_->end(); it++) {
			bool found = false;
			// Compare against each field alteady selected
			for (unsigned int i = 0; i < fields_.size() && !found; i++) {
				if (fields_[i] == it->first) {
					found = true;
				}
			}
			if (!found) {
				// Not already selected - add it to the list of fields to display
				fields_.push_back(it->first);
			}
		}
	}

	// now add all the remaining fields from the query record
	if (query_record_ != NULL) {
		// For each field in the query record
		for (auto it = query_record_->begin(); it != query_record_->end(); it++) {
			bool found = false;
			// Compare against each field already selected
			for (unsigned int i = 0; i < fields_.size() && !found; i++) {
				if (fields_[i] == it->first) {
					found = true;
				}
			}
			if (!found) {
				// Not already selected - add it to the list of fields to display
				fields_.push_back(it->first);
			}
		}
	}
}

// Return the field in specified row.
string record_table::field(int row) {
	return fields_[row];
}
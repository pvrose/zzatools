#include "printer.h"
#include "log_table.h"
#include "tabbed_forms.h"
#include "status.h"
#include "qsl_display.h"
#include "drawing.h"
#include "qso_manager.h"
#include "book.h"
#include "record.h"

#include <climits>

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Button.H>




extern tabbed_forms* tabbed_forms_;
extern status* status_;
extern book* navigation_book_;
extern Fl_Preferences* settings_;
extern qso_manager* qso_manager_;

const int LINE_MARGIN = 1;
const int LINE_WIDTH = 1;
const Fl_Font PRINT_FONT = FL_HELVETICA;
const int HEADER_SIZE = 12;
const int ROW_SIZE = 8;

// Constructor - supply the book to print (main log or extract or extract(cards))
printer::printer(object_t type) :
	Fl_Printer()
	, printable_height_(0)
	, printable_width_(0)
	, items_per_page_(0)
	, type_(type)
	, current_y_(0)
	, number_pages_(0)
	, cwin_h_(0)
	, cwin_w_(0)
	, cwin_x_(0)
	, cwin_y_(0)
	, card_data_(nullptr)
{
	fields_.clear();
}

// Print the whole document
int printer::do_job() {
	switch (type_) {
	case OT_EXTRACT:
	case OT_MAIN:
		return print_book();
	case OT_CARD:
		return print_cards();
	default:
		// Should not get here
		return 1;
	}
}

// Print records from the book
int printer::print_book() {
	// Set status 
	char message[256];
	sprintf(message, "PRINTER: Starting print, %zu records", navigation_book_->size());
	status_->misc_status(ST_NOTE, message);
	// Get page title for log print
	if (navigation_book_->size() > 0) {
		record* record_0 = navigation_book_->get_record(0, false);
		string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
		const char* temp = fl_input("Please specify the page title", callsign.c_str());
		if (temp != nullptr) page_title_ = temp;
		else page_title_ = callsign;
	}
	int from_page = 1; 
	int to_page = INT_MAX;
	if (!start_printer(from_page, to_page)) {
		return 1;
	}

	fl_cursor(FL_CURSOR_WAIT);
	// calculate basic properies - row height etc.
	calculate_properties();
	// Initialise progress - switch to display device and back again
	status_->progress(min(to_page + 1 - from_page, number_pages_), type_, "Printing log", "pages");
	// Get field data and calculate field widths
	book_properties();
	// Start the page - exit on error
	if (begin_page()) return 1;
	// For each record
	int page_number = from_page;
	int error = 0;
	print_page_header(page_number);
	// For each record that would be in the page range
	size_t i = (from_page - 1) * items_per_page_;
	while (i < navigation_book_->size() && page_number <= to_page && !error) {
		// Print the record
		print_record(navigation_book_->get_record(i, false));
		i++;
		// When the record would be the last on the page and is not the last in the book
		if (!error && (i % items_per_page_ == 0) && i < navigation_book_->size()) {
			fl_line(0, current_y_, printable_width_, current_y_);
			page_number++;
			// not yet reached the last wanted page
			if (page_number <= to_page) {
				// End the page
				error = end_page();
				// Update progress
				status_->progress(page_number - from_page, type_);
				// Start the next page
				if (!error) error = begin_page();
				if (!error) print_page_header(page_number);
			}
		}
		else if (!error && i == navigation_book_->size()) {
			// Draw thick line at the bottom
			fl_line(0, current_y_, printable_width_, current_y_);
		} else {
			// Print line between records a bit shorter so that the vertical lines win.
			fl_color(FL_DARK1);
			fl_line(LINE_WIDTH, current_y_, printable_width_ - LINE_WIDTH, current_y_);
		}
	}
	// End the page
	end_page();
	end_job();
	// Final progress
	status_->progress(min(to_page + 1 - from_page, number_pages_), type_);
	fl_cursor(FL_CURSOR_DEFAULT);
	if (error) {
		status_->misc_status(ST_ERROR, "PRINTER: Failed!");
		status_->progress("Failed", type_);
	}
	else {
		status_->misc_status(ST_OK, "PRINTER: Done!");
	}
	return error;
}

// calculate_properties
void printer::calculate_properties() {
	int available_height;
	// Get the area that will be printed on
	printable_rect(&printable_width_, &printable_height_);
	// now start allocating the height to header and records
	available_height = printable_height_;
	// Header has two lines - page title and column headers
	// get the font for the page header - and adjust available height by its height plus a line width
	fl_font(PRINT_FONT + FL_BOLD, HEADER_SIZE);
	available_height -= (fl_height() + LINE_WIDTH);
	// get the font for the field header - and ditto
	fl_font(PRINT_FONT + FL_BOLD + FL_ITALIC, ROW_SIZE);
	available_height -= (fl_height() + LINE_WIDTH);
	// set the record font - keep 1 point margin
	fl_font(PRINT_FONT, ROW_SIZE);
	int item_height = fl_height() + LINE_WIDTH + LINE_MARGIN;
	// set page properties
	items_per_page_ = available_height / item_height;
	number_pages_ = (((signed)(navigation_book_->size() - 1)) / items_per_page_) + 1;
}

// get field properties
void printer::book_properties() {
	// we only support printing from log_table views
	log_table* form = (log_table*)tabbed_forms_->get_view(type_);
	// Copy fields from log table
	fields_.clear();
	fields_.insert(fields_.end(), form->fields().begin(), form->fields().end());
	// get book from log table
	// adjust field widths to the widths available
	int required_width = 0;
	// total up required width
	for (auto it = fields_.begin(); it != fields_.end(); it++) {
		required_width += it->width;
	}
	// Calculate scale factor
	double multiplier = (double)printable_width_ / (double)required_width;
	// adjust widths accordingly
	for (auto it = fields_.begin(); it != fields_.end(); it++) {
		// Scale as floating point and convert back to integer
		it->width = (int)((double)it->width * multiplier);
	}
}

// Print the page header
void printer::print_page_header(int page_number) {
	// Get height of first line
	fl_color(FL_BLACK);
	fl_font(PRINT_FONT + FL_BOLD, HEADER_SIZE);
	current_y_ = fl_height() - fl_descent();
	// Add the page title
	char title[1024];
	sprintf(title, "Log: %s - Page %d of %d", page_title_.c_str(), page_number, number_pages_);
	// Position it centrally
	fl_draw(title, ((printable_width_ - (int)fl_width(title)) / 2), current_y_);
	// Draw a line beneath the pagetitle
	current_y_ += fl_descent();
	fl_line(0, current_y_, printable_width_, current_y_);
	current_y_ += LINE_WIDTH;
	// Add the field header - change the font to get new height and descent
	int current_x = 0;
	fl_font(PRINT_FONT + FL_BOLD + FL_ITALIC, ROW_SIZE);
	// Get the depth of the line
	int height = fl_height();
	int descent = fl_descent();
	int delta_y = height - descent;
	fl_line(0, current_y_, 0, current_y_ + height);
	// For each field
	for (auto it = fields_.begin(); it != fields_.end(); it++) {
		// Limit drawing area to cell to avoid overlap (allow descenders to be drawn outside the box)
		fl_push_clip(current_x, current_y_, it->width, height);
		// Draw the text
		fl_draw(it->header.c_str(), current_x, current_y_ + delta_y);
		// Inter field lines
		if (it != fields_.begin()) {
			fl_color(FL_DARK1);
			fl_line(current_x, current_y_, current_x, current_y_ + height);
			fl_color(FL_BLACK);
		}
		// Step to next field position
		current_x += it->width;
		fl_pop_clip();
	}
	// Draw right hand vertical  line
	fl_line(printable_width_, current_y_, printable_width_, current_y_ + height);
	// Draw another horizontal line
	current_y_ += height;
	fl_line(0, current_y_, printable_width_, current_y_);
	current_y_ += LINE_WIDTH;
}

// Print a record
void printer::print_record(record* record) {
	// set up the record font and calculate depth dimensions
	fl_color(FL_BLACK);
	fl_font(PRINT_FONT, ROW_SIZE);
	int height = fl_height();
	int descent = fl_descent();
	int delta_y = height - descent;
	int current_x = 0;
	// For each field
	for (auto it = fields_.begin(); it != fields_.end(); it++) {
		// Limit drawing area to the cell
		fl_push_clip(current_x + 1, current_y_, it->width - 2, height);
		// Draw the text
		fl_draw(record->item(it->field, true, true).c_str(), current_x + 1, current_y_ + delta_y);
		// Inter field lines
		if (it != fields_.begin()) {
			fl_color(FL_DARK1);
		}
		fl_pop_clip();

		fl_line(current_x, current_y_, current_x, current_y_ + height + LINE_MARGIN + LINE_WIDTH);
		fl_color(FL_BLACK);
		// Step to next field position
		current_x += it->width;
	}
	// Draw right hand vertical  line
	fl_line(printable_width_, current_y_, printable_width_, current_y_ + height + LINE_MARGIN + LINE_WIDTH);

	current_y_ += height + LINE_WIDTH + LINE_MARGIN;
}

// Print cards
int printer::print_cards() {
	// Set status 
	char message[256];
	sprintf(message, "PRINTER: Starting print, %zu cards", navigation_book_->size());
	status_->misc_status(ST_NOTE, message);
	int from_page = 1; 
	int to_page = INT_MAX;
	if (!start_printer(from_page, to_page)) {
		return 1;
	}
	if (card_properties()) {
		return 1;
	}

	fl_cursor(FL_CURSOR_WAIT);
	// calculate basic properies - row height etc.
	// Initialise progress - switch to display device and back again
	status_->progress(min(to_page + 1 - from_page, number_pages_), type_, "Printing QSL labels", "pages");
	// For each record
	int page_number = from_page;
	int error = 0;

	// For each record that would be in the page range
	size_t i = (from_page - 1) * items_per_page_;

	while ((i < navigation_book_->size()) && page_number <= to_page && !error) {
		// Print the record and those following
		error = print_page_cards(i);
		// When the record would be the last on the page and is not the last in the book
		page_number++;
	}
	end_job();
	// Final progress
	status_->progress(min(to_page + 1 - from_page, number_pages_), type_);
	fl_cursor(FL_CURSOR_DEFAULT);
	if (error) {
		status_->misc_status(ST_ERROR, "PRINTER: Failed!");
	}
	else {
		status_->misc_status(ST_OK, "PRINTER: Done!");
		// Set QSL_SENT to Q to indicate it has been printed
		for (auto it = navigation_book_->begin(); it != navigation_book_->end(); it++) {
			char message[128];
			snprintf(message, 128, "PRINTER: Setting %s %s QSL_SENT to Q", (*it)->item("QSO_DATE").c_str(), (*it)->item("CALL").c_str());
			(*it)->item("QSL_SENT", string("Q"));
		}
	}
	return error;
}

// Print a single page of cards - update item_num when finished
int printer::print_page_cards(size_t &item_num) {
	int x, y;
	origin(&x, &y);
	begin_page();
	int card;
	// Instantiate the cards that will fit on the page
	for (card = 0; item_num < navigation_book_->size() && card < items_per_page_; card++) {
		// Get the number of items with the same callsign
		record* record_1 = navigation_book_->get_record(item_num, false);
		int num_records = 1;
		while (item_num + num_records < navigation_book_->size() && 
			navigation_book_->get_record(item_num + num_records, false)->item("CALL") == record_1->item("CALL")) {
			num_records++;
		}
		while (num_records > 0) {
			int print_records = num_records > card_data_->max_qsos ? card_data_->max_qsos : num_records;
			record** records = new record * [print_records];
			for (int i = 0; i < print_records; i++) {
				records[i] = navigation_book_->get_record(item_num + i, false);
			}
			int imagex = cwin_x_ + ((card % card_data_->columns) * card_data_->width);
			int imagey = cwin_y_ + (((card / card_data_->columns) % card_data_->rows) * card_data_->height);
			qsl_display* qsl = new qsl_display(imagex, imagey);
			qsl->value(records[0]->item("STATION_CALLSIGN"), qsl_display::LABEL, records, print_records);

			item_num += print_records;
			num_records -= print_records;
			// If did not print all with this callsign create a new label to print
			if (num_records) card++;
		}
	}
	end_page();
	return 0;
}

// Get the various dimensions of the card page 
int printer::card_properties() {
	string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
	qsl_display* qsl = new qsl_display();
	qsl->value(callsign, qsl_display::LABEL);
	card_data_ = qsl->data();
	
	items_per_page_ = card_data_->rows * card_data_->columns;
	int top_margin = 0;
	int left_margin = 0;
	margins(&left_margin, &top_margin, nullptr, nullptr);
	double conversion = nan("");
	switch (card_data_->unit) {
		case qsl_display::INCH:
			conversion = IN_TO_POINT;
			break;
		case qsl_display::MILLIMETER:
			conversion = MM_TO_POINT;
			break;
		case qsl_display::POINT:
			conversion = 1.0;
			break;
	}
	cwin_x_ = (int)(conversion * card_data_->col_left) - left_margin;
	cwin_y_ =  (int)(conversion * card_data_->row_top) - top_margin;
	card_data_->width = (int)(conversion * card_data_->col_width);
	cwin_w_ = card_data_->columns * card_data_->width;
	card_data_->height = (int)(conversion * card_data_->row_height);
	cwin_h_ = card_data_->rows * card_data_->height;
	//
	int last_item = navigation_book_->size() - 1;
	number_pages_ = (last_item / items_per_page_) + 1;

	return 0;
}

// Start the print job. Display user dialog and report abnormalities
bool printer::start_printer(int& from_page, int& to_page) {
	char* error_message = new char[256];;
	memset(error_message, 0, 256);
	char* message = new char[266];
	bool result;
	// Start the job with unknown number of pages - exit if cancelled
	int ecode = begin_job(0, &from_page, &to_page, &error_message);
	switch (ecode) {
	case 0:
		// Successful
		result = true;
		break;
	case 1:
		// User cancel
		strcpy(message, "PRINTER: Print cancelled by user");
		status_->misc_status(ST_WARNING, message);
		result = false;
		break;
	default:
		if (strlen(error_message) == 0) {
			snprintf(message, 256, "PRINTER: Unknown error - %d", ecode);
		} else {
			snprintf(message, 256, "PRINTER: %s", error_message);
		}
		status_->misc_status(ST_ERROR, message);
		result = false;
		break;
	}
	// For ALL pages Windows returns 10000 in to_page, Linux returns 0
	if (to_page < from_page) {
		to_page = INT_MAX;
	}
	delete[] error_message;
	delete[] message;
	return result;
}

#include "printer.h"
#include "log_table.h"
#include "tabbed_forms.h"
#include "status.h"

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

using namespace zzalog;

extern tabbed_forms* tabbed_view_;
extern status* status_;

const int LINE_GAP = 1;
const int LINE_WIDTH = 1;
const Fl_Font PRINT_FONT = FL_HELVETICA;
const int HEADER_SIZE = 12;
const int ROW_SIZE = 9;

// Constructor - supply the book to print (main log or extract)
printer::printer(book* book) :
	Fl_Printer()
	, printable_height_(0)
	, printable_width_(0)
	, items_per_page_(0)
	, item_height_(0)
	, header_height_(0)
	, my_book_(book)
	, current_y_(0)
	, number_pages_(0)
{
	fields_.clear();
}

// Destructor
printer::~printer()
{
	// Not sure of the rationale for doing it here rather than explicit call of do_job()
	do_job();
}

// Print the whole document
int printer::do_job() {
	int from_page;
	int to_page;
	// Start the job with unknown number of pages - exit if cancelled
	if (start_job(0, &from_page, &to_page)) return 1;

	fl_cursor(FL_CURSOR_WAIT);
	// calculate basic properies - row height etc.
	calculate_properties();
	// Get field data and calculate field widths
	book_properties();
	// Start the page - exit on error
	if (start_page()) return 1;
	// For each record
	int page_number = from_page;
	int error = 0;
	print_page_header(page_number);
	// For each record that would be in the page range
	for (size_t i = ((from_page - 1) * items_per_page_) + 1; i < my_book_->size() && page_number <= to_page && !error; i++) {
		// Print the record
		print_record(my_book_->get_record(i, false));
		// When the record would be the last on the page and is not the last in the book
		if (!error && (i % items_per_page_ == 0) && i < my_book_->size() - 1) {
			page_number++;
			// not yet reached the last wanted page
			if (page_number <= to_page) {
				// End the page
				error = end_page();
				// Start the next page
				if (!error) error = start_page();
				if (!error) print_page_header(page_number);
			}
		}
	}
	// End the page
	if (!error) error = end_page();
	if (!error) end_job();
	fl_cursor(FL_CURSOR_DEFAULT);
	return error;
}

// calculate_properties
void printer::calculate_properties() {
	int available_height;
	const int line_gap = 1;
	// Get the area that will be printed on
	printable_rect(&printable_width_, &printable_height_);
	// Now start allocating the height to header and records
	available_height = printable_height_;
	// Header has two lines - page title and column headers
	header_height_ = 0;
	// get the font for the page header - and adjust available height by its height plus a line width
	fl_font(PRINT_FONT + FL_BOLD, HEADER_SIZE);
	available_height -= (fl_height() + LINE_WIDTH);
	header_height_ += fl_height() + LINE_WIDTH;
	// get the font for the field header - and ditto
	fl_font(PRINT_FONT + FL_BOLD + FL_ITALIC, ROW_SIZE);
	available_height -= (fl_height() + LINE_WIDTH);
	header_height_ += fl_height() + LINE_WIDTH;
	// set the record font
	fl_font(PRINT_FONT, ROW_SIZE);
	item_height_ = fl_height();
	// set page properties
	items_per_page_ = available_height / item_height_;
	number_pages_ = ((my_book_->get_count() - 1) / items_per_page_) + 1;
}

// get field properties
void printer::book_properties() {
	// we only support printing from log_table views
	log_table* form = dynamic_cast<log_table*>(tabbed_view_->value());
	if (form == nullptr) {
		status_->misc_status(ST_ERROR, "PRINT: Attempting to print a view that doesn't support it!");
	}
	else {
		// Copy fields from log table
		fields_.clear();
		fields_.insert(fields_.end(), form->fields().begin(), form->fields().end());
		// get book from log table
		my_book_ = form->get_book();
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
}

// Print the page header
void printer::print_page_header(int page_number) {
	// Get height of first line
	fl_color(FL_BLACK);
	fl_font(PRINT_FONT + FL_BOLD, HEADER_SIZE);
	current_y_ = fl_height() - fl_descent();
	// Add the page title
	char title[1024];
	sprintf(title, "Log: %s - Page %d of %d", my_book_->filename(false).c_str(), page_number, number_pages_);
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
	// For each field
	for (auto it = fields_.begin(); it != fields_.end(); it++) {
		// Limit drawing area to cell to avoid overlap (allow descenders to be drawn outside the box)
		fl_push_clip(current_x, current_y_, it->width, height);
		// Draw the text
		fl_draw(it->header.c_str(), current_x, current_y_ + delta_y);
		// Step to next field position
		current_x += it->width;
		fl_pop_clip();
	}
	// Draw another line
	current_y_ += height;
	fl_line(0, current_y_, printable_width_, current_y_);
	current_y_ += LINE_WIDTH;
}

// Print a record
void printer::print_record(record* record) {
	// set up the record font and calculate depth dimensions
	fl_font(PRINT_FONT, ROW_SIZE);
	int height = fl_height();
	int descent = fl_descent();
	int delta_y = height - descent;
	int current_x = 0;
	// For each field
	for (auto it = fields_.begin(); it != fields_.end(); it++) {
		// Limit drawing area to the cell
		fl_push_clip(current_x, current_y_, it->width, height);
		// Draw the text
		fl_draw(record->item(it->field).c_str(), current_x, current_y_ + delta_y);
		// Step to next field position
		current_x += it->width;
		fl_pop_clip();
	}
	current_y_ += height;
}

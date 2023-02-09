#include "printer.h"
#include "log_table.h"
#include "tabbed_forms.h"
#include "status.h"
#include "qsl_form.h"
#include "../zzalib/drawing.h"
#include "qso_manager.h"

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Button.H>

using namespace zzalog;
using namespace zzalib;

extern tabbed_forms* tabbed_forms_;
extern status* status_;
extern book* navigation_book_;
extern Fl_Preferences* settings_;
extern qso_manager* qso_manager_;

const int LINE_MARGIN = 1;
const int LINE_WIDTH = 1;
const Fl_Font PRINT_FONT = FL_HELVETICA;
const int HEADER_SIZE = 12;
const int ROW_SIZE = 9;

// Constructor - supply the book to print (main log or extract or extract(cards))
printer::printer(object_t type) :
	Fl_Printer()
	, printable_height_(0)
	, printable_width_(0)
	, items_per_page_(0)
	, type_(type)
	, current_y_(0)
	, number_pages_(0)
	, card_h_(0)
	, card_w_(0)
	, cwin_h_(0)
	, cwin_w_(0)
	, cwin_x_(0)
	, cwin_y_(0)
	, num_cols_(0)
	, num_rows_(0)
	, col_left_(nan(""))
	, col_width_(nan(""))
	, row_height_(nan(""))
	, row_top_(nan(""))
	, unit_(qsl_form::MILLIMETER)
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
	sprintf(message, "PRINTER: Starting print, %d records", navigation_book_->size());
	status_->misc_status(ST_NOTE, message);
	// Get page title for log print
	if (navigation_book_->size() > 0) {
		record* record_0 = navigation_book_->get_record(0, false);
		string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
		const char* temp = fl_input("Please specify the page title", callsign.c_str());
		if (temp != nullptr) page_title_ = temp;
		else page_title_ = callsign;
	}
	int from_page;
	int to_page;
	if (!start_printer(from_page, to_page)) {
		return 1;
	}

	fl_cursor(FL_CURSOR_WAIT);
	// calculate basic properies - row height etc.
	calculate_properties();
	// Initialise progress - switch to display device and back again
	Fl_Display_Device::display_device()->set_current();
	status_->progress(min(to_page + 1 - from_page, number_pages_), type_, "Printing log", "pages");
	set_current();
	// Get field data and calculate field widths
	book_properties();
	// Start the page - exit on error
	if (start_page()) return 1;
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
				Fl_Display_Device::display_device()->set_current();
				status_->progress(page_number - from_page, type_);
				set_current();
				// Start the next page
				if (!error) error = start_page();
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
	if (!error) error = end_page();
	if (!error) end_job();
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
		fl_draw(record->item(it->field, true).c_str(), current_x + 1, current_y_ + delta_y);
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
	sprintf(message, "PRINTER: Starting print, %d cards", navigation_book_->size());
	status_->misc_status(ST_NOTE, message);
	int from_page;
	int to_page;
	if (!start_printer(from_page, to_page)) {
		return 1;
	}
	if (card_properties()) {
		return 1;
	}

	fl_cursor(FL_CURSOR_WAIT);
	// calculate basic properies - row height etc.
	// Initialise progress - switch to display device and back again
	Fl_Display_Device::display_device()->set_current();
	status_->progress(min(to_page + 1 - from_page, number_pages_), type_, "Printing QSL labels", "pages");
	set_current();
	// For each record
	int page_number = from_page;
	int error = 0;

	// For each record that would be in the page range
	size_t i = (from_page - 1) * items_per_page_;

	while ((i < navigation_book_->size()) && page_number <= to_page && !error) {
		error = start_page();
		// Print the record
		error = print_page_cards(i);
		// When the record would be the last on the page and is not the last in the book
		if (!error) {
			page_number++;
			// not yet reached the last wanted page
			if ((page_number <= to_page) && i) {
				// End the page
				error = end_page();
				// Update progress
				Fl_Display_Device::display_device()->set_current();
				status_->progress(page_number - from_page, type_);
				set_current();
				// Start the next page
				if (i < navigation_book_->size() && !error) {
					error = start_page();
				}
			}
			else {
				error = end_page();
			}
		}
	}
	if (!error) end_job();
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

int printer::print_page_cards(size_t &item_num) {
	Fl_Window* win = new Fl_Window(cwin_x_, cwin_y_, cwin_w_, cwin_h_);
	win->clear_border();
	win->color(FL_WHITE);
	int card;
	for (card = 0; item_num < navigation_book_->size() && card < items_per_page_; card++) {
		// Get the number of items with the same callsign
		record* record_1 = navigation_book_->get_record(item_num, false);
		int num_records = 1;
		while (item_num + num_records < navigation_book_->size() && 
			navigation_book_->get_record(item_num + num_records, false)->item("CALL") == record_1->item("CALL")) {
			num_records++;
		}
		while (num_records > 0) {
			int print_records = num_records > max_number_qsos_ ? max_number_qsos_ : num_records;
			record** records = new record * [print_records];
			for (int i = 0; i < print_records; i++) {
				records[i] = navigation_book_->get_record(item_num + i, false);
			}
			qsl_form* qsl = new qsl_form(cwin_x_ + ((card % num_cols_) * card_w_), cwin_y_ + (((card / num_cols_) % num_rows_) * card_h_), records, print_records);
			item_num += print_records;
			num_records -= print_records;
			// If did not print all with this callsign create a new label to print
			if (num_records) card++;
		}
	}
	win->end();
	win->show();
	print_window(win);
	Fl::delete_widget(win);
	return 0;
}

// Get the various dimensions of the card page 
int printer::card_properties() {
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	// Callsign specific settings
	Fl_Preferences stations_settings(settings_, "Stations");
	Fl_Preferences calls_settings(stations_settings, "Callsigns");
	char* callsign;
	calls_settings.get("Current", callsign, "");
	Fl_Preferences call_settings(qsl_settings, callsign);
	free(callsign);
	char* filename;
	call_settings.get("Filename", filename, "");
	card_filename_ = filename;
	free(filename);
	call_settings.get("Max QSOs per Card", max_number_qsos_, 1);
	call_settings.get("Number Rows", num_rows_, 4);
	call_settings.get("Number Columns", num_cols_, 2);
	call_settings.get("Column Width", col_width_, 101.6);
	call_settings.get("Row Height", row_height_, 67.7);
	call_settings.get("First Row", row_top_, 12.9);
	call_settings.get("First Column", col_left_, 4.6);
	call_settings.get("Unit", (int&)unit_, (int)qsl_form::MILLIMETER);

	items_per_page_ = num_rows_ * num_cols_;
	int top_margin = 0;
	int left_margin = 0;
	margins(&left_margin, &top_margin, nullptr, nullptr);
	double conversion = nan("");
	switch (unit_) {
		case qsl_form::INCH:
			conversion = IN_TO_POINT;
			break;
		case qsl_form::MILLIMETER:
			conversion = MM_TO_POINT;
			break;
		case qsl_form::POINT:
			conversion = 1.0;
			break;
	}
	cwin_x_ = (int)(conversion * col_left_) - left_margin;
	cwin_y_ =  (int)(conversion * row_top_) - top_margin;
	card_w_ = (int)(conversion * col_width_);
	cwin_w_ = num_cols_ * card_w_;
	card_h_ = (int)(conversion * row_height_);
	cwin_h_ = num_rows_ * card_h_;
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
	switch (start_job(0, &from_page, &to_page, &error_message)) {
	case 0:
		// Successful
		result = true;
		break;
	case 1:
		// User cancel
		strcpy(message, "PRINTER: Print cancelled by user");
		Fl_Display_Device::display_device()->set_current();
		status_->misc_status(ST_WARNING, message);
		result = false;
		break;
	default:
		snprintf(message, 256, "PRINTER: %s", error_message);
		Fl_Display_Device::display_device()->set_current();
		status_->misc_status(ST_ERROR, message);
		result = false;
		break;
	}
	delete[] error_message;
	delete[] message;
	return result;
}
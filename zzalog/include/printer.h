#ifndef __PRINTER__
#define __PRINTER__

#include "record.h"
#include "view.h"
#include "fields.h"
#include "qsl_form.h"

#include <vector>
#include <string>

#include <FL/Fl_Printer.H>
#include <FL/Fl.H>

using namespace std;



	// This class provides the print facility to print the log book and QSL cards
	class printer : public Fl_Printer
	{
	public:
		// Constructor - defines whether the main log in book format, extracted log in book format or as QSL cards 
		printer(object_t object);

		// Does the printing
		int do_job();

	protected:
		// calculate_properties
		void calculate_properties();
		// get field properties
		void book_properties();
		// get card properties
		int card_properties();
		// Print the page header
		void print_page_header(int page_number);
		// Print a record
		void print_record(record* record);
		// Print book
		int print_book();
		// Print cards
		int print_cards();
		// Print 1 page of cards
		int print_page_cards(size_t &item_num);
		// Start the print job and handle error messages
		bool start_printer(int& from_page, int& to_page);

	protected:
		// current drawing position
		int current_y_;
		// number of records per page
		int items_per_page_;
		// number of pages
		int number_pages_;
		// printable width and height
		int printable_width_;
		int printable_height_;
		// field data
		vector<field_info_t> fields_;
		// Page title
		string page_title_;
		// Object being printed
		object_t type_;
		// Dimensions of QSL card window
		int cwin_x_;
		int cwin_y_;
		int cwin_w_;
		int cwin_h_;
		// Card parameters
		qsl_form::dim_unit unit_;
		int card_w_;
		int card_h_;
		// Label parameters
		int num_rows_;
		int num_cols_;
		double col_left_;
		double col_width_;
		double row_top_;
		double row_height_;
		// Callsign specific
		int max_number_qsos_;
		string card_filename_;
	};

#endif
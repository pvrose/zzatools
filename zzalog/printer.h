#ifndef __PRINTER__
#define __PRINTER__

#include "record.h"
#include "view.h"
#include "fields.h"

#include <vector>

#include <FL/Fl_Printer.H>
#include <FL/Fl.H>

using namespace std;

namespace zzalog {

	// This class provides the print facility to print the log book
	class printer : public Fl_Printer
	{
	public:
		printer(object_t object);
		virtual ~printer();

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
		int print_page_cards(size_t page_number);

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
		// Object being printed
		object_t type_;
		// Dimensions of card window
		int cwin_x_;
		int cwin_y_;
		int cwin_w_;
		int cwin_h_;
		// Card parameters
		int card_w_;
		int card_h_;
		int num_rows_;
		int num_cols_;

	};

}
#endif
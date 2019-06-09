#ifndef __PRINTER__
#define __PRINTER__

#include "record.h"
#include "book.h"
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
		printer(book* book);
		virtual ~printer();

		// Print the whole document
		int do_job();

	protected:
		// calculate_properties
		void calculate_properties();
		// get field properties
		void book_properties();
		// Print the page header
		void print_page_header(int page_number);
		// Print a record
		void print_record(record* record);

	protected:
		// height of header rows
		int header_height_;
		// height of record
		int item_height_;
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
		// the book
		book* my_book_;
	};

}
#endif
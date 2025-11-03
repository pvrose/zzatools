#ifndef __PRINTER__
#define __PRINTER__

#include "fields.h"
#include "drawing.h"

#include <vector>
#include <string>

#include <FL/Fl_Printer.H>



class record;
class qsl_data;



	//! This class provides the print facility to print the log book and QSL cards.
	class printer : public Fl_Printer
	{
	public:
		//! Constructor.
		
		//! \param object defines:
		//! - OT_MAIN the main log in book format.
		//! - OT_EXTRACT extracted log in book format.
		//! - OT_CARD as QSL cards 
		printer(object_t object);

		//! Does the print job: this is called by the Fl_Printer desctructor.
		int do_job();

	protected:
		//! Calculate page properties
		
		//! Calculates from size of the available print area how many entries can be printed on a page
		//! and how many pages are necessary when printing a log.
		void calculate_properties();
		//! Calculates the width of each field when printing a log.
		void book_properties();
		//! Calculates the size and position of each QSL card label image from the QSL label design.
		int card_properties();
		//! Print the page header when printing a log.
		void print_page_header(int page_number);
		//! Print a single log QSO record.
		void print_record(record* record);
		//! Print the entire log.
		int print_book();
		//! Print the QSL labels for all records.
		int print_cards();
		//! Print one page of cards.
		int print_page_cards(size_t &item_num);
		//! Start the print job and handle error messages for the specified page range.
		bool start_printer(int& from_page, int& to_page);

	protected:
		//! current drawing position
		int current_y_;
		//! number of records per page
		int items_per_page_;
		//! number of pages
		int number_pages_;
		//! Available width for printing.
		int printable_width_;
		//! Available height for printing.
		int printable_height_;
		//! List of fields to be printed for each QSO record.
		collection_t fields_;
		//! Page title when printing log.
		std::string page_title_;
		//! Object being printed
		object_t type_;
		// Dimensions of QSL card window
		int cwin_x_;  //!< Left edge of printable area.
		int cwin_y_;  //!< Top edge of printable area.
		int cwin_w_;  //!< Width of printable area.
		int cwin_h_;  //!< Height of printable area.

		//! Individual card image parameters.
		qsl_data* card_data_;
	};

#endif
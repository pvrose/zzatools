#ifndef __VIEW__
#define __VIEW__

#include "drawing.h"
#include "record.h"

#include <map>
#include <string>

#include <FL/Fl_Widget.H>

using namespace std;



	class book;
	enum hint_t : unsigned char;



	// This is a base class for use with updating - each view will also inherit indirectly from Fl_Widget
	class view
	{
	public:
		view();
		virtual ~view();

		// Public methods
	public:
		// something has changed in the book - usually record 1 is to be selected, record_2 usage per view
		virtual void update(hint_t hint, record_num_t record_num_1, record_num_t record_num_2 = 0) = 0;
		// Returns minimum resizing capability
		int min_w();
		int min_h();
		// set book
		void set_book(book* book);
		// get book
		book* get_book();

		// protected methods
	protected:

		// protected attributes
	protected:
		// The version of the book being displayed in the view
		book * my_book_;
		// Minimum resizeing capability
		int min_w_;
		int min_h_;
	};
#endif

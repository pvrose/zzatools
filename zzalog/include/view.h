#ifndef __VIEW__
#define __VIEW__

#include "drawing.h"

#include <map>
#include <string>

using namespace std;

	class book;
	enum hint_t : uchar;
	typedef size_t qso_num_t;

	//! This is a base class for use with updating - each view will also inherit indirectly from Fl_Widget.
	class view
	{
	public:
		//! Constructor.
		view();
		//! Destructor
		virtual ~view();

		// Public methods
	public:
		//! something has changed in the book

		//! \param hint An indication of what has changed.
		//! \param record_num_1 The index of the QSO record that has specifically ben modified.
		//! \param record_num_2 The index of an asscoiated QSO record. 
		virtual void update(hint_t hint, qso_num_t record_num_1, qso_num_t record_num_2 = 0) = 0;
		//! Returns minimum width the view can be resized
		int min_w();
		//! Returns minimum height the view can be resized
		int min_h();
		//! set book used by the view.
		void set_book(book* book);
		//! Returns the book 
		book* get_book();

		// protected methods
	protected:

		// protected attributes
	protected:
		//! The version of the book being displayed in the view
		book * my_book_;
		//! Minimum width
		int min_w_;
		//! Minimum height
		int min_h_;
	};
#endif

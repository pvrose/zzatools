#include "view.h"
#include "book.h"
#include "drawing.h"



// Constructor
view::view()
	: my_book_(nullptr)
	, min_w_(WIDTH/4)
	, min_h_(HEIGHT/4)
{
}

// Destructor
view::~view()
{
}

//  Return the book std::set for this view
book* view::get_book() {
	return my_book_;
}

// Set the book for this view
void view::set_book(book* book) {
	my_book_ = book;
}

// Return the minimum width resizing
int view::min_w() { return min_w_; }

// Return the minimum height resizing
int view::min_h() { return min_h_; }

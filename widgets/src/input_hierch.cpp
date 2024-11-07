#include "input_hierch.h"

#include <climits>

input_hierch::input_hierch(int X, int Y, int W, int H, const char* L) :
	Fl_Input_Choice(X, Y, W, H, L)
	, start_(1)
	, end_(0)
{

}

input_hierch::~input_hierch()
{

}

// Overload value
const char* input_hierch::value() {
	if (menubutton()->changed()) {
		// Value got from menu - return the value after the last slash
		return menubutton()->mvalue()->label();
	}
	else {
		// Retrun the typed in value
		const char* v = input()->value();
		// Add it back o the menu
		add(v);
		return v;
	}
}

// Overload value
void input_hierch::value(const char* val) {
	// Set the input value
	input()->value(val);
	// Find the index of the expanded value
	const char* s = hierise(val);
	int ix = menubutton()->find_index(s);
	if (ix != -1) {
		menubutton()->value(ix);
	}
}

// Overload Fl_Input_Choice::add(const char* s) 
// For add("GM3ZZA") changes to add("GM3/GM3Z/GM3ZZ/GM3ZZA")
void input_hierch::add(const char* val) {
	const char* s = hierise(val);
	if (menubutton()->find_item(s) == nullptr) {
		Fl_Input_Choice::add(s);
	}
}

// Set the hierarchy description
void input_hierch::hierarchy(int start, int end) {
	end_ = end;
	start_ = start;
}

// Get the hierarchy description
void input_hierch::hierarchy(int* start, int* end) {
	*end = end_;
	*start = start_;
}

const char* input_hierch::hierise(const char* val) {
	int len = end_ * end_;
	char* s = new char[len];
	memset(s, 0, len);
	for (int ix = start_; ix <= end_; ix++) {
		strncat(s, val, ix);
		if (ix < end_)	strcat(s, "/");
	}
	return s;
}
#pragma once
#include <FL/Fl_Input_Choice.H>

// Class as Fl_Input_Choice but with a the choice menu in alphabetic hierarchic

class input_hierch :
    public Fl_Input_Choice
{
public:
    input_hierch(int X, int Y, int W, int H, const char* L = nullptr);
    ~input_hierch();

	// Overload value
	const char* value();
	//
	void value(const char* val);

	// Overload Fl_Input_Choice::add(const char* s)
	void add(const char* val);

	// Set the hierarchy description
	void hierarchy(int start, int end);
	// Get the hierarchy description
	void hierarchy(int* start, int* end);

protected:
	const char* hierise(const char* val);

	int start_;
	int end_;


};


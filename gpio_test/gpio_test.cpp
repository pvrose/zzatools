#include "test_win.h"

#include <FL/Fl.H>

int main(int argc, char** argv)
{
	// Set default font size for all widgets
	FL_NORMAL_SIZE = 10;

	test_win* win = new test_win();

	win->show(argc, argv);

	return Fl::run();
}
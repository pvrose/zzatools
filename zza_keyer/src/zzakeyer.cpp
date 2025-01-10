#include "display.h"

#include <string>

#include <FL/Fl.H>

using namespace std;

// Program strings
string COPYRIGHT = "\302\251 Philip Rose GM3ZZA 2018. All rights reserved.";
string PROGRAM_ID = "ZZAKEYER";
string PROG_ID = "ZKY";
string PROGRAM_VERSION = "0.0.1";
string VENDOR = "GM3ZZA";

int main(int argc, char** argv)
{
	// Set default font size for all widgets
	FL_NORMAL_SIZE = 10;
	// Set courier font - ensure it's Courier New
	Fl::set_font(FL_COURIER, "Courier New");
	Fl::set_font(FL_COURIER_BOLD, "Courier New Bold");
	Fl::set_font(FL_COURIER_ITALIC, "Courier New Italic");
	Fl::set_font(FL_COURIER_BOLD_ITALIC, "Courier New Bold Italic");

	display* win = new display(1000, 1000);

	win->show(argc, argv);

	return Fl::run();
}
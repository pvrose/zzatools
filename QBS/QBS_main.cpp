#include "QBS_data.h"
#include "QBS_window.h"
#include "QBS_consts.h"

#include <string>

#include <FL/Fl_Preferences.H>

using namespace std;

const char* DATE_FORMAT = "%Y-%m-%d";
extern int FL_NORMAL_SIZE;
const char* VERSION = "1.0.1";

QBS_window* window_;

void create_window(string filename) {
	char title[100];
	snprintf(title, 100, "QBS - GM4-8 QSL Bureau status - %s", VERSION);
	window_ = new QBS_window(10, 10, title, filename.c_str());
}

// The main app entry point
int main(int argc, char** argv)
{	
	// Change FL defaults
	FL_NORMAL_SIZE = 11;

	// Get filename - use argument if set
	string filename = "";
	if (argc > 1) filename = string(argv[argc - 1]);
	// Create the window
	create_window(filename);
	window_->show(argc, argv);

	int code = 0;

	// Run the application until it is closed
	code = Fl::run();

	return code;

}



#include "QBS_data.h"
#include "QBS_window.h"
#include "QBS_consts.h"
#include "QBS_logo.h"

#include <string>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_PNG_Image.H>

using namespace std;

const char* DATE_FORMAT = "%Y-%m-%d";
extern int FL_NORMAL_SIZE;
const char* VERSION = "2.0.0";
// Main logo
Fl_PNG_Image main_icon_("QBS_ICON", qbs_png, qbs_png_len);


QBS_window* window_;

void create_window(string filename) {
	char title[100];
	snprintf(title, 100, "QBS - GM4-8 QSL Bureau status - %s (%s)", VERSION, __DATE__);
	window_ = new QBS_window(400, 400, title, filename.c_str());
	printf("%s\n", title);
}

// The main app entry point
int main(int argc, char** argv)
{	
	// Change FL defaults
	FL_NORMAL_SIZE = 11;

	Fl_Window::default_icon(&main_icon_);

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



/* Standard drawing types and constants for use in views and dialogs

*/

#ifndef __DRAWING__
#define __DRAWING__

#include <map>

#include<FL/fl_draw.H>

using namespace std;

// Default font-size to use (& override FLTK defaults?)
const int DEFAULT_SIZE = 10;
const Fl_Fontsize FONT_SIZE = DEFAULT_SIZE;
const Fl_Font FONT = FL_HELVETICA;

// Main window default sizes
const unsigned int MENU_HEIGHT = 30;
const unsigned int TOOL_HEIGHT = 20;
const unsigned int TOOL_GAP = 5;
const unsigned int BORDER_SIZE = 5;
const unsigned int TAB_HEIGHT = 20;

// drawing constants
const int EDGE = 10;                        // Gap between group edge and widget edge
const int HBUTTON = 20;                     // Height of a normal button
const int WBUTTON = 60;                     // Width of a normal button
const int XLEFT = EDGE;                     // Start of widgets
const int YTOP = EDGE;                      // Start of widgets
const int GAP = 10;                         // Gap between non-related widgets
const int HTEXT = 20;                       // Gap to leave for text
const int WRADIO = 15;                      // Width of a box-less rado button
const int HRADIO = WRADIO;                  // Height of a boxless button
const int WLABEL = 50;                      // gap for a label outwith widget
const int WLLABEL = 100;                    // gap for a large label outwith widget
const int HMLIN = 3 * HTEXT;                // Height of a multi-line text box
const int WEDIT = 3 * WBUTTON;              // Width of a text edit box
const int WSMEDIT = 2 * WBUTTON;            // Width of a small text edit box
const int ROW_HEIGHT = DEFAULT_SIZE + 4;    // Default height for table rows

	// Colours to use for buttons - defined using FLTK colour palette
	const Fl_Color COLOUR_ORANGE = 93;       /* R=4/4, B=0/4, G=5/7 */
	const Fl_Color COLOUR_APPLE = 87;        /* R=3/4, B=0/4, G=7/7 */
	const Fl_Color COLOUR_PINK = 170;        /* R=4/4, B=2/4, G=2/7 */
	const Fl_Color COLOUR_MAUVE = 212;       /* R-4/4, B=3/4, G=4/7 */
	const Fl_Color COLOUR_NAVY = 136;        /* R=0/4, B=2/4, G=0/7 */
	const Fl_Color COLOUR_CLARET = 80;     /* R=3/4, B=0/4, G=0/4 */
	const Fl_Color COLOUR_GREY = fl_color_average(FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR, 0.33);


	// the various views and data objects
	enum object_t {
		OT_NONE,             // No object (for default us)
		OT_MAIN,             // The main log view (book & view)
		OT_RECORD,           // The record form (view)
		OT_EXTRACT,          // Extracted records (book & view)
		OT_IMPORT,           // Imported records (book)
		OT_PREFIX,           // Prefix reference view (data & view)
		OT_ADIF,             // ADIF specification (data & view)
		OT_REPORT,           // Report view (view)
		OT_BAND,             // Band-plan data
		OT_SCRATCH,          // Scratchpad 
		OT_CARD,             // Used for printing QSL card labels
		OT_WSJTX,            // Import from WSJT-X
	};

	// Default colours to use in tab view labels and/or progress bar
	const map<object_t, Fl_Color> OBJECT_COLOURS = {
		{ OT_NONE, FL_BACKGROUND_COLOR },
		{ OT_MAIN, FL_GREEN },
		{ OT_EXTRACT, FL_CYAN },
		{ OT_IMPORT, FL_BLUE },
		{ OT_RECORD, FL_MAGENTA },
		{ OT_PREFIX, FL_YELLOW },
		{ OT_ADIF, fl_color_average(FL_RED, FL_YELLOW, (float)(1.0 / 3.0)) },
		{ OT_REPORT, fl_color_average(FL_RED, FL_WHITE, 0.25) },
		{ OT_BAND, FL_GRAY },
		{ OT_CARD, fl_color_average(FL_BLUE, FL_WHITE, 0.25) },
		{ OT_WSJTX, COLOUR_NAVY }
	};

	// Names of the objects
	const map<object_t, const char*> OBJECT_NAMES = {
		{ OT_NONE, "NOTHING" },
		{ OT_MAIN, "LOG" },
		{ OT_EXTRACT, "EXTRACT" },
		{ OT_IMPORT, "IMPORT" },
		{ OT_RECORD, "RECORD" },
		{ OT_PREFIX,"PREFIX" },
		{ OT_ADIF, "ADIF" },
		{ OT_REPORT, "REPORT" },
		{ OT_BAND, "BAND" },
		{ OT_CARD, "QSL" },
		{ OT_WSJTX, "WSJT-X" },
	};

	// Main window default sizes
	const unsigned int WIDTH = 1000;
	const unsigned int HEIGHT = 650;


#endif

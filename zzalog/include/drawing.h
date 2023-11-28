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
const int EDGE = 10;
const int HBUTTON = 20;
const int WBUTTON = 60;
const int XLEFT = EDGE;
const int YTOP = EDGE;
const int GAP = 10;
const int HTEXT = 20;
const int WRADIO = 15;
const int HRADIO = WRADIO;
const int WMESS = 200;
const int WLABEL = 50;
const int WLLABEL = 100;
const int HMLIN = 3 * HTEXT;
const int WEDIT = 3 * WBUTTON;
const int WSMEDIT = 2 * WBUTTON;
const int ROW_HEIGHT = DEFAULT_SIZE + 4;


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
		OT_DXATLAS,          // DxAtlas control view
		OT_CARD,             // Used for printing QSL card labels
		OT_MEMORY,           // Memory table (view)
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
		{ OT_DXATLAS, fl_color_average(FL_BLUE, FL_MAGENTA, (float)(1.0 / 3.0)) },
		{ OT_CARD, fl_color_average(FL_BLUE, FL_WHITE, 0.25) },
		{ OT_MEMORY, fl_color_average(FL_GREEN, FL_WHITE, 0.25) }
	};

	// Names of the objects
	const map<object_t, const char*> OBJECT_NAMES = {
		{ OT_NONE, "OT_NONE" },
		{ OT_MAIN, "OT_MAIN" },
		{ OT_EXTRACT, "OT_EXTRACT" },
		{ OT_IMPORT, "OT_IMPORT" },
		{ OT_RECORD, "OT_RECORD" },
		{ OT_PREFIX,"OT_PREFIX" },
		{ OT_ADIF, "OT_ADIF" },
		{ OT_REPORT, "OT_REPORT" },
		{ OT_BAND, "OT_BAND" },
		{ OT_DXATLAS, "OT_DXATLAS" },
		{ OT_CARD, "OT_CARD" },
		{ OT_MEMORY, "OT_MEMORY" }
	};

	// Main window default sizes
	const unsigned int WIDTH = 1000;
	const unsigned int HEIGHT = 650;

	const Fl_Color COLOUR_ORANGE = 93;       /* R=4/4, B=0/4, G=5/7 */
	const Fl_Color COLOUR_APPLE = 87;        /* R=3/4, B=0/4, G=7/7 */
	const Fl_Color COLOUR_PINK = 170;        /* R=4/4, B=2/4, G=2/7 */
	const Fl_Color COLOUR_MAUVE = 212;       /* R-4/4, B=3/4, G=4/7 */
	const Fl_Color COLOUR_NAVY = 136;        /* R=0/4, B=2/4, G=0/7 */
	const Fl_Color COLOUR_CLARET = 80;     /* R=3/4, B=0/4, G=0/4 */
	const Fl_Color COLOUR_GREY = fl_color_average(FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR, 0.33);

#endif

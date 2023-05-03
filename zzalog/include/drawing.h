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

	// Colour palette used in DxAtlas legend and band plan
	// DxAtlas only has a limited palette of colours for drawing - 
	// so after about 12 colours used these are not seen in the actual colours
	const Fl_Color ZLG_PALETTE[] =
	{ fl_rgb_color(0,0,0),
	fl_rgb_color(255,255,255),fl_rgb_color(0,255,0),fl_rgb_color(0,0,255),
	fl_rgb_color(0,255,255),fl_rgb_color(255,0,255),fl_rgb_color(255,255,0),
	fl_rgb_color(128,0,0),fl_rgb_color(0,128,0),fl_rgb_color(0,0,128),
	fl_rgb_color(0,128,128),fl_rgb_color(128,0,128),fl_rgb_color(128,128,0),
	fl_rgb_color(128,255,0),fl_rgb_color(0,128,255),fl_rgb_color(255,0,128),
	fl_rgb_color(255,128,0),fl_rgb_color(0,255,128),fl_rgb_color(128,0,255),
	fl_rgb_color(128,128,255),fl_rgb_color(255,128,128),fl_rgb_color(128,255,128),
	fl_rgb_color(255,255,128),fl_rgb_color(128,255,255),fl_rgb_color(255,128,255),
	fl_rgb_color(64,0,0),fl_rgb_color(0,64,0),fl_rgb_color(0,0,64),
	fl_rgb_color(0,64,64),fl_rgb_color(64,0,64),fl_rgb_color(64,64,0),
	fl_rgb_color(64,255,0),fl_rgb_color(0,64,255),fl_rgb_color(255,0,64),
	fl_rgb_color(255,64,0),fl_rgb_color(0,255,64),fl_rgb_color(64,0,255),
	fl_rgb_color(64,64,255),fl_rgb_color(255,64,64),fl_rgb_color(64,255,64),
	fl_rgb_color(255,255,64),fl_rgb_color(64,255,255),fl_rgb_color(255,64,255),
	fl_rgb_color(64,128,0),fl_rgb_color(0,64,128),fl_rgb_color(128,0,64),
	fl_rgb_color(128,64,0),fl_rgb_color(0,128,64),fl_rgb_color(64,0,128),
	fl_rgb_color(64,64,128),fl_rgb_color(128,64,64),fl_rgb_color(64,128,64),
	fl_rgb_color(128,128,64),fl_rgb_color(64,128,128),fl_rgb_color(128,64,128),
	fl_rgb_color(192,0,0),fl_rgb_color(0,192,0),fl_rgb_color(0,0,192),
	fl_rgb_color(0,192,192),fl_rgb_color(192,0,192),fl_rgb_color(192,192,0),
	fl_rgb_color(192,255,0),fl_rgb_color(0,192,255),fl_rgb_color(255,0,192),
	fl_rgb_color(255,192,0),fl_rgb_color(0,255,192),fl_rgb_color(192,0,255),
	fl_rgb_color(192,192,255),fl_rgb_color(255,192,192),fl_rgb_color(192,255,192),
	fl_rgb_color(255,255,192),fl_rgb_color(192,255,255),fl_rgb_color(255,192,255),
	fl_rgb_color(192,128,0),fl_rgb_color(0,192,128),fl_rgb_color(128,0,192),
	fl_rgb_color(128,192,0),fl_rgb_color(0,128,192),fl_rgb_color(192,0,128),
	fl_rgb_color(192,192,128),fl_rgb_color(128,192,192),fl_rgb_color(192,128,192),
	fl_rgb_color(128,128,192),fl_rgb_color(192,128,128),fl_rgb_color(128,192,128),
	fl_rgb_color(192,64,0),fl_rgb_color(0,192,64),fl_rgb_color(64,0,192),
	fl_rgb_color(64,192,0),fl_rgb_color(0,64,192),fl_rgb_color(192,0,64),
	fl_rgb_color(192,192,64),fl_rgb_color(64,192,192),fl_rgb_color(192,64,192),
	fl_rgb_color(64,64,192),fl_rgb_color(192,64,64),fl_rgb_color(64,192,64) };

	// Main window default sizes
	const unsigned int WIDTH = 1000;
	const unsigned int HEIGHT = 650;

	const Fl_Color COLOUR_ORANGE = 93;       /* R=4/4, B=0/4, G=5/7 */
	const Fl_Color COLOUR_APPLE = 87;        /* R=3/4, B=0/4, G=7/7 */
	const Fl_Color COLOUR_PINK = 170;        /* R=4/4, B=2/4, G=2/7 */
	const Fl_Color COLOUR_MAUVE = 212;       /* R-4/4, B=3/4, G=4/7 */
	const Fl_Color COLOUR_NAVY = 136;        /* R=0/4, B=2/4, G=0/7 */
	const Fl_Color COLOUR_CLARET = 80;     /* R=3/4, B=0/4, G=0/4 */

#endif

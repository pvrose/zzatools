/* Standard drawing types and constants for use in views and dialogs

*/

#ifndef __DRAWING__
#define __DRAWING__

#include "..//zzalib/drawing.h"

#include <map>

#include<FL/fl_draw.H>

using namespace std;

namespace zzalog {
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
		OT_MEMORY            // Memory table (view)
	};

	// Default colours to use in tab view labels 
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
		{ OT_DXATLAS,fl_color_average(FL_BLUE, FL_MAGENTA, (float)(1.0 / 3.0)) },
		{ OT_CARD, fl_color_average(FL_BLUE, FL_WHITE, 0.25) },
		{ OT_MEMORY, fl_color_average(FL_GREEN, FL_WHITE, 0.25) }
	};

	// Names of the objects
	const map<object_t, char*> OBJECT_NAMES = {
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
}

#endif
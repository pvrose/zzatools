#ifndef __LIB_DRAWING__
#define __LIB_DRAWING__

#include "pch.h"

#include <FL/fl_draw.H>
/* 
This file contains solution wide drawing constants
*/

namespace zzalib {

	// Default font-size to use
	const int DEFAULT_SIZE = 10;
	const Fl_Fontsize FONT_SIZE = DEFAULT_SIZE;
	const Fl_Font FONT = FL_HELVETICA;

	// Main window default sizes
	const unsigned int MENU_HEIGHT = 30;
	const unsigned int TOOL_HEIGHT = 20;
	const unsigned int TOOL_GAP = 5;
	const unsigned int BORDER_SIZE = 5;
	const unsigned int TAB_HEIGHT = 20;

	// Constants used for laying out controls
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

	// Tip window dimensions
	const unsigned int TIP_WIDTH = 200;

}
#endif
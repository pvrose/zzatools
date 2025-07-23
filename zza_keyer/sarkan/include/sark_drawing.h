/* Standard drawing types and constants for use in views and dialogs

*/

#ifndef __DRAWING__
#define __DRAWING__

#include<FL/fl_draw.H>

using namespace std;

// Default font-size to use (& override FLTK defaults?)
const int DEFAULT_SIZE = 10;
const Fl_Fontsize FONT_SIZE = DEFAULT_SIZE;
const Fl_Font FONT = FL_HELVETICA;

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
const int WSSEDIT = WBUTTON * 3 / 2;
const int ROW_HEIGHT = DEFAULT_SIZE + 4;
const int AXIS_GAP = 40;
 

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

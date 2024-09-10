#pragma once

#include "utils.h"

#include <cmath>

#include <FL/Enumerations.H>

// Class to initialise and draw custom label symbols 
// 
// @eyeopen 
//		An open eye with lashes on upper lid
// @eyeshut
//      A shut eye with upper lid lashes pointing down


// Draw the eye open symbol
void draw_eyeopen(Fl_Color c);
// Draw the eye shut sybol
void draw_eyeshut(Fl_Color c);

// Draw the individual componnents...
// An arc for the edge of the upper lid
void draw_upper_lid();
// An arc for the edge of the lower lid
void draw_lower_lid();
// A circle for the iris
void draw_iris();
// Individual eyelashes
void draw_lash(int lash);

// Drawing constants
const double lid_arc = 90.0;
const double lid_angle = (lid_arc / 2.0) * DEGREE_RADIAN;
const double lid_incr = lid_angle / 3.0;
// Lid origion Y value
const double origin_y = 1 / tan(lid_angle);
// Lid radius (inner)
const double radius_i = 1 / sin(lid_angle);
// Lash radius (outer)
const double radius_o = 1.0 + origin_y;
// Iris radius
const double radius_c = radius_i - origin_y;
// eyelash coordinate struct
struct lash_xy {
	double x0;
	double y0;
	double x1;
	double y1;
};
// individual lashes
const lash_xy lash1 = {
	radius_i * sin(lid_incr),
	radius_i * cos(lid_incr) - origin_y,
	radius_o * sin(lid_incr),
	radius_o * cos(lid_incr) - origin_y
};
const lash_xy lash2 = {
	radius_i * sin(lid_incr + lid_incr),
	radius_i * cos(lid_incr + lid_incr) - origin_y,
	radius_o * sin(lid_incr + lid_incr),
	radius_o * cos(lid_incr + lid_incr) - origin_y
};
// Lash co-ordinate table
const lash_xy lash_lut[10] = {
	{ -lash2.x0, lash2.y0, -lash2.x1, lash2.y1},
	{ -lash1.x0, lash1.y0, -lash1.x1, lash1.y1},
	{ 0.0, radius_c, 0.0, 1.0},
	lash1,
	lash2,
	{ -lash2.x0, -lash2.y0, -lash2.x1, lash2.y1},
	{ -lash1.x0, -lash1.y0, -lash1.x1, -lash1.y1},
	{ 0.0, -radius_c, 0.0, -1.0},
	{ lash2.x0, -lash2.y0, lash2.x1, lash2.y1},
	{ lash1.x0, -lash1.y0, lash1.x1, -lash1.y1}
};

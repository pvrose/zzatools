#include "symbols.h"
#include "utils.h"

#include <cmath>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>


// Contains the drawing routines for the label suymbols required in ZZALO

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
const double radius_o = 0.9 + origin_y;
// Iris radius
const double radius_c = (radius_i - origin_y) * 2.0 / 3.0;
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
	origin_y - (radius_i * cos(lid_incr)),
	radius_o * sin(lid_incr),
	origin_y - (radius_o * cos(lid_incr))
};
const lash_xy lash2 = {
	radius_i * sin(lid_incr + lid_incr),
	origin_y - (radius_i * cos(lid_incr + lid_incr)),
	radius_o * sin(lid_incr + lid_incr),
	origin_y - (radius_o * cos(lid_incr + lid_incr))
};
// Lash co-ordinate table
const lash_xy lash_lut[10] = {
	{ -lash2.x0, lash2.y0, -lash2.x1, lash2.y1},
	{ -lash1.x0, lash1.y0, -lash1.x1, lash1.y1},
	{ 0.0, origin_y - radius_i, 0.0, origin_y - radius_o},
	lash1,
	lash2,
	{ -lash2.x0, -lash2.y0, -lash2.x1, -lash2.y1},
	{ -lash1.x0, -lash1.y0, -lash1.x1, -lash1.y1},
	{ 0.0, radius_i - origin_y, 0.0, radius_o - origin_y},
	{ lash1.x0, -lash1.y0, lash1.x1, -lash1.y1},
	{ lash2.x0, -lash2.y0, lash2.x1, -lash2.y1}
};


// Draw the individual componnents...
// An arc for the edge of the upper lid
void draw_upper_lid() {

	fl_begin_line();
	fl_arc(0.0, origin_y, radius_i, 90.0 - (lid_arc / 2), 90.0 + (lid_arc/2));
	fl_end_line();
}

// An arc for the edge of the lower lid
void draw_lower_lid() {

	fl_begin_line();
	fl_arc(0.0, -origin_y, radius_i, 270.0 - (lid_arc / 2), 270.0 + (lid_arc / 2));
	fl_end_line();
}

// A circle for the iris
void draw_iris() {
	fl_begin_polygon();
	fl_circle(0.0, 0.0, radius_c);
	fl_end_polygon();
}

// Individual eyelashes
void draw_lash(int lash) {
	fl_begin_line();
	const lash_xy& xy = lash_lut[lash];
	fl_vertex(xy.x0, xy.y0);
	fl_vertex(xy.x1, xy.y1);
	fl_end_line();
}

// Draw the eye open symbol
void draw_eyeopen(Fl_Color c) {
	Fl_Color save = fl_color();
	fl_color(c);
	draw_upper_lid();
	draw_lower_lid();
	draw_iris();
	for (int i = 0; i < 10; i++) {
		draw_lash(i);
	}
	fl_color(save);
}

// Draw the eye shut sybol
void draw_eyeshut(Fl_Color c) {
	Fl_Color save = fl_color();
	fl_color(c);
	draw_upper_lid();
	draw_lower_lid();
	for (int i = 5; i < 10; i++) {
		draw_lash(i);
	}
	fl_color(save);
}

// Data for drawing calendar
const double cal[8] = { -1.00, -5. / 7., -3. / 7., -1. / 7., 1. / 7., 3. / 7., 5. / 7., 1.0 };

void draw_horizontal(int i) {
	fl_begin_line();
	fl_vertex(cal[0], cal[i]);
	fl_vertex(cal[7], cal[i]);
	fl_end_line();
}

void draw_vertical(int i, bool shrt) {
	fl_begin_line();
	fl_vertex(cal[i], cal[shrt ? 2 : 0]);
	fl_vertex(cal[i], cal[7]);
	fl_end_line();
}

// Draw the calendar symbole
void draw_calendar(Fl_Color c) {
	Fl_Color save = fl_color();
	fl_color(c);

	// Draw outer box +/- 0.9
	fl_begin_loop();
	fl_vertex(-1.0, -1.0);
	fl_vertex(1.0, -1.0);
	fl_vertex(1.0, 1.0);
	fl_vertex(-1.0, 1.0);
	fl_end_loop();
	// Draw top filled box
	fl_begin_polygon();
	fl_vertex(-1.0, -1.0);
	fl_vertex(1.0, -1.0);
	fl_vertex(1.0, -0.5);
	fl_vertex(-1.0, -0.5);
	fl_end_polygon();

	// Attempt to draw "1"
	fl_begin_line();
	fl_vertex(-0.2, -0.1);
	fl_vertex(0.1, -0.3);
	fl_vertex(0.1, 0.7);
	fl_end_line();

	fl_color(save);
}

// Draw the mail symbol
void draw_mail(Fl_Color c) {
	Fl_Color sv = fl_color();
	// Fill in lower section]
	fl_color(fl_lighter(fl_lighter(c)));
	fl_begin_polygon();
	fl_vertex(-0.7, -0.5);
	fl_vertex(+0.7, -0.5);
	fl_vertex(+0.7, +0.5);
	fl_vertex(-0.7, +0.5);
	fl_end_polygon();
	// Fill in flap
	fl_color(c);
	fl_begin_polygon();
	fl_vertex(-0.7, -0.5);
	fl_vertex(+0.7, -0.5);
	fl_vertex(0, +0.1);
	fl_end_polygon();
	// Draw the outline
	fl_begin_loop();
	fl_vertex(-0.7, -0.5);
	fl_vertex(+0.7, -0.5);
	fl_vertex(+0.7, +0.5);
	fl_vertex(-0.7, +0.5);
	fl_end_loop();
	// Draw the upward join
	fl_begin_line();
	fl_vertex(-0.7, +0.5);
	fl_vertex(0, -0.1);
	fl_vertex(+0.7, +0.5);
	fl_end_line();
	// Restore colour
	fl_color(sv);
}





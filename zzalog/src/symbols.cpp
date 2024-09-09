#include "symbols.h"

#include <FL/fl_draw.H>


// Contains the drawing routines for the label suymbols required in ZZALO


// Draw the individual componnents...
// An arc for the edge of the upper lid
void draw_upper_lid() {

	fl_begin_line();
	fl_arc(0.0, -origin_y, radius_i, 90.0 - (lid_arc / 2), 90.0 + (lid_arc/2));
	fl_end_line();
}

// An arc for the edge of the lower lid
void draw_lower_lid() {

	fl_begin_line();
	fl_arc(0.0, origin_y, radius_i, 270.0 - (lid_arc / 2), 270.0 + (lid_arc / 2));
	fl_end_line();
}

// A circle for the iris
void draw_iris() {
	fl_begin_line();
	fl_circle(0.0, 0.0, radius_c);
	fl_end_line();
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
	for (int i = 0; i < 5; i++) {
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





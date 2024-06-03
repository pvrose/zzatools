#include "font_dialog.h"
#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Color_Chooser.H>

font_dialog::font_dialog(Fl_Font f, Fl_Fontsize sz, Fl_Color c) :
    win_dialog(10, 10),
    font_(f),
    fontsize_(sz),
    colour_(c)
{
    int curr_x = GAP;
    int curr_y = GAP + HTEXT;

    Fl_Hold_Browser* w01 = new Fl_Hold_Browser(curr_x, curr_y, WEDIT, HMLIN, "Font");
    w01->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    w01->tooltip("Please select the font used for the QSL card field entry");
    populate_font(w01, &font_);

    curr_x += w01->w();
    Fl_Hold_Browser* w02 = new Fl_Hold_Browser(curr_x, curr_y, WBUTTON, HMLIN, "Size");
    w02->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    w02->callback(cb_size, &font_);
 	w02->tooltip("Please select the font size used for the QSL card field entry");
    w01->callback(cb_font, w02);

    curr_x += w02->w();
    Fl_Color_Chooser* w03 = new Fl_Color_Chooser(curr_x, curr_y, 200, 94, "Colour");
    w03->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    w03->callback(cb_colour);
    w03->tooltip("Please select the colour to be used for the QSL card field entry");
    uchar r, g, b;
    Fl::get_color(colour_, r, g, b);
    w03->rgb((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);

    curr_x += w03->w() + GAP;
    curr_y += max(w01->h(), w03->h()) + GAP;

    resizable(nullptr);
    size(curr_x - x(), curr_y - h());
    end();

}

font_dialog::~font_dialog() {}

// callback - OK button
void font_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	font_dialog* that = ancestor_view<font_dialog>(w);
	that->do_button(BN_OK);
}

// callback - cancel button (window close only)
void font_dialog::cb_bn_cancel(Fl_Widget* w, void * v) {
    font_dialog* that = ancestor_view<font_dialog>(w);
	that->do_button(BN_CANCEL);

}

// Callback - font chooser
void font_dialog::cb_font(Fl_Widget* w, void* v) {
    Fl_Hold_Browser* br = (Fl_Hold_Browser*)w;
    font_dialog* that = ancestor_view<font_dialog>(w);
    that->font_ = (Fl_Font)br->value() - 1;
    that->populate_size((Fl_Widget*)v, &that->font_, &that->fontsize_);
}

// size chooser
void font_dialog::cb_size(Fl_Widget* w, void* v) {
    Fl_Hold_Browser* br = (Fl_Hold_Browser*)w;
    font_dialog* that = ancestor_view<font_dialog>(w);
    int line = br->value();
    *(Fl_Fontsize*)v = stoi(br->text(line));
}

// Colour chooser
void font_dialog::cb_colour(Fl_Widget* w, void* v) {
    Fl_Color_Chooser* cc = (Fl_Color_Chooser*)w;
    font_dialog* that = ancestor_view<font_dialog>(w);
    that->colour_ = fl_rgb_color(255 * cc->r(), 255 * cc->g(), 255 * cc->b());
}

void font_dialog::populate_font(Fl_Widget* w, const Fl_Font* font) {
    Fl_Hold_Browser* br = (Fl_Hold_Browser*)w;
  	br->clear();
	// Only get FLTK default fonts
	for (int i = 0; i < FL_FREE_FONT; i++) {
		// Contains any combination of FL_BOLD and FL_ITALIC
		const char* name = Fl::get_font_name(Fl_Font(i), nullptr);
		char buffer[128];
		// display in the named font
		sprintf(buffer, "@F%d@.%s", i, name);
		br->add(buffer);
	}
	br->value(*font + 1); 
}

void font_dialog::populate_size(Fl_Widget* w, const Fl_Font* font, const Fl_Fontsize* size) {
    Fl_Hold_Browser* br = (Fl_Hold_Browser*)w;
	br->clear();
	// To receive the array of sizes
	int* sizes;
	int num_sizes = Fl::get_font_sizes(*font, sizes);
	if (num_sizes) {
		// We have some sizes
		if (sizes[0] == 0) {
			// Scaleable font - so any size available 
			for (int i = 1; i < max(64, sizes[num_sizes - 1]); i++) {
				char buff[20];
				sprintf(buff, "%d", i);
				br->add(buff);
			}
			br->value(*size);
		}
		else {
			// Only list available sizes
			int select = 0;
			for (int i = 0; i < num_sizes; i++) {
				// while the current value is less than required up the selected value
				if (sizes[i] < *size) {
					select = i;
				}
				char buff[20];
				sprintf(buff, "%d", sizes[i]);
				br->add(buff);
			}
			br->value(select);
		}
	}   
}

Fl_Font font_dialog::font() {
	return font_;
}

Fl_Fontsize font_dialog::font_size() {
	return fontsize_;
}

Fl_Color font_dialog::colour() {
	return colour_;
}
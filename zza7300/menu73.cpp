#include "menu73.h"
#include "view73.h"
#include "../zzalib/drawing.h"
#include "../zzalib/utils.h"

#include <FL/Fl_Menu_Item.H>

using namespace zza7300;

extern view* view_;

Fl_Menu_Item items_[] = {
	{ "Settings", 'S', menu73::cb_mi_settings, 0},
	{ "View", 'V', nullptr, nullptr, FL_SUBMENU},
		{ "Memories", 'M', menu73::cb_mi_view, (void*)VT_MEMORIES},
		{ "Panadapter Bands", 'P', menu73::cb_mi_view, (void*)VT_PANADAPTER_BANDS},
		{ "User Bands", 'U', menu73::cb_mi_view, (void*)VT_USER_BANDS},
		{ "CW Messages", 'C', menu73::cb_mi_view, (void*)VT_CW_MESSAGES},
		{ "RTTP Messages", 'V', menu73::cb_mi_view, (void*)VT_RTTY_MESSAGES},
		{ 0 },
	{ 0 }
};

menu73::menu73(int X, int Y, int W, int H, const char* label) : Fl_Menu_Bar(X, Y, W, H, label) {
	// Set the menu73
	Fl_Menu_Bar::menu73(items_);
	// Font size
	textsize(FONT_SIZE + 1);
}

// Settings
void menu73::cb_mi_settings(Fl_Widget* w, void* v) {
	// To do open settings dialog
}

// View->item
// v is view_type
void menu73::cb_mi_view(Fl_Widget* w, void* v) {
	view_type type = (view_type)(long)v;
	view_->type(type);
}
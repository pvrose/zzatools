#include "menu73.h"
#include "view73.h"
#include "../zzalib/drawing.h"
#include "../zzalib/utils.h"

#include <FL/Fl_Menu_Item.H>

using namespace zza7300;
using namespace zzalib;

extern view73* view_;

Fl_Menu_Item items_[] = {
	{ "Settings", 'S', nullptr, nullptr, FL_SUBMENU},
		{ "Rig", 'R', menu73::cb_mi_sett_rig },
		{ 0 },
	{ "View", 'V', nullptr, nullptr, FL_SUBMENU},
		{ "Memories", 'M', menu73::cb_mi_view, (void*)VT_MEMORIES},
		{ "Scope Edge Bands", 'S', menu73::cb_mi_view, (void*)VT_SCOPE_BANDS},
		{ "User Bands", 'U', menu73::cb_mi_view, (void*)VT_USER_BANDS},
		{ "CW Messages", 'C', menu73::cb_mi_view, (void*)VT_CW_MESSAGES},
		{ "RTTP Messages", 'R', menu73::cb_mi_view, (void*)VT_RTTY_MESSAGES},
		{ 0 },
	{ 0 }
};

menu73::menu73(int X, int Y, int W, int H, const char* label) : Fl_Menu_Bar(X, Y, W, H, label) {
	// Set the menu
	Fl_Menu_Bar::menu(items_);
	// Font size
	textsize(FONT_SIZE + 1);
}

// Settings
void menu73::cb_mi_sett_rig(Fl_Widget* w, void* v) {
	// To do open settings dialog
}

// View->item
// v is view_type
void menu73::cb_mi_view(Fl_Widget* w, void* v) {
	view_type type = (view_type)(long)v;
	view_->type(type);
}
#include "view73.h"

#include <FL/Fl_Preferences.H>

using namespace zza7300;

extern Fl_Preferences* settings_;

view73::view73(int X, int Y, int W, int H, const char* label) : Fl_Table_Row(X, Y, W, H, label) {
	Fl_Preferences view_settings(settings_, "View");
	view_settings.get("Type", (int)
}

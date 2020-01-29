#ifndef __MENU73__
#define __MENU73__

#include <FL/Fl_Menu_Bar.H>

namespace zza7300 {
	class menu73 : public Fl_Menu_Bar
	{
	public:
		menu73(int X, int Y, int W, int H, const char* label = 0);
		~menu73();

	public:
		// Settings
		static void cb_mi_settings(Fl_Widget* w, void* v);
		// View->view
		static void cb_mi_view(Fl_Widget* w, void* v);
	};
}

#endif

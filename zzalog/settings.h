#ifndef __SETTINGS__
#define __SETTINGS__

#include "page_dialog.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>

namespace zzalog {

	// This class provides a window to display all the settings dialogs as separate tabbed panes
	class settings : public Fl_Window

	{

	public:

	public:
		settings(int W, int H, const char* label, cfg_dialog_t active);
		~settings();

		// callbacks
		static void cb_bn_cal(Fl_Widget* w, long arg);

		// protected methods
	protected:
		// protected attributes
	protected:
		// The currently active dialog
		Fl_Widget * settings_view_;


	};

}
#endif

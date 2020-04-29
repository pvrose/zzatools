#ifndef __SETTINGS__
#define __SETTINGS__

#include "../zzalib/page_dialog.h"

#include <set>

#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>

using namespace zzalib;

namespace zzalog {

	// This class provides a window to display all the settings dialogs as separate tabbed panes
	class settings : public Fl_Window

	{

	public:

		// The tab types in the settings dialog
		enum cfg_dialog_t {
			DLG_FORMAT,
			DLG_RIG,
			DLG_FILES,
			DLG_WEB,
			DLG_STATION,
			DLG_COLUMN,
			DLG_ADIF,
			DLG_QSL,
			DLG_ALL
		};

	public:
		settings(int W, int H, const char* label, cfg_dialog_t active);
		~settings();

		// callbacks
		static void cb_bn_cal(Fl_Widget* w, long arg);

		void update();
		// Test whether any we've had OK or Cancel
		bool active();
		// Clear active flag
		void inactive();

		// protected methods
	protected:
		// protected attributes
	protected:
		// The currently active dialog
		Fl_Widget * settings_view_;
		// Any widgets that need updating on record selection
		set<page_dialog*> updatable_views_;
		// Active flag
		bool active_;



	};

}
#endif

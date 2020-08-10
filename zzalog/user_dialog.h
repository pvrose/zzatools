#ifndef __USER_DIALOG__
#define __USER_DIALOG__

#include "../zzalib/page_dialog.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Hold_Browser.H>

using namespace zzalib;

namespace zzalog {
	class user_dialog : public page_dialog
	{
	public:
		user_dialog(int X, int Y, int W, int H, const char* label);
		virtual ~user_dialog();
		// Load values from settings_
		virtual void load_values();
		// Used to create the form
		virtual void create_form(int X, int Y);
		// Used to write settings back
		virtual void save_values();
		// Used to enable/disable specific widget - any widgets enabled must be attributes
		virtual void enable_widgets();

	protected:
		// Callback for log_font browser
		static void cb_br_logfont(Fl_Widget* w, void* v);
		// Callback for log fontsize browser
		static void cb_br_logsize(Fl_Widget* w, void* v);
		// Callback for top font browser
		static void cb_br_tipfont(Fl_Widget* w, void* v);
		// Callback for tip fontsize browser
		static void cb_br_tipsize(Fl_Widget* w, void* v);
		// Populate font and size browsers
		void populate_font(Fl_Hold_Browser* b);
		void populate_size(Fl_Hold_Browser* b, Fl_Font* font, Fl_Fontsize* size);

		// Font for log_table contents
		Fl_Font log_font_;
		// Size of that font#
		Fl_Fontsize log_size_;
		// Duration a tool tip appears for (seconds)
		float tip_duration_;
		// Font used for tool tip
		Fl_Font tip_font_;
		// and its size
		Fl_Fontsize tip_size_;


	};

}
#endif


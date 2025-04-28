#ifndef __USER_DIALOG__
#define __USER_DIALOG__

#include "page_dialog.h"

#include <FL/Enumerations.H>

class Fl_Hold_Browser;
class Fl_Widget;

	// This class provides a dialog so that the user may modify the way certain
	// features (fonts, data formats etc) may be displayed 
	class user_dialog : public page_dialog
	{
	public:
		user_dialog(int X, int Y, int W, int H, const char* label);
		virtual ~user_dialog();
		// Load values from settings
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
		// Callback for top font browser
		static void cb_br_tipfont(Fl_Widget* w, void* v);
		// Call back for tree views
		static void cb_br_treefont(Fl_Widget* w, void* v);
		// Callback for all size browsers
		static void cb_br_size(Fl_Widget* w, void* v);
		// Populate font and size browsers
		void populate_font(Fl_Hold_Browser* b, const Fl_Font* font);
		void populate_size(Fl_Hold_Browser* b, const Fl_Font* font, const Fl_Fontsize* size);

		// Font for log_table contents
		Fl_Font log_font_;
		// Size of that font#
		Fl_Fontsize log_size_;
		// Duration a tool tip appears for (seconds)
		float tip_duration_;
		// Duration (in minutes) before a session is deemed closed
		float session_elapse_;
		// Font used for tool tip
		Fl_Font tip_font_;
		// and its size
		Fl_Fontsize tip_size_;
		// Font for tree-based views
		Fl_Font tree_font_;
		// and its size
		Fl_Fontsize tree_size_;
	};
#endif


#ifndef __USER_DIALOG__
#define __USER_DIALOG__

#include "page_dialog.h"

#include <FL/Enumerations.H>

class Fl_Hold_Browser;
class Fl_Widget;

	//! \brief This class provides a dialog so that the user may modify the way certain
	// features (fonts, data formats etc) may be displayed 
	class user_dialog : public page_dialog
	{
	public:
		//! Constructor

		//! \param X horizontal position within host window
		//! \param Y vertical position with hosr window
		//! \param W width 
		//! \param H height
		//! \param L label
		user_dialog(int X, int Y, int W, int H, const char* L);
		//! Destructor.
		virtual ~user_dialog();

		//! Inherited from page_dialog to allow keyboard F1 to open userguide.
		virtual int handle(int event);

		//! Load values from settings
		virtual void load_values();
		//! Instantiate coponent widgets.
		virtual void create_form(int X, int Y);
		//! Save values back to settings.
		virtual void save_values();
		//! Configure component widgets after data change.
		virtual void enable_widgets();

	protected:
		//! Callback from font browser for log_table
		static void cb_br_logfont(Fl_Widget* w, void* v);
		//! Callback from font browser for tooltips.
		static void cb_br_tipfont(Fl_Widget* w, void* v);
		//! Callback from font browser for Fl_Tree based widgets
		static void cb_br_treefont(Fl_Widget* w, void* v);
		//! Callback for all size browsers.
		static void cb_br_size(Fl_Widget* w, void* v);
		//! Populate font browser \p b std::set \p font as default selection.
		void populate_font(Fl_Hold_Browser* b, const Fl_Font* font);
		//! Populate size browser \p b with sizes available for \p font: std::set \p size as default selection.
		void populate_size(Fl_Hold_Browser* b, const Fl_Font* font, const Fl_Fontsize* size);

		//! Font for log_table contents
		Fl_Font log_font_;
		//! Size of log_table font.
		Fl_Fontsize log_size_;
		//! Duration a tooltip appears for (in seconds)
		float tip_duration_;
		//! Duration (in minutes) before a session is deemed closed
		float session_elapse_;
		//! Font used for tool tip
		Fl_Font tip_font_;
		//! Size of font used for tooltip.
		Fl_Fontsize tip_size_;
		//! Font for Fl_Tree derived views
		Fl_Font tree_font_;
		//! Size of font used for Fl_Tree derived views
		Fl_Fontsize tree_size_;
	};
#endif


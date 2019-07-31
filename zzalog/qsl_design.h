#ifndef __QSL_DESIGN__
#define __QSL_DESIGN__

#include "page_dialog.h"
#include "qsl_form.h"
#include "intl_widgets.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>

namespace zzalog {
	class qsl_design : public page_dialog
	{

	public:
		qsl_design(int X, int Y, int W, int H, const char* label);
		~qsl_design();
		// Display widget design
		void display_design_data(Fl_Button* widget, qsl_form::qsl_widget* data);

	protected:
		enum size_object {
			WIDTH,
			HEIGHT,
			TL_SIZE,
			TR_SIZE,
			BL_SIZE,
			BR_SIZE,
			TAB_ROWS,
			TAB_COLS
		};

		// Standard methods - need to be written for each class that inherits from this
// Load values from settings_
		virtual void load_values();
		// Used to create the form
		virtual void create_form(int X, int Y);
		// Used to write settings back
		virtual void save_values();
		// Used to enable/disable specific widget - any widgets enabled musr be attributes
		virtual void enable_widgets();

		// Handle
		virtual int handle(int event);

		// Callbacks
		// Any size change - double
		static void cb_vip_sized(Fl_Widget* w, void* v);
		// Any size changed - unsigned int
		static void cb_vip_sizeu(Fl_Widget* w, void* v);
		// Change to the text
		static void cb_ip_text(Fl_Widget* w, void* v);
		// Change to the font browser
		static void cb_br_font(Fl_Widget* w, void* v);
		// Change to the colour button
		static void cb_bn_colour(Fl_Widget* w, void* v);
		// Change to the font size
		static void cb_br_size(Fl_Widget* w, void* v);
		// Dimension radio 
		static void cb_radio_dim(Fl_Widget* w, void* v);

		// Populate font and size browsers
		void populate_font();
		void populate_size();

		// Current design data
		qsl_form* current_design_;
		// Current widget data being edited
		qsl_form::qsl_widget* current_data_;
		// Current widget
		Fl_Button* current_widget_;
		// Font and size selectors
		intl_input* ip_text_;
		Fl_Button* bn_colour_;
		Fl_Hold_Browser* br_font_;
		Fl_Hold_Browser* br_size_;
		Fl_Window* card_window_;

	};

}

#endif



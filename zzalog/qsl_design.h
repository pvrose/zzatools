#ifndef __QSL_DESIGN__
#define __QSL_DESIGN__

#include "page_dialog.h"
#include "qsl_form.h"
#include "intl_widgets.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Light_Button.H>

namespace zzalog {
	// This class allows the user to customise the way the QSL card is printed.
	// It allows a set of lines of text in the four corners of the card (used for operator and station info)
	// and a set of boxes across  the middle for QSO details
	class qsl_design : public page_dialog
	{

	public:
		qsl_design(int X, int Y, int W, int H, const char* label);
		~qsl_design();
		// Display widget design
		void display_design_data(Fl_Button* widget, qsl_form::qsl_widget* data);

	protected:
		// The various parameter types
		enum size_object {
			WIDTH,        // width of card
			HEIGHT,       // height of card
			TL_SIZE,      // number of lines of text in top-left
			TR_SIZE,      // do. in top right
			BL_SIZE,      // do. in bottom left
			BR_SIZE,      // do. in bottom right
			TAB_ROWS,     // Number of rows in the central box
			TAB_COLS,     // Number of columns in the central box
			ADDRESS       // Address label
		};

		// inherited from page_dialog
		// Load values from settings_
		virtual void load_values();
		// Used to create the form
		virtual void create_form(int X, int Y);
		// Used to write settings back
		virtual void save_values();
		// Used to enable/disable specific widget - any widgets enabled musr be attributes
		virtual void enable_widgets();

		// Inherited from Fl_Widget
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
		// Change to the font browser
		static void cb_br_font_add(Fl_Widget* w, void* v);
		// Change to the font size
		static void cb_br_size_add(Fl_Widget* w, void* v);
		// Set address label parameters
		static void cb_bn_address(Fl_Widget* w, void* v);
		// Show/Hide address label in window
		static void cb_bn_show_add(Fl_Widget* w, void* v);
		// Populate font and size browsers
		void populate_font(Fl_Hold_Browser* b);
		void populate_size(Fl_Hold_Browser* b);
		// Redraw the address in the window
		void redraw_address();

		// Current design data
		qsl_form* current_design_;
		// Current widget data being edited
		qsl_form::qsl_widget* current_data_;
		// Current widget - each line of text is omplemented as a box-less button
		Fl_Button* current_widget_;
		// Input that accepts pasting to edit current button label
		intl_input* ip_text_;
		// Button to change colour
		Fl_Button* bn_colour_;
		// Browser to select font
		Fl_Hold_Browser* br_font_;
		// Browser to select font size
		Fl_Hold_Browser* br_size_;
		// Surrounding window to display current design
		Fl_Window* card_window_;
		// Bureau address included
		bool include_address_label_;
		// Address 
		char** address_;
		// Number of lines
		unsigned int num_lines_;
		// Show/Hide address
		bool show_address_;
		// Address font
		Fl_Font font_add_;
		// Address size
		int size_add_;
		// Browser to select font (address)
		Fl_Hold_Browser* br_font_add_;
		// Browser to select font size (address)
		Fl_Hold_Browser* br_size_add_;
		// Text editor
		intl_editor* editor_;
		// Show/Hide button
		Fl_Light_Button* bn_show_add_;
		// Window to show address
		Fl_Button* bn_address_;
		// Number of rows in print
		int num_rows_;
		// Number of columns to print
		int num_cols_;
		// Column width
		float col_width_;
		// Row height
		float row_height_;
		// First column position
		float col_left_;
		// First row position
		float row_top_;

	};

}

#endif



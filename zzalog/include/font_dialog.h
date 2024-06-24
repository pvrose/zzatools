#pragma once

#include "win_dialog.h"

#include <string>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Output.H>

using namespace std;

	// Dialog to display a font chooser, size chooser and colour chooser
	class font_dialog :
		public win_dialog
	{
	public:
		font_dialog(Fl_Font f, Fl_Fontsize sz, Fl_Color c, const char* L = nullptr);
		virtual ~font_dialog();

        // Get the selected font
        Fl_Font font();
        // Get the selected size
        Fl_Fontsize font_size();
        // Get the selected colour
        Fl_Color colour();


		// callback - OK button
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// callback - cancel button (window close only)
		static void cb_bn_cancel(Fl_Widget* w, void * v);
        // Callback - font chooser
        static void cb_font(Fl_Widget* w, void* v);
        // size chooser
        static void cb_size(Fl_Widget* w, void* v);
        // Colour chooser
        static void cb_colour(Fl_Widget* w, void* v);

	protected:

        // Populate the font chooser
        void populate_font(Fl_Widget* w, const Fl_Font* f);
        // Populate the size chooser
        void populate_size(Fl_Widget* w, const Fl_Font* f, const Fl_Fontsize* sz);

        // Display a smaple in the selected style
        void set_sample();

        // Selecetd font
        Fl_Font font_;
        // Selected size
        Fl_Fontsize fontsize_;
        // Selected colour
        Fl_Color colour_;
        // Douput to display selected 
        Fl_Output* op_sample_;

	};

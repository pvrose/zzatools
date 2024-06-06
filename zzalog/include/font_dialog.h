#pragma once

#include "win_dialog.h"

#include <string>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Output.H>

using namespace std;

	// Dialog to provide program information
	class font_dialog :
		public win_dialog
	{
	public:
		font_dialog(Fl_Font f, Fl_Fontsize sz, Fl_Color c, const char* L = nullptr);
		virtual ~font_dialog();

        Fl_Font font();
        Fl_Fontsize font_size();
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

        void populate_font(Fl_Widget* w, const Fl_Font* f);
        void populate_size(Fl_Widget* w, const Fl_Font* f, const Fl_Fontsize* sz);

        void set_sample();

        Fl_Font font_;
        Fl_Fontsize fontsize_;
        Fl_Color colour_;

        Fl_Output* op_sample_;

	};

#pragma once

#include "win_dialog.h"

#include <string>

#include <FL/Enumerations.H>

using namespace std;

class Fl_Output;


	//! Dialog to display a font chooser, size chooser and colour chooser
	class font_dialog :
		public win_dialog
	{
	public:
        //! Constructor.
        
        //! \param f Current font.
        //! \param sz Current size of the font.
        //! \param c Current colour of the font.
        //! \param L Window label.
		font_dialog(Fl_Font f, Fl_Fontsize sz, Fl_Color c, const char* L = nullptr);
        //! Destructor.
		virtual ~font_dialog();

        //! Returns the selected font.
        Fl_Font font();
        //! Retunrs the selected size.
        Fl_Fontsize font_size();
        //! Returns the selected colour.
        Fl_Color colour();


		//! callback - OK button.
		static void cb_bn_ok(Fl_Widget* w, void* v);
		//! callback - cancel button (window close only).
		static void cb_bn_cancel(Fl_Widget* w, void * v);
        //! Callback - font chooser.
        static void cb_font(Fl_Widget* w, void* v);
        //! Callback - size chooser.
        static void cb_size(Fl_Widget* w, void* v);
        //! Callback - Colour chooser.
        static void cb_colour(Fl_Widget* w, void* v);

	protected:

        //! Populate the font chooser, setting current selection to \p f.
        void populate_font(Fl_Widget* w, const Fl_Font* f);
        //! Populate the size chooser, for font \p f, setting selection to \p sz.
        void populate_size(Fl_Widget* w, const Fl_Font* f, const Fl_Fontsize* sz);

        //! Display a sample in the selected font, size and colour.
        void set_sample();

        //! Selected font
        Fl_Font font_;
        //! Selected size
        Fl_Fontsize fontsize_;
        //! Selected colour
        Fl_Color colour_;
        //! Output to display sample in the selected font, size and colour. 
        Fl_Output* op_sample_;

	};

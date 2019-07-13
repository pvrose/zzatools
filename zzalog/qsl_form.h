#ifndef __QSL_FORM__
#define __QSL_FORM__

#include "record.h"

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

using namespace std;

// mm to points conversion
const float MM_TO_POINT = 72.0f / 25.4f;

namespace zzalog {

	class qsl_form : public Fl_Group
	{
		enum widget_type {
			TEXT,
			BOX
		};

		struct qsl_widget {
			string text;
			int x;
			int y;
			int w;
			int h;
			Fl_Color colour;
			int font_size;
			Fl_Font font;
			Fl_Align align;
			Fl_Boxtype type;
		};

	public:
		qsl_form(int x, int y, record* record);
		~qsl_form();

	protected:
		// Get the widget data - TODO: get from settings once we have a designer
		void load_data();
		// Draw the form
		void create_form();


	protected:
		// The record to be printed
		record* record_;
		// Width of all instances of qsl_card
		static int width_;
		// Height of all instances of qsl_card
		static int height_;
		// The widgets 
		static vector<qsl_widget> widget_data_;
		// Initialised
		static bool data_initialised_;

	};

}

#endif

#ifndef __QSL_FORM__
#define __QSL_FORM__

#include "record.h"

#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>

using namespace std;

// mm to points conversion
const float MM_TO_POINT = 72.0f / 25.4f;
const float IN_TO_POINT = 72.0f;

namespace zzalog {

	class qsl_form : public Fl_Group
	{
	public:
		// Units of measurement
		enum dim_unit {
			INCH,       // = 25.4 mm
			MILLIMETER,
			POINT       // = 1/72 in.
		};

		// Data for an individual widget
		struct qsl_widget {
			string text;
			Fl_Color colour;
			int font_size;
			Fl_Font font;
		};

		// Widget set position
		enum widget_set {
			TOP_LEFT,
			TOP_RIGHT,
			TABLE,
			BOTTOM_LEFT,
			BOTTOM_RIGHT
		};

		const int MARGIN = 10;

	public:
		qsl_form(int x, int y, record* record);
		~qsl_form();
		// Resize widget set
		void resize_set(widget_set set, int rows);
		// Resize table
		void resize_table(int rows, int cols);
		// Change widget data
		void update_text(qsl_widget* widget, string value);
		void update_font(qsl_widget* widget, Fl_Font value);
		void update_size(qsl_widget* widget, int value);
		void update_colour(qsl_widget* widget, Fl_Color value);
		// Return unit
		dim_unit unit();
		void unit(dim_unit unit);
		// Return width
		float width();
		void width(float width);
		// Return height
		float height();
		void height(float heigth);
		// Return size
		int set_size(widget_set set);
		// Returb table rows
		int table_rows();
		int table_cols();

		// Save data
		void save_data();
		// Write one set of settings
		void write_settings(Fl_Preferences& settings, vector<qsl_form::qsl_widget>&);
		// Set the designer
		void designer(Fl_Group* designer);


	protected:
		// Callbacks - when clicking any item in the form
		static void cb_button(Fl_Widget* w, void* v);
		// Get the widget data - TODO: get from settings once we have a designer
		void load_data();
		// Default data initialise
		void load_default();
		// Get one set of settings
		void read_settings(Fl_Preferences& settings, vector<qsl_widget>&, int count);
		// Draw the form
		void create_form();
		// Draw one set of lines
		void draw_lines(Fl_Align align, int& y, vector<qsl_widget>& widgets);
		// Draw a table
		void draw_table(int& y);
		// Convert to points
		int to_points(float value);
		// Update the widget sets
		void update();




	protected:
		// The card design interface
		Fl_Group* designer_;
		// The record to be printed
		record* record_;
		// Width of all instances of qsl_card
		float width_;
		// Height of all instances of qsl_card
		float height_;
		// Unit of width and height
		dim_unit unit_;
		// The widgets 
		vector<qsl_widget> tl_widgets_;
		vector<qsl_widget> tr_widgets_;
		vector<vector<qsl_widget> > tab_widgets_;
		vector<qsl_widget> bl_widgets_;
		vector<qsl_widget> br_widgets_;

	};

}

#endif

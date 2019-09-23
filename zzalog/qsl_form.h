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

	// THis class defines the current QSL design for printing and displaying
	class qsl_form : public Fl_Group
	{
	public:
		// Units of measurement
		enum dim_unit {
			INCH,       // = 25.4 mm
			MILLIMETER,
			POINT       // = 1/72 in.
		};

		// Data for an individual widget (item of text on the card)
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

		// Default margin
		const int MARGIN = 10;

	public:
		// Constructor provides X and Y position (W and H set from design data) and current records to display in card format
		qsl_form(int x, int y, record** records, int num_records);
		~qsl_form();
		// Resize widget set - number of lines of text
		void resize_set(widget_set set, int rows);
		// Resize table - rows x columns
		void resize_table(int rows, int cols);
		// Change widget text
		void update_text(qsl_widget* widget, string value);
		// Change widget font
		void update_font(qsl_widget* widget, Fl_Font value);
		// Change size of font used
		void update_size(qsl_widget* widget, int value);
		// Change the text colour
		void update_colour(qsl_widget* widget, Fl_Color value);
		// Return unit
		dim_unit unit();
		// Set unit
		void unit(dim_unit unit);
		// Return width
		float width();
		// Set width
		void width(float width);
		// Return height
		float height();
		// Set height
		void height(float heigth);
		// Return size
		int set_size(widget_set set);
		// Return number of rows
		int table_rows();
		// Return number of columns
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
		// Get the widget data
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
		// The records to be printed
		record** records_;
		// Number of them
		int num_records_;
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

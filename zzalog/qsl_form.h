#ifndef __QSL_FORM__
#define __QSL_FORM__

#include "record.h"
#include "drawing.h"
#include "qsl_html_view.h"
#include <string>
#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Preferences.H>

using namespace std;
using namespace zzalib;

// mm to points conversion: 1" = 72pt = 25.4 mm
const float MM_TO_POINT = 72.0f / 25.4f;
// inch to points - 1" = 72 pt
const float IN_TO_POINT = 72.0f;

namespace zzalog {

	// This class defines the current QSL design for printing and displaying
	class qsl_form : public Fl_Group
	{
	public:
		// Units of measurement
		enum dim_unit {
			INCH,       // = 25.4 mm
			MILLIMETER,
			POINT       // = 1/72 in.
		};

		//// Data for an individual widget (item of text on the card)
		//struct qsl_widget {
		//	string text;
		//	Fl_Color colour;
		//	int font_size;
		//	Fl_Font font;

		//	qsl_widget() :
		//		text(""),
		//		colour(FL_BLACK),
		//		font_size(FONT_SIZE),
		//		font(FONT) {};
		//	qsl_widget(string t, Fl_Color c, int s, Fl_Font f) :
		//		text(t),
		//		colour(c),
		//		font_size(s),
		//		font(f) {};
		//};

		//// Widget set position
		//enum widget_set {
		//	TOP_LEFT,
		//	TOP_RIGHT,
		//	TABLE,
		//	BOTTOM_LEFT,
		//	BOTTOM_RIGHT
		//};

		//// Default margin
		//const int MARGIN = 10;

	public:
		// Constructor provides X and Y position (W and H set from design data) and current records to display in card format
		qsl_form(int x, int y, record** records, int num_records);
		~qsl_form();
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
		// Size error drawing card
		bool size_error();

	protected:
		// Get the widget data
		void load_data();
		// Draw the form
		void create_form(int X, int Y);
		// Convert to points
		int to_points(float value);

	protected:
		// The records to be printed
		record** records_;
		// Number of them
		int num_records_;
		// Width of each instance of qsl_card
		float width_;
		// Height of each instance of qsl_card
		float height_;
		// Unit of width and height
		dim_unit unit_;
		// Size error
		bool size_error_;
		// HTML Filename
		string filename_;
		// The QSL card HTML view
		qsl_html_view* card_view_;
	};

}

#endif

#ifndef __QSL_DESIGN__
#define __QSL_DESIGN__

#include "qsl_form.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Light_Button.H>

namespace zzalog {
	// This class allows the user to customise the way the QSL card is printed.
	class qsl_design : public Fl_Window
	{

	public:
		qsl_design(int X, int Y, int W, int H, const char* callsign);
		~qsl_design();

	protected:
		// Load values from settings_
		void load_values();
		// Used to create the form
		void create_form(int X, int Y);
		// Used to write settings back
		void save_values();

		// Callbacks
		// Dimension radio 
		static void cb_radio_dim(Fl_Widget* w, void* v);
		// OK button
		static void cb_bn_ok(Fl_Widget* w, void* v);
		// Cancel button
		static void cb_bn_can(Fl_Widget* w, void* v);

		// Number of rows in print
		int num_rows_;
		// Number of columns to print
		int num_cols_;
		// dimension usnits
		qsl_form::dim_unit unit_;
		// Label width
		double width_;
		// Label height
		double height_;
		// Column width
		double col_width_;
		// Row height
		double row_height_;
		// First column position
		double col_left_;
		// First row position
		double row_top_;
		// Number of QSOs per card
		int number_qsos_;
		// Callsign to read parameters
		string callsign_;

	};

}

#endif



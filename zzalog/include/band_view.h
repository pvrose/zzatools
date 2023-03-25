#ifndef __BAND_VIEW__
#define __BAND_VIEW__

#include "record.h"

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Window.H>

using namespace std;



	// Constants
	// Number of pixels per division on the horizontal scale
	const int PIXELS_PER_TICK = 10;
	// POssible sub-division values for intermediate divisions
	const double POSS_MINOR[] = { 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1., 2., 5., 10., 20., 50., 100., 200., 500. };
	// Possible values for the larger sub-divisions
	const double POSS_MAJOR[] = { 0.1, 1., 10., 100., 1000. };
	// Number of values
	const int NUM_ZOOMS = sizeof(POSS_MINOR) / sizeof(double);

	// This class provides a stand-alone window that displays the band-plan around the current
	// frequency
	class band_view :
		public Fl_Window
	{
	protected:
		// Band entry structure
		struct band_entry_t {
			bool is_spot;       // Entry is a spot frequency
			double lower;       // Lower end of sub-band (kHz)
			double upper;       // Upper end of sub-band (kHz)
			double bandwidth;   // Maximum bandwidth usable in sub-band
			string mode;        // Modes allowed
			string notes;       // Notes about usage
			// Default constructor
			band_entry_t() :
				is_spot(false),
				lower(0.0),
				upper(0.0),
				bandwidth(0.0),
				mode(""),
				notes("") {};
		};

	public:

		band_view(double frequency, int W = 400, int H = 100, const char* label = nullptr);
		~band_view();

		// Redraw the view with the current frequency
		void update(double frequency);
		// Redraw the view from the selected record
		void update(qso_num_t record_num);

		// Redeclaration of draw
		virtual void draw();

		// zoom roller callback
		static void cb_roll_zoom(Fl_Widget* w, void* v);
		// Frequency roller callback
		static void cb_redraw_value(Fl_Widget* w, void* v);
		// reset button callback
		static void cb_bn_reset(Fl_Widget* w, void* v);
		// Overloaded to handle mouse actions on band plan
		virtual int handle(int event);

		// Data loaded OK
		bool valid();
		// is the frequency supplied in band?
		bool in_band(double frequency);

	protected:
		// Add the fixed widgets
		void create_form();
		// Read the data
		bool load_data();
		// Get the file name
		string get_path();
		// Parse the band entry
		band_entry_t* get_entry(string line);
		// Draw the scale - returns position of bottom of scale
		int draw_scale();
		// Draw the band-plan
		int draw_plan(int pos_y);
		// Draw the current frequency
		int draw_frequency(int pos_y);
		// Convert frequency to x
		int x_pos(double frequency);
		// Convert frequency to text
		void freqy_to_text(double frequency, char* text);
		// Recalculate cycle_per_minor and major from zoom value
		void recalculate(double frequency);
		// Display band information
		void display_info(double frequency, int x, int y);
		// Convert x to frequency
		double x_frequency(int x);
		// Add title
		void title();

		// The current rig frequency
		double rig_frequency_;
		// The current target frequency
		double frequency_;
		// Tracking rig frequency - changes frequency when rig_frequncy changes
		bool tracking_rig_;
		// The band entries
		vector<band_entry_t*> entries_;
		// Drawing area left hand edge
		int pixel_lhs_;
		// Drawing area right hand edge
		int pixel_rhs_;
		// Drawing area - top
		int pixel_top_;
		// Drawing area - bottom
		int pixel_bottom_;
		// Left hand frequency
		double freqy_lhs_;
		// Right hand frequency
		double freqy_rhs_;
		// Number of kHz per minor tick
		double khz_per_minor_;
		// Number of kHz per major tick
		double khz_per_major_;
		// Zoom value
		int zoom_value_;
		// Mid frequency
		double mid_frequency_;
		// Frequency step value
		double step_value_;
		// Number ticks
		int num_ticks_;
		// Frequency slider
		Fl_Widget* w_freq_slider_;
		// Cycle display
		Fl_Widget* w_cycles_per_;
		// Minimum freaquency supported
		double min_frequency_;
		// Maximum frequency supported
		double max_frequency_;
		// Colour palette for modes
		map<string, int> mode_palette_;
		// Colour palette for bandwidths
		map<double, int> bw_palette_;
		// Valid band-plan
		bool valid_;

	};
#endif
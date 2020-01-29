#ifndef __WINDOW__
#define __WINDOW__

#include "../zzalib/serial.h"
#include "../zzalib/callback.h"

#include <string>

#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Counter.H>
#include <FL/fl_draw.H>

using namespace zzalib;

namespace zzaportm {

	//constants and types
	// timeout constants (in s)
	const double MAX_TIMEOUT = 10.0;
	const double DEF_TIMEOUT = 1.0;
	const double MIN_TIMEOUT = 0.1;
	// timeout counter steps - small and large
	const double STEPS_TIMEOUT = 0.1;
	const double STEPL_TIEMOUT = 1.0;
	// number requests
	const unsigned int MAX_REQUESTS = 10;
	const unsigned int DEF_REQUESTS = 5;
	const unsigned int MIN_REQUESTS = 0; // Zero used for indefinite
	// counter step
	const unsigned int STEP_REQUESTS = 1;
	// Display formats
	const Fl_Text_Display::Style_Table_Entry STYLES[] = {
		{ FL_BLACK, FL_COURIER, 12, 0 },                // A - Timestamp
		{ FL_BLUE, FL_COURIER, 12, 0 },                 // B - transmit
		{ fl_darker(FL_GREEN), FL_COURIER, 12, 0 },     // C - Receive
		{ FL_RED, FL_COURIER_BOLD, 12, 0 }              // D - Error
	};


	class window :
		public Fl_Window
	{
		// Display direction
		enum direction {
			TX,
			RX,
			TX_ERROR,
			RX_ERROR,
			OPEN_ERROR
		};
		// Format
		enum format : int {
			HEX,
			ASCII
		};
		// Mode 
		enum mode : int {
			TX_RX,
			RX_ONLY
		};
	public:
		window(int W, int H, const char* label);
		~window();

	protected:

		// Callbacks

		// Baud choice
		void static cb_ch_baud(Fl_Widget* w, void* v);
		// RX timeout spinner
		void static cb_sp_timeout(Fl_Widget* w, void* v);
		// Number tries spinner
		void static cb_sp_number(Fl_Widget* w, void* v);
		// Go button
		void static cb_bn_go(Fl_Widget* w, void* v);
		// Format button
		void static cb_bn_format(Fl_Widget* w, void* v);
		// Mode button
		void static cb_bn_mode(Fl_Widget* w, void* v);
		// Clear button
		void static cb_bn_clear(Fl_Widget* w, void* v);
		// Stop button
		void static cb_bn_stop(Fl_Widget* w, void* v);

		// Methods

		// The method to create the forms
		void create_form();
		// Make serial request
		void send_request(string data);
		// Display the transmitted/received data
		void display_data(direction direction, string data);
		// Populate the port choice
		void populate_port_choice(Fl_Choice* ch);
		// Populate the baud choice
		void populate_baud_choice(Fl_Choice* ch);

		//attributes
		serial* port_;
		// Port name
		string port_name_;
		// Baud rate
		unsigned int baud_rate_;
		// Transmit data
		string data_;
		// Format
		format format_;
		// Receive timeout
		unsigned int rx_timeout_;
		// Number of requests
		unsigned int num_requests_;
		// Mode
		mode mode_;
		// Stop button pressed
		bool stopped_;
		// The text display
		Fl_Text_Display* display_;
		// The text buffer
		Fl_Text_Buffer* buffer_;
		// The style buffer
		Fl_Text_Buffer* style_buffer_;
		// Stop button
		Fl_Button* bn_stop_;
		// Clear button
		Fl_Button* bn_clear_;
		// Requests counter
		Fl_Counter* ctr_requests_;

	};

}

#endif
#ifndef __WINDOW__
#define __WINDOW__

#include "../zzalib/serial.h"
#include "../zzalib/callback.h"

#include <string>

#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_draw.H>

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
	const unsigned int MIN_REQUESTS = 1;
	// counter step
	const unsigned int STEP_REQUESTS = 1;
	// Display direction
	enum direction {
		TX,
		RX,
		TX_ERROR,
		RX_ERROR,
		OPEN_ERROR
	};
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
		enum { HEX, ASCII } format_;
		// Receive timeout
		unsigned int rx_timeout_;
		// Number of requests
		unsigned int num_requests_;
		// Radio button parameters
		radio_param_t parameters_[2];
		// The text display
		Fl_Text_Display* display_;
		// The text buffer
		Fl_Text_Buffer* buffer_;
		// The style buffer
		Fl_Text_Buffer* style_buffer_;

	};

}

#endif
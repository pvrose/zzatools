#pragma once

#include "record.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Box.H>



    class qsl_viewer :
        public Fl_Window
    {		
		
		// The specific image required - determines the file location of the image file
		enum image_t {
			QI_EQSL,            // Downloaded eQSL image
			QI_CARD_FRONT,      // Scanned-in front of a card
			QI_CARD_BACK,       // Scanned-in back of a card
			QI_GEN_CARD,        // Generated QSL card
			QI_TEXT             // Display an Fl_Text_Display
		};

	public:
		qsl_viewer(int W, int H, const char* L = nullptr);
		~qsl_viewer();

		// Card radio button
		static void cb_rad_card(Fl_Widget* w, void* v);
		// Fetch eQSL button has been clicked
		static void cb_bn_fetch(Fl_Widget* w, void* v);
		// Stretcch/Scale button has been clicked
		static void cb_bn_scale(Fl_Widget*, void* v);
		// Log Card button has been clicked
		static void cb_bn_log_card(Fl_Widget* w, void* v);

		// Set record
		void set_qso(record* qso, record_num_t number);
	
	protected:
		// Access to selected record (for use in callback)
		void set_selected_image(image_t image);
		// set the image data
		void set_image();
		// set the image radio buttons
		void set_image_buttons();
		// draw the image
		void draw_image();
		// enable the widgets according to current use mode
		void update_widgets();
		// Set QSL status widgets
		void set_qsl_status();

	protected:

		// the source of the displayed image
		image_t selected_image_;
		// the image file
		Fl_Image* image_;
		// Scale or stretch the image
		bool scaling_image_;
		// Current QSO
		record* current_qso_;
		// Current QSO number
		record_num_t current_qso_num_;

		// Card image and info
		Fl_Group* card_display_;
		Fl_Box* card_filename_out_;
		Fl_Light_Button* keep_bn_;
		// Card controls
		Fl_Group* card_type_grp_;
		Fl_Radio_Light_Button* eqsl_radio_;
		Fl_Radio_Light_Button* card_front_radio_;
		Fl_Radio_Light_Button* card_back_radio_;
		Fl_Radio_Light_Button* gen_card_radio_;
		Fl_Button* fetch_bn_;
		Fl_Button* log_card_bn_;
		Fl_Light_Button* scale_bn_;
		// Text display
		Fl_Text_Display* text_display_;
		// QSL status
		Fl_Box* eqsl_status_box_;
		Fl_Box* lotw_status_box_;


	};


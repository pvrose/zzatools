#pragma once

#include "record.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Box.H>

class qso_qsl_vwr :
    public Fl_Group
{
	// The specific image required - determines the file location of the image file
	enum image_t {
		QI_EQSL,            // Downloaded eQSL image
		QI_CARD_FRONT,      // Scanned-in front of a card
		QI_CARD_BACK,       // Scanned-in back of a card
	};

public:
	qso_qsl_vwr(int X, int Y, int W, int H, const char* L = nullptr);
	~qso_qsl_vwr();

	// get settings
	void load_values();
	// Create form
	void create_form();
	// Enable/disab;e widgets
	void enable_widgets();
	// save value
	void save_values();

	// Image button
	static void cb_bn_image(Fl_Widget* w, void* v);
	// Card radio button
	static void cb_rad_card(Fl_Widget* w, void* v);
	// Fetch eQSL button has been clicked
	static void cb_bn_fetch(Fl_Widget* w, void* v);
	// Log Card button has been clicked
	static void cb_bn_log_card(Fl_Widget* w, void* v);

	// Set record
	void set_qso(record* qso, qso_num_t number);

protected:
	// Access to selected record (for use in callback)
	void set_selected_image(image_t image);
	// set the image data
	void set_image();
	// set the image radio buttons
	void set_image_buttons();
	// draw the image
	void draw_image();
	// Set QSL status widgets
	void set_qsl_status();
	// Update full window
	void update_full_view();

protected:

	// the source of the displayed image
	image_t selected_image_;
	// the image file
	Fl_Image* raw_image_;
	// Image scaled to fit button
	Fl_Image* scaled_image_;
	// Current QSO
	record* current_qso_;
	// Current QSO number
	qso_num_t current_qso_num_;
	// Directory string
	string qsl_directory_;
	// Fullname 
	string full_name_;

	// Card image and info
	Fl_Button* card_display_;
	// Card controls
	Fl_Group* card_type_grp_;
	Fl_Radio_Light_Button* eqsl_radio_;
	Fl_Radio_Light_Button* card_front_radio_;
	Fl_Radio_Light_Button* card_back_radio_;
	Fl_Button* fetch_bn_;
	Fl_Button* log_card_bn_;
	// QSL status
	Fl_Box* eqsl_status_box_;
	Fl_Box* lotw_status_box_;
	Fl_Box* card_status_box_;
	// Window to contain full view
	Fl_Window* win_full_view_;
	// Button
	Fl_Button* bn_full_view_;
	Fl_Button* bn_no_image_;


};

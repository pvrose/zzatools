#pragma once

#include <string>

#include <FL/Fl_Group.H>

using namespace std;

class record;
class qsl_display;
class Fl_Button;
class Fl_Light_Button;
class Fl_Radio_Light_Button;
class Fl_Image;

typedef size_t qso_num_t;

// This displays the QSL status of this QSO.
// It displays images received and the label that can be printed
// It allows update of QSL status
class qso_qsl_vwr :
    public Fl_Group
{
	// The specific image required - determines the file location of the image file
	enum image_t {
		QI_NONE,            // No image
		QI_EQSL,            // Downloaded eQSL image
		QI_CARD_FRONT,      // Scanned-in front of a card
		QI_CARD_BACK,       // Scanned-in back of a card
		QI_EMAIL,          	// e-mailed image
		QI_MY_QSL,          // My QSL image for QSO
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
	// Set "Card requested"
	static void cb_bn_card_reqd(Fl_Widget* w, void* v);
	// My QSL button
	static void cb_bn_myqsl(Fl_Widget* w, void* v);

	// Set record
	void set_qso(record* qso, qso_num_t number);

protected:
	// Access to selected record (for use in callback)
	void set_selected_image(image_t image);
	// set the image data
	void set_image();
	// set the image radio buttons
	void set_image_buttons();
	// Set the log buttons
	void set_log_buttons();
	// draw the image
	void draw_image();
	// Set QSL status widgets
	void set_qsl_status();
	// Update full window
	void update_full_view();
	// Set full view size
	void resize_full_view();

protected:

	// the source of the displayed image
	image_t selected_image_;
	// the image file
	Fl_Image* raw_image_;
	// Image scaled to fit button
	Fl_Image* scaled_image_;
	// Desaturated image for deactivated dispaly
	Fl_Image* desat_image_;
	// Current QSO
	record* current_qso_;
	// Current QSO number
	qso_num_t current_qso_num_;
	// Directory string
	string qsl_directory_;
	// Fullname 
	string full_name_;

	// Card image and info
	Fl_Button* bn_card_display_;
	// Card controls
	Fl_Group* grp_card_type_;
	Fl_Radio_Light_Button* radio_eqsl_;
	Fl_Radio_Light_Button* radio_card_front_;
	Fl_Radio_Light_Button* radio_card_back_;
	Fl_Radio_Light_Button* radio_email_;
	Fl_Radio_Light_Button* radio_myqsl_;
	Fl_Button* bn_fetch_;
	Fl_Button* bn_log_bureau_;
	Fl_Button* bn_log_email_;
	Fl_Button* bn_log_direct_;
	Fl_Button* bn_card_reqd_;
	Fl_Button* bn_card_decl_;
	// QSL status
	Fl_Light_Button* bn_eqsl_status_;
	Fl_Light_Button* bn_lotw_status_;
	Fl_Light_Button* bn_card_status_;
	// Window to contain full view
	Fl_Window* win_full_view_;
	// Button
	Fl_Button* bn_full_view_;
	Fl_Button* bn_no_image_;
	qsl_display* display_myqsl_;


};

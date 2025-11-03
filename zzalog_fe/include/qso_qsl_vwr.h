#pragma once

#include <string>

#include <FL/Fl_Group.H>



class record;
class qsl_widget;

class Fl_Button;
class Fl_Check_Button;
class Fl_Image;
class Fl_Radio_Light_Button;
class Fl_Tabs;


typedef size_t qso_num_t;

//! This class displays the QSL status of this QSO.

//! It displays images received and the label that can be printed
//! It allows update of QSL status
class qso_qsl_vwr :
    public Fl_Group
{
	//! The specific image required - determines the file location of the image file
	enum image_t {
		QI_NONE,            //!< No image
		QI_EQSL,            //!< Downloaded eQSL image
		QI_CARD_FRONT,      //!< Scanned-in front of a card
		QI_CARD_BACK,       //!< Scanned-in back of a card
		QI_EMAILR,         	//!< received e-mailed image
		QI_LABEL,           //!< Image for printing label for cards
		QI_EMAILS,          //!< Image for outgoing e-mail

	};

public:
	//! Constructor.

	//! \param X horizontal position within host window
	//! \param Y vertical position with hosr window
	//! \param W width 
	//! \param H height
	//! \param L label
	qso_qsl_vwr(int X, int Y, int W, int H, const char* L = nullptr);
	//! Destructor.
	~qso_qsl_vwr();

	//! Inherited from Fl_Group::handle: allows keyboard F1 to open userguide.
	virtual int handle(int event);

	//! Load QSL directory and previous usage from settings.
	void load_values();
	//! Instantiate component widgets.
	void create_form();
	//! Configure component widgets after data change
	void enable_widgets();
	//! Save current usage to settings.
	void save_values();

	//! Callback from QSO image: opens or closes a window with a full-size image.
	static void cb_bn_image(Fl_Widget* w, void* v);
	//! Callback from image select radio buttons: /p v indicates image_t type
	static void cb_rad_card(Fl_Widget* w, void* v);
	//! Callback from "Fetch" button: fetches eQSL image.
	static void cb_bn_fetch(Fl_Widget* w, void* v);
	//! Callback from "Bureau", "e-Mail" or "Direct" buttons: /p v indicates which.
	static void cb_bn_log_card(Fl_Widget* w, void* v);
	//! Callback from "Requested" or "Declined": /p v indicates which.
	static void cb_bn_card_reqd(Fl_Widget* w, void* v);
	//! Callback when switching tabs: formats labels.
	static void cb_tabs(Fl_Widget* w, void* v);
	//! Callback from QSL Declined (in Status): /p v indicates the QSL server
	static void cb_bn_decline(Fl_Widget* w, void* v);

	//! Set QSO record \p qso and its index \p number.
	void set_qso(record* qso, qso_num_t number);

protected:
	//! Set the selected \p image to be displayed.
	void set_selected_image(image_t image);
	//! Add the selected image to the display widget.
	void set_image();
	//! Configure image select radio buttons.
	void set_image_buttons();
	//! Enable the "Edit" tab.
	void set_log_buttons();
	//! Configure the QSL received sttaus buttons.
	void set_qsl_status();
	//! Set the *declined widgets
	void set_declines();
	//! Update the full-size image display window.
	void update_full_view();

protected:

	//! the source of the displayed image
	image_t selected_image_;
	//! the image file
	Fl_Image* raw_image_;
	//! Current QSO
	record* current_qso_;
	//! Current QSO index
	qso_num_t current_qso_num_;
	//! QSL card Directory std::string
	std::string qsl_directory_;
	//! Full filanem of the QSL card image.
	std::string full_name_;
	//! Flag is true if selected QSO is true.
	bool qso_changed_;

	//! Tabs: "Display", "Status", and "Edit"
	Fl_Tabs* tabs_;

	//! "Display" tab.
	Fl_Group* grp_viewer_;
	//! Card image and info
	qsl_widget* qsl_thumb_;
	//! Card controls
	Fl_Group* grp_card_type_;                 //!< Radio button group.
	Fl_Radio_Light_Button* radio_eqsl_;       //!< Light: view eQSL image
	Fl_Radio_Light_Button* radio_card_front_; //!< Light: view Card front
	Fl_Radio_Light_Button* radio_card_back_;  //!< Light: view Card back
	Fl_Radio_Light_Button* radio_emailr_;     //!< Light: view e_Mail received
	Fl_Radio_Light_Button* radio_label_;      //!< Light: view label to be printed
	Fl_Radio_Light_Button* radio_emails_;     //!< Light: view e-Mail to be sent.
	// "Status" tab.
	Fl_Group* grp_status_;                    //!< QSL status pane.
	Fl_Check_Button* bn_eqsl_rstatus_;        //!< Check: eQSL received.
	Fl_Check_Button* bn_lotw_rstatus_;        //!< Check: LotW received
	Fl_Check_Button* bn_qrz_rstatus_;         //!< Check: QRZ.com received
	Fl_Check_Button* bn_card_rstatus_;        //!< Check: card received.
	Fl_Check_Button* bn_eqsl_sstatus_;        //!< Check: eQSL sent
	Fl_Check_Button* bn_lotw_sstatus_;        //!< Check: LotW sent.
	Fl_Check_Button* bn_qrz_sstatus_;         //!< Check: QRZ.com sent
	Fl_Check_Button* bn_club_sstatus_;        //!< Check: Clublog sent.
	Fl_Check_Button* bn_card_sstatus_;        //!< Check: Card sent.
	Fl_Check_Button* bn_eqsl_decline_;        //!< Check: Decline eQSL
	Fl_Check_Button* bn_lotw_decline_;        //!< Check: Decline LotW
	Fl_Check_Button* bn_qrz_decline_;         //!< Check: Decline QRZ.com
	Fl_Check_Button* bn_club_decline_;        //!< Check: Decline Clublog
	Fl_Check_Button* bn_card_decline_;        //!< Check: Decline card
	// "Edit" tab
	Fl_Group* grp_editor_;                    //!< Pane to allow status to be edited.
	Fl_Button* bn_fetch_;                     //!< Button: Fetch image from eQSL.cc
	Fl_Button* bn_log_bureau_;                //!< Button: Log card received from bureau.
	Fl_Button* bn_log_email_;                 //!< Button: Log image received by e-Mail.
	Fl_Button* bn_log_direct_;                //!< Button: Log card received direct.
	Fl_Button* bn_card_reqd_;                 //!< Button: Log card requested via bureau.
	Fl_Button* bn_card_decl_;                 //!< Button: Log card declined. (Do not send)

	//! Window to contain full-size image.
	Fl_Window* win_full_view_;
	//! qsl_widget container for full-size image.
	qsl_widget* qsl_full_;
};

#include "web_dialog.h"
#include "utils.h"
#include "calendar.h"

#include "icons.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_RGB_Image.H>

using namespace zzalog;

extern Fl_Preferences* settings_;

// Constructor
web_dialog::web_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)

	, eqsl_cal_cb_data_(cal_cb_data_t())
	, lotw_cal_cb_data_(cal_cb_data_t())
	, eqsl_enable_(false)
	, eqsl_last_got_("")
	, eqsl_username_("")
	, eqsl_password_("")
	, eqsl_use_qso_msg_(false)
	, eqsl_qso_msg_("")
	, eqsl_use_swl_msg_(false)
	, eqsl_swl_msg_("")
	, lotw_enable_(false)
	, lotw_last_got_("")
	, lotw_username_("")
	, lotw_password_("")
	, qrz_enable_(false)
	, qrz_username_("")
	, qrz_password_("")
{
	image_widgets_.clear();

	do_creation(X, Y);
}

// Destructor
web_dialog::~web_dialog()
{
	// For each widget with images
	for (auto it = image_widgets_.begin(); it != image_widgets_.end(); it++) {
		// Delete the image
		Fl_RGB_Image* i = (Fl_RGB_Image*)(*it)->image();
		if (i != nullptr) {
			delete i;
			(*it)->image(nullptr);
		}
	}
}

// Load initial values from settings
void web_dialog::load_values() {
	// Get the various settings
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences eqsl_settings(qsl_settings, "eQSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	Fl_Preferences qrz_settings(qsl_settings, "QRZ");
	Fl_Preferences card_settings(qsl_settings, "Card");

	// eQSL User/Password
	eqsl_settings.get("Enable", (int&)eqsl_enable_, false);
	char * temp;
	eqsl_settings.get("User", temp, "");
	eqsl_username_ = temp;
	free(temp);
	eqsl_settings.get("Password", temp, "");
	eqsl_password_ = temp;
	free(temp);
	eqsl_settings.get("Last Accessed", temp, "");
	eqsl_last_got_ = temp;
	free(temp);
	// LotW settings
	lotw_settings.get("Enable", (int&)lotw_enable_, false);
	lotw_settings.get("User", temp, "");
	lotw_username_ = temp;
	free(temp);
	lotw_settings.get("Password", temp, "");
	lotw_password_ = temp;
	free(temp);
	lotw_settings.get("Last Accessed", temp, "");
	lotw_last_got_ = temp;
	free(temp);

	// QRZ.com User/Password
	qrz_settings.get("Enable", (int&)qrz_enable_, false);
	qrz_settings.get("User", temp, "");
	qrz_username_ = temp;
	free(temp);
	qrz_settings.get("Password", temp, "");
	qrz_password_ = temp;
	free(temp);

	// QSL message settings (for eQSL and card)
	card_settings.get("QSL Enable", (int&)eqsl_use_qso_msg_, false);
	card_settings.get("QSL Message", temp, "Tnx QSO <NAME>, 73");
	eqsl_qso_msg_ = temp;
	free(temp);
	card_settings.get("SWL Enable", (int&)eqsl_use_swl_msg_, false);
	card_settings.get("SWL Message", temp, "Tnx SWL Report <NAME>, 73");
	eqsl_swl_msg_ = temp;
	free(temp);

}

// Create the dialog
void web_dialog::create_form(int X, int Y) {
	// position constants
	// Group 1 eQSL widgets
	const int GRP1 = EDGE;
	const int R1_1 = GRP1 + GAP + HTEXT;
	const int H1_1 = HBUTTON;
	const int R1_2 = R1_1 + H1_1 + GAP;
	const int H1_2 = HBUTTON;
	const int R1_3 = R1_2 + H1_2 + GAP;
	const int H1_3 = HBUTTON;
	const int HGRP1 = R1_3 + H1_3 + GAP - GRP1;

	// Group 2 LotW widgets
	const int GRP2 = GRP1 + HGRP1 + GAP;
	const int R2_1 = GRP2 + GAP + HTEXT;
	const int H2_1 = HBUTTON;
	const int HGRP2 = R2_1 + H2_1 + GAP - GRP2;

	// Group 3 QRZ.com widgets
	const int GRP3 = GRP2 + HGRP2 + GAP;
	const int R3_1 = GRP3 + GAP + HTEXT;
	const int H3_1 = HBUTTON;
	const int HGRP3 = R3_1 + H3_1 + GAP - GRP3;

	// overall height 
	const int HALL = GRP3 + HGRP3 + EDGE;

	// Columns
	// main columns
	const int C1 = EDGE + GAP;
	const int W1 = HBUTTON; // square check button
	const int C2 = C1 + W1 + GAP;
	const int W2 = WSMEDIT;
	const int C3 = C2 + W2 + GAP;
	const int W3 = HBUTTON * 3 / 2;
	const int C4 = C3 + W3 + GAP;
	const int W4 = WSMEDIT;
	const int C5 = C4 + W4 + GAP;
	const int W5 = WSMEDIT;

	// offset columns
	const int C2A = C1 + W1 + WLLABEL + GAP;
	const int W2A = WMESS;

	// overall width 
	const int WGRP = max(C5 + W5, C2A + W2A) + GAP;
	const int WALL = WGRP + EDGE;

	image_widgets_.clear();

	// Row 1 Col 1 - eQSL Enable
	Fl_Check_Button* bn1_1_1 = new Fl_Check_Button(X + C1, Y + R1_1, W1, H1_1, "En");
	bn1_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn1_1_1->labelsize(FONT_SIZE);
	bn1_1_1->value(eqsl_enable_);
	bn1_1_1->callback(cb_ch_enable, &eqsl_enable_);
	bn1_1_1->when(FL_WHEN_CHANGED);
	bn1_1_1->tooltip("Enable eQSL.cc access");

	// eQSL group
	Fl_Group* gp1 = new Fl_Group(X + EDGE, Y + GRP1, WGRP, HGRP1, "eQSL.cc");
	gp1->labelsize(FONT_SIZE);
	gp1->box(FL_THIN_DOWN_FRAME);
	gp1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	Fl_Input* in1_1_2 = new Fl_Input(X + C2, Y + R1_1, W2, H1_1, "Last accessed");
	in1_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_2->labelsize(FONT_SIZE);
	in1_1_2->textsize(FONT_SIZE);
	in1_1_2->value(eqsl_last_got_.c_str());
	in1_1_2->callback(cb_value<Fl_Input, string>, &eqsl_last_got_);
	in1_1_2->when(FL_WHEN_CHANGED);
	in1_1_2->tooltip("Last time eQSL.cc accessed");

	// Row 1 Col 3 - Calendar button
	Fl_Button* bn1_1_3 = new Fl_Button(X + C3, Y + R1_1, W3, H1_1);
	bn1_1_3->image(new Fl_RGB_Image(ICON_CALENDAR, 16, 16, 4));
	eqsl_cal_cb_data_ = { &eqsl_last_got_, in1_1_2 };
	bn1_1_3->callback(calendar::cb_cal_open, &eqsl_cal_cb_data_);
	bn1_1_3->when(FL_WHEN_RELEASE);
	bn1_1_3->tooltip("Open calendar to chnage date to fetch eQSL.cc");
	image_widgets_.insert(bn1_1_3);

	// Row 1 Col 4 - User entry field
	Fl_Input* in1_1_4 = new Fl_Input(X + C4, Y + R1_1, W4, H1_1, "User");
	in1_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_4->labelsize(FONT_SIZE);
	in1_1_4->textsize(FONT_SIZE);
	in1_1_4->value(eqsl_username_.c_str());
	in1_1_4->callback(cb_value<Fl_Input, string>, &eqsl_username_);
	in1_1_4->when(FL_WHEN_CHANGED);
	in1_1_4->tooltip("Enter user name for eQSL.cc");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in1_1_5 = new Fl_Secret_Input(X + C5, Y + R1_1, W5, H1_1, "Password");
	in1_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_5->labelsize(FONT_SIZE);
	in1_1_5->textsize(FONT_SIZE);
	in1_1_5->value(eqsl_password_.c_str());
	in1_1_5->callback(cb_value<Fl_Secret_Input, string>, &eqsl_password_);
	in1_1_5->when(FL_WHEN_CHANGED);
	in1_1_5->tooltip("Enter password for eQSL.cc");

	// Row 2 Col 1 - QSO Message enable
	Fl_Check_Button* bn1_2_1 = new Fl_Check_Button(X + C1, Y + R1_2, W1, H1_2, "Use QSO Message");
	bn1_2_1->align(FL_ALIGN_RIGHT);
	bn1_2_1->labelsize(FONT_SIZE);
	bn1_2_1->value(eqsl_use_qso_msg_);
	bn1_2_1->callback(cb_ch_enable, &eqsl_use_qso_msg_);
	bn1_2_1->when(FL_WHEN_CHANGED);
	bn1_2_1->tooltip("Use the QSO message when sending to eQSL.cc and printing cards");

	// Row2 Col 2A - QSO Message
	Fl_Input* in1_2_2 = new Fl_Input(X + C2A, Y + R1_2, W2A, H1_2);
	in1_2_2->value(eqsl_qso_msg_.c_str());
	in1_2_2->textsize(FONT_SIZE);
	in1_2_2->callback(cb_value<Fl_Input, string>, &eqsl_qso_msg_);
	in1_2_2->when(FL_WHEN_CHANGED);
	in1_2_2->tooltip("Message to send to eQSL.cc or print on cards for QSOs");

	// Row 3 Col 1 - SWL Message enable
	Fl_Check_Button* bn1_3_1 = new Fl_Check_Button(X + C1, Y + R1_3, W1, H1_3, "Use SWL Message");
	bn1_3_1->align(FL_ALIGN_RIGHT);
	bn1_3_1->labelsize(FONT_SIZE);
	bn1_3_1->value(eqsl_use_swl_msg_);
	bn1_3_1->callback(cb_ch_enable, &eqsl_use_swl_msg_);
	bn1_3_1->when(FL_WHEN_CHANGED);
	bn1_3_1->tooltip("Use the SWL message when sending to eQSL.cc and printing cards");

	// Row2 Col 2A - SWL Message
	Fl_Input* in1_3_2 = new Fl_Input(X + C2A, Y + R1_3, W2A, H1_3);
	in1_3_2->value(eqsl_swl_msg_.c_str());
	in1_3_2->textsize(FONT_SIZE);
	in1_3_2->callback(cb_value<Fl_Input, string>, &eqsl_swl_msg_);
	in1_3_2->when(FL_WHEN_CHANGED);
	in1_3_2->tooltip("Message to send to eQSL.cc or print on cards for SWL reports");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_eqsl_ = gp1;

	gp1->end();

	// Row 1 Col 1 - LotW Enable
	Fl_Check_Button* bn2_1_1 = new Fl_Check_Button(X + C1, Y + R2_1, W1, H2_1, "En");
	bn2_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn2_1_1->labelsize(FONT_SIZE);
	bn2_1_1->value(lotw_enable_);
	bn2_1_1->callback(cb_ch_enable, &lotw_enable_);
	bn2_1_1->when(FL_WHEN_CHANGED);
	bn2_1_1->tooltip("Enable Logbook of the World access");

	// LotW group
	Fl_Group* gp2 = new Fl_Group(X + EDGE, Y + GRP2, WGRP, HGRP2, "Logbook of the World");
	gp2->labelsize(FONT_SIZE);
	gp2->box(FL_THIN_DOWN_FRAME);
	gp2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	Fl_Input* in2_1_2 = new Fl_Input(X + C2, Y + R2_1, W2, H2_1, "Last accessed");
	in2_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_2->labelsize(FONT_SIZE);
	in2_1_2->textsize(FONT_SIZE);
	in2_1_2->value(lotw_last_got_.c_str());
	in2_1_2->callback(cb_value<Fl_Input, string>, &lotw_last_got_);
	in2_1_2->when(FL_WHEN_CHANGED);
	in2_1_2->tooltip("Last time Logbook of the World accessed");


	// Row 1 Col 3 - Calendar button
	Fl_Button* bn2_1_3 = new Fl_Button(X + C3, Y + R2_1, W3, H2_1);
	bn2_1_3->image(new Fl_RGB_Image(ICON_CALENDAR, 16, 16, 4));
	lotw_cal_cb_data_ = { &lotw_last_got_, in2_1_2 };
	bn2_1_3->callback(calendar::cb_cal_open, &lotw_cal_cb_data_);
	bn2_1_3->when(FL_WHEN_RELEASE);
	bn2_1_3->tooltip("Open calendar to chnage date to fetch from Logbook of the World");
	image_widgets_.insert(bn2_1_3);

	// Row 1 Col 4 - User entry field
	Fl_Input* in2_1_4 = new Fl_Input(X + C4, Y + R2_1, W4, H2_1, "User");
	in2_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_4->labelsize(FONT_SIZE);
	in2_1_4->textsize(FONT_SIZE);
	in2_1_4->value(lotw_username_.c_str());
	in2_1_4->callback(cb_value<Fl_Input, string>, &lotw_username_);
	in2_1_4->when(FL_WHEN_CHANGED);
	in2_1_4->tooltip("Enter user name for Logbook of the World");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in2_1_5 = new Fl_Secret_Input(X + C5, Y + R2_1, W5, H2_1, "Password");
	in2_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_5->labelsize(FONT_SIZE);
	in2_1_5->textsize(FONT_SIZE);
	in2_1_5->value(lotw_password_.c_str());
	in2_1_5->callback(cb_value<Fl_Secret_Input, string>, &lotw_password_);
	in2_1_5->when(FL_WHEN_CHANGED);
	in2_1_5->tooltip("Enter password for Logbook of the World");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_lotw_ = gp2;

	gp2->end();

	// Row 1 Col 1 - QRZ Enable
	Fl_Check_Button* bn3_1_1 = new Fl_Check_Button(X + C1, Y + R3_1, W1, H3_1, "En");
	bn3_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn3_1_1->labelsize(FONT_SIZE);
	bn3_1_1->value(qrz_enable_);
	bn3_1_1->callback(cb_ch_enable, &qrz_enable_);
	bn3_1_1->when(FL_WHEN_CHANGED);
	bn3_1_1->tooltip("Enable QRZ.com access");

	// QRZ.com group
	Fl_Group* gp3 = new Fl_Group(X + EDGE, Y + GRP3, WGRP, HGRP3, "QRZ.com");
	gp3->labelsize(FONT_SIZE);
	gp3->box(FL_THIN_DOWN_FRAME);
	gp3->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);


	// Row 1 Col 4 - User entry field
	Fl_Input* in3_1_4 = new Fl_Input(X + C4, Y + R3_1, W4, H3_1, "User");
	in3_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_4->labelsize(FONT_SIZE);
	in3_1_4->textsize(FONT_SIZE);
	in3_1_4->value(qrz_username_.c_str());
	in3_1_4->callback(cb_value<Fl_Input, string>, &qrz_username_);
	in3_1_4->when(FL_WHEN_CHANGED);
	in3_1_4->tooltip("Enter user name for QRZ.com");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in3_1_5 = new Fl_Secret_Input(X + C5, Y + R3_1, W5, H3_1, "Password");
	in3_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_5->labelsize(FONT_SIZE);
	in3_1_5->textsize(FONT_SIZE);
	in3_1_5->value(qrz_password_.c_str());
	in3_1_5->callback(cb_value<Fl_Secret_Input, string>, &qrz_password_);
	in3_1_5->when(FL_WHEN_CHANGED);
	in3_1_5->tooltip("Enter password for QRZ.com");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_qrz_ = gp3;

	gp3->end();

	Fl_Group::end();
}

// Save values to settings
void web_dialog::save_values() {
	// Get settings
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences eqsl_settings(qsl_settings, "eQSL");
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	Fl_Preferences qrz_settings(qsl_settings, "QRZ");
	Fl_Preferences card_settings(qsl_settings, "Card");
	// eQSL Settings
	eqsl_settings.set("Enable", eqsl_enable_);
	eqsl_settings.set("User", eqsl_username_.c_str());
	eqsl_settings.set("Password", eqsl_password_.c_str());
	eqsl_settings.set("Last Accessed", eqsl_last_got_.c_str());
	// LotW settings
	lotw_settings.set("Enable", lotw_enable_);
	lotw_settings.set("User", lotw_username_.c_str());
	lotw_settings.set("Password", lotw_password_.c_str());
	lotw_settings.set("Last Accessed", lotw_last_got_.c_str());
	// QRZ.com Settings
	qrz_settings.set("Enable", qrz_enable_);
	qrz_settings.set("User", qrz_username_.c_str());
	qrz_settings.set("Password", qrz_password_.c_str());

	// QSL message settings (for eQSL and card)
	card_settings.set("QSL Enable", eqsl_use_qso_msg_);
	card_settings.set("QSL Message", eqsl_qso_msg_.c_str());
	card_settings.set("SWL Enable", eqsl_use_swl_msg_);
	card_settings.set("SWL Message", eqsl_swl_msg_.c_str());
}

// Enable widgets after enabling/disabling stuff
void web_dialog::enable_widgets() {
	// Enable/Disable eQSL widgets
	if (eqsl_enable_) {
		grp_eqsl_->activate();
	}
	else {
		grp_eqsl_->deactivate();
	}
	// Enable/disable LotW widgets
	if (lotw_enable_) {
		grp_lotw_->activate();
	}
	else {
		grp_lotw_->deactivate();
	}
	// Enable/disable QRZ.com widgets
	if (qrz_enable_) {
		grp_qrz_->activate();
	}
	else {
		grp_qrz_->deactivate();
	}
}

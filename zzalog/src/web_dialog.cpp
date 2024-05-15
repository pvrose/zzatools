#include "web_dialog.h"
#include "utils.h"
#include "calendar.h"
#include "intl_widgets.h"
#include "icons.h"
#include "wsjtx_handler.h"
#include "fllog_emul.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Int_Input.H>

extern Fl_Preferences* settings_;
extern wsjtx_handler* wsjtx_handler_;
extern fllog_emul* fllog_emul_;

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
	, grp_eqsl_(nullptr)
	, grp_lotw_(nullptr)
	, grp_qrz_(nullptr)
	, qrz_xml_merge_(false)
	, club_enable_(false)
	, club_username_("")
	, club_password_("")
	, club_interval_(0)
	, wsjtx_enable_(true)
	, fldigi_enable_(true)
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
	Fl_Preferences club_settings(qsl_settings, "ClubLog");
	Fl_Preferences nw_settings(settings_, "Network");
	Fl_Preferences wsjtx_settings(nw_settings, "WSJT-X");
	Fl_Preferences fllog_settings(nw_settings, "Fllog");

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
	eqsl_settings.get("Upload per QSO", (int&)eqsl_upload_qso_, false);
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
	lotw_settings.get("Upload per QSO", (int&)lotw_upload_qso_, false);

	// QRZ.com User/Password
	qrz_settings.get("Enable", (int&)qrz_enable_, false);
	qrz_settings.get("User", temp, "");
	qrz_username_ = temp;
	free(temp);
	qrz_settings.get("Password", temp, "");
	qrz_password_ = temp;
	free(temp);
	qrz_settings.get("Use XML Database", (int&)qrz_xml_merge_, false);

	// QSL message settings (for eQSL and card)
	card_settings.get("QSL Enable", (int&)eqsl_use_qso_msg_, false);
	card_settings.get("QSL Message", temp, "Tnx QSO <NAME>, 73");
	eqsl_qso_msg_ = temp;
	free(temp);
	card_settings.get("SWL Enable", (int&)eqsl_use_swl_msg_, false);
	card_settings.get("SWL Message", temp, "Tnx SWL Report <NAME>, 73");
	eqsl_swl_msg_ = temp;
	free(temp);

	// ClubLog settings
	club_settings.get("Enable", (int&)club_enable_, false);
	club_settings.get("Email", temp, "");
	club_username_ = temp;
	free(temp);
	club_settings.get("Password", temp, "");
	club_password_ = temp;
	free(temp);
	club_settings.get("Interval", club_interval_, 7);
	club_settings.get("Upload per QSO", (int&)club_upload_qso_, false);

	// Server ports
	wsjtx_settings.get("Address", temp, "127.0.0.1");
	wsjtx_udp_addr_ = temp;
	free(temp);
	wsjtx_settings.get("Port Number", wsjtx_udp_port_, 2237);
	fllog_settings.get("Address", temp, "127.0.0.1");
	fldigi_rpc_addr_ = temp;
	free(temp);
	fllog_settings.get("Port Number", fldigi_rpc_port_, 8421);

}

// Create the dialog
void web_dialog::create_form(int X, int Y) {
	// position constants
	// Group 1 eQSL widgets
	const int GRP1 = EDGE;
	const int R1_1 = GRP1 + GAP + HTEXT;
	const int H1_1 = HBUTTON;
	const int R1_1A = R1_1 + H1_1;
	const int H1_1A = HBUTTON;
	const int R1_2 = R1_1A + H1_1A + GAP;
	const int H1_2 = HBUTTON;
	const int R1_3 = R1_2 + H1_2 + GAP;
	const int H1_3 = HBUTTON;
	const int HGRP1 = R1_3 + H1_3 + GAP - GRP1;

	// Group 2 LotW widgets
	const int GRP2 = GRP1 + HGRP1 + GAP;
	const int R2_1 = GRP2 + GAP + HTEXT;
	const int H2_1 = HBUTTON;
	const int R2_1A = R2_1 + H2_1;
	const int H2_1A = HBUTTON;
	const int HGRP2 = R2_1A + H2_1A + GAP - GRP2;

	// Group 3 QRZ.com widgets
	const int GRP3 = GRP2 + HGRP2 + GAP;
	const int R3_1 = GRP3 + GAP + HTEXT;
	const int H3_1 = HBUTTON;
	const int R3_2 = R3_1 + H3_1;
	const int H3_2 = HRADIO;
	const int HGRP3 = R3_2 + H3_2 + GAP - GRP3;

	// Group 4 ClubLog widgets
	const int GRP4 = GRP3 + HGRP3 + GAP;
	const int R4_1 = GRP4 + GAP + HTEXT;
	const int H4_1 = HBUTTON;
	const int R4_1A = R4_1 + H4_1;
	const int H4_1A = HBUTTON;
	const int R4_2 = R4_1A + H4_1A + HTEXT;
	const int H4_2 = HTEXT;
	const int HGRP4 = R4_2 + H4_2 + GAP - GRP4;

	// Group 5 Network ports
	const int GRP5 = GRP4 + HGRP4 + GAP;
	const int R5_1 = GRP5 + GAP + HTEXT;
	const int H5_1 = HBUTTON;
	const int R5_2 = R5_1 + H5_1;
	const int H5_2 = HBUTTON;
	const int HGRP5 = R5_2 + H5_2 + GAP - GRP5;

	// overall height 
	const int HALL = GRP5 + HGRP5 + EDGE;

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
	const int W34 = W3 + W4 + GAP;
	const int C5 = C4 + W4 + GAP;
	const int W5 = WSMEDIT;
	const int C6 = C5 + W5;
	const int W6 = HBUTTON;

	// offset columns
	const int C2A = C1 + W1 + WLLABEL + GAP;
	const int W2A = WMESS;

	// overall width 
	const int WGRP = max(C6 + W6,C2A + W2A) + GAP;
	const int WALL = WGRP + EDGE;

	image_widgets_.clear();

	// eQSL group
	Fl_Group* gp1 = new Fl_Group(X + EDGE, Y + GRP1, WGRP, HGRP1, "eQSL.cc");
	gp1->labelsize(FL_NORMAL_SIZE + 2);
	gp1->labelfont(FL_BOLD);
	gp1->box(FL_BORDER_BOX);
	gp1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	intl_input* in1_1_2 = new intl_input(X + C2, Y + R1_1, W2, H1_1, "Last accessed");
	in1_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_2->value(eqsl_last_got_.c_str());
	in1_1_2->callback(cb_value<intl_input, string>, &eqsl_last_got_);
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
	intl_input* in1_1_4 = new intl_input(X + C4, Y + R1_1, W4, H1_1, "User");
	in1_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_4->value(eqsl_username_.c_str());
	in1_1_4->callback(cb_value<intl_input, string>, &eqsl_username_);
	in1_1_4->when(FL_WHEN_CHANGED);
	in1_1_4->tooltip("Enter user name for eQSL.cc");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in1_1_5 = new Fl_Secret_Input(X + C5, Y + R1_1, W5, H1_1, "Password");
	in1_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_5->value(eqsl_password_.c_str());
	in1_1_5->callback(cb_value<Fl_Secret_Input, string>, &eqsl_password_);
	in1_1_5->when(FL_WHEN_CHANGED);
	in1_1_5->tooltip("Enter password for eQSL.cc");

	// Row 1 Col 6 - Password visible
	Fl_Button* bn1_1_6 = new Fl_Button(X + C6, Y + R1_1, W6, H1_1, "@search");
	bn1_1_6->type(FL_TOGGLE_BUTTON);
	bn1_1_6->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	bn1_1_6->value(false);
	bn1_1_6->callback(cb_bn_plain, in1_1_5);
	bn1_1_6->tooltip("See password in plain");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn1_1A_1 = new Fl_Check_Button(X + C1, Y + R1_1A, W1, H1_1A, "Update each QSO");
	bn1_1A_1->align(FL_ALIGN_RIGHT);
	bn1_1A_1->value(eqsl_upload_qso_);
	bn1_1A_1->callback(cb_value < Fl_Check_Button, bool>, &eqsl_upload_qso_);
	bn1_1A_1->when(FL_WHEN_CHANGED);
	bn1_1A_1->tooltip("Upload each QSO as it is logged");

	// Row 2 Col 1 - QSO Message enable
	Fl_Check_Button* bn1_2_1 = new Fl_Check_Button(X + C1, Y + R1_2, W1, H1_2, "Use QSO Message");
	bn1_2_1->align(FL_ALIGN_RIGHT);
	bn1_2_1->value(eqsl_use_qso_msg_);
	bn1_2_1->callback(cb_ch_enable, &eqsl_use_qso_msg_);
	bn1_2_1->when(FL_WHEN_CHANGED);
	bn1_2_1->tooltip("Use the QSO message when sending to eQSL.cc and printing cards");

	// Row2 Col 2A - QSO Message
	intl_input* in1_2_2 = new intl_input(X + C2A, Y + R1_2, W2A, H1_2);
	in1_2_2->value(eqsl_qso_msg_.c_str());
	in1_2_2->callback(cb_value<intl_input, string>, &eqsl_qso_msg_);
	in1_2_2->when(FL_WHEN_CHANGED);
	in1_2_2->tooltip("Message to send to eQSL.cc or print on cards for QSOs");

	// Row 3 Col 1 - SWL Message enable
	Fl_Check_Button* bn1_3_1 = new Fl_Check_Button(X + C1, Y + R1_3, W1, H1_3, "Use SWL Message");
	bn1_3_1->align(FL_ALIGN_RIGHT);
	bn1_3_1->value(eqsl_use_swl_msg_);
	bn1_3_1->callback(cb_ch_enable, &eqsl_use_swl_msg_);
	bn1_3_1->when(FL_WHEN_CHANGED);
	bn1_3_1->tooltip("Use the SWL message when sending to eQSL.cc and printing cards");

	// Row2 Col 2A - SWL Message
	intl_input* in1_3_2 = new intl_input(X + C2A, Y + R1_3, W2A, H1_3);
	in1_3_2->value(eqsl_swl_msg_.c_str());
	in1_3_2->callback(cb_value<intl_input, string>, &eqsl_swl_msg_);
	in1_3_2->when(FL_WHEN_CHANGED);
	in1_3_2->tooltip("Message to send to eQSL.cc or print on cards for SWL reports");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_eqsl_ = gp1;

	gp1->end();

	// Row 1 Col 1 - eQSL Enable
	Fl_Check_Button* bn1_1_1 = new Fl_Check_Button(X + C1, Y + R1_1, W1, H1_1, "En");
	bn1_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn1_1_1->value(eqsl_enable_);
	bn1_1_1->callback(cb_ch_enable, &eqsl_enable_);
	bn1_1_1->when(FL_WHEN_CHANGED);
	bn1_1_1->tooltip("Enable eQSL.cc access");


	// LotW group
	Fl_Group* gp2 = new Fl_Group(X + EDGE, Y + GRP2, WGRP, HGRP2, "Logbook of the World");
	gp2->labelsize(FL_NORMAL_SIZE + 2);
	gp2->labelfont(FL_BOLD);
	gp2->box(FL_BORDER_BOX);
	gp2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	intl_input* in2_1_2 = new intl_input(X + C2, Y + R2_1, W2, H2_1, "Last accessed");
	in2_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_2->value(lotw_last_got_.c_str());
	in2_1_2->callback(cb_value<intl_input, string>, &lotw_last_got_);
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
	intl_input* in2_1_4 = new intl_input(X + C4, Y + R2_1, W4, H2_1, "User");
	in2_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_4->value(lotw_username_.c_str());
	in2_1_4->callback(cb_value<intl_input, string>, &lotw_username_);
	in2_1_4->when(FL_WHEN_CHANGED);
	in2_1_4->tooltip("Enter user name for Logbook of the World");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in2_1_5 = new Fl_Secret_Input(X + C5, Y + R2_1, W5, H2_1, "Password");
	in2_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_5->value(lotw_password_.c_str());
	in2_1_5->callback(cb_value<Fl_Secret_Input, string>, &lotw_password_);
	in2_1_5->when(FL_WHEN_CHANGED);
	in2_1_5->tooltip("Enter password for Logbook of the World");

	// Row 1 Col 6 - Password visible
	Fl_Button* bn2_1_6 = new Fl_Button(X + C6, Y + R2_1, W6, H2_1, "@search");
	bn2_1_6->type(FL_TOGGLE_BUTTON);
	bn2_1_6->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	bn2_1_6->value(false);
	bn2_1_6->callback(cb_bn_plain, in2_1_5);
	bn2_1_6->tooltip("See password in plain");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn2_1A_1 = new Fl_Check_Button(X + C1, Y + R2_1A, W1, H2_1A, "Update each QSO");
	bn2_1A_1->align(FL_ALIGN_RIGHT);
	bn2_1A_1->value(lotw_upload_qso_);
	bn2_1A_1->callback(cb_value < Fl_Check_Button, bool>, &lotw_upload_qso_);
	bn2_1A_1->when(FL_WHEN_CHANGED);
	bn2_1A_1->tooltip("Upload each QSO as it is logged");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_lotw_ = gp2;

	gp2->end();

	// Row 1 Col 1 - LotW Enable
	Fl_Check_Button* bn2_1_1 = new Fl_Check_Button(X + C1, Y + R2_1, W1, H2_1, "En");
	bn2_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn2_1_1->value(lotw_enable_);
	bn2_1_1->callback(cb_ch_enable, &lotw_enable_);
	bn2_1_1->when(FL_WHEN_CHANGED);
	bn2_1_1->tooltip("Enable Logbook of the World access");

	// QRZ.com group
	Fl_Group* gp3 = new Fl_Group(X + EDGE, Y + GRP3, WGRP, HGRP3, "QRZ.com");
	gp3->labelsize(FL_NORMAL_SIZE + 2);
	gp3->labelfont(FL_BOLD);
	gp3->box(FL_BORDER_BOX);
	gp3->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);


	// Row 1 Col 4 - User entry field
	intl_input* in3_1_4 = new intl_input(X + C4, Y + R3_1, W4, H3_1, "User");
	in3_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_4->value(qrz_username_.c_str());
	in3_1_4->callback(cb_value<intl_input, string>, &qrz_username_);
	in3_1_4->when(FL_WHEN_CHANGED);
	in3_1_4->tooltip("Enter user name for QRZ.com");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in3_1_5 = new Fl_Secret_Input(X + C5, Y + R3_1, W5, H3_1, "Password");
	in3_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_5->value(qrz_password_.c_str());
	in3_1_5->callback(cb_value<Fl_Secret_Input, string>, &qrz_password_);
	in3_1_5->when(FL_WHEN_CHANGED);
	in3_1_5->tooltip("Enter password for QRZ.com");

	// Row 1 Col 6 - Password visible
	Fl_Button* bn3_1_6 = new Fl_Button(X + C6, Y + R3_1, W6, H3_1, "@search");
	bn3_1_6->type(FL_TOGGLE_BUTTON);
	bn3_1_6->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	bn3_1_6->value(false);
	bn3_1_6->callback(cb_bn_plain, in3_1_5);
	bn3_1_6->tooltip("See password in plain");

	// Row 2 Col 4 - Always use XML database
	Fl_Check_Button* bn3_2_1 = new Fl_Check_Button(X + C4, Y + R3_2, WRADIO, HRADIO, "Use XML Database");
	bn3_2_1->align(FL_ALIGN_RIGHT);
	bn3_2_1->value(qrz_xml_merge_);
	bn3_2_1->callback(cb_value<Fl_Check_Button, bool>, &qrz_xml_merge_);
	bn3_2_1->tooltip("Always the QRZ XML database even if non-subscriber");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_qrz_ = gp3;

	gp3->end();

	// Row 1 Col 1 - QRZ Enable
	Fl_Check_Button* bn3_1_1 = new Fl_Check_Button(X + C1, Y + R3_1, W1, H3_1, "En");
	bn3_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn3_1_1->value(qrz_enable_);
	bn3_1_1->callback(cb_ch_enable, &qrz_enable_);
	bn3_1_1->when(FL_WHEN_CHANGED);
	bn3_1_1->tooltip("Enable QRZ.com access");

	// QRZ.com group
	Fl_Group* gp4 = new Fl_Group(X + EDGE, Y + GRP4, WGRP, HGRP4, "ClubLog");
	gp4->labelsize(FL_NORMAL_SIZE + 2);
	gp4->labelfont(FL_BOLD);
	gp4->box(FL_BORDER_BOX);
	gp4->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);


	// Row 1 Col 3/4 - User entry field
	intl_input* in4_1_4 = new intl_input(X + C3, Y + R4_1, W34, H4_1, "User (e-mail)");
	in4_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_1_4->value(club_username_.c_str());
	in4_1_4->callback(cb_value<intl_input, string>, &club_username_);
	in4_1_4->when(FL_WHEN_CHANGED);
	in4_1_4->tooltip("Enter e-mail address for ClubLog");

	// Row 1 Col 5 - Password entry field
	Fl_Secret_Input* in4_1_5 = new Fl_Secret_Input(X + C5, Y + R4_1, W5, H4_1, "Password");
	in4_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_1_5->value(club_password_.c_str());
	in4_1_5->callback(cb_value<Fl_Secret_Input, string>, &club_password_);
	in4_1_5->when(FL_WHEN_CHANGED);
	in4_1_5->tooltip("Enter password for ClubLog");

	// Row 1 Col 6 - Password visible
	Fl_Button* bn4_1_6 = new Fl_Button(X + C6, Y + R4_1, W6, H4_1, "@search");
	bn4_1_6->type(FL_TOGGLE_BUTTON);
	bn4_1_6->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	bn4_1_6->value(false);
	bn4_1_6->callback(cb_bn_plain, in4_1_5);
	bn4_1_6->tooltip("See password in plain");

	// Row 2 Col 2 - Interval bewteen downloads
	Fl_Int_Input* in4_2_2 = new Fl_Int_Input(X + C2, Y + R4_2, W2, H4_2, "Interval");
	in4_2_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_2_2->value(to_string(club_interval_).c_str());
	in4_2_2->callback(cb_value_int<Fl_Int_Input>, &club_interval_);
	in4_2_2->when(FL_WHEN_CHANGED);
	in4_2_2->tooltip("Specify the days between updating the exception file");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn4_1A_1 = new Fl_Check_Button(X + C1, Y + R4_1A, W1, H4_1A, "Update each QSO");
	bn4_1A_1->align(FL_ALIGN_RIGHT);
	bn4_1A_1->value(club_upload_qso_);
	bn4_1A_1->callback(cb_value < Fl_Check_Button, bool>, &club_upload_qso_);
	bn4_1A_1->when(FL_WHEN_CHANGED);
	bn4_1A_1->tooltip("Upload each QSO as it is logged");


	grp_club_ = gp4;
	gp4->end();

	// Row 1 Col 1 - ClubLog Enable
	Fl_Check_Button* bn4_1_1 = new Fl_Check_Button(X + C1, Y + R4_1, W1, H4_1, "En");
	bn4_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn4_1_1->value(club_enable_);
	bn4_1_1->callback(cb_ch_enable, &club_enable_);
	bn4_1_1->when(FL_WHEN_CHANGED);
	bn4_1_1->tooltip("Enable ClubLog access");

	Fl_Group* gp5 = new Fl_Group(X + EDGE, Y + GRP5, WGRP, HGRP5, "Network ports");
	gp5->labelsize(FL_NORMAL_SIZE + 2);
	gp5->labelfont(FL_BOLD);
	gp5->box(FL_BORDER_BOX);
	gp5->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	Fl_Check_Button* bn5_1_1 = new Fl_Check_Button(X + C1, Y + R5_1, HBUTTON, HBUTTON, "En");
	bn5_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn5_1_1->value(wsjtx_enable_);
	bn5_1_1->callback(cb_bn_wsjtx);
	bn5_1_1->when(FL_WHEN_CHANGED);
	bn5_1_1->tooltip("Enable port, disable changing port");

	const int C51 = X + EDGE + HBUTTON + WLABEL;
	const int C52 = C51 + WSMEDIT;
	const int WG5 = WGRP - HBUTTON;

	Fl_Group* gp5a = new Fl_Group(C51, Y + R5_1, WG5, H5_1, "WSJT-X");
	gp5a->box(FL_FLAT_BOX);
	gp5a->align(FL_ALIGN_LEFT);
	
	Fl_Input* ip5_1_1 = new Fl_Input(C51, Y + R5_1, WSMEDIT, H5_1, "Address");
	ip5_1_1->align(FL_ALIGN_TOP);
	ip5_1_1->value(wsjtx_udp_addr_.c_str());
	ip5_1_1->callback(cb_value<Fl_Input, string>, &wsjtx_udp_addr_);
	ip5_1_1->tooltip("Specify WSJT-X UDP server address - blank for any");

	Fl_Input* ip5_1_2 = new Fl_Input(C52, Y + R5_1, WBUTTON, H5_1, "Port");
	ip5_1_2->align(FL_ALIGN_TOP);
	ip5_1_2->value(to_string(wsjtx_udp_port_).c_str());
	ip5_1_2->callback(cb_value_int<Fl_Input>, &wsjtx_udp_port_);
	ip5_1_2->when(FL_WHEN_CHANGED);
	ip5_1_2->tooltip("Specify WSJT-X UDP server port number");

	gp5a->end();

	grp_wsjtx_ = gp5a;

	Fl_Check_Button* bn5_2_1 = new Fl_Check_Button(X + C1, Y + R5_2, HBUTTON, HBUTTON);
	bn5_2_1->value(fldigi_enable_);
	bn5_2_1->callback(cb_bn_fldigi);
	bn5_2_1->when(FL_WHEN_CHANGED);
	bn5_2_1->tooltip("Enable port, disable changing port");

	Fl_Group* gp5b = new Fl_Group(C51, Y + R5_2, WG5, H5_2, "Fllog");
	gp5b->box(FL_FLAT_BOX);
	gp5b->align(FL_ALIGN_LEFT);
	
	Fl_Input* ip5_2_1 = new Fl_Input(C51, Y + R5_2, WSMEDIT, H5_2);
	ip5_2_1->value(fldigi_rpc_addr_.c_str());
	ip5_2_1->callback(cb_value<Fl_Input, string>, &fldigi_rpc_addr_);
	ip5_2_1->tooltip("Specify Fllog server address - blank for any");

	Fl_Input* ip5_2_2 = new Fl_Input(C52, Y + R5_2, WBUTTON, H5_2);
	ip5_2_2->value(to_string(fldigi_rpc_port_).c_str());
	ip5_2_2->callback(cb_value_int<Fl_Input>, &fldigi_rpc_port_);
	ip5_2_2->when(FL_WHEN_CHANGED);
	ip5_2_2->tooltip("Specify Fldigi server port number");
	gp5b->end();

	grp_fldigi_ = gp5b;

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
	Fl_Preferences club_settings(qsl_settings, "ClubLog");
	Fl_Preferences nw_settings(settings_, "Network");
	Fl_Preferences wsjtx_settings(nw_settings, "WSJT-X");
	Fl_Preferences fllog_settings(nw_settings, "Fllog");
	// eQSL Settings
	eqsl_settings.set("Enable", eqsl_enable_);
	eqsl_settings.set("User", eqsl_username_.c_str());
	eqsl_settings.set("Password", eqsl_password_.c_str());
	eqsl_settings.set("Last Accessed", eqsl_last_got_.c_str());
	eqsl_settings.set("Upload per QSO", eqsl_upload_qso_);
	// LotW settings
	lotw_settings.set("Enable", lotw_enable_);
	lotw_settings.set("User", lotw_username_.c_str());
	lotw_settings.set("Password", lotw_password_.c_str());
	lotw_settings.set("Last Accessed", lotw_last_got_.c_str());
	lotw_settings.set("Upload per QSO", lotw_upload_qso_);
	// QRZ.com Settings
	qrz_settings.set("Enable", qrz_enable_);
	qrz_settings.set("User", qrz_username_.c_str());
	qrz_settings.set("Password", qrz_password_.c_str());
	qrz_settings.set("Use XML Database", qrz_xml_merge_);

	// QSL message settings (for eQSL and card)
	card_settings.set("QSL Enable", eqsl_use_qso_msg_);
	card_settings.set("QSL Message", eqsl_qso_msg_.c_str());
	card_settings.set("SWL Enable", eqsl_use_swl_msg_);
	card_settings.set("SWL Message", eqsl_swl_msg_.c_str());

	// ClubLog settings
	club_settings.set("Enable", club_enable_);
	club_settings.set("Email", club_username_.c_str());
	club_settings.set("Password", club_password_.c_str());
	club_settings.set("Interval", club_interval_);
	club_settings.set("Upload per QSO", club_upload_qso_);

	// Network settings
	wsjtx_settings.set("Address", wsjtx_udp_addr_.c_str());
	wsjtx_settings.set("Port Number", wsjtx_udp_port_);
	fllog_settings.set("Address", fldigi_rpc_addr_.c_str());
	fllog_settings.set("Port Number", fldigi_rpc_port_);

	settings_->flush();
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
	// Enable/disable ClubLog widgets
	if (club_enable_) {
		grp_club_->activate();
	}
	else {
		grp_club_->deactivate();
	}
	//WSJTX widgets
	if (wsjtx_enable_) {
		grp_wsjtx_->deactivate();
	} else {
		grp_wsjtx_->activate();
	}
	// FLDIGI widgets
	if (fldigi_enable_) {
		grp_fldigi_->deactivate();
	} else {
		grp_fldigi_->activate();
	}
}

// Callback to make passwords plain or secret
void web_dialog::cb_bn_plain(Fl_Widget* w, void* v) {
	Fl_Secret_Input* ip = (Fl_Secret_Input*)v;
	if (((Fl_Button*)w)->value()) {
		ip->input_type(FL_NORMAL_INPUT);
	}
	else {
		ip->input_type(FL_SECRET_INPUT);
	}
	ip->redraw();
}

// Callback to implement WSJTX enable
void web_dialog::cb_bn_wsjtx(Fl_Widget* w, void* v) {
	web_dialog* that = ancestor_view<web_dialog>(w);
	cb_value<Fl_Check_Button, bool>(w, &that->wsjtx_enable_);
	that->save_values();
	that->enable_widgets();

	if(that->wsjtx_enable_) {
		wsjtx_handler_->run_server();
	} else {
		wsjtx_handler_->close_server();
	}
}

// Callback to implement Fldigi enable
void web_dialog::cb_bn_fldigi(Fl_Widget* w, void* v) {
	web_dialog* that = ancestor_view<web_dialog>(w);
	cb_value<Fl_Check_Button, bool>(w, &that->fldigi_enable_);
	that->save_values();
	that->enable_widgets();

	if(that->fldigi_enable_) {
		fllog_emul_->run_server();
	} else {
		fllog_emul_->close_server();
	}
}

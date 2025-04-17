#include "web_dialog.h"
#include "utils.h"
#include "calendar_input.h"
#include "intl_widgets.h"
#include "icons.h"
#include "wsjtx_handler.h"
#include "fllog_emul.h"
#include "password_input.h"
#include "spec_data.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Tabs.H>

extern Fl_Preferences* settings_;
extern wsjtx_handler* wsjtx_handler_;
extern fllog_emul* fllog_emul_;
extern spec_data* spec_data_;
extern uint32_t seed_;

// Constructor
web_dialog::web_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)

	, eqsl_enable_(false)
	, eqsl_last_got_("")
	, eqsl_username_("")
	, eqsl_password_("")
	, eqsl_use_qso_msg_(false)
	, eqsl_qso_msg_("")
	, eqsl_use_swl_msg_(false)
	, eqsl_swl_msg_("")
	, eqsl_confirmed_too_(false)
	, eqsl_max_fetches_(0)
	, eqsl_upload_qso_(false)
	, lotw_enable_(false)
	, lotw_last_got_("")
	, lotw_username_("")
	, lotw_password_("")
	, lotw_upload_qso_(false)
	, qrz_username_("")
	, qrz_password_("")
	, qrz_api_enable_(false)
	, grp_eqsl_(nullptr)
	, grp_lotw_(nullptr)
	, grp_qrz_(nullptr)
	, grp_fldigi_(nullptr)
	, grp_email_(nullptr)
	, grp_server_(nullptr)
	, grp_club_(nullptr)
	, grp_wsjtx_(nullptr)
	, qrz_xml_merge_(false)
	, club_enable_(false)
	, club_username_("")
	, club_password_("")
	, club_interval_(0)
	, club_upload_qso_(false)
	, wsjtx_enable_(true)
	, wsjtx_udp_port_(0)
	, fldigi_enable_(true)
	, fldigi_rpc_port_(0)
	, email_server_("")
	, email_account_("")
	, email_password_("")
{
	image_widgets_.clear();
	qrz_api_data_.clear();

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
	Fl_Preferences email_settings(settings_, "e-Mail");

	// eQSL User/Password
	eqsl_settings.get("Enable", (int&)eqsl_enable_, false);
	char * temp;
	int sz_temp;
	char* nu_temp;
	const unsigned long long u64_zero = 0ull; 
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
	eqsl_settings.get("Download Confirmed", (int&)eqsl_confirmed_too_, false);
	eqsl_settings.get("Maximum Fetches", eqsl_max_fetches_, 50);
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
	qrz_settings.get("User", temp, "");
	qrz_username_ = temp;
	free(temp);
	qrz_settings.get("Password", temp, "");
	qrz_password_ = temp;
	free(temp);
	qrz_settings.get("Use XML Database", (int&)qrz_xml_merge_, false);
	qrz_settings.get("Use API", (int&)qrz_api_enable_, false);
	string name = spec_data_->enumeration_name("STATION_CALLSIGN", nullptr);
	spec_dataset* calls = spec_data_->dataset(name);
	Fl_Preferences api_settings(qrz_settings, "Logbooks");
	uchar offset = hash8(api_settings.path());
	for (auto it = calls->data.begin(); it != calls->data.end(); it++) {
		string call = (*it).first;
		de_slash(call);
		Fl_Preferences call_settings(api_settings, call.c_str());
		api_logbook_data* data = new api_logbook_data;
		call_settings.get("Key Length", sz_temp, 128);
		nu_temp = new char[sz_temp];
		call_settings.get("Key", nu_temp, (void*)"", 0, &sz_temp);
		string crypt = string(nu_temp, sz_temp);
		string key = xor_crypt(crypt, seed_, offset);
		delete[] nu_temp;
		data->key = key;
		call_settings.get("Last Log ID", (void*)&data->last_logid, (void*)&u64_zero, sizeof(uint64_t), sizeof(uint64_t));
		call_settings.get("Last Download", temp, "");
		data->last_download = temp;
		free(temp);
		qrz_api_data_[it->first] = data;
		int tint;
		call_settings.get("In Use", tint, false);
		data->used = tint;
	}

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

	// e-Mail settings
	email_settings.get("Server", temp, "");
	email_server_ = temp;
	free(temp);
	email_settings.get("Account", temp, "");
	email_account_ = temp;
	free(temp);
	email_settings.get("Password", temp, "");
	email_password_ = temp;
	free(temp);
	email_settings.get("CC Address", temp, "");
	email_cc_address_ = temp;
	free(temp);

}

// Create the dialog
void web_dialog::create_form(int X, int Y) {

	// Create a FL_Tabs
	Fl_Tabs* tabs = new Fl_Tabs(X +GAP, Y + GAP, w() - GAP - GAP, h() - GAP - GAP);
	tabs->callback(cb_tab);
	tabs->box(FL_FLAT_BOX);

	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	// Get clienet area
	tabs->client_area(rx, ry, rw, rh, 0);

	create_eqsl(rx, ry, rw, rh);
	create_lotw(rx, ry, rw, rh);
	create_qrz(rx, ry, rw, rh);
	create_club(rx, ry, rw, rh);
	create_server(rx, ry, rw, rh);
	create_email(rx, ry, rw, rh);

	tabs->end();

	image_widgets_.clear();

	Fl_Group::end();
}

void web_dialog::create_eqsl(int rx, int ry, int rw, int rh) {
	// Group 1 eQSL widgets
	const int GRP1 = ry + GAP;
	const int R1_1 = GRP1 + GAP;
	const int H1_1 = HBUTTON;
	const int R1_1A = R1_1 + H1_1;
	const int H1_1A = HBUTTON;
	const int R1_1B = R1_1A + H1_1A;
	const int H1_1B = HBUTTON;
	const int R1_2 = R1_1B + H1_1B + GAP;
	const int H1_2 = HBUTTON;
	const int R1_3 = R1_2 + H1_2 + GAP;
	const int H1_3 = HBUTTON;
	// main columns
	const int C1 = rx + GAP;
	const int W1 = HBUTTON; // square check button
	const int C2 = C1 + W1 + GAP;
	const int W2 = WSMEDIT;
	const int C3 = C2 + W2 + GAP;
	const int W3 = HBUTTON * 3 / 2;
	const int C4 = C3 + W3 + GAP;
	const int W4 = WSMEDIT;
	const int C5 = C4 + W4 + GAP;
	const int W5 = WSMEDIT;
	const int W6 = HBUTTON;
	// offset columns
	const int C2A = C1 + W1 + WLLABEL + GAP;
	const int W2A = WEDIT;

	Fl_Group* gp01 = new Fl_Group(rx, ry, rw, rh, "eQSL.cc");

	// eQSL group
	Fl_Group* gp1 = new Fl_Group(rx, ry, rw, rh);
	gp1->labelsize(FL_NORMAL_SIZE + 2);
	gp1->labelfont(FL_BOLD);
	gp1->box(FL_BORDER_BOX);
	gp1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	calendar_input* in1_1_2 = new calendar_input(C2, R1_1, W2 + H1_1, H1_1, "Last accessed");
	in1_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_2->value(eqsl_last_got_.c_str());
	in1_1_2->callback(cb_value<intl_input, string>, &eqsl_last_got_);
	in1_1_2->when(FL_WHEN_CHANGED);
	in1_1_2->tooltip("Last time eQSL.cc accessed");

	// Row 1 Col 4 - User entry field
	intl_input* in1_1_4 = new intl_input(C4, R1_1, W4, H1_1, "User");
	in1_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_4->value(eqsl_username_.c_str());
	in1_1_4->callback(cb_value<intl_input, string>, &eqsl_username_);
	in1_1_4->when(FL_WHEN_CHANGED);
	in1_1_4->tooltip("Enter user name for eQSL.cc");

	// Row 1 Col 5 - Password entry field
	password_input* in1_1_5 = new password_input(C5, R1_1, W5 + W6, H1_1, "Password");
	in1_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_5->value(eqsl_password_.c_str());
	in1_1_5->callback(cb_value<Fl_Input, string>, &eqsl_password_);
	in1_1_5->when(FL_WHEN_CHANGED);
	in1_1_5->tooltip("Enter password for eQSL.cc");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn1_1A_1 = new Fl_Check_Button(C1, R1_1A, W1, H1_1A, "Update each QSO");
	bn1_1A_1->align(FL_ALIGN_RIGHT);
	bn1_1A_1->value(eqsl_upload_qso_);
	bn1_1A_1->callback(cb_value < Fl_Check_Button, bool>, &eqsl_upload_qso_);
	bn1_1A_1->when(FL_WHEN_CHANGED);
	bn1_1A_1->tooltip("Upload each QSO as it is logged");

	// Row 1A Col 3 - Download Confirmed
	Fl_Check_Button* bn1_1A_2 = new Fl_Check_Button(C3, R1_1A, HBUTTON, H1_1A, "Download Confirmed");
	bn1_1A_2->align(FL_ALIGN_RIGHT);
	bn1_1A_2->value(eqsl_confirmed_too_);
	bn1_1A_2->callback(cb_value<Fl_Check_Button, bool>, &eqsl_confirmed_too_);
	bn1_1A_2->when(FL_WHEN_CHANGED);
	bn1_1A_2->tooltip("Include previously confirmed QSOs");

	// Row 1B Col 2 - Maximum number of fetches
	Fl_Int_Input* ip1_1B_1 = new Fl_Int_Input(C2, R1_1B, WBUTTON, H1_1B, "Max. Fetches");
	ip1_1B_1->align(FL_ALIGN_RIGHT);
	ip1_1B_1->value(to_string(eqsl_max_fetches_).c_str());
	ip1_1B_1->callback(cb_value_int<Fl_Int_Input>, &eqsl_max_fetches_);
	ip1_1B_1->when(FL_WHEN_CHANGED);
	ip1_1B_1->tooltip("Specify maximum number of image fetches at a time");

	// Row 2 Col 1 - QSO Message enable
	Fl_Check_Button* bn1_2_1 = new Fl_Check_Button(C1, R1_2, W1, H1_2, "Use QSO Message");
	bn1_2_1->align(FL_ALIGN_RIGHT);
	bn1_2_1->value(eqsl_use_qso_msg_);
	bn1_2_1->callback(cb_ch_enable, &eqsl_use_qso_msg_);
	bn1_2_1->when(FL_WHEN_CHANGED);
	bn1_2_1->tooltip("Use the QSO message when sending to eQSL.cc and printing cards");

	// Row2 Col 2A - QSO Message
	intl_input* in1_2_2 = new intl_input(C2A, R1_2, W2A, H1_2);
	in1_2_2->value(eqsl_qso_msg_.c_str());
	in1_2_2->callback(cb_value<intl_input, string>, &eqsl_qso_msg_);
	in1_2_2->when(FL_WHEN_CHANGED);
	in1_2_2->tooltip("Message to send to eQSL.cc or print on cards for QSOs");

	// Row 3 Col 1 - SWL Message enable
	Fl_Check_Button* bn1_3_1 = new Fl_Check_Button(C1, R1_3, W1, H1_3, "Use SWL Message");
	bn1_3_1->align(FL_ALIGN_RIGHT);
	bn1_3_1->value(eqsl_use_swl_msg_);
	bn1_3_1->callback(cb_ch_enable, &eqsl_use_swl_msg_);
	bn1_3_1->when(FL_WHEN_CHANGED);
	bn1_3_1->tooltip("Use the SWL message when sending to eQSL.cc and printing cards");

	// Row2 Col 2A - SWL Message
	intl_input* in1_3_2 = new intl_input(C2A, R1_3, W2A, H1_3);
	in1_3_2->value(eqsl_swl_msg_.c_str());
	in1_3_2->callback(cb_value<intl_input, string>, &eqsl_swl_msg_);
	in1_3_2->when(FL_WHEN_CHANGED);
	in1_3_2->tooltip("Message to send to eQSL.cc or print on cards for SWL reports");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_eqsl_ = gp1;

	gp1->end();

	// Row 1 Col 1 - eQSL Enable
	Fl_Check_Button* bn1_1_1 = new Fl_Check_Button(C1, R1_1, W1, H1_1, "En");
	bn1_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn1_1_1->value(eqsl_enable_);
	bn1_1_1->callback(cb_ch_enable, &eqsl_enable_);
	bn1_1_1->when(FL_WHEN_CHANGED);
	bn1_1_1->tooltip("Enable eQSL.cc access");

	gp01->end();


}

// Craete LotW group
void web_dialog::create_lotw(int rx, int ry, int rw, int rh) {
	// Group 2 LotW widgets
	const int GRP2 = ry + GAP;
	const int R2_1 = GRP2 + GAP;
	const int H2_1 = HBUTTON;
	const int R2_1A = R2_1 + H2_1;
	const int H2_1A = HBUTTON;
	// main columns
	const int C1 = rx + GAP;
	const int W1 = HBUTTON; // square check button
	const int C2 = C1 + W1 + GAP;
	const int W2 = WSMEDIT;
	const int C3 = C2 + W2 + GAP;
	const int W3 = HBUTTON * 3 / 2;
	const int C4 = C3 + W3 + GAP;
	const int W4 = WSMEDIT;
	const int C5 = C4 + W4 + GAP;
	const int W5 = WSMEDIT;
	const int W6 = HBUTTON;

	// Overall group for TAB
	Fl_Group* gp02 = new Fl_Group(rx, ry, rw, rh, "LotW");


	// LotW group
	Fl_Group* gp2 = new Fl_Group(rx, ry, rw, rh);
	gp2->labelsize(FL_NORMAL_SIZE + 2);
	gp2->labelfont(FL_BOLD);
	gp2->box(FL_BORDER_BOX);
	gp2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	calendar_input* in2_1_2 = new calendar_input(C2, R2_1, W2, H2_1, "Last accessed");
	in2_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_2->value(lotw_last_got_.c_str());
	in2_1_2->callback(cb_value<intl_input, string>, &lotw_last_got_);
	in2_1_2->when(FL_WHEN_CHANGED);
	in2_1_2->tooltip("Last time Logbook of the World accessed");

	                         // Row 1 Col 4 - User entry field
	intl_input* in2_1_4 = new intl_input(C4, R2_1, W4, H2_1, "User");
	in2_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_4->value(lotw_username_.c_str());
	in2_1_4->callback(cb_value<intl_input, string>, &lotw_username_);
	in2_1_4->when(FL_WHEN_CHANGED);
	in2_1_4->tooltip("Enter user name for Logbook of the World");

	// Row 1 Col 5 - Password entry field
	password_input* in2_1_5 = new password_input(C5, R2_1, W5 + W6, H2_1, "Password");
	in2_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_5->value(lotw_password_.c_str());
	in2_1_5->callback(cb_value<Fl_Input, string>, &lotw_password_);
	in2_1_5->when(FL_WHEN_CHANGED);
	in2_1_5->tooltip("Enter password for Logbook of the World");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn2_1A_1 = new Fl_Check_Button(C1, R2_1A, W1, H2_1A, "Update each QSO");
	bn2_1A_1->align(FL_ALIGN_RIGHT);
	bn2_1A_1->value(lotw_upload_qso_);
	bn2_1A_1->callback(cb_value < Fl_Check_Button, bool>, &lotw_upload_qso_);
	bn2_1A_1->when(FL_WHEN_CHANGED);
	bn2_1A_1->tooltip("Upload each QSO as it is logged");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_lotw_ = gp2;

	gp2->end();

	// Row 1 Col 1 - LotW Enable
	Fl_Check_Button* bn2_1_1 = new Fl_Check_Button(C1, R2_1, W1, H2_1, "En");
	bn2_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn2_1_1->value(lotw_enable_);
	bn2_1_1->callback(cb_ch_enable, &lotw_enable_);
	bn2_1_1->when(FL_WHEN_CHANGED);
	bn2_1_1->tooltip("Enable Logbook of the World access");

	gp02->end();
}

// Craete QRZ.com group
void web_dialog::create_qrz(int rx, int ry, int rw, int rh) {

	// Overall group for tabs
	Fl_Group* gp03 = new Fl_Group(rx, ry, rw, rh, "QRZ.com");
	// QRZ.com group
	Fl_Group* gp3 = new Fl_Group(rx, ry, rw, rh);
	gp3->labelsize(FL_NORMAL_SIZE + 2);
	gp3->labelfont(FL_BOLD);
	gp3->box(FL_BORDER_BOX);
	gp3->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	int curr_x = rx + GAP;
	int curr_y = ry + GAP;

	// XML enable

	Fl_Check_Button* bn3_2_1 = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Use XML Database");
	bn3_2_1->align(FL_ALIGN_RIGHT);
	bn3_2_1->value(qrz_xml_merge_);
	bn3_2_1->callback(cb_ch_enable, &qrz_xml_merge_);
	bn3_2_1->tooltip("Always the QRZ XML database even if non-subscriber");

	curr_x += WLLABEL + WRADIO;

	grp_qrz_xml_ = new Fl_Group(curr_x, curr_y, rw + rx - curr_x - GAP, HBUTTON + GAP);
	grp_qrz_xml_->box(FL_FLAT_BOX);

	curr_y += GAP;

	// Row 1 Col 4 - User entry field
	intl_input* in3_1_4 = new intl_input(curr_x, curr_y, WSMEDIT, HBUTTON, "User");
	in3_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_4->value(qrz_username_.c_str());
	in3_1_4->callback(cb_value<intl_input, string>, &qrz_username_);
	in3_1_4->when(FL_WHEN_CHANGED);
	in3_1_4->tooltip("Enter user name for QRZ.com");

	curr_x += WSMEDIT + GAP;

	// Row 1 Col 5 - Password entry field
	password_input* in3_1_5 = new password_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Password");
	in3_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_5->value(qrz_password_.c_str());
	in3_1_5->callback(cb_value<Fl_Input, string>, &qrz_password_);
	in3_1_5->when(FL_WHEN_CHANGED);
	in3_1_5->tooltip("Enter password for QRZ.com");

	grp_qrz_xml_->end();

	curr_x = rx + GAP;
	curr_y += HBUTTON + GAP;

	Fl_Check_Button* bn_qrz_api = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "API Enable");
	bn_qrz_api->align(FL_ALIGN_RIGHT);
	bn_qrz_api->value(qrz_api_enable_);
	bn_qrz_api->callback(cb_ch_enable, &qrz_api_enable_);
	bn_qrz_api->tooltip("Select if can upload/download using QRZ.com API");

	curr_x += WLLABEL + WRADIO;
	
	grp_qrz_api_ = new Fl_Group(curr_x, curr_y, rw + rx - curr_x - GAP, HBUTTON * (qrz_api_data_.size() + 1));
	grp_qrz_api_->box(FL_FLAT_BOX);

	// Now the headers
	const int WBOOK = WBUTTON;
	const int WUSE = WRADIO;
	const int WKEY = WSMEDIT;
	const int WLASTID = WBUTTON;
	const int WLASTDL = WSMEDIT;
	const int XBOOK = curr_x;
	const int XUSE = XBOOK + WBOOK;
	const int XKEY = XUSE + WUSE + GAP;
	const int XLASTID = XKEY + WKEY;
	const int XLASTDL = XLASTID + WLASTID;

	Fl_Box* b1 = new Fl_Box(XBOOK, curr_y, WBOOK, HBUTTON, "Book");
	b1->align(FL_ALIGN_CENTER);
	b1->box(FL_FLAT_BOX);

	Fl_Box* b1a = new Fl_Box(XUSE, curr_y, WUSE, HBUTTON, "Used");
	b1a->align(FL_ALIGN_CENTER);
	b1a->box(FL_FLAT_BOX);
	
	Fl_Box* b2 = new Fl_Box(XKEY, curr_y, WKEY, HBUTTON, "Key");
	b2->align(FL_ALIGN_CENTER);
	b2->box(FL_FLAT_BOX);
	
	Fl_Box* b3 = new Fl_Box(XLASTID, curr_y, WLASTID, HBUTTON, "Last Log ID");
	b3->align(FL_ALIGN_CENTER);
	b3->box(FL_FLAT_BOX);
	
	Fl_Box* b4 = new Fl_Box(XLASTDL, curr_y, WLASTDL, HBUTTON, "Last Downloaded");
	b4->align(FL_ALIGN_CENTER);
	b4->box(FL_FLAT_BOX);
	
	curr_y += HBUTTON;
	char temp[128];
	for (auto it = qrz_api_data_.begin(); it != qrz_api_data_.end(); it ++) {
		Fl_Output* op_book = new Fl_Output(XBOOK, curr_y, WBOOK, HBUTTON);
		op_book->box(FL_FLAT_BOX);
		op_book->color(FL_BACKGROUND_COLOR);
		op_book->value(it->first.c_str());

		Fl_Check_Button* bn_use = new Fl_Check_Button(XUSE, curr_y, WUSE, HBUTTON);
		bn_use->value(it->second->used);
		bn_use->callback(cb_ch_enable, &it->second->used);
		bn_use->tooltip("Select to allow access for this book");

		Fl_Group* grp_api = new Fl_Group(XKEY, curr_y, WKEY + WLASTID + WLASTDL, HBUTTON);
		grp_api->box(FL_FLAT_BOX);

		password_input* ip_key = new password_input(XKEY, curr_y, WKEY, HBUTTON);
		ip_key->value(it->second->key.c_str());
		ip_key->callback(cb_value<Fl_Input, string>, &it->second->key);
		ip_key->tooltip("Enter the QRZ.com API key for logbook");

		Fl_Int_Input* ip_lastid = new Fl_Int_Input(XLASTID, curr_y, WLASTID, HBUTTON);
		snprintf(temp, sizeof(temp), "%llu", it->second->last_logid);
		ip_lastid->value(temp);
		ip_lastid->callback(cb_value_ull<Fl_Int_Input>, &it->second->last_logid);
		ip_lastid->tooltip("Normally displays the LogID of the last record downloaded");

		calendar_input* ip_lastdl = new calendar_input(XLASTDL, curr_y, WLASTDL, HBUTTON);
		ip_lastdl->value(it->second->last_download.c_str());
		ip_lastdl->callback(cb_value<intl_input, string>, &it->second->last_download);
		ip_lastdl->tooltip("The date of the last download");

		grp_api->end();
		grp_api_books_[it->first] = grp_api;

		curr_y += HBUTTON;
	}

	grp_qrz_api_->end();

	// Create the list of widgets to be disabled when eQSL disabled
	grp_qrz_ = gp3;

	gp3->end();

	gp03->end();
}

// Create Clublog group
void web_dialog::create_club(int rx, int ry, int rw, int rh) {
	// Group 4 ClubLog widgets
	const int GRP4 = ry + GAP;
	const int R4_1 = GRP4 + GAP;
	const int H4_1 = HBUTTON;
	const int R4_1A = R4_1 + H4_1;
	const int H4_1A = HBUTTON;
	const int R4_2 = R4_1A + H4_1A + HTEXT;
	const int H4_2 = HTEXT;
	// main columns
	const int C1 = rx + GAP;
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

	// Group for tabs
	Fl_Group* gp04 = new Fl_Group(rx, ry, rw, rh, "ClubLog");

	// QRZ.com group
	Fl_Group* gp4 = new Fl_Group(rx, ry, rw, rh);
	gp4->labelsize(FL_NORMAL_SIZE + 2);
	gp4->labelfont(FL_BOLD);
	gp4->box(FL_BORDER_BOX);
	gp4->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);


	// Row 1 Col 3/4 - User entry field
	intl_input* in4_1_4 = new intl_input(C3, R4_1, W34, H4_1, "User (e-mail)");
	in4_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_1_4->value(club_username_.c_str());
	in4_1_4->callback(cb_value<intl_input, string>, &club_username_);
	in4_1_4->when(FL_WHEN_CHANGED);
	in4_1_4->tooltip("Enter e-mail address for ClubLog");

	// Row 1 Col 5 - Password entry field
	password_input* in4_1_5 = new password_input(C5, R4_1, W5, H4_1, "Password");
	in4_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_1_5->value(club_password_.c_str());
	in4_1_5->callback(cb_value<Fl_Input, string>, &club_password_);
	in4_1_5->when(FL_WHEN_CHANGED);
	in4_1_5->tooltip("Enter password for ClubLog");

	// Row 2 Col 2 - Interval bewteen downloads
	Fl_Int_Input* in4_2_2 = new Fl_Int_Input(C2, R4_2, W2, H4_2, "Interval");
	in4_2_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_2_2->value(to_string(club_interval_).c_str());
	in4_2_2->callback(cb_value_int<Fl_Int_Input>, &club_interval_);
	in4_2_2->when(FL_WHEN_CHANGED);
	in4_2_2->tooltip("Specify the days between updating the exception file");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn4_1A_1 = new Fl_Check_Button(C1, R4_1A, W1, H4_1A, "Update each QSO");
	bn4_1A_1->align(FL_ALIGN_RIGHT);
	bn4_1A_1->value(club_upload_qso_);
	bn4_1A_1->callback(cb_value < Fl_Check_Button, bool>, &club_upload_qso_);
	bn4_1A_1->when(FL_WHEN_CHANGED);
	bn4_1A_1->tooltip("Upload each QSO as it is logged");


	grp_club_ = gp4;
	gp4->end();

	// Row 1 Col 1 - ClubLog Enable
	Fl_Check_Button* bn4_1_1 = new Fl_Check_Button(C1, R4_1, W1, H4_1, "En");
	bn4_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn4_1_1->value(club_enable_);
	bn4_1_1->callback(cb_ch_enable, &club_enable_);
	bn4_1_1->when(FL_WHEN_CHANGED);
	bn4_1_1->tooltip("Enable ClubLog access");

	gp04->end();

}

// Create Network ports
void web_dialog::create_server(int rx, int ry, int rw, int rh) {
	// Group 5 Network ports
	const int GRP5 = ry + GAP;
	const int R5_1 = GRP5 + GAP;
	const int H5_1 = HBUTTON;
	const int R5_2 = R5_1 + H5_1;
	const int H5_2 = HBUTTON;
	// main columns
	const int C1 = rx + GAP;

	// Group for Fl_Tabs
	Fl_Group* gp05 = new Fl_Group(rx, ry, rw, rh, "Network");

	Fl_Group* gp5 = new Fl_Group(rx, ry, rw, rh);
	gp5->labelsize(FL_NORMAL_SIZE + 2);
	gp5->labelfont(FL_BOLD);
	gp5->box(FL_BORDER_BOX);
	gp5->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	Fl_Check_Button* bn5_1_1 = new Fl_Check_Button(C1, R5_1, HBUTTON, HBUTTON, "En");
	bn5_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn5_1_1->value(wsjtx_enable_);
	bn5_1_1->callback(cb_bn_wsjtx);
	bn5_1_1->when(FL_WHEN_CHANGED);
	bn5_1_1->tooltip("Enable port, disable changing port");

	const int C51 = EDGE + HBUTTON + WLABEL;
	const int C52 = C51 + WSMEDIT;
	const int WG5 = C52 + WBUTTON - rx;

	Fl_Group* gp5a = new Fl_Group(C51, R5_1, WG5, H5_1, "WSJT-X");
	gp5a->box(FL_FLAT_BOX);
	gp5a->align(FL_ALIGN_LEFT);
	
	Fl_Input* ip5_1_1 = new Fl_Input(C51, R5_1, WSMEDIT, H5_1, "Address");
	ip5_1_1->align(FL_ALIGN_TOP);
	ip5_1_1->value(wsjtx_udp_addr_.c_str());
	ip5_1_1->callback(cb_value<Fl_Input, string>, &wsjtx_udp_addr_);
	ip5_1_1->tooltip("Specify WSJT-X UDP server address - blank for any");

	Fl_Input* ip5_1_2 = new Fl_Input(C52, R5_1, WBUTTON, H5_1, "Port");
	ip5_1_2->align(FL_ALIGN_TOP);
	ip5_1_2->value(to_string(wsjtx_udp_port_).c_str());
	ip5_1_2->callback(cb_value_int<Fl_Input>, &wsjtx_udp_port_);
	ip5_1_2->when(FL_WHEN_CHANGED);
	ip5_1_2->tooltip("Specify WSJT-X UDP server port number");

	gp5a->end();

	grp_wsjtx_ = gp5a;

	Fl_Check_Button* bn5_2_1 = new Fl_Check_Button(C1, R5_2, HBUTTON, HBUTTON);
	bn5_2_1->value(fldigi_enable_);
	bn5_2_1->callback(cb_bn_fldigi);
	bn5_2_1->when(FL_WHEN_CHANGED);
	bn5_2_1->tooltip("Enable port, disable changing port");

	Fl_Group* gp5b = new Fl_Group(C51, R5_2, WG5, H5_2, "Fllog");
	gp5b->box(FL_FLAT_BOX);
	gp5b->align(FL_ALIGN_LEFT);
	
	Fl_Input* ip5_2_1 = new Fl_Input(C51, R5_2, WSMEDIT, H5_2);
	ip5_2_1->value(fldigi_rpc_addr_.c_str());
	ip5_2_1->callback(cb_value<Fl_Input, string>, &fldigi_rpc_addr_);
	ip5_2_1->tooltip("Specify Fllog server address - blank for any");

	Fl_Input* ip5_2_2 = new Fl_Input(C52, R5_2, WBUTTON, H5_2);
	ip5_2_2->value(to_string(fldigi_rpc_port_).c_str());
	ip5_2_2->callback(cb_value_int<Fl_Input>, &fldigi_rpc_port_);
	ip5_2_2->when(FL_WHEN_CHANGED);
	ip5_2_2->tooltip("Specify Fldigi server port number");
	gp5b->end();

	grp_fldigi_ = gp5b;

	gp5->end();

	gp05->end();


}

// Create the e-mail details screen
void web_dialog::create_email(int rx, int ry, int rw, int rh) {

	Fl_Group* gp06 = new Fl_Group(rx, ry, rw, rh, "e-Mail");

	Fl_Group* gp6 = new Fl_Group(rx, ry, rw, rh);
	gp6->labelsize(FL_NORMAL_SIZE + 2);
	gp6->labelfont(FL_BOLD);
	gp6->box(FL_BORDER_BOX);
	gp6->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	int curr_x = rx + GAP + WLABEL;
	int curr_y = ry + GAP;

	Fl_Input* ip61 = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Server");
	ip61->align(FL_ALIGN_LEFT);
	ip61->value(email_server_.c_str());
	ip61->callback(cb_value< Fl_Input, string >, &email_server_);
	ip61->when(FL_WHEN_CHANGED);
	ip61->tooltip("Please enter the address of the SMTP server");

	curr_x += ip61->w() + WLABEL;
	Fl_Input* ip62 = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "User");
	ip62->align(FL_ALIGN_LEFT);
	ip62->value(email_account_.c_str());
	ip62->callback(cb_value< Fl_Input, string >, &email_account_);
	ip62->when(FL_WHEN_CHANGED);
	ip62->tooltip("Please enter the account for the SMTP server (usually an e-mail address)");
	
	curr_y += HBUTTON;
	password_input* ip63 = new password_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Password");
	ip63->align(FL_ALIGN_LEFT);
	ip63->value(email_password_.c_str());
	ip63->callback(cb_value< Fl_Input, string >, &email_password_);
	ip63->when(FL_WHEN_CHANGED);
	ip63->tooltip("Please enter the password for the above account");

	curr_y += HBUTTON + GAP;
	Fl_Input* ip64 = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "CC");
	ip64->align(FL_ALIGN_LEFT);
	ip64->value(email_cc_address_.c_str());
	ip64->callback(cb_value< Fl_Input, string >, &email_cc_address_);
	ip64->when(FL_WHEN_CHANGED);
	ip64->tooltip("Please enter an e-mail address to receive copy of e-mail sent");

	gp6->end();

	gp06->end();
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
	Fl_Preferences email_settings(settings_, "e-Mail");
	// eQSL Settings
	eqsl_settings.set("Enable", eqsl_enable_);
	eqsl_settings.set("User", eqsl_username_.c_str());
	eqsl_settings.set("Password", eqsl_password_.c_str());
	eqsl_settings.set("Last Accessed", eqsl_last_got_.c_str());
	eqsl_settings.set("Upload per QSO", eqsl_upload_qso_);
	eqsl_settings.set("Download Confirmed", eqsl_confirmed_too_);
	eqsl_settings.set("Maximum Fetches", eqsl_max_fetches_);
	// LotW settings
	lotw_settings.set("Enable", lotw_enable_);
	lotw_settings.set("User", lotw_username_.c_str());
	lotw_settings.set("Password", lotw_password_.c_str());
	lotw_settings.set("Last Accessed", lotw_last_got_.c_str());
	lotw_settings.set("Upload per QSO", lotw_upload_qso_);
	// QRZ.com Settings
	qrz_settings.set("User", qrz_username_.c_str());
	qrz_settings.set("Password", qrz_password_.c_str());
	qrz_settings.set("Use XML Database", qrz_xml_merge_);
	qrz_settings.set("Use API", (int)qrz_api_enable_);
	Fl_Preferences api_settings(qrz_settings, "Logbooks");
	uchar offset = hash8(api_settings.path());
	for (auto it = qrz_api_data_.begin(); it != qrz_api_data_.end(); it++) {
		string call = it->first;
		de_slash(call);
		Fl_Preferences call_settings(api_settings, call.c_str());
		string encrypt = xor_crypt(it->second->key, seed_, offset);
		call_settings.set("Key Length", (int)encrypt.length());
		call_settings.set("Key", (void*)encrypt.c_str(), encrypt.length());
		call_settings.set("Last Log ID", (void*)&it->second->last_logid, sizeof(uint64_t));
		call_settings.set("Last Download", it->second->last_download.c_str());
		call_settings.set("In Use", (int)it->second->used);
	}

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

	// e-Mail settings
	email_settings.set("Server", email_server_.c_str());
	email_settings.set("Account", email_account_.c_str());
	email_settings.set("Password", email_password_.c_str());
	email_settings.set("CC Address", email_cc_address_.c_str());

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
	if (qrz_xml_merge_) {
		grp_qrz_xml_->activate();
	} else {
		grp_qrz_xml_->deactivate();
	}
	if (qrz_api_enable_) {
		grp_qrz_api_->activate();
		for (auto it = qrz_api_data_.begin(); it != qrz_api_data_.end(); it++) {
			if (it->second->used) {
				grp_api_books_.at(it->first)->activate();
			} else {
				grp_api_books_.at(it->first)->deactivate();
			}
		}
	} else {
		grp_qrz_api_->deactivate();
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
	// Standard tab formats
	// value() returns the selected widget. We need to test which widget it is.
	Fl_Tabs* tabs = (Fl_Tabs*)child(0);
	Fl_Widget* tab = tabs->value();
	for (int ix = 0; ix < tabs->children(); ix++) {
		Fl_Widget* wx = tabs->child(ix);
		if (wx == tab) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
			wx->activate();
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
			wx->deactivate();
		}
	}

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

// Callback on changing tab
void web_dialog::cb_tab(Fl_Widget* w, void* v) {
	web_dialog* that = ancestor_view<web_dialog>(w);
	that->enable_widgets();
}



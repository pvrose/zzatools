#include "web_dialog.h"
#include "utils.h"
#include "calendar_input.h"
#include "intl_widgets.h"
#include "icons.h"
#include "wsjtx_handler.h"
#include "fllog_emul.h"
#include "password_input.h"
#include "spec_data.h"
#include "qrz_handler.h"
#include "qsl_dataset.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Tabs.H>

extern wsjtx_handler* wsjtx_handler_;
extern fllog_emul* fllog_emul_;
extern spec_data* spec_data_;
extern uint32_t seed_;
extern string VENDOR;
extern string PROGRAM_ID;
extern qsl_dataset* qsl_dataset_;
extern Fl_Preferences::Root prefs_mode_;
extern void open_html(const char*);

// Constructor
web_dialog::web_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label)
	, grp_eqsl_(nullptr)
	, grp_lotw_(nullptr)
	, grp_qrz_(nullptr)
	, grp_fldigi_(nullptr)
	, grp_email_(nullptr)
	, grp_server_(nullptr)
	, grp_club_(nullptr)
	, grp_wsjtx_(nullptr)
	, wsjtx_enable_(true)
	, wsjtx_udp_port_(0)
	, fldigi_enable_(true)
	, fldigi_rpc_port_(0)
	, eqsl_data_(nullptr)
	, lotw_data_(nullptr)
	, club_data_(nullptr)
	, qrz_data_(nullptr)
	, email_data_(nullptr)
{

	do_creation(X, Y);
}

// Destructor
web_dialog::~web_dialog()
{
}

// Handle
int web_dialog::handle(int event) {
	int result = page_dialog::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		if (!result) take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("web_dialog.html");
			return true;
		}
		break;
	}
	return result;
}

// Load initial values from settings
void web_dialog::load_values() {
	// Get the various settings
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences nw_settings(settings, "Network");
	Fl_Preferences wsjtx_settings(nw_settings, "WSJT-X");
	Fl_Preferences fllog_settings(nw_settings, "Fllog");

	// Server ports
	char* temp;
	wsjtx_settings.get("Address", temp, "127.0.0.1");
	wsjtx_udp_addr_ = temp;
	free(temp);
	wsjtx_settings.get("Port Number", wsjtx_udp_port_, 2237);
	fllog_settings.get("Address", temp, "127.0.0.1");
	fldigi_rpc_addr_ = temp;
	free(temp);
	fllog_settings.get("Port Number", fldigi_rpc_port_, 8421);

	eqsl_data_ = get_server("eQSL");
	lotw_data_ = get_server("LotW");
	club_data_ = get_server("Club");
	qrz_data_ = get_server("QRZ");
	email_data_ = get_server("eMail");
	// Add any STATION_CALLSIGNs not yet in the data read from file
	spec_dataset* call_set = spec_data_->dataset("Dynamic STATION_CALLSIGN");
	if (call_set) {
		for (auto it : call_set->data) {
			string call = it.first;
			if (qrz_data_->call_data.find(call) == qrz_data_->call_data.end()) {
				qrz_data_->call_data[call] = new qsl_call_data;
			}
			if (eqsl_data_->call_data.find(call) == eqsl_data_->call_data.end()) {
				eqsl_data_->call_data[call] = new qsl_call_data;
			}
		}
	}

}

// Get the existing server data or create a new one.
server_data_t* web_dialog::get_server(string name) {
	server_data_t* result = qsl_dataset_->get_server_data(name);
	if (!result) {
		if (qsl_dataset_->new_server(name)) {
			result = qsl_dataset_->get_server_data(name);
			return result;
		} else {
			return nullptr;
		}
	} else {
		return result;
	}

}

// Create the dialog
void web_dialog::create_form(int X, int Y) {

	box(FL_FLAT_BOX);

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

	Fl_Group::end();
}

void web_dialog::create_eqsl(int rx, int ry, int rw, int rh) {
	//// Group 1 eQSL widgets
	//const int GRP1 = ry + GAP;
	//const int R1_1 = GRP1 + GAP;
	//const int H1_1 = HBUTTON;
	//const int R1_1A = R1_1 + H1_1;
	//const int H1_1A = HBUTTON;
	//const int R1_1B = R1_1A + H1_1A;
	//const int H1_1B = HBUTTON;
	//const int R1_2 = R1_1B + H1_1B + GAP;
	//const int H1_2 = HBUTTON;
	//const int R1_3 = R1_2 + H1_2 + GAP;
	//const int H1_3 = HBUTTON;
	//// main columns
	//const int C1 = rx + GAP;
	//const int W1 = HBUTTON; // square check button
	//const int C2 = C1 + W1 + GAP;
	//const int W2 = WSMEDIT;
	//const int C3 = C2 + W2 + GAP;
	//const int W3 = HBUTTON * 3 / 2;
	//const int C4 = C3 + W3 + GAP;
	//const int W4 = WSMEDIT;
	//const int C5 = C4 + W4 + GAP;
	//const int W5 = WSMEDIT;
	//const int W6 = HBUTTON;
	//// offset columns
	//const int C2A = C1 + W1 + WLLABEL + GAP;
	//const int W2A = WEDIT;

	Fl_Group* gp01 = new Fl_Group(rx, ry, rw, rh, "eQSL.cc");

	// eQSL group
	Fl_Group* gp1 = new Fl_Group(rx, ry, rw, rh);
	gp1->labelsize(FL_NORMAL_SIZE + 2);
	gp1->labelfont(FL_BOLD);
	gp1->box(FL_FLAT_BOX);
	gp1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	//// Row 1 Col 2 - Date last accessed
	//calendar_input* in1_1_2 = new calendar_input(C2, R1_1, W2 + H1_1, H1_1, "Last accessed");
	//in1_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	//in1_1_2->value(eqsl_data_->last_downloaded.c_str());
	//in1_1_2->callback(cb_value<intl_input, string>, &eqsl_data_->last_downloaded);
	//in1_1_2->when(FL_WHEN_CHANGED);
	//in1_1_2->tooltip("Last time eQSL.cc accessed");

	// Leave room for the eQSL enable button
	int curr_x = rx + GAP + HBUTTON;
	int curr_y = ry + GAP + GAP;

	// Row 1 Col 4 - User entry field
	intl_input* in1_1_4 = new intl_input(curr_x, curr_y, WSMEDIT, HBUTTON, "User");
	in1_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_4->value(eqsl_data_->user.c_str());
	in1_1_4->callback(cb_value<intl_input, string>, &eqsl_data_->user);
	in1_1_4->when(FL_WHEN_CHANGED);
	in1_1_4->tooltip("Enter user name for eQSL.cc");

	// Row 1 Col 5 - Password entry field
	curr_x += WSMEDIT + GAP;
	password_input* in1_1_5 = new password_input(curr_x, curr_y, WSMEDIT + HBUTTON, HBUTTON, "Password");
	in1_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in1_1_5->value(eqsl_data_->password.c_str());
	in1_1_5->callback(cb_value<Fl_Input, string>, &eqsl_data_->password);
	in1_1_5->when(FL_WHEN_CHANGED);
	in1_1_5->tooltip("Enter password for eQSL.cc");

	curr_y += HBUTTON + GAP;
	curr_x = in1_1_4->x();
	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn1_1A_1 = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Update each QSO");
	bn1_1A_1->align(FL_ALIGN_RIGHT);
	bn1_1A_1->value(eqsl_data_->upload_per_qso);
	bn1_1A_1->callback(cb_value < Fl_Check_Button, bool>, &eqsl_data_->upload_per_qso);
	bn1_1A_1->when(FL_WHEN_CHANGED);
	bn1_1A_1->tooltip("Upload each QSO as it is logged");

	curr_x = in1_1_5->x();
	// Row 1A Col 3 - Download Confirmed
	Fl_Check_Button* bn1_1A_2 = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Download Confirmed");
	bn1_1A_2->align(FL_ALIGN_RIGHT);
	bn1_1A_2->value(eqsl_data_->download_confirmed);
	bn1_1A_2->callback(cb_value<Fl_Check_Button, bool>, &eqsl_data_->download_confirmed);
	bn1_1A_2->when(FL_WHEN_CHANGED);
	bn1_1A_2->tooltip("Include previously confirmed QSOs");

	// Now the 
	curr_y += GAP + HBUTTON;
	curr_x = rx + GAP + HBUTTON;
	grp_eqsl_calls_ = new Fl_Group(curr_x, curr_y, rw + rx - curr_x - GAP, HBUTTON * (eqsl_data_->call_data.size() + 2) + GAP);

	Fl_Box* b1 = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON, "Call");
	b1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	b1->box(FL_FLAT_BOX);

	curr_x += WBUTTON + GAP;
	Fl_Box* b1a = new Fl_Box(curr_x, curr_y, HBUTTON, HBUTTON, "Used");
	b1a->align(FL_ALIGN_CENTER);
	b1a->box(FL_FLAT_BOX);

	curr_x += HBUTTON + GAP;
	Fl_Box* b4 = new Fl_Box(curr_x, curr_y, WSMEDIT + HBUTTON, HBUTTON, "Last Downloaded");
	b4->align(FL_ALIGN_CENTER);
	b4->box(FL_FLAT_BOX);

	curr_y += HBUTTON;
	for (auto it = eqsl_data_->call_data.begin(); it != eqsl_data_->call_data.end(); it++) {
		curr_x = rx + GAP + HBUTTON;
		Fl_Output* op_book = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON);
		op_book->box(FL_FLAT_BOX);
		op_book->color(FL_BACKGROUND_COLOR);
		op_book->value(it->first.c_str());

		curr_x += WBUTTON + GAP;
		Fl_Check_Button* bn_use = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON);
		bn_use->value(it->second->used);
		bn_use->callback(cb_ch_enable, &it->second->used);
		bn_use->tooltip("Select to allow access for this book");

		curr_x += HBUTTON + GAP;
		calendar_input* ip_lastdl = new calendar_input(curr_x, curr_y, WSMEDIT + HBUTTON, HBUTTON);
		ip_lastdl->value(it->second->last_download.c_str());
		ip_lastdl->callback(cb_value<intl_input, string>, &it->second->last_download);
		ip_lastdl->tooltip("The date of the last download");

		w_eqsl_lupds_[it->first] = ip_lastdl;

		curr_y += HBUTTON;
	}

	grp_eqsl_calls_->end();

	curr_x = rx + GAP + HBUTTON + WLLABEL;
	curr_y += GAP;

	// Row2 Col 2A - QSO Message
	intl_input* in1_2_2 = new intl_input(curr_x, curr_y, WEDIT, HBUTTON, "QSL Message (QSO)");
	in1_2_2->align(FL_ALIGN_LEFT);
	in1_2_2->value(eqsl_data_->qso_message.c_str());
	in1_2_2->callback(cb_value<intl_input, string>, &eqsl_data_->qso_message);
	in1_2_2->when(FL_WHEN_CHANGED);
	in1_2_2->tooltip("Message to send to eQSL.cc or print on cards for QSOs");

	curr_y += HBUTTON;
	// Row2 Col 2A - SWL Message
	intl_input* in1_3_2 = new intl_input(curr_x, curr_y, WEDIT, HBUTTON, "QSL Message (SWL)");
	in1_3_2->align(FL_ALIGN_LEFT);
	in1_3_2->value(eqsl_data_->swl_message.c_str());
	in1_3_2->callback(cb_value<intl_input, string>, &eqsl_data_->swl_message);
	in1_3_2->when(FL_WHEN_CHANGED);
	in1_3_2->tooltip("Message to send to eQSL.cc or print on cards for SWL reports");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_eqsl_ = gp1;

	gp1->end();

	// Row 1 Col 1 - eQSL Enable
	Fl_Check_Button* bn1_1_1 = new Fl_Check_Button(rx + GAP, ry + GAP, HBUTTON, HBUTTON, "En");
	bn1_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn1_1_1->value(eqsl_data_->enabled);
	bn1_1_1->callback(cb_ch_enable, &eqsl_data_->enabled);
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
	gp2->box(FL_FLAT_BOX);
	gp2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	// Row 1 Col 2 - Date last accessed
	calendar_input* in2_1_2 = new calendar_input(C2, R2_1, W2, H2_1, "Last accessed");
	in2_1_2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_2->value(lotw_data_->last_downloaded.c_str());
	in2_1_2->callback(cb_value<intl_input, string>, &lotw_data_->last_downloaded);
	in2_1_2->when(FL_WHEN_CHANGED);
	in2_1_2->tooltip("Last time Logbook of the World accessed");

	                         // Row 1 Col 4 - User entry field
	intl_input* in2_1_4 = new intl_input(C4, R2_1, W4, H2_1, "User");
	in2_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_4->value(lotw_data_->user.c_str());
	in2_1_4->callback(cb_value<intl_input, string>, &lotw_data_->user);
	in2_1_4->when(FL_WHEN_CHANGED);
	in2_1_4->tooltip("Enter user name for Logbook of the World");

	// Row 1 Col 5 - Password entry field
	password_input* in2_1_5 = new password_input(C5, R2_1, W5 + W6, H2_1, "Password");
	in2_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in2_1_5->value(lotw_data_->password.c_str());
	in2_1_5->callback(cb_value<Fl_Input, string>, &lotw_data_->password);
	in2_1_5->when(FL_WHEN_CHANGED);
	in2_1_5->tooltip("Enter password for Logbook of the World");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn2_1A_1 = new Fl_Check_Button(C1, R2_1A, W1, H2_1A, "Update each QSO");
	bn2_1A_1->align(FL_ALIGN_RIGHT);
	bn2_1A_1->value(lotw_data_->upload_per_qso);
	bn2_1A_1->callback(cb_value < Fl_Check_Button, bool>, &lotw_data_->upload_per_qso);
	bn2_1A_1->when(FL_WHEN_CHANGED);
	bn2_1A_1->tooltip("Upload each QSO as it is logged");

	// Create the list of widgets to be disabled when eQSL disabled
	grp_lotw_ = gp2;

	gp2->end();

	// Row 1 Col 1 - LotW Enable
	Fl_Check_Button* bn2_1_1 = new Fl_Check_Button(C1, R2_1, W1, H2_1, "En");
	bn2_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn2_1_1->value(lotw_data_->enabled);
	bn2_1_1->callback(cb_ch_enable, &lotw_data_->enabled);
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
	gp3->box(FL_FLAT_BOX);
	gp3->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	int curr_x = rx + GAP;
	int curr_y = ry + GAP;

	// XML enable

	Fl_Check_Button* bn3_2_1 = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Use XML Database");
	bn3_2_1->align(FL_ALIGN_RIGHT);
	bn3_2_1->value(qrz_data_->use_xml);
	bn3_2_1->callback(cb_ch_enable, &qrz_data_->use_xml);
	bn3_2_1->tooltip("Always the QRZ XML database even if non-subscriber");

	curr_x += WLLABEL + WRADIO;

	grp_qrz_xml_ = new Fl_Group(curr_x, curr_y, rw + rx - curr_x - GAP, HBUTTON + GAP);
	grp_qrz_xml_->box(FL_FLAT_BOX);

	// Row 1 Col 4 - User entry field
	intl_input* in3_1_4 = new intl_input(curr_x, curr_y, WSMEDIT, HBUTTON, "User");
	in3_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_4->value(qrz_data_->user.c_str());
	in3_1_4->callback(cb_value<intl_input, string>, &qrz_data_->user);
	in3_1_4->when(FL_WHEN_CHANGED);
	in3_1_4->tooltip("Enter user name for QRZ.com");

	curr_x += WSMEDIT + GAP;

	// Row 1 Col 5 - Password entry field
	password_input* in3_1_5 = new password_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Password");
	in3_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in3_1_5->value(qrz_data_->password.c_str());
	in3_1_5->callback(cb_value<Fl_Input, string>, &qrz_data_->password);
	in3_1_5->when(FL_WHEN_CHANGED);
	in3_1_5->tooltip("Enter password for QRZ.com");

	grp_qrz_xml_->end();

	curr_x = rx + GAP;
	curr_y += HBUTTON + GAP;

	Fl_Check_Button* bn_qrz_api = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "API Enable");
	bn_qrz_api->align(FL_ALIGN_RIGHT);
	bn_qrz_api->value(qrz_data_->use_api);
	bn_qrz_api->callback(cb_ch_enable, &qrz_data_->use_api);
	bn_qrz_api->tooltip("Select if can upload/download using QRZ.com API");

	curr_y += HBUTTON;
	
	grp_qrz_api_ = new Fl_Group(curr_x, curr_y, rw + rx - curr_x - GAP, HBUTTON * (qrz_data_->call_data.size() + 2) + GAP);
	grp_qrz_api_->box(FL_FLAT_BOX);

	// Now the headers
	const int WBOOK = WBUTTON;
	const int WUSE = WRADIO;
	const int WKEY = WEDIT;
	const int WLASTID = WSMEDIT;
	const int WLASTDL = WSMEDIT;
	const int XBOOK = curr_x + WRADIO;
	const int XUSE = XBOOK + WBOOK;
	const int XKEY = XUSE + WUSE + GAP;
	const int XLASTDL = XKEY + WKEY;

	Fl_Check_Button* bn_qrz_upload = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "Update each QSO");
	bn_qrz_upload->align(FL_ALIGN_RIGHT);
	bn_qrz_upload->value(qrz_data_->upload_per_qso);
	bn_qrz_upload->callback(cb_value < Fl_Check_Button, bool>, &qrz_data_->upload_per_qso);
	bn_qrz_upload->when(FL_WHEN_CHANGED);
	bn_qrz_upload->tooltip("Upload each QSO as it is logged");

	curr_x += WRADIO;
	curr_y += HRADIO;
	Fl_Box* b1 = new Fl_Box(XBOOK, curr_y, WBOOK, HBUTTON, "Book");
	b1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	b1->box(FL_FLAT_BOX);

	Fl_Box* b1a = new Fl_Box(XUSE, curr_y, WUSE, HBUTTON, "Used");
	b1a->align(FL_ALIGN_CENTER);
	b1a->box(FL_FLAT_BOX);
	
	Fl_Box* b2 = new Fl_Box(XKEY, curr_y, WKEY, HBUTTON, "Key");
	b2->align(FL_ALIGN_CENTER);
	b2->box(FL_FLAT_BOX);
	
	Fl_Box* b4 = new Fl_Box(XLASTDL, curr_y, WLASTDL, HBUTTON, "Last Downloaded");
	b4->align(FL_ALIGN_CENTER);
	b4->box(FL_FLAT_BOX);
	
	curr_y += HBUTTON;
	for (auto it = qrz_data_->call_data.begin(); it != qrz_data_->call_data.end(); it ++) {
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
	gp4->box(FL_FLAT_BOX);
	gp4->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);


	// Row 1 Col 3/4 - User entry field
	intl_input* in4_1_4 = new intl_input(C3, R4_1, W34, H4_1, "User (e-mail)");
	in4_1_4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_1_4->value(club_data_->user.c_str());
	in4_1_4->callback(cb_value<intl_input, string>, &club_data_->user);
	in4_1_4->when(FL_WHEN_CHANGED);
	in4_1_4->tooltip("Enter e-mail address for ClubLog");

	// Row 1 Col 5 - Password entry field
	password_input* in4_1_5 = new password_input(C5, R4_1, W5, H4_1, "Password");
	in4_1_5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in4_1_5->value(club_data_->password.c_str());
	in4_1_5->callback(cb_value<Fl_Input, string>, &club_data_->password);
	in4_1_5->when(FL_WHEN_CHANGED);
	in4_1_5->tooltip("Enter password for ClubLog");

	// Row 1A Col 1 - Update evry QSO
	Fl_Check_Button* bn4_1A_1 = new Fl_Check_Button(C1, R4_1A, W1, H4_1A, "Update each QSO");
	bn4_1A_1->align(FL_ALIGN_RIGHT);
	bn4_1A_1->value(club_data_->upload_per_qso);
	bn4_1A_1->callback(cb_value < Fl_Check_Button, bool>, &club_data_->upload_per_qso);
	bn4_1A_1->when(FL_WHEN_CHANGED);
	bn4_1A_1->tooltip("Upload each QSO as it is logged");


	grp_club_ = gp4;
	gp4->end();

	// Row 1 Col 1 - ClubLog Enable
	Fl_Check_Button* bn4_1_1 = new Fl_Check_Button(C1, R4_1, W1, H4_1, "En");
	bn4_1_1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	bn4_1_1->value(club_data_->enabled);
	bn4_1_1->callback(cb_ch_enable, &club_data_->enabled);
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
	gp5->box(FL_FLAT_BOX);
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
	gp6->box(FL_FLAT_BOX);
	gp6->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	int curr_x = rx + GAP + WLABEL;
	int curr_y = ry + GAP;

	Fl_Input* ip61 = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Server");
	ip61->align(FL_ALIGN_LEFT);
	ip61->value(email_data_->mail_server.c_str());
	ip61->callback(cb_value< Fl_Input, string >, &email_data_->mail_server);
	ip61->when(FL_WHEN_CHANGED);
	ip61->tooltip("Please enter the address of the SMTP server");

	curr_x += ip61->w() + WLABEL;
	Fl_Input* ip62 = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "User");
	ip62->align(FL_ALIGN_LEFT);
	ip62->value(email_data_->user.c_str());
	ip62->callback(cb_value< Fl_Input, string >, &email_data_->user);
	ip62->when(FL_WHEN_CHANGED);
	ip62->tooltip("Please enter the account for the SMTP server (usually an e-mail address)");
	
	curr_y += HBUTTON;
	password_input* ip63 = new password_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Password");
	ip63->align(FL_ALIGN_LEFT);
	ip63->value(email_data_->password.c_str());
	ip63->callback(cb_value< Fl_Input, string >, &email_data_->password);
	ip63->when(FL_WHEN_CHANGED);
	ip63->tooltip("Please enter the password for the above account");

	curr_y += HBUTTON + GAP;
	Fl_Input* ip64 = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "CC");
	ip64->align(FL_ALIGN_LEFT);
	ip64->value(email_data_->cc_address.c_str());
	ip64->callback(cb_value< Fl_Input, string >, &email_data_->cc_address);
	ip64->when(FL_WHEN_CHANGED);
	ip64->tooltip("Please enter an e-mail address to receive copy of e-mail sent");

	gp6->end();

	gp06->end();
}

// Save values to settings
void web_dialog::save_values() {
	// Get settings
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences nw_settings(settings, "Network");
	Fl_Preferences wsjtx_settings(nw_settings, "WSJT-X");
	Fl_Preferences fllog_settings(nw_settings, "Fllog");
	// Network settings
	wsjtx_settings.set("Address", wsjtx_udp_addr_.c_str());
	wsjtx_settings.set("Port Number", wsjtx_udp_port_);
	fllog_settings.set("Address", fldigi_rpc_addr_.c_str());
	fllog_settings.set("Port Number", fldigi_rpc_port_);
	qsl_dataset_->save_data();

}

// Enable widgets after enabling/disabling stuff
void web_dialog::enable_widgets() {
	// Enable/Disable eQSL widgets
	if (eqsl_data_->enabled) {
		grp_eqsl_->activate();
		for (auto it = eqsl_data_->call_data.begin(); it != eqsl_data_->call_data.end(); it++) {
			if (it->second->used) {
				w_eqsl_lupds_.at(it->first)->activate();
			}
			else {
				w_eqsl_lupds_.at(it->first)->deactivate();
			}
		}
	}
	else {
		grp_eqsl_->deactivate();
	}
	// Enable/disable LotW widgets
	if (lotw_data_->enabled) {
		grp_lotw_->activate();
	}
	else {
		grp_lotw_->deactivate();
	}
	// Enable/disable QRZ.com widgets
	if (qrz_data_->use_xml) {
		grp_qrz_xml_->activate();
	} else {
		grp_qrz_xml_->deactivate();
	}
	if (qrz_data_->use_api) {
		grp_qrz_api_->activate();
		for (auto it = qrz_data_->call_data.begin(); it != qrz_data_->call_data.end(); it++) {
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
	if (club_data_->enabled) {
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



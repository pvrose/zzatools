#include "qso_qsl.h"
#include "drawing.h"
#include "extract_data.h"
#include "import_data.h"
#include "tabbed_forms.h"
#include "printer.h"
#include "status.h"
#include "book.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Box.H>

extern Fl_Preferences* settings_;
extern import_data* import_data_;
extern extract_data* extract_records_;
extern tabbed_forms* tabbed_forms_;
extern status* status_;
extern book* book_;

qso_qsl::qso_qsl(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	// Log status group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	load_values();
	create_form();
	enable_widgets();
}

qso_qsl::~qso_qsl() {
	save_values();
}

// Load settings data
void qso_qsl::load_values() {
	// eQSL
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences eqsl_settings(qsl_settings, "eQSL");
	int upload_qso;
	eqsl_settings.get("Upload per QSO", upload_qso, false);
	auto_eqsl_ = upload_qso;

	// LotW
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	lotw_settings.get("Upload per QSO", upload_qso, false);
	auto_lotw_ = upload_qso;

	// Clublog
	Fl_Preferences club_settings(qsl_settings, "ClubLog");
	club_settings.get("Upload per QSO", upload_qso, false);
	auto_club_ = upload_qso;
}

void qso_qsl::create_form() {


	const int C1 = x() + GAP;
	const int W1 = WBUTTON / 2;
	const int C2 = C1 + W1;
	const int W2 = HBUTTON;
	const int C3 = C2 + W2;
	const int W3 = WBUTTON / 2;
	const int C4 = C3 + W3;
	const int W4 = WBUTTON / 2;
	const int C5 = C4 + W4;
	const int W5 = WBUTTON / 2;
	const int C6 = C5 + W5;
	const int W6 = WBUTTON / 2;
	int max_x = C6 + W6 + GAP;
	int curr_y = y() + GAP;

	// eQSL buttons
	// Title
	Fl_Box* box_e = new Fl_Box(C1, curr_y, W1, HBUTTON, "eQSL");
	box_e->box(FL_FLAT_BOX);
	box_e->color(FL_BACKGROUND_COLOR);
	// Auto upload
	bn_auto_eqsl_ = new Fl_Check_Button(C2, curr_y, W2, HBUTTON);
	bn_auto_eqsl_->value(auto_eqsl_);
	bn_auto_eqsl_->callback(cb_auto, (void*)extract_data::EQSL);
	bn_auto_eqsl_->tooltip("Enable automatic upload of eQSL after logging");
	// Download
	bn_down_eqsl_ = new Fl_Button(C3, curr_y, W3, HBUTTON, "@2->");
	bn_down_eqsl_->callback(cb_download, (void*)import_data::EQSL_UPDATE);
	bn_down_eqsl_->tooltip("Download latest records from eQSL");
	// Extract
	bn_extr_eqsl_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_eqsl_->callback(cb_extract, (void*)extract_data::EQSL);
	bn_extr_eqsl_->tooltip("Extract records for upload to eQSL");
	// Upload
	bn_upld_eqsl_ = new Fl_Button(C5, curr_y, W5, HBUTTON, "@8->");
	bn_upld_eqsl_->callback(cb_upload, (void*)extract_data::EQSL);
	bn_upld_eqsl_->tooltip("Upload extracted records to eQSL");
	//// Fetch cards
	//bn_down_ecard_ = new Fl_Button(C6, curr_y, W6, HBUTTON, "eCards");
	//bn_down_ecard_->callback(cb_fetch);
	//bn_down_ecard_->tooltip("Fetch any eQSL card images missing");

	curr_y += HBUTTON;

	// LotW buttons
	Fl_Box* box_l = new Fl_Box(C1, curr_y, W1, HBUTTON, "LotW");
	box_l->box(FL_FLAT_BOX);
	box_l->color(FL_BACKGROUND_COLOR);
	// Auto upload
	bn_auto_lotw_ = new Fl_Check_Button(C2, curr_y, W2, HBUTTON);
	bn_auto_lotw_->value(auto_lotw_);
	bn_auto_lotw_->callback(cb_auto, (void*)extract_data::LOTW);
	bn_auto_lotw_->tooltip("Enable automatic upload of LotW after logging");
	// Download
	bn_down_lotw_ = new Fl_Button(C3, curr_y, W3, HBUTTON, "@2->");
	bn_down_lotw_->callback(cb_download, (void*)import_data::LOTW_UPDATE);
	bn_down_lotw_->tooltip("Download latest records from LotW");
	// Extract
	bn_extr_lotw_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_lotw_->callback(cb_extract, (void*)extract_data::LOTW);
	bn_extr_lotw_->tooltip("Extract records for upload to LotW");
	// Upload
	bn_upld_lotw_ = new Fl_Button(C5, curr_y, W5, HBUTTON, "@8->");
	bn_upld_lotw_->callback(cb_upload, (void*)extract_data::LOTW);
	bn_upld_lotw_->tooltip("Upload extracted records to LotW");

	curr_y += HBUTTON;

	Fl_Box* box_c = new Fl_Box(C1, curr_y, W1, HBUTTON, "ClubLog");
	box_e->box(FL_FLAT_BOX);
	box_e->color(FL_BACKGROUND_COLOR);
	// Auto upload
	bn_auto_club_ = new Fl_Check_Button(C2, curr_y, W2, HBUTTON);
	bn_auto_club_->value(auto_club_);
	bn_auto_club_->callback(cb_auto, (void*)extract_data::CLUBLOG);
	bn_auto_club_->tooltip("Enable automatic upload of ClubLog after logging");
	//// Download
	//bn_down_club_ = new Fl_Button(C3, curr_y, W3, HBUTTON, "@2->");
	//bn_down_club_->callback(cb_download, (void*)extract_data::CLUBLOG);
	//bn_down_club_->tooltip("Download latest records from ClubLog");
	// Extract
	bn_extr_club_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_club_->callback(cb_extract, (void*)extract_data::CLUBLOG);
	bn_extr_club_->tooltip("Extract records for upload to ClubLog");
	// Upload
	bn_upld_club_ = new Fl_Button(C5, curr_y, W5, HBUTTON, "@8->");
	bn_upld_club_->callback(cb_upload, (void*)extract_data::CLUBLOG);
	bn_upld_club_->tooltip("Upload extracted records to ClubLog");

	curr_y += HBUTTON;

	// Title
	Fl_Box* box_q = new Fl_Box(C1, curr_y, W1, HBUTTON, "Cards");
	box_e->box(FL_FLAT_BOX);
	box_e->color(FL_BACKGROUND_COLOR);
	//// Auto upload
	//bn_auto_eqsl_ = new Fl_Check_Button(C2, curr_y, W2, HBUTTON);
	//bn_auto_eqsl_->value(auto_eqsl_);
	//bn_auto_eqsl_->callback(cb_auto, (void*)extract_data::EQSL);
	//bn_auto_eqsl_->tooltip("Enable automatic upload of eQSL after logging");
	//// Download
	//bn_down_eqsl_ = new Fl_Button(C3, curr_y, W3, HBUTTON, "@2->");
	//bn_down_eqsl_->callback(cb_download, (void*)extract_data::EQSL);
	//bn_down_eqsl_->tooltip("Download latest records from eQSL");
	// Extract
	bn_extr_card_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_card_->callback(cb_extract, (void*)extract_data::CARD);
	bn_extr_card_->tooltip("Extract records for printing labels");
	// print
	bn_print_ = new Fl_Button(C5, curr_y, W5, HBUTTON, "@fileprint");
	bn_print_->callback(cb_upload);
	bn_print_->tooltip("Print labels from extracted records");
	// Mark read
	bn_mark_done_ = new Fl_Button(C6, curr_y, W6, HBUTTON, "Done");
	bn_mark_done_->callback(cb_mark_done);
	bn_mark_done_->tooltip("Mark extracted records as printed");

	curr_y += HBUTTON + GAP;

	resizable(nullptr);
	size(max_x - x(), curr_y - y());
	end();

}
void qso_qsl::save_values() {
	// eQSL
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences eqsl_settings(qsl_settings, "eQSL");
	eqsl_settings.set("Upload per QSO", auto_eqsl_);
	// LotW
	Fl_Preferences lotw_settings(qsl_settings, "LotW");
	lotw_settings.set("Upload per QSO", auto_lotw_);
	// Clublog
	Fl_Preferences club_settings(qsl_settings, "ClubLog");
	club_settings.set("Upload per QSO", auto_club_);
}

void qso_qsl::enable_widgets() {
	// Disable extract and upload buttons if auto-upload enabled
	if (extract_records_->size() && extract_records_->use_mode() == extract_data::EQSL) bn_upld_eqsl_->activate();
	else bn_upld_eqsl_->deactivate();
	if (extract_records_->size() && extract_records_->use_mode() == extract_data::LOTW) bn_upld_lotw_->activate();
	else bn_upld_lotw_->deactivate();
	if (extract_records_->size() && extract_records_->use_mode() == extract_data::CLUBLOG) bn_upld_club_->activate();
	else bn_upld_club_->deactivate();
	if (extract_records_->size() && extract_records_->use_mode() == extract_data::CARD) {
		bn_print_->activate();
		bn_mark_done_->activate();
	}
	else { 
		bn_print_->deactivate();
		bn_mark_done_->deactivate();
	}
}

// callbacks
// Auto download v = eQSL/LotW/ClubLog
void qso_qsl::cb_auto(Fl_Widget* w, void* v) {
	Fl_Check_Button* bn = (Fl_Check_Button*)w;
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	bool* enable;
	extract_data::extract_mode_t server = (extract_data::extract_mode_t)(intptr_t)v;
	switch (server) {
	case extract_data::EQSL:
		enable = &that->auto_eqsl_;
		break;
	case extract_data::LOTW:
		enable = &that->auto_lotw_;
		break;
	case extract_data::CLUBLOG:
		enable = &that->auto_club_;
		break;
	}
	*enable = bn->value();
	that->save_values();
	that->enable_widgets();
}

// Download. v = eQSL/LotW (import data enum)
void qso_qsl::cb_download(Fl_Widget* w, void* v) {
	// v has QSL server
	import_data::update_mode_t server = (import_data::update_mode_t)(intptr_t)v;
	import_data_->download_data(server);
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->enable_widgets();
}

// Extract. v = eQSL/LotW/ClubLog/Card
void qso_qsl::cb_extract(Fl_Widget* w, void* v) {
	extract_data::extract_mode_t server = (extract_data::extract_mode_t)(intptr_t)v;
	extract_records_->extract_qsl(server);
	tabbed_forms_->activate_pane(OT_EXTRACT, true);
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->enable_widgets();
}

// Upload. v = eQSL/LotW/Clublog
void qso_qsl::cb_upload(Fl_Widget* w, void* v) {
	// extract_records has remembered the server
	extract_records_->upload();
	tabbed_forms_->activate_pane(OT_MAIN, true);
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->enable_widgets();
	book_->modified(true);
}

// Print . Card only
void qso_qsl::cb_print(Fl_Widget* w, void* v) {
	printer* ptr = new printer(OT_CARD);
	delete ptr;
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->enable_widgets();
}


// Mark printing done
void qso_qsl::cb_mark_done(Fl_Widget* w, void* v) {
	if (extract_records_->size()) {
		char message[200];
		string date_name;
		string sent_name;
			date_name = "QSLSDATE";
			sent_name = "QSL_SENT";
		string today = now(false, "%Y%m%d");
		snprintf(message, 200, "EXTRACT: Setting %s to \"%s\", %s to \"Y\"", date_name.c_str(), today.c_str(), sent_name.c_str());
		status_->misc_status(ST_NOTE, message);
		for (auto it = extract_records_->begin(); it != extract_records_->end(); it++) {
			(*it)->item(date_name, today);
			(*it)->item(sent_name, string("Y"));
		}
		book_->modified(true);
	}
	else {
		status_->misc_status(ST_WARNING, "EXTRACT: No records to change");
	}
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->enable_widgets();
}

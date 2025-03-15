#include "qso_qsl.h"
#include "qso_manager.h"
#include "qso_data.h"
#include "drawing.h"
#include "extract_data.h"
#include "import_data.h"
#include "tabbed_forms.h"
#include "printer.h"
#include "status.h"
#include "book.h"
#include "ticker.h"
#include "record.h"
#include "png_writer.h"
#include "callback.h"
#include "eqsl_handler.h"
#include "club_handler.h"
#include "lotw_handler.h"
#include "qsl_data.h"
#include "qsl_image.h"
#include "qsl_emailer.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Fill_Dial.H>

extern Fl_Preferences* settings_;
extern import_data* import_data_;
extern extract_data* extract_records_;
extern tabbed_forms* tabbed_forms_;
extern status* status_;
extern book* book_;
extern book* navigation_book_;
extern ticker* ticker_;
extern eqsl_handler* eqsl_handler_;
extern club_handler* club_handler_;
extern lotw_handler* lotw_handler_;

// Constructor
qso_qsl::qso_qsl(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	// Log status group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	os_eqsl_dnld_ = 0;
	tkr_value_ = 0.0;
	extract_in_progress_ = false;
	single_qso_ = false;
	via_code_ = "";
	load_values();
	create_form();
	enable_widgets();

	ticker_->add_ticker(this, cb_ticker, 10);
}

// Destructor
qso_qsl::~qso_qsl() {
	save_values();
	ticker_->remove_ticker(this);
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

// Draw the widgets
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
	const int C7 = C6 + W6;
	const int W7 = WBUTTON / 2;
	int max_x = C7 + W7 + GAP;
	int curr_y = y() + GAP;

	// Single QSO button
	bn_single_qso_ = new Fl_Check_Button(C2, curr_y, W2, HBUTTON, "Only handle selected QSO");
	bn_single_qso_->align(FL_ALIGN_RIGHT);
	bn_single_qso_->value(single_qso_);
	bn_single_qso_->callback(cb_single, &single_qso_);
	bn_single_qso_->tooltip("Only uploads or send e-mail for the current selected QSO");

	curr_y += HBUTTON + GAP;

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
	bn_upld_eqsl_ = new Fl_Button(C6, curr_y, W6, HBUTTON, "@8->");
	bn_upld_eqsl_->callback(cb_upload, (void*)extract_data::EQSL);
	bn_upld_eqsl_->tooltip("Upload extracted records to eQSL");
	// Fetch cards
	op_eqsl_count_ = new Fl_Fill_Dial(C7 + W7 - HBUTTON, curr_y, HBUTTON, HBUTTON, nullptr);
	char text[10];
	snprintf(text, sizeof(text), "%d", os_eqsl_dnld_);
	op_eqsl_count_->copy_label(text);
	op_eqsl_count_->labelcolor(FL_FOREGROUND_COLOR);
	op_eqsl_count_->tooltip("Displays the number of outstanding image downloads");
	op_eqsl_count_->minimum(0.0);
	op_eqsl_count_->maximum(1.0);
	op_eqsl_count_->value(0.0);
	op_eqsl_count_->color(FL_WHITE, FL_YELLOW);
	op_eqsl_count_->align(FL_ALIGN_INSIDE);
	op_eqsl_count_->angles(180, 540);

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
	bn_upld_lotw_ = new Fl_Button(C6, curr_y, W6, HBUTTON, "@8->");
	bn_upld_lotw_->callback(cb_upload, (void*)extract_data::LOTW);
	bn_upld_lotw_->tooltip("Upload extracted records to LotW");

	curr_y += HBUTTON;

	Fl_Box* box_c = new Fl_Box(C1, curr_y, W1, HBUTTON, "ClubLog");
	box_c->box(FL_FLAT_BOX);
	box_c->color(FL_BACKGROUND_COLOR);
	// Auto upload
	bn_auto_club_ = new Fl_Check_Button(C2, curr_y, W2, HBUTTON);
	bn_auto_club_->value(auto_club_);
	bn_auto_club_->callback(cb_auto, (void*)extract_data::CLUBLOG);
	bn_auto_club_->tooltip("Enable automatic upload of ClubLog after logging");
	// Extract
	bn_extr_club_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_club_->callback(cb_extract, (void*)extract_data::CLUBLOG);
	bn_extr_club_->tooltip("Extract records for upload to ClubLog");
	// Upload
	bn_upld_club_ = new Fl_Button(C6, curr_y, W6, HBUTTON, "@8->");
	bn_upld_club_->callback(cb_upload, (void*)extract_data::CLUBLOG);
	bn_upld_club_->tooltip("Upload extracted records to ClubLog");

	curr_y += HBUTTON;

	// Title
	Fl_Box* box_q = new Fl_Box(C1, curr_y, W1, HBUTTON, "Cards");
	box_q->box(FL_FLAT_BOX);
	box_q->color(FL_BACKGROUND_COLOR);
	// Extract
	bn_extr_card_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_card_->callback(cb_extract, (void*)extract_data::CARD);
	bn_extr_card_->tooltip("Extract records for printing labels");
	// print
	bn_print_ = new Fl_Button(C5, curr_y, W5, HBUTTON, "@fileprint");
	bn_print_->callback(cb_print);
	bn_print_->tooltip("Print labels from extracted records");
	// Mark read
	bn_card_done_ = new Fl_Button(C7, curr_y, W7, HBUTTON, "Done");
	bn_card_done_->callback(cb_mark_done, (void*)(intptr_t)'B');
	bn_card_done_->tooltip("Mark extracted records as printed");

	curr_y += HBUTTON;

	// Title
	Fl_Box* box_m = new Fl_Box(C1, curr_y, W1, HBUTTON, "e-Mails");
	box_m->box(FL_FLAT_BOX);
	box_m->color(FL_BACKGROUND_COLOR);
	// Extract
	bn_extr_email_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@search");
	bn_extr_email_->callback(cb_extract, (void*)extract_data::EMAIL);
	bn_extr_email_->tooltip("Extract records for sending e-mails");
	// print
	bn_png_ = new Fl_Button(C5, curr_y, W5, HBUTTON, "@filesave");
	bn_png_->callback(cb_png);
	bn_png_->tooltip("Generate PNG files for sending");
	// Send e-mail
	bn_send_email_ = new Fl_Button(C6, curr_y, W6, HBUTTON, "@mail");
	bn_send_email_->labelsize(HBUTTON - 2);
	bn_send_email_->callback(cb_email);
	bn_send_email_->tooltip("Send e-mails for extracetd QSOs");
	// Mark read
	bn_email_done_ = new Fl_Button(C7, curr_y, W7, HBUTTON, "Done");
	bn_email_done_->callback(cb_mark_done, (void*)(intptr_t)'E');
	bn_email_done_->tooltip("Mark extracted records as printed");

	curr_y += HBUTTON;
	// cacncel 
	bn_cancel_ = new Fl_Button(C4, curr_y, W4, HBUTTON, "@undo");
	bn_cancel_->callback(cb_cancel, nullptr);
	bn_cancel_->tooltip("Cancel extract");

	curr_y += HBUTTON + GAP;

	resizable(nullptr);
	size(max_x - x(), curr_y - y());
	end();

}

// Save settings
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
	settings_->flush();
}

// Configure widgets
void qso_qsl::enable_widgets() {
	// Single QSO - do this first as the valeu of single_qso_ affects the others
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	if (mgr->data()->current_qso()) bn_single_qso_->activate();
	else {
		single_qso_ = false;
		bn_single_qso_->value(single_qso_);
		bn_single_qso_->deactivate();
	}
	// Disable download and extract buttons if not is the correct state
	if (mgr->data()->inactive() && os_eqsl_dnld_ == 0 && !extract_in_progress_ && !single_qso_) {
		bn_down_eqsl_->activate();
		bn_down_lotw_->activate();
		bn_extr_club_->activate();
		bn_extr_eqsl_->activate();
		bn_extr_lotw_->activate();
		bn_extr_card_->activate();
		bn_extr_email_->activate();
	} else {
		bn_down_eqsl_->deactivate();
		bn_down_lotw_->deactivate();
		bn_extr_club_->deactivate();
		bn_extr_eqsl_->deactivate();
		bn_extr_lotw_->deactivate();
		bn_extr_card_->deactivate();
		bn_extr_email_->deactivate();
	}
	// Cancel button
	if (!extract_in_progress_ && extract_records_->size()) {
		bn_cancel_->activate();
	}
	else {
		bn_cancel_->deactivate();
	}
	// Disable extract and upload buttons if auto-upload enabled
	if (!extract_in_progress_ && extract_records_->use_mode() == extract_data::EQSL) {
		bn_upld_eqsl_->activate();
	}
	else if (single_qso_) {
		bn_upld_eqsl_->activate();
	}
	else {
		bn_upld_eqsl_->deactivate();
	}
	if (!extract_in_progress_ && extract_records_->use_mode() == extract_data::LOTW) {
		bn_upld_lotw_->activate();
	}
	else if (single_qso_) {
		bn_upld_lotw_->activate();
	}
	else {
		bn_upld_lotw_->deactivate();
	}
	if (!extract_in_progress_ && extract_records_->use_mode() == extract_data::CLUBLOG) {
		bn_upld_club_->activate();
	}
	else if (single_qso_) {
		bn_upld_club_->activate();
	}
	else {
		bn_upld_club_->deactivate();
	}
	if (!extract_in_progress_ && extract_records_->use_mode() == extract_data::CARD && !single_qso_) {
		bn_print_->activate();
		bn_card_done_->activate();
	}
	else {
		bn_print_->deactivate();
		bn_card_done_->deactivate();
	}
	if (!extract_in_progress_ && extract_records_->use_mode() == extract_data::EMAIL) {
		bn_png_->activate();
		bn_send_email_->activate();
		bn_email_done_->activate();
	}
	else if (single_qso_) {
		bn_png_->activate();
		bn_send_email_->activate();
		bn_email_done_->activate();
	}
	else {
		bn_png_->deactivate();
     	bn_send_email_->deactivate();
		bn_email_done_->deactivate();
	}
	char text[10];
	int curr = atoi(op_eqsl_count_->label());
	snprintf(text, sizeof(text), "%d", os_eqsl_dnld_);
	op_eqsl_count_->copy_label(text);
	if (os_eqsl_dnld_ == 0) {
		op_eqsl_count_->labelcolor(FL_FOREGROUND_COLOR);
		op_eqsl_count_->labelfont(0);
		op_eqsl_count_->color(FL_BACKGROUND_COLOR);
		op_eqsl_count_->value(0.0);
	} else if ( os_eqsl_dnld_ > curr) {
		op_eqsl_count_->labelcolor(FL_FOREGROUND_COLOR);
		op_eqsl_count_->labelfont(FL_BOLD);
		op_eqsl_count_->color(FL_WHITE, FL_DARK_YELLOW);
		op_eqsl_count_->value(tkr_value_);
	} else {
		op_eqsl_count_->labelcolor(FL_RED);
		op_eqsl_count_->labelfont(FL_BOLD);
		op_eqsl_count_->color(FL_WHITE, FL_DARK_YELLOW);
		op_eqsl_count_->value(tkr_value_);
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
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->qsl_download(server);
}

// Extract. v = eQSL/LotW/ClubLog/Card
void qso_qsl::cb_extract(Fl_Widget* w, void* v) {
	extract_data::extract_mode_t server = (extract_data::extract_mode_t)(intptr_t)v;
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->qsl_extract(server);
}

// Upload. v = eQSL/LotW/Clublog
void qso_qsl::cb_upload(Fl_Widget* w, void* v) {
	// extract_records has remembered the server
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	if (that->single_qso_) that->qsl_1_upload((extract_data::extract_mode_t)(intptr_t)v);
	else that->qsl_upload();
}

// Print . Card only
void qso_qsl::cb_print(Fl_Widget* w, void* v) {
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->qsl_print();
}

// Mark printing done
void qso_qsl::cb_mark_done(Fl_Widget* w, void* v) {
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	string via = { (char)(intptr_t)v };
	that->qsl_mark_done();
}

// Cancel extraction
void qso_qsl::cb_cancel(Fl_Widget* w, void* v) {
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	that->qsl_cancel();
}

// Generate PNG files
void qso_qsl::cb_png(Fl_Widget* w, void* v) {
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	if (that->single_qso_) that->qsl_1_generate_png();
	else that->qsl_generate_png();
}

// Send e-mails
void qso_qsl::cb_email(Fl_Widget* w, void* v) {
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	if (that->single_qso_) that->qsl_1_send_email();
	else that->qsl_send_email();
}

// Single QSO
void qso_qsl::cb_single(Fl_Widget* w, void* v) {
	qso_qsl* that = ancestor_view<qso_qsl>(w);
	cb_value< Fl_Check_Button, bool >(w, v);
	that->enable_widgets();
}

// Download. v = eQSL/LotW (import data enum)
void qso_qsl::qsl_download(import_data::update_mode_t server) {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	if (mgr->data()->inactive()) {
		import_data_->download_data(server);
		enable_widgets();
	} else {
		status_->misc_status(ST_ERROR, "Not ready to download - finish operating");
	}
}

// Extract 
void qso_qsl::qsl_extract(extract_data::extract_mode_t server) {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	if (mgr->data()->inactive()) {
		// Set flag to indicate this is being done - used to disable further attempts
		extract_in_progress_ = true;
		enable_widgets();
		extract_records_->extract_qsl(server);
		extract_in_progress_ = false;
		tabbed_forms_->activate_pane(OT_EXTRACT, true);
		navigation_book_->selection(0);
		enable_widgets();
	} else {
		status_->misc_status(ST_ERROR, "Not ready to extract - finish operating");
	}
}

// Upload
void qso_qsl::qsl_upload() {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	if (mgr->data()->inactive()) {
		extract_records_->upload();
		tabbed_forms_->activate_pane(OT_MAIN, true);
		enable_widgets();
	} else {
		status_->misc_status(ST_ERROR, "Not ready to upload - finish operating");
	}
}

// Upload single
void qso_qsl::qsl_1_upload(extract_data::extract_mode_t mode) {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	qso_num_t number = mgr->data()->current_number();
	if (number != -1) {
		switch (mode) {
		case extract_data::EQSL: {
			eqsl_handler_->upload_single_qso(number);
			break;
		}
		case extract_data::CLUBLOG: {
			club_handler_->upload_single_qso(number);
			break;
		}
		case extract_data::LOTW: {
			lotw_handler_->upload_single_qso(number);
			break;
		}
		}
	} else {
		status_->misc_status(ST_ERROR, "QSL: Have no QSO to upload");
	}
}

// Print
void qso_qsl::qsl_print() {
	printer* ptr = new printer(OT_CARD);
	if (ptr->do_job()) {
		status_->misc_status(ST_ERROR, "DASH: Printing cards failed");
	} else {
		status_->misc_status(ST_OK, "DASH: Printing cards OK");
		via_code_ = "B";
	}
	delete ptr;
	enable_widgets();
}

// Mark printing done
void qso_qsl::qsl_mark_done() {
	if (extract_records_->size() || single_qso_) {
		char message[200];
		string date_name = "QSLSDATE";
		string sent_name = "QSL_SENT";
		string via_name = "QSL_SENT_VIA";
		string today = now(false, "%Y%m%d");
		snprintf(message, 200, "EXTRACT: Setting %s to \"%s\", %s to \"Y\"", date_name.c_str(), today.c_str(), sent_name.c_str());
		status_->misc_status(ST_NOTE, message);
		if (single_qso_) {
			qso_manager* mgr = ancestor_view<qso_manager>(this);
			record* qso = mgr->data()->current_qso();
			if (qso) {
				qso->item(date_name, today);
				qso->item(sent_name, string("Y"));
				qso->item(via_name, via_code_);
			}
		}
		else for (auto it = extract_records_->begin(); it != extract_records_->end(); it++) {
			(*it)->item(date_name, today);
			(*it)->item(sent_name, string("Y"));
			(*it)->item(via_name, via_code_);
		}
		book_->modified(true);
	}
	else {
		status_->misc_status(ST_WARNING, "EXTRACT: No records to change");
	}
	enable_widgets();
}

// Cancel uploads
void qso_qsl::qsl_cancel() {
	extract_records_->clear_criteria();
	enable_widgets();
}

// Generate PNG files
void qso_qsl::qsl_generate_png() {
	status_->misc_status(ST_LOG, "QSL: Starting to generate PNG files");
	png_writer* png = new png_writer();
	if (png->write_book(extract_records_)) {
		status_->misc_status(ST_OK, "QSL: PNG file generation done!");
	}
	else {
		status_->misc_status(ST_ERROR, "QSL: PNG file generation failed");
	}
}

// Generate single PNG file
void qso_qsl::qsl_1_generate_png() {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	record* qso = mgr->data()->current_qso();
	if (qso) {
		png_writer* png = new png_writer();
		Fl_RGB_Image* image = qsl_image::image(qso, qsl_data::FILE);
		string filename = png_writer::png_filename(qso);
		if (png->write_image(image, filename)) {
			status_->misc_status(ST_ERROR, "QSL: Single PNG file generation failed");
		}
	}
	else {
		status_->misc_status(ST_ERROR, "QSL: No QSO to generate PNG file for");
	}
}

// Send e--mails
void qso_qsl::qsl_send_email() {
	for (auto it = extract_records_->begin(); it != extract_records_->end(); it++) {
		qsl_1_send_email(*it);
	}
}

// Send single e-mail
void qso_qsl::qsl_1_send_email(record* qso) {
	qsl_emailer* emailer = new qsl_emailer;
	if (emailer->generate_email(qso)) {
		if (emailer->send_email()) {
			status_->misc_status(ST_OK, "QSL: E-mail sent");
			via_code_ = "E";
		}
		else {
			status_->misc_status(ST_ERROR, "QSL: E-mail not sent");
		}
	}
	else {
		char msg[128];
		snprintf(msg, sizeof(msg), "QSL: Did not generate e-mail for %s", qso->item("CALL").c_str());
		status_->misc_status(ST_ERROR, msg);
	}
}

// Send single e-mail to current QSO
void qso_qsl::qsl_1_send_email() {
	qso_manager* mgr = ancestor_view<qso_manager>(this);
	record* qso = mgr->data()->current_qso();
	if (qso) {
		qsl_1_send_email(qso);
	}
	else {
		status_->misc_status(ST_ERROR, "QSL: No QSO selected for send_email");
	}
}

// Update eQSL download count
void qso_qsl::update_eqsl(int count) {
	os_eqsl_dnld_ = count;
	tkr_value_ = 0.0;
	enable_widgets();
}

// Update ticker
void qso_qsl::ticker() {
	if (os_eqsl_dnld_ > 0) {
		if (tkr_value_ < 1.0) {
			tkr_value_ += 0.1;
			enable_widgets();
		}
	}
}

// 1 s ticker
void qso_qsl::cb_ticker(void* v) {
	((qso_qsl*)v)->ticker();
}
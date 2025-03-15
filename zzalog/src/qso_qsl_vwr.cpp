#include "qso_qsl_vwr.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "eqsl_handler.h"
#include "status.h"
#include "book.h"
#include "record.h"
#include "qsl_display.h"
#include "qsl_widget.h"
#include "qsl_dataset.h"
#include "callback.h"
#include "drawing.h"
#include "tabbed_forms.h"
#include "utils.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Box.H>



extern eqsl_handler* eqsl_handler_;
extern Fl_Preferences* settings_;
extern status* status_;
extern book* book_;
extern string default_station_;
extern tabbed_forms* tabbed_forms_;
extern qsl_dataset* qsl_dataset_;

// Constructor
qso_qsl_vwr::qso_qsl_vwr(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, selected_image_(QI_NONE)
	, raw_image_(nullptr)
	, current_qso_(nullptr)
	, qso_changed_(true)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	load_values();
	create_form();
	enable_widgets();
}

// Destructor
qso_qsl_vwr::~qso_qsl_vwr() {
}

// Load values
void qso_qsl_vwr::load_values() {
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.get("Image Type", (int&)selected_image_, QI_NONE);
	Fl_Preferences datapath(settings_, "Datapath");
	char* temp;
	if (!datapath.get("QSLs", temp, "")) {
		//Fl_File_Chooser* chooser = new Fl_File_Chooser("", nullptr, Fl_File_Chooser::DIRECTORY,
		//	"Select QSL card directory");
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select QSL Card directory");
		if (chooser->show() == 0) {
			qsl_directory_ = chooser->filename();
		}
		datapath.set("QSLs", qsl_directory_.c_str());
		settings_->flush();
		delete chooser;
	}
	else {
		qsl_directory_ = temp;
	}
}

// Save values
void qso_qsl_vwr::save_values() {
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.set("Image Type", selected_image_);
	Fl_Preferences datapath(settings_, "Datapath");
	datapath.set("QSLs", qsl_directory_.c_str());
	settings_->flush();
}

// Create form
void qso_qsl_vwr::create_form() {

	begin();

	// Initial size of card
	const int WCARD = 540;
	const int HCARD = 340;

	int curr_x = x();
	int curr_y = y();
	// Create image size in ratio 540x340 to fit supplied width
	const int WIMAGE = w();
	const int HIMAGE = w() * HCARD / WCARD;


	// Box - for the display of a card image 
	qsl_thumb_ = new qsl_widget(curr_x, curr_y, WIMAGE, HIMAGE, "Image");
	qsl_thumb_->box(FL_BORDER_BOX);
	qsl_thumb_->tooltip("The card image is displayed here!");
	qsl_thumb_->callback(cb_bn_image, nullptr);
	qsl_thumb_->when(FL_WHEN_RELEASE);

	curr_y += qsl_thumb_->h();

	curr_x = x() + GAP;

	// Row header
	Fl_Box* label1 = new Fl_Box(curr_x, curr_y, WSMEDIT, HRADIO, "QSL status..");
	label1->box(FL_FLAT_BOX);
	label1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	label1->color(FL_BACKGROUND_COLOR);

	curr_y += HTEXT;
	curr_x = x() + GAP;

	Fl_Box* label1A = new Fl_Box(curr_x, curr_y, WRADIO, HRADIO, "R");
	label1A->box(FL_FLAT_BOX);
	label1A->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	label1A->color(FL_BACKGROUND_COLOR);

	curr_x += WRADIO;

	// Light to indicate received eQSL
	bn_eqsl_rstatus_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HRADIO, "eQSL");
	bn_eqsl_rstatus_->box(FL_FLAT_BOX);
	bn_eqsl_rstatus_->type(bn_eqsl_rstatus_->type() & ~FL_TOGGLE_BUTTON);

	curr_x += WBUTTON;
	// Light to indicate received LotW
	bn_lotw_rstatus_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HRADIO, "LotW");
	bn_lotw_rstatus_->box(FL_FLAT_BOX);
	bn_lotw_rstatus_->type(bn_lotw_rstatus_->type() & ~FL_TOGGLE_BUTTON);

	curr_x += WBUTTON;
	// Light to indicate received a paper card
	bn_card_rstatus_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HRADIO, "Card");
	bn_card_rstatus_->box(FL_FLAT_BOX);
	bn_card_rstatus_->type(bn_card_rstatus_->type() & ~FL_TOGGLE_BUTTON);

	curr_y += HRADIO;
	curr_x = x() + GAP;

	Fl_Box* label1B = new Fl_Box(curr_x, curr_y, WRADIO, HRADIO, "S");
	label1B->box(FL_FLAT_BOX);
	label1B->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	label1B->color(FL_BACKGROUND_COLOR);

	curr_x += WRADIO;

	// Light to indicate received eQSL
	bn_eqsl_sstatus_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HRADIO, "eQSL");
	bn_eqsl_sstatus_->box(FL_FLAT_BOX);
	bn_eqsl_sstatus_->type(bn_eqsl_sstatus_->type() & ~FL_TOGGLE_BUTTON);

	curr_x += WBUTTON;
	// Light to indicate received LotW
	bn_lotw_sstatus_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HRADIO, "LotW");
	bn_lotw_sstatus_->box(FL_FLAT_BOX);
	bn_lotw_sstatus_->type(bn_lotw_sstatus_->type() & ~FL_TOGGLE_BUTTON);

	curr_x += WBUTTON;
	// Light to indicate received a paper card
	bn_card_sstatus_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HRADIO, "Card");
	bn_card_sstatus_->box(FL_FLAT_BOX);
	bn_card_sstatus_->type(bn_card_sstatus_->type() & ~FL_TOGGLE_BUTTON);

	curr_y += HRADIO;
	curr_x = x() + GAP;

	// Row header
	Fl_Box* label3 = new Fl_Box(curr_x, curr_y, WSMEDIT, HTEXT, "Display card...");
	label3->box(FL_FLAT_BOX);
	label3->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	label3->color(FL_BACKGROUND_COLOR);

	curr_y += HTEXT;
	curr_x = x() + GAP;
	// Group the following radio buttons
	grp_card_type_ = new Fl_Group(curr_x, curr_y, w() - GAP * 2, HBUTTON * 2);
	grp_card_type_->box(FL_FLAT_BOX);

	// Radio - Display eQSL.cc card image
	radio_eqsl_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "eQSL");
	radio_eqsl_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_eqsl_->selection_color(FL_FOREGROUND_COLOR);
	radio_eqsl_->callback(cb_rad_card, (void*)QI_EQSL);
	radio_eqsl_->when(FL_WHEN_RELEASE_ALWAYS);
	radio_eqsl_->tooltip("Select image downloaded from eQSL");
	curr_x += WBUTTON;
	// Radio - display scanned image of front of paper card
	radio_card_front_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Front");
	radio_card_front_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_card_front_->selection_color(FL_FOREGROUND_COLOR);
	radio_card_front_->callback(cb_rad_card, (void*)QI_CARD_FRONT);
	radio_card_front_->when(FL_WHEN_RELEASE_ALWAYS);
	radio_card_front_->tooltip("Select image scanned of paper card front");
	curr_x += WBUTTON;
	// Radio - display scanned image of reverse of paper card
	radio_card_back_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Back");
	radio_card_back_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_card_back_->selection_color(FL_FOREGROUND_COLOR);
	radio_card_back_->callback(cb_rad_card, (void*)QI_CARD_BACK);
	radio_card_back_->when(FL_WHEN_RELEASE_ALWAYS);
	radio_card_back_->tooltip("Select image scanned of paper card back");
	curr_x = x() + GAP;
	curr_y += HBUTTON;
	// Radio - display received e-mail image
	radio_emailr_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "eMail (R)");
	radio_emailr_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_emailr_->selection_color(FL_FOREGROUND_COLOR);
	radio_emailr_->callback(cb_rad_card, (void*)QI_EMAILR);
	radio_emailr_->when(FL_WHEN_RELEASE_ALWAYS);
	radio_emailr_->tooltip("Select image received by e-mail");
	curr_x += WBUTTON;
	// Radio - display my QSL
	radio_label_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Label");
	radio_label_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_label_->selection_color(FL_FOREGROUND_COLOR);
	radio_label_->callback(cb_rad_card, (void*)QI_LABEL);
	radio_label_->when(FL_WHEN_RELEASE_ALWAYS);
	radio_label_->tooltip("Select QSL label as printed");
	curr_x += WBUTTON;
	// Radio - display sending e-mail image
	radio_emails_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "eMail (S)");
	radio_emails_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_emails_->selection_color(FL_FOREGROUND_COLOR);
	radio_emails_->callback(cb_rad_card, (void*)QI_EMAILS);
	radio_emails_->when(FL_WHEN_RELEASE_ALWAYS);
	radio_emails_->tooltip("Select image to be sent by e-mail");

	grp_card_type_->end();

	curr_x += WBUTTON;

	curr_y += HBUTTON;
	curr_x = x() + GAP;

	// Row header
	Fl_Box* label2 = new Fl_Box(curr_x, curr_y, WSMEDIT, HTEXT, "Log received");
	label2->box(FL_FLAT_BOX);
	label2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	label2->color(FL_BACKGROUND_COLOR);

	curr_x = x() + GAP;
	curr_y += HTEXT;
	// Button - Mark paper card received
	bn_log_bureau_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Bureau");
	bn_log_bureau_->tooltip("Log a paper card received from the bureau on today's date");
	bn_log_bureau_->callback(cb_bn_log_card, (void*)"B");
	bn_log_bureau_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;
	// Button - Mark e-mail received
	bn_log_email_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "e-mail");
	bn_log_email_->tooltip("Log an e-mail card received on today's date");
	bn_log_email_->callback(cb_bn_log_card, (void*)"E");
	bn_log_email_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;
	// Button - Mark e-mail received
	bn_log_direct_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Direct");
	bn_log_direct_->tooltip("Log a paper card received direct on today's date");
	bn_log_direct_->callback(cb_bn_log_card, (void*)"D");
	bn_log_direct_->when(FL_WHEN_RELEASE);

	curr_x = x() + GAP;
	curr_y += HBUTTON + GAP;;
	// Button - Fetch the card image from eQSL.cc
	bn_fetch_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Fetch");
	bn_fetch_->tooltip("Request a fresh download of the eQSL image");
	bn_fetch_->callback(cb_bn_fetch, nullptr);
	bn_fetch_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;
	// Button - Set QSL_SENT=R in current QSO
	bn_card_reqd_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Requested");
	bn_card_reqd_->tooltip("Mark QSO as wanting a card QSL");
	bn_card_reqd_->callback(cb_bn_card_reqd, (void*)"R");
	bn_card_reqd_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;
	// Button - QSO partner declines a card - set QSL_SENT=N
	bn_card_decl_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Declined");
	bn_card_decl_->tooltip("Mark QSO as declining a card QSL");
	bn_card_decl_->callback(cb_bn_card_reqd, (void*)"N");
	bn_card_decl_->when(FL_WHEN_RELEASE);
	
	curr_x += WBUTTON;

	end();
	show();

	Fl_Group* sv = Fl_Group::current();
	Fl_Group::current(nullptr);

	// Now create the full view window - this has three alternate widgets
	win_full_view_ = new Fl_Window(WCARD, HCARD);
	// Display the received image at full size
	qsl_full_ = new qsl_widget(0, 0, WCARD, HCARD);
	qsl_full_->box(FL_FLAT_BOX);
	//// Display a warning - No image available!
	//bn_no_image_ = new Fl_Button(0, 0, WCARD, HCARD);
	//bn_no_image_->box(FL_FLAT_BOX);
	//bn_no_image_->label("No image available!");
	//bn_no_image_->labelsize(FL_NORMAL_SIZE * 3);
	//bn_no_image_->labelcolor(FL_RED);

	win_full_view_->resizable(qsl_full_);
	win_full_view_->end();
	win_full_view_->hide();

	Fl_Group::current(sv);
}

// Call-backs
// called when any of the image selection radio buttons is released
// v is enum image_t.
void qso_qsl_vwr::cb_rad_card(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	// Get the selected image and display it.
	image_t value = (image_t)(intptr_t)v;
	if (value == that->selected_image_) {
		((Fl_Light_Button*)w)->value(false);
		that->set_selected_image(QI_NONE);
	} else {
		that->set_selected_image((image_t)(intptr_t)v);
	}
	that->set_image();
	// Save setting
	//	main_window_->flush();
}

// Fetch the eQSL card for the supplied record
// v is record_num_t* containing the number of the record to fetch the eQSL card for
void qso_qsl_vwr::cb_bn_fetch(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	// Put the card request onto the eQSL request queue - so that requests are made
	eqsl_handler_->enqueue_request(that->current_qso_num_, true);
	eqsl_handler_->enable_fetch(eqsl_handler::EQ_START);
	// Wait until donwload complete
	while (eqsl_handler_->requests_queued()) Fl::check();
	that->set_selected_image(QI_EQSL);
	that->set_image();
	that->enable_widgets();
}

// Set the QSL_RCVD and QSLRDATE values in current record
// Set QSL_SENT to QUEUED if not already set
// v selects scanned card (QI_CARD_FRONT) or e-mail (QI_EMAIL)
void qso_qsl_vwr::cb_bn_log_card(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	const char* source = (const char*)v;
	string today = now(false, "%Y%m%d");
	that->current_qso_->item("QSLRDATE", today);
	that->current_qso_->item("QSL_RCVD", string("Y"));
	that->current_qso_->item("QSL_RCVD_VIA", string(source));
	if (that->current_qso_->item("QSL_SENT") != string("Y") ||
		that->current_qso_->item("QSL_SENT_VIA") != string(source)) {
		that->current_qso_->item("QSL_SENT", string("Q"));
		that->current_qso_->item("QSL_SENT_VIA", string(source));
	}
	// now pretend the Card Front radio button has been pressed
	if (*source == 'E') {
		cb_rad_card(that->radio_card_front_, (void*)QI_EMAILR);
	} else {
		cb_rad_card(that->radio_card_front_, (void*)QI_CARD_FRONT);
	}
	that->set_image_buttons();
	book_->selection(that->current_qso_num_, HT_MINOR_CHANGE);
}

// Show/hide full view window
// v is not used
void qso_qsl_vwr::cb_bn_image(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	if (that->win_full_view_->visible()) {
		that->win_full_view_->hide();
	}
	else {
		that->update_full_view();
		that->win_full_view_->show();
	}
}

// Set card requested "QSL_SENT=[v]"
// v is a pointer to const char - either R or N
void qso_qsl_vwr::cb_bn_card_reqd(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	char* value = (char*)v;
	that->current_qso_->item("QSL_SENT", string(value));
	switch (*value) {
	case 'R':
		that->current_qso_->item("QSL_SENT_VIA", string("B"));
		break;
	}
	qso_data* data = ancestor_view<qso_data>(that);
	data->enable_widgets();
	tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, that->current_qso_num_);

}

// set the current QSO
void qso_qsl_vwr::set_qso(record* qso, qso_num_t number) {
	if (current_qso_ == qso) qso_changed_ = false;
	else qso_changed_ = true;
	current_qso_ = qso;
	current_qso_num_ = number;
	enable_widgets();
}

// Redraw the form after stuff has changed
void qso_qsl_vwr::enable_widgets() {
	set_image();
	set_qsl_status();
	char title[128];
	if (current_qso_ == nullptr || current_qso_->item("CALL").length() == 0) {
		strcpy(title, "No contact");
	}
	else {
		snprintf(title, 128, "QSL: %s %s %s %s %s",
			current_qso_->item("CALL").c_str(),
			current_qso_->item("QSO_DATE").c_str(),
			current_qso_->item("TIME_ON").c_str(),
			current_qso_->item("BAND").c_str(),
			current_qso_->item("MODE", true, true).c_str());
	}
	win_full_view_->copy_label(title);
	// Display the image choice buttons
	set_image_buttons();
	set_log_buttons();
	update_full_view();
	redraw();
}

// Get the card image to display and put it in the image widget.
void qso_qsl_vwr::set_image() {
	qsl_display* full = qsl_full_->display();
	qsl_display* thumb = qsl_thumb_->display();
	switch(selected_image_) {
		case QI_NONE: {
			if (current_qso_) {
				thumb->set_text(current_qso_->item("CALL").c_str(), FL_FOREGROUND_COLOR);
				full->set_text(nullptr, FL_FOREGROUND_COLOR);
			}
			else {
				thumb->set_text(nullptr, FL_FOREGROUND_COLOR);
				full->set_text(nullptr, FL_FOREGROUND_COLOR);
			}
			qsl_thumb_->box(FL_BORDER_BOX);
			break;
		}
		case QI_LABEL:
		case QI_EMAILS: {
			qsl_data* card;
			qsl_data::qsl_type type = selected_image_ == QI_LABEL ? qsl_data::LABEL : qsl_data::FILE;
			if (current_qso_) {
				card = qsl_dataset_->get_card(current_qso_->item("STATION_CALLSIGN"), type, false);
				full->set_card(card);
				full->set_qsos(&current_qso_, 1);
				thumb->set_card(card);
				thumb->set_qsos(&current_qso_, 1);
			} else {
				qso_manager* mgr = ancestor_view<qso_manager>(this);
				string def_call = mgr->get_default(qso_manager::CALLSIGN);
				card = qsl_dataset_->get_card(def_call, type, false);
				full->set_card(card);
				full->set_qsos(nullptr, 0);
				thumb->set_card(card);
				thumb->set_qsos(nullptr, 0);
			}
			qsl_thumb_->box(FL_FLAT_BOX);
			break;
		}
		default: {
			bool use_default = false;
			bool found_image = false;
			bool png_is_jpeg = false;
			char filename[256];
			string target_name = "";
			// string default_name;
			if (current_qso_ != nullptr) {
				// We want to display the received card (eQSL or scanned image)
				// Get callsign
				string call = current_qso_->item("CALL");
				// OPtional file types
				string file_types[] = { ".png", ".jpg", ".bmp" };
				const int num_types = 3;
				if (call != "") {
					// Replace all / with _ - e.g. PA/GM3ZZA/P => PA_GM3ZZA_P
					size_t pos = call.find('/');
					while (pos != call.npos) {
						call[pos] = '_';
						pos = call.find('/', pos + 1);
					}
					// Look for a possible image file and try and load into the image object
					delete raw_image_;
					raw_image_ = nullptr;
					string station = current_qso_->item("STATION_CALLSIGN");
					de_slash(station);

					// Select the image type: eQSL or scanned in card (front or back)
					switch (selected_image_) {
					case QI_EQSL:
						// Card image downloaded from eQSL
						// Find the filename saved when card was downloaded - i.e. use same algorithm
						strcpy(filename, eqsl_handler_->card_filename_l(current_qso_).c_str());
						break;
					case QI_CARD_FRONT:
						// Card image of a scanned-in paper QSL card (front - i.e. callsign side)
						// File name e.g.= <root>/<MY_CALL>\scans\<received date>\PA_GM3ZZA_P__<QSO date>.png
						sprintf(filename, "%s/%s/scans/%s/%s__%s",
							qsl_directory_.c_str(),
							station.c_str(),
							current_qso_->item("QSLRDATE").c_str(),
							call.c_str(),
							current_qso_->item("QSO_DATE").c_str());
						break;
					case QI_CARD_BACK:
						// Card image of a scanned-in paper QSL card (back - i.e. QSO details side)
						// File name e.g.= <root>\scans\<received date>\PA_GM3ZZA_P++<QSO date>.png
						sprintf(filename, "%s/%s/scans/%s/%s++%s",
							qsl_directory_.c_str(),
							station.c_str(),
							current_qso_->item("QSLRDATE").c_str(),
							call.c_str(),
							current_qso_->item("QSO_DATE").c_str());
						break;
					case QI_EMAILR:
						// Card image of a scanned-in paper QSL card (front - i.e. callsign side)
						// File name e.g.= <root>\emails\PA_GM3ZZA_P__<QSO date>.png
						sprintf(filename, "%s/%s/email/%s__%s",
							qsl_directory_.c_str(),
							station.c_str(),
							call.c_str(),
							current_qso_->item("QSO_DATE").c_str());
						break;
					}
					// Images could be any graphic format
					// Test first with existing filename then replace with default station
					for (int ia = 0; ia < 2 && !found_image; ia++) {
						string testname = filename;
						if (ia == 1) {
							size_t pos = testname.find(station);
							testname.replace(pos, station.length(), default_station_);
							use_default = true;
						}
						for (int i = 0; i < num_types && !found_image; i++) {
							full_name_ = testname + file_types[i];
							target_name = filename + file_types[i];
							// Read files depending on file type
							if (file_types[i] == ".jpg") {
								raw_image_ = new Fl_JPEG_Image(full_name_.c_str());
							}
							else if (file_types[i] == ".png") {
								raw_image_ = new Fl_PNG_Image(full_name_.c_str());
								// Some JPEGs may have been saved as .png
								if (raw_image_->fail()) {
									delete raw_image_;
									raw_image_ = new Fl_JPEG_Image(full_name_.c_str());
									if (raw_image_->fail()) {
										delete raw_image_;
										raw_image_ = nullptr;
									} else {
										png_is_jpeg = true;
									}
								}
							}
							else if (file_types[i] == ".bmp") {
								raw_image_ = new Fl_BMP_Image(full_name_.c_str());
							}
							else {
								raw_image_ = nullptr;
							}
							if (raw_image_) {
								char msg[128];
								switch(raw_image_->fail()) {
									case 0: {
										if (ia == 1) use_default = true;
										found_image = true;
										break;
									}
									case Fl_Image::ERR_NO_IMAGE:
									case Fl_Image::ERR_FILE_ACCESS: {
										// snprintf(msg, sizeof(msg), "QSL: Error %s - %s", full_name_.c_str(), strerror(errno));
										// status_->misc_status(ST_WARNING, msg);
										break;
									}
									case Fl_Image::ERR_FORMAT: {
										snprintf(msg, sizeof(msg), "QSL: Error format of %s incorrect", full_name_.c_str());
										status_->misc_status(ST_WARNING, msg);
										break;
									}
								}
								if (!found_image) {
									delete raw_image_;
									raw_image_ = nullptr;
								}
							}
						}
					}
					if (found_image) {
						update_full_view();
						// Test Whether we've used default station callsign
					}
				}
			} else {
				delete raw_image_;
				raw_image_ = nullptr;
			}
			// Got an image: draw it
			full->set_image(raw_image_);
			thumb->set_image(raw_image_);
			// If we've found an image under the default station intended for this station 
			// move (rename) it
			if (found_image && use_default) {
				char message[200];
				switch(fl_choice("Image for station call: %s found looking for %s - Replace?", 
					"no", "yes", nullptr, default_station_.c_str(),
					current_qso_->item("STATION_CALLSIGN").c_str())) {
				case 0:
					snprintf(message, sizeof(message), 
						"QSL: Ignoring %s", full_name_.c_str());
					status_->misc_status(ST_WARNING, message);
					break;
				case 1:
					snprintf(message, sizeof(message),
						"QSL: Renaming station %s as %s", full_name_.c_str(), target_name.c_str());
					status_->misc_status(ST_WARNING, message);
					fl_make_path_for_file(target_name.c_str());
					rename(full_name_.c_str(), target_name.c_str());
					full_name_ = target_name;
					break;
				}
			}
			// If we've found a .png that is actually a JPEG - rename it
			if (found_image && png_is_jpeg) {
				char message[200];
				switch(fl_choice("Image for station call: JPEG found looking for PNG - Replace?", 
					"no", "yes", nullptr)) {
				case 0:
					snprintf(message, sizeof(message), 
						"QSL: Leaving %s as a JPED without renaming", full_name_.c_str());
					status_->misc_status(ST_WARNING, message);
					break;
				case 1:
					string jpeg_name = full_name_;
					size_t pos = jpeg_name.find(".png");
					jpeg_name.replace(pos, 4, ".jpg");
					snprintf(message, sizeof(message),
						"QSL: Renaming type %s as %s", full_name_.c_str(), jpeg_name.c_str());
					status_->misc_status(ST_WARNING, message);
					fl_make_path_for_file(jpeg_name.c_str());
					rename(full_name_.c_str(), jpeg_name.c_str());
					full_name_ = jpeg_name;
					break;
				}
			}
			qsl_thumb_->box(FL_FLAT_BOX);
			break;
		}
	}
	update_full_view();
	qsl_full_->redraw();
	qsl_thumb_->redraw();
}

// Set the values of the various buttons associated with the image.
void qso_qsl_vwr::set_image_buttons() {
	switch (selected_image_) {
	case QI_EQSL:
		// eQSL displayed
		radio_eqsl_->value(true);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_emailr_->value(false);
		radio_emails_->value(false);
		radio_label_->value(false);
		break;
	case QI_CARD_FRONT:
		// Display the front of a scanned-in paper card
		radio_eqsl_->value(false);
		radio_card_front_->value(true);
		radio_card_back_->value(false);
		radio_emailr_->value(false);
		radio_label_->value(false);
		radio_emails_->value(false);
		break;
	case QI_CARD_BACK:
		// Display the back of a scanned-in paper card
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(true);
		radio_emailr_->value(false);
		radio_label_->value(false);
		radio_emails_->value(false);
		break;
	case QI_EMAILR:
		// Display the back of a scanned-in paper card
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_emailr_->value(true);
		radio_label_->value(false);
		radio_emails_->value(false);
		break;
	case QI_LABEL:
		// Display my QSL label as generated image
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_emailr_->value(false);
		radio_label_->value(true);
		radio_emails_->value(false);
		break;
	case QI_EMAILS:
		// Display my QSL label as generated image
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_emailr_->value(false);
		radio_label_->value(false);
		radio_emails_->value(true);
		break;
	default:
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_emailr_->value(false);
		radio_label_->value(false);
		radio_emails_->value(false);
		break;
	}
	// Deactivate radio buttons according to whether QSLs of that type have been received
	if (current_qso_ && current_qso_->item("EQSL_QSL_RCVD") == "Y") {
		radio_eqsl_->activate();
	}
	else {
		radio_eqsl_->deactivate();
	}
	if (current_qso_ && current_qso_->item("QSL_RCVD") == "Y") {
		if (current_qso_ && current_qso_->item("QSL_RCVD_VIA") == "E") {
			radio_emailr_->activate();
			radio_card_back_->deactivate();
			radio_card_front_->deactivate();
		} else {
			radio_card_front_->activate();
			radio_card_back_->activate();
			radio_emailr_->deactivate();
		}
	}
	else {
		radio_card_front_->deactivate();
		radio_card_back_->deactivate();
		radio_emailr_->deactivate();
	}
}


// Set the selected image to that provided
void qso_qsl_vwr::set_selected_image(image_t value) {
	selected_image_ = value;
}

// Set QSL status
void qso_qsl_vwr::set_qsl_status() {
	if (current_qso_) {
		if (current_qso_->item("EQSL_QSL_RCVD") == "Y") {
			bn_eqsl_rstatus_->value(true);
		}
		else {
			bn_eqsl_rstatus_->value(false);
		}
		if (current_qso_->item("LOTW_QSL_RCVD") == "Y") {
			bn_lotw_rstatus_->value(true);
		}
		else {
			bn_lotw_rstatus_->value(false);
		}
		if (current_qso_->item("QSL_RCVD") == "Y") {
			bn_card_rstatus_->value(true);
			string s = current_qso_->item("QSL_RCVD_VIA");
			if (s == "B") {
				bn_card_rstatus_->label("Bureau");
			}
			else if (s == "D") {
				bn_card_rstatus_->label("Direct");
			}
			else if (s == "E") {
				bn_card_rstatus_->label("e-mail");
			}
			else {
				bn_card_rstatus_->label("Card");
			}
		}
		else {
			bn_card_rstatus_->value(false);
			bn_card_rstatus_->label("Card");
		}
		if (current_qso_->item("EQSL_QSL_SENT") == "Y") {
			bn_eqsl_sstatus_->value(true);
		}
		else {
			bn_eqsl_sstatus_->value(false);
		}
		if (current_qso_->item("LOTW_QSL_SENT") == "Y") {
			bn_lotw_sstatus_->value(true);
		}
		else {
			bn_lotw_sstatus_->value(false);
		}
		if (current_qso_->item("QSL_SENT") == "Y") {
			bn_card_sstatus_->value(true);
			string s = current_qso_->item("QSL_SENT_VIA");
			if (s == "B") {
				bn_card_sstatus_->label("Bureau");
			}
			else if (s == "D") {
				bn_card_sstatus_->label("Direct");
			}
			else if (s == "E") {
				bn_card_sstatus_->label("e-mail");
			}
			else {
				bn_card_sstatus_->label("Card");
			}
		}
		else {
			bn_card_sstatus_->value(false);
			bn_card_sstatus_->label("Card");
		}
	}
	else {
		bn_eqsl_rstatus_->value(false);
		bn_lotw_rstatus_->value(false);
		bn_card_rstatus_->value(false);
		bn_card_rstatus_->label("Card");
		bn_eqsl_sstatus_->value(false);
		bn_lotw_sstatus_->value(false);
		bn_card_sstatus_->value(false);
		bn_card_sstatus_->label("Card");
	}
}

// Display log buttons 
void qso_qsl_vwr::set_log_buttons() {
	qso_data* data = ancestor_view<qso_data>(this);
	if (current_qso_ && data->qso_editing(current_qso_num_)) {
		bn_fetch_->activate();
		bn_log_bureau_->activate();
		bn_log_email_->activate();
		bn_log_direct_->activate();
		bn_card_reqd_->activate();
		bn_card_decl_->activate();
	} else {
		bn_fetch_->deactivate();
		bn_log_bureau_->deactivate();
		bn_log_email_->deactivate();
		bn_log_direct_->deactivate();
		bn_card_reqd_->deactivate();
		bn_card_decl_->deactivate();
	}
}

// Update  the full view window
void qso_qsl_vwr::update_full_view() {
	int w;
	int h;
	qsl_full_->display()->get_size(w, h);
	int sx, sy, sw, sh;
	Fl::screen_work_area(sx, sy, sw, sh);
	// If already visible
	if (win_full_view_->visible() && qso_changed_) {
		int ww = win_full_view_->w();
		int wh = win_full_view_->h();
		sw = min(sw, ww);
		sh = min(sh, wh);
	}
	// Keep the size within the screen work area
	w = min(w, sw);
	h = min(h, sh);
	// And resize the full view window
	win_full_view_->size(w, h);
	qsl_full_->size(w, h);
	win_full_view_->redraw();
}


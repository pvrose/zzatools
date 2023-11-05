#include "qso_qsl_vwr.h"
#include "qso_data.h"
#include "eqsl_handler.h"
#include "status.h"
#include "book.h"
#include "qsl_form.h"
#include "callback.h"
#include "drawing.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Box.H>



extern eqsl_handler* eqsl_handler_;
extern Fl_Preferences* settings_;
extern status* status_;
extern book* book_;

qso_qsl_vwr::qso_qsl_vwr(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, selected_image_(QI_EQSL)
	, raw_image_(nullptr)
	, scaled_image_(nullptr)
	, current_qso_(nullptr)
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
	display_settings.get("Image Type", (int&)selected_image_, QI_EQSL);
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
	const int WBN = WIMAGE / 4;


	// Box - for the display of a card image or if no card exists the callsign in large text
	bn_card_display_ = new Fl_Button(curr_x, curr_y, WIMAGE, HIMAGE, "Image");
	bn_card_display_->box(FL_FLAT_BOX);
	bn_card_display_->tooltip("The card image is displayed here!");
	bn_card_display_->callback(cb_bn_image, nullptr);

	curr_y += bn_card_display_->h();

	curr_x = x();


	Fl_Box* label1 = new Fl_Box(curr_x, curr_y, WBN, HBUTTON, "Rcvd");
	label1->box(FL_FLAT_BOX);
	label1->color(FL_BACKGROUND_COLOR);

	curr_x += WBN;
	bn_eqsl_status_ = new Fl_Light_Button(curr_x, curr_y, WBN, HBUTTON, "eQSL");
	bn_eqsl_status_->box(FL_FLAT_BOX);
	bn_eqsl_status_->value(true);

	curr_x += WBN;
	bn_lotw_status_ = new Fl_Light_Button(curr_x, curr_y, WBN, HBUTTON, "LotW");
	bn_lotw_status_->box(FL_FLAT_BOX);
	bn_lotw_status_->value(true);

	curr_x += WBN;
	bn_card_status_ = new Fl_Light_Button(curr_x, curr_y, WBN, HBUTTON, "Card");
	bn_card_status_->box(FL_FLAT_BOX);
	bn_card_status_->value(true);

	curr_y += HBUTTON;
	curr_x = x();

	grp_card_type_ = new Fl_Group(curr_x, curr_y, WBN * 4, HBUTTON);
	grp_card_type_->box(FL_FLAT_BOX);

	// Radio - Display eQSL.cc card image
	radio_eqsl_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBN, HBUTTON, "eQSL");
	radio_eqsl_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_eqsl_->selection_color(FL_BLACK);
	radio_eqsl_->callback(cb_rad_card, (void*)QI_EQSL);
	radio_eqsl_->when(FL_WHEN_RELEASE);
	radio_eqsl_->tooltip("Select image downloaded from eQSL");
	curr_x += WBN;
	// Radio - display scanned image of front of paper card
	radio_card_front_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBN, HBUTTON, "Card(F)");
	radio_card_front_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_card_front_->selection_color(FL_BLACK);
	radio_card_front_->callback(cb_rad_card, (void*)QI_CARD_FRONT);
	radio_card_front_->when(FL_WHEN_RELEASE);
	radio_card_front_->tooltip("Select image scanned of paper card front");
	curr_x += WBN;
	// Radio - display scanned image of reverse of paper card
	radio_card_back_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBN, HBUTTON, "Card(B)");
	radio_card_back_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_card_back_->selection_color(FL_BLACK);
	radio_card_back_->callback(cb_rad_card, (void*)QI_CARD_BACK);
	radio_card_back_->when(FL_WHEN_RELEASE);
	radio_card_back_->tooltip("Select image scanned of paper card back");
	curr_x += WBN;
	// Radio - display received e-mail image
	radio_email_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBN, HBUTTON, "e-mail");
	radio_email_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	radio_email_->selection_color(FL_BLACK);
	radio_email_->callback(cb_rad_card, (void*)QI_EMAIL);
	radio_email_->when(FL_WHEN_RELEASE);
	radio_email_->tooltip("Select image received by e-mail");

	grp_card_type_->end();

	curr_x += WBN;

	curr_y += HBUTTON;
	curr_x = x();

	Fl_Box* label2 = new Fl_Box(curr_x, curr_y, WBN, HBUTTON, "Log");
	label2->box(FL_FLAT_BOX);
	label2->color(FL_BACKGROUND_COLOR);

	curr_x += WBN;
	// Button - Mark paper card received
	bn_log_bureau_ = new Fl_Button(curr_x, curr_y, WBN, HBUTTON, "Bureau");
	bn_log_bureau_->tooltip("Log a paper card received from the bureau on today's date");
	bn_log_bureau_->callback(cb_bn_log_card, (void*)"B");
	bn_log_bureau_->when(FL_WHEN_RELEASE);
	curr_x += WBN;
	// Button - Mark e-mail received
	bn_log_bureau_ = new Fl_Button(curr_x, curr_y, WBN, HBUTTON, "e-mail");
	bn_log_bureau_->tooltip("Log an e-mail card received on today's date");
	bn_log_bureau_->callback(cb_bn_log_card, (void*)"E");
	bn_log_bureau_->when(FL_WHEN_RELEASE);
	curr_x += WBN;
	// Button - Mark e-mail received
	bn_log_direct_ = new Fl_Button(curr_x, curr_y, WBN, HBUTTON, "Direct");
	bn_log_direct_->tooltip("Log a paper card received direct on today's date");
	bn_log_direct_->callback(cb_bn_log_card, (void*)"D");
	bn_log_direct_->when(FL_WHEN_RELEASE);

	curr_x = x();
	curr_y += HBUTTON;
	// Button - Fetch the card image from eQSL.cc
	bn_fetch_ = new Fl_Button(curr_x, curr_y, WBN, HBUTTON, "Fetch");
	bn_fetch_->tooltip("Request a fresh download of the eQSL image");
	bn_fetch_->callback(cb_bn_fetch, nullptr);
	bn_fetch_->when(FL_WHEN_RELEASE);
	curr_x += WBN;
	// Button - Set QSL_SENT=R in current QSO
	bn_card_reqd_ = new Fl_Button(curr_x, curr_y, WBN, HBUTTON, "Requested");
	bn_card_reqd_->tooltip("Marl QSL as wanting a card QSL");
	bn_card_reqd_->callback(cb_bn_card_reqd);
	bn_card_reqd_->when(FL_WHEN_RELEASE);
	curr_x += WBN;


	end();
	show();

	// Now create the full view window
	win_full_view_ = new Fl_Window(WCARD, HCARD);
	bn_full_view_ = new Fl_Button(0, 0, WCARD, HCARD);
	bn_full_view_->box(FL_FLAT_BOX);
	// bn_full_view_->labelcolor(FL_RED);
	// bn_full_view_->labelsize(FL_NORMAL_SIZE * 3);
	// bn_full_view_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	bn_no_image_ = new Fl_Button(0, 0, WCARD, HCARD);
	bn_no_image_->box(FL_FLAT_BOX);
	bn_no_image_->label("No image available!");
	bn_no_image_->labelsize(FL_NORMAL_SIZE * 3);
	bn_no_image_->labelcolor(FL_RED);
	win_full_view_->resizable(bn_full_view_);
	win_full_view_->end();
	win_full_view_->hide();
}


// Call-backs
// called when any of the image selection radio buttons is released
// v is enum image_t.
void qso_qsl_vwr::cb_rad_card(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	// Get the selected image and display it.
	that->set_selected_image((image_t)(intptr_t)v);
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
	if (!that->current_qso_->item("QSL_SENT").length()) {
		that->current_qso_->item("QSL_SENT", string("Q"));
	}
	// now pretend the Card Front radio button has been pressed
	if (*source == 'E') {
		cb_rad_card(that->radio_card_front_, (void*)QI_EMAIL);
	} else {
		cb_rad_card(that->radio_card_front_, (void*)QI_CARD_FRONT);
	}
	that->set_image_buttons();
	book_->selection(that->current_qso_num_, HT_MINOR_CHANGE);
}

// Show/hide full view window
void qso_qsl_vwr::cb_bn_image(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	if (that->win_full_view_->visible()) {
		that->win_full_view_->hide();
	}
	else {
		that->win_full_view_->show();
	}
}

// Set card requested "QSL_SENT=R"
void qso_qsl_vwr::cb_bn_card_reqd(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	that->current_qso_->item("QSL_SENT", string("R"));
	qso_data* data = ancestor_view<qso_data>(that);
	data->enable_widgets();
}

void qso_qsl_vwr::set_qso(record* qso, qso_num_t number) {
	current_qso_ = qso;
	current_qso_num_ = number;
	enable_widgets();
}

// Redraw the form after stuff has changed
void qso_qsl_vwr::enable_widgets() {
	if (current_qso_ != nullptr) {
		set_image();
		set_qsl_status();
		char title[128];
		snprintf(title, 128, "QSL: %s %s %s %s %s",
			current_qso_->item("CALL").c_str(),
			current_qso_->item("QSO_DATE").c_str(),
			current_qso_->item("TIME_ON").c_str(),
			current_qso_->item("BAND").c_str(),
			current_qso_->item("MODE", true, true).c_str());
		win_full_view_->copy_label(title);
	}
	// Display the image choice buttons
	set_image_buttons();
	update_full_view();
	redraw();
}

// Get the card image to display and put it in the image widget.
void qso_qsl_vwr::set_image() {
	if (current_qso_ != nullptr) {
		// We want to display the received card (eQSL or scanned image)
		char filename[256];
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
			bool found_image = false;
			delete raw_image_;
			raw_image_ = nullptr;
			delete scaled_image_;
			scaled_image_ = nullptr;
			// Select the image type: eQSL or scanned in card (front or back)
			switch (selected_image_) {
			case QI_EQSL:
				// Card image downloaded from eQSL
				// Find the filename saved when card was downloaded - i.e. use same algorithm
				strcpy(filename, eqsl_handler_->card_filename_l(current_qso_).c_str());
				// Cards are downloaded from eQSL in PNG format
				raw_image_ = new Fl_PNG_Image(filename);
				if (raw_image_->fail()) {
					delete raw_image_;
					raw_image_ = nullptr;
				}
				else {
					full_name_ = string(filename);
					found_image = true;
				}
				break;
			case QI_CARD_FRONT:
			case QI_CARD_BACK:
			case QI_EMAIL:
				for (int ix = 0; ix < 2 && !found_image; ix++) {
					if (ix == 0) call = to_upper(call);
					else call = to_lower(call);
					switch (selected_image_) {
					case QI_CARD_FRONT:
						// Card image of a scanned-in paper QSL card (front - i.e. callsign side)
						// File name e.g.= <root>\scans\<received date>\PA_GM3ZZA_P__<QSO date>.png
						sprintf(filename, "%s/scans/%s/%s__%s",
							qsl_directory_.c_str(),
							current_qso_->item("QSLRDATE").c_str(),
							call.c_str(),
							current_qso_->item("QSO_DATE").c_str());
						break;
					case QI_CARD_BACK:
						// Card image of a scanned-in paper QSL card (back - i.e. QSO details side)
						// File name e.g.= <root>\scans\<received date>\PA_GM3ZZA_P++<QSO date>.png
						sprintf(filename, "%s/scans/%s/%s++%s",
							qsl_directory_.c_str(),
							current_qso_->item("QSLRDATE").c_str(),
							call.c_str(),
							current_qso_->item("QSO_DATE").c_str());
						break;
					case QI_EMAIL:
						// Card image of a scanned-in paper QSL card (front - i.e. callsign side)
						// File name e.g.= <root>\emails\PA_GM3ZZA_P__<QSO date>.png
						sprintf(filename, "%s/email/%s__%s",
							qsl_directory_.c_str(),
							call.c_str(),
							current_qso_->item("QSO_DATE").c_str());
						break;
					}
					// Scanned-in paper card - could be any graphic format
					for (int i = 0; i < num_types && !found_image; i++) {
						full_name_ = string(filename) + file_types[i];
						printf("DEBUG QSO_QSL_VWR: Image file %s\n", full_name_.c_str());
						// Read files depending on file type
						if (file_types[i] == ".jpg") {
							raw_image_ = new Fl_JPEG_Image(full_name_.c_str());
						}
						else if (file_types[i] == ".png") {
							raw_image_ = new Fl_PNG_Image(full_name_.c_str());
						}
						else if (file_types[i] == ".bmp") {
							raw_image_ = new Fl_BMP_Image(full_name_.c_str());
						}
						else {
							raw_image_ = nullptr;
						}
						if (raw_image_ && raw_image_->fail()) {
							delete raw_image_;
							raw_image_ = nullptr;
						}
						if (raw_image_) {
							found_image = true;
						}
					}
				}
			}
			if (found_image) {
				// Resize the image to fit the control
				// Resize keeping original height/width ratio
				float scale_w = (float)raw_image_->w() / (float)bn_card_display_->w();
				float scale_h = (float)raw_image_->h() / (float)bn_card_display_->h();
				if (scale_w < scale_h) {
					scaled_image_ = raw_image_->copy((int)(raw_image_->w() / scale_h), bn_card_display_->h());
				}
				else {
					scaled_image_ = raw_image_->copy(bn_card_display_->w(), (int)(raw_image_->h() / scale_w));
				}
				update_full_view();
			}
		}
	}
	// Got an image: draw it
	draw_image();
}

// Set the values of the various buttons associated with the image.
void qso_qsl_vwr::set_image_buttons() {
	switch (selected_image_) {
	case QI_EQSL:
		// eQSL displayed
		radio_eqsl_->value(true);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_email_->value(false);
		break;
	case QI_CARD_FRONT:
		// Display the front of a scanned-in paper card
		radio_eqsl_->value(false);
		radio_card_front_->value(true);
		radio_card_back_->value(false);
		radio_email_->value(false);
		break;
	case QI_CARD_BACK:
		// Display the back of a scanned-in paper card
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(true);
		radio_email_->value(false);
		break;
	case QI_EMAIL:
		// Display the back of a scanned-in paper card
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_email_->value(true);
		break;
	default:
		// Shouldn't get here
		radio_eqsl_->value(false);
		radio_card_front_->value(false);
		radio_card_back_->value(false);
		radio_email_->value(false);
		break;
	}
	if (current_qso_ && current_qso_->item("EQSL_QSL_RCVD") == "Y") {
		radio_eqsl_->activate();
	}
	else {
		radio_eqsl_->deactivate();
	}
	if (current_qso_ && current_qso_->item("QSL_RCVD") == "Y") {
		if (current_qso_ && current_qso_->item("QSL_RCVD_VIA") == "E") {
			radio_email_->activate();
			radio_card_back_->deactivate();
			radio_card_front_->deactivate();
		} else {
			radio_card_front_->activate();
			radio_card_back_->activate();
			radio_email_->deactivate();
		}
	}
	else {
		radio_card_front_->deactivate();
		radio_card_back_->deactivate();
		radio_email_->deactivate();
	}
}

// Draw the selected QSL card image (or callsign if no image)
void qso_qsl_vwr::draw_image() {
	// Remove existing images
	bn_card_display_->label(nullptr);
	bn_card_display_->image(nullptr);
	bn_card_display_->deimage(nullptr);
	switch (selected_image_) {
	case QI_EQSL:
	case QI_CARD_BACK:
	case QI_CARD_FRONT:
	case QI_EMAIL:
		// We want to display the saved QSL image
		if (scaled_image_) {
			// we have an image
			// Set the resized image as the selected and unselected image for the control
			bn_card_display_->image(scaled_image_);
			bn_card_display_->deimage(scaled_image_);
		}
		else {
			// Display the error message in red.
			// Display a label instead in large letters - 36 pt.
			bn_card_display_->copy_label(current_qso_->item("CALL").c_str());
			bn_card_display_->labelsize(36);
			bn_card_display_->labelcolor(FL_BLACK);
			//bn_card_display_->color(FL_WHITE);
		}
		break;
	}

	bn_card_display_->redraw();
}

// Set the selected image to that provided
void qso_qsl_vwr::set_selected_image(image_t value) {
	selected_image_ = value;
}

// Set QSL status
void qso_qsl_vwr::set_qsl_status() {
	if (current_qso_) {
		if (current_qso_->item("EQSL_QSL_RCVD") == "Y") {
			bn_eqsl_status_->selection_color(FL_GREEN);
		}
		else {
			bn_eqsl_status_->selection_color(FL_RED);
		}
		if (current_qso_->item("LOTW_QSL_RCVD") == "Y") {
			bn_lotw_status_->selection_color(FL_GREEN);
		}
		else {
			bn_lotw_status_->selection_color(FL_RED);
		}
		if (current_qso_->item("QSL_RCVD") == "Y") {
			bn_card_status_->selection_color(FL_GREEN);
			string s = current_qso_->item("QSL_RCVD_VIA");
			if (s == "B") {
				bn_card_status_->label("Bureau");
			} else if (s == "D") {
				bn_card_status_->label("Direct");
			} else if (s == "E") {
				bn_card_status_->label("e-mail");
			} else {
				bn_card_status_->label("Card");
			}
		}
		else {
			bn_card_status_->selection_color(FL_RED);
			bn_card_status_->label("Card");
		}
	}
}

void qso_qsl_vwr::update_full_view() {
	bn_full_view_->image(raw_image_);
	bn_full_view_->deimage(raw_image_);
	if (raw_image_) {
		bn_full_view_->show();
		bn_no_image_->hide();
		win_full_view_->size(raw_image_->w(), raw_image_->h());
	} else {
		bn_full_view_->hide();
		bn_no_image_->show();
	}
	win_full_view_->redraw();
}
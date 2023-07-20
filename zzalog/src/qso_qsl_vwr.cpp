#include "qso_qsl_vwr.h"
#include "eqsl_handler.h"
#include "status.h"
#include "book.h"
#include "qsl_form.h"
#include "callback.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Native_File_Chooser.H>



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
		chooser->title("Select QSL card directory");
		if (chooser->show() == 0) {
			qsl_directory_ = chooser->filename();
		}
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
	int w_image = w();
	int h_image = w() * HCARD / WCARD;

	// Box - for the display of a card image or if no card exists the callsign in large text
	card_display_ = new Fl_Button(curr_x, curr_y, w_image, h_image, "Image");
	card_display_->box(FL_FLAT_BOX);
	card_display_->tooltip("The card image is displayed here!");
	card_display_->callback(cb_bn_image, nullptr);

	curr_y += card_display_->h();

	curr_x = x() + ((w() - (3 * WBUTTON)) / 2);
	eqsl_status_box_ = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON, "eQSL");
	eqsl_status_box_->box(FL_FLAT_BOX);

	curr_x += eqsl_status_box_->w();
	lotw_status_box_ = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON, "LotW");
	lotw_status_box_->box(FL_FLAT_BOX);

	curr_x += lotw_status_box_->w();
	card_status_box_ = new Fl_Box(curr_x, curr_y, WBUTTON, HBUTTON, "Card");
	card_status_box_->box(FL_FLAT_BOX);

	curr_x += card_status_box_->w();

	curr_y += HBUTTON;
	curr_x = x() + GAP;

	card_type_grp_ = new Fl_Group(curr_x, curr_y, WBUTTON * 3, HBUTTON + HTEXT, "QSL type selection");
	card_type_grp_->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	card_type_grp_->box(FL_FLAT_BOX);
	curr_y += HTEXT;
	// Radio - Display eQSL.cc card image
	eqsl_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "eQSL");
	eqsl_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	eqsl_radio_->selection_color(FL_BLACK);
	eqsl_radio_->callback(cb_rad_card, (void*)QI_EQSL);
	eqsl_radio_->when(FL_WHEN_RELEASE);
	eqsl_radio_->tooltip("Select image downloaded from eQSL");
	curr_x += WBUTTON;
	// Radio - display scanned image of front of paper card
	card_front_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Card(F)");
	card_front_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	card_front_radio_->selection_color(FL_BLACK);
	card_front_radio_->callback(cb_rad_card, (void*)QI_CARD_FRONT);
	card_front_radio_->when(FL_WHEN_RELEASE);
	card_front_radio_->tooltip("Select image scanned of paper card front");
	curr_x += WBUTTON;
	// Radio - display scanned image of reverse of paper card
	card_back_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Card(B)");
	card_back_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	card_back_radio_->selection_color(FL_BLACK);
	card_back_radio_->callback(cb_rad_card, (void*)QI_CARD_BACK);
	card_back_radio_->when(FL_WHEN_RELEASE);
	card_back_radio_->tooltip("Select image scanned of paper card back");

	card_type_grp_->end();

	curr_x += WBUTTON;

	curr_y += HBUTTON;
	curr_x = x() + GAP;

	// Button - Fetch the card image from eQSL.cc
	fetch_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Fetch");
	fetch_bn_->tooltip("Request a fresh download of the eQSL image");
	fetch_bn_->callback(cb_bn_fetch, nullptr);
	fetch_bn_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;
	// Button - Mark paper card received
	log_card_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Log card");
	log_card_bn_->tooltip("Log a paper card received on today's date");
	log_card_bn_->callback(cb_bn_log_card, nullptr);
	log_card_bn_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;

	end();
	show();

	// Now create the full view window
	win_full_view_ = new Fl_Window(WCARD, HCARD);
	bn_full_view_ = new Fl_Button(0, 0, WCARD, HCARD);
	bn_full_view_->box(FL_FLAT_BOX);
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
// v is unused
void qso_qsl_vwr::cb_bn_log_card(Fl_Widget* w, void* v) {
	qso_qsl_vwr* that = ancestor_view<qso_qsl_vwr>(w);
	string today = now(false, "%Y%m%d");
	that->current_qso_->item("QSLRDATE", today);
	that->current_qso_->item("QSL_RCVD", string("Y"));
	if (!that->current_qso_->item("QSL_SENT").length()) {
		that->current_qso_->item("QSL_SENT", string("Q"));
	}
	// now pretend the Card Front radio button has been pressed
	cb_rad_card(that->card_front_radio_, (void*)QI_CARD_FRONT);
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
			// Select the image type: eQSL or scanned in card (front or back)
			switch (selected_image_) {
			case QI_EQSL:
				// Card image downloaded from eQSL
				// Find the filename saved when card was downloaded - i.e. use same algorithm
				strcpy(filename, eqsl_handler_->card_filename_l(current_qso_).c_str());
				break;
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
			}
			// Look for a possible image file and try and load into the image object
			bool found_image = false;
			delete raw_image_;
			raw_image_ = nullptr;
			delete scaled_image_;
			scaled_image_ = nullptr;
			// For each of coded image file types
			if (selected_image_ == QI_EQSL) {
				// Cards are downloaded from eQSL in PNG format
				raw_image_ = new Fl_PNG_Image(filename);
				if (raw_image_->fail()) {
					delete raw_image_;
					raw_image_ = nullptr;
				}
			}
			else {
				// Scanned-in paper card - could be any graphic format
				for (int i = 0; i < num_types && !found_image; i++) {
					full_name_ = string(filename) + file_types[i];
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
				}
			}

			if (raw_image_) {
				found_image = true;
				// Resize the image to fit the control
				// Resize keeping original height/width ratio
				float scale_w = (float)raw_image_->w() / (float)card_display_->w();
				float scale_h = (float)raw_image_->h() / (float)card_display_->h();
				if (scale_w < scale_h) {
					scaled_image_ = raw_image_->copy((int)(raw_image_->w() / scale_h), card_display_->h());
				}
				else {
					scaled_image_ = raw_image_->copy(card_display_->w(), (int)(raw_image_->h() / scale_w));
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
		eqsl_radio_->value(true);
		card_front_radio_->value(false);
		card_back_radio_->value(false);
		break;
	case QI_CARD_FRONT:
		// Display the front of a scanned-in paper card
		eqsl_radio_->value(false);
		card_front_radio_->value(true);
		card_back_radio_->value(false);
		break;
	case QI_CARD_BACK:
		// Display the back of a scanned-in paper card
		eqsl_radio_->value(false);
		card_front_radio_->value(false);
		card_back_radio_->value(true);
		break;
	default:
		// Shouldn't get here
		eqsl_radio_->value(false);
		card_front_radio_->value(false);
		card_back_radio_->value(false);
		break;
	}
	if (current_qso_ && current_qso_->item("EQSL_QSL_RCVD") == "Y") {
		eqsl_radio_->activate();
	}
	else {
		eqsl_radio_->deactivate();
	}
	if (current_qso_ && current_qso_->item("QSL_RCVD") == "Y") {
		card_front_radio_->activate();
		card_back_radio_->activate();
	}
	else {
		card_front_radio_->deactivate();
		card_back_radio_->deactivate();
	}
}

// Draw the selected QSL card image (or callsign if no image)
void qso_qsl_vwr::draw_image() {
	// Remove existing images
	card_display_->label(nullptr);
	card_display_->image(nullptr);
	card_display_->deimage(nullptr);
	switch (selected_image_) {
	case QI_EQSL:
	case QI_CARD_BACK:
	case QI_CARD_FRONT:
		// We want to display the saved QSL image
		if (scaled_image_) {
			// we have an image
			// Set the resized image as the selected and unselected image for the control
			card_display_->image(scaled_image_);
			card_display_->deimage(scaled_image_);
		}
		else {
			// Display the error message in red.
			// Display a label instead in large letters - 36 pt.
			card_display_->copy_label(current_qso_->item("CALL").c_str());
			card_display_->labelsize(36);
			card_display_->labelcolor(FL_BLACK);
			//card_display_->color(FL_WHITE);
		}
		break;
	}

	card_display_->redraw();
}

// Set the selected image to that provided
void qso_qsl_vwr::set_selected_image(image_t value) {
	selected_image_ = value;
}

// Set QSL status
void qso_qsl_vwr::set_qsl_status() {
	if (current_qso_) {
		if (current_qso_->item("EQSL_QSL_RCVD") == "Y") {
			eqsl_status_box_->color(FL_GREEN);
		}
		else {
			eqsl_status_box_->color(fl_lighter(FL_RED));
		}
		if (current_qso_->item("LOTW_QSL_RCVD") == "Y") {
			lotw_status_box_->color(FL_GREEN);
		}
		else {
			lotw_status_box_->color(fl_lighter(FL_RED));
		}
		if (current_qso_->item("QSL_RCVD") == "Y") {
			card_status_box_->color(FL_GREEN);
		}
		else {
			card_status_box_->color(fl_lighter(FL_RED));
		}
	}
}

void qso_qsl_vwr::update_full_view() {
	bn_full_view_->image(raw_image_);
	bn_full_view_->deimage(raw_image_);
	if (raw_image_) {
		win_full_view_->size(raw_image_->w(), raw_image_->h());
	}
	win_full_view_->redraw();
}
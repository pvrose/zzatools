#include "qsl_viewer.h"
#include "eqsl_handler.h"
#include "status.h"
#include "book.h"
#include "qsl_form.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>



extern eqsl_handler* eqsl_handler_;
extern Fl_Preferences* settings_;
extern status* status_;
extern book* book_;

qsl_viewer::qsl_viewer(int W, int H, const char* L) :
	Fl_Window(W, H, L)
	, selected_image_(QI_EQSL)
	, image_(nullptr)
	, scaling_image_(false)
	, current_qso_(nullptr)
{
	// Window origin
	int curr_x = 0;
	int curr_y = 0;

	// Leave a gap
	curr_x += GAP;
	curr_y += GAP;
	// Initial size of card
	const int WCARD = 540;
	const int HCARD = 340;

	// Box - for the display of a card image or if no card exists the callsign in large text
	card_display_ = new Fl_Group(curr_x, curr_y, WCARD, HCARD);
	card_display_->box(FL_UP_BOX);
	card_display_->tooltip("The card image is displayed here!");
	card_display_->align(FL_ALIGN_CENTER);
	card_display_->end();
	curr_y += HCARD;
	// Output - Filename of the image
	card_filename_out_ = new Fl_Box(curr_x, curr_y, WCARD, HTEXT);
	card_filename_out_->align(FL_ALIGN_CENTER);
	card_filename_out_->box(FL_DOWN_BOX);
	card_filename_out_->color(FL_WHITE);
	card_filename_out_->label("Filename");
	card_filename_out_->tooltip("The filename of the displayed card image ");
	curr_y += HTEXT + GAP;
	curr_x += WCARD + GAP;
	int save_x = curr_x;
	int max_w = curr_x - x();

	curr_x = x() + GAP;
	eqsl_status_box_ = new Fl_Box(curr_x, curr_y, WSMEDIT, HBUTTON);
	eqsl_status_box_->box(FL_FLAT_BOX);

	curr_x += WSMEDIT + GAP;
	lotw_status_box_ = new Fl_Box(curr_x, curr_y, WSMEDIT, HBUTTON);
	lotw_status_box_->box(FL_FLAT_BOX);

	curr_x += WSMEDIT + GAP;
	max_w = max(max_w, curr_x - x());

	curr_y += HBUTTON + GAP;
	int max_h = curr_y - y();

	curr_x = save_x;
	curr_y = y() + HTEXT;

	card_type_grp_ = new Fl_Group(curr_x, curr_y, WBUTTON + (2 * GAP), 8 * HBUTTON + (2 * GAP), "QSL type selection");
	card_type_grp_->align(FL_ALIGN_TOP_LEFT);
	card_type_grp_->box(FL_DOWN_BOX);
	curr_x += GAP;
	curr_y += GAP;
	// Light - Keep the current selection for displaying the card image
	keep_bn_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Keep?");
	keep_bn_->selection_color(FL_RED);
	keep_bn_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	keep_bn_->value(false);
	keep_bn_->tooltip("Remembers selection after record update");
	curr_y += HBUTTON;
	// Radio - Display eQSL.cc card image
	eqsl_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "eQSL");
	eqsl_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	eqsl_radio_->selection_color(FL_BLACK);
	eqsl_radio_->callback(cb_rad_card, (void*)QI_EQSL);
	eqsl_radio_->when(FL_WHEN_RELEASE);
	eqsl_radio_->tooltip("Select image downloaded from eQSL");
	curr_y += HBUTTON;
	// Radio - display scanned image of front of paper card
	card_front_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Card(F)");
	card_front_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	card_front_radio_->selection_color(FL_BLACK);
	card_front_radio_->callback(cb_rad_card, (void*)QI_CARD_FRONT);
	card_front_radio_->when(FL_WHEN_RELEASE);
	card_front_radio_->tooltip("Select image scanned of paper card front");
	curr_y += HBUTTON;
	// Radio - display scanned image of reverse of paper card
	card_back_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Card(B)");
	card_back_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	card_back_radio_->selection_color(FL_BLACK);
	card_back_radio_->callback(cb_rad_card, (void*)QI_CARD_BACK);
	card_back_radio_->when(FL_WHEN_RELEASE);
	card_back_radio_->tooltip("Select image scanned of paper card back");
	curr_y += HBUTTON;
	// Radio - generate QSL card and display
	gen_card_radio_ = new Fl_Radio_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Generate");
	gen_card_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	gen_card_radio_->selection_color(FL_BLACK);
	gen_card_radio_->callback(cb_rad_card, (void*)QI_GEN_CARD);
	gen_card_radio_->when(FL_WHEN_RELEASE);
	gen_card_radio_->tooltip("Select generated label for card");
	curr_y += HBUTTON;

	// Button - Fetch the card image from eQSL.cc
	fetch_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Fetch");
	fetch_bn_->tooltip("Request a fresh download of the eQSL image");
	fetch_bn_->callback(cb_bn_fetch, nullptr);
	fetch_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - Mark paper card received
	log_card_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Log card");
	log_card_bn_->tooltip("Log a paper card received on today's date");
	log_card_bn_->callback(cb_bn_log_card, nullptr);
	log_card_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - Scale or stretch
	scale_bn_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Scale image");
	scale_bn_->tooltip("Scale - keep in proportion");
	scale_bn_->callback(cb_bn_scale, nullptr);
	scale_bn_->when(FL_WHEN_RELEASE);
	card_type_grp_->end();

	curr_x += card_type_grp_->w() + GAP;
	curr_y += HBUTTON + GAP;
	
	max_w = max(max_w, curr_x - x());
	max_h = max(max_h, curr_y - y());

	end();
	resizable(nullptr);
	size(max_w, max_h);
	show();
}

qsl_viewer::~qsl_viewer() {
}

// Call-backs
// called when any of the image selection radio buttons is released
// v is enum image_t.
void qsl_viewer::cb_rad_card(Fl_Widget* w, void* v) {
	qsl_viewer* that = ancestor_view<qsl_viewer>(w);
	// Get the selected image and display it.
	that->set_selected_image((image_t)(long)v);
	that->set_image();
	// Save setting
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.set("Image Type", (int)that->selected_image_);
	//	main_window_->flush();
}

// Fetch the eQSL card for the supplied record
// v is record_num_t* containing the number of the record to fetch the eQSL card for
void qsl_viewer::cb_bn_fetch(Fl_Widget* w, void* v) {
	qsl_viewer* that = ancestor_view<qsl_viewer>(w);
	// Put the card request onto the eQSL request queue - so that requests are made
	eqsl_handler_->enqueue_request(that->current_qso_num_, true);
	eqsl_handler_->enable_fetch(eqsl_handler::EQ_START);
}

// Set whether we stretch or scale the image
// v is unused
void qsl_viewer::cb_bn_scale(Fl_Widget* w, void* v) {
	qsl_viewer* that = ancestor_view<qsl_viewer>(w);
	cb_value<Fl_Light_Button, bool>(w, &that->scaling_image_);
	that->set_image();
	// Save setting
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.set("Image Scale", (int)that->scaling_image_);

}

// Set the QSL_RCVD and QSLRDATE values in current record
// Set QSL_SENT to QUEUED if not already set
// v is unused
void qsl_viewer::cb_bn_log_card(Fl_Widget* w, void* v) {
	qsl_viewer* that = ancestor_view<qsl_viewer>(w);
	string today = now(false, "%Y%m%d");
	that->current_qso_->item("QSLRDATE", today);
	that->current_qso_->item("QSL_RCVD", string("Y"));
	if (!that->current_qso_->item("QSL_SENT").length()) {
		that->current_qso_->item("QSL_SENT", string("Q"));
	}
	// now pretend the Card Front radio button has been pressed
	cb_rad_card(that->card_front_radio_, (void*)QI_CARD_FRONT);
	that->set_image_buttons();
}

void qsl_viewer::set_qso(record* qso, record_num_t number) {
	current_qso_ = qso;
	current_qso_num_ = number;
	update_widgets();
}

// Redraw the form after stuff has changed
void qsl_viewer::update_widgets() {
	if (current_qso_ != nullptr) {
		// A record to display - copy any saved card image to the image widget
		if (keep_bn_->value() == false) {
			// If we are not wanting to redisplay the existing QSL card try and display the eQSL card
			selected_image_ = QI_EQSL;
		}
		set_image();
		set_qsl_status();
	}
	// Display the image choice buttons
	set_image_buttons();

	redraw();
}

// Get the card image to display and put it in the image widget.
void qsl_viewer::set_image() {
	if (current_qso_ != nullptr) {
		if (selected_image_ != QI_GEN_CARD) {
			// We want to display the received card (eQSL or scanned image)
			char filename[256];
			string directory;
			char* temp;
			Fl_Preferences datapath(settings_, "Datapath");
			if (!datapath.get("QSLs", temp, "")) {
				//Fl_File_Chooser* chooser = new Fl_File_Chooser("", nullptr, Fl_File_Chooser::DIRECTORY,
				//	"Select QSL card directory");
				Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
				chooser->title("Select QSL card directory");
				if (chooser->show() == 0) {
					directory = chooser->filename();
				}
				delete chooser;
				datapath.set("QSLs", directory.c_str());
			}
			else {
				directory = temp;
			}
			free(temp);
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
					sprintf(filename, "%s\\Scans\\%s\\%s__%s",
						directory.c_str(),
						current_qso_->item("QSLRDATE").c_str(),
						call.c_str(),
						current_qso_->item("QSO_DATE").c_str());
					break;
				case QI_CARD_BACK:
					// Card image of a scanned-in paper QSL card (back - i.e. QSO details side)
					// File name e.g.= <root>\scans\<received date>\PA_GM3ZZA_P++<QSO date>.png
					sprintf(filename, "%s\\Scans\\%s\\%s++%s",
						directory.c_str(),
						current_qso_->item("QSLRDATE").c_str(),
						call.c_str(),
						current_qso_->item("QSO_DATE").c_str());
					break;
				}
				// Look for a possible image file and try and load into the image object
				bool found_image = false;
				string full_name;
				Fl_Image* raw_image = nullptr;
				delete image_;
				image_ = nullptr;
				// For each of coded image file types
				if (selected_image_ == QI_EQSL) {
					// Cards are downloaded from eQSL in PNG format
					raw_image = new Fl_PNG_Image(filename);
					if (raw_image->fail()) {
						// File didn't load OK 
						if (current_qso_->item("APP_ZZA_ERROR").length() > 0) {
							// Set error message ton what we already know
							full_name = current_qso_->item("APP_ZZA_ERROR");
						}
						else {
							switch (raw_image->fail()) {
							case Fl_Image::ERR_NO_IMAGE:
								// No image file
								full_name = "*** ERROR FILE NOT FOUND ***";
								break;
							case Fl_Image::ERR_FILE_ACCESS:
								// Got a problem accessing file
								full_name = "*** ERROR ACCESSING FILE: " + string(strerror(errno)) + " ***";
								break;
							case Fl_Image::ERR_FORMAT:
								// Not a recognised format
								full_name = "*** ERROR FILE FORMAT ERROR ***";
								break;
							}
						}
					}
					else {
						image_ = raw_image->copy(card_display_->w(), card_display_->h());
					}
					delete raw_image;
				}
				else {
					// Scanned-in paper card - could be any graphic format
					for (int i = 0; i < num_types && !found_image; i++) {
						full_name = string(filename) + file_types[i];
						// Read files depending on file type
						if (file_types[i] == ".jpg") {
							raw_image = new Fl_JPEG_Image(full_name.c_str());
						}
						else if (file_types[i] == ".png") {
							raw_image = new Fl_PNG_Image(full_name.c_str());
						}
						else if (file_types[i] == ".bmp") {
							raw_image = new Fl_BMP_Image(full_name.c_str());
						}
						else {
							raw_image = nullptr;
						}
						if (raw_image && raw_image->fail()) {
							// File didn't load OK 
							if (current_qso_->item("APP_ZZA_ERROR").length() > 0) {
								// Set error message ton what we already know
								full_name = current_qso_->item("APP_ZZA_ERROR");
							}
							else {
								switch (raw_image->fail()) {
								case Fl_Image::ERR_NO_IMAGE:
									// No image file
									full_name = "*** ERROR FILE NOT FOUND ***";
									break;
								case Fl_Image::ERR_FILE_ACCESS:
									// Got a problem accessing file
									full_name = "*** ERROR ACCESSING FILE: " + string(strerror(errno)) + " ***";
									break;
								case Fl_Image::ERR_FORMAT:
									// Not a recognised format
									full_name = "*** ERROR FILE FORMAT ERROR ***";
									break;
								}
							}
						}
						else if (raw_image) {
							// Clear error message
							current_qso_->item("APP_ZZA_ERROR", string(""));
							found_image = true;
							// Resize the image to fit the control
							if (scaling_image_) {
								// Resize keeping original height/width ratio
								float scale_w = (float)raw_image->w() / (float)card_display_->w();
								float scale_h = (float)raw_image->h() / (float)card_display_->h();
								if (scale_w < scale_h) {
									image_ = raw_image->copy((int)(raw_image->w() / scale_h), card_display_->h());
								}
								else {
									image_ = raw_image->copy(card_display_->w(), (int)(raw_image->h() / scale_w));
								}
							}
							else {
								// Stretch image to fit entire box
								image_ = raw_image->copy(card_display_->w(), card_display_->h());
							}
						}
						// Tidy up
						delete raw_image;
					}
				}

				// Update view.
				card_filename_out_->copy_label(full_name.c_str());
			}
		}
		else {
			// Display what a generated QSL card label looks like, we'll generate it later as it's drawn into the image widget
			delete image_;
			image_ = nullptr;
		}
	}
	else {
		// Asking to accept a new contact so expect no image
		delete image_;
		image_ = nullptr;
	}
	// Got an image: draw it
	draw_image();
}

// Set the values of the various buttons associated with the image.
void qsl_viewer::set_image_buttons() {
	switch (selected_image_) {
	case QI_EQSL:
		// eQSL displayed
		eqsl_radio_->value(true);
		card_front_radio_->value(false);
		card_back_radio_->value(false);
		gen_card_radio_->value(false);
		break;
	case QI_CARD_FRONT:
		// Display the front of a scanned-in paper card
		eqsl_radio_->value(false);
		card_front_radio_->value(true);
		card_back_radio_->value(false);
		gen_card_radio_->value(false);
		break;
	case QI_CARD_BACK:
		// Display the back of a scanned-in paper card
		eqsl_radio_->value(false);
		card_front_radio_->value(false);
		card_back_radio_->value(true);
		gen_card_radio_->value(false);
		break;
	case QI_GEN_CARD:
		// Display the generated design for a QSL card label
		eqsl_radio_->value(false);
		card_front_radio_->value(false);
		card_back_radio_->value(false);
		gen_card_radio_->value(true);
		break;
	default:
		// Shouldn't get here
		eqsl_radio_->value(false);
		card_front_radio_->value(false);
		card_back_radio_->value(false);
		gen_card_radio_->value(false);
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
void qsl_viewer::draw_image() {
	// Remove existing images
	card_display_->clear();
	card_display_->label(nullptr);
	card_display_->image(nullptr);
	card_display_->deimage(nullptr);
	switch (selected_image_) {
	case QI_EQSL:
	case QI_CARD_BACK:
	case QI_CARD_FRONT:
		// We want to display the saved QSL image
		if (image_) {
			// we have an image
			// Set the resized image as the selected and unselected image for the control
			card_display_->image(image_);
			card_display_->deimage(image_);
			// Set the image fileanme text colour black (i.e. OK)
			card_filename_out_->labelcolor(FL_BLACK);
		}
		else {
			// Display a label instead in large letters - 36 pt.
			card_display_->copy_label(current_qso_->item("CALL").c_str());
			card_display_->labelsize(36);
			card_display_->color(FL_WHITE);
			card_display_->labelcolor(FL_BLACK);
			// Display the error message in red.
			card_filename_out_->labelcolor(FL_RED);
		}
		break;
	case QI_GEN_CARD:
		// Generate the QSL card (as to be printed)
		if (current_qso_ != nullptr) {
			// Generate the QSL card label and add it to the display widget
			qsl_form* card = new qsl_form(card_display_->x(), card_display_->y(), &current_qso_, 1);
			card_display_->color(FL_WHITE);
			card_display_->align(FL_ALIGN_CENTER);
			card_display_->add(card);
			char label[128];
			snprintf(label, 128, "Generated QSL label for %s", current_qso_->item("CALL").c_str());
			card_filename_out_->copy_label(label);
		}
		break;
	case QI_TEXT:
		// Generate a text display for parse_all_text() to use
		text_display_ = new Fl_Text_Display(card_display_->x(), card_display_->y(), card_display_->w(), card_display_->h(), nullptr);
		text_display_->textcolor(FL_BLACK);
		text_display_->textfont(FL_COURIER);
		text_display_->textsize(12);
		card_display_->add(text_display_);
		card_filename_out_->label("The records found that match the query");
		card_filename_out_->labelcolor(FL_BLACK);
		break;
	}
	card_display_->redraw();
}

// Set the selected image to that provided
void qsl_viewer::set_selected_image(image_t value) {
	selected_image_ = value;
}

// Set QSL status
void qsl_viewer::set_qsl_status() {
	if (current_qso_) {
		if (current_qso_->item("EQSL_QSL_RCVD") == "Y") {
			eqsl_status_box_->label("eQSL Confirmed");
			eqsl_status_box_->color(FL_GREEN);
		}
		else {
			eqsl_status_box_->label("eQSL Not Confirmed");
			eqsl_status_box_->color(fl_lighter(FL_RED));
		}
		if (current_qso_->item("LOTW_QSL_RCVD") == "Y") {
			lotw_status_box_->label("LotW Confirmed");
			lotw_status_box_->color(FL_GREEN);
		} else {
			lotw_status_box_->label("LotW Not Confirmed");
			lotw_status_box_->color(fl_lighter(FL_RED));
		}
	}
}


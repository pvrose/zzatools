#include "record_form.h"
#include "record_table.h"

#include "../zzalib/utils.h"
#include "spec_data.h"
#include "book.h"
#include "import_data.h"
#include "eqsl_handler.h"
#include "menu.h"
#include "../zzalib/callback.h"
#include "intl_dialog.h"
#include "field_choice.h"
#include "qsl_form.h"
#include "qrz_handler.h"
#include "status.h"
#include "main_window.h"

#include <array>
#include <string>
#include <map>

#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Tooltip.H>

using namespace zzalog;
using namespace zzalib;

extern spec_data* spec_data_;
extern import_data* import_data_;
extern eqsl_handler* eqsl_handler_;
extern Fl_Preferences* settings_;
extern menu* menu_;
extern main_window* main_window_;
extern book* navigation_book_;
extern intl_dialog* intl_dialog_;
extern qrz_handler* qrz_handler_;
extern status* status_;
extern book* book_;


using namespace std;

// Actions for non-dedicated buttons
enum edit_action_t {
	NONE,         // No action
	START,        // Start a new entry
	RESTART,      // Save current entry and start a new one
	SAVE,         // Save current entry
	REVERT,       // Revert to unmodified entry
	CANCEL,       // Cancel current entry
	REJECT,       // Reject imported entry
	ADD,          // Add imported entry
	MERGE,        // Merge log and import query
	FIND_WSJTX,   // Find in WSJT-X ALL.Txt file
	KEEP_LOG,     // Keep existing log version of duplicate entry
	KEEP_DUPE,    // Replace log with duplicate version
	MERGE_DUPE,   // Merge log and duplicate records
	KEEP_BOTH,    // Keep both log and duplicate records
	MERGE_DONE    // Merge additional details
};

// Constructor
record_form::record_form(int X, int Y, int W, int H, const char* label, field_ordering_t /*app*/) :
	Fl_Group(X, Y, W, H, label)
	, view()
	, card_display_(nullptr)
	, card_filename_out_(nullptr)
	, keep_bn_(nullptr)
	, card_type_grp_(nullptr)
	, eqsl_radio_(nullptr)
	, card_front_radio_(nullptr)
	, card_back_radio_(nullptr)
	, gen_card_radio_(nullptr)
	, fetch_bn_(nullptr)
	, scaling_image_(false)
	, record_table_(nullptr)
	, question_out_(nullptr)
	, message_grp_(nullptr)
	, qsl_message_in_(nullptr)
	, swl_message_in_(nullptr)
	, modify_message_bn_(nullptr)
	, editting_grp_(nullptr)
	, field_choice_(nullptr)
	, value_in_(nullptr)
	, enum_choice_(nullptr)
	, quick_grp_(nullptr)
	, call_bn_(nullptr)
	, name_bn_(nullptr)
	, qth_bn_(nullptr)
	, rst_rcvd_bn_(nullptr)
	, rst_sent_bn_(nullptr)
	, grid_bn_(nullptr)
	, use_bn_(nullptr)
	, edit1_bn_(nullptr)
	, edit2_bn_(nullptr)
	, edit3_bn_(nullptr)
	, find_bn_(nullptr)
	, previous_bn_(nullptr)
	, next_bn_(nullptr)
	, record_1_(nullptr)
	, record_2_(nullptr)
	, saved_record_(nullptr)
	, item_num_1_(0)
	, item_num_2_(0)
	, use_mode_(UM_DISPLAY)
	, query_message_("")
	, is_enumeration_(false)
	, current_enum_type_("")
	, selected_image_(QI_EQSL)
	, image_(nullptr)
	, edit_mode_(EM_ORIGINAL)
	, current_field_("")
	, modifying_(false)
	, enable_all_search_(false)

{
	// widget positioning
	const int WCARD = 540;
	const int HCARD = 340;
	const int WTABLE = 420;
	const int WQSL = WBUTTON;
	const int YQSL = YTOP + HCARD + HTEXT + GAP;
	const int XMESS = XLEFT + WQSL + GAP + GAP + GAP;
	const int HMESS = (2 * GAP) + (2 * HTEXT) + HBUTTON;
	const int WMGRP = WMESS + (2 * GAP) + WLABEL;
	const int YDUPE = YQSL + HMESS + GAP;
	const int XEDIT = XMESS + WMGRP + GAP + GAP;
	const int WEGRP = WLABEL + GAP + WEDIT + GAP + WBUTTON + GAP + GAP + WBUTTON + WBUTTON + GAP + GAP;
	const int XQGRP = XEDIT + WLABEL + GAP + WEDIT + GAP + WBUTTON + GAP;
	const int YQGRP = YQSL + GAP + GAP;
	const int HEGRP = max(GAP + HRADIO + HTEXT + HMLIN + HTEXT + GAP + HBUTTON + GAP, GAP + (7*HBUTTON) + GAP);

	// Initialise variables
	selected_image_ = QI_EQSL;
	saved_record_ = nullptr;
	image_ = nullptr;

	// Get image settings
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.get("Image Type", (int&)selected_image_, QI_EQSL);
	display_settings.get("Image Scale", (int&)scaling_image_, false);

	// Card image and its filename
	int curr_x = X + XLEFT;
	int curr_y = Y + YTOP;
	// Box - for the display of a card image or if no cardd exists the callsign in large text
	card_display_ = new Fl_Group(curr_x, curr_y, WCARD, HCARD);
	card_display_->box(FL_UP_BOX);
	card_display_->tooltip("The card image is displayed here!");
	card_display_->align(FL_ALIGN_CENTER);
	card_display_->end();
	curr_y += HCARD;
	// Output - Filename of the image
	card_filename_out_ = new Fl_Box(curr_x, curr_y, WCARD, HTEXT);
	card_filename_out_->labelsize(FONT_SIZE);
	card_filename_out_->align(FL_ALIGN_CENTER);
	card_filename_out_->box(FL_DOWN_BOX);
	card_filename_out_->color(FL_WHITE);
	card_filename_out_->label("Filename");
	card_filename_out_->tooltip("The filename of the displayed card image ");

	// record data and message
	curr_x += WCARD + GAP;
	// Box - record merge or dupe query message
	question_out_ = new Fl_Box(curr_x, curr_y, WTABLE, HTEXT);
	question_out_->labelcolor(FL_BLACK);
	question_out_->labelsize(FONT_SIZE);
	question_out_->box(FL_DOWN_BOX);
	question_out_->color(FL_BACKGROUND_COLOR);
	question_out_->label("");
	question_out_->align(FL_ALIGN_CENTER);
	question_out_->tooltip("A query is presented here for actioning");

	// QSL display controls
	curr_x = X + XLEFT;
	curr_y = Y + YQSL;
	card_type_grp_ = new Fl_Group(curr_x, curr_y, WQSL + (2 * GAP), 8 * HBUTTON + (2 * GAP), "QSL type selection");
	card_type_grp_->labelsize(FONT_SIZE);
	card_type_grp_->align(FL_ALIGN_TOP_LEFT);
	card_type_grp_->box(FL_DOWN_BOX);
	curr_x += GAP;
	curr_y += GAP;
	// Light - Keep the current selection for displaying the card image
	keep_bn_ = new Fl_Light_Button(curr_x, curr_y, WQSL, HBUTTON, "Keep?");
	keep_bn_->selection_color(FL_RED);
	keep_bn_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	keep_bn_->value(false);
	keep_bn_->labelsize(FONT_SIZE);
	keep_bn_->tooltip("Remembers selection after record update");
	curr_y += HBUTTON;
	// Radio - Display eQSL.cc card image
	eqsl_radio_ = new Fl_Radio_Round_Button(curr_x, curr_y, WQSL, HBUTTON, "eQSL");
	eqsl_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	eqsl_radio_->callback(cb_rad_card, (void*)QI_EQSL);
	eqsl_radio_->when(FL_WHEN_RELEASE);
	eqsl_radio_->labelsize(FONT_SIZE);
	eqsl_radio_->tooltip("Select image downloaded from eQSL");
	curr_y += HBUTTON;
	// Radio - display scanned image of front of paper card
	card_front_radio_ = new Fl_Radio_Round_Button(curr_x, curr_y, WQSL, HBUTTON, "Card(F)");
	card_front_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	card_front_radio_->callback(cb_rad_card, (void*)QI_CARD_FRONT);
	card_front_radio_->when(FL_WHEN_RELEASE);
	card_front_radio_->labelsize(FONT_SIZE);
	card_front_radio_->tooltip("Select image scanned of paper card front");
	curr_y += HBUTTON;
	// Radio - display scanned image of reverse of paper card
	card_back_radio_ = new Fl_Radio_Round_Button(curr_x, curr_y, WQSL, HBUTTON, "Card(B)");
	card_back_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	card_back_radio_->callback(cb_rad_card, (void*)QI_CARD_BACK);
	card_back_radio_->when(FL_WHEN_RELEASE);
	card_back_radio_->labelsize(FONT_SIZE);
	card_back_radio_->tooltip("Select image scanned of paper card back");
	curr_y += HBUTTON;
	// Radio - generate QSL card and display
	gen_card_radio_ = new Fl_Radio_Round_Button(curr_x, curr_y, WQSL, HBUTTON, "Generate");
	gen_card_radio_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	gen_card_radio_->callback(cb_rad_card, (void*)QI_GEN_CARD);
	gen_card_radio_->when(FL_WHEN_RELEASE);
	gen_card_radio_->labelsize(FONT_SIZE);
	gen_card_radio_->tooltip("Select image scanned of paper card back");
	curr_y += HBUTTON;

	// Button - Fetch the card image from eQSL.cc
	fetch_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Fetch");
	fetch_bn_->labelsize(FONT_SIZE);
	fetch_bn_->tooltip("Request a fresh download of the eQSL image");
	fetch_bn_->callback(cb_bn_fetch, &item_num_1_);
	fetch_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - Mark paper card received
	log_card_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Log card");
	log_card_bn_->labelsize(FONT_SIZE);
	log_card_bn_->tooltip("Log a paper card received on today's date");
	log_card_bn_->callback(cb_bn_log_card, &record_1_);
	log_card_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - Scale or streth
	scale_bn_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Scale image");
	scale_bn_->labelsize(FONT_SIZE);
	scale_bn_->tooltip("Scale - keep in proportion");
	scale_bn_->callback(cb_bn_scale, nullptr);
	scale_bn_->when(FL_WHEN_RELEASE);
	card_type_grp_->end();

	// Message group
	curr_x = X + XMESS;
	curr_y = Y + YQSL;
	message_grp_ = new Fl_Group(curr_x, curr_y, WMGRP, HMESS, "Default Messages");
	message_grp_->align(FL_ALIGN_TOP_LEFT);
	message_grp_->labelsize(FONT_SIZE);
	message_grp_->box(FL_DOWN_BOX);
	curr_x += GAP + WLABEL;
	curr_y += GAP;
	// Input - QSL message to be added to eQSL
	qsl_message_in_ = new intl_input(curr_x, curr_y, WMESS, HTEXT, "QSL");
	qsl_message_in_->labelsize(FONT_SIZE);
	qsl_message_in_->value("Tnx QSO <NAME>, 73");
	qsl_message_in_->textsize(FONT_SIZE);
	qsl_message_in_->align(FL_ALIGN_LEFT);
	qsl_message_in_->tooltip("The message sent in QSLMSG when uploading to eQSL or printing card for a contact");
	curr_y += HTEXT;
	// Input - SWL message to be added to eQSL
	swl_message_in_ = new intl_input(curr_x, curr_y, WMESS, HTEXT, "SWL");
	swl_message_in_->labelsize(FONT_SIZE);
	swl_message_in_->value("Tnx SWL Report <NAME>, 73");
	swl_message_in_->textsize(FONT_SIZE);
	swl_message_in_->align(FL_ALIGN_LEFT);
	swl_message_in_->tooltip("The message sent in QSLMSG when uploading to eQSL or printing card in reply to an SWL report");
	curr_y += HTEXT;
	// Button - Set the default messages to current display
	modify_message_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Modify");
	modify_message_bn_->labelsize(FONT_SIZE);
	modify_message_bn_->tooltip("Changes the default messages");
	modify_message_bn_->callback(cb_bn_modify, nullptr);
	message_grp_->end();

	// QSO Editting group
	curr_x = X + XEDIT;
	curr_y = Y + YQSL;
	editting_grp_ = new Fl_Group(curr_x, curr_y, WEGRP, HEGRP, "QSO Editting");
	editting_grp_->align(FL_ALIGN_TOP_LEFT);
	editting_grp_->labelsize(FONT_SIZE);
	editting_grp_->box(FL_DOWN_BOX);
	curr_x += GAP + WLABEL;
	curr_y += GAP;

	// Check button 
	all_fields_bn_ = new Fl_Check_Button(curr_x, curr_y, WRADIO, HRADIO, "All fields");
	all_fields_bn_->align(FL_ALIGN_RIGHT);
	all_fields_bn_->labelsize(FONT_SIZE);
	all_fields_bn_->labelfont(FONT);
	all_fields_bn_->tooltip("Selects whether to have all fields in the below selection");
	all_fields_bn_->callback(cb_bn_all_fields, (void*)&display_all_fields_);
	all_fields_bn_->when(FL_WHEN_RELEASE);
	curr_y += all_fields_bn_->h();

	// Choice - field name to be edited
	field_choice_ = new field_choice(curr_x, curr_y, WEDIT, HTEXT, "Field");
	field_choice_->align(FL_ALIGN_LEFT);
	field_choice_->labelsize(FONT_SIZE);
	field_choice_->textsize(FONT_SIZE);
	field_choice_->tooltip("Select the field to add/edit");
	field_choice_->callback(cb_ch_field);
	field_choice_->when(FL_WHEN_RELEASE | FL_WHEN_NOT_CHANGED);

	curr_y += HTEXT;
	// Text editor - new value of field
	value_in_ = new intl_editor(curr_x, curr_y, WEDIT, HMLIN, "Value");
	value_in_->align(FL_ALIGN_LEFT);
	value_in_->labelsize(FONT_SIZE);
	value_in_->textsize(FONT_SIZE);
	value_in_->callback(cb_editor);
	value_in_->when(FL_WHEN_CHANGED);
	value_in_->tooltip("Enter the new value");
	Fl_Text_Buffer* buff = new Fl_Text_Buffer(1024);
	value_in_->buffer(buff);
	curr_y += HMLIN;
	// Choice - list of values for an enumeration field
	enum_choice_ = new Fl_Choice(curr_x, curr_y, WEDIT, HTEXT, "Enum");
	enum_choice_->align(FL_ALIGN_LEFT);
	enum_choice_->labelsize(FONT_SIZE);
	enum_choice_->textsize(FONT_SIZE);
	enum_choice_->tooltip("Select the enumerated value for the field");
	enum_choice_->callback(cb_ch_enum);
	enum_choice_->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED);
	curr_y += HTEXT + GAP;
	// Button - update record with editor value or selected enumeration value
	use_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Use");
	use_bn_->labelsize(FONT_SIZE);
	use_bn_->color(fl_lighter(FL_GREEN));
	use_bn_->tooltip("Copy the edit value to the log record");
	use_bn_->callback(cb_bn_use);
	use_bn_->when(FL_WHEN_RELEASE);
	curr_x += WBUTTON;
	// Button - clear the text editor or select blank enumeration value
	clear_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Clear");
	clear_bn_->labelsize(FONT_SIZE);
	clear_bn_->color(fl_color_average(FL_RED, FL_WHITE, 0.25));
	clear_bn_->tooltip("Clear the edit value");
	clear_bn_->callback(cb_bn_clear);
	clear_bn_->when(FL_WHEN_RELEASE);
	curr_x = X + XEDIT + WEDIT + WLABEL + GAP + GAP;
	curr_y = editting_grp_->y() + GAP;
	// Button - Find a record that may match the query record
	find_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "@search");
	find_bn_->labelsize(FONT_SIZE);
	find_bn_->tooltip("Find a possible matching query");
	find_bn_->color(fl_lighter(FL_YELLOW));
	find_bn_->callback(cb_bn_find);
	find_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - Go the previous record when matching query
	previous_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "@<-");
	previous_bn_->labelsize(FONT_SIZE);
	previous_bn_->tooltip("Get the previous log record");
	previous_bn_->color(fl_lighter(FL_YELLOW));
	previous_bn_->callback(cb_bn_prev);
	previous_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - Go to the next record when matching query
	next_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "@->");
	next_bn_->labelsize(FONT_SIZE);
	next_bn_->tooltip("Get the next log record");
	next_bn_->color(fl_lighter(FL_YELLOW));
	next_bn_->callback(cb_bn_next);
	next_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - meaning depends on current usage
	edit1_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "1");
	edit1_bn_->labelsize(FONT_SIZE);
	curr_y += HBUTTON;
	// Button - meaning depends on current usage
	edit2_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "2");
	edit2_bn_->labelsize(FONT_SIZE);
	curr_y += HBUTTON;
	// Button - meaning depends on current usage
	edit3_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "3");
	edit3_bn_->labelsize(FONT_SIZE);
	curr_y += HBUTTON;
	// Button - meaning depends on current usage
	edit4_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "4");
	edit4_bn_->labelsize(FONT_SIZE);

	// Quick entry button - copies the entry field directlt to the specified item
	curr_x = X + XQGRP;
	curr_y = Y + YQGRP;
	quick_grp_ = new Fl_Group(curr_x, curr_y, GAP + WBUTTON + WBUTTON + GAP, GAP + (5 * HBUTTON) + GAP, "Quick editting");
	quick_grp_->align(FL_ALIGN_TOP_LEFT);
	quick_grp_->labelsize(FONT_SIZE);
	quick_grp_->box(FL_DOWN_BOX);
	curr_x += GAP;
	curr_y += GAP;
	// Button - copy editor text to CALL field
	call_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "CALL");
	call_bn_->labelsize(FONT_SIZE);
	call_bn_->color(fl_lighter(FL_MAGENTA));
	call_bn_->tooltip("Copy the value text into the CALL field of the log record");
	call_bn_->callback(cb_bn_quick, (void*)"CALL");
	call_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to NAME field
	name_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "NAME");
	name_bn_->labelsize(FONT_SIZE);
	name_bn_->color(fl_lighter(FL_MAGENTA));
	name_bn_->tooltip("Copy the value text into the NAME field of the log record");
	name_bn_->callback(cb_bn_quick, (void*)"NAME");
	name_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to QTH field
	qth_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "QTH");
	qth_bn_->labelsize(FONT_SIZE);
	qth_bn_->color(fl_lighter(FL_MAGENTA));
	qth_bn_->tooltip("Copy the value text into the QTH field of the log record");
	qth_bn_->callback(cb_bn_quick, (void*)"QTH");
	qth_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to RST_RCVD field
	rst_rcvd_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "RST_R");
	rst_rcvd_bn_->labelsize(FONT_SIZE);
	rst_rcvd_bn_->color(fl_lighter(FL_MAGENTA));
	rst_rcvd_bn_->tooltip("Copy the value text into the RST_RCVD field of the log record");
	rst_rcvd_bn_->callback(cb_bn_quick, (void*)"RST_RCVD");
	rst_rcvd_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to GRIDSQUARE field
	grid_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "GRID");
	grid_bn_->labelsize(FONT_SIZE);
	grid_bn_->color(fl_lighter(FL_MAGENTA));
	grid_bn_->tooltip("Copy the value text into the GRIDSQUARE field of the log record");
	grid_bn_->callback(cb_bn_quick, (void*)"GRIDSQUARE");
	grid_bn_->when(FL_WHEN_RELEASE);
	curr_y = Y + YQGRP + GAP;
	curr_x += WBUTTON;
	// Button - copy editor text to FREQ field
	freq_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "FREQ");
	freq_bn_->labelsize(FONT_SIZE);
	freq_bn_->color(fl_lighter(FL_MAGENTA));
	freq_bn_->tooltip("Copy the value text into the FREQ field of the log record");
	freq_bn_->callback(cb_bn_quick, (void*)"FREQ");
	freq_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to MODE field
	mode_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "MODE");
	mode_bn_->labelsize(FONT_SIZE);
	mode_bn_->color(fl_lighter(FL_MAGENTA));
	mode_bn_->tooltip("Copy the value text into the MODE field of the log record");
	mode_bn_->callback(cb_bn_quick, (void*)"MODE");
	mode_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to POWER field
	power_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "POWER");
	power_bn_->labelsize(FONT_SIZE);
	power_bn_->color(fl_lighter(FL_MAGENTA));
	power_bn_->tooltip("Copy the value text into the TX_PWR field of the log record");
	power_bn_->callback(cb_bn_quick, (void*)"TX_PWR");
	power_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;
	// Button - copy editor text to RST_SENT field
	rst_rcvd_bn_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "RST_S");
	rst_rcvd_bn_->labelsize(FONT_SIZE);
	rst_rcvd_bn_->color(fl_lighter(FL_MAGENTA));
	rst_rcvd_bn_->tooltip("Copy the value text into the RST_SENT field of the log record");
	rst_rcvd_bn_->callback(cb_bn_quick, (void*)"RST_SENT");
	rst_rcvd_bn_->when(FL_WHEN_RELEASE);
	curr_y += HBUTTON;

	quick_grp_->end();
	editting_grp_->end();

	// For some reason creating this at the start prevents the rest of the widgets being installed
	// This can be fixed by using this->add() after each of the above widget instantiations, but, meh!
	curr_x = X + XLEFT + WCARD + GAP;
	curr_y = Y + YTOP;
	// The table displaying all fields
	record_table_ = new record_table(curr_x, curr_y, WTABLE, HCARD, "");
	record_table_->tooltip("Displays the record contents");
	record_table_->callback(cb_tab_record);
	record_table_->when(FL_WHEN_RELEASE);
	this->end();
	Fl_Group::show();

	// Let the minimum resizing be the size of this group.
	min_w_ = w();
	min_h_ = h();
	resizable(nullptr);

}

// Destructor
record_form::~record_form()
{
	delete saved_record_;
	delete image_;
	// This seems to work - remove the text buffer in the value editting window
	Fl_Text_Buffer* b = value_in_->buffer();
	value_in_->buffer(nullptr);
	delete b;
	field_choice_->clear();
	enum_choice_->clear();

}

// Call-backs
// called when any of the image selection radio buttons is released
// v is enum image_t.
void record_form::cb_rad_card(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	// Get the selected image and display it.
	that->set_selected_image((image_t)(long)v);
	that->set_image();
	// Save setting
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.set("Image Type", (int)that->selected_image_);
//	main_window_->flush();
}

// Called when a cell is clicked in the record table
void record_form::cb_tab_record(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	record_table* table = (record_table*)w;
	if (table->callback_context() == Fl_Table::CONTEXT_CELL ||
		table->callback_context() == Fl_Table::CONTEXT_ROW_HEADER) {
		// clicked on a valid row or row header
		// Copy field name and text into editing controls in the form
		int row = table->callback_row();
		int col = table->callback_col();
		string field = table->field(row);
		// Which button did we click?
		switch (Fl::event_button()) {
		case FL_LEFT_MOUSE: {
			// Left click - edit the field
			string text;
			if (that->record_1_ != nullptr && col == 0) {
				// We have a base record and user clicked base record - enable editting the record
				text = that->record_1_->item(field);
				if (that->use_mode_ == UM_DISPLAY || that->use_mode_ == UM_MODIFIED) {
					that->modifying_ = true;
				}
				that->edit_mode_ = EM_EDIT;
			}
			else if (that->record_2_ != nullptr && 
				((that->record_1_ == nullptr && col == 0) ||
				(that->record_1_ != nullptr && col == 1) ) ) {
				// Copy the query data to editting widget, set editting mode
				text = that->record_2_->item(field);
				that->edit_mode_ = EM_QUERY;
			}
			else if (that->record_1_ != nullptr &&
				that->record_2_ != nullptr &&
				that->saved_record_ != nullptr &&
				col == 2) {
				// Clicked original value, prepare to restore original record value
				text = that->saved_record_->item(field);
				that->edit_mode_ = EM_ORIGINAL;
			}
			// Copy the selected text into the text and enum value controls
			that->set_edit_widgets(field, text);
			that->modifying_ = true;
			that->enable_widgets();
			that->redraw();
			break;
		}
		case FL_RIGHT_MOUSE: {
			// Right click - display tip
			string tip;
			switch (table->callback_context()) {
			case Fl_Table::CONTEXT_ROW_HEADER: {
				// Fetch tip to explain the field
				tip = spec_data_->get_tip(field);
				break;
			}
			case Fl_Table::CONTEXT_CELL: {
				// Fetch  to explain the field and validate its contect on the clicked record
				switch (table->callback_col()) {
				case 0:
					tip = spec_data_->get_tip(field, that->record_1_);
					break;
				case 1:
					tip = spec_data_->get_tip(field, that->record_2_);
					break;
				case 2:
					tip = spec_data_->get_tip(field, that->saved_record_);
					break;
				}
			}
			}
			// Display the tip window at the position of the click
			int x_root = Fl::event_x_root();
			int y_root = Fl::event_y_root();
			Fl_Window* tip_window = ::tip_window(tip, x_root, y_root);
			Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tip_window);
			break;
		}
		}
	}
}

// Call when all_fields button is changed
// v points to display_all_fields_
void record_form::cb_bn_all_fields(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	cb_value<Fl_Check_Button, bool>(w, v);
	((field_choice*)that->field_choice_)->repopulate(that->display_all_fields_, that->current_field_);
}

// Called when the field choice widget is clicked and released
// v is not used
void record_form::cb_ch_field(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	Fl_Choice* choice = (Fl_Choice*)w;
	char temp[128];
	// Get the item clicked in the choice
	if (choice->item_pathname(temp, sizeof(temp) - 1) == 0) {
		// Get the value of the item clicked and allow it to be edited
		string field = &temp[1];
		string text;
		// If the log record exists - copy that
		if (that->record_1_) {
			text = that->record_1_->item(field, true);
		}
		// Otherwise use the query record
		else if (that->record_2_) {
			text = that->record_2_->item(field, true);
		}
		that->set_edit_widgets(field, text);
		that->modifying_ = true;
		that->enable_widgets();
		that->redraw();
	}
}

// Called after typing in the editor
// v is not used
void record_form::cb_editor(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	that->modifying_ = true;
	that->enable_widgets();
	that->redraw();
}


// Enum choice widget has its value changed - explain what the enum value means
// v is not used
void record_form::cb_ch_enum(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	Fl_Choice* choice = (Fl_Choice*)w;
	string enum_value = "";
	char temp[128];
	// Get the name of the item selected in the Fl_Choice
	choice->item_pathname(temp, sizeof(temp) - 1);
	// If there is a value get its text
	if (temp[0] != 0) enum_value = &temp[1];
	// Get the enum specification dataset and display the value's meaning
	spec_dataset* dataset = spec_data_->dataset(that->current_enum_type_);
	that->explain_enum(dataset, enum_value);
	that->modifying_ = true;
	that->enable_widgets();
}

// Use the text in the text edit or selected enum choice value 
// v is not used
void record_form::cb_bn_use(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	// Update the record - get the fieldname in upper case - note item name in choice is prefixed with root symbol '/'
	char temp[128];
	// Get the name of current selected field-name in the Fl_Choice
	that->field_choice_->item_pathname(temp, sizeof(temp) - 1);
	string field = to_upper(string(&temp[1]));
	// Get the new value from the text edit widget
	if (field != "") {
		// Get book to remember the record
		that->my_book_->remember_record();
		// Get the new value in the text buffer
		string text = that->value_in_->buffer()->text();
		// Get the selected value of the enum Fl_Choice
		that->enum_choice_->item_pathname(temp, sizeof(temp) - 1);
		string enum_value = "";
		// Get the enumeation value if there is one if it exists
		if (temp[0] != 0) enum_value = &temp[1];

		if (that->is_enumeration_) {
			// Get the enumerated value if there was one
			that->record_1_->item(field, enum_value);
		}
		else {
			// Get the text itself
			that->record_1_->item(field, text);
		}
		// Redraw the list control
		that->record_table_->set_records(that->record_1_, that->record_2_, that->saved_record_);
		// Tell views what sort of change it is
		if (!that->my_book_->new_record()) {
			that->my_book_->modified_record(true);
		}
		if (field == "QSO_DATE" || field == "TIME_ON") {
		// This will result in a whole-scale redraw
		// so only update views if not in the middle of a query
			switch (that->use_mode_) {
			case UM_DISPLAY:
			case UM_QSO:
			case UM_MODIFIED:
				// Tell the book the ordering of records has changed. Book will then let all views know.
				book_->selection(that->my_book_->record_number(that->item_num_1_), HT_START_CHANGED);
				break;
			}
		}
		else if (field == "DXCC" || field == "GRIDSQUARE") {
			// Loction has changed - DxAtlas needs to redraw.
			that->my_book_->selection(that->item_num_1_, HT_CHANGED, that);
		}
		else {
			// Any other change
			that->my_book_->selection(that->item_num_1_, HT_MINOR_CHANGE, that);
		}

		that->modifying_ = false;
		if (that->use_mode_ == UM_DISPLAY) {
			that->use_mode_ = UM_MODIFIED;
		}
	}
	// Clear value in buffer
	that->value_in_->buffer()->text("");
	that->enable_widgets();
}

// Clear edit value button has been clicked
// v is not used
void record_form::cb_bn_clear(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	// Clear value in buffer
	that->value_in_->buffer()->text("");
	// Clear enum and field choices
	that->enum_choice_->value(0);
	that->field_choice_->value(0);
	that->enable_widgets();
}

// Use the txt in the text edit for a specific field
// v is char* with name of field to update
void record_form::cb_bn_quick(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	string field = (char*)v;
	// Remeber the record
	that->my_book_->remember_record();
	// Set the text edit value to the selected item
	string value = that->value_in_->buffer()->text();
	that->record_1_->item(field, value);
	// Redraw the list control
	that->record_table_->set_records(that->record_1_, that->record_2_, that->saved_record_);
	if (!that->my_book_->new_record()) {
		that->my_book_->modified_record(true);
	}
	// Only a limited number of fileds are covered by the Quick button
	if (field == "GRIDSQUARE") {
		// Location has changed
		that->my_book_->selection(that->item_num_1_, HT_CHANGED, that);
	}
	else {
		// Any other change
		that->my_book_->selection(that->item_num_1_, HT_MINOR_CHANGE, that);
	}
	// Clear value in buffer
	that->value_in_->buffer()->text("");
	// Clear modifying flag
	that->modifying_ = false;
	that->enable_widgets();
}

// Modify messages button has been pressed - save to settings
// v is unused
void record_form::cb_bn_modify(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences card_settings(qsl_settings, "Card");
	card_settings.set("QSL Message", that->qsl_message_in_->value());
	card_settings.set("SWL Message", that->swl_message_in_->value());
}

// Fetch the eQSL card for the supplied record
// v is record_num_t* containing the number of the record to fetch the eQSL card for
void record_form::cb_bn_fetch(Fl_Widget* w, void* v) {
	record_num_t record = *((record_num_t*)v);
	// Put the card request onto the eQSL request queue - so that requests are made
	eqsl_handler_->enqueue_request(record, true);
	eqsl_handler_->enable_fetch(eqsl_handler::EQ_START);
}

// Set whether we stretch or scale the image
// v is unused
void record_form::cb_bn_scale(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	cb_value<Fl_Light_Button, bool>(w, &that->scaling_image_);
	that->set_image();
	// Save setting
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.set("Image Scale", (int)that->scaling_image_);

}

// Set the QSL_RCVD and QSLRDATE values in current record
// v is unused
void record_form::cb_bn_log_card(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	string today = now(false, "%Y%m%d");
	that->record_1_->item("QSLRDATE", today);
	that->record_1_->item("QSL_RCVD", string("Y"));
	// Redraw the list table and notify views of a minor change
	that->record_table_->set_records(that->record_1_, that->record_2_, that->saved_record_);
	that->my_book_->selection(that->item_num_1_, HT_MINOR_CHANGE, that);
	if (that->use_mode_ == UM_DISPLAY) {
		that->use_mode_ = UM_MODIFIED;
	}
	// now pretend the Card Front radio button has been pressed
	cb_rad_card(that->card_front_radio_, (void*)QI_CARD_FRONT);
	that->set_image_buttons();
}

// Actions for configurable buttons
// v is enum edit_action_t indicating which of the buttons was pressed under which condition
void record_form::cb_bn_edit(Fl_Widget* w, long v) {
	edit_action_t action = (edit_action_t)v;
	record_form* that = ancestor_view<record_form>(w);
	switch (action) {
	case START:
		// Start a new record
		that->my_book_->new_record(menu_->logging());
		that->use_mode_ = UM_QSO;
		break;
	case RESTART:
		// Save current record and start a new one
		that->my_book_->save_record();
		that->my_book_->new_record(menu_->logging());
		that->use_mode_ = UM_QSO;
		break;
	case SAVE:
		// Save current record
		that->my_book_->save_record();
		that->use_mode_ = UM_DISPLAY;
		break;
	case REVERT:
		//copy saved record back to original record
		that->my_book_->delete_record(false);
		break;
	case CANCEL:
		// Cancelling a new QSO entry.
		that->my_book_->delete_record(true);
		break;
	case REJECT:
		// Discard the queried record
		import_data_->discard_update(true);
		break;
	case ADD:
		// Add the queried record to the log
		import_data_->save_update();
		break;
	case MERGE:
		// Merge the changes from the queried record
		import_data_->merge_update();
		break;
	case KEEP_LOG:
		// Discard the queried possible duplicate
		navigation_book_->reject_dupe(false);
		break;
	case KEEP_DUPE:
		// Keep the queried possible duplicate record and discard the original record
		navigation_book_->reject_dupe(true);
		break;
	case MERGE_DUPE:
		// Discard the queried duplicate record after merging the two records
		navigation_book_->merge_dupe();
		break;
	case KEEP_BOTH:
		// Queried duplicate record is not a duplicate, keep both it and the original record
		navigation_book_->accept_dupe();
		break;
	case FIND_WSJTX:
		// Find details of the QSO in WSJT-X ALL.txh file
		switch (that->use_mode_) {
		case UM_QUERY:
			if (that->parse_all_txt(that->record_2_)) {
				that->record_table_->set_records(that->record_1_, that->record_2_, nullptr);
			}
			break;
		default:
			if (that->parse_all_txt(that->record_1_)) {
				that->record_table_->set_records(that->record_1_, that->record_2_, nullptr);
			}
			break;
		}
		that->enable_all_search_ = false;
		that->enable_widgets();
		// Drop out to allow user to chose again 
		return;
	}

	// delete pointer to query record
	that->record_2_ = nullptr;
	that->item_num_2_ = -1;
	that->record_table_->set_records(that->record_1_, that->record_2_, nullptr);
	that->enable_widgets();
	switch (action) {
	case REJECT:
	case ADD:
	case MERGE:
		// Restart the update process
		import_data_->update_book();
		break;
	case KEEP_LOG:
	case KEEP_DUPE:
	case MERGE_DUPE:
	case KEEP_BOTH:
		// Restart the duplicate check process
		navigation_book_->check_dupes(true);
		break;
	case MERGE_DONE:
		// Merge additional data complete
		qrz_handler_->merge_done();
		break;
	}
}

// Find the possible matching record
// v is not used
void record_form::cb_bn_find(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);
	// The update provided both record numbers but we did not get the log record at the time
	that->record_1_ = that->my_book_->get_record(that->item_num_1_, false);
	// Remove saved record, as may need new one
	delete that->saved_record_;
	that->saved_record_ = new record(*that->record_1_);
	// Get another set of records to display
	that->record_table_->set_records(that->record_1_, that->record_2_, that->saved_record_);
	that->enable_widgets();
	that->redraw();
}

// Find next possible matching record
// v is not used
void record_form::cb_bn_next(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);

	if ((unsigned)that->item_num_1_ < that->my_book_->size() - 1) {
		// We are not at the end of the book - display the next record
		that->record_1_ = that->my_book_->get_record(++that->item_num_1_, true);
		delete that->saved_record_;
		that->saved_record_ = new record(*that->record_1_);
		that->record_table_->set_records(that->record_1_, that->record_2_, that->saved_record_);
	}
	that->enable_widgets();
	that->redraw();
}

// Find previous possible matching record
void record_form::cb_bn_prev(Fl_Widget* w, void* v) {
	record_form* that = ancestor_view<record_form>(w);

	if ((unsigned)that->item_num_1_ > 0) {
		// If we are not at the start of the book - display the previous record
		that->record_1_ = that->my_book_->get_record(--that->item_num_1_, true);
		delete that->saved_record_;
		that->saved_record_ = new record(*that->record_1_);
		that->record_table_->set_records(that->record_1_, that->record_2_, that->saved_record_);
	}
	that->enable_widgets();
	that->redraw();

}

// Update the form because a new record has been selected or the record has been changed
void record_form::update(hint_t hint, record_num_t record_num_1, record_num_t record_num_2) {
	// Convert record numbers into item numbers
	record_num_t item_num_1 = my_book_->item_number(record_num_1);
	record_num_t item_num_2 = my_book_->item_number(record_num_2);
	switch (hint) {
	case HT_ALL:
	case HT_CHANGED:
	case HT_MINOR_CHANGE:
	case HT_DELETED:
	case HT_SELECTED:
	case HT_DUPE_DELETED:
	case HT_INSERTED:
	case HT_NEW_DATA:
	case HT_NO_DATA:
	case HT_EXTRACTION:
		// Any sort of change to the book or record
		// Current selected item number may have changed - display the selected record
		item_num_1_ = item_num_1;
		item_num_2_ = -1;
		this->record_1_ = my_book_->get_record(item_num_1_, false);
		this->record_2_ = nullptr;
		delete saved_record_;
		if (record_1_) {
			saved_record_ = new record(*record_1_);
		}
		else {
			saved_record_ = nullptr;
		}
		use_mode_ = UM_DISPLAY;
		edit_mode_ = EM_ORIGINAL;
		query_message_ = "";
		is_enumeration_ = false;
		enable_all_search_ = true;
		update_form();
		break;
	case HT_STARTING:
		// Starting a QSO.
		// Current selected record number may have changed - display the record and allow editting
		item_num_1_ = item_num_1;
		item_num_2_ = -1;
		this->record_1_ = my_book_->get_record(item_num_1_, false);
		this->record_2_ = nullptr;
		delete saved_record_;
		saved_record_ = new record(*record_1_);
		use_mode_ = UM_QSO;
		edit_mode_ = EM_ORIGINAL;
		query_message_ = "";
		is_enumeration_ = false;
		enable_all_search_ = false;
		update_form();
		break;
	case HT_IMPORT_QUERY:
		// Import upload has detected a possible match
		// display both the possible log entry and the queried record - allow addressing the query
		item_num_1_ = item_num_1;
		item_num_2_ = item_num_2;
		this->record_1_ = my_book_->get_record(item_num_1_, false);
		this->record_2_ = import_data_->get_record(item_num_2_, false);
		delete saved_record_;
		saved_record_ = new record(*record_1_);
		use_mode_ = UM_QUERY;
		edit_mode_ = EM_ORIGINAL;
		query_message_ = import_data_->match_question();
		enable_all_search_ = true;
		is_enumeration_ = false;
		update_form();
		break;
	case HT_IMPORT_QUERYNEW:
		// Import upload has not found a possible match
		// Display just the queried record
		item_num_1_ = item_num_1;
		item_num_2_ = item_num_2;
		this->record_1_ = nullptr;
		this->record_2_ = import_data_->get_record(item_num_2_, false);
		delete saved_record_;
		saved_record_ = nullptr;
		use_mode_ = UM_QUERY;
		edit_mode_ = EM_ORIGINAL;
		query_message_ = import_data_->match_question();
		is_enumeration_ = false;
		enable_all_search_ = true;
		update_form();
		break;
	case HT_DUPE_QUERY:
		// Check dupe has found a possible pair of duplicate records
		// Display the two possible duplicate records - allow addressing the dupe query
		item_num_1_ = item_num_1;
		item_num_2_ = item_num_2;
		this->record_1_ = my_book_->get_record(record_num_1, false);
		this->record_2_ = my_book_->get_record(record_num_2, false);
		delete saved_record_;
		saved_record_ = new record(*record_1_);
		use_mode_ = UM_DUPEQUERY;
		edit_mode_ = EM_ORIGINAL;
		query_message_ = my_book_->match_question();
		is_enumeration_ = false;
		enable_all_search_ = false;
		update_form();
		break;
	case HT_MERGE_DETAILS:
		// Import upload has found a probable match
		// display both the possible log entry and the queried record - allow addressing the query
		item_num_1_ = item_num_1;
		item_num_2_ = item_num_2;
		this->record_1_ = my_book_->get_record(item_num_1_, false);
		this->record_2_ = qrz_handler_->get_record();
		delete saved_record_;
		saved_record_ = new record(*record_1_);
		use_mode_ = UM_MERGEDETAILS;
		edit_mode_ = EM_ORIGINAL;
		query_message_ = qrz_handler_->get_merge_message();
		is_enumeration_ = false;
		enable_all_search_ = false;
		update_form();
		break;
	case HT_FORMAT:
		// The settings dialog may have change the order of the fields to display in the field table.
		// Format and/or columns have changed - just redraw the form without changing records
		update_form();
		break;
	}
	
	redraw();
}

// Redraw the form after stuff has changed
void record_form::update_form() {
	if (record_1_ != nullptr) {
		// A record to display - copy any saved card image to the image widget
		if (keep_bn_->value() == false) {
			// If we are not wanting to redisplay the existing QSL card try and display the eQSL card
			selected_image_ = QI_EQSL;
		}
		set_image();
	}
	// Display the image choice buttons
	set_image_buttons();
	// Get the QSL messages from settings
	Fl_Preferences qsl_settings(settings_, "QSL");
	Fl_Preferences qsl_card_settings(qsl_settings, "Card");
	char* qsl_message; 
	qsl_card_settings.get("QSL Message", qsl_message, "Tnx QSO <NAME>, 73");
	qsl_message_in_->value(qsl_message);
	free(qsl_message);
	qsl_card_settings.get("SWL Message", qsl_message, "Tnx SWL Report <NAME>, 73");
	swl_message_in_->value(qsl_message);
	free(qsl_message);

	// Set the question from the Import upload process
	question_out_->copy_label(query_message_.c_str());
	if (query_message_.length()) {
		// WE have such a message - highlight the fact by setting the widget background a light red
		question_out_->color(fl_lighter(FL_RED));
	}
	else {
		// No message, so set the background colour to the default
		question_out_->color(FL_BACKGROUND_COLOR);
	}

	// Tell the table which records to display
	record_table_->set_records(record_1_, record_2_, saved_record_);

	// Enable/Disable widgets depending on settings
	enable_widgets();

	redraw();

}

// Get the card image to display and put it in the image widget.
void record_form::set_image() {
	if (record_1_ != nullptr) {
		if (selected_image_ != QI_GEN_CARD) {
			// We want to display the received card image
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
			string call = record_1_->item("CALL");
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
					strcpy(filename, eqsl_handler_->card_filename_l(record_1_).c_str());
					break;
				case QI_CARD_FRONT:
					// Card image of a scanned-in paper QSL card (front - i.e. callsign side)
					// File name e.g.= <root>\scans\<received date>\PA_GM3ZZA_P__<QSO date>.png
					sprintf(filename, "%s\\Scans\\%s\\%s__%s",
						directory.c_str(),
						record_1_->item("QSLRDATE").c_str(),
						call.c_str(),
						record_1_->item("QSO_DATE").c_str());
					break;
				case QI_CARD_BACK:
					// Card image of a scanned-in paper QSL card (back - i.e. QSO details side)
					// File name e.g.= <root>\scans\<received date>\PA_GM3ZZA_P++<QSO date>.png
					sprintf(filename, "%s\\Scans\\%s\\%s++%s",
						directory.c_str(),
						record_1_->item("QSLRDATE").c_str(),
						call.c_str(),
						record_1_->item("QSO_DATE").c_str());
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
						if (record_1_->item("APP_ZZA_ERROR").length() > 0) {
							// Set error message ton what we already know
							full_name = record_1_->item("APP_ZZA_ERROR");
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
							if (record_1_->item("APP_ZZA_ERROR").length() > 0) {
								// Set error message ton what we already know
								full_name = record_1_->item("APP_ZZA_ERROR");
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
							record_1_->item("APP_ZZA_ERROR", string(""));
							found_image = true;
							// Resize the image to fit the control
							if (scaling_image_) {
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
void record_form::set_image_buttons() {
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
	}
}

// Draw the selected QSL card image (or callsign if no image)
void record_form::draw_image() {
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
			card_display_->copy_label(record_1_->item("CALL").c_str());
			card_display_->labelsize(36);
			card_display_->color(FL_WHITE);
			card_display_->labelcolor(FL_BLACK);
			// Display the error message in red.
			card_filename_out_->labelcolor(FL_RED);
		}
		break;
	case QI_GEN_CARD:
		// Generate the QSL card (as to be printed)
		if (record_1_ != nullptr) {
			// Generate the QSL card label and add it to the display widget
			qsl_form* card = new qsl_form(card_display_->x(), card_display_->y(), &record_1_, 1);
			card_display_->color(FL_WHITE);
			card_display_->align(FL_ALIGN_CENTER);
			card_display_->add(card);
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

// Enable or disable widget according to usage
void record_form::enable_widgets() {
	// Field modification controls
	field_choice_->activate();
	// Enable/disable enumeration choice and field-nam widgets
	if (is_enumeration_) {
		value_in_->activate();
		enum_choice_->activate();
	}
	else {
		value_in_->activate();
		enum_choice_->deactivate();
	}
	// Depending on use-mode activate or deactivate widgets. Allocate the 4 spare buttons
	switch (use_mode_) {
	case UM_DISPLAY:
	case UM_MODIFIED:
		// Activate buttons that we need to add or modify records
		// Card display 
		card_display_->activate();
		card_filename_out_->activate();
		keep_bn_->activate();
		eqsl_radio_->activate();
		card_front_radio_->activate();
		card_back_radio_->activate();
		fetch_bn_->activate();
		log_card_bn_->activate();
		// Query question
		question_out_->deactivate();
		// QSL message
		qsl_message_in_->activate();
		swl_message_in_->activate();
		modify_message_bn_->activate();
		// Import Query 
		find_bn_->deactivate();
		previous_bn_->deactivate();
		next_bn_->deactivate();
		// Start QSO - from EDIT and QSO
		// Variable editing butttons
		if (modifying_ || use_mode_ == UM_MODIFIED) {
			// Modiying a record: Restart, Save, Revert
			edit1_bn_->label("Restart");
			edit1_bn_->color(fl_lighter(FL_BLUE));
			edit1_bn_->tooltip("Save edit and restart a new QSO");
			edit1_bn_->callback(cb_bn_edit, (long)RESTART);
			edit1_bn_->activate();
			edit2_bn_->label("Save");
			edit2_bn_->color(FL_GREEN);
			edit2_bn_->tooltip("Save edit");
			edit2_bn_->callback(cb_bn_edit, (long)SAVE);
			edit2_bn_->activate();
			edit3_bn_->label("Revert");
			edit3_bn_->color(fl_lighter(FL_RED));
			edit3_bn_->tooltip("Revert to original record");
			edit3_bn_->callback(cb_bn_edit, (long)REVERT);
			edit3_bn_->activate();
			// Quick QSL entry
			quick_grp_->activate();
		}
		else {
			// Displaying: Start
			edit1_bn_->label("Start");
			edit1_bn_->color(fl_lighter(FL_BLUE));
			edit1_bn_->tooltip("Start a new QSO");
			edit1_bn_->callback(cb_bn_edit, (long)START);
			edit1_bn_->activate();
			edit2_bn_->label("2");
			edit2_bn_->color(FL_BACKGROUND_COLOR);
			edit2_bn_->tooltip("");
			edit2_bn_->callback(cb_bn_edit, (long)NONE);
			edit2_bn_->deactivate();
			edit3_bn_->label("3");
			edit3_bn_->color(FL_BACKGROUND_COLOR);
			edit3_bn_->tooltip("");
			edit3_bn_->callback(cb_bn_edit, (long)NONE);
			edit3_bn_->deactivate();
			// Quick QSL entry
			quick_grp_->deactivate();
		}
		// If this record is a WSJT-X mode and we don't have a possible match set button 4 to look inn ALL.txt
		if (record_1_ &&
			(record_1_->item("MODE") == "JT65" || record_1_->item("MODE") == "JT9" || record_1_->item("MODE") == "FT8" || record_1_->item("MODE") == "FT4")) {
			// Need to activate display
			card_display_->activate();
			card_filename_out_->activate();
			edit4_bn_->label("Find text");
			edit4_bn_->color(FL_YELLOW);
			edit4_bn_->tooltip("Find record in WSJT-X ALL.txt file");
			if (enable_all_search_) {
				edit4_bn_->callback(cb_bn_edit, (long)FIND_WSJTX);
				edit4_bn_->activate();
			}
			else {
				edit4_bn_->callback(cb_bn_edit, (long)NONE);
				edit4_bn_->deactivate();
			}
		}
		else {
			edit4_bn_->label("4");
			edit4_bn_->color(FL_BACKGROUND_COLOR);
			edit4_bn_->tooltip("");
			edit4_bn_->callback(cb_bn_edit, (long)NONE);
			edit4_bn_->deactivate();
		}
		break;
	case UM_QSO:
		// We are currently capturing a QSO on-air, only enable relevant widgets
		// Card display 
		card_display_->deactivate();
		card_filename_out_->deactivate();
		keep_bn_->deactivate();
		eqsl_radio_->deactivate();
		card_front_radio_->deactivate();
		card_back_radio_->deactivate();
		fetch_bn_->deactivate();
		log_card_bn_->deactivate();
		// Query question
		question_out_->deactivate();
		// QSL message
		qsl_message_in_->deactivate();
		swl_message_in_->deactivate();
		modify_message_bn_->deactivate();
		// Quick QSL entry
		quick_grp_->activate();
		// Import Query 
		find_bn_->deactivate();
		previous_bn_->deactivate();
		next_bn_->deactivate();
		// Set spare buttons - entering QSO mode: Save and Restart, Save, Cancel
		edit1_bn_->label("Restart");
		edit1_bn_->color(fl_lighter(FL_BLUE));
		edit1_bn_->tooltip("Save QSO and start a new QSO");
		edit1_bn_->callback(cb_bn_edit, (long)RESTART);
		edit1_bn_->activate();
		edit2_bn_->label("Save");
		edit2_bn_->color(FL_GREEN);
		edit2_bn_->tooltip("Save QSO");
		edit2_bn_->callback(cb_bn_edit, (long)SAVE);
		edit2_bn_->activate();
		edit3_bn_->label("Cancel");
		edit3_bn_->color(fl_lighter(FL_RED));
		edit3_bn_->tooltip("Cancel QSO entry");
		edit3_bn_->callback(cb_bn_edit, (long)CANCEL);
		edit3_bn_->activate();
		edit4_bn_->label("4");
		edit4_bn_->color(FL_BACKGROUND_COLOR);
		edit4_bn_->tooltip("");
		edit4_bn_->callback(cb_bn_edit, (long)NONE);
		edit4_bn_->deactivate();
		break;
	case UM_QUERY:
		// We are comparing two records. Enable widgets we need to merge the two.
		// Card display - NB activated below in certain circumstances 
		card_display_->deactivate();
		card_filename_out_->deactivate();
		keep_bn_->deactivate();
		eqsl_radio_->deactivate();
		card_front_radio_->deactivate();
		card_back_radio_->deactivate();
		fetch_bn_->deactivate();
		log_card_bn_->deactivate();
		// Query question
		question_out_->activate();
		// QSL message
		qsl_message_in_->deactivate();
		swl_message_in_->deactivate();
		modify_message_bn_->deactivate();
		// Quick QSL entry
		quick_grp_->deactivate();
		// Enable find if no log record on view
		if (record_1_ == nullptr) {
			find_bn_->activate();
		}
		else {
			find_bn_->deactivate();
		}
		// Enable Show Next only if not last entry in log
		if ((unsigned)item_num_1_ < my_book_->size() - 1) {
			next_bn_->activate();
		}
		else {
			next_bn_->deactivate();
		}
		// Enable Show Previous only if not first record in log
		if ((unsigned)item_num_1_ > 1) {
			previous_bn_->activate();
		}
		else {
			previous_bn_->deactivate();
		}
		// Allocate spare buttons: Query: Reject, Merge, Add
		edit1_bn_->label("Reject");
		edit1_bn_->color(fl_lighter(FL_RED));
		edit1_bn_->tooltip("Reject import query record");
		edit1_bn_->callback(cb_bn_edit, (long)REJECT);
		edit1_bn_->activate();
		edit2_bn_->label("Merge");
		edit2_bn_->color(fl_darker(FL_YELLOW));
		edit2_bn_->tooltip("Merge log and import query records");
		edit2_bn_->callback(cb_bn_edit, (long)MERGE);
		if (record_1_) {
			edit2_bn_->activate();
		}
		else {
			edit2_bn_->deactivate();
		}
		edit3_bn_->label("Add");
		edit3_bn_->color(FL_GREEN);
		edit3_bn_->tooltip("Add query record to log");
		edit3_bn_->callback(cb_bn_edit, (long)ADD);
		edit3_bn_->activate();
		// If this record is a WSJT-X mode and we don't have a possible match set button 4 to look inn ALL.txt
		if (record_2_ && 
			(record_2_->item("MODE") == "JT65" || record_2_->item("MODE") == "JT9" || record_2_->item("MODE") == "FT8" || record_2_->item("MODE") == "FT4")) {
			// Need to activate display
			card_display_->activate();
			card_filename_out_->activate();
			edit4_bn_->label("Find text");
			edit4_bn_->color(FL_YELLOW);
			edit4_bn_->tooltip("Find record in WSJT-X ALL.txt file");
			if (enable_all_search_) {
				edit4_bn_->callback(cb_bn_edit, (long)FIND_WSJTX);
				edit4_bn_->activate();
			}
			else {
				edit4_bn_->callback(cb_bn_edit, (long)NONE);
				edit4_bn_->deactivate();
			}
		}
		else {
			edit4_bn_->label("4");
			edit4_bn_->color(FL_BACKGROUND_COLOR);
			edit4_bn_->tooltip("");
			edit4_bn_->callback(cb_bn_edit, (long)NONE);
			edit4_bn_->deactivate();
		}
		break;
	case UM_DUPEQUERY:
		// Card display 
		card_display_->deactivate();
		card_filename_out_->deactivate();
		keep_bn_->deactivate();
		eqsl_radio_->deactivate();
		card_front_radio_->deactivate();
		card_back_radio_->deactivate();
		fetch_bn_->deactivate();
		log_card_bn_->deactivate();
		// Query question
		question_out_->activate();
		// QSL message
		qsl_message_in_->deactivate();
		swl_message_in_->deactivate();
		modify_message_bn_->deactivate();
		// Quick QSL entry
		quick_grp_->deactivate();
		// NAvigation group
		find_bn_->deactivate();
		previous_bn_->deactivate();
		next_bn_->deactivate();
		// Allocate spare buttons: Duplicate query - Keep First, Merge, Keep Second, Keep Both
		edit1_bn_->label("Keep Log");
		edit1_bn_->color(fl_lighter(FL_GREEN));
		edit1_bn_->tooltip("Keep first record and delete second");
		edit1_bn_->callback(cb_bn_edit, (long)KEEP_LOG);
		edit1_bn_->activate();
		edit2_bn_->label("Merge");
		edit2_bn_->color(fl_darker(FL_YELLOW));
		edit2_bn_->tooltip("Merge first and second records");
		edit2_bn_->callback(cb_bn_edit, (long)MERGE_DUPE);
		edit2_bn_->activate();
		edit3_bn_->label("Keep Query");
		edit3_bn_->color(fl_lighter(FL_GREEN));
		edit3_bn_->tooltip("Keep second record and delete first");
		edit3_bn_->callback(cb_bn_edit, (long)KEEP_DUPE);
		edit3_bn_->activate();
		edit4_bn_->label("Keep both");
		edit4_bn_->color(FL_GREEN);
		edit4_bn_->tooltip("Keep both records");
		edit4_bn_->callback(cb_bn_edit, (long)KEEP_BOTH);
		edit4_bn_->activate();
		break;
	case UM_MERGEDETAILS:
		// Card display 
		card_display_->deactivate();
		card_filename_out_->deactivate();
		keep_bn_->deactivate();
		eqsl_radio_->deactivate();
		card_front_radio_->deactivate();
		card_back_radio_->deactivate();
		fetch_bn_->deactivate();
		log_card_bn_->deactivate();
		// Query question
		question_out_->activate();
		// QSL message
		qsl_message_in_->deactivate();
		swl_message_in_->deactivate();
		modify_message_bn_->deactivate();
		// Quick QSL entry
		quick_grp_->deactivate();
		// NAvigation group
		find_bn_->deactivate();
		previous_bn_->deactivate();
		next_bn_->deactivate();
		// Allocate spare buttons: Merge done
		edit1_bn_->label("Done");
		edit1_bn_->color(fl_lighter(FL_GREEN));
		edit1_bn_->tooltip("Completed merging data");
		edit1_bn_->callback(cb_bn_edit, (long)MERGE_DONE);
		edit1_bn_->activate();
		edit2_bn_->label("2");
		edit2_bn_->color(FL_BACKGROUND_COLOR);
		edit2_bn_->tooltip("");
		edit2_bn_->deactivate();
		edit3_bn_->label("3");
		edit3_bn_->color(FL_BACKGROUND_COLOR);
		edit3_bn_->tooltip("");
		edit3_bn_->deactivate();
		edit4_bn_->label("4");
		edit4_bn_->color(FL_BACKGROUND_COLOR);
		edit4_bn_->tooltip("");
		edit4_bn_->deactivate();
		break;
	}
	// Use button - from EDIT
	if (modifying_) {
		use_bn_->activate();
		clear_bn_->activate();
	}
	else {
		use_bn_->deactivate();
		clear_bn_->deactivate();
	}
}

// Set the selected image to that provided
void record_form::set_selected_image(image_t value) {
	selected_image_ = value;
}

// Set either the text value or enumeration value control
void record_form::set_edit_widgets(string field, string text) {
	// Get the field item
	const Fl_Menu_Item* item = field_choice_->find_item(field.c_str());
	if (item == nullptr) {
		// If it doesn't exist try all fields
		display_all_fields_ = true;
		((field_choice*)field_choice_)->repopulate(true, field);
		all_fields_bn_->value(true);
		item = field_choice_->find_item(field.c_str());
		if (item == nullptr) {
			// The field we are trying to access is not valid, program error?
			status_->misc_status(ST_FATAL, string("LOG: " + field + " is not a valid field - application error").c_str());
			field_choice_->value(0);
		}
		field_choice_->value(field_choice_->find_index(item));
	}
	else {
		// else get selected
		field_choice_->value(field_choice_->find_index(item));
	}

	// Set the data in the edit info controls
	value_in_->buffer()->text(text.c_str());

	// Set the enumeration control - first see if the field is an enumeration
	is_enumeration_ = false;
	// Get the enumeration value, modify by DXCC code or Mode if necessary
	string enumeration_type = spec_data_->enumeration_name(field, record_1_);
	if (enumeration_type.length() > 0) {
		// Populate the enumeartion choice if we are an enumeration
		set_enum_choice(enumeration_type, text);
	}
	else {
		// Enable the text value and disable the enum value controls
		enum_choice_->deactivate();
		value_in_->activate();
	}
}

// Populate the enum choice with valid values for that enumeration and select the 
// specific value
void record_form::set_enum_choice(string enumeration_type, string text) {
	current_enum_type_ = enumeration_type;
	// Set enumeration, disable test and enable enum value controls
	is_enumeration_ = true;
	enum_choice_->activate();
	// delete existing menu
	enum_choice_->clear();

	// Get all the enumeration values and populate drop-down list
	spec_dataset* dataset = spec_data_->dataset(enumeration_type);
	int enum_index = 0;
	auto it = dataset->data.begin();
	char prev = 0;
	// Add null entry to list
	enum_choice_->add("", 0, (Fl_Callback*)nullptr);
	// Look at each enumeration values
	while (it != dataset->data.end()) {
		// Get first letter of the value
		char curr = it->first[0];
		int index;
		if (curr == prev) {
			// Add the enumeration value to the menu
			index = enum_choice_->add(it->first.c_str(), 0, (Fl_Callback*)nullptr);
			prev = curr;
		}
		else {
			// Add the enumeration value with its initial letter as a short-cut
			index = enum_choice_->add(it->first.c_str(), it->first[0], (Fl_Callback*)nullptr);
		}
		// 
		if (it->first == text) enum_index = index;
		it++;
	}
	// If the data is not in the drop-down list set it to first entry
	enum_choice_->value(enum_index);

	// If text is "" then we need to explain the first entry on the list, otherwise use text
	if (text == "") {
		explain_enum(dataset, dataset->data.begin()->first);
	}
	else {
		explain_enum(dataset, text);
	}

}

// add text explaining the enumeration value in the value_in_ widget
void record_form::explain_enum(spec_dataset* dataset, string enum_value) {
	string enum_text = spec_data_->describe_enumeration(dataset, enum_value);
	// Put the explanation in the text edit field and disallow editing
	value_in_->buffer()->text(enum_text.c_str());
	value_in_->deactivate();

}

// Create a text display for the found records in all.txt and return the equivalent record
bool record_form::parse_all_txt(record* record) {
	Fl_Preferences datapath_settings(settings_, "Datapath");
	char* temp;
	datapath_settings.get("WSJT-X", temp, "");
	string filename = string(temp) + "/all.txt";
	ifstream* all_file = new ifstream(filename.c_str());
	// This will take a while so display the timer cursor
	fl_cursor(FL_CURSOR_WAIT);
	// calculate the file size and initialise the progress bar
	streampos startpos = all_file->tellg();
	all_file->seekg(0, ios::end);
	streampos endpos = all_file->tellg();
	long file_size = (long)(endpos - startpos);
	status_->misc_status(ST_NOTE, "LOG: Starting to parse all.txt");
	status_->progress(file_size, OT_RECORD, "bytes");
	// reposition back to beginning
	all_file->seekg(0, ios::beg);
	bool start_copying = false;
	bool stop_copying = false;
	// Get search items from record
	string my_call = record->item("STATION_CALLSIGN", true, true);
	string their_call = record->item("CALL");
	string datestamp = record->item("QSO_DATE").substr(2);
	string timestamp = record->item("TIME_ON");
	string mode = record->item("MODE");
	// Mark QSO incomplete 
	record->item("QSO_COMPLETE", string("N"));
	// Draw the text buffer
	selected_image_ = QI_TEXT;
	draw_image();
	Fl_Text_Buffer* buffer = new Fl_Text_Buffer;
	text_display_->buffer(buffer);
	int count = 0;
	// Now read the file - search for the QSO start time
	while (all_file->good() && !stop_copying) {
		string line;
		getline(*all_file, line);
		count += line.length() + 1;
		status_->progress(count, OT_RECORD);

		// Does the line contain sought date, time, both calls and "Tx" or "Transmitting"
		if (line.substr(0, 6) == datestamp &&
			line.substr(7, 6) == timestamp &&
			(line.find("Transmitting") != string::npos || line.find("Tx")) &&
			line.find(my_call) != string::npos &&
			line.find(their_call) != string::npos &&
			line.find(mode) != string::npos) {
			start_copying = true;
		}
		if (start_copying) {
			if (line.find(my_call) != string::npos &&
				line.find(their_call) != string::npos) {
				// It has both calls - copy to buffer, and parse for QSO details (report, grid and QSO completion
				buffer->append(line.c_str());
				buffer->append("\n");
				copy_all_txt(line, record);
			}
			else if (line.find(my_call) == string::npos &&
				line.find(their_call) == string::npos) {
				// It has neither call - ignore
			}
			else {
				// It has one or the other call - indicates QSO complete
				stop_copying = true;
			}
		}
	}
	if (stop_copying) {
		status_->progress("Found record!", OT_RECORD);
		// If we are complete then say so
		if (record->item("QSO_COMPLETE") != "N" && record->item("QSO_COMPLETE") != "?") {
			all_file->close();
			return true;
		}
	}
	all_file->close();
	char message[100];
	snprintf(message, 100, "LOG: Cannot find contact with %s in WSJT-X text.all file.", their_call.c_str());
	status_->progress("Did not find record", OT_RECORD);
	status_->misc_status(ST_WARNING, message);
	return false;
}

// Copy the text line back to the record - look to see whether it's transmit or receive and then 
void record_form::copy_all_txt(string text, record* record) {
	bool tx_record;
	// After this initial processing pos will point to the start og the QSO decode string - look for old-style transmit record
	size_t pos = text.find("Transmitting");
	if (pos != string::npos) {
		pos = text.find(record->item("MODE"));
		pos += record->item("MODE").length() + 3;
		tx_record = true;
		// Nothing else to get from this record
	}
	else {
		// Now see if it's a new-style Tx record
		pos = text.find("Tx");
		if (pos != string::npos) {
			// Get frequency of transmission - including audio offset
			string freq = text.substr(14, 9);
			// Replace leading spaces with zeroes
			for (size_t i = 0; freq[i] == ' '; i++) {
				freq[i] = '0';
			}
			string freq_offset = text.substr(43, 4);
			// Replace leading spaces with zeroes
			for (size_t i = 0; freq_offset[i] == ' '; i++) {
				freq_offset[i] = '0';
			}
			double frequency = stod(freq) + (stod(freq_offset) / 1000000.0);
			record->item("FREQ", to_string(frequency));
			pos = 48;
			tx_record = true;
		}
		else {
			// Look for a new-style Rx record
			pos = text.find("Rx");
			if (pos != string::npos) {
				pos = 48;
				tx_record = false;
			}
			else {
				// Default to old-style Rx record
				pos = 24;
				tx_record = false;
			}
		}
	}
	// Now parse the exchange
	vector<string> words;
	zzalib::split_line(text.substr(pos), words, ' ');
	string report = words.back();
	if (report == "RR73" || report == "RRR") {
		// If we've seen the R-00 then mark the QSO complete, otherwise mark in provisional until we see the 73
		if (record->item("QSO_COMPLETE") == "?") {
			record->item("QSO_COMPLETE", string(""));
		}
		else if (record->item("QSO_COMPLETE") == "N") {
			record->item("QSO_COMPLETE", string("?"));
		}
	}
	else if (report == "73") {
		// A 73 definitely indicates QSO compplete
		record->item("QSO_COMPLETE", string(""));
	}
	else if (report[0] == 'R') {
		// The first of the rogers
		if (record->item("QSO_COMPLETE") == "N") {
			record->item("QSO_COMPLETE", string("?"));
		}
		// Update reports if they've not been provided
		if (tx_record && !record->item_exists("RST_SENT")) {
			record->item("RST_SENT", report.substr(1));
		}
		else if (!tx_record && !record->item_exists("RST_RCVD")) {
			record->item("RST_RCVD", report.substr(1));
		}
	}
	else if (!tx_record && !record->item_exists("GRIDSQUARE")) {
			// Update gridsquare if it's not been provided
			record->item("GRIDSQUARE", report);
	}
	else if (report[0] == '-' || (report[0] >= '0' && report[0] <= '9')) {
		// Numeric report
		if (tx_record && !record->item_exists("RST_SENT")) {
			record->item("RST_SENT", report);
		}
		else if (!tx_record && !record->item_exists("RST_RCVD")) {
			record->item("RST_RCVD", report);
		}
	}
}
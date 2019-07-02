#include "search_dialog.h"

#include "calendar.h"
#include "field_choice.h"
#include "prefix.h"
#include "pfx_data.h"
#include "icons.h"
#include "utils.h"
#include "spec_data.h"

#include <set>
#include <string>

#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_RGB_Image.H>

using namespace std;
using namespace zzalog;

extern Fl_Preferences* settings_;
extern spec_data* spec_data_;
extern pfx_data* pfx_data_;

// Constructor
search_dialog::search_dialog() :
	// Default size initially - will be resized
	win_dialog(10, 10, "Search criteria")
	, criteria_(nullptr)
	, from_params_(cal_cb_data_t())
	, to_params_(cal_cb_data_t())

{
	// Widget position - calculations
	const int XG = EDGE;
	const int WBN = WBUTTON;
	const int WCAL = HBUTTON;
	const int WDATE = WSMEDIT;
	const int WCHOICE = 2 * WBN;
	const int HBN = max(HRADIO, HBUTTON);
	const int HBNT = max(HBN, HTEXT);
	// Group 1 - conditions
	// rows 1-2   o radio o radio o radio o radio
	// rows 3     o radio o radio [ text ]
	// row  4     # bn    [ text ]
	const int C11 = XG + GAP;
	const int C12 = C11 + WBN;
	const int C13 = C12 + WBN;
	const int C14 = C13 + WBN;
	const int YG1 = EDGE;
	const int WG1 = max(C14 + WBN + GAP, C12 + WCHOICE + GAP) - XG;
	const int R11 = YG1 + HTEXT;
	const int R12 = R11 + HBN;
	const int R13 = R12 + HBN;
	const int R14 = R13 + HBN + GAP;
	const int HG1 = R14 + HBN + GAP - YG1;
	// Group 2 - Refinement
	const int YG2 = YG1 + HG1 + GAP;
	// row 1 # Date  [From]v [To  ]v
	const int R21 = YG2 + HTEXT;
	const int C211 = XG + GAP;
	const int W211 = WBUTTON;
	const int C212 = C211 + W211 + GAP;
	const int W212 = WDATE;
	const int C213 = C212 + W212;
	const int W213 = WCAL;
	const int C214 = C213 + W213 + GAP;
	const int W214 = WDATE;
	const int C215 = C214 + W214;
	const int W215 = WCAL;
	const int W21 = C215 + W215 + GAP;
	// row 2 [Band v] [Mode v]
	const int R22 = R21 + HBNT + GAP;
	const int C221 = XG + GAP;
	const int W221 = WSMEDIT;
	const int C222 = C221 + W221 + GAP;
	const int W222 = WSMEDIT;
	const int W22 = C222 + W222 + GAP;
	// row 3 - # eQSL # LotW # Card
	const int R23 = R22 + HTEXT + GAP;
	const int C231 = XG + GAP;
	const int W231 = WBUTTON;
	const int C232 = C231 + W231;
	const int W232 = WBUTTON;
	const int C233 = C232 + W232;
	const int W233 = WBUTTON;
	const int W23 = C233 + W233 + GAP;
	const int WG2 = max(W21, max(W22, W23)) - XG;
	const int HG2 = R23 + HBUTTON + GAP - YG2;
	// Group 3
	const int YG3 = YG2 + HG2 + GAP;
	// row 1 - o New o AND o OR
	// row 2 - # negate
	const int R31 = YG3 + HTEXT;
	const int R32 = R31 + HBN + GAP;
	const int C31 = XG + GAP;
	const int C32 = C31 + WBN;
	const int C33 = C32 + WBN;
	const int WG3 = C33 + WBN + GAP - XG;
	const int HG3 = R32 + HBUTTON + GAP - YG3;

	// Ungrouped OK and Cancel buttons
	const int W = XG + max(WG1, max(WG2, WG3)) + EDGE;
	const int YGX = YG3 + HG3 + GAP;
	const int CX2 = W - EDGE - WBUTTON;
	const int CX1 = CX2 - GAP - WBUTTON;
	// Fail message box
	const int CX0 = EDGE;
	const int WX0 = CX1 - CX0 - GAP;

	const int H = YGX + HBUTTON + EDGE;

	// Now set the correct size
	size(W, H);
	// Read initial settings
	load_values();

	Fl_Group* gp1 = new Fl_Group(XG, YG1, WG1, HG1, "Condition");
	gp1->labelsize(FONT_SIZE);
	gp1->box(FL_THIN_DOWN_BOX);
	gp1->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	// Positions for the 10 radio buttons
	const int col1[10] = { C11, C12, C13, C14, C11, C12, C13, C14, C11, C12 };
	const int row1[10] = { R11, R11, R11, R11, R12, R12, R12, R12, R13, R13 };
	// For each condition
	for (int i = 0; i < 10; i++) {
		// Radio - one for each element of the search criterion
		Fl_Radio_Round_Button* bn11 = new Fl_Radio_Round_Button(col1[i], row1[i], WBN, HBN, condition_labels_[i].c_str());
		bn11->labelsize(FONT_SIZE);
		bn11->box(FL_THIN_UP_BOX);
		bn11->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
		bn11->value(criteria_->condition == i);
		bn11->color(fl_lighter(FL_MAGENTA));
		char* temp = new char[128];
		sprintf(temp, "Use %s to search for records", condition_labels_[i].c_str());
		bn11->copy_tooltip(temp);
		delete[] temp;
		condition_params_[i].attribute = (int*)&criteria_->condition;
		bn11->callback(cb_radio, (void*)&condition_params_[i]);
		bn11->when(FL_WHEN_RELEASE);
	}
	// Choice - Field-name choice
	field_choice* ch12 = new field_choice(C13, R13, WCHOICE, HTEXT);
	ch12->repopulate(true, criteria_->field_name);
	ch12->textsize(FONT_SIZE);
	ch12->tooltip("Select field to search on");
	ch12->callback(cb_text<Fl_Choice, string>, (void*)&criteria_->field_name);
	ch12->when(FL_WHEN_RELEASE);
	// Check - Use regular expression mapping
	Fl_Light_Button* bn13 = new Fl_Light_Button(C11, R14, WBUTTON, HBUTTON, "Regex");
	bn13->labelsize(FONT_SIZE);
	bn13->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn13->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->by_regex);
	bn13->color(FL_YELLOW, FL_RED);
	bn13->value(criteria_->by_regex);
	bn13->tooltip("Use regular expression matching");
	bn13->when(FL_WHEN_RELEASE);
	// Input - text to match
	Fl_Input* ip14 = new Fl_Input(C12, R14, WEDIT, HTEXT, "Search text");
	ip14->labelsize(FONT_SIZE);
	ip14->textsize(FONT_SIZE);
	ip14->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip14->callback(cb_value<Fl_Input, string>, (void*)&criteria_->pattern);
	ip14->when(FL_WHEN_CHANGED);
	ip14->value(criteria_->pattern.c_str());
	ip14->tooltip("The expression to match records against");

	gp1->end();

	Fl_Group* gp2 = new Fl_Group(XG, YG2, WG2, HG2, "Refinement");
	gp2->labelsize(FONT_SIZE);
	gp2->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	gp2->box(FL_THIN_DOWN_BOX);

	// Row 1 - button and two date/calendars
	// Check - Limit to date range
	Fl_Light_Button* bn21 = new Fl_Light_Button(C211, R21, W211, HBUTTON, "By date");
	bn21->labelsize(FONT_SIZE);
	bn21->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn21->selection_color(FL_RED);
	bn21->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->by_dates);
	bn21->when(FL_WHEN_RELEASE);
	bn21->value(criteria_->by_dates);
	bn21->tooltip("Use date range to limit records to match");
	// Input - Start date
	Fl_Input* in22 = new Fl_Input(C212, R21, W212, HTEXT, "From:");
	in22->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in22->labelsize(FONT_SIZE);
	in22->textsize(FONT_SIZE);
	in22->value(criteria_->from_date.c_str());
	in22->callback(cb_value<Fl_Input, string>, &criteria_->from_date);
	in22->when(FL_WHEN_CHANGED);
	in22->tooltip("Select date to start matching records");
	// Button - open calendar widget to get date
	Fl_Button* bn23 = new Fl_Button(C213, R21, W213, HBUTTON);
	bn23->image(new Fl_RGB_Image(ICON_CALENDAR, 16, 16, 4));
	from_params_ = { &criteria_->from_date, in22 };
	bn23->callback(calendar::cb_cal_open, &from_params_);
	bn23->when(FL_WHEN_RELEASE);
	bn23->tooltip("Opens calendar to select start date");
	// Input - End date
	Fl_Input* in24 = new Fl_Input(C214, R21, W214, HTEXT, "To:");
	in24->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in24->labelsize(FONT_SIZE);
	in24->textsize(FONT_SIZE);
	in24->value(criteria_->to_date.c_str());
	in24->callback(cb_value<Fl_Input, string>, &criteria_->to_date);
	in24->when(FL_WHEN_CHANGED);
	in24->tooltip("Select date to end matching records");
	// Button - open calendar widget to get date
	Fl_Button* bn25 = new Fl_Button(C215, R21, W215, HBUTTON);
	bn25->image(new Fl_RGB_Image(ICON_CALENDAR, 16, 16, 4));
	to_params_ = { &criteria_->to_date, in24 };
	bn25->callback(calendar::cb_cal_open, &to_params_);
	bn25->when(FL_WHEN_RELEASE);
	bn25->tooltip("Opens calendar to select end date");

	// Row 2 - band and mode
	// Choice - Band to limit search to
	Fl_Choice* ch26 = new Fl_Choice(C221, R22, W221, HTEXT, "Band");
	ch26->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ch26->labelsize(FONT_SIZE);
	ch26->textsize(FONT_SIZE);
	// Get the list of bands from ADIF specification
	spec_dataset* bands = spec_data_->dataset("Band");
	// Start with "Any"
	ch26->add("Any");
	if (criteria_->band == "Any") {
		ch26->value(0);
	}
	int ix = 1;
	// Append all the bands in the dataset
	for (auto it = bands->data.begin(); it != bands->data.end(); it++, ix++) {
		ch26->add(it->first.c_str());
		if (it->first == criteria_->band) {
			ch26->value(ix);
		}
	}
	ch26->callback(cb_text<Fl_Choice, string>, (void*)&criteria_->band);
	ch26->when(FL_WHEN_RELEASE);
	ch26->tooltip("Select the band for matching records");
	// Choice - Mode to limit search to
	Fl_Choice* ch27 = new Fl_Choice(C222, R22, W222, HTEXT, "Mode");
	ch27->labelsize(FONT_SIZE);
	ch27->textsize(FONT_SIZE);
	ch27->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	// Get the list of modes from the ADIF specification
	spec_dataset* modes = spec_data_->dataset("Mode");
	// Start with "Any"
	ch27->add("Any");
	if (criteria_->mode == "Any") {
		ch27->value(0);
	}
	ix = 1;
	// Append all the modes in the data set
	for (auto it = modes->data.begin(); it != modes->data.end(); it++, ix++) {
		ch27->add(it->first.c_str());
		if (it->first == criteria_->mode) {
			ch27->value(ix);
		}
	}
	ch27->callback(cb_text<Fl_Choice, string>, (void*)&criteria_->mode);
	ch27->when(FL_WHEN_RELEASE);
	ch27->tooltip("Select the mode for matching records");

	// Row 3 - confirmed
	// Check - Restrict to records confirmed by eQSL
	Fl_Light_Button* bn28 = new Fl_Light_Button(C231, R23, W231, HBUTTON, "eQSL");
	bn28->labelsize(FONT_SIZE);
	bn28->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn28->selection_color(FL_RED);
	bn28->value(criteria_->confirmed_eqsl);
	bn28->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->confirmed_eqsl);
	bn28->when(FL_WHEN_RELEASE);
	bn28->tooltip("Match only when confirmed by eQSL");
	// Check - Restrict to records confirmed by LotW
	Fl_Light_Button* bn29 = new Fl_Light_Button(C232, R23, W232, HBUTTON, "LotW");
	bn29->labelsize(FONT_SIZE);
	bn29->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn29->selection_color(FL_RED);
	bn29->value(criteria_->confirmed_lotw);
	bn29->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->confirmed_lotw);
	bn29->when(FL_WHEN_RELEASE);
	bn29->tooltip("Match only when confirmed by LotW");
	// Check - Restrict to records confirmed by Paper QSL
	Fl_Light_Button* bn210 = new Fl_Light_Button(C233, R23, W233, HBUTTON, "Card");
	bn210->labelsize(FONT_SIZE);
	bn210->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn210->selection_color(FL_RED);
	bn210->value(criteria_->confirmed_card);
	bn210->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->confirmed_card);
	bn210->when(FL_WHEN_RELEASE);
	bn210->tooltip("Match only when confirmed by card");

	gp2->end();

	// Group 3 - Combination
	Fl_Group* gp3 = new Fl_Group(XG, YG3, WG3, HG3, "Combination");
	gp3->labelsize(FONT_SIZE);
	gp3->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	gp3->box(FL_THIN_DOWN_BOX);

	// Row 1 - radio buttons, NEW, AND, OR
	const int col3[3] = { C31, C32, C33 };

	for (int i = 0; i < 3; i++) {
		// Radio - select the combinnation mode: NEW, AND or OR
		Fl_Radio_Round_Button* bn31 = new Fl_Radio_Round_Button(col3[i], R31, WBN, HBN, combination_labels_[i].c_str());
		bn31->labelsize(FONT_SIZE);
		bn31->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
		bn31->box(FL_THIN_UP_BOX);
		bn31->value(criteria_->combi_mode == i);
		combination_params_[i].attribute = (int*)&criteria_->combi_mode;
		bn31->callback(cb_radio, (void*)&combination_params_[i]);
		bn31->when(FL_WHEN_RELEASE);
		switch ((search_combi_t)i) {
		case XM_NEW:
			bn31->tooltip("Create a new condition");
			break;
		case XM_AND:
			bn31->tooltip("Use this condition to further restrict the search");
			break;
		case XM_OR:
			bn31->tooltip("Use this condition to broaden the search");
			break;
		}
	}

	// Row 2 - negate button
	// Check - Extract records that do NOT match the condition
	Fl_Light_Button* bn32 = new Fl_Light_Button(C31, R32, WBN, HBUTTON, "Negate\nresults");
	bn32->labelsize(FONT_SIZE);
	bn32->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn32->selection_color(FL_RED);
	bn32->value(criteria_->negate_results);
	bn32->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->negate_results);
	bn32->when(FL_WHEN_RELEASE);
	bn32->tooltip("Extract records that do not match the condition");

	gp3->end();

	// Box - Fail message
	Fl_Box* box = new Fl_Box(CX0, YGX, WX0, HTEXT);
	box->copy_label("");
	box->labelsize(FONT_SIZE);
	box->box(FL_FLAT_BOX);
	box->labelcolor(FL_BLACK);
	box->color(FL_BACKGROUND_COLOR);
	box->tooltip("Error message displays here");
	fail_box_ = box;
	// Button - OK
	Fl_Button* bn_ok = new Fl_Button(CX1, YGX, WBUTTON, HBUTTON, "OK");
	bn_ok->labelsize(FONT_SIZE);
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	bn_ok->tooltip("Start the search");
	// Button - Cancel
	Fl_Button* bn_cancel = new Fl_Button(CX2, YGX, WBUTTON, HBUTTON, "Cancel");
	bn_cancel->labelsize(FONT_SIZE);
	bn_cancel->callback(cb_bn_cancel);
	bn_cancel->when(FL_WHEN_RELEASE);
	bn_cancel->tooltip("Cancel the search");

	end();
	// Window close button - acts as Cancel
	callback(cb_bn_cancel);
}

// Destructor
search_dialog::~search_dialog()
{
//	delete criteria_;
}

// Load the data from the settings
void search_dialog::load_values() {
	criteria_ = new search_criteria_t;
	criteria_->from_date = "";
	// Get previous criteria from settings
	Fl_Preferences search_settings(settings_, "Search");
	char * temp;
	search_settings.get("By Dates", (int&)criteria_->by_dates, false);
	search_settings.get("By Regular Expression", (int&)criteria_->by_regex, false);
	search_settings.get("Confirmed Card", (int&)criteria_->confirmed_card, false);
	search_settings.get("Confirmed eQSL", (int&)criteria_->confirmed_eqsl, false);
	search_settings.get("Confirmed LotW", (int&)criteria_->confirmed_lotw, false);
	search_settings.get("Negate Results", (int&)criteria_->negate_results, false);
	search_settings.get("From Date", temp, "");
	criteria_->from_date = temp;
	free(temp);
	search_settings.get("To Date", temp, "");
	criteria_->to_date = temp;
	free(temp);
	search_settings.get("Combine Extract", (int&)criteria_->combi_mode, XM_NEW);
	search_settings.get("Criterion", (int&)criteria_->condition, XC_DXCC);
	search_settings.get("Band", temp, "Any");
	criteria_->band = temp;
	free(temp);
	search_settings.get("Condition", temp, "");
	criteria_->pattern = temp;
	free(temp);
	search_settings.get("Field", temp, "ADDRESS");
	criteria_->field_name = temp;
	free(temp);
	search_settings.get("Mode", temp, "Any");
	criteria_->mode = temp;
	free(temp);
}

// Save the data to the settings
void search_dialog::save_values() {
	// Convert to upper-case
	switch (criteria_->condition) {
	case XC_DXCC:
		// special case - DXCC and pattern not numeric
		criteria_->pattern = to_upper(criteria_->pattern);
		if (!criteria_->by_regex) {
			string::size_type dummy;
			int dxcc_id;
			// Match by DXCC - either code number of nickname (default prefix)
			try {
				dxcc_id = stoi(criteria_->pattern, &dummy);
			}
			catch (const invalid_argument&) {
				// exception raised if first character is non-numeric
				dummy = 0;
			}
			// Check that the whole condition is numeric or not 
			if (dummy != criteria_->pattern.length()) {
				// Some letters so nickname supplied - get the ID
				prefix* prefix = pfx_data_->get_prefix(criteria_->pattern);
				if (prefix != nullptr) {
					criteria_->pattern = to_string(prefix->dxcc_code_);
				}
			}

		}

	case XC_CALL:
	case XC_CONT:
	case XC_SQ2:
	case XC_SQ4:
		// Convert to upper case
		criteria_->pattern = to_upper(criteria_->pattern);
		break;
	}
	// Save criteria in settings
	Fl_Preferences search_settings(settings_, "Search");
	search_settings.set("By Dates", criteria_->by_dates);
	search_settings.set("By Regular Expression", criteria_->by_regex);
	search_settings.set("Confirmed Card", criteria_->confirmed_card);
	search_settings.set("Confirmed eQSL", criteria_->confirmed_eqsl);
	search_settings.set("Confirmed LotW", criteria_->confirmed_lotw);
	search_settings.set("Negate Results", criteria_->negate_results);
	search_settings.set("From Date", criteria_->from_date.c_str());
	search_settings.set("To Date", criteria_->to_date.c_str());
	search_settings.set("Combine Extract", criteria_->combi_mode);
	search_settings.set("Criterion", criteria_->condition);
	search_settings.set("Band", criteria_->band.c_str());
	search_settings.set("Condition", criteria_->pattern.c_str());
	search_settings.set("Field", criteria_->field_name.c_str());
	search_settings.set("Mode", criteria_->mode.c_str());
}

// get the extract criteria
search_criteria_t* search_dialog::criteria() {
	return criteria_;
}

// Add a fail message to the dialog - empty string clears it
void search_dialog::fail(string message) {
	if (message.length()) {
		// Copy message to box and colour the box red.
		fail_box_->copy_label(message.c_str());
		fail_box_->color(fl_lighter(FL_RED));
	}
	else {
		// Write empty string to box and set its colour to the background
		fail_box_->copy_label("");
		fail_box_->color(FL_BACKGROUND_COLOR);
	}
	show();
}

// callback - OK button clicked
void search_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	search_dialog* that = ancestor_view<search_dialog>(w);
	that->save_values();
	// Closes the dialog - returns OK
	that->do_button(BN_OK);
}

// callback - Cancel button clicked
void search_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	search_dialog* that = ancestor_view<search_dialog>(w);
	// Closes the dialog - returns FAIL
	that->do_button(BN_CANCEL);
}

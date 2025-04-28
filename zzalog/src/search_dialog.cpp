#include "search_dialog.h"

#include "calendar_input.h"
#include "field_choice.h"
#include "cty_data.h"
#include "icons.h"
#include "utils.h"
#include "spec_data.h"
#include "band.h"
#include "intl_widgets.h"

#include <set>
#include <string>

#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_RGB_Image.H>

using namespace std;



extern spec_data* spec_data_;
extern cty_data* cty_data_;
extern string VENDOR;
extern string PROGRAM_ID;

// Constructor
search_dialog::search_dialog() :
	// Default size initially - will be resized
	win_dialog(10, 10, "Search criteria")
	, criteria_(nullptr)

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
	// rows 4     o radio o radio o radio o radio o radio o radio o radio
	// row 5     [ text ]
	const int C11 = XG + GAP;
	const int C12 = C11 + WBN;
	const int C13 = C12 + WBN;
	const int C14 = C13 + WBN;
	const int YG1 = EDGE;
	const int WG1 = max(C14 + WBN + GAP, C12 + WCHOICE + GAP) - XG;
	const int R11 = YG1 + HTEXT;
	const int R12 = R11 + HBN;
	const int R13 = R12 + HBN;
	const int HG1A = R13 + HBN;
	const int R14 = R13 + HBN + GAP;
	const int HG1B = R14 + HBN;
	const int R15 = R14 + HBN + HTEXT;
	const int HG1 = R15 + HBN + GAP - YG1;
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
	// row 2 [Band v][Mode v][My call v]
	const int R22 = R21 + HBNT + HTEXT;
	const int C221 = XG + GAP;
	const int W221 = WSMEDIT;
	const int C222 = C221 + W221;
	const int W222 = WSMEDIT;
	const int C223 = C222 + W222;
	const int W223 = WSMEDIT;
	const int W22 = C223 + W223;
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
	const int R31 = YG3 + HTEXT;
	const int C31 = XG + GAP;
	const int C32 = C31 + WBN;
	const int C33 = C32 + WBN;
	const int WG3 = C33 + WBN + GAP - XG;
	const int HG3 = R31 + HBUTTON + GAP - YG3;

	// Ungrouped OK and Cancel buttons
	const int W = XG + max(WG1, max(WG2, WG3)) + EDGE;
	const int YGX = YG3 + HG3 + GAP;
	const int CX2 = W - EDGE - WBUTTON;
	const int CX1 = CX2 - GAP - WBUTTON;
	// Fail message box
	const int CX0 = EDGE;
	const int WX0 = CX1 - CX0 - GAP;

	const int H = YGX + HBUTTON + EDGE;

	// now set the correct size
	size(W, H);
	// Read initial settings
	load_values();

	// Group to spcifiy the condition to use as the search criterion
	Fl_Group* gp1 = new Fl_Group(XG, YG1, WG1, HG1, "Match condition");
	gp1->labelsize(FL_NORMAL_SIZE + 2);
	gp1->labelfont(FL_BOLD);
	gp1->box(FL_BORDER_BOX);
	gp1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	// Positions for the 10 radio buttons

	// Group the field selection buttons
	Fl_Group* gp1a = new Fl_Group(XG, YG1, WG1, HG1A);
	gp1a->box(FL_NO_BOX);
	// Set the positions of the buttons
	const int col1[XC_MAXIMUM] = { C11, C12, C13, C14, C11, C12, C13, C14, C11};
	const int row1[XC_MAXIMUM] = { R11, R11, R11, R11, R12, R12, R12, R12, R13};
	// For each condition
	for (int i = 0; i < XC_MAXIMUM; i++) {
		// Radio - one for each element of the search criterion
		Fl_Radio_Round_Button* bn11 = new Fl_Radio_Round_Button(col1[i], row1[i], WBN, HBN, condition_labels_[i].c_str());
		bn11->box(FL_THIN_UP_BOX);
		bn11->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
		bn11->value(criteria_->condition == i);
		char* temp = new char[128];
		sprintf(temp, "Use %s to search for records", condition_labels_[i].c_str());
		bn11->copy_tooltip(temp);
		delete[] temp;
		condition_params_[i].attribute = (int*)&criteria_->condition;
		bn11->callback(cb_bn_condx, (void*)&condition_params_[i]);
		bn11->when(FL_WHEN_RELEASE);
	}
	// Choice - Field-name choice
	field_choice* ch12 = new field_choice(C13, R13, WCHOICE, HTEXT);
	ch12->set_dataset("Fields",criteria_->field_name);
	ch12->tooltip("Select field to search on");
	ch12->callback(cb_ch_field, (void*)&criteria_->field_name);
	ch12->when(FL_WHEN_RELEASE);
	field_name_ = ch12;
	gp1a->end();
	
	// Group the comparsion operators
	Fl_Group* gp1b = new Fl_Group(XG, R14, WG1, HG1B);
	gp1b->box(FL_NO_BOX);
	// Position of the 7 comparator buttons
	const int col2[7] = { C11, (C11 + C12)/2, C12, (C12 + C13)/2, C13, (C13 + C14)/2, C14 };
	const int row2[7] = { R14, R14, R14, R14, R14, R14, R14 };
	// Radio: regex or comparator
	for (int i = 0; i < 7; i++) {
		Fl_Radio_Round_Button* bn13 = new Fl_Radio_Round_Button(col2[i], row2[i], WBN/2, HBN, comparator_labels_[i].c_str());
		bn13->box(FL_THIN_UP_BOX);
		bn13->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
		bn13->value(criteria_->comparator == i);
		char* temp = new char[128];
		sprintf(temp, "Use %s to compare records", comparator_labels_[i].c_str());
		bn13->copy_tooltip(temp);
		delete[] temp;
		comparator_params_[i].attribute = (int*)&criteria_->comparator;
		bn13->callback(cb_radio, (void*)&comparator_params_[i]);
		bn13->when(FL_WHEN_RELEASE);
	}
	gp1b->end();
	// Use field input to offer eneumerated type values plus a any other text
	field_input* ip14 = new field_input(C11, R15
		, WEDIT, HTEXT, "Search text");
	ip14->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	ip14->callback(cb_value<field_input, string>, (void*)&criteria_->pattern);
	ip14->when(FL_WHEN_CHANGED);
	ip14->value(criteria_->pattern.c_str());
	ip14->tooltip("The expression to match records against");
	search_text_ = ip14;

	gp1->end();

	// Refine the search by date, band, mode or station callsign
	Fl_Group* gp2 = new Fl_Group(XG, YG2, WG2, HG2, "Refinement");
	gp2->labelsize(FL_NORMAL_SIZE + 2);
	gp2->labelfont(FL_BOLD);
	gp2->box(FL_BORDER_BOX);
	gp2->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	// Row 1 - button and two date/calendars
	// Check - Limit to date range
	Fl_Light_Button* bn21 = new Fl_Light_Button(C211, R21, W211, HBUTTON, "By date");
	bn21->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn21->selection_color(FL_RED);
	bn21->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->by_dates);
	bn21->when(FL_WHEN_RELEASE);
	bn21->value(criteria_->by_dates);
	bn21->tooltip("Use date range to limit records to match");
	// Input - Start date
	calendar_input* in22 = new calendar_input(C212, R21, W212 + HBUTTON, HTEXT, "From:");
	in22->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in22->value(criteria_->from_date.c_str());
	in22->callback(cb_value<Fl_Input, string>, &criteria_->from_date);
	in22->when(FL_WHEN_CHANGED);
	in22->tooltip("Select date to start matching records");
	// Input - End date
	calendar_input* in24 = new calendar_input(C214, R21, W214 + HBUTTON , HTEXT, "To:");
	in24->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	in24->value(criteria_->to_date.c_str());
	in24->callback(cb_value<Fl_Input, string>, &criteria_->to_date);
	in24->when(FL_WHEN_CHANGED);
	in24->tooltip("Select date to end matching records");

	// Row 2 - band and mode
	// Choice - Band to limit search to
	Fl_Choice* ch26 = new Fl_Choice(C221, R22, W221, HTEXT, "Band");
	ch26->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	// Get the list of bands from ADIF specification
	band_set* bands = spec_data_->bands();
	// Start with "Any"
	ch26->add("Any");
	if (criteria_->band == "Any") {
		ch26->value(0);
	}
	int ix = 1;
	// Append all the bands in the dataset
	for (auto it = bands->begin(); it != bands->end(); it++, ix++) {
		ch26->add((*it).c_str());
		if (*it == criteria_->band) {
			ch26->value(ix);
		}
	}
	ch26->callback(cb_text<Fl_Choice, string>, (void*)&criteria_->band);
	ch26->when(FL_WHEN_RELEASE);
	ch26->tooltip("Select the band for matching records");
	// Choice - Mode to limit search to
	Fl_Choice* ch27 = new Fl_Choice(C222, R22, W222, HTEXT, "Mode");
	ch27->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	// Get the list of modes from the ADIF specification
	spec_dataset* modes = spec_data_->dataset("Combined");
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
	ch27->tooltip("Select the mode for matching records - a mode will match all submodes");
	// Choice - Mode to limit search to
	Fl_Choice* ch27a = new Fl_Choice(C223, R22, W223, HTEXT, "My Call");
	ch27a->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	// Get the list of modes from the ADIF specification
	spec_dataset* callsigns = spec_data_->dataset("Dynamic STATION_CALLSIGN");
	// Start with "Any"
	ch27a->add("Any");
	if (criteria_->mode == "Any") {
		ch27a->value(0);
	}
	ix = 1;
	// Append all the modes in the data set
	for (auto it = callsigns->data.begin(); it != callsigns->data.end(); it++, ix++) {
		ch27a->add(escape_menu(it->first).c_str());
		if (it->first == criteria_->mode) {
			ch27a->value(ix);
		}
	}
	ch27a->callback(cb_text<Fl_Choice, string>, (void*)&criteria_->my_call);
	ch27a->when(FL_WHEN_RELEASE);
	ch27a->tooltip("Select 'STATION_CALLSIGN' for matching records");

	// Row 3 - confirmed
	// Check - Restrict to records confirmed by eQSL
	Fl_Light_Button* bn28 = new Fl_Light_Button(C231, R23, W231, HBUTTON, "eQSL");
	bn28->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn28->selection_color(FL_RED);
	bn28->value(criteria_->confirmed_eqsl);
	bn28->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->confirmed_eqsl);
	bn28->when(FL_WHEN_RELEASE);
	bn28->tooltip("Match only when confirmed by eQSL");
	// Check - Restrict to records confirmed by LotW
	Fl_Light_Button* bn29 = new Fl_Light_Button(C232, R23, W232, HBUTTON, "LotW");
	bn29->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn29->selection_color(FL_RED);
	bn29->value(criteria_->confirmed_lotw);
	bn29->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->confirmed_lotw);
	bn29->when(FL_WHEN_RELEASE);
	bn29->tooltip("Match only when confirmed by LotW");
	// Check - Restrict to records confirmed by Paper QSL
	Fl_Light_Button* bn210 = new Fl_Light_Button(C233, R23, W233, HBUTTON, "Card");
	bn210->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	bn210->selection_color(FL_RED);
	bn210->value(criteria_->confirmed_card);
	bn210->callback(cb_value<Fl_Light_Button, bool>, (void*)&criteria_->confirmed_card);
	bn210->when(FL_WHEN_RELEASE);
	bn210->tooltip("Match only when confirmed by card");

	gp2->end();

	// Group 3 - Combination
	Fl_Group* gp3 = new Fl_Group(XG, YG3, WG3, HG3, "Combination");
	gp3->labelsize(FL_NORMAL_SIZE + 2);
	gp3->labelfont(FL_BOLD);
	gp3->box(FL_BORDER_BOX);
	gp3->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	// Row 1 - radio buttons, NEW, AND, OR
	const int col3[3] = { C31, C32, C33 };

	for (int i = 0; i < 3; i++) {
		// Radio - select the combinnation mode: NEW, AND or OR
		Fl_Radio_Round_Button* bn31 = new Fl_Radio_Round_Button(col3[i], R31, WBN, HBN, combination_labels_[i].c_str());
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

	gp3->end();

	// Box - Fail message
	Fl_Box* box = new Fl_Box(CX0, YGX, WX0, HTEXT);
	box->copy_label("");
	box->box(FL_FLAT_BOX);
	box->tooltip("Error message displays here");
	fail_box_ = box;
	// Button - OK
	Fl_Button* bn_ok = new Fl_Button(CX1, YGX, WBUTTON, HBUTTON, "OK");
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	bn_ok->tooltip("Start the search");
	// Button - Cancel
	Fl_Button* bn_cancel = new Fl_Button(CX2, YGX, WBUTTON, HBUTTON, "Cancel");
	bn_cancel->callback(cb_bn_cancel);
	bn_cancel->when(FL_WHEN_RELEASE);
	bn_cancel->tooltip("Cancel the search");

	end();
	// Window close button - acts as Cancel
	callback(cb_bn_cancel);

	enable_widgets();
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
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences search_settings(settings, "Search");
	char * temp;
	search_settings.get("By Dates", (int&)criteria_->by_dates, false);
	search_settings.get("By Comparison", (int&)criteria_->comparator, XP_EQ);
	search_settings.get("Confirmed Card", (int&)criteria_->confirmed_card, false);
	search_settings.get("Confirmed eQSL", (int&)criteria_->confirmed_eqsl, false);
	search_settings.get("Confirmed LotW", (int&)criteria_->confirmed_lotw, false);
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
		if (!criteria_->comparator != XP_REGEX && criteria_->pattern.length()) {
			string::size_type dummy;
			int dxcc;
			// Match by DXCC - either code number or nickname (default prefix)
			try {
				dxcc = stoi(criteria_->pattern, &dummy);
			}
			catch (const invalid_argument&) {
				// exception raised if first character is non-numeric
				dummy = 0;
			}
			// Check that the whole condition is numeric or not 
			if (dummy != criteria_->pattern.length()) {
				// Some letters so nickname supplied - get the ID
				dxcc = cty_data_->entity(criteria_->pattern);
				criteria_->pattern = to_string(dxcc);
			}

		}

	case XC_CALL:
	case XC_CONT:
	case XC_SQ2:
	case XC_SQ4:
		// Convert to upper case
		criteria_->pattern = to_upper(criteria_->pattern);
		break;
	default:
		break;
	}
	// Save criteria in settings
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences search_settings(settings, "Search");
	search_settings.set("By Dates", criteria_->by_dates);
	search_settings.set("By Comparison", criteria_->comparator);
	search_settings.set("Confirmed Card", criteria_->confirmed_card);
	search_settings.set("Confirmed eQSL", criteria_->confirmed_eqsl);
	search_settings.set("Confirmed LotW", criteria_->confirmed_lotw);
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
		fail_box_->labelcolor(FL_BLACK);
	}
	else {
		// Write empty string to box and set its colour to the background
		fail_box_->copy_label("");
		fail_box_->color(FL_BACKGROUND_COLOR);
		fail_box_->labelcolor(FL_FOREGROUND_COLOR);
	}
	show();
}

// callback - OK button clicked
// v is not used
void search_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	search_dialog* that = ancestor_view<search_dialog>(w);
	that->save_values();
	// Closes the dialog - returns OK
	that->do_button(BN_OK);
}

// callback - Cancel button clicked
// v is not used
void search_dialog::cb_bn_cancel(Fl_Widget* w, void* v) {
	search_dialog* that = ancestor_view<search_dialog>(w);
	// Closes the dialog - returns FAIL
	that->do_button(BN_CANCEL);
}

// call back condition button
void search_dialog::cb_bn_condx(Fl_Widget* w, void* v) {
	search_dialog* that = ancestor_view<search_dialog>(w);
	cb_radio(w, v);
	that->enable_widgets();
}

// Field name choice
void search_dialog::cb_ch_field(Fl_Widget* w, void* v) {
	search_dialog* that = ancestor_view<search_dialog>(w);
	cb_value<field_choice, string>(w, v);
	that->criteria_->pattern = "";
	that->enable_widgets();
}

// Enable widgets
void search_dialog::enable_widgets() {
	switch(criteria_->condition) {
		case XC_FIELD: {
			field_name_->activate();
			((field_input*)search_text_)->field_name(criteria_->field_name.c_str());
			((field_input*)search_text_)->value(criteria_->pattern.c_str());
			break;
		}
		case XC_CONT: {
			field_name_->deactivate();
			((field_input*)search_text_)->field_name("CONT");
			((field_input*)search_text_)->value(criteria_->pattern.c_str());
			break;
		}
		default: {
			field_name_->deactivate();
			((field_input*)search_text_)->field_name("");
			((field_input*)search_text_)->value("");
		}
	}
}
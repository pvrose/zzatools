#include "qso_dxcc.h"
#include "qso_entry.h"
#include "qso_data.h"
#include "cty_data.h"
#include "book.h"
#include "drawing.h"
#include "utils.h"
#include "menu.h"
#include "search.h"
#include "extract_data.h"
#include "tabbed_forms.h"
#include "spec_data.h"

#include <FL/Fl_Toggle_Button.H>

extern cty_data* cty_data_;
extern book* book_;
extern extract_data* extract_records_;
extern book* navigation_book_;
extern tabbed_forms* tabbed_forms_;
extern spec_data* spec_data_;
extern bool DARK;

// Constructor
qso_dxcc::qso_dxcc(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, bands_worked_(nullptr)
	, modes_worked_(nullptr)
	, qso_(nullptr)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	create_form();
	enable_widgets();
	end();
}

// Destructor
qso_dxcc::~qso_dxcc() {

}

// Create the widgets
void qso_dxcc::create_form() {
	int avail_width = w() - GAP - GAP;
	int avail_height = h() - GAP;
	int curr_x = x() + GAP;
	int curr_y = y() + 1;

	// Display callsign
	op_call_ = new Fl_Output(curr_x, curr_y, avail_width - WBUTTON, HBUTTON);
	op_call_->box(FL_FLAT_BOX);
	op_call_->color(FL_BACKGROUND_COLOR);
	op_call_->textfont(FL_BOLD);
	op_call_->textsize(FL_NORMAL_SIZE + 2);
	op_call_->textcolor(DARK ? FL_RED : COLOUR_CLARET);
	curr_x += op_call_->w();

	// QRZ.com button
	bn_qrz_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "QRZ.com");
	bn_qrz_->callback(cb_bn_qrz, nullptr);

	curr_x = x() + GAP;
	curr_y += op_call_->h();

	// Display the DXCC prefix
	op_prefix_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_prefix_->box(FL_FLAT_BOX);
	op_prefix_->color(FL_BACKGROUND_COLOR);
	op_prefix_->textfont(FL_BOLD);
	op_prefix_->textcolor(DARK ? FL_RED : COLOUR_CLARET);
	curr_y += ROW_HEIGHT;

	// Display how the DXCC was found (decoded or from exception file)
	op_source_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_source_->box(FL_FLAT_BOX);
	op_source_->color(FL_BACKGROUND_COLOR);
	curr_y += ROW_HEIGHT;

	// Display geographic information
	op_geo_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_geo_->box(FL_FLAT_BOX);
	op_geo_->color(FL_BACKGROUND_COLOR);
	curr_y += ROW_HEIGHT + HTEXT + HTEXT / 2;

	avail_height -= (curr_y - y());

	// Display worked before status (bands and modes)
	g_wb4_ = new wb4_buttons(curr_x, curr_y, avail_width, avail_height);
	g_wb4_->align(FL_ALIGN_TOP);

	end();
	enable_widgets();

}

// Enable the widgets
void qso_dxcc::enable_widgets() {
	if (callsign_.length()) {
		op_call_->value(callsign_.c_str());
		char text[64];
		switch (dxcc_) {
			case -1:
				strcpy(text, "Invalid DXCC");
				break;
			case 0:
				strcpy(text, "Not in a DXCC");
				break;
			default:
				snprintf(text, sizeof(text), "%s: %s", nickname_.c_str(), name_.c_str());
				break;
		}
		op_prefix_->value(text);
		switch (source_) {
		case cty_data::INVALID:
			op_source_->value("Invalid Operation");
			op_source_->color(FL_RED);
			break;
		case cty_data::EXCEPTION:
			op_source_->value("Entity Exception");
			op_source_->color(FL_BACKGROUND_COLOR);
			break;
		case cty_data::ZONE_EXCEPTION:
			op_source_->value("CQ Zone Exception");
			op_source_->color(FL_BACKGROUND_COLOR);
			break;
		case cty_data::DEFAULT:
			op_source_->value("Default decode");
			op_source_->color(FL_BACKGROUND_COLOR);
			break;
		}
		if (location_.is_nan()) {
			snprintf(text, sizeof(text), "CQ Zone %d.", cq_zone_);
		} else {
			snprintf(text, sizeof(text), "CQ Zone %d. %.0f\302\260%c %.0f\302\260%c",
				cq_zone_, abs(location_.latitude), location_.latitude > 0 ? 'N' : 'S',
				abs(location_.longitude), location_.longitude > 0 ? 'E' : 'W');
		}
		op_geo_->value(text);
	} else {
		op_call_->value("No Contact");
		op_prefix_->value("");
		op_source_->value("");
		op_geo_->value("");
	}
	g_wb4_->enable_widgets();
}

// Set the data - parse the callsign in the current qso
void qso_dxcc::set_data(record* qso) {
	qso_ = qso;
	callsign_ = "";
	bands_worked_ = nullptr;
	modes_worked_ = nullptr;
	if (qso_) {
		callsign_ = qso_->item("CALL");
		// Is a prefix supplied
		nickname_ = cty_data_->nickname(qso_);
		name_ = cty_data_->name(qso_);
		source_ = cty_data_->get_source(qso_);
		cq_zone_ = cty_data_->cq_zone(qso_);
		location_ = cty_data_->location(qso_);
		dxcc_ = cty_data_->entity(qso_);
	}

}

// QRZ.com button clicked
void qso_dxcc::cb_bn_qrz(Fl_Widget* w, void* v) {
	qso_dxcc* that = ancestor_view<qso_dxcc>(w);
	menu::cb_mi_info_qrz(w, (void*)&that->callsign_);
}

// The "worked before" lights
// Inherit from Fl_Scroll as there are too many buttons for the available display
qso_dxcc::wb4_buttons::wb4_buttons(int X, int Y, int W, int H, const char* L) :
	Fl_Scroll(X, Y, W, H, L),
	dxcc_bands_(nullptr),
	dxcc_modes_(nullptr),
	dxcc_submodes_(nullptr),
	all_bands_(nullptr),
	all_modes_(nullptr)
{
	type(VERTICAL_ALWAYS);
	end();
}

// Destructor
qso_dxcc::wb4_buttons::~wb4_buttons() {}

// Create the buttons
void qso_dxcc::wb4_buttons::create_form() {
	const int NUM_WIDE = 4;
	const int AWIDTH = w() - Fl::scrollbar_size();
	const int BWIDTH = AWIDTH / NUM_WIDE;
	int bn_number = 0;
	clear();
	Fl_Group* saveg = Fl_Group::current();
	Fl_Group::current(this);
	int curr_x = x();
	int curr_y = y();
	Fl_Button* b1 = new Fl_Button(curr_x, curr_y, BWIDTH * NUM_WIDE, HBUTTON, "Bands:");
	b1->box(FL_FLAT_BOX);
	b1->align(FL_ALIGN_INSIDE);
	b1->labelcolor(FL_FOREGROUND_COLOR);
	curr_y += HBUTTON;
	// For all bands in the log...
	for (auto it = all_bands_->begin(); it != all_bands_->end(); it++) {
		// ... create a button
		Fl_Toggle_Button* bn = new Fl_Toggle_Button(curr_x, curr_y, BWIDTH, HBUTTON);
		bn->copy_label((*it).c_str());
		bn->callback(cb_bn_band, (void*)(*it).c_str());
		map_[*it] = bn;
		curr_x += BWIDTH;
		bn_number++;
		if (bn_number % NUM_WIDE == 0) {
			curr_x = x();
			curr_y += HBUTTON;
		}
	}
	if (curr_x != x()) {
		curr_x = x();
		curr_y += HBUTTON;
	}

	Fl_Button* b2 = new Fl_Button(curr_x, curr_y, BWIDTH * NUM_WIDE, HBUTTON, "Modes:");
	b2->box(FL_FLAT_BOX);
	b2->align(FL_ALIGN_INSIDE);
	b2->labelcolor(FL_FOREGROUND_COLOR);
	curr_y += HBUTTON;
	bn_number = 0;
	set<string> modes = *all_modes_;
	// Add submodes to the list of modes (all in the log)
	for(auto ix = all_submodes_->begin(); ix != all_submodes_->end(); ix++) {
		modes.insert(*ix);
	}
	// For all modes and submodes in the log...
	for (auto it = modes.begin(); it != modes.end(); it++) {
		// ... add a log
		Fl_Toggle_Button* bn = new Fl_Toggle_Button(curr_x, curr_y, BWIDTH, HBUTTON);
		bn->copy_label((*it).c_str());
		bn->callback(cb_bn_mode, (void*)(*it).c_str());
		map_[*it] = bn;
		curr_x += BWIDTH;
		bn_number++;
		if (bn_number % NUM_WIDE == 0) {
			curr_x = x();
			curr_y += HBUTTON;
		}
	}
	Fl_Group::current(saveg);
}

// Light the buttons that have been worked: 
void qso_dxcc::wb4_buttons::enable_widgets() {
	record* qso = ((qso_dxcc*)parent())->qso_;
	string qso_band = "";
	string qso_mode = "";
	string qso_submode = "";
	all_bands_ = navigation_book_->used_bands();
	all_modes_ = navigation_book_->used_modes();
	all_submodes_ = navigation_book_->used_submodes();
	if (qso) {
		// Get the details for this QSO
		qso_band = qso->item("BAND");
		qso_submode = qso->item("SUBMODE");
		qso_mode = qso->item("MODE");
		int dxcc = cty_data_->entity(qso);
		string my_call = qso->item("STATION_CALLSIGN");
		// Add this QSOs band and mode to the list of logged 
		if (qso_band.length()) all_bands_->insert(qso_band);
		if (qso_mode.length()) all_modes_->insert(qso_mode);
		if (qso_submode.length()) all_modes_->insert(qso_submode);
		// Get the bands and modes worked for this DXCC
		dxcc_bands_ = navigation_book_->used_bands(dxcc, my_call);
		dxcc_modes_ = navigation_book_->used_modes(dxcc, my_call);
		dxcc_submodes_ = navigation_book_->used_submodes(dxcc, my_call);
		string caveat = "";
		if (book_ != navigation_book_) caveat = "\n(Extracted data only)";
		char l[128];
		snprintf(l, sizeof(l), "DXCC worked status as %s%s", my_call.c_str(), caveat.c_str());
		copy_label(l);
	}
	else {
		string caveat = "";
		if (book_ != navigation_book_) caveat = "\n(Extracted data only)";
		char l[128];
		snprintf(l, sizeof(l), "DXCC worked status%s", caveat.c_str());
		copy_label(l);

	}
	// Create all the buttons
	create_form();
	// Reset all the buttons' colouring
	for (int ix = 0; ix < children(); ix++) {
		if (child(ix)->type() == FL_TOGGLE_BUTTON) {
			// Only for the band and mode toggle buttons
			Fl_Toggle_Button* bn = (Fl_Toggle_Button*)child(ix);
			bn->labelcolor(FL_FOREGROUND_COLOR);
			bn->deactivate();
			bn->value(false);
		}
	}
	// Mark this QSO buttons: Band in claret, Mode in dark green
	if (qso && qso->item("CALL").length()) {
		if (qso_band.length()) {
			Fl_Toggle_Button* bn = (Fl_Toggle_Button*)map_.at(qso_band);
			bn->activate();
			bn->labelcolor(FL_FOREGROUND_COLOR);
			bn->labelfont(FL_BOLD);
			bn->value(true);
		}
		if (qso_mode.length()) {
			Fl_Toggle_Button* bn = (Fl_Toggle_Button*)map_.at(qso_mode);
			bn->activate();
			bn->labelcolor(FL_FOREGROUND_COLOR);
			bn->labelfont(FL_BOLD);
			bn->value(true);
		}
		if (qso_submode.length()) {
			Fl_Toggle_Button* bn = (Fl_Toggle_Button*)map_.at(qso_submode);
			bn->activate();
			bn->labelcolor(FL_FOREGROUND_COLOR);
			bn->labelfont(FL_BOLD);
			bn->value(true);
		}
		// Set bands worked for this DXCC in mauve - this will overwrite this
		// QSO's if it has been worked before
		if (dxcc_bands_) {
			for (auto ix = dxcc_bands_->begin(); ix != dxcc_bands_->end(); ix++) {
				Fl_Toggle_Button * bn = (Fl_Toggle_Button*)map_.at(*ix);
				bn->activate();
				bn->labelcolor(FL_FOREGROUND_COLOR);
				bn->labelfont(0);
			}
		}
		// Set modes worked for this DXCC in apple green - this will overwrite this
		// QSO's if it has been worked before
		if (dxcc_modes_) {
			for (auto ix = dxcc_modes_->begin(); ix != dxcc_modes_->end(); ix++) {
				Fl_Toggle_Button* bn = (Fl_Toggle_Button*)map_.at(*ix);
				bn->activate();
				bn->labelcolor(FL_FOREGROUND_COLOR);
				bn->labelfont(0);
			}
		}
		if (dxcc_submodes_) {
			for (auto ix = dxcc_submodes_->begin(); ix != dxcc_submodes_->end(); ix++) {
				Fl_Toggle_Button* bn = (Fl_Toggle_Button*)map_.at(*ix);
				bn->activate();
				bn->labelcolor(FL_FOREGROUND_COLOR);
				bn->labelfont(0);
			}
		}
	}
}

// Get records that match nickname, station and pressed button
void qso_dxcc::wb4_buttons::cb_bn_mode(Fl_Widget* w, void* v) {
	qso_dxcc* qd = ancestor_view<qso_dxcc>(w);
	qso_data* qdd = ancestor_view<qso_data>(qd);
	record* qso = qd->qso_;
	Fl_Button* bn = (Fl_Button*)w;
	string mode = string(bn->label());
	bool save = bn->value();
	// Extract those records not sent to QSL server !(*QSL_SENT==Y) 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_DXCC,
		/*search_comp_t comparator*/ XP_EQ,
		/*bool by_dates*/ false,
		/*string from_date*/"",
		/*string to_date;*/"",
		/*string band;*/ "Any",
		/*string mode;*/ mode,
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*string field_name; */ "",
		/*string pattern;*/ qd->nickname_,
		/*string my_call*/ qso->item("STATION_CALLSIGN")
	};
	extract_records_->criteria(new_criteria);
	tabbed_forms_->activate_pane(OT_EXTRACT, true);
	item_num_t item = navigation_book_->item_number(qdd->current_number());
	navigation_book_->selection(item, HT_EXTRACTION);
	bn->value(!save);
}

// Get records that match nickname, station and pressed button
void qso_dxcc::wb4_buttons::cb_bn_band(Fl_Widget* w, void* v) {
	qso_dxcc* qd = ancestor_view<qso_dxcc>(w);
	qso_data* qdd = ancestor_view<qso_data>(qd);
	record* qso = qd->qso_;
	Fl_Button* bn = (Fl_Button*)w;
	string band = string((char*)v);
	bool save = bn->value();
	// Extract those records not sent to QSL server !(*QSL_SENT==Y) 
	search_criteria_t	new_criteria = {
		/*search_cond_t condition*/ XC_DXCC,
		/*search_comp_t comparator*/ XP_EQ,
		/*bool by_dates*/ false,
		/*string from_date*/"",
		/*string to_date;*/"",
		/*string band;*/ band,
		/*string mode;*/ "Any",
		/*bool confirmed_eqsl;*/ false,
		/*bool confirmed_lotw;*/ false,
		/*bool confirmed_card;*/ false,
		/*search_combi_t combi_mode;*/ XM_NEW,
		/*string field_name; */ "",
		/*string pattern;*/ qd->nickname_,
		/*string my_call*/ qso->item("STATION_CALLSIGN")
	};
	extract_records_->criteria(new_criteria);
	tabbed_forms_->activate_pane(OT_EXTRACT, true);
	item_num_t item = navigation_book_->item_number(qdd->current_number());
	navigation_book_->selection(item, HT_EXTRACTION);
	bn->value(!save);
}

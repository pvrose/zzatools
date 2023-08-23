#include "qso_dxcc.h"
#include "qso_entry.h"
#include "cty_data.h"
#include "book.h"
#include "drawing.h"
#include "utils.h"
#include "menu.h"
#include "search.h"
#include "extract_data.h"
#include "tabbed_forms.h"

extern cty_data* cty_data_;
extern book* book_;
extern extract_data* extract_records_;
extern book* navigation_book_;
extern tabbed_forms* tabbed_forms_;

qso_dxcc::qso_dxcc(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, bands_worked_(nullptr)
	, modes_worked_(nullptr)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	create_form();
	enable_widgets();
	end();
}

qso_dxcc::~qso_dxcc() {

}

void qso_dxcc::create_form() {
	int avail_width = w() - GAP - GAP;
	int avail_height = h() - GAP;
	int curr_x = x() + GAP;
	int curr_y = y() + 1;

	// "Title" is callsign
	op_call_ = new Fl_Output(curr_x, curr_y, avail_width - WBUTTON, HBUTTON);
	op_call_->box(FL_FLAT_BOX);
	op_call_->color(FL_BACKGROUND_COLOR);
	op_call_->textfont(FL_BOLD);
	op_call_->textsize(FL_NORMAL_SIZE + 2);
	op_call_->textcolor(COLOUR_CLARET);
	curr_x += op_call_->w();

	// QRZ.com button
	bn_qrz_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "QRZ.com");
	bn_qrz_->callback(cb_bn_qrz, nullptr);

	curr_x = x() + GAP;
	curr_y += op_call_->h();

	op_prefix_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_prefix_->box(FL_FLAT_BOX);
	op_prefix_->color(FL_BACKGROUND_COLOR);
	op_prefix_->textfont(FL_BOLD);
	op_prefix_->textcolor(COLOUR_CLARET);
	curr_y += ROW_HEIGHT;

	op_source_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_source_->box(FL_FLAT_BOX);
	op_source_->color(FL_BACKGROUND_COLOR);
	curr_y += ROW_HEIGHT;

	op_geo_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_geo_->box(FL_FLAT_BOX);
	op_geo_->color(FL_BACKGROUND_COLOR);
	curr_y += ROW_HEIGHT + HTEXT;

	avail_height -= (curr_y - y());

	g_wb4_ = new wb4_buttons(curr_x, curr_y, avail_width, avail_height, "DXCC worked before");
	g_wb4_->align(FL_ALIGN_TOP);

	end();
	enable_widgets();

}

// Enable the widgets
void qso_dxcc::enable_widgets() {
	if (callsign_.length()) {
		op_call_->value(callsign_.c_str());
		char text[64];
		snprintf(text, sizeof(text), "%s: %s", nickname_.c_str(), name_.c_str());
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
		snprintf(text, sizeof(text), u8"CQ Zone %d. %.0f°%c %.0f°%c",
			cq_zone_, abs(location_.latitude), location_.latitude > 0 ? 'N' : 'S',
			abs(location_.longitude), location_.longitude > 0 ? 'E' : 'W');
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
void qso_dxcc::set_data() {
	qso_entry* qe = ancestor_view<qso_entry>(this);
	record* qso = qe->qso();
	callsign_ = "";
	bands_worked_ = nullptr;
	modes_worked_ = nullptr;
	if (qso) {
		callsign_ = qso->item("CALL");
		// Is a prefix supplied
		nickname_ = cty_data_->nickname(qso);
		name_ = cty_data_->name(qso);
		source_ = cty_data_->get_source(qso);
		cq_zone_ = cty_data_->cq_zone(qso);
		location_ = cty_data_->location(qso);
		int dxcc = cty_data_->entity(qso);;
	}

}

// QRZ.com button clicked
void qso_dxcc::cb_bn_qrz(Fl_Widget* w, void* v) {
	qso_dxcc* that = ancestor_view<qso_dxcc>(w);
	menu::cb_mi_info_qrz(w, (void*)&that->callsign_);
}

// The "worked before" lights
qso_dxcc::wb4_buttons::wb4_buttons(int X, int Y, int W, int H, const char* L) :
	Fl_Scroll(X, Y, W, H, L),
	dxcc_bands_(nullptr),
	dxcc_modes_(nullptr),
	all_bands_(nullptr),
	all_modes_(nullptr)
{
	end();
}

qso_dxcc::wb4_buttons::~wb4_buttons() {}

void qso_dxcc::wb4_buttons::create_form() {
	const int NUM_WIDE = 5;
	const int BWIDTH = w() / NUM_WIDE;
	int bn_number = 0;
	clear();
	begin();
	int curr_x = x();
	int curr_y = y();
	for (auto it = all_bands_->begin(); it != all_bands_->end(); it++) {
		Fl_Button* bn = new Fl_Button(curr_x, curr_y, BWIDTH, HBUTTON);
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
	bn_number = 0;
	for (auto it = all_modes_->begin(); it != all_modes_->end(); it++) {
		Fl_Button* bn = new Fl_Button(curr_x, curr_y, BWIDTH, HBUTTON);
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
	end();
}

// Light the buttons that have been worked: 
void qso_dxcc::wb4_buttons::enable_widgets() {
	qso_entry* qe = ancestor_view<qso_entry>(this);
	record* qso = qe->qso();
	string qso_band = "";
	string qso_mode = "";
	all_bands_ = book_->used_bands();
	all_modes_ = book_->used_submodes();
	if (qso) {
		qso_band = qso->item("BAND");
		qso_mode = qso->item("MODE", false, true);
		int dxcc;
		qso->item("DXCC", dxcc);
		string my_call = qso->item("STATION_CALLSIGN");
		if (qso_band.length()) all_bands_->insert(qso_band);
		if (qso_mode.length()) all_modes_->insert(qso_mode);
		dxcc_bands_ = book_->used_bands(dxcc, my_call);
		dxcc_modes_ = book_->used_submodes(dxcc, my_call);
		char l[128];
		snprintf(l, sizeof(l), "DXCC worked before as %s", my_call.c_str());
		copy_label(l);
	}
	// Create all the buttons
	create_form();
	// Clear all widgets
	for (int ix = 0; ix < children(); ix++) {
		Fl_Button* bn = (Fl_Button*)child(ix);
		bn->color(FL_BACKGROUND_COLOR);
		bn->labelcolor(fl_inactive(FL_BLACK));
	}
	// Mark QSO buttons
	if (qso_band.length()) {
		map_.at(qso_band)->color(COLOUR_ORANGE);
		map_.at(qso_band)->labelcolor(FL_BLACK);
	}
	if (qso_mode.length()) {
		map_.at(qso_mode)->color(COLOUR_ORANGE);
		map_.at(qso_mode)->labelcolor(FL_BLACK);
	}
	// Set band colours - CLARET this QSO, MAUVE other QSOs
	if (dxcc_bands_) {
		for (auto ix = dxcc_bands_->begin(); ix != dxcc_bands_->end(); ix++) {
			Fl_Button* bn = (Fl_Button*)map_.at(*ix);
			if ((*ix) == qso_band) {
				bn->color(COLOUR_CLARET);
			}
			else {
				bn->color(COLOUR_MAUVE);
			}
			bn->labelcolor(fl_contrast(FL_BLACK, bn->color()));
		}
	}
	// Set mode colours - CALRET this QSO, APPLE other QSOs
	if (dxcc_modes_) {
		for (auto ix = dxcc_modes_->begin(); ix != dxcc_modes_->end(); ix++) {
			Fl_Button* bn = (Fl_Button*)map_.at(*ix);
			if ((*ix) == qso_mode) {
				bn->color(FL_DARK_GREEN);
			}
			else {
				bn->color(COLOUR_APPLE);
			}
			bn->labelcolor(fl_contrast(FL_BLACK, bn->color()));
		}
	}
}

// Get records that match nickname, station and pressed button
void qso_dxcc::wb4_buttons::cb_bn_mode(Fl_Widget* w, void* v) {
	qso_dxcc* qd = ancestor_view<qso_dxcc>(w);
	qso_entry* qe = ancestor_view<qso_entry>(qd);
	record* qso = qe->qso();
	string mode = string((char*)v);
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
	item_num_t item = navigation_book_->item_number(qe->qso_number());
	navigation_book_->selection(item, HT_EXTRACTION);
}

// Get records that match nickname, station and pressed button
void qso_dxcc::wb4_buttons::cb_bn_band(Fl_Widget* w, void* v) {
	qso_dxcc* qd = ancestor_view<qso_dxcc>(w);
	qso_entry* qe = ancestor_view<qso_entry>(qd);
	record* qso = qe->qso();
	string band = string((char*)v);
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
	item_num_t item = navigation_book_->item_number(qe->qso_number());
	navigation_book_->selection(item, HT_EXTRACTION);
}

#include "qso_dxcc.h"
#include "qso_entry.h"
#include "qso_data.h"
#include "cty_data.h"
#include "book.h"
#include "record.h"
#include "drawing.h"
#include "utils.h"
#include "menu.h"
#include "search.h"
#include "extract_data.h"
#include "tabbed_forms.h"
#include "spec_data.h"
#include "callback.h"

#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Scroll.H>

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
	tooltip("Displays the current worked before status for the DXCC");
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
	op_call_->tooltip("Current callsign of interest");
	curr_x += op_call_->w();

	// QRZ.com button
	bn_qrz_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "QRZ.com");
	bn_qrz_->callback(cb_bn_qrz, nullptr);
	bn_qrz_->tooltip("Look up call in QRZ.com");

	curr_y += bn_qrz_->h();

	curr_x = x() + GAP;
	curr_y = op_call_->y() + op_call_->h();

	// Display the DXCC prefix
	op_prefix_ = new Fl_Output(curr_x, curr_y, avail_width - WBUTTON, ROW_HEIGHT);
	op_prefix_->box(FL_FLAT_BOX);
	op_prefix_->color(FL_BACKGROUND_COLOR);
	op_prefix_->textfont(FL_BOLD);
	op_prefix_->tooltip("Shows the DXCC of the call of interest");
	curr_y += ROW_HEIGHT;

	// Display how the DXCC was found (decoded or from exception file)
	op_source_ = new Fl_Output(curr_x, curr_y, avail_width - WBUTTON, ROW_HEIGHT);
	op_source_->box(FL_FLAT_BOX);
	op_source_->color(FL_BACKGROUND_COLOR);
	op_source_->tooltip("Shows how the DXCC was obtained - decoded or an exception");
	curr_y += ROW_HEIGHT;

	// Display geographic information
	op_geo_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_geo_->box(FL_FLAT_BOX);
	op_geo_->color(FL_BACKGROUND_COLOR);
	op_geo_->tooltip("Shows geographical data for the DXCC");
	curr_y += ROW_HEIGHT;

	// Display geographic information
	op_dist_bear_ = new Fl_Output(curr_x, curr_y, avail_width, ROW_HEIGHT);
	op_dist_bear_->box(FL_FLAT_BOX);
	op_dist_bear_->color(FL_BACKGROUND_COLOR);
	op_dist_bear_->tooltip("Shows the distance and bearing of the DXCC - centre or specified grid");
	curr_y += ROW_HEIGHT + HTEXT;

	avail_height -= (curr_y - y());

	// Display worked before status (bands and modes)
	g_wb4_ = new wb4_table(curr_x, curr_y, avail_width, avail_height, "Worked Before?");
	//g_wb4_ = new wb4_buttons(curr_x, curr_y, avail_width, avail_height);
	g_wb4_->align(FL_ALIGN_TOP);
	g_wb4_->box(FL_FLAT_BOX);
	g_wb4_->tooltip("Shows the worked before status: any, specific band or mode");

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
			char ls[10];
			switch(loc_source_) {
				case LOC_PREFIX:
					strcpy(ls, "(PFX)");
					break;
				case LOC_NONE:
					strcpy(ls, "(N/A)");
					break;
				case LOC_LATLONG:
					strcpy(ls, "(QSO)");
					break;
				default:
					strcpy(ls, "(GRID)");
					break;
			}
			snprintf(text, sizeof(text), "%s: CQ Zone %d. Loc: %.0f\302\260%c %.0f\302\260%c %s",
				continent_.c_str(), cq_zone_, 
				abs(location_.latitude), location_.latitude > 0 ? 'N' : 'S',
				abs(location_.longitude), location_.longitude > 0 ? 'E' : 'W',
				ls);
		}
		op_geo_->value(text);
		if (location_.is_nan() || my_location_.is_nan()) {
			snprintf(text, sizeof(text), "Distance N/A");
		} else {
			double bearing;
			double distance;
			// Calculate bearing and distance
			great_circle(my_location_, location_, bearing, distance);
			snprintf(text, sizeof(text), "Distance %0.fkm, Bearing %0.f\302\260",
				distance, bearing);
		}
		op_dist_bear_->value(text);
	} else {
		op_call_->value("No contact");
		op_prefix_->value("Prefix N/A");
		op_source_->value("Decode N/A");
		op_geo_->value("Location N/A");
		op_dist_bear_->value("Distance N/A");
	}
}

// Set the data - parse the callsign in the current qso
void qso_dxcc::set_data(record* qso) {
	qso_ = qso;
	callsign_ = "";
	bands_worked_ = nullptr;
	modes_worked_ = nullptr;
	if (qso_) {
		callsign_ = qso_->item("CALL");
		station_ = qso_->item("STATION_CALLSIGN");
		// Is a prefix supplied
		nickname_ = cty_data_->nickname(qso_);
		name_ = cty_data_->name(qso_);
		source_ = cty_data_->get_source(qso_);
		cq_zone_ = cty_data_->cq_zone(qso_);
		location_ = qso_->location(false, loc_source_);
		dxcc_ = cty_data_->entity(qso_);
		my_location_ = qso_->location(true);
		continent_ = cty_data_->continent(qso_);
		g_wb4_->set_data();
	}

}

// QRZ.com button clicked
void qso_dxcc::cb_bn_qrz(Fl_Widget* w, void* v) {
	qso_dxcc* that = ancestor_view<qso_dxcc>(w);
	menu::cb_mi_info_qrz(w, (void*)&that->callsign_);
}

qso_dxcc::wb4_table::wb4_table(int X, int Y, int W, int H, const char* L) :
	Fl_Table(X, Y, W, H, L) 
{
	rows(4);
	cols(3);
	col_header(true);
	row_header(true);
	fl_font(0, FL_NORMAL_SIZE);
	row_height_all(HBUTTON);
	col_width_all(w() / (cols() + 1));
	row_header_width(col_width(0));
	col_header_height(HBUTTON);
}

qso_dxcc::wb4_table::~wb4_table() {

}

// inherited from Fl_Table
void qso_dxcc::wb4_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	worked_t cat = (worked_t)(R + 1);
	qso_dxcc* qd = ancestor_view<qso_dxcc>(this);
	switch (context) {
	case CONTEXT_ROW_HEADER:
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X, Y, W, H);
		fl_color(FL_FOREGROUND_COLOR);
		fl_yxline(X, Y, Y + H - 1, X + W);
		if (wkd_matrix_.at(cat).text.length()) {
			fl_draw(wkd_matrix_.at(cat).text.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
		}
		else {
			fl_font(FL_ITALIC, FL_NORMAL_SIZE);
			switch (cat) {
			case WK_DXCC:
				fl_draw("DXCC", X, Y, W, H, FL_ALIGN_CENTER);
				break;
			case WK_GRID4:
				fl_draw("Grid", X, Y, W, H, FL_ALIGN_CENTER);
				break;
			case WK_CQZ:
				fl_draw("CQ Zone", X, Y, W, H, FL_ALIGN_CENTER);
				break;
			case WK_CONT:
				fl_draw("Continent", X, Y, W, H, FL_ALIGN_CENTER);
				break;
			default:
				break;
			}
			fl_font(0, FL_NORMAL_SIZE);
		}
		break;
	case CONTEXT_COL_HEADER:
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X, Y, W, H);
		fl_color(FL_FOREGROUND_COLOR);
		fl_yxline(X, Y, Y + H - 1, X + W);
		fl_line(X, Y, X + W, Y);
		switch (C) {
		case 0:
			fl_draw("ANY", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 1:
			fl_draw("BAND", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 2:
			fl_draw("MODE", X, Y, W, H, FL_ALIGN_CENTER);
			fl_line(X + W - 1, Y, X + W - 1, Y + H - 1);
			break;
		}
		break;
	case CONTEXT_CELL: {
		bool new_entity;
		wkd_line& w = wkd_matrix_.at(cat);
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X, Y, W, H);
		fl_color(FL_FOREGROUND_COLOR);
		fl_yxline(X, Y, Y + H - 1, X + W);
		switch (C) {
		case 0:
			new_entity = w.any;
			break;
		case 1:
			new_entity = w.band;
			break;
		case 2:
			new_entity = w.mode;
			fl_line(X + W - 1, Y, X + W - 1, Y + H - 1);
			break;
		}
		if (w.text.length()) {
			if (new_entity) {
				fl_font(FL_BOLD, FL_NORMAL_SIZE);
				fl_draw("NEW", X, Y, W, H, FL_ALIGN_CENTER);
				fl_font(0, FL_NORMAL_SIZE);
			}
			else {
				fl_draw("\342\234\224", X, Y, W, H, FL_ALIGN_CENTER);
			}
		}
		else {
			fl_draw("?", X, Y, W, H, FL_ALIGN_CENTER);
		}
		break;
	}
	case CONTEXT_ENDPAGE:
	{
		int X1 = Fl_Table::wix;
		int Y1 = Fl_Table::wiy;
		int W1 = row_header_width();
		int H1 = col_header_height();
		fl_color(FL_BACKGROUND_COLOR);
		fl_rectf(X1, Y1, W1, H1);
		fl_color(FL_FOREGROUND_COLOR);
		fl_yxline(X1, Y1, Y1 + H1 - 1, X1 + W1);
		fl_line(X1, Y1, X1 + W1, Y1);
		fl_draw(qd->station_.c_str(), X1, Y1, W1, H1, FL_ALIGN_CENTER);
		// Also draw top and left lines
		break;
	}
	default:
		break;
	}
}

// Getthe data for the table
void qso_dxcc::wb4_table::set_data() {
	qso_dxcc* qd = ancestor_view<qso_dxcc>(this);

	double freq;
	qd->qso_->item("FREQ", freq);
	string band = spec_data_->band_for_freq(freq);
	string mode = qd->qso_->item("MODE");
	string grid = qd->qso_->item("GRIDSQUARE").substr(0, 4);
	band_set* dxcc_b = book_->used_bands(WK_DXCC, qd->dxcc_, qd->station_);
	set<string>* dxcc_m = book_->used_modes(WK_DXCC, qd->dxcc_, qd->station_);
	if (dxcc_b) {
		wkd_matrix_[WK_DXCC].any = (bool)dxcc_b->size() == 0;
		wkd_matrix_[WK_DXCC].band = dxcc_b->find(band) == dxcc_b->end();
	}
	else {
		wkd_matrix_[WK_DXCC].any = true;
		wkd_matrix_[WK_DXCC].band = true;
	}
	if (dxcc_m) {
		wkd_matrix_[WK_DXCC].mode = dxcc_m->find(mode) == dxcc_m->end();
	}
	else {
		wkd_matrix_[WK_DXCC].mode = true;
	}
	wkd_matrix_[WK_DXCC].text = qd->nickname_;

	band_set* grid_b = book_->used_bands(WK_GRID4, grid, qd->station_);
	set<string>* grid_m = book_->used_modes(WK_GRID4, grid, qd->station_);
	if (grid_b) {
		wkd_matrix_[WK_GRID4].any = (bool)grid_b->size() == 0;
		wkd_matrix_[WK_GRID4].band = grid_b->find(band) == grid_b->end();
	}
	else {
		wkd_matrix_[WK_GRID4].any = true;
		wkd_matrix_[WK_GRID4].band = true;
	}
	if (grid_m) {
		wkd_matrix_[WK_GRID4].mode = grid_m->find(mode) == grid_m->end();
	}
	else {
		wkd_matrix_[WK_GRID4].mode = true;
	}
	wkd_matrix_[WK_GRID4].text = grid;

	band_set* cqz_b = book_->used_bands(WK_CQZ, qd->cq_zone_, qd->station_);
	set<string>* cqz_m = book_->used_modes(WK_CQZ, qd->cq_zone_, qd->station_);
	if (cqz_b) {
		wkd_matrix_[WK_CQZ].any = (bool)cqz_b->size() == 0;
		wkd_matrix_[WK_CQZ].band = cqz_b->find(band) == cqz_b->end();
	}
	else {
		wkd_matrix_[WK_CQZ].any = true;
		wkd_matrix_[WK_CQZ].band = true;
	}
	if (cqz_m) {
		wkd_matrix_[WK_CQZ].mode = cqz_m->find(mode) == cqz_m->end();
	}
	else {
		wkd_matrix_[WK_CQZ].mode = true;
	}
	wkd_matrix_[WK_CQZ].text = qd->cq_zone_ > 0 ? "CQZ " + to_string(qd->cq_zone_) : "";

	band_set* cont_b = book_->used_bands(WK_CONT, qd->continent_, qd->station_);
	set<string>* cont_m = book_->used_modes(WK_CONT, qd->continent_, qd->station_);
	if (cont_b) {
		wkd_matrix_[WK_CONT].any = (bool)cont_b->size() == 0;
		wkd_matrix_[WK_CONT].band = cont_b->find(band) == cont_b->end();
	}
	else {
		wkd_matrix_[WK_CONT].any = true;
		wkd_matrix_[WK_CONT].band = true;
	}
	if (cont_m) {
		wkd_matrix_[WK_CONT].mode = cont_m->find(mode) == cont_m->end();
	}
	else {
		wkd_matrix_[WK_CONT].mode = true;
	}
	wkd_matrix_[WK_CONT].text = qd->continent_;

}


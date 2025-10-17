#include "init_dialog.h"

#include "stn_data.h"
#include "stn_dialog.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Radio_Round_Button.H>

extern stn_data* stn_data_;
extern stn_dialog* stn_dialog_;

//! Constructor - sizes and labels itself
init_dialog::init_dialog(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	create_form();
	load_values();
}


//! Destructor
init_dialog::~init_dialog() {
}

//! Instantiate the component widgets
void init_dialog::create_form() {
	int cx = x() + GAP + WLABEL;
	int cy = y() + GAP;
	const int cw = WLABEL + WSMEDIT;

	Fl_Group* g_radio = new Fl_Group(cx, cy, WBUTTON, 2 * HBUTTON);
	g_radio->box(FL_FLAT_BOX);

	bn_club_ = new Fl_Radio_Round_Button(cx, cy, HBUTTON, HBUTTON, "Club");
	bn_club_->align(FL_ALIGN_LEFT);
	bn_club_->callback(cb_type, (void*)(intptr_t)CLUB);
	bn_club_->tooltip("Select this if the log is for a club station");
	
	cy += HBUTTON;
	bn_indiv_ = new Fl_Radio_Round_Button(cx, cy, HBUTTON, HBUTTON, "Individual");
	bn_indiv_->align(FL_ALIGN_LEFT);
	bn_indiv_->callback(cb_type, (void*)(intptr_t)INDIVIDUAL);
	bn_indiv_->tooltip("Selecet this if teh log is for an individual licensee");

	g_radio->end();
	cy += HBUTTON;
	ip_call_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Callsign");
	ip_call_->align(FL_ALIGN_LEFT);
	ip_call_->tooltip("Enter the station callsign");

	cy += HBUTTON;
	ip_club_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Club name");
	ip_club_->align(FL_ALIGN_LEFT);
	ip_club_->tooltip("Input the club name");

	cy += HBUTTON + GAP;
	ip_location_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Location");
	ip_location_->align(FL_ALIGN_LEFT);
	ip_location_->tooltip("Enter the default location");
	ip_location_->value("Main station");

	cy += HBUTTON;
	ip_name_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Operator");
	ip_name_->align(FL_ALIGN_LEFT);
	ip_name_->tooltip("Enter the default operator's name");

	cy += HBUTTON + GAP;
	cx = GAP + cw - WBUTTON;
	bn_accept_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Accept");
	bn_accept_->callback(cb_accept, nullptr);
	bn_accept_->tooltip("Click to accept the entered data");

	int ch = cy + HBUTTON + GAP;
	resizable(nullptr);
	size(GAP + cw + GAP, ch);

	end();
	show();
}

//! Copy from station_defauts_

void init_dialog::load_values() {
	stn_default defaults = stn_data_->defaults();
	if (defaults.type == NOT_USED) {
		bn_club_->value(false);
		bn_indiv_->value(false);
		ip_club_->value("");
		ip_call_->value("");
		ip_location_->value("Main station");
		ip_name_->value("");
	}
	else {
		switch (defaults.type) {
		case CLUB:
			bn_club_->value(true);
			bn_indiv_->value(false);
			break;
		case INDIVIDUAL:
			bn_club_->value(false);
			bn_indiv_->value(true);
			break;
		default:
			break;
		}
		ip_call_->value(defaults.callsign.c_str());
		ip_location_->value(defaults.location.c_str());
		ip_name_->value(defaults.name.c_str());
		ip_club_->value(defaults.club_name.c_str());
	}
}


//! Callback when "Accept" button clicked
void init_dialog::cb_accept(Fl_Widget* w, void* v) {
	init_dialog* that = ancestor_view<init_dialog>(w);
	stn_default defaults = stn_data_->defaults();
	defaults.callsign = to_upper(that->ip_call_->value());
	defaults.location = that->ip_location_->value();
	defaults.name = that->ip_name_->value();
	defaults.club_name = that->ip_club_->value();
	stn_data_->set_defaults(defaults);
	stn_dialog* dlg = ancestor_view<stn_dialog>(that);
	dlg->enable_widgets();
	dlg->set_tab(stn_dialog::QTH, defaults.location, "Set initial values.");
}

//! Callback when radio button is selected

//! \param w: widget clicked
//! \param v: object of type stn_type indicates the new station type
void init_dialog::cb_type(Fl_Widget* w, void* v) {
	init_dialog* that = ancestor_view<init_dialog>(w);
	stn_data_->set_type((stn_type)(intptr_t)v);
	that->enable_widgets();
}

//! SAvevalues for club use
void init_dialog::enable_widgets() {
	stn_default defaults = stn_data_->defaults();
	switch (defaults.type) {
	case NOT_USED:
		bn_club_->value(false);
		bn_indiv_->value(false);
		ip_club_->deactivate();
		ip_club_->value("");
		ip_call_->deactivate();
		ip_call_->value("");
		ip_name_->deactivate();
		ip_name_->value("");
		ip_location_->deactivate();
		ip_location_->value("");
		break;
	case CLUB:
		bn_club_->value(true);
		bn_indiv_->value(false);
		ip_club_->activate();
		ip_club_->value(defaults.club_name.c_str());
		ip_call_->activate();
		ip_call_->value(defaults.callsign.c_str());
		ip_name_->deactivate();
		ip_name_->value("");
		ip_location_->activate();
		ip_location_->value(defaults.location.c_str());
		break;
	case INDIVIDUAL:
		bn_club_->value(false);
		bn_indiv_->value(true);
		ip_club_->deactivate();
		ip_club_->value("");
		ip_call_->activate();
		ip_call_->value(defaults.callsign.c_str());
		ip_name_->activate();
		ip_name_->value(defaults.name.c_str());
		ip_location_->activate();
		ip_location_->value(defaults.location.c_str());
		break;
	}
}
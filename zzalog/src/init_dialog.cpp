#include "init_dialog.h"

#include "stn_data.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Radio_Round_Button.H>

extern stn_default station_defaults_;
extern std::string PROGRAM_ID;
extern std::string VENDOR;

//! Constructor - sizes and labels itself
init_dialog::init_dialog() : Fl_Double_Window(100, 100, "New installation")
{
	create_form();
	load_values();
}


//! Destructor
init_dialog::~init_dialog() {
	save_values();
}

//! Instantiate the component widgets
void init_dialog::create_form() {
	int cx = GAP + WLABEL;
	int cy = GAP;
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
	ip_location_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Location");
	ip_location_->align(FL_ALIGN_LEFT);
	ip_location_->tooltip("Enter a description of the location");
	ip_location_->value("Main station");

	cy += HBUTTON;
	ip_grid_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Gridsquare");
	ip_grid_->align(FL_ALIGN_LEFT);
	ip_grid_->tooltip("Enter the Maidenhead gridsquare locator");

	cy += HBUTTON;
	ip_name_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Name");
	ip_name_->align(FL_ALIGN_LEFT);
	ip_name_->tooltip("Enter the opretaor's ot the club's name");

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
	if (station_defaults_.type == NOT_USED) {
		bn_club_->value(false);
		bn_indiv_->value(false);
		ip_call_->value("");
		ip_location_->value("Main station");
		ip_grid_->value("");
		ip_name_->value("");
	}
	else {
		switch (station_defaults_.type) {
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
		ip_call_->value(station_defaults_.callsign.c_str());
		ip_location_->value(station_defaults_.location.c_str());
		ip_grid_->value(station_defaults_.grid.c_str());
		ip_name_->value(station_defaults_.name.c_str());
	}
}

//! Returns the station details
stn_default init_dialog::get_default() {
	return *default_station_;
}

//! Callback when "Accept" button clicked
void init_dialog::cb_accept(Fl_Widget* w, void* v) {
	init_dialog* that = ancestor_view<init_dialog>(w);
	that->default_station_ = new stn_default;
	that->default_station_->type = that->station_type_;
	that->default_station_->callsign = to_upper(that->ip_call_->value());
	that->default_station_->location = that->ip_location_->value();
	that->default_station_->grid = to_upper(that->ip_grid_->value());
	that->default_station_->name = that->ip_name_->value();
	that->hide();
	that->save_values();
}

//! Callback when radio button is selected

//! \param w: widget clicked
//! \param v: object of type stn_type indicates the new station type
void init_dialog::cb_type(Fl_Widget* w, void* v) {
	init_dialog* that = ancestor_view<init_dialog>(w);
	that->station_type_ = (stn_type)(intptr_t)v;
}

//! SAvevalues for club use
void init_dialog::save_values() {

}
#include "club_stn_dlg.h"

#include "config.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "stn_data.h"
#include "stn_dialog.h"

#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Preferences.H>

extern config* config_;
extern qso_manager* qso_manager_;
extern stn_data* stn_data_;
extern bool new_installation_;
extern stn_default station_defaults_;
extern std::string PROGRAM_ID;
extern std::string VENDOR;

club_stn_dlg::club_stn_dlg() :
	win_dialog(640, 480, "Club log-in") {
	load_data();
	create_form();	
	enable_widgets();
}

club_stn_dlg::~club_stn_dlg() {
	store_data();
}

void club_stn_dlg::create_form() {

	int curr_x = GAP;
	int curr_y = GAP;

	curr_x += WLABEL;
	w_club_name_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON, "Club name");
	w_club_name_->value(club_name_.c_str());
	w_club_name_->callback(cb_value<Fl_Input, std::string>, &club_name_);
	w_club_name_->tooltip("Please enter the name of the club");

	curr_y += HBUTTON;
	w_club_call_ = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Callsign");
	w_club_call_->value(club_call_.c_str());
	w_club_call_->callback(cb_value<Fl_Input, std::string>, &club_call_);
	w_club_call_->tooltip("Please enter the club callsign");

	curr_y += HBUTTON;
	w_club_location_ = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Location");
	w_club_location_->value(club_location_.c_str());
	w_club_location_->callback(cb_value<Fl_Input, std::string>, &club_location_);
	w_club_location_->tooltip("Please enter the club location");

	int save_x = curr_x;
	curr_x += WBUTTON + GAP;
	w_edit_qth_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Details");
	w_edit_qth_->callback(cb_edit_qth);
	w_edit_qth_->tooltip("Opens a dialog to edit QTH details");

	curr_x = save_x;
	curr_y += HBUTTON + GAP;
	w_operator_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Operator");
	w_operator_->callback(cb_operator);
	w_operator_->tooltip("Select your callsign from the drop-down std::list (or type in new");
	populate_login();

	curr_y += HBUTTON;
	w_name_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Name");
	w_name_->tooltip("Please enter your name");

	curr_y += HBUTTON;
	w_call_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Callsign");
	w_call_->tooltip("Please enter your callsign");

	curr_y += HBUTTON + GAP;
	curr_x += WEDIT + GAP;

	w_login_ = new Fl_Button(curr_x - WBUTTON - GAP, curr_y, WBUTTON, HBUTTON, "Login");
	w_login_->callback(cb_bn_login);
	w_login_->tooltip("Start ZZALOG as selected operator");
	curr_y += HBUTTON + GAP;

	resizable(nullptr);
	size(curr_x, curr_y);

	end();
	show();

}

// Read data from settings
void club_stn_dlg::load_data() {
	// Get club details from settings
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences station_settings(settings, "Station");
	char* temp;
	station_settings.get("Club Name", temp, "");
	club_name_ = temp;
	free(temp);
	station_settings.get("Callsign", temp, "");
	club_call_ = to_upper(std::string(temp));
	free(temp);
	if (club_call_.length() == 0) {
		// No call in seettings - use default from log
		club_call_ = station_defaults_.callsign;
	}
	station_settings.get("Location", temp, "");
	club_location_ = temp;
	free(temp);
	station_settings.get("Operator", temp, "");
	nickname_ = temp;
	free(temp);
}

// Enable widgets
void club_stn_dlg::enable_widgets() {
	// If we expect to change them 
	if (club_name_.length() == 0 || new_installation_) {
		w_club_name_->type(FL_NORMAL_INPUT);
		w_club_name_->color(FL_BACKGROUND2_COLOR);
	}
	else {
		w_club_name_->type(FL_NORMAL_OUTPUT);
		w_club_name_->color(FL_BACKGROUND_COLOR);
	}
	if (club_call_.length() == 0 || new_installation_) {
		w_club_call_->type(FL_NORMAL_INPUT);
		w_club_call_->color(FL_BACKGROUND2_COLOR);
	}
	else {
		w_club_call_->type(FL_NORMAL_OUTPUT);
		w_club_call_->color(FL_BACKGROUND_COLOR);
	}
	if (club_location_.length() == 0 || new_installation_) {
		w_club_location_->type(FL_NORMAL_INPUT);
		w_club_location_->color(FL_BACKGROUND2_COLOR);
	}
	else {
		w_club_location_->type(FL_NORMAL_OUTPUT);
		w_club_location_->color(FL_BACKGROUND_COLOR);
	}
	const oper_info_t* info = stn_data_->get_oper(nickname_);
	w_operator_->value(nickname_.c_str());
	if (info) {
		if (info->data.find(NAME) == info->data.end()) {
			w_name_->value("");
		}
		else {
			w_name_->value(info->data.at(NAME).c_str());
		}
		if (info->data.find(CALLSIGN) == info->data.end()) {
			w_call_->value("");
		}
		else {
			w_call_->value(info->data.at(CALLSIGN).c_str());
		}
	}
	else {
		w_name_->value("");
		w_call_->value("");
	}
	redraw();
}


void club_stn_dlg::add_login() {
	if (!stn_data_->known_oper(nickname_)) {
		oper_info_t* info = new oper_info_t;
		stn_data_->add_oper(nickname_, info);
	}
	stn_data_->add_oper_item(nickname_, NAME, std::string(w_name_->value()));
	stn_data_->add_oper_item(nickname_, CALLSIGN, std::string(w_call_->value()));
}

void club_stn_dlg::add_callsign() {
	if (!stn_data_->known_call(club_call_)) {
		stn_data_->add_call(club_call_);
	}
}

void club_stn_dlg::store_data() {
	// Get club details from settings
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences station_settings(settings, "Station");
	station_settings.set("Club Name", club_name_.c_str());
	station_settings.set("Callsign", club_call_.c_str());
	station_settings.set("Location", club_location_.c_str());
	station_settings.set("Operator", nickname_.c_str());
}

void club_stn_dlg::cb_bn_login(Fl_Widget* w, void* v) {
	club_stn_dlg* that = ancestor_view<club_stn_dlg>(w);
	that->club_call_ = to_upper(that->club_call_);
	that->add_callsign();
	that->add_login();
	that->store_data();
	qso_manager_->data()->update_station_choices();
	that->default_callback(that, v);
}

void club_stn_dlg::cb_operator(Fl_Widget* w, void* v) {
	club_stn_dlg* that = ancestor_view<club_stn_dlg>(w);
	that->nickname_ = std::string((char*)((Fl_Input_Choice*)w)->value());
	that->enable_widgets();
}

void club_stn_dlg::cb_edit_qth(Fl_Widget* w, void* v) {
	club_stn_dlg* that = ancestor_view<club_stn_dlg>(w);
	config_->show();
	stn_dialog* dlg = (stn_dialog*)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::QTH, that->club_location_);
}

void club_stn_dlg::populate_login() {
	const std::map<std::string, oper_info_t*>* opers = stn_data_->get_opers();
	w_operator_->add("");
	for (auto it : *opers)
	{
		w_operator_->add(it.first.c_str());
	}
}

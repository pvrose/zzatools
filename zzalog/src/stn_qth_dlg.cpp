#include "stn_qth_dlg.h"

#include "book.h"
#include "cty_data.h"
#include "qso_manager.h"
#include "record.h"
#include "status.h"
#include "stn_data.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Output.H>

extern book* navigation_book_;
extern cty_data* cty_data_;
extern qso_manager* qso_manager_;
extern status* status_;
extern stn_data* stn_data_;

const std::string LABELS[] = {"Strret", "City", "Postcode", "Locator", "Country", "DXCC",
	"Prim'y Sub", "Sec'y Sub", "CQ Zone", "ITU Zone", "Continent", "IOTA", "WAB" };


//! Constructor 

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
stn_qth_widget::stn_qth_widget(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, "")
{
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	create_form();
}

//! Destructor
stn_qth_widget::~stn_qth_widget() {
}

//! Instantiate the component widgets
void stn_qth_widget::create_form() {
	box(FL_BORDER_BOX);
	const int WIDGETS_PER_ROW = 4;
	const int WWIDGET = (w() / WIDGETS_PER_ROW) - WLABEL;
	int cx = x() + WLABEL;
	int cy = y();

	cx += WWIDGET + WLABEL;
	// Description input
	ip_descr_ = new Fl_Input(cx, cy, (WIDGETS_PER_ROW - 1) * WWIDGET + (WIDGETS_PER_ROW * 2) * WLABEL, ROW_HEIGHT, "Descr'n");
	ip_descr_->align(FL_ALIGN_LEFT);
	ip_descr_->callback(cb_ip_data, (void*)(intptr_t)DESCRIPTION);
	ip_descr_->box(FL_BORDER_BOX);
	ip_descr_->tooltip("Please enter the description of the location");

	cx = x() + WLABEL;
	cy += ROW_HEIGHT;
	char tip[128];
	// Inputs for all other members of qth_info_t
	int ix = 0;
	for (qth_value_t it = (qth_value_t)0; it != DESCRIPTION; it = (qth_value_t)(it + 1)) {
		Fl_Input* ip = new Fl_Input(cx, cy, WWIDGET, ROW_HEIGHT);
		ip_data_[it] = ip;
		ip->box(FL_BORDER_BOX);
		ip->copy_label(LABELS[ix].c_str());
		ip->align(FL_ALIGN_LEFT);
		ip->callback(cb_ip_data, (void*)(intptr_t)it);
		// Stt tooltip
		snprintf(tip, sizeof(tip), "Please enter the %s for location %s",
			QTH_VALUE_T_2_STRING[it].c_str(),
			label());
		ip->copy_tooltip(tip);
		// Position next widget
		if ((++ix % WIDGETS_PER_ROW) == 0) {
			cx = x() + WLABEL;
			cy += ROW_HEIGHT;
		}
		else {
			cx += WWIDGET + WLABEL;
		}
	}
	end();
}

//! Set the values into the inputs
void stn_qth_widget::enable_widgets() {
	auto data = stn_data_->get_qth(std::string(label()));
	// Id 
	// Description
	if (data->data.find(DESCRIPTION) != data->data.end()) {
		ip_descr_->value(data->data.at(DESCRIPTION).c_str());
	}
	else {
		ip_descr_->value("");
	}
	// data items
	for (qth_value_t it = (qth_value_t)0; it != DESCRIPTION; it = (qth_value_t)(it + 1)) {
		if (data->data.find(it) != data->data.end()) {
			ip_data_[it]->value(data->data.at(it).c_str());
		}
		else {
			ip_data_[it]->value("");
		}
	}
	redraw();
}


//! Callback - entering value into an input

//! \param widget that raised the callback
//! \param v pointer to std::string item to take input
void stn_qth_widget::cb_ip_data(Fl_Widget* w, void* v) {
	Fl_Input* ip = (Fl_Input*)w;
	stn_qth_widget* that = ancestor_view<stn_qth_widget>(w);
	qth_value_t field = (qth_value_t)(intptr_t)v;
	stn_data_->add_qth_item(that->label(), field, ip->value());
	// Tidy up display
	that->enable_widgets();
}

//! Constructor 

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
stn_qth_cntnr::stn_qth_cntnr(int X, int Y, int W, int H, const char* L) :
	Fl_Scroll(X, Y, W, H, L)
{
	selected_ = "";
	type(Fl_Scroll::VERTICAL);
	end();
}

//! Destructor
stn_qth_cntnr::~stn_qth_cntnr() {

}

//! Set the data
void stn_qth_cntnr::redraw_widgets() {
	// Delete wxisting widgets
	for (auto it : widgets_) {
		Fl_Widget_Tracker wp(it.second);
		Fl::delete_widget(it.second);
		while (!wp.deleted()) Fl::check();
	}
	widgets_.clear();
	// Save current
	Fl_Group* save_curr = Fl_Group::current();
	// Begin adding new widgets
	// Get the data
	const std::map<std::string, qth_info_t*>* data = stn_data_->get_qths();
	if (selected_.length() == 0 && data->size()) {
		selected_ = data->begin()->first;
	}
	// Get bound width
	int rx, ry, rw, rh;
	bbox(rx, ry, rw, rh);

	begin();
	int row = 0;
	int cy = ry;
	for (auto it : *data) {
		// Create widget
		stn_qth_widget* w = new stn_qth_widget(rx, cy, rw, ROW_HEIGHT * 5);
		w->copy_label(it.first.c_str());
		if (it.first == selected_) w->activate();
		else w->deactivate();
		if (it.first == stn_data_->defaults().location) w->labelfont(FL_BOLD);
		else w->labelfont(0);
		w->enable_widgets();
		// remember widget
		widgets_[it.first] = w;
		cy += w->h();
	}
	end();
	// Restore daved current
	Fl_Group::current(save_curr);
	redraw();
}


std::string stn_qth_cntnr::get_selected() {
	return selected_;
}

// Set selected
void stn_qth_cntnr::set_selected(std::string s) {
	selected_ = s;
	redraw_widgets();
}

stn_qth_widget* stn_qth_cntnr::value() {
	return widgets_.at(selected_);
}

//! Constructor 

 //! \param X horizontal position within host window
 //! \param Y vertical position with hosr window
 //! \param W width 
 //! \param H height
 //! \param L label
stn_qth_dlg::stn_qth_dlg(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L) 
{
	create_form();
}

//! Desctructor
stn_qth_dlg::~stn_qth_dlg() {
}

//! Callback from "Add" button
void stn_qth_dlg::cb_add(Fl_Widget* w, void* v) {
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	qth_info_t* info = new qth_info_t;
	stn_data_->add_qth(that->new_location_, info);
	that->location_ = that->new_location_;
	that->selected_new_ = false;
	that->enable_widgets();
}

//! Callback from "Delete" button
void stn_qth_dlg::cb_delete(Fl_Widget* w, void* v) {
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	stn_data_->delete_qth(that->location_);
	that->location_ = "";
	that->ip_new_->value("");
	that->enable_widgets();
}

//! Callback from "Clear" button to clear all values for specific identifier.
void stn_qth_dlg::cb_clear(Fl_Widget* w, void* v) {
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	qth_info_t* info = new qth_info_t;
	stn_data_->delete_qth(that->location_);
	stn_data_->add_qth(that->location_, info);
	that->enable_widgets();
}

//! Callback from "Rename" button to clear all values for specific identifier.
void stn_qth_dlg::cb_rename(Fl_Widget* w, void* v) {
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	qth_info_t* info = new qth_info_t;
	*info = *stn_data_->get_qth(that->location_);
	stn_data_->add_qth(that->new_location_, info);
	stn_data_->delete_qth(that->location_);
	that->location_ = that->new_location_;
	that->enable_widgets();
}

//! Callback from "Update from call" updates locator items that can be decoded from the call.
void stn_qth_dlg::cb_update(Fl_Widget* w, void* v) {
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	that->update_from_call();
}

//! Callback from choice
void stn_qth_dlg::cb_choice(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	if (ch->menubutton()->changed()) {
		that->selected_new_ = false;
		that->location_ = ch->value();
	}
	else {
		that->selected_new_ = true;
		that->new_location_ = ch->value();
	}
	that->enable_widgets();
}

// Callback from "Check log"
void stn_qth_dlg::cb_check(Fl_Widget* w, void* v) {
	status_->progress(navigation_book_->size(), OT_STN, "QTH match ", "records");
	int num_cant = 0;
	int num_do = 0;
	int num_dont = 0;
	int num_extra = 0;
	int num_multi = 0;
	int ix = 0;
	for (auto qso : *navigation_book_) {
		std::string s;
		switch (stn_data_->match_qso_qths(qso, s)) {
		case stn_data::CANT:
			num_cant++;
			break;
		case stn_data::DO:
			num_do++;
			break;
		case stn_data::DONT:
			num_dont++;
			break;
		case stn_data::EXTRA:
			num_extra++;
			break;
		case stn_data::MULTIPLE:
			num_multi++;
			break;
		}
		status_->progress(++ix, OT_STN);
	}
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	char msg[128];
	snprintf(msg, sizeof(msg), "STN DATA: %d do match; %d don't; %d can't; %d extra; %d multi",
		num_do, num_dont, num_cant, num_extra, num_multi);
	status_->misc_status(ST_NOTE, msg);
	that->enable_widgets();
}

//! Set default button
void stn_qth_dlg::cb_default(Fl_Widget* w, void* v) {
	stn_qth_dlg* that = ancestor_view<stn_qth_dlg>(w);
	stn_default defaults = stn_data_->defaults();
	defaults.location = that->location_;
	stn_data_->set_defaults(defaults);
	that->enable_widgets();
}

//! Instantiate component widgets
void stn_qth_dlg::create_form() {
	int cx = x() + GAP;
	int cy = y() + GAP;
	const int WINPUT = WBUTTON * 3 / 2;
	// New IS
	ip_new_ = new Fl_Input_Choice(cx, cy, WINPUT, HBUTTON);
	ip_new_->callback(cb_choice);
	ip_new_->tooltip("Name of location to ba added");
	// "Add" button - adds a record
	cx += WINPUT;
	bn_add_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Add");
	bn_add_->callback(cb_add);
	bn_add_->tooltip("Add a record to the table");
	// "Delete" button - removes selected record
	cx += WBUTTON;
	bn_delete_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Delete");
	bn_delete_->callback(cb_delete);
	bn_delete_->tooltip("Delete the selected record");
	// "Clear" button - clears selected record
	cx += WBUTTON;
	bn_clear_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Clear");
	bn_clear_->callback(cb_clear);
	bn_clear_->tooltip("Clears the selected record");
	// "Rename" button
	cx += WBUTTON;
	bn_rename_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Rename");
	bn_rename_->callback(cb_rename);
	bn_rename_->tooltip("Rename the selected record as new name"); 
	// "Set default"
	cx += WBUTTON + GAP;
	bn_default_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Set default");
	bn_default_->callback(cb_default);
	bn_default_->tooltip("Set the current location as default");

	// "Check all button
	cx += WBUTTON + GAP;
	bn_check_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Check Log");
	bn_check_->callback(cb_check);
	bn_check_->tooltip("Check the log or extracted data for QTH fields");

	// Callsign input
	cx = x() + GAP;
	cy += HBUTTON;
	ch_call_ = new Fl_Input_Choice(cx, cy, WINPUT, HBUTTON);
	ch_call_->tooltip("Select or enter a callsign to pre-populate selected record");
	// "Use call" button
	cx += WINPUT;
	bn_update_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Use call");
	bn_update_->callback(cb_update);
	bn_update_->tooltip("Decode callsign and enter details to record");

	cx = x() + GAP;
	cy += HBUTTON + GAP;

	int avail_h = h() - (cy - y());
	int avail_w = w() - GAP - GAP;

	table_ = new stn_qth_cntnr(cx, cy, avail_w, avail_h);
	load_data();

	show();
	end();
}

//! Enable widgets
void stn_qth_dlg::enable_widgets() {
	if (selected_new_) {
		bn_add_->activate();
		bn_rename_->activate();
		bn_delete_->deactivate();
		bn_clear_->deactivate();
		ch_call_->deactivate();
	}
	else {
		bn_add_->deactivate();
		bn_rename_->deactivate();
		bn_delete_->activate();
		bn_clear_->activate();
		ch_call_->activate();

	}
	if (qso_manager_) {
		ch_call_->activate();
		bn_update_->activate();
	}
	else {
		ch_call_->deactivate();
		bn_update_->deactivate();
	}
	load_data();
}


//! Updatefrom call
void stn_qth_dlg::update_from_call() {
	record* dummy_qso = qso_manager_->dummy_qso();
	dummy_qso->item("CALL", std::string(ch_call_->value()));
	cty_data_->update_qso(dummy_qso, true);
	for (auto it : QTH_ADIF_MAP) {
		if (dummy_qso->item(it.second).length()) {
			stn_data_->add_qth_item(location_, it.first, dummy_qso->item(it.second));
		}
	}
	enable_widgets();

}

//! Load data
void stn_qth_dlg::load_data() {
	if (location_.length() == 0) location_ = stn_data_->defaults().location;
	populate_calls();
	populate_locations();
	table_->set_selected(location_);
}

//! Populate call choice
void stn_qth_dlg::populate_calls() {
	ch_call_->clear();
	const std::map<std::string, std::string>* calls = stn_data_->get_calls();
	for (auto it : *calls) {
		ch_call_->add(escape_menu(it.first).c_str());
	}
}

// ! Populate locations
void stn_qth_dlg::populate_locations() {
	ip_new_->clear();
	const std::map<std::string, qth_info_t*>* qths = stn_data_->get_qths();
	for (auto it : *qths) {
		ip_new_->add(escape_menu(it.first).c_str());
	}
}

//! Set selected qTH
void stn_qth_dlg::set_location(std::string s) {
	if (stn_data_->known_qth(s)) {
		table_->set_selected(s);
		ip_new_->value(s.c_str());
	}
	else {
		qth_info_t* info = new qth_info_t;
		stn_data_->add_qth(s, info);
	}
	location_ = s;
	selected_new_ = false;
	enable_widgets();
}
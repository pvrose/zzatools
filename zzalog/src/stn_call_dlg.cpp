#include "stn_call_dlg.h"

#include "cty_data.h"
#include "qso_manager.h"
#include "record.h"
#include "stn_data.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Output.H>

extern qso_manager* qso_manager_;
extern stn_data* stn_data_;

//! Constructor 

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
stn_call_widget::stn_call_widget(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, "")
{
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	create_form();
}

//! Destructor
stn_call_widget::~stn_call_widget() {
}

//! Instantiate the component widgets
void stn_call_widget::create_form() {
	box(FL_FLAT_BOX);
	const int WIDGETS_PER_ROW = 4;
	const int WWIDGET = (w() / WIDGETS_PER_ROW) - WLABEL;
	int cx = x() + WLABEL;
	int cy = y();

	cx += WWIDGET + WLABEL;

	cx = x() + WLABEL + WLLABEL;
	// Inputs for all other members of call_info_t
	int ix = 0;
	ip_descr_ = new Fl_Input(cx, cy, w() - WLABEL - WLLABEL, ROW_HEIGHT, "Description");
	ip_descr_->box(FL_BORDER_BOX);
	ip_descr_->align(FL_ALIGN_LEFT);
	ip_descr_->callback(cb_ip_data);
	// Stt tooltip
	char tip[128];
	snprintf(tip, sizeof(tip), "Please enter the description for callsign %s",
		label());
	ip_descr_->copy_tooltip(tip);
	end();
}

//! Set the values into the inputs
void stn_call_widget::enable_widgets() {
	ip_descr_->value(stn_data_->get_call_descr(label()).c_str());
	redraw();
}


//! Callback - entering value into an input

//! \param widget that raised the callback
//! \param v pointer to std::string item to take input
void stn_call_widget::cb_ip_data(Fl_Widget* w, void* v) {
	Fl_Input* ip = (Fl_Input*)w;
	stn_call_widget* that = ancestor_view<stn_call_widget>(w);
	call_value_t field = (call_value_t)(intptr_t)v;
	stn_data_->set_call_descr(that->label(), ip->value());
	// Tidy up display
	that->enable_widgets();
}

//! Constructor 

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
stn_call_cntnr::stn_call_cntnr(int X, int Y, int W, int H, const char* L) :
	Fl_Scroll(X, Y, W, H, L)
{
	selected_ = "";
	type(Fl_Scroll::VERTICAL);
	end();
}

//! Destructor
stn_call_cntnr::~stn_call_cntnr() {

}

//! Set the data
void stn_call_cntnr::redraw_widgets() {
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
	const std::map<std::string, std::string>* data = stn_data_->get_calls();
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
		stn_call_widget* w = new stn_call_widget(rx, cy, rw, ROW_HEIGHT * 2);
		w->copy_label(it.first.c_str());
		if (it.first == selected_) w->labelfont(FL_BOLD);
		else w->labelfont(FL_ITALIC);
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


std::string stn_call_cntnr::get_selected() {
	return selected_;
}

// Set selected
void stn_call_cntnr::set_selected(std::string s) {
	selected_ = s;
	redraw_widgets();
}

stn_call_widget* stn_call_cntnr::value() {
	return widgets_.at(selected_);
}

//! Constructor 

 //! \param X horizontal position within host window
 //! \param Y vertical position with hosr window
 //! \param W width 
 //! \param H height
 //! \param L label
stn_call_dlg::stn_call_dlg(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	create_form();
}

//! Desctructor
stn_call_dlg::~stn_call_dlg() {
}

//! Callback from "Add" button
void stn_call_dlg::cb_add(Fl_Widget* w, void* v) {
	stn_call_dlg* that = ancestor_view<stn_call_dlg>(w);
	stn_data_->add_call(that->new_callsign_);
	that->callsign_ = that->new_callsign_;
	that->selected_new_ = false;
	that->enable_widgets();
}

//! Callback from "Delete" button
void stn_call_dlg::cb_delete(Fl_Widget* w, void* v) {
	stn_call_dlg* that = ancestor_view<stn_call_dlg>(w);
	stn_data_->delete_call(that->callsign_);
	that->callsign_ = "";
	that->ip_new_->value("");
	that->enable_widgets();
}

//! Callback from "Clear" button to clear all values for specific identifier.
void stn_call_dlg::cb_clear(Fl_Widget* w, void* v) {
	stn_call_dlg* that = ancestor_view<stn_call_dlg>(w);
	stn_data_->delete_call(that->callsign_);
	stn_data_->add_call(that->callsign_);
	that->enable_widgets();
}

//! Callback from choice
void stn_call_dlg::cb_choice(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	stn_call_dlg* that = ancestor_view<stn_call_dlg>(w);
	if (ch->menubutton()->changed()) {
		that->selected_new_ = false;
		that->callsign_ = ch->value();
	}
	else {
		that->selected_new_ = true;
		that->new_callsign_ = ch->value();
	}
	that->enable_widgets();
}

//! Set default button
void stn_call_dlg::cb_default(Fl_Widget* w, void* v) {
	stn_call_dlg* that = ancestor_view<stn_call_dlg>(w);
	stn_default defaults = stn_data_->defaults();
	defaults.callsign = that->callsign_;
	stn_data_->set_defaults(defaults);
	that->enable_widgets();
}

//! Instantiate component widgets
void stn_call_dlg::create_form() {
	int cx = x() + GAP;
	int cy = y() + GAP;
	// New IS
	ip_new_ = new Fl_Input_Choice(cx, cy, WSMEDIT, HBUTTON);
	ip_new_->callback(cb_choice);
	ip_new_->tooltip("Name of callsign to ba added");
	// "Add" button - adds a record
	cx += WSMEDIT;
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

	// "Set default"
	cx += WBUTTON + GAP;
	bn_default_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Set default");
	bn_default_->callback(cb_default);
	bn_default_->tooltip("Set the current location as default");

	cx = x() + GAP;
	cy += HBUTTON + GAP;

	int avail_h = h() - (cy - y());
	int avail_w = w() - GAP - GAP;

	table_ = new stn_call_cntnr(cx, cy, avail_w, avail_h);
	load_data();

	show();
	end();
}

//! Enable widgets
void stn_call_dlg::enable_widgets() {
	if (selected_new_) {
		bn_add_->activate();
		bn_delete_->deactivate();
		bn_clear_->deactivate();
	}
	else {
		bn_add_->deactivate();
		bn_delete_->activate();
		bn_clear_->activate();

	}
	load_data();
}



//! Load data
void stn_call_dlg::load_data() {
	if (callsign_.length() == 0) callsign_ = stn_data_->defaults().callsign;
	populate_callsigns();
	table_->set_selected(callsign_);
}

// ! Populate locations
void stn_call_dlg::populate_callsigns() {
	ip_new_->clear();
	const std::map<std::string, std::string>* calls = stn_data_->get_calls();
	for (auto it : *calls) {
		ip_new_->add(escape_menu(it.first).c_str());
	}
}

//! Set selected call
void stn_call_dlg::set_callsign(std::string s) {
	if (stn_data_->known_call(s)) {
		table_->set_selected(s);
		ip_new_->value(s.c_str());
	}
	else {
		stn_data_->add_call(s);
		callsign_ = s;
		selected_new_ = false;
		enable_widgets();
	}
}
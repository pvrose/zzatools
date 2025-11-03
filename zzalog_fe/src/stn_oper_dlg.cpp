#include "stn_oper_dlg.h"

#include "book.h"
#include "cty_data.h"
#include "main.h"
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

const std::string LABELS[] = { "Name", "Callsign" };

//! Constructor 

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
stn_oper_widget::stn_oper_widget(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, "")
{
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	create_form();
}

//! Destructor
stn_oper_widget::~stn_oper_widget() {
}

//! Instantiate the component widgets
void stn_oper_widget::create_form() {
	box(FL_FLAT_BOX);
	const int WIDGETS_PER_ROW = 4;
	const int WWIDGET = (w() / WIDGETS_PER_ROW) - WLABEL;
	int cx = x() + WLABEL;
	int cy = y();

	cx += WWIDGET + WLABEL;

	cx = x() + WLABEL + WLABEL;
	char tip[128];
	// Inputs for all other members of oper_info_t
	int ix = 0;
	for (int it = 0; it != 2; it++) {
		Fl_Input* ip = new Fl_Input(cx, cy, WWIDGET, ROW_HEIGHT);
		ip_data_[(oper_value_t)it] = ip;
		ip->box(FL_BORDER_BOX);
		ip->copy_label(LABELS[ix].c_str());
		ip->align(FL_ALIGN_LEFT);
		ip->callback(cb_ip_data, (void*)(intptr_t)it);
		// Stt tooltip
		snprintf(tip, sizeof(tip), "Please enter the %s for operator %s",
			LABELS[it].c_str(),
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
void stn_oper_widget::enable_widgets() {
	auto data = stn_data_->get_oper(std::string(label()));
	// data items
	for (oper_value_t it = (oper_value_t)0; it != MAX_OPER; it = (oper_value_t)(it + 1)) {
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
void stn_oper_widget::cb_ip_data(Fl_Widget* w, void* v) {
	Fl_Input* ip = (Fl_Input*)w;
	stn_oper_widget* that = ancestor_view<stn_oper_widget>(w);
	oper_value_t field = (oper_value_t)(intptr_t)v;
	stn_data_->add_oper_item(that->label(), field, ip->value());
	// Tidy up display
	that->enable_widgets();
}

//! Constructor 

//! \param X horizontal position within host window
//! \param Y vertical position with hosr window
//! \param W width 
//! \param H height
//! \param L label
stn_oper_cntnr::stn_oper_cntnr(int X, int Y, int W, int H, const char* L) :
	Fl_Scroll(X, Y, W, H, L)
{
	selected_ = "";
	type(Fl_Scroll::VERTICAL);
	end();
}

//! Destructor
stn_oper_cntnr::~stn_oper_cntnr() {

}

//! Set the data
void stn_oper_cntnr::redraw_widgets() {
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
	const std::map<std::string, oper_info_t*>* data = stn_data_->get_opers();
	if (selected_.length() == 0 && data->size()) {
		selected_ = data->begin()->first;
	}
	// Get bound width
	int rx, ry, rw, rh;
	bbox(rx, ry, rw, rh);

	begin();
	int cy = ry;
	for (auto it : *data) {
		// Create widget
		stn_oper_widget* w = new stn_oper_widget(rx, cy, rw, ROW_HEIGHT * 4/3);
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


std::string stn_oper_cntnr::get_selected() {
	return selected_;
}

// Set selected
void stn_oper_cntnr::set_selected(std::string s) {
	selected_ = s;
	redraw_widgets();
}

stn_oper_widget* stn_oper_cntnr::value() {
	return widgets_.at(selected_);
}

//! Constructor 

 //! \param X horizontal position within host window
 //! \param Y vertical position with hosr window
 //! \param W width 
 //! \param H height
 //! \param L label
stn_oper_dlg::stn_oper_dlg(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	create_form();
}

//! Desctructor
stn_oper_dlg::~stn_oper_dlg() {
}

//! Callback from "Add" button
void stn_oper_dlg::cb_add(Fl_Widget* w, void* v) {
	stn_oper_dlg* that = ancestor_view<stn_oper_dlg>(w);
	oper_info_t* info = new oper_info_t;
	stn_data_->add_oper(that->new_operator_, info);
	that->operator_ = that->new_operator_;
	that->selected_new_ = false;
	that->enable_widgets();
}

//! Callback from "Delete" button
void stn_oper_dlg::cb_delete(Fl_Widget* w, void* v) {
	stn_oper_dlg* that = ancestor_view<stn_oper_dlg>(w);
	stn_data_->delete_oper(that->operator_);
	that->operator_ = "";
	that->ip_new_->value("");
	that->enable_widgets();
}

//! Callback from "Clear" button to clear all values for specific identifier.
void stn_oper_dlg::cb_clear(Fl_Widget* w, void* v) {
	stn_oper_dlg* that = ancestor_view<stn_oper_dlg>(w);
	oper_info_t* info = new oper_info_t;
	stn_data_->delete_oper(that->operator_);
	stn_data_->add_oper(that->operator_, info);
	that->enable_widgets();
}

//! Callback from choice
void stn_oper_dlg::cb_choice(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	stn_oper_dlg* that = ancestor_view<stn_oper_dlg>(w);
	if (ch->menubutton()->changed()) {
		that->selected_new_ = false;
		that->operator_ = ch->value();
	}
	else {
		that->selected_new_ = true;
		that->new_operator_ = ch->value();
	}
	that->enable_widgets();
}

// Callback from "Check log"
void stn_oper_dlg::cb_check(Fl_Widget* w, void* v) {
	status_->progress(navigation_book_->size(), OT_STN, "Operator match ", "records");
	int num_cant = 0;
	int num_do = 0;
	int num_dont = 0;
	int num_extra = 0;
	int num_multi = 0;
	int ix = 0;
	for (auto qso : *navigation_book_) {
		std::string s;
		switch (stn_data_->match_qso_opers(qso, s)) {
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
	stn_oper_dlg* that = ancestor_view<stn_oper_dlg>(w);
	char msg[128];
	snprintf(msg, sizeof(msg), "STN DATA: %d do match; %d don't; %d can't; %d extra; %d multi",
		num_do, num_dont, num_cant, num_extra, num_multi);
	status_->misc_status(ST_NOTE, msg);
	that->enable_widgets();
}

//! Set default button
void stn_oper_dlg::cb_default(Fl_Widget* w, void* v) {
	stn_oper_dlg* that = ancestor_view<stn_oper_dlg>(w);
	stn_default defaults = stn_data_->defaults();
	defaults.name = that->operator_;
	stn_data_->set_defaults(defaults);
	that->enable_widgets();
}

//! Instantiate component widgets
void stn_oper_dlg::create_form() {
	int cx = x() + GAP;
	int cy = y() + GAP;
	// New IS
	ip_new_ = new Fl_Input_Choice(cx, cy, WSMEDIT, HBUTTON);
	ip_new_->callback(cb_choice);
	ip_new_->tooltip("Name of operator to ba added");
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

	// "Check all button
	cx += WBUTTON + GAP;
	bn_check_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Check Log");
	bn_check_->callback(cb_check);
	bn_check_->tooltip("Check the log or extracted data for operator fields");
	cx = x() + GAP;
	cy += HBUTTON + GAP;

	int avail_h = h() - (cy - y());
	int avail_w = w() - GAP - GAP;

	table_ = new stn_oper_cntnr(cx, cy, avail_w, avail_h);
	load_data();

	show();
	end();
}

//! Enable widgets
void stn_oper_dlg::enable_widgets() {
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
void stn_oper_dlg::load_data() {
	if (operator_.length() == 0) operator_ = stn_data_->defaults().name;
	populate_operators();
	table_->set_selected(operator_);
}

// ! Populate locations
void stn_oper_dlg::populate_operators() {
	ip_new_->clear();
	const std::map<std::string, oper_info_t*>* opers = stn_data_->get_opers();
	for (auto it : *opers) {
		ip_new_->add(escape_menu(it.first).c_str());
	}
}

//! Set selected oper
void stn_oper_dlg::set_operator(std::string s) {
	if (stn_data_->known_oper(s) || s.length() == 0) {
		table_->set_selected(s);
		ip_new_->value(s.c_str());
		ip_new_->update_menubutton();
	}
	else {
		oper_info_t* info = new oper_info_t;
		stn_data_->add_oper(s, info);
		operator_ = s;
		selected_new_ = false;
		enable_widgets();
	}
}

//! Get operator
std::string stn_oper_dlg::get_operator() {
	return operator_;
}
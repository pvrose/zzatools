#include "stn_dialog.h"

#include "cty_data.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "spec_data.h"
#include "stn_data.h"

#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tabs.H>

extern Fl_Preferences::Root prefs_mode_;
extern cty_data* cty_data_;
extern qso_manager* qso_manager_;
extern spec_data* spec_data_;
extern stn_data* stn_data_;
extern void open_html(const char*);

stn_dialog::stn_dialog(int X, int Y, int W, int H, const char* L) :
	page_dialog(X, Y, W, H, L),
	g_qth_(nullptr),
	g_oper_(nullptr),
	g_call_(nullptr)
{
	create_form(x(), y());
	load_values();
	enable_widgets();
	redraw();
}

stn_dialog::~stn_dialog() {}

// Handle
int stn_dialog::handle(int event) {
	int result = page_dialog::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		if (!result) take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("stn_dialog.html");
			return true;
		}
		break;
	}
	return result;
}

// inherited methods
// Standard methods - need to be written for each
// Load values
void stn_dialog::load_values() {
	if (qso_manager_ && qso_manager_->data()) {
		qso_ = qso_manager_->data()->current_qso();
	}
	else {
		qso_ = nullptr;
	}
}

// Used to create the form
void stn_dialog::create_form(int X, int Y) {
	// Create an Fl_Tabs
	tabs_ = new Fl_Tabs(x() + GAP, y() + GAP, w() - GAP - GAP, h() - GAP - GAP);
	tabs_->callback(cb_tab);
	tabs_->box(FL_FLAT_BOX);

	int rx = 0, ry = 0, rw = 0, rh = 0;
	// Get individual area
	tabs_->client_area(rx, ry, rw, rh);

	g_qth_ = new single_tab(rx, ry, rw, rh, QTH, "QTHs");

	g_oper_ = new single_tab(rx, ry, rw, rh, OPERATOR, "Operators");

	g_call_ = new single_tab(rx, ry, rw, rh, CALLSIGN, "Callsigns");

	tabs_->end();

	end();
	show();
}

// Used to write settings back
void stn_dialog::save_values() {
	g_qth_->save_data();
	g_oper_->save_data();
	g_call_->save_data();
	// Make the changes availble in qso_data
	qso_manager_->data()->update_station_choices();
}

// Used to enable/disable specific widget - any widgets enabled musr be attributes
void stn_dialog::enable_widgets() {
	// Standard tab formats
// value() returns the selected widget. We need to test which widget it is.
	Fl_Tabs* tabs = (Fl_Tabs*)child(0);
	Fl_Widget* tab = tabs->value();
	for (int ix = 0; ix < tabs->children(); ix++) {
		Fl_Widget* wx = tabs->child(ix);
		if (wx == tab) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
			wx->activate();
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
			wx->deactivate();
		}
	}
	g_qth_->load_data();
	g_oper_->load_data();
	g_call_->load_data();
	g_qth_->enable_widgets();
	g_oper_->enable_widgets();
	g_call_->enable_widgets();
}

// Switch tabs
void stn_dialog::cb_tab(Fl_Widget* w, void* v) {
	stn_dialog* that = ancestor_view<stn_dialog>(w);
	that->enable_widgets();
}

// Set tab
void stn_dialog::set_tab(tab_type t, std::string id) {
	for (int ix = 0; ix < tabs_->children(); ix++) {
		single_tab* w = (single_tab*)tabs_->child(ix);
		if (w->type() == t) {
			tabs_->value(w);
			w->activate();
			w->id(id);
		}
	}
	enable_widgets();
}

stn_dialog::single_tab::single_tab(int rx, int ry, int rw, int rh, char type,
	const char* l) :
	Fl_Group(rx, ry, rw, rh, l) {
	single_tab::type(type);
	update_from_call_ = false;
	load_data();
	create_form();
	enable_widgets();
}

stn_dialog::single_tab::~single_tab() {}

// Tooltips for the specific fields entrie
std::map <qth_value_t, std::string> QTH_TIPS = {
	{ STREET, "Please enter your street address - optional" },
	{ CITY, "Please enter your town or city - recommended" },
	{ POSTCODE, "Please enter your postal code - optional" },
	{ LOCATOR, "Please enter your grid square (2, 4, 6 or 8 character) - 6 recommended" },
	{ DXCC_NAME, "Please enter the name of your DXCC entity - not recomended" },
	{ DXCC_ID, "Please enter the ARRL numerical code for your DXCC entity - recommended" },
	{ PRIMARY_SUB, "Please enter the primary administrative subdivision name (eg US State) - if supported for your entity" },
	{ SECONDARY_SUB, "Please enter the secondary administrative subdivision name (eg US County) - if supported for your entity" },
	{ CQ_ZONE, "Please enter the CQ Zone number for your location - optional, needed for some contests" },
	{ ITU_ZONE, "Please enter the ITU Zone number for your location - optional, needed for some contests" },
	{ CONTINENT, "Please enter the two-character code for your continent (AN, AF, AS, EU, NA, OC or SA) - optional" },
	{ IOTA, "Please enter the IOTA id for your location - if a registered island location" },
	{ WAB, "Please enter the Worked All Britain (4-caharacter Ordnance Survey geolocator) for your location - optional" }
};
std::map <oper_value_t, std::string> OPER_TIPS = {
	{ NAME, "Please enter your name as you want to see it in exchanges - recommended"},
	{ CALLSIGN, "Please enter your callsign appropriate for operating this station - mandatory"}
};


void stn_dialog::single_tab::create_form() {
	int curr_x = x() + GAP + WLABEL;
	int curr_y = y() + HTEXT;

	begin();

	Fl_Box* b_msg = new Fl_Box(x() + GAP, curr_y, w() - GAP - GAP, HBUTTON * 2);
	b_msg->box(FL_FLAT_BOX);
	b_msg->labelsize(FL_NORMAL_SIZE + 2);
	b_msg->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	switch (type()) {
	case QTH:
		b_msg->label("Please select or enter an identifier for the location the station is being operated from\n"
			"as well as information about that location (Locator etc.)");
		break;
	case OPERATOR:
		if (prefs_mode_ == Fl_Preferences::SYSTEM_L) {
			b_msg->label("Please select or enter the initials of the person intending to operate (NECESSARY for a club station).\n"
				"Add the operator's name and callsign.");
		}
		else {
			b_msg->label("Please select or enter the initials of the person intending to operate. \n"
				"Add the operator's name and callsign.");
		}
		break;
	case CALLSIGN:
		b_msg->label("Please select or enter the callsign being used by the station.");
		break;
	}

	curr_y += b_msg->h() + GAP;

	ch_id_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON);
	ch_id_->label(type() == CALLSIGN ? "Call" : "Id");
	ch_id_->align(FL_ALIGN_LEFT);
	ch_id_->callback(cb_ch_id, &current_id_);
	ch_id_->tooltip("Select the Id (or Call)");

	if (type() == QTH) {
		curr_x += ch_id_->w() + GAP;
		curr_x += WLLABEL;
		bn_update_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Update from call");
		bn_update_->align(FL_ALIGN_LEFT);
		bn_update_->value(update_from_call_);
		bn_update_->callback(cb_update, &update_from_call_);
		bn_update_->tooltip("If checked, selecting a call will update DXCC etc");

		curr_x += HBUTTON;
		ch_call_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON);
		ch_call_->tooltip("Selecting a call will update DXCC etc");

		curr_x += WSMEDIT;
		bn_do_update_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Update");
		bn_do_update_->callback(cb_ch_call);
		bn_do_update_->tooltip("Update fields from callsign");
	}

	curr_x = x() + GAP + WLABEL;
	curr_y += HBUTTON;

	if (type() != CALLSIGN) {
		Fl_Box* b2 = new Fl_Box(x() + GAP, curr_y, w() - GAP - GAP, HBUTTON, "Enter data below if you want the information saved in the log file");
		b2->box(FL_FLAT_BOX);
		b2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		curr_y += HBUTTON;
	}

	if (type() == QTH) {
		curr_x = x() + GAP + WLABEL;
		bn_clear_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Clear");
		bn_clear_->callback(cb_clear);
		bn_clear_->tooltip("Clear all entries");
		curr_y += HBUTTON;
	}

	int save_y = curr_y;

	int vertical = (h() - curr_y) / HBUTTON;
	ip_values_ = new Fl_Input * [num_inputs_];
	int ix = 0;
	for (; ix < vertical && ix < num_inputs_; ix++) {
		ip_values_[ix] = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON);
		ip_values_[ix]->copy_label(labels_[ix].c_str());
		switch (type()) {
		case QTH:
			ip_values_[ix]->copy_tooltip(QTH_TIPS.at((qth_value_t)ix).c_str());
			break;
		case OPERATOR:
			ip_values_[ix]->copy_tooltip(OPER_TIPS.at((oper_value_t)ix).c_str());
			break;
		}

		curr_y += HBUTTON;
	}
	if (ix == vertical && ix < num_inputs_) {
		// Note I am assuming that no more than two colums will be needed
		curr_y = save_y;
		curr_x += WSMEDIT + GAP + WLABEL;
		for (; ix < num_inputs_; ix++) {
			ip_values_[ix] = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON);
			ip_values_[ix]->copy_label(labels_[ix].c_str());
			switch (type()) {
			case QTH:
				ip_values_[ix]->copy_tooltip(QTH_TIPS.at((qth_value_t)ix).c_str());
				break;
			case OPERATOR:
				ip_values_[ix]->copy_tooltip(OPER_TIPS.at((oper_value_t)ix).c_str());
				break;
			}
			curr_y += HBUTTON;
		}
	}
	populate_choice(ch_id_, (tab_type)type());
	if (type() == QTH) {
		populate_choice(ch_call_, CALLSIGN);
	}
	end();
	show();
}

void stn_dialog::single_tab::enable_widgets() {
	populate_choice(ch_id_, (tab_type)type());
	if (type() == QTH) {
		populate_choice(ch_call_, CALLSIGN);
	}
	if (visible()) {
		switch (type()) {
		case QTH:
			if (qth_) {
				ch_id_->value(current_id_.c_str());
				int dxcc;
				bool has_states = true;
				if (qth_->data.find(DXCC_ID) != qth_->data.end() && qth_->data.at(DXCC_ID).length()) {
					dxcc = std::stoi(qth_->data.at(DXCC_ID));
					if (!spec_data_->has_states(dxcc)) has_states = false;
				}
				for (auto it : QTH_ADIF_MAP) {
					if (qth_->data.find(it.first) != qth_->data.end()) {
						ip_values_[(int)it.first]->value(qth_->data.at(it.first).c_str());
						switch (it.first) {
						case PRIMARY_SUB:
						case SECONDARY_SUB:
							if (has_states) {
								ip_values_[(int)it.first]->activate();
							}
							else {
								ip_values_[(int)it.first]->deactivate();
							}
							break;
						default:
							ip_values_[(int)it.first]->activate();
							break;
						}
					}
					else {
						ip_values_[(int)it.first]->value("");
					}
				}
			}
			else {
				ch_id_->value(current_id_.c_str());
				for (auto it : QTH_ADIF_MAP) {
					ip_values_[(int)it.first]->value("");
				}
			}
			if (update_from_call_) {
				ch_call_->activate();
			}
			else {
				ch_call_->deactivate();
			}
			break;
		case OPERATOR:
			if (oper_) {
				ch_id_->value(current_id_.c_str());
				for (auto it : OPER_ADIF_MAP) {
					if (oper_->data.find(it.first) != oper_->data.end()) {
						ip_values_[(int)it.first]->value(oper_->data.at(it.first).c_str());
					}
					else {
						ip_values_[(int)it.first]->value("");
					}
				}
			}
			else {
				ch_id_->value(current_id_.c_str());
				for (auto it : OPER_ADIF_MAP) {
					ip_values_[(int)it.first]->value("");
				}
			}
			break;
		case CALLSIGN:
			ch_id_->value(current_id_.c_str());
			break;
		}
		redraw();
		show();
	}
}

void stn_dialog::single_tab::load_data() {
	switch (type()) {
	case QTH:
		current_id_ = qso_manager_->get_default(qso_manager::QTH);
		qth_ = stn_data_->get_qth(current_id_);
		break;
	case OPERATOR:
		current_id_ = qso_manager_->get_default(qso_manager::OP);
		oper_ = stn_data_->get_oper(current_id_);
		break;
	case CALLSIGN:
		current_id_ = qso_manager_->get_default(qso_manager::CALLSIGN);
		call_descr_ = stn_data_->get_call_descr(current_id_);
		break;
	}
}

void stn_dialog::single_tab::save_data() {
	if (current_id_.length()) {
		switch (type()) {
		case QTH:
			if (!stn_data_->known_qth(current_id_)) {
				qth_info_t* info = new qth_info_t;
				stn_data_->add_qth(current_id_, info);
			}
			qth_ = stn_data_->get_qth(current_id_);
			for (auto it : QTH_ADIF_MAP) {
				std::string data;
				switch (it.first) {
				case STREET:
				case CITY:
					data = ip_values_[(int)it.first]->value();
					break;
				default:
					data = to_upper(ip_values_[(int)it.first]->value());
					break;
				}
				stn_data_->add_qth_item(current_id_, it.first, data);
			}
			break;
		case OPERATOR:
			if (!stn_data_->known_oper(current_id_)) {
				oper_info_t* info = new oper_info_t;
				stn_data_->add_oper(current_id_, info);
			}
			oper_ = stn_data_->get_oper(current_id_);
			for (auto it : OPER_ADIF_MAP) {
				std::string data;
				switch (it.first) {
				case NAME:
					data = ip_values_[(int)it.first]->value();
					break;
				default:
					data = to_upper(ip_values_[(int)it.first]->value());
					break;
				}
				stn_data_->add_oper_item(current_id_, it.first, ip_values_[(int)it.first]->value());
			}
			break;
		case CALLSIGN:
			if (!stn_data_->known_call(current_id_)) {
				stn_data_->add_call(current_id_);
			}
			call_descr_ = stn_data_->get_call_descr(current_id_);
			stn_data_->add_call(current_id_);
			break;
		}
	}
}

// Save new ID value
void stn_dialog::single_tab::cb_ch_id(Fl_Widget* w, void* v) {
	single_tab* that = ancestor_view<single_tab>(w);
	std::string s;
	cb_value<Fl_Input_Choice, std::string>(w, &s);
	that->id(s);
	that->enable_widgets();
}

// Selecting a callsign in QTH
void stn_dialog::single_tab::cb_ch_call(Fl_Widget* w, void* v) {
	single_tab* that = ancestor_view<single_tab>(w);
	that->update_from_call();
}

// Clearing the values
void stn_dialog::single_tab::cb_clear(Fl_Widget* w, void* v) {
	single_tab* that = ancestor_view<single_tab>(w);
	that->clear_entry();
}

// Set update from callsign
void stn_dialog::single_tab::cb_update(Fl_Widget* w, void* v) {
	cb_value<Fl_Check_Button, bool>(w, v);
	single_tab* that = ancestor_view<single_tab>(w);
	that->enable_widgets();
}


// Set type value and configure dialog
void stn_dialog::single_tab::type(char t) {
	Fl_Group::type(t);
	switch (type()) {
	case QTH:
		num_inputs_ = QTH_ADIF_MAP.size();
		labels_ = { "Strret", "City", "Postcode", "Locator", "Country", "DXCC",
			"Prim'y Sub", "Sec'y Sub", "CQ Zone", "ITU Zone", "Continent", "IOTA", "WAB" };
		break;
	case OPERATOR:
		num_inputs_ = OPER_ADIF_MAP.size();
		labels_ = { "Name", "Callsign" };
		break;
	case CALLSIGN:
		num_inputs_ = 0;
		labels_ = { };
		break;
	}
}

// Return type value
char stn_dialog::single_tab::type() { return Fl_Group::type(); }

// Return current ID
std::string stn_dialog::single_tab::id() { return current_id_; }

// Set ID and initialise data
void stn_dialog::single_tab::id(std::string s) {
	current_id_ = s;
	switch (type()) {
	case QTH:
		qth_ = stn_data_->get_qth(current_id_);
		break;
	case OPERATOR:
		oper_ = stn_data_->get_oper(current_id_);
		break;
	case CALLSIGN:
		call_descr_ = stn_data_->get_call_descr(current_id_);
		break;
	}
	enable_widgets();
}

void stn_dialog::single_tab::populate_choice(Fl_Input_Choice* ch, tab_type t) {
	ch->clear();
	ch->add("");
	switch (t) {
	case QTH:
	{
		const std::map<std::string, qth_info_t*>* qths = stn_data_->get_qths();
		for (auto it : *qths) {
			ch->add(escape_menu(it.first).c_str());
		}
		break;
	}
	case OPERATOR:
	{
		const std::map<std::string, oper_info_t*>* opers = stn_data_->get_opers();
		for (auto it : *opers) {
			ch->add(escape_menu(it.first).c_str());
		}
		break;
	}
	case CALLSIGN:
	{
		const std::map<std::string, std::string>* calls = stn_data_->get_calls();
		for (auto it : *calls) {
			ch->add(escape_menu(it.first).c_str());
		}
		break;
	}
	}
}

// Update values from call
void stn_dialog::single_tab::update_from_call() {
	switch (type()) {
	case QTH: {
		if (current_id_.length() == 0) {
			fl_alert("Please enter a name for the location");
		}
		else {
			record* dummy_qso = qso_manager_->dummy_qso();
			dummy_qso->item("CALL", std::string(ch_call_->value()));
			cty_data_->update_qso(dummy_qso, true);
			for (auto it : QTH_ADIF_MAP) {
				if (dummy_qso->item(it.second).length()) {
					stn_data_->add_qth_item(current_id_, it.first, dummy_qso->item(it.second));
				}
			}
			load_data();
			enable_widgets();
		}
	}
	}
}

// Clear values
void stn_dialog::single_tab::clear_entry() {
	switch (type()) {
	case QTH:
		for (auto it : QTH_ADIF_MAP) {
			stn_data_->remove_qth_item(current_id_, it.first);
		}
	}
	enable_widgets();
}

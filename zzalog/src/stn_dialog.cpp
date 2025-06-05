#include "stn_dialog.h"

#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "stn_data.h"

#include "callback.h"
#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Tabs.H>

extern stn_data* stn_data_;
extern qso_manager* qso_manager_;

stn_dialog::stn_dialog(int X, int Y, int W, int H, const char* L) :
	page_dialog(X, Y, W, H, L),
	g_qth_(nullptr),
	g_oper_(nullptr),
	g_call_(nullptr)
{
	create_form(x(), y());
	load_values();
	enable_widgets();
}

stn_dialog::~stn_dialog() {}

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
}

// Used to write settings back
void stn_dialog::save_values() {
	g_qth_->save_data();
	g_oper_->save_data();
	g_call_->save_data();
}

// Used to enable/disable specific widget - any widgets enabled musr be attributes
void stn_dialog::enable_widgets() {
	g_qth_->enable_widgets();
	g_oper_->enable_widgets();
	g_call_->enable_widgets();
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
}

// Switch tabs
void stn_dialog::cb_tab(Fl_Widget* w, void* v) {
	stn_dialog* that = ancestor_view<stn_dialog>(w);
	that->enable_widgets();
}

// Set tab
void stn_dialog::set_tab(tab_type t, string id) {
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
	load_data();
	create_form();
	enable_widgets();
}

stn_dialog::single_tab::~single_tab() {}

void stn_dialog::single_tab::create_form() {
	int curr_x = x() + GAP + WLABEL;
	int curr_y = y() + HTEXT;

	ch_id_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON);
	ch_id_->label(type() == CALLSIGN ? "Call" : "Id");
	ch_id_->align(FL_ALIGN_LEFT);
	ch_id_->callback(cb_ch_id, &current_id_);
	ch_id_->tooltip("Select the Id (or Call)");

	curr_x = x() + GAP;
	curr_y += HBUTTON;

	ip_descr_ = new Fl_Multiline_Input(curr_x, curr_y, WLABEL + WSMEDIT, 4 * HBUTTON);
	ip_descr_->wrap(true);
	ip_descr_->tooltip("Enter a brief description of the item");

	curr_y += ip_descr_->h();
	curr_x = x() + GAP + WLABEL;

	int save_y = curr_y;

	int vertical = (h() - curr_y) / HBUTTON;
	ip_values_ = new Fl_Input * [num_inputs];
	int ix = 0;
	for (; ix < vertical && ix < num_inputs; ix++) {
		ip_values_[ix] = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON);
		ip_values_[ix]->copy_label(labels_[ix].c_str());
		ip_values_[ix]->tooltip("Enter a value if you want this in ADIF");

		curr_y += HBUTTON;
	}
	if (ix == vertical && ix < num_inputs) {
		// Note I am assuming that no more than two colums will be needed
		curr_y = save_y;
		curr_x += WSMEDIT + GAP + WLABEL;
		for (; ix < num_inputs; ix++) {
			ip_values_[ix] = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON);
			ip_values_[ix]->copy_label(labels_[ix].c_str());
			ip_values_[ix]->tooltip("Enter a value if you want this in ADIF");

			curr_y += HBUTTON;
		}
	}
	populate_choice();
	end();
}

void stn_dialog::single_tab::enable_widgets() {
	switch (type()) {
	case QTH:
		if (qth_) {
			ch_id_->value(current_id_.c_str());
			ip_descr_->value(qth_->description.c_str());
			for (auto it : QTH_ADIF_MAP) {
				if (qth_->data.find(it.first) != qth_->data.end()) {
					ip_values_[(int)it.first]->value(qth_->data.at(it.first).c_str());
				}
				else {
					ip_values_[(int)it.first]->value("");
				}
			}
		}
		break;
	case OPERATOR:
		if (oper_) {
			ch_id_->value(current_id_.c_str());
			ip_descr_->value(oper_->description.c_str());
			for (auto it : OPER_ADIF_MAP) {
				if (oper_->data.find(it.first) != oper_->data.end()) {
					ip_values_[(int)it.first]->value(oper_->data.at(it.first).c_str());
				}
				else {
					ip_values_[(int)it.first]->value("");
				}
			}
		}
		break;
	case CALLSIGN:
		ch_id_->value(current_id_.c_str());
		ip_descr_->value(call_descr_.c_str());
		break;
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
			stn_data_->add_qth_descr(current_id_, ip_descr_->value());
			for (auto it : QTH_ADIF_MAP) {
				string data;
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
			stn_data_->add_oper_descr(current_id_, ip_descr_->value());
			for (auto it : OPER_ADIF_MAP) {
				string data;
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
			stn_data_->add_call(current_id_, to_upper(ip_descr_->value()));
			break;
		}
	}
}

void stn_dialog::single_tab::cb_ch_id(Fl_Widget* w, void* v) {
	single_tab* that = ancestor_view<single_tab>(w);
	cb_value<Fl_Input_Choice, string>(w, &that->current_id_);
	switch (that->type()) {
	case QTH:
		if (!stn_data_->known_qth(that->current_id_)) {
			qth_info_t* info = new qth_info_t;
			stn_data_->add_qth(that->current_id_, info);
		}
		that->qth_ = stn_data_->get_qth(that->current_id_);
		break;
	case OPERATOR:
		if (!stn_data_->known_oper(that->current_id_)) {
			oper_info_t* info = new oper_info_t;
			stn_data_->add_oper(that->current_id_, info);
		}
		that->oper_ = stn_data_->get_oper(that->current_id_);
		break;
	case CALLSIGN:
		if (!stn_data_->known_call(that->current_id_)) {
			stn_data_->add_call(that->current_id_, "");
		}
		that->call_descr_ = stn_data_->get_call_descr(that->current_id_);
		break;
	}
	that->enable_widgets();
}

void stn_dialog::single_tab::type(char t) {
	Fl_Group::type(t);
	switch (type()) {
	case QTH:
		num_inputs = QTH_ADIF_MAP.size();
		labels_ = { "Strret", "City", "Postcode", "Locator", "Country", "DXCC",
			"Prim'y Sub", "Sec'y Sub", "CQ Zone", "ITU Zone", "Continent", "IOTA", "WAB" };
		break;
	case OPERATOR:
		num_inputs = OPER_ADIF_MAP.size();
		labels_ = { "Name", "Callsign" };
		break;
	case CALLSIGN:
		num_inputs = 0;
		labels_ = { };
		break;
	}
}

char stn_dialog::single_tab::type() { return Fl_Group::type(); }

string stn_dialog::single_tab::id() { return current_id_; }

void stn_dialog::single_tab::id(string s) {
	current_id_ = s;
	switch (type()) {
	case QTH:
		if (!stn_data_->known_qth(current_id_)) {
			qth_info_t* info = new qth_info_t;
			stn_data_->add_qth(current_id_, info);
		}
		qth_ = stn_data_->get_qth(current_id_);
		break;
	case OPERATOR:
		if (!stn_data_->known_oper(current_id_)) {
			oper_info_t* info = new oper_info_t;
			stn_data_->add_oper(current_id_, info);
		}
		oper_ = stn_data_->get_oper(current_id_);
		break;
	case CALLSIGN:
		if (!stn_data_->known_call(current_id_)) {
			stn_data_->add_call(current_id_, "");
		}
		call_descr_ = stn_data_->get_call_descr(current_id_);
		break;
	}
}

void stn_dialog::single_tab::populate_choice() {
	ch_id_->clear();
	ch_id_->add("");
	switch (type()) {
	case QTH:
	{
		const map<string, qth_info_t*>* qths = stn_data_->get_qths();
		for (auto it : *qths) {
			ch_id_->add(escape_menu(it.first).c_str());
		}
		break;
	}
	case OPERATOR:
	{
		const map<string, oper_info_t*>* opers = stn_data_->get_opers();
		for (auto it : *opers) {
			ch_id_->add(escape_menu(it.first).c_str());
		}
		break;
	}
	case CALLSIGN:
	{
		const map<string, string>* calls = stn_data_->get_calls();
		for (auto it : *calls) {
			ch_id_->add(escape_menu(it.first).c_str());
		}
		break;
	}
	}
}

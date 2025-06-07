#include "qso_operation.h"

#include "config.h"
#include "qso_data.h"
#include "record.h"
#include "status.h"
#include "stn_data.h"
#include "stn_dialog.h"


#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Button.H>;

extern config* config_;
extern status* status_;
extern stn_data* stn_data_;

qso_operation::qso_operation(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L),
	current_qth_(""),
	current_oper_(""),
	current_qso_(nullptr)
{
	create_form();
	populate_choices();
	enable_widgets();
}

qso_operation::~qso_operation() {

}

void qso_operation::create_form() {
	int curr_x = x() + GAP;
	int curr_y = y();

	curr_x += WLABEL;
	ch_qth_ = new annotated_choice(curr_x, curr_y, WSMEDIT, HBUTTON, "QTH");
	ch_qth_->align(FL_ALIGN_LEFT);
	ch_qth_->callback(cb_qth, &current_qth_);
	ch_qth_->input()->when(FL_WHEN_ENTER_KEY_ALWAYS);
	ch_qth_->tooltip("Select the current operating location (or eneter a new one)");

	curr_x += ch_qth_->w();
	Fl_Button* bn_show_qth = new Fl_Button(curr_x, curr_y, WBUTTON/2, HBUTTON, "Show"); 
	bn_show_qth->callback(cb_show, (void*)(intptr_t)stn_dialog::QTH);

	curr_x += bn_show_qth->w() + WLABEL;
	ch_oper_ = new annotated_choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Operator");
	ch_oper_->align(FL_ALIGN_LEFT);
	ch_oper_->callback(cb_oper, &current_oper_);
	ch_oper_->input()->when(FL_WHEN_ENTER_KEY_ALWAYS);
	ch_oper_->tooltip("Specify the current operator - select or enter new");


	curr_x += ch_qth_->w();
	Fl_Button* bn_show_oper = new Fl_Button(curr_x, curr_y, WBUTTON/2, HBUTTON, "Show"); 
	bn_show_oper->callback(cb_show, (void*)(intptr_t)stn_dialog::OPERATOR);

	curr_x += bn_show_oper->w() + WLABEL;
	ch_call_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Station\nCallsign");
	ch_call_->align(FL_ALIGN_LEFT);
	ch_call_->callback(cb_call, &current_call_);
	ch_call_->input()->when(FL_WHEN_ENTER_KEY_ALWAYS);
	ch_call_->tooltip("Specify the current station callsign");


	curr_x += ch_qth_->w();
	Fl_Button* bn_show_call = new Fl_Button(curr_x, curr_y, WBUTTON/2, HBUTTON, "Show"); 
	bn_show_call->callback(cb_show, (void*)(intptr_t)stn_dialog::CALLSIGN);

	curr_x += bn_show_call->w() + GAP;
	curr_y += HBUTTON + GAP;

	resizable(nullptr);
	size(curr_x - x(), curr_y - y());
	end();
}

// Update the widget values
void qso_operation::enable_widgets() {
	qso_data* qd = ancestor_view<qso_data>(this);
	ch_qth_->value(current_qth_.c_str());
	ch_oper_->value(current_oper_.c_str());
	ch_call_->value(current_call_.c_str());
	if (qd->inactive()) {
		ch_qth_->activate();
		ch_oper_->activate();
		ch_call_->activate();
	}
	else {
		ch_qth_->deactivate();
		ch_oper_->deactivate();
		ch_call_->deactivate();
	}
}

// QTH button clicked
void qso_operation::cb_qth(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	qso_operation* that = ancestor_view<qso_operation>(w);
	*(string*)v = ch->value();
	if (!ch->menubutton()->changed()) {
		// New QTH typed in - check if it's really new
		if (!stn_data_->known_qth(that->current_qth_)) {
			that->new_qth();
			return;
		}
	}
}

void qso_operation::cb_oper(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	qso_operation* that = ancestor_view<qso_operation>(w);
	*(string*)v = ch->value();
	if (!ch->menubutton()->changed()) {
		// New QTH typed in - check if it's really new
		if (!stn_data_->known_oper(that->current_oper_)) {
			that->new_oper();
			return;
		}
	}
}

void qso_operation::cb_call(Fl_Widget* w, void* v) {
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	qso_operation* that = ancestor_view<qso_operation>(w);
	*(string*)v = ch->value();
	if (!ch->menubutton()->changed()) {
		// New QTH typed in - check if it's really new
		if (!stn_data_->known_call(that->current_call_)) {
			that->new_call();
			return;
		}
	}
}

void qso_operation::cb_show(Fl_Widget* w, void* v) {
	qso_operation* that = ancestor_view<qso_operation>(w);
	stn_dialog::tab_type t = (stn_dialog::tab_type)(intptr_t)v;
	switch(t) {
		case stn_dialog::QTH:
		that->new_qth();
		break;
	case stn_dialog::OPERATOR:
		that->new_oper();
		break;
	case stn_dialog::CALLSIGN:
		that->new_call();
		break;
	}
}
// set the QSO
void qso_operation::qso(record* qso) {
	current_qso_ = qso;
	evaluate_qso();
	enable_widgets();
}

// get the current QTH
string qso_operation::current_qth() {
	return current_qth_;
}

string qso_operation::current_oper() {
	return current_oper_;
}

string qso_operation::current_call() {
	return current_call_;
}

// Populate the choices
void qso_operation::populate_choices() {
	char l[64];
	// Populate QTH choice
	ch_qth_->add("");
	for (auto it : *stn_data_->get_qths()) {
		if (it.second->description.length()) {
			snprintf(l, sizeof(l), "%s:--->%s", it.first.c_str(), it.second->description.c_str());
		} else {
			map<qth_value_t, string>& data = it.second->data;
			snprintf(l, sizeof(l), "%s:--->%s,%s,%s", 
				escape_menu(it.first).c_str(),
				data.find(STREET) == data.end() ? "" : data.at(STREET).c_str(),
				data.find(CITY) == data.end() ? "" : data.at(CITY).c_str(),
				data.find(LOCATOR) == data.end() ? "" : data.at(LOCATOR).c_str());
		}
		ch_qth_->add(l);
		ch_qth_->menubutton()->user_data((void*)&it.first);
	}  
	// Populate Operator choice
	ch_oper_->add("");
	for (auto it : *stn_data_->get_opers()) {
		if (it.second->description.length()) {
			snprintf(l, sizeof(l), "%s:--->%s", it.first.c_str(), it.second->description.c_str());
		} else {
			map<oper_value_t, string>& data = it.second->data;
			snprintf(l, sizeof(l), "%s:--->%s,%s", 
				escape_menu(it.first).c_str(),
				data.find(NAME) == data.end() ? "" : data.at(NAME).c_str(),
				data.find(CALLSIGN) == data.end() ? "" : data.at(CALLSIGN).c_str());
		}
		ch_oper_->add(l);
		ch_oper_->menubutton()->user_data((void*)&it.first);
	}
	// Populate callsign choice
	ch_call_->add("");
	for (auto it : *stn_data_->get_calls()) {
		ch_call_->add(escape_menu(it.first).c_str());
	}
}

// Local matching method
bool match(string lhs, string rhs) {
	if (lhs == rhs) return true;
	else if (rhs == "") return true;
	else return false;
}

// Try and evaluate the current operation from QSO
bool qso_operation::evaluate_qso() {
	// Try an analyse existing QTHs for match
	set<string> matching_qths;
	bool ok = true;
	if (current_qso_) {
		bool nodata = true;
		for (auto it : *stn_data_->get_qths()) {
			bool possible = true;
			for (auto ita : it.second->data) {
				string data;
				switch (ita.first) {
				case STREET:
				case CITY:
					data = ita.second;
					break;
				default:
					data = to_upper(ita.second);
					break;
				}
				if (!match(current_qso_->item(QTH_ADIF_MAP.at(ita.first)), data)) possible = false;
				if (current_qso_->item(QTH_ADIF_MAP.at(ita.first)).length()) nodata = false;
			}
			if (possible) matching_qths.insert(it.first);
		}
		if (nodata) {
			// Probably a new QSO
			ok = true;
		}
		else {
			switch (matching_qths.size()) {
			case 0:
				status_->misc_status(ST_WARNING, "DASH: No QTHs match current QSO");
				current_qth_ = "";
				ok = false;
				break;
			case 1:
				current_qth_ = *matching_qths.begin();
				break;
			default:
				status_->misc_status(ST_WARNING, "DASH: Multiple QTHs match QSO - using first match");
				current_qth_ = *matching_qths.begin();
				break;
			}
		}
		// Try an analyse existing Operators for match
		set<string> matching_opers;
		nodata = true;
		for (auto it : *stn_data_->get_opers()) {
			bool possible = true;
			for (auto ita : it.second->data) {
				string data;
				switch (ita.first) {
				case NAME:
					data = ita.second;
					break;
				default:
					data = to_upper(ita.second);
					break;
				}
				if (!match(current_qso_->item(OPER_ADIF_MAP.at(ita.first)), data)) possible = false;
				if (current_qso_->item(OPER_ADIF_MAP.at(ita.first)).length()) nodata = false;
			}
			if (possible) matching_opers.insert(it.first);
		}
		if (nodata) {
			// Probably a new QSO
			ok = true;
		}
		else {
			switch (matching_opers.size()) {
			case 0:
				status_->misc_status(ST_WARNING, "DASH: No Operators match current QSO");
				current_oper_ = "";
				ok = false;
				break;
			case 1:
				current_oper_ = *matching_opers.begin();
				break;
			default:
				status_->misc_status(ST_WARNING, "DASH: Multiple Operators match QSO - using first match");
				current_oper_ = *matching_opers.begin();
				break;
			}
		}
		// Just use the current station callsign
		current_call_ = current_qso_->item("STATION_CALLSIGN");
		if (!current_call_.length()) {
			status_->misc_status(ST_WARNING, "DASH: No Station callsign in current QSO");
		}
	}
	else {
		// No QSL to evaluate
		ok = false;
	}
	return ok;
}

// Handle new QTH 
void qso_operation::new_qth() {
	config_->show();
	stn_dialog* dlg = (stn_dialog*)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::QTH, current_qth_);
	enable_widgets();
}

// handle new Operator
void qso_operation::new_oper() {
	config_->show();
	stn_dialog* dlg = (stn_dialog*)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::OPERATOR, current_oper_);
	enable_widgets();
}

// Handle new call
void qso_operation::new_call() {
	config_->show();
	stn_dialog* dlg = (stn_dialog*)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::CALLSIGN, current_call_);
	enable_widgets();
}

// Copy the values mentioned into the current QSO
void qso_operation::update_qso(record* qso) {
	char msg[128];
	// Copy all values from QTH, Operator & callsign to QSO
	const qth_info_t* qth = stn_data_->get_qth(current_qth_);
	if (qth) {
		for (auto it : QTH_ADIF_MAP) {
			if (qth->data.find(it.first) != qth->data.end()) {
				qso->item(it.second, qth->data.at(it.first));
			}
		}
	}
	else {
		snprintf(msg, sizeof(msg), "DASH: Invalid QTH %s", current_qth_.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	const oper_info_t* oper = stn_data_->get_oper(current_oper_);
	if (oper) {
		for (auto it : OPER_ADIF_MAP) {
			if (oper->data.find(it.first) != oper->data.end()) {
				qso->item(it.second, oper->data.at(it.first));
			}
		}
	}
	else {
		snprintf(msg, sizeof(msg), "DASH: Invalid Oprator %s", current_oper_.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	qso->item("STATION_CALLSIGN", current_call_);
}


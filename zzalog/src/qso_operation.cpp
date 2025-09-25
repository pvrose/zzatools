#include "qso_operation.h"

#include "config.h"
#include "qso_data.h"
#include "record.h"
#include "status.h"
#include "stn_data.h"
#include "stn_dialog.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Preferences.H>

extern config *config_;
extern status *status_;
extern stn_data *stn_data_;
extern stn_default station_defaults_;

extern std::string PROGRAM_ID;
extern std::string VENDOR;
extern void open_html(const char*);

qso_operation::qso_operation(int X, int Y, int W, int H, const char *L) : Fl_Group(X, Y, W, H, L),
																		  current_qth_(""),
																		  current_oper_(""),
																		  current_qso_(nullptr)
{
	tooltip("Displays the current station opertaion: QTH, Operator and callsign used");
	label("Station");
	box(FL_BORDER_BOX);
	load_data();
	create_form();
	populate_choices();
	enable_widgets();
}

qso_operation::~qso_operation()
{
	store_data();
}

// Handle
int qso_operation::handle(int event) {
	int result = Fl_Group::handle(event);
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
			open_html("qso_operation.html");
			return true;
		}
		break;
	}
	return result;
}
void qso_operation::create_form()
{
	int curr_x = x() + GAP;
	int curr_y = y() + GAP;

	curr_x += WLABEL + GAP;
	ch_qth_ = new annotated_choice(curr_x, curr_y, WSMEDIT, HBUTTON, "QTH");
	ch_qth_->align(FL_ALIGN_LEFT);
	ch_qth_->callback(cb_qth, &current_qth_);
	//ch_qth_->when(FL_WHEN_ENTER_KEY_ALWAYS);
	ch_qth_->tooltip("Select the current operating location (or eneter a new one)");

	curr_x += ch_qth_->w();
	Fl_Button *bn_show_qth = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Show");
	bn_show_qth->callback(cb_show, (void *)(intptr_t)stn_dialog::QTH);

	curr_x += bn_show_qth->w() + WLABEL;
	ch_oper_ = new annotated_choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Operator");
	ch_oper_->align(FL_ALIGN_LEFT);
	ch_oper_->callback(cb_oper, &current_oper_);
	//ch_oper_->when(FL_WHEN_ENTER_KEY_ALWAYS);
	ch_oper_->tooltip("Specify the current operator - select or enter new");

	curr_x += ch_qth_->w();
	Fl_Button *bn_show_oper = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Show");
	bn_show_oper->callback(cb_show, (void *)(intptr_t)stn_dialog::OPERATOR);

	curr_x += bn_show_oper->w() + WLABEL;
	ch_call_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Station\nCallsign");
	ch_call_->align(FL_ALIGN_LEFT);
	ch_call_->callback(cb_call, &current_call_);
	//ch_call_->when(FL_WHEN_ENTER_KEY_ALWAYS);
	ch_call_->tooltip("Specify the current station callsign");

	curr_x += ch_qth_->w();
	Fl_Button *bn_show_call = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "Show");
	bn_show_call->callback(cb_show, (void *)(intptr_t)stn_dialog::CALLSIGN);

	curr_x += bn_show_call->w() + GAP;
	curr_y += HBUTTON + GAP;

	resizable(nullptr);
	size(curr_x - x(), curr_y - y());
	end();
}

// Update the widget values
void qso_operation::enable_widgets()
{
	store_data();
	ch_qth_->value(current_qth_.c_str());
	ch_oper_->value(current_oper_.c_str());
	ch_call_->value(current_call_.c_str());
	ch_qth_->activate();
	ch_oper_->activate();
	ch_call_->activate();
}

// Load data
void qso_operation::load_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences station_settings(settings, "Station");
	char* temp;
	station_settings.get("Operator", temp, "");
	current_oper_ = temp;
	if (current_oper_.length() == 0 && station_defaults_.type != CLUB) {
		current_oper_ = station_defaults_.name;
	}
	free(temp);
	station_settings.get("Callsign", temp, "");
	current_call_ = to_upper(std::string(temp));
	if (current_call_.length() == 0) {
		current_call_ = station_defaults_.callsign;
	}
	free(temp);
	station_settings.get("Location", temp, "");
	current_qth_ = temp;
	if (current_qth_.length() == 0) {
		current_qth_ = station_defaults_.location;
	}
	free(temp);
}

// Store data
void qso_operation::store_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences station_settings(settings, "Station");
	station_settings.set("Operator", current_oper_.c_str());
	station_settings.set("Callsign", current_call_.c_str());
	station_settings.set("Location", current_qth_.c_str());
}

// QTH button clicked
void qso_operation::cb_qth(Fl_Widget *w, void *v)
{
	Fl_Input_Choice *ch = (Fl_Input_Choice *)w;
	qso_operation *that = ancestor_view<qso_operation>(w);
	*(std::string *)v = ch->value();
	if (!ch->menubutton()->changed())
	{
		// New QTH typed in - check if it's really new
		if (!stn_data_->known_qth(that->current_qth_))
		{
			that->new_qth();
			return;
		}
	}
}

void qso_operation::cb_oper(Fl_Widget *w, void *v)
{
	Fl_Input_Choice *ch = (Fl_Input_Choice *)w;
	qso_operation *that = ancestor_view<qso_operation>(w);
	*(std::string *)v = ch->value();
	if (!ch->menubutton()->changed())
	{
		// New QTH typed in - check if it's really new
		if (!stn_data_->known_oper(that->current_oper_))
		{
			that->new_oper();
			return;
		}
	}
}

void qso_operation::cb_call(Fl_Widget *w, void *v)
{
	Fl_Input_Choice *ch = (Fl_Input_Choice *)w;
	qso_operation *that = ancestor_view<qso_operation>(w);
	*(std::string *)v = ch->value();
	if (!ch->menubutton()->changed())
	{
		// New QTH typed in - check if it's really new
		if (!stn_data_->known_call(that->current_call_))
		{
			that->new_call();
			return;
		}
	}
}

void qso_operation::cb_show(Fl_Widget *w, void *v)
{
	qso_operation *that = ancestor_view<qso_operation>(w);
	stn_dialog::tab_type t = (stn_dialog::tab_type)(intptr_t)v;
	switch (t)
	{
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
// std::set the QSO
void qso_operation::qso(record *qso)
{
	current_qso_ = qso;
	enable_widgets();
}

// get the current QTH
std::string qso_operation::current_qth()
{
	return current_qth_;
}

std::string qso_operation::current_oper()
{
	return current_oper_;
}

std::string qso_operation::current_call()
{
	return current_call_;
}

// Populate the choices
void qso_operation::populate_choices()
{
	char l[64];
	// Populate QTH choice
	ch_qth_->clear();
	ch_qth_->add("");
	for (auto it : *stn_data_->get_qths())
	{
		std::map<qth_value_t, std::string> &data = it.second->data;
		snprintf(l, sizeof(l), "%s:--->%s,%s,%s",
					escape_menu(it.first).c_str(),
					data.find(STREET) == data.end() ? "" : data.at(STREET).c_str(),
					data.find(CITY) == data.end() ? "" : data.at(CITY).c_str(),
					data.find(LOCATOR) == data.end() ? "" : data.at(LOCATOR).c_str());
		ch_qth_->add(l);
		ch_qth_->menubutton()->user_data((void *)&it.first);
	}
	// Populate Operator choice
	ch_oper_->clear();
	ch_oper_->add("");
	for (auto it : *stn_data_->get_opers())
	{
		std::map<oper_value_t, std::string> &data = it.second->data;
		snprintf(l, sizeof(l), "%s:--->%s,%s",
			escape_menu(it.first).c_str(),
			data.find(NAME) == data.end() ? "" : data.at(NAME).c_str(),
			data.find(CALLSIGN) == data.end() ? "" : data.at(CALLSIGN).c_str());
		ch_oper_->add(l);
		ch_oper_->menubutton()->user_data((void *)&it.first);
	}
	// Populate callsign choice
	ch_call_->clear();
	ch_call_->add("");
	for (auto it : *stn_data_->get_calls())
	{
		ch_call_->add(escape_menu(it.first).c_str());
	}
}


// Handle new QTH
void qso_operation::new_qth()
{
	config_->show();
	stn_dialog *dlg = (stn_dialog *)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::QTH, current_qth_);
	enable_widgets();
}

// handle new Operator
void qso_operation::new_oper()
{
	config_->show();
	stn_dialog *dlg = (stn_dialog *)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::OPERATOR, current_oper_);
	enable_widgets();
}

// Handle new call
void qso_operation::new_call()
{
	config_->show();
	stn_dialog *dlg = (stn_dialog *)config_->get_tab(config::DLG_STATION);
	dlg->set_tab(stn_dialog::CALLSIGN, current_call_);
	enable_widgets();
}

// Copy the values mentioned into the current QSO
void qso_operation::update_qso(record *qso)
{
	char msg[128];
	// Copy all values from QTH, Operator & callsign to QSO
	const qth_info_t *qth = stn_data_->get_qth(current_qth_);
	if (qth)
	{
		for (auto it : QTH_ADIF_MAP)
		{
			if (qth->data.find(it.first) != qth->data.end())
			{
				qso->item(it.second, qth->data.at(it.first));
			}
		}
	}
	else
	{
		snprintf(msg, sizeof(msg), "DASH: Invalid QTH %s", current_qth_.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	const oper_info_t *oper = stn_data_->get_oper(current_oper_);
	if (oper)
	{
		for (auto it : OPER_ADIF_MAP)
		{
			if (oper->data.find(it.first) != oper->data.end())
			{
				qso->item(it.second, oper->data.at(it.first));
			}
		}
	}
	else
	{
		snprintf(msg, sizeof(msg), "DASH: Invalid Operator %s", current_oper_.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	qso->item("STATION_CALLSIGN", current_call_);
}

// Station details have been changed - update everything
void qso_operation::update_details() {
	load_data();
	populate_choices();
	enable_widgets();
}

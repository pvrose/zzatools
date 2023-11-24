#include "qso_entry.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "rig_if.h"
#include "status.h"
#include "spec_data.h"
#include "band_view.h"
#include "tabbed_forms.h"
#include "book.h"

extern status* status_;
extern spec_data* spec_data_;
//extern band_view* band_view_;
extern tabbed_forms* tabbed_forms_;
extern book* book_;

extern double prev_freq_;

#ifdef _WIN32
#include "dxa_if.h"
extern dxa_if* dxa_if_;
#endif

map <string, vector<string> > qso_entry::field_map_;

qso_entry::qso_entry(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, number_locked_(0)
	, qso_(nullptr)
	, qso_number_(-1)
	, original_qso_(nullptr)
	, previous_serial_(0)
{
	// Get the enclosing qso_data instance
	qso_data_ = ancestor_view<qso_data>(this);
	box(FL_BORDER_BOX);

	for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
		ip_field_[ix] = nullptr;
		ch_field_[ix] = nullptr;
	}

	// Initialise field input map
	field_ip_map_.clear();
	load_values();
	create_form(X, Y);
	enable_widgets();
}

qso_entry::~qso_entry() {
	save_values();
}

// When shown set the focus to the most likely input widgets
int qso_entry::handle(int event) {
	switch (event) {
	case FL_SHOW:
		set_initial_focus();
		return 1;
	case FL_HIDE:
		// Don't do anything but absorb the event
		return 1;
	}
	return Fl_Group::handle(event);
}

// Nothing so far
void qso_entry::load_values() {
}

// Nothing so far
void qso_entry::save_values() {
}

void qso_entry::create_form(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	int col2_y = curr_y;
	int max_x = X;
	int max_y = Y;

	curr_x += GAP;
	curr_y += HTEXT;

	int save_y = curr_y;

	// Fixed fields
	// N rows of NUMBER_PER_ROW
	const int NUMBER_PER_ROW = 2;
	const int WCHOICE = WBUTTON * 3 / 2;
	const int WINPUT = WBUTTON * 7 / 4;
	for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
		if (ix >= NUMBER_FIXED) {
			ch_field_[ix] = new field_choice(curr_x, curr_y, WCHOICE, HBUTTON);
			ch_field_[ix]->align(FL_ALIGN_RIGHT);
			ch_field_[ix]->tooltip("Specify the field to provide");
			ch_field_[ix]->callback(cb_ch_field, (void*)(intptr_t)ix);
			ch_field_[ix]->set_dataset("Fields");
		}
		curr_x += WCHOICE;
		ip_field_[ix] = new field_input(curr_x, curr_y, WINPUT, HBUTTON);
		ip_field_[ix]->align(FL_ALIGN_LEFT);
		ip_field_[ix]->tooltip("Enter required value to log");
		ip_field_[ix]->callback(cb_ip_field, (void*)(intptr_t)ix);
		ip_field_[ix]->input()->when(FL_WHEN_RELEASE_ALWAYS);
		if (ix < NUMBER_FIXED) {
			ip_field_[ix]->field_name(fixed_names_[ix].c_str(), qso_);
			field_ip_map_[fixed_names_[ix]] = ix;
			field_names_[ix] = fixed_names_[ix];
			ip_field_[ix]->label(fixed_names_[ix].c_str());
		}
		else {
			field_names_[ix] = "";
		}
		number_fields_in_use_ = NUMBER_FIXED;
		curr_x += ip_field_[ix]->w() + GAP;
		if (ix % NUMBER_PER_ROW == (NUMBER_PER_ROW - 1)) {
			max_x = max(max_x, curr_x);
			curr_x = X + GAP;
			curr_y += HBUTTON;
		}
	}
	max_x = max(max_x, curr_x);
	max_y = curr_y;
	// Clear QSO fields

	curr_x = max_x;
	curr_y = save_y;

	misc_ = new qso_misc(curr_x, curr_y, WCHOICE + WINPUT + GAP, max_y - curr_y);
	curr_x += misc_->w();
	curr_y += misc_->h();
	max_x = max(max_x, misc_->x() + misc_->w()) + GAP;
	max_y = max(max_y, misc_->y() + misc_->h());

	// nOtes input
	curr_x = X + WCHOICE;
	curr_y += HBUTTON;

	ip_notes_ = new intl_input(curr_x, curr_y, max_x - curr_x - GAP, HBUTTON, "NOTES");
	ip_notes_->callback(cb_ip_notes, nullptr);
	ip_notes_->when(FL_WHEN_RELEASE_ALWAYS);
	ip_notes_->tooltip("Add any notes for the QSO");

	curr_y += HBUTTON + GAP;
	resizable(nullptr);
	size(max_x - X, curr_y - Y);
	end();

	initialise_fields();
}

void qso_entry::enable_widgets() {
	// Now enable disan=
	switch (qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->deactivate();
		misc_->deactivate();
		break;
	case qso_data::QSO_PENDING:
		for (int ix = 0; ix <= number_fields_in_use_ && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->input()->color(FL_BACKGROUND_COLOR);
			ip_field_[ix]->type(FL_NORMAL_INPUT);
		}
		for (int ix = number_fields_in_use_ + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND_COLOR);
		ip_notes_->type(FL_NORMAL_INPUT);
		misc_->activate();
		misc_->enable_widgets();
		break;
	case qso_data::QSO_STARTED:
	case qso_data::NET_STARTED:
	case qso_data::QSO_MODEM:
	case qso_data::QSO_EDIT:
	case qso_data::NET_EDIT:
	case qso_data::QSO_ENTER:
		for (int ix = 0; ix <= number_fields_in_use_ && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->input()->color(FL_BACKGROUND_COLOR);
			ip_field_[ix]->type(FL_NORMAL_INPUT);
		}
		for (int ix = number_fields_in_use_ + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND_COLOR);
		ip_notes_->type(FL_NORMAL_INPUT);
		misc_->activate();
		misc_->enable_widgets();
		break;
	case qso_data::QSO_VIEW:
		for (int ix = 0; ix <= number_fields_in_use_ && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->input()->color(FL_BACKGROUND_COLOR);
			ip_field_[ix]->type(FL_NORMAL_OUTPUT);
		}
		for (int ix = number_fields_in_use_ + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND_COLOR);
		ip_notes_->type(FL_NORMAL_OUTPUT);
		misc_->activate();
		misc_->enable_widgets();
		break;
	case qso_data::MANUAL_ENTRY:
		for (int ix = 0; ix <= number_fields_in_use_ && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->input()->color(FL_BACKGROUND_COLOR);
			ip_field_[ix]->type(FL_NORMAL_INPUT);
		}
		for (int ix = number_fields_in_use_ + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND_COLOR);
		ip_notes_->type(FL_NORMAL_INPUT);
		misc_->deactivate();
		break;
	default:
		// Reserver=d for Query states
		hide();
		break;
	}
	set_initial_focus();
}

// Copy record to the fields - reverse of above
void qso_entry::copy_qso_to_display(int flags) {
	if (qso_) {
		// Uodate band
		qso_->update_band();
		// For each field input
		for (int i = 0; i < NUMBER_TOTAL; i++) {
			string field;
			if (i < NUMBER_FIXED) field = fixed_names_[i];
			else field = ch_field_[i]->value();
			if (field.length()) {
				if (flags == CF_ALL_FLAGS) {
					// Copy all fields that have edit fields defined
					ip_field_[i]->value(qso_->item(field, false, true).c_str());
				}
				else {
					// Copy per flag bits
					for (auto sf = COPY_SET.begin(); sf != COPY_SET.end(); sf++) {
						copy_flags f = (*sf);
						if (flags & f) {
							for (auto fx = COPY_FIELDS.at(f).begin(); fx != COPY_FIELDS.at(f).end(); fx++) {
								if ((*fx) == field)	ip_field_[i]->value(qso_->item(field, false, true).c_str());
							}
						}
					}
				}
				if (field == "STATE") {
					ip_field_[i]->reload_choice(qso_);
				}
			}
		}
		// Handle NOTES separately 
		if (flags == CF_ALL_FLAGS) {
			// ALL_FLAGS includes NOTES
			ip_notes_->value(qso_->item("NOTES", false, true).c_str());
		} else {
			// Check if each flag includes it
			for (auto sf = COPY_SET.begin(); sf != COPY_SET.end(); sf++) {
				copy_flags f = (*sf);
				if (flags & f) {
					for (auto fx = COPY_FIELDS.at(f).begin(); fx != COPY_FIELDS.at(f).end(); fx++) {
						if ((*fx) == "NOTES")	
							ip_notes_->value(qso_->item("NOTES", false, true).c_str());
					}
				}
			}
		}
		// If QTH changes tell DXA-IF to update home_location
		switch (qso_data_->logging_state()) {
		case qso_data::QSO_EDIT:
		case qso_data::QSO_VIEW:
		case qso_data::QSO_INACTIVE:
		case qso_data::QSO_PENDING:
		case qso_data::QSO_STARTED:
		case qso_data::QSO_ENTER:
			check_qth_changed();
			break;
		case qso_data::NET_ADDING:
		case qso_data::NET_EDIT:
		case qso_data::NET_STARTED:
			break;
		}
		if (flags == CF_ALL_FLAGS || (flags & CF_DETAILS)) {
			misc_->qso(qso_, qso_number_);
			misc_->enable_widgets();
		}
	}
	else {
		// Clear all fields
		for (int i = 0; i < NUMBER_TOTAL; i++) {
			string field;
			if (i < NUMBER_FIXED) field = fixed_names_[i];
			else field = ch_field_[i]->value();
			if (field.length()) {
				ip_field_[i]->value("");
			}
		}
		ip_notes_->value("");
	}
}

// Copy from an existing record: fields depend on flags set
void qso_entry::copy_qso_to_qso(record* old_record, int flags) {
	// Create a new record
	qso(-1);
	if (old_record) {
		// printf("DEBUG: Copying QSO %p %s %s %s %s F=%d \n", old_record,
		// old_record->item("QSO_DATE").c_str(),
		// old_record->item("TIME_ON").c_str(), old_record->item("CALL").c_str(),
		// old_record->item("APP_ZZA_OP").c_str(), flags);
		// For all flag bits
		for (auto sf = COPY_SET.begin(); sf != COPY_SET.end(); sf++) {
			copy_flags f = (*sf);
			for (auto fx = COPY_FIELDS.at(f).begin(); fx != COPY_FIELDS.at(f).end(); fx++) {
				if (flags & f) {
					// If it's set copy it
					qso_->item((*fx), old_record->item((*fx)));
				}
				else {
					// else clear it
					qso_->item(string(""));
				}
			}
		}
	}
	copy_qso_to_display(CF_ALL_FLAGS);
}

// Copy fields from CAT and default rig etc.
void qso_entry::copy_cat_to_qso() {
	rig_if* rig = ((qso_manager*)qso_data_->parent())->rig();
	// Rig can be temporarily missing q
	if (rig && rig->is_good() && qso_ != nullptr) {
		string freqy = rig->get_frequency(true);
		string mode;
		string submode;
		rig->get_string_mode(mode, submode);
		// Get the maximum power over course of QSO.
		double tx_power;
		qso_->item("TX_PWR", tx_power);
		tx_power = max(tx_power, rig->get_dpower(true));
		switch (qso_data_->logging_state()) {
		case qso_data::QSO_PENDING:
		case qso_data::NET_STARTED:
		{
			// Load values from rig
			qso_->item("FREQ", freqy);
			// Get mode - NB USB/LSB need further processing
			if (mode != "DATA L" && mode != "DATA U") {
				qso_->item("MODE", mode);
				qso_->item("SUBMODE", submode);
			}
			else {
				qso_->item("MODE", string(""));
				qso_->item("SUBMODE", string(""));
			}
			qso_->item("TX_PWR", tx_power);
			break;
		}
		case qso_data::QSO_STARTED: {
			// Ignore values except TX_PWR which accumulates maximum value
			char message[128];
			if (qso_->item("FREQ") != freqy) {
				snprintf(message, 128, "DASH: Rig frequency changed during QSO, New value %s", freqy.c_str());
				status_->misc_status(ST_WARNING, message);
				qso_->item("FREQ", freqy);
			}
			if (qso_->item("MODE") != mode) {
				snprintf(message, 128, "DASH: Rig mode changed during QSO, New value %s", mode.c_str());
				status_->misc_status(ST_WARNING, message);
				qso_->item("MODE", mode);
			}
			if (qso_->item("SUBMODE") != submode) {
				snprintf(message, 128, "DASH: Rig submode changed during QSO, New value %s", submode.c_str());
				status_->misc_status(ST_WARNING, message);
				qso_->item("SUBMODE", submode);
			}
			qso_->item("TX_PWR", tx_power);
			break;
		}
		}
		copy_qso_to_display(CF_CAT);
	}
}

// Copy current timestamp to QSO
void qso_entry::copy_clock_to_qso() {
	// Only allow this if it hasn't been done 
	if (qso_ != nullptr && qso_->item("TIME_OFF").length() == 0) {
		switch (qso_data_->logging_state()) {
		case qso_data::QSO_INACTIVE:
		case qso_data::QSO_PENDING:
		case qso_data::NET_ADDING:
		{
			time_t now = time(nullptr);
			tm* value = gmtime(&now);
			char result[100];
			// convert date
			strftime(result, 99, "%Y%m%d", value);
			qso_->item("QSO_DATE", string(result));
			// convert time
			strftime(result, 99, "%H%M%S", value);
			qso_->item("TIME_ON", string(result));
			qso_->item("QSO_DATE_OFF", string(""));
			qso_->item("TIME_OFF", string(""));
			copy_qso_to_display(CF_TIME | CF_DATE);
			break;
		}
		}
	}
}

// Copy default values to QSO
void qso_entry::copy_default_to_qso() {
	if (qso_) {
		record* latest = book_->get_latest();
		qso_->item("STATION_CALLSIGN", latest->item("STATION_CALLSIGN"));
		qso_->item("APP_ZZA_QTH", latest->item("APP_ZZA_QTH"));
		qso_->item("MY_RIG", latest->item("MY_RIG"));
		qso_->item("MY_ANTENNA", latest->item("MY_ANTENNA"));
		qso_->item("APP_ZZA_OP", latest->item("APP_ZZA_OP"));
	}
}

// Clear fields
void qso_entry::clear_display() {
	for (int i = 0; i < NUMBER_TOTAL; i++) {
		ip_field_[i]->value("");
	}
	ip_notes_->value("");
}

// Clear fields in current QSO
void qso_entry::clear_qso() {
	qso_->delete_contents();
	copy_qso_to_display(CF_QSO);
}

// Initialise field map
void qso_entry::initialise_field_map() {
	field_ip_map_.clear();
	for (int ix = 0; ix < NUMBER_FIXED; ix++) {
		field_ip_map_[fixed_names_[ix]] = ix;
	}
}

// Initialise fields
void qso_entry::initialise_fields() {
	// Now set fields
	string contest = qso_ ? qso_->item("CONTEST_ID") : "";
	if (contest == "") {
		if (field_map_.find("None") == field_map_.end()) {
			field_map_["None"] = DEFAULT_NONTEST;
		}
		contest = "None";
	} else {
		if (field_map_.find(contest) == field_map_.end()) {
			field_map_[contest] = DEFAULT_CONTEST;
		}
	}
	vector<string>& field_names = field_map_.at(contest);
	// Clear field map
	initialise_field_map();
	size_t ix = 0;
	int iy;
	for (ix = 0, iy = NUMBER_FIXED; ix < field_names.size(); ix++, iy++) {
		ch_field_[iy]->value(field_names[ix].c_str());
		ip_field_[iy]->field_name(field_names[ix].c_str(), qso_);
		field_names_[iy] = field_names[ix];
	}
	number_fields_in_use_ = iy;
	for (; iy < NUMBER_TOTAL; iy++) {
		ch_field_[iy]->value("");
		ip_field_[iy]->value("");
		ip_field_[iy]->field_name("");
	}
}

// Initialise the values of the above fields
void qso_entry::initialise_values() {
	string contest = qso_ ? qso_->item("CONTEST_ID") : "";
	if (contest == "") contest = "None";
	vector<string>& fields = field_map_.at(contest);
	int ix = NUMBER_FIXED;
	for (size_t i = 0; i < fields.size(); i++, ix++) {
		if (qso_) {
			if (contest != "None") {
				string contest_mode = spec_data_->dxcc_mode(qso_->item("MODE"));
				if (fields[i] == "RST_SENT" || fields[i] == "RST_RCVD") {
					if (contest_mode == "CW" || contest_mode == "DATA") {
						qso_->item(fields[i], string("599"));
					}
					else {
						qso_->item(fields[i], string("59"));
					}
				}
				else if (fields[i] == "STX") {
					char text[10];
					snprintf(text, 10, "%03d", ++previous_serial_);
					qso_->item(fields[i], string(text));
				}
				if (fields[i] == "CALL") {
					ip_field_[ix]->value("");
				}
				else {
					ip_field_[ix]->value(qso_->item(fields[i]).c_str());
				}
			}
			else {
				ip_field_[ix]->value(qso_->item(fields[i]).c_str());
			}
		}
		else {
			ip_field_[ix]->value("");
		}
	}
	for (; ix < NUMBER_TOTAL; ix++) {
		ip_field_[ix]->value("");
	}
	ip_notes_->value("");
}

// Return fields that have been defines as comma seperated list
string qso_entry::get_defined_fields() {
	string defn = "";
	for (int i = NUMBER_FIXED; i < NUMBER_TOTAL; i++) {
		const char* field = ch_field_[i]->value();
		if (strlen(field)) {
			if (i > NUMBER_FIXED) {
				defn += ",";
			}
			defn += field;
		}
	}
	return defn;
}

// Action add field - add the selected field to the set of entries
void qso_entry::action_add_field(int ix, string field) {
	if (ix == -1 && number_fields_in_use_ == NUMBER_TOTAL) {
		char msg[128];
		snprintf(msg, 128, "DASH: Cannot add any more fields to edit - %s ignored", field.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	else if (ix >= 0 && ix < number_fields_in_use_) {
		const char* old_field = field_names_[ix].c_str();
		ip_field_[ix]->field_name(field.c_str(), qso_);
		ip_field_[ix]->value(qso_->item(field).c_str());
		// Change mapping
		if (strlen(old_field)) {
			field_ip_map_.erase(old_field);
		}
		field_ip_map_[field] = ix;
		field_names_[ix] = field;
	}
	else if (ix == number_fields_in_use_) {
		if (field_ip_map_.find(field) == field_ip_map_.end()) {
			ch_field_[number_fields_in_use_]->value(field.c_str());
			ip_field_[number_fields_in_use_]->field_name(field.c_str(), qso_);
			ip_field_[number_fields_in_use_]->value(qso_->item(field).c_str());
			field_ip_map_[field] = number_fields_in_use_;
			field_names_[number_fields_in_use_] = field;
			number_fields_in_use_++;
		}
		else {
			char msg[128];
			snprintf(msg, sizeof(msg), "DASH: Already have a field for %s, new field ignored", field.c_str());
			status_->misc_status(ST_ERROR, msg);
		}
	}
	else {
		status_->misc_status(ST_SEVERE, "DASH: Trying to select a deactivated widget");
	}

	// Save the altered field names
	string contest = qso_ ? qso_->item("CONTEST_ID") : "";
	if (contest == "") contest = "None";
	field_map_[contest].clear();
	for (int iy = NUMBER_FIXED; iy < number_fields_in_use_; iy++) {
		field_map_[contest].push_back(field_names_[iy]);
	}
	enable_widgets();
}

// Delete a field
void qso_entry::action_del_field(int ix) {
	string& old_field = field_names_[ix];
	field_ip_map_.erase(old_field);
	int pos = ix;
	for (; pos < number_fields_in_use_ - 1; pos++) {
		string& field = field_names_[pos + 1];
		field_names_[pos] = field;
		ch_field_[pos]->value(field.c_str());
		ip_field_[pos]->field_name(field.c_str(), qso_);
		ip_field_[pos]->value(qso_->item(field).c_str());
	}
	ch_field_[pos]->value("");
	ip_field_[pos]->field_name("");
	ip_field_[pos]->value("");
	number_fields_in_use_--;

	// Save the altered field names
	string contest = qso_ ? qso_->item("CONTEST_ID") : "";
	if (contest == "") contest = "None";
	field_map_[contest].clear();
	for (int iy = NUMBER_FIXED; iy < number_fields_in_use_; iy++) {
		field_map_[contest].push_back(field_names_[iy]);
	}

	enable_widgets();
}


// Callback change field selected
void qso_entry::cb_ch_field(Fl_Widget* w, void* v) {
	qso_entry* that = ancestor_view<qso_entry>(w);
	field_choice* ch = (field_choice*)w;
	const char* field = ch->value();
	int ix = (int)(intptr_t)v;
	if (strlen(field)) {
		that->action_add_field(ix, field);
	}
	else {
		that->action_del_field(ix);
	}
}

// Callback - general input
// v - index of input widget
void qso_entry::cb_ip_field(Fl_Widget* w, void* v) {
	qso_entry* that = ancestor_view<qso_entry>(w);
	field_input* ip = (field_input*)w;
	string field = ip->field_name();
	string value = ip->value();
	that->qso_->item(field, value);

	if (field == "FREQ") {
		double freq_MHz = atof(value.c_str());
		double freq_kHz = freq_MHz * 1000.0;
		//if (band_view_) {
		//	band_view_->update(freq_kHz);
		//}
		prev_freq_ = freq_kHz;
		that->qso_->item("BAND", spec_data_->band_for_freq(freq_MHz));
	}
	else if (field == "MODE") {
		if (spec_data_->is_submode(value)) {
			that->qso_->item("SUBMODE", value);
			that->qso_->item("MODE", spec_data_->mode_for_submode(value));
		}
		else {
			that->qso_->item("MODE", value);
			that->qso_->item("SUBMODE", string(""));
		}
	}
	else if (field == "APP_ZZA_QTH") {
		// Send new value to spec_data to create an empty entry if it's a new one
		if (!ip->menubutton()->changed()) {
			macro_defn entry = { nullptr, "" };
			spec_data_->add_user_macro(field, value, entry);
		}
		that->check_qth_changed();
	}
	else if (field == "APP_ZZA_OP") {
		// Send new value to spec_data to create an empty entry if it's a new one
		if (!ip->menubutton()->changed()) {
			macro_defn entry = { nullptr, "" };
			spec_data_->add_user_macro(field, value, entry);
		}
		that->check_qth_changed();
	}
	else if (field == "MY_RIG") {
		// Update the selected rig CAT group
		((qso_manager*)that->qso_data_->parent())->change_rig(value);
	}
	else if (field == "QSO_DATE" || field == "TIME_ON") {
		if (that->qso_number_ != -1) {
			// Reposition QSO in book
			item_num_t item = book_->item_number(that->qso_number_);
			item = book_->correct_record_position(item);
			that->qso_number_ = book_->record_number(item);
		}
	}
	else if (field == "CALL") {
		// Remove any dependent fields that may be left over from previous edits
		that->qso_->unparse();
	}
	// Update other views if editing or logging
	switch (that->qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
	case qso_data::QSO_VIEW:
		break;
	case qso_data::NET_STARTED:
	case qso_data::NET_EDIT:
		if (field == "CALL") {
			that->copy_label(that->qso_->item("CALL").c_str());
			that->parent()->redraw();
		}
		// drop through
	case qso_data::QSO_PENDING:
	case qso_data::QSO_STARTED:
	case qso_data::QSO_EDIT:
	case qso_data::QSO_ENTER:
		that->enable_widgets();
		that->qso_data_->enable_widgets();
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, that->qso_number_);
#ifdef WIN32
		if (dxa_if_) {
			if (field == "CALL" || field == "GRIDSQUARE") {
				// Place DX Location flag in DxAtlas
				string call = that->qso_->item("CALL");
				string grid = that->qso_->item("GRIDSQUARE");
				if (call.length()) {
					if (grid.length()) {
						dxa_if_->set_dx_loc(grid, call);
					}
					else {
						dxa_if_->set_dx_loc(call);
					}
				}
				else {
					dxa_if_->clear_dx_loc();
				}
			}
		}
#endif
		break;
	default:
		break;
	}
}

// Callback -notes input
// v - not used
void qso_entry::cb_ip_notes(Fl_Widget* w, void* v) {
	qso_entry* that = ancestor_view<qso_entry>(w);
	string notes;
	cb_value<intl_input, string>(w, &notes);
	if (that->qso_) {
		that->qso_->item("NOTES", notes);
		tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, that->qso_number_);
	}
}

// Get current qso
record* qso_entry::qso() {
	return qso_;
}

// Set current qso
void qso_entry::qso(qso_num_t number) {
	if (qso_number_ == -1 || qso_number_ != number) {
		qso_number_ = number;
		if (number == -1) {
			qso_ = new record();
		}
		else {
			qso_ = book_->get_record(number, false);
		}
		delete original_qso_;
		original_qso_ = new record(*qso_);
	}
}

// Set current qso as supplied on
void qso_entry::qso(record* qso) {
	qso_number_ = -1;
	qso_ = qso;
}

// Get original qso
record* qso_entry::original_qso() {
	return original_qso_;
}

// Get current number
qso_num_t qso_entry::qso_number() {
	return qso_number_;
}

// Append QSO to book
void qso_entry::append_qso() {
	qso_number_ = book_->insert_record(qso_);
}

// DElete QSO
void qso_entry::delete_qso() {
	if (qso_number_ == -1) {
		// QSO is not in the book yet
		delete qso_;
		qso_ = nullptr;
		delete original_qso_;
		original_qso_ = nullptr;
	}
	else {
		// QSO is in the book, remove this reference
		qso_ = nullptr;
		qso_number_ = -1;
		delete original_qso_;
		original_qso_ = nullptr;
	}
}

// Set initial focus - try QSO_DATE then CALL 
void qso_entry::set_initial_focus() {
	if (field_ip_map_.find("QSO_DATE") != field_ip_map_.end()) {
		field_input* w = ip_field_[field_ip_map_["QSO_DATE"]];
		if (w && strlen(w->value()) == 0) {
			if (w->take_focus()) return;
		}
	}
	if (field_ip_map_.find("CALL") != field_ip_map_.end()) {
		field_input* w = ip_field_[field_ip_map_["CALL"]];
		if (w && strlen(w->value()) == 0) {
			if (w->take_focus()) return;
		}
	}
}

// Check if QTH has changed and action change (redraw DxAtlas
void qso_entry::check_qth_changed() {
	if (qso_) {
		if (qso_->item("MY_GRIDSQUARE", true) != previous_locator_ ||
			qso_->item("APP_ZZA_QTH") != previous_qth_) {
			previous_locator_ = qso_->item("MY_GRIDSQUARE", true);
			previous_qth_ = qso_->item("APP_ZZA_QTH");
#ifdef _WIN32
			if (dxa_if_) dxa_if_->update(HT_LOCATION);
#endif
		}
	}
}

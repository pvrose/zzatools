#include "qso_entry.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "qso_contest.h"
#include "qso_rig.h"
#include "rig_if.h"
#include "status.h"
#include "spec_data.h"
#include "tabbed_forms.h"
#include "book.h"
#include "record.h"
#include "ticker.h"
#include "fields.h"
#include "intl_widgets.h"
#include "field_choice.h"

extern status* status_;
extern spec_data* spec_data_;
extern tabbed_forms* tabbed_forms_;
extern book* book_;
extern bool DARK;
extern ticker* ticker_;
extern fields* fields_;

extern double prev_freq_;

// Map showing what fields to display for what usage
// Used for contest, but not currently used
collection_t* qso_entry::field_map_ = nullptr;

// Constructor
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

	// Add 1s ticker
	ticker_->add_ticker(this, cb_ticker, 10);
}

// Destructor
qso_entry::~qso_entry() {
	save_values();
	ticker_->remove_ticker(this);
}

// When shown set the focus to the most likely input widgsets
int qso_entry::handle(int event) {
	int result = Fl_Group::handle(event);
	switch (event) {
	case FL_SHOW:
		set_initial_focus();
		break;
	}
	return result;
}

// Nothing so far
void qso_entry::load_values() {
}

// Nothing so far
void qso_entry::save_values() {
}

// Create the widgets
void qso_entry::create_form(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	int col2_y = curr_y;
	int max_x = X;
	int max_y = Y;

	curr_x += GAP;
	curr_y += HTEXT;

	int save_y = curr_y;

	// N rows of NUMBER_PER_ROW
	const int NUMBER_PER_ROW = 2;
	const int WCHOICE = WBUTTON * 3 / 2;
	const int WINPUT = WBUTTON * 7 / 4;

	// For all the fields we can support
	for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
		// Only allow the userto choose the filed to display for those
		// inputs that are not for fixed fields
		if (ix >= NUMBER_FIXED) {
			ch_field_[ix] = new field_choice(curr_x, curr_y, WCHOICE, HBUTTON);
			ch_field_[ix]->align(FL_ALIGN_RIGHT);
			ch_field_[ix]->tooltip("Specify the field to provide");
			ch_field_[ix]->callback(cb_ch_field, (void*)(intptr_t)ix);
			ch_field_[ix]->set_dataset("Fields");
		}
		curr_x += WCHOICE;
		// Add a field_inpu widget
		ip_field_[ix] = new field_input(curr_x, curr_y, WINPUT, HBUTTON);
		ip_field_[ix]->align(FL_ALIGN_LEFT);
		ip_field_[ix]->tooltip("Enter required value to log");
		ip_field_[ix]->callback(cb_ip_field, (void*)(intptr_t)ix);
		ip_field_[ix]->input()->when(FL_WHEN_RELEASE_ALWAYS);
		if (ix < NUMBER_FIXED) {
			// For a fixed field add the name of the field as a label where
			// the field choice would have been
			string name = STN_FIELDS[ix];
			ip_field_[ix]->field_name(name.c_str(), qso_);
			field_ip_map_[name] = ix;
			ip_field_[ix]->copy_label(name.c_str());
		}

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

	// nOtes input
	curr_x = X + WCHOICE;
	curr_y += GAP;

	// Input for the NOTES field of the QSO record
	ip_notes_ = new intl_input(curr_x, curr_y, max_x - curr_x - GAP, HBUTTON, "NOTES");
	ip_notes_->callback(cb_ip_notes, nullptr);
	ip_notes_->when(FL_WHEN_CHANGED);
	ip_notes_->tooltip("Add any notes for the QSO");

	curr_y += HBUTTON + GAP;
	resizable(nullptr);
	size(max_x - X, curr_y - Y);
	end();

	initialise_fields();
}

// Configure the various widgets
void qso_entry::enable_widgets() {
	// Now enable disan=
	switch (qso_data_->logging_state()) {
	case qso_data::QSO_INACTIVE:
		// Doing nothing - deactivate all widgets
		for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix]) ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
			ip_field_[ix]->qso(nullptr);
		}
		ip_notes_->deactivate();
		set_initial_focus();
		break;
	case qso_data::QSO_PENDING:
	case qso_data::TEST_PENDING:
		// Waiting to start a QSO - activate all fields in use ...
		for (int ix = 0; ix <= fields_in_use_.size() && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->qso(qso_);
			ip_field_[ix]->input()->color(FL_BACKGROUND2_COLOR);
			ip_field_[ix]->type(FL_NORMAL_INPUT);
		}
		// ... and deactivate the others
		for (int ix = fields_in_use_.size() + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
			ip_field_[ix]->qso(nullptr);
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND2_COLOR);
		ip_notes_->type(FL_NORMAL_INPUT);
		set_initial_focus();
		break;
	case qso_data::QSO_STARTED:
	case qso_data::NET_STARTED:
	case qso_data::QSO_WSJTX:
	case qso_data::QSO_FLDIGI:
	case qso_data::QSO_EDIT:
	case qso_data::NET_EDIT:
	case qso_data::QSO_ENTER:
	case qso_data::TEST_ACTIVE:
		// Entering a QSO - activate all fields in use ...
		for (int ix = 0; ix <= fields_in_use_.size() && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->qso(qso_);
			ip_field_[ix]->input()->color(FL_BACKGROUND2_COLOR);
			ip_field_[ix]->type(FL_NORMAL_INPUT);
		}
		// ... and deactivate the others
		for (int ix = fields_in_use_.size() + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
			ip_field_[ix]->qso(nullptr);
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND2_COLOR);
		ip_notes_->type(FL_NORMAL_INPUT);
		set_next_focus();
		break;
	case qso_data::QSO_VIEW:
		// Viewing a QSO - activate all fields in use, but don't enable data entry
		for (int ix = 0; ix <= fields_in_use_.size() && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->qso(qso_);
			ip_field_[ix]->input()->color(FL_BACKGROUND_COLOR);
			ip_field_[ix]->type(FL_NORMAL_OUTPUT);
		}
		// Deactivate all field not in use
		for (int ix = fields_in_use_.size() + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
			ip_field_[ix]->qso(nullptr);
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND_COLOR);
		ip_notes_->type(FL_NORMAL_OUTPUT);
		set_initial_focus();
		break;
	case qso_data::MANUAL_ENTRY:
		// Entering data for search - enable all fields in use...
		for (int ix = 0; ix <= fields_in_use_.size() && ix < NUMBER_TOTAL; ix++) {
			if (ch_field_[ix])
				if (ix < number_locked_ + NUMBER_FIXED) ch_field_[ix]->deactivate();
				else ch_field_[ix]->activate();
			ip_field_[ix]->activate();
			ip_field_[ix]->qso(qso_);
			ip_field_[ix]->input()->color(FL_BACKGROUND2_COLOR);
			ip_field_[ix]->type(FL_NORMAL_INPUT);
		}
		// ... and deactivate all those not in use
		for (int ix = fields_in_use_.size() + 1; ix < NUMBER_TOTAL; ix++) {
			ch_field_[ix]->deactivate();
			ip_field_[ix]->deactivate();
			ip_field_[ix]->qso(nullptr);
		}
		ip_notes_->activate();
		ip_notes_->color(FL_BACKGROUND2_COLOR);
		ip_notes_->type(FL_NORMAL_INPUT);
		set_next_focus();
		break;
	default:
		// Reserver=d for Query states
		hide();
		break;
	}
	for (int ix = 0; ix < NUMBER_TOTAL; ix++) {
		ip_field_[ix]->redraw();
	}
}

// Copy record to the fields - flags indicate specific subsets
void qso_entry::copy_qso_to_display(int flags) {
	if (qso_) {
		// Uodate band
		qso_->update_band();
		// For each field input
		for (int i = 0; i < NUMBER_TOTAL; i++) {
			string field;
			if (i < NUMBER_FIXED) field = STN_FIELDS[i];
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
		ip_notes_->value(qso_->item("NOTES", false, true).c_str());

		// If QTH changes tell DXA-IF to update home_location
		switch (qso_data_->logging_state()) {
		case qso_data::QSO_EDIT:
		case qso_data::QSO_VIEW:
		case qso_data::QSO_INACTIVE:
		case qso_data::QSO_PENDING:
		case qso_data::TEST_PENDING:
		case qso_data::QSO_STARTED:
		case qso_data::TEST_ACTIVE:
		case qso_data::QSO_ENTER:
			check_qth_changed();
			break;
		case qso_data::NET_ADDING:
		case qso_data::NET_EDIT:
		case qso_data::NET_STARTED:
			break;
		}
	}
	else {
		// Clear all fields
		for (int i = 0; i < NUMBER_TOTAL; i++) {
			string field;
			if (i < NUMBER_FIXED) field = STN_FIELDS[i];
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
	qso_->item("QSO_COMPLETE", string("N"));
	copy_qso_to_display(CF_ALL_FLAGS);
}

// Update MY_RIG and MY_ANTENNA when switch rig tab
void qso_entry::update_rig() {
	qso_rig* rig_control = ((qso_manager*)qso_data_->parent())->rig_control();
	if (rig_control && qso_) {
		qso_->item("MY_RIG", string(rig_control->label()));
		qso_->item("MY_ANTENNA", rig_control->antenna());
		copy_qso_to_display(CF_RIG_ETC);
	}
}

// Copy fields from CAT and default rig etc.
void qso_entry::copy_cat_to_qso(bool clear) {
	qso_rig* rig_control = ((qso_manager*)qso_data_->parent())->rig_control();
	if (rig_control){
		rig_if* rig = rig_control->rig();
		// Clear values before reloading them - called when switching rigs
		if (clear && qso_ != nullptr) {
			qso_->item("FREQ", string(""));
			qso_->item("MODE", string(""));
			qso_->item("SUBMODE", string(""));
			qso_->item("TX_PWR", string(""));
		}
		// Rig can be temporarily missing q
		if (rig && rig->is_good() && qso_ != nullptr) {
			string freqy = rig->get_frequency(true);
			string mode;
			string submode;
			char message[128];
			rig->get_string_mode(mode, submode);
			// Get the maximum power over course of QSO.
			double tx_power;
			char txp[10];
			switch (qso_data_->logging_state()) {
			case qso_data::QSO_PENDING:
			case qso_data::NET_STARTED:
			case qso_data_->TEST_PENDING:
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
				tx_power = rig->get_dpower(true);
				snprintf(txp, sizeof(txp), "%0.0f", tx_power);
				qso_->item("TX_PWR", string(txp));
				copy_qso_to_display(CF_CAT);
				break;
			}
			case qso_data::QSO_STARTED:
			case qso_data::TEST_ACTIVE:
			{
				// Ignore values except TX_PWR which accumulates maximum value
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
				if (isnan(tx_power)) tx_power = 0.0;
				tx_power = max(tx_power, rig->get_dpower(true));
				snprintf(txp, sizeof(txp), "%0.0f", tx_power);
				qso_->item("TX_PWR", string(txp));
				copy_qso_to_display(CF_CAT);
				break;
			}
			case qso_data::QSO_EDIT: {
				// Ignore values except TX_PWR which accumulates maximum value
				if (qso_->item("FREQ") == "") {
					snprintf(message, 128, "DASH: Frequency not specified, New value '%s'", freqy.c_str());
					status_->misc_status(ST_WARNING, message);
					qso_->item("FREQ", freqy);
				}
				if (qso_->item("MODE") == "") {
					snprintf(message, 128, "DASH: Mode not specified, New value '%s'", mode.c_str());
					status_->misc_status(ST_WARNING, message);
					qso_->item("MODE", mode);
				}
				if (qso_->item("SUBMODE") == "") {
					snprintf(message, 128, "DASH: Submode not specified, New value '%s'", submode.c_str());
					status_->misc_status(ST_WARNING, message);
					qso_->item("SUBMODE", submode);
				}
				string old_txp = qso_->item("TX_POWER");
				qso_->item("TX_PWR", tx_power);
				if (isnan(tx_power)) tx_power = 0.0;
				tx_power = max(tx_power, rig->get_dpower(true));
				snprintf(txp, sizeof(txp), "%0.0f", tx_power);
				qso_->item("TX_PWR", string(txp));
				snprintf(message, sizeof(message), "DASH: TX_power changed from '%s' to '%s'", old_txp.c_str(), txp);
				status_->misc_status(ST_WARNING, message);
				copy_qso_to_display(CF_CAT);
				tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, qso_number_);
				break;
			}
			}
		}
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
		case qso_data::TEST_PENDING:
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
		if (latest) {
			qso_->item("STATION_CALLSIGN", latest->item("STATION_CALLSIGN"));
			qso_->item("APP_ZZA_QTH", latest->item("APP_ZZA_QTH"));
			qso_->item("MY_RIG", latest->item("MY_RIG"));
			qso_->item("MY_ANTENNA", latest->item("MY_ANTENNA"));
			qso_->item("APP_ZZA_OP", latest->item("APP_ZZA_OP"));
		}
	}
}

// Copy contest values to QSO - 599 001 etc.
void qso_entry::copy_contest_to_qso() {
	if (qso_) {
		string contest_mode = spec_data_->dxcc_mode(qso_->item("MODE"));
		if (contest_mode == "CW" || contest_mode == "DATA") {
			// CW/Data
			qso_->item("RST_SENT", string("599"));
			qso_->item("RST_RCVD", string("599"));
		}
		else {
			// Phone
			qso_->item("RST_SENT", string("59"));
			qso_->item("RST_RCVD", string("59"));
		}
		// Exchange information: serial number, other doobry
		qso_->item("STX", qso_data_->contest()->serial());
		qso_->item("STX_STRING", qso_data_->contest()->exchange());
		qso_->item("CONTEST_ID", qso_data_->contest()->contest_id());
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
		field_ip_map_[STN_FIELDS[ix]] = ix;
		fields_in_use_[ix] = STN_FIELDS[ix];
	}
}

// Initialise fields
void qso_entry::initialise_fields() {
	// Now set fields
	// TODO: this is where we configure for context
	switch (qso_data_->logging_state()) {
	case qso_data::TEST_ACTIVE:
	case qso_data::TEST_PENDING:
		field_map_ = fields_->collection(qso_data_->contest()->collection());
		break;
	default:
		field_map_ = fields_->collection(fields_->coll_name(FO_QSOVIEW));
		break;
	}
	fields_in_use_.resize(field_map_->size() + NUMBER_FIXED);
	// Clear field map
	initialise_field_map();
	size_t ix = 0;
	int iy;
	// For non-fixed fields - set field name into field choice and populate
	// drop-down menu into field input with permitted values
	for (ix = 0, iy = NUMBER_FIXED; ix < field_map_->size(); ix++, iy++) {
		string name = (*field_map_)[ix].field;
		ch_field_[iy]->value(name.c_str());
		ip_field_[iy]->field_name(name.c_str(), qso_);
		field_ip_map_[name] = iy;
		fields_in_use_[iy] = name;
	}
	// And removed such info for inputs that are not used
	for (; iy < NUMBER_TOTAL; iy++) {
		ch_field_[iy]->value("");
		ip_field_[iy]->value("");
		ip_field_[iy]->field_name("");
	}
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
	if (ix == -1 && fields_in_use_.size() == NUMBER_TOTAL) {
		char msg[128];
		snprintf(msg, 128, "DASH: Cannot add any more fields to edit - %s ignored", field.c_str());
		status_->misc_status(ST_ERROR, msg);
	}
	else if (ix >= 0 && ix < fields_in_use_.size()) {
		const char* old_field = fields_in_use_[ix].c_str();
		ip_field_[ix]->field_name(field.c_str(), qso_);
		ip_field_[ix]->value(qso_->item(field).c_str());
		// Change mapping
		if (strlen(old_field)) {
			field_ip_map_.erase(old_field);
		}
		field_ip_map_[field] = ix;
		(*field_map_)[ix - NUMBER_FIXED].field = field;
	}
	else if (ix == fields_in_use_.size()) {
		if (field_ip_map_.find(field) == field_ip_map_.end()) {
			ch_field_[fields_in_use_.size()]->value(field.c_str());
			ip_field_[fields_in_use_.size()]->field_name(field.c_str(), qso_);
			ip_field_[fields_in_use_.size()]->value(qso_->item(field).c_str());
			field_ip_map_[field] = fields_in_use_.size();
			field_map_->push_back({ field, field, 50 });
			fields_in_use_.push_back(field);
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
	enable_widgets();
}

// Delete a field
void qso_entry::action_del_field(int ix) {
	int iy = ix - NUMBER_FIXED;
	string& old_field = (*field_map_)[iy].field;
	int ip = field_ip_map_[old_field];
	field_ip_map_.erase(old_field);
	int pos = ix;
	for (; pos < fields_in_use_.size() - 1; pos++) {
		int posy = pos - NUMBER_FIXED;
		string& field = (*field_map_)[posy + 1].field;
		(*field_map_)[posy].field = field;
		ch_field_[pos]->value(field.c_str());
		ip_field_[pos]->field_name(field.c_str(), qso_);
		ip_field_[pos]->value(qso_->item(field).c_str());
		fields_in_use_[pos] = field;
	}
	ch_field_[pos]->value("");
	ip_field_[pos]->field_name("");
	ip_field_[pos]->value("");
	field_map_->resize(field_map_->size() - 1);
	fields_in_use_.resize(fields_in_use_.size() - 1);

	enable_widgets();
}


// Callback change field selected
void qso_entry::cb_ch_field(Fl_Widget* w, void* v) {
	qso_entry* that = ancestor_view<qso_entry>(w);
	field_choice* ch = (field_choice*)w;
	const char* field = ch->value();
	int ix = (int)(intptr_t)v;
	if (strlen(field)) {
		// Add a field to the list if its not null
		that->action_add_field(ix, field);
	}
	else {
		// Delete it if the name is null string
		that->action_del_field(ix);
	}
}

// Callback - general input
// v - index of input widget
void qso_entry::cb_ip_field(Fl_Widget* w, void* v) {
	qso_entry* that = ancestor_view<qso_entry>(w);
	qso_manager* mgr = ancestor_view<qso_manager>(that->qso_data_);
	field_input* ip = (field_input*)w;
	string field = ip->field_name();
	string value = ip->value();
	string old_value = that->qso_->item(field);
	// Save the index to set next focus
	that->current_ix_ = (int)(intptr_t)v;
	if (old_value != value) {
		that->qso_->item(field, value);

		if (field == "FREQ") {
			double freq_MHz = atof(value.c_str());
			double freq_kHz = freq_MHz * 1000.0;
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
			qso_manager* mgr = (qso_manager*)that->qso_data_->parent();
			mgr->change_rig(value);
			switch(that->qso_data_->logging_state()) {
				case qso_data::QSO_INACTIVE:
				case qso_data::QSO_PENDING:
				case qso_data::QSO_STARTED:
				case qso_data::QSO_ENTER:
				case qso_data::NET_STARTED:
				case qso_data::NET_ADDING:
				case qso_data::QSO_WSJTX:
				case qso_data::QSO_FLDIGI:
				case qso_data::TEST_PENDING:
				case qso_data::TEST_ACTIVE:
				{
					// Copy matching antenna
					qso_rig* rig_control = mgr->rig_control();
					string antenna;
					if (rig_control) {
						antenna = rig_control->antenna();
						that->qso_->item("MY_ANTENNA", antenna);
					}
					that->copy_cat_to_qso(true);
					break;
				}
			}
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
		// QSO has changed, change QSL servers' status 
		if (field == "FREQ" || field == "BAND" || field == "MODE" || field == "CALL" ||
			field == "QSO_DATE" || field == "TIME_ON") {
			that->qso_->invalidate_qsl_status();
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
		case qso_data::TEST_PENDING:
		case qso_data::TEST_ACTIVE:
			that->enable_widgets();
			mgr->enable_widgets();
			tabbed_forms_->update_views(nullptr, HT_MINOR_CHANGE, that->qso_number_);
			break;
		default:
			break;
		}
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
		if (qso_) {
			original_qso_ = new record(*qso_);
		} else {
			original_qso_ = nullptr;
		}
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

// Set initial focus - go to first blank input
void qso_entry::set_initial_focus() {
	if (!visible_r()) return;
	bool found = false;
	for (int ix = NUMBER_FIXED; !found && ix < fields_in_use_.size(); ix ++) {
		field_input* w = ip_field_[ix];
		if (w && strlen(w->value()) == 0) {
			if (w->take_focus()) found = true;
		}
	}
}

// set next focus
void qso_entry::set_next_focus() {
	if (!visible_r()) return;
	if (current_ix_ < fields_in_use_.size()) {
		Fl_Widget* w0 = Fl::focus();
		if (w0) {
			// Disable events fom current widget
			Fl_When save = w0->when();
			w0->when(0);
			field_input* w1 = ip_field_[current_ix_++];
			w1->take_focus();
			w0->when(save);
		} else {
			field_input* w1 = ip_field_[current_ix_++];
			w1->take_focus();
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
		}
	}
}

// 1 second clock
void qso_entry::cb_ticker(void* v) {
	qso_entry* that = (qso_entry*)v;
	that->copy_clock_to_qso();
	switch(that->qso_data_->logging_state()) {
		case qso_data::QSO_PENDING:
		case qso_data::QSO_STARTED:
		case qso_data::NET_STARTED:
		case qso_data::TEST_PENDING:
		case qso_data::TEST_ACTIVE:
			that->copy_cat_to_qso();
			break;
	}
}

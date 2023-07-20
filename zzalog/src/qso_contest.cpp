#include "qso_contest.h"
#include "qso_data.h"
#include "book.h"
#include "record.h"
#include "status.h"
#include "callback.h"
#include "utils.h"

#include <FL/Fl_Preferences.H>


extern Fl_Preferences* settings_;
extern book* book_;
extern status* status_;

// How to use the contest facility
string instructions =
"Using an existing contest definition:-\n"
"Select the contest in the CONTEST_ID field choice\n"
"Select the exchange definition\n"
"Click \"Enable\"\n"
"\n"
"Defining a new contest definition:-\n"
"Select the contest in the CONTEST_ID field choice\n"
"Type in a name for the exchange deifinition\n"
"Click \"Edit\"\n"
"Add the fields that you need to log your reception\n"
"Click \"Set RX\"\n"
"Add the fields that you need to log your transmission\n"
"Click \"Set TX\"\n"
"Click \"Save\"\n";

// QSO contest group
qso_contest::qso_contest(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, contest_id_("")
	, exch_fmt_ix_(0)
	, exch_fmt_id_("")
	, max_ef_index_(0)
	, contest_mode_(NO_CONTEST)
	, wn_instructions_(nullptr)
{
	load_values();
	create_form(X, Y);
}


qso_contest::~qso_contest() {
	save_values();
}

void qso_contest::load_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");
	// Contest definitions
	Fl_Preferences contest_settings(dash_settings, "Contests");
	contest_settings.get("Next Serial Number", serial_num_, 0);
	contest_settings.get("Contest Status", (int&)contest_mode_, false);
	char* temp;
	contest_settings.get("Contest ID", temp, "");
	contest_id_ = temp;
	free(temp);
	contest_settings.get("Exchange Format", exch_fmt_ix_, 0);
	// Contest exchnage formats
	Fl_Preferences format_settings(contest_settings, "Formats");
	unsigned int num_formats = format_settings.groups();
	max_ef_index_ = num_formats;
	// Already have contests set, read them
	//	Format ID (nickname)
	//		Index: n
	//		TX: Transmit fields
	//		RX: Receive fields
	if (num_formats == 0) {
		max_ef_index_ = 1;
		ef_ids_[0] = "";
		ef_txs_[0] = "RST_SENT,STX";
		ef_rxs_[0] = "RST_RCVD,SRX";
	}
	else {
		for (unsigned int i = 0; i < num_formats; i++) {
			Fl_Preferences one_setting(format_settings, format_settings.group(i));
			int index;
			one_setting.get("Index", index, 0);
			ef_ids_[index] = format_settings.group(i);
			one_setting.get("TX", temp, "RST_SENT,STX");
			ef_txs_[index] = string(temp);
			free(temp);
			one_setting.get("RX", temp, "RST_RCVD,SRX");
			ef_rxs_[index] = string(temp);
			free(temp);
		}
	}
	// Set active contest format ID
	exch_fmt_id_ = ef_ids_[exch_fmt_ix_];

	// Read contest details from most recent QSO
	if (book_->size()) {
		record* prev_record = book_->get_record(book_->size() - 1, false);
		string prev_contest = prev_record->item("CONTEST_ID");
		char message[100];
		if (contest_mode_ != NO_CONTEST) {
			if (prev_contest != contest_id_) {
				snprintf(message, 100, "DASH: Contest ID in log (%s) differs from settings (%s)", prev_contest.c_str(), contest_id_.c_str());
				status_->misc_status(ST_WARNING, message);
			}
			else {
				// Get serial number from log
				string serno = prev_record->item("STX");
				if (serno.length() > 0) {
					int sernum = stoi(serno);
					if (sernum > serial_num_) {
						snprintf(message, 100, "DASH: Contest serial in log (%d) greater than settings (%d) - using log", sernum, serial_num_);
						status_->misc_status(ST_WARNING, message);
						serial_num_ = sernum;
					}
					else {
						snprintf(message, 100, "DASH: Contest serial in log (%d) less than settings (%d) - using settings", sernum, serial_num_);
						status_->misc_status(ST_NOTE, message);
					}
				}
			}
		}
	}
}

// Create contest group
void qso_contest::create_form(int X, int Y) {
	int curr_x = X;
	int curr_y = Y;

	label("Contest");
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);

	curr_x += GAP;
	curr_y += HTEXT;

	bn_enable_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Enable");
	bn_enable_->value(contest_mode_ != NO_CONTEST);
	bn_enable_->selection_color(FL_GREEN);
	bn_enable_->callback(cb_ena_contest, nullptr);
	bn_enable_->tooltip("Enable contest operation - resets contest parameters");

	curr_x += bn_enable_->w() + GAP;

	// Pause contest button
	bn_pause_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Pause");
	bn_pause_->value(contest_mode_ == PAUSED);
	bn_pause_->selection_color(FL_RED);
	bn_pause_->callback(cb_pause_contest, nullptr);
	bn_pause_->tooltip("Pause contest, i.e. keep parameters when resume");

	curr_x += bn_pause_->w() + GAP + (WLABEL * 3 / 2);

	// Contest ID - used for logging
	ch_contest_id_ = new field_choice(curr_x, curr_y, WBUTTON * 2, HBUTTON, "CONTEST_ID");
	ch_contest_id_->set_dataset("Contest_ID");
	ch_contest_id_->value(contest_id_.c_str());
	ch_contest_id_->callback(cb_value<field_choice, string>, &contest_id_);
	ch_contest_id_->tooltip("Select the ID for the contest (for logging)");

	int max_w = curr_x + ch_contest_id_->w() + GAP - x();
	curr_x = x() + GAP + WLABEL;
	curr_y += ch_contest_id_->h();

	// Choice widget to select the required exchanges
	ch_format_ = new Fl_Input_Choice(curr_x, curr_y, WBUTTON + WBUTTON, HBUTTON, "Exch.");
	ch_format_->value(exch_fmt_id_.c_str());
	ch_format_->align(FL_ALIGN_LEFT);
	ch_format_->callback(cb_format, nullptr);
	ch_format_->when(FL_WHEN_RELEASE);
	ch_format_->tooltip("Select existing exchange format or type in new (click \"Add\" to add)");
	populate_exch_fmt();

	curr_x += ch_format_->w();

	// Add exchange button
	bn_add_exch_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Add");
	bn_add_exch_->value(contest_mode_ == EDIT);
	bn_add_exch_->callback(cb_add_exch, nullptr);
	bn_add_exch_->selection_color(FL_RED);
	bn_add_exch_->tooltip("Add new exchange format - choose fields and click \"TX\" or \"RX\" to create them");

	curr_x += bn_add_exch_->w();

	// Define contest
	bn_define_rx_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Set RX");
	bn_define_rx_->callback(cb_def_format, (void*)false);
	bn_define_rx_->selection_color(FL_BLUE);
	bn_define_rx_->tooltip("Use the specified fields as contest exchange on receive");

	curr_x += bn_define_rx_->w();

	// Define contest exchanges
	bn_define_tx_ = new Fl_Light_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Set TX");
	bn_define_tx_->callback(cb_def_format, (void*)true);
	bn_define_tx_->selection_color(FL_BLUE);
	bn_define_tx_->tooltip("Use the specified fields as contest exchange on transmit");

	curr_x += bn_define_tx_->w() + GAP;

	// Serial number control buttons - Initialise to 001
	bn_init_serno_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@|<");
	bn_init_serno_->callback(cb_init_serno, nullptr);
	bn_init_serno_->tooltip("Reset the contest serial number counter to \"001\"");

	curr_x += bn_init_serno_->w();

	// Serial number control buttons - Decrement
	bn_dec_serno_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@<");
	bn_dec_serno_->labelsize(FL_NORMAL_SIZE + 2);
	bn_dec_serno_->callback(cb_dec_serno, nullptr);
	bn_dec_serno_->tooltip("Decrement the contest serial number counter by 1");

	curr_x += bn_dec_serno_->w();

	// Serial number control buttons - Decrement
	bn_inc_serno_ = new Fl_Button(curr_x, curr_y, WBUTTON / 2, HBUTTON, "@>");
	bn_inc_serno_->labelsize(FL_NORMAL_SIZE + 2);
	bn_inc_serno_->callback(cb_inc_serno, nullptr);
	bn_inc_serno_->tooltip("Increment the contest serial number counter by 1");

	curr_x += bn_inc_serno_->w();

	// Transmitted contest exchange
	op_serno_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Serial");
	op_serno_->align(FL_ALIGN_TOP);
	op_serno_->tooltip("This is the serial number you should be sending");

	curr_x += op_serno_->w() + GAP;
	curr_y += op_serno_->h() + GAP;

	resizable(nullptr);
	size(curr_x - x(), curr_y - y());

	end();

	return;
}

void qso_contest::enable_widgets() {
	// Get exchange data
	if (exch_fmt_ix_ < MAX_CONTEST_TYPES && contest_mode_ == CONTEST) {
		char text[10];
		snprintf(text, 10, "%03d", serial_num_);
		op_serno_->value(text);
	}
	else {
		op_serno_->value("");
	}
	// Basic contest on/off widgets 
	switch (contest_mode_) {
	case NO_CONTEST:
		op_serno_->deactivate();
		bn_pause_->deactivate();
		ch_contest_id_->deactivate();
		ch_format_->deactivate();
		bn_init_serno_->deactivate();
		bn_inc_serno_->deactivate();
		bn_dec_serno_->deactivate();
		break;
	default:
		op_serno_->activate();
		bn_pause_->activate();
		ch_contest_id_->activate();
		ch_format_->activate();
		bn_init_serno_->activate();
		bn_inc_serno_->activate();
		bn_dec_serno_->activate();
	}
	op_serno_->redraw();
	bn_pause_->redraw();
	ch_contest_id_->redraw();
	ch_format_->redraw();
	bn_init_serno_->redraw();
	bn_inc_serno_->redraw();
	bn_dec_serno_->redraw();
	// Mode dependent
	switch (contest_mode_) {
	case NEW:
		bn_add_exch_->activate();
		bn_add_exch_->label("Add");
		bn_define_rx_->deactivate();
		bn_define_tx_->deactivate();
		break;
	case DEFINE:
	case EDIT:
		bn_add_exch_->activate();
		bn_add_exch_->label("Save");
		bn_define_rx_->activate();
		bn_define_rx_->value(rx_set_);
		if (!rx_set_) bn_define_tx_->deactivate();
		else bn_define_tx_->activate();
		bn_define_tx_->value(tx_set_);
		break;
	case CONTEST:
		bn_add_exch_->activate();
		bn_add_exch_->label("Edit");
		bn_define_rx_->deactivate();
		bn_define_tx_->deactivate();
		break;
	default:
		bn_add_exch_->deactivate();
		bn_add_exch_->label(nullptr);
		bn_define_rx_->deactivate();
		bn_define_tx_->deactivate();
		break;
	}
	bn_add_exch_->redraw();
	bn_add_exch_->redraw();
	bn_define_rx_->redraw();
	bn_define_tx_->redraw();

}

// Set contest enable/disable
// v - not used
void qso_contest::cb_ena_contest(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	bool enable = false;
	cb_value<Fl_Light_Button, bool>(w, &enable);

	if (enable) {
		that->serial_num_ = 1;
		that->contest_mode_ = CONTEST;
	}
	else {
		that->contest_mode_ = NO_CONTEST;
	}
	that->initialise_fields();
	that->enable_widgets();
}

// Pause contest mode
void qso_contest::cb_pause_contest(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	bool pause = false;
	cb_value<Fl_Light_Button, bool>(w, &pause);

	if (pause) {
		that->contest_mode_ = PAUSED;
	}
	else {
		that->contest_mode_ = CONTEST;
	}
	that->initialise_fields();
	that->enable_widgets();
}

// Set exchange format
// v is exchange
void qso_contest::cb_format(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	Fl_Input_Choice* ch = (Fl_Input_Choice*)w;
	// Get format ID
	that->exch_fmt_id_ = ch->value();
	// Get source
	if (ch->menubutton()->changed()) {
		// Selected menu item
		that->exch_fmt_ix_ = ch->menubutton()->value();
		that->contest_mode_ = CONTEST; // Should already be so
		that->initialise_fields();
	}
	else {
		that->contest_mode_ = NEW;
	}
	that->enable_widgets();
	that->instructions_window(true);
}

// Add exchange format
// v is unused
void qso_contest::cb_add_exch(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	bool update = false;
	cb_value<Fl_Light_Button, bool>(w, &update);
	if (update) {
		// Switch to update format definitions
		switch (that->contest_mode_) {
		case NEW:
			// Adding a new format - add it to the list
			that->exch_fmt_ix_ = that->add_format_id(that->exch_fmt_id_);
			that->contest_mode_ = DEFINE;
			that->rx_set_ = false;
			that->tx_set_ = false;
			that->initialise_fields();
			break;
		case CONTEST:
			// Editing an existing format
			that->contest_mode_ = EDIT;
			that->rx_set_ = false;
			that->tx_set_ = false;
			that->initialise_fields();
			break;
		}
		that->enable_widgets();
		that->instructions_window(true);
	}
	else {
		// Start/Resume contest
		that->contest_mode_ = CONTEST;
		that->populate_exch_fmt();
		that->initialise_fields();
		that->enable_widgets();
		that->instructions_window(false);
	}
}

// Define contest exchange
// v - bool TX or RX
void qso_contest::cb_def_format(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	bool tx = (bool)(intptr_t)v;
	bool set;
	cb_value<Fl_Light_Button, bool>(w, &set);
	// If the button is being set then update the exchange fileds required
	if (set) {
		that->add_format_def(that->exch_fmt_ix_, tx);
	} 
	// Record the set state TX or RX
	if (tx) that->tx_set_ = set;
	else that->rx_set_ = set;
	// Re initialise
	that->initialise_fields();
	that->enable_widgets();
}

// Reset contest serial number
void qso_contest::cb_init_serno(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->serial_num_ = 1;
	that->initialise_fields();
	that->enable_widgets();
}

// Increment serial number
void qso_contest::cb_inc_serno(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->serial_num_++;
	that->initialise_fields();
	that->enable_widgets();
}

// Decrement serial number
void qso_contest::cb_dec_serno(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->serial_num_--;
	that->initialise_fields();
	that->enable_widgets();
}

// Save the settings
void qso_contest::save_values() {
	// Dashboard configuration
	Fl_Preferences dash_settings(settings_, "Dashboard");

	// Contest definitions
	Fl_Preferences contest_settings(dash_settings, "Contests");
	contest_settings.clear();
	contest_settings.set("Next Serial Number", serial_num_);
	contest_settings.set("Contest Status", (int)contest_mode_);
	contest_settings.set("Contest ID", contest_id_.c_str());
	contest_settings.set("Exchange Format", exch_fmt_ix_);
	// Exchanges
	Fl_Preferences format_settings(contest_settings, "Formats");
	for (int ix = 0; ix < max_ef_index_; ix++) {
		Fl_Preferences one_settings(format_settings, ef_ids_[ix].c_str());
		one_settings.set("Index", (signed)ix);
		one_settings.set("TX", ef_txs_[ix].c_str());
		one_settings.set("RX", ef_rxs_[ix].c_str());
	}
}

// Add contests
void qso_contest::populate_exch_fmt() {
	for (int ix = 0; ix < max_ef_index_; ix++) {
		ch_format_->add(ef_ids_[ix].c_str());
	}
}

// Initialise fields
void qso_contest::initialise_fields() {
	((qso_data*)parent())->enable_widgets();
}

// Return field mode
qso_contest::contest_mode_t qso_contest::mode() {
	return contest_mode_;
}

// Contest fields
string qso_contest::contest_fields() {
	string& rx = ef_rxs_[exch_fmt_ix_];
	string& tx = ef_txs_[exch_fmt_ix_];
	switch (contest_mode_) {
	case CONTEST:
		// Only concatenate RX and TX strings if both are not empty
		if (rx.length() == 0) return tx;
		else if (tx.length() == 0) return rx;
		else return rx + ',' + tx;
	case EDIT:
		if (!rx_set_) return rx;
		if (!tx_set_) return tx;
		return "";
	default:
		return "";
	}
}

// Add new format - return format index
int qso_contest::add_format_id(string id) {
	// Add the string to the choice
	ch_format_->add(id.c_str());
	int index = ch_format_->menubutton()->find_index(id.c_str());
	// Add the format id to the array
	ef_ids_[index] = id;
	max_ef_index_ = index + 1;
	// Return the index
	return index;
}

// Add new format definition 
void qso_contest::add_format_def(int ix, bool tx) {
	// Get the string from the field choices
	string defn = ((qso_data*)parent())->get_defined_fields();
	// Add the format definition to the array
	if (tx) {
		ef_txs_[ix] = defn;
	}
	else {
		ef_rxs_[ix] = defn;
	}
}

// get serial number
int qso_contest::serial_number() {
	return serial_num_;
}

// increment serial number
void qso_contest::increment_serial() {
	if (contest_mode_ == CONTEST) serial_num_++;
}

// Create instructions window
void qso_contest::instructions_window(bool show) {
	if (wn_instructions_ == nullptr) {
		Fl_Window* win = window();
		wn_instructions_ = tip_window(instructions, win->x_root() + x() + w(), win->y_root() + y());
	}
	if (show) {
		wn_instructions_->show();
	}
	else {
		wn_instructions_->hide();
	}
	wn_instructions_->focus(this);

}
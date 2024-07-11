#include "qso_contest.h"
#include "qso_data.h"
#include "drawing.h"
#include "callback.h"
#include "icons.h"
#include "fields.h"
#include "menu.h"
#include "settings.h"
#include "status.h"
#include "extract_data.h"
#include "tabbed_forms.h"

#include <FL/Fl_Preferences.H>

extern Fl_Preferences* settings_;
extern fields* fields_;
extern status* status_;
extern extract_data* extract_records_;
extern tabbed_forms* tabbed_forms_;

// Constructor
qso_contest::qso_contest(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L),
	contest_id_(""),
	collection_(""),
	notes_(""),
	start_date_(""),
	start_time_(""),
	end_date_(""),
	end_time_(""),
	serial_(1),
	active_(false),
	write_format_(ADIF)
{
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	load_settings();
	create_form();
	// Set default values if necessary
	(void)fields_->collection("Contest/General", CONTEST_FIELDS);

	enable_widgets();
}

// Destructor
qso_contest::~qso_contest() {
	save_settings();
}

// Build the widget
void qso_contest::create_form() {
	int curr_x = x() + GAP + WLABEL;
	int curr_y = y() + GAP;

	// Contest ID choice
	ip_contest_id_ = new field_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Contest ID");
	ip_contest_id_->align(FL_ALIGN_LEFT);
	ip_contest_id_->callback(cb_contest_id, &contest_id_);
	ip_contest_id_->tooltip("Select the ID of the contest (as logged)");
	ip_contest_id_->field_name("CONTEST_ID");

	curr_y += ip_contest_id_->h();
	// Collection
	ip_collection_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Logging");
	ip_collection_->align(FL_ALIGN_LEFT);
	ip_collection_->callback(cb_collection, &collection_);
	ip_collection_->tooltip("Selecet the name of the collections of fields to log");

	curr_y += ip_collection_->h();
	// Notes
	ip_notes_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Notes");
	ip_notes_->align(FL_ALIGN_LEFT);
	ip_notes_->tooltip("English explanation of contest exchange");
	ip_notes_->callback(cb_value<Fl_Input, string>, &notes_);

	curr_y += ip_notes_->h() + GAP;
	// Start date
	ip_start_date_ = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON, "Start");
	ip_start_date_->align(FL_ALIGN_LEFT);
	ip_start_date_->callback(cb_value<Fl_Input, string>, &start_date_);
	ip_start_date_->tooltip("Enter date the contest starts (or use calendar)");

	curr_x += ip_start_date_->w();
	bn_start_date_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	bn_start_date_->image(new Fl_RGB_Image(ICON_CALENDAR, 16, 16, 4));
	start_data_ = { &start_date_, ip_start_date_ };
	bn_start_date_->callback(calendar::cb_cal_open, &start_data_);
	bn_start_date_->when(FL_WHEN_RELEASE);
	bn_start_date_->tooltip("Open calendar to chnage date to fetch eQSL.cc");

	curr_x += bn_start_date_->w();
	ip_start_time_ = new Fl_Input(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON);
	ip_start_time_->callback(cb_value<Fl_Input, string>, &start_time_);
	ip_start_time_->tooltip("Enter the time the contest starts");

	curr_x = x() + GAP + WLABEL;
	curr_y += ip_start_date_->h();
	// end date
	ip_end_date_ = new Fl_Input(curr_x, curr_y, WBUTTON, HBUTTON, "end");
	ip_end_date_->align(FL_ALIGN_LEFT);
	ip_end_date_->callback(cb_value<Fl_Input, string>, &end_date_);
	ip_end_date_->tooltip("Enter date the contest ends (or use calendar)");

	curr_x += ip_end_date_->w();
	bn_end_date_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON);
	bn_end_date_->image(new Fl_RGB_Image(ICON_CALENDAR, 16, 16, 4));
	end_data_ = { &end_date_, ip_end_date_ };
	bn_end_date_->callback(calendar::cb_cal_open, &end_data_);
	bn_end_date_->when(FL_WHEN_RELEASE);
	bn_end_date_->tooltip("Open calendar to chnage date to fetch eQSL.cc");

	curr_x += bn_end_date_->w();
	ip_end_time_ = new Fl_Input(curr_x, curr_y, WBUTTON * 3 / 2, HBUTTON);
	ip_end_time_->callback(cb_value<Fl_Input, string>, &end_time_);
	ip_end_time_->tooltip("Enter the time the contest ends");

	curr_x = x() + GAP + WLABEL;
	curr_y += ip_end_date_->h();
	// Exchange
	ip_exchange_ = new Fl_Input(curr_x, curr_y, WSMEDIT, HBUTTON, "Exchange");
	ip_exchange_->align(FL_ALIGN_LEFT);
	ip_exchange_->tooltip("Enter the default exchange for your transmission");
	ip_exchange_->callback(cb_value<Fl_Input, string>, &exchange_);

	curr_y += ip_exchange_->h();
	op_serial_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON, "Serial#");
	op_serial_->align(FL_ALIGN_LEFT);
	op_serial_->tooltip("Displays the next serial number to use");

	// Serial number widgets
	curr_x += op_serial_->w();
	bn_rst_ser_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@4->|");
	bn_rst_ser_->align(FL_ALIGN_INSIDE);
	bn_rst_ser_->callback(cb_rst_ser, &serial_);
	bn_rst_ser_->tooltip("Reset serial to \"001\"");

	curr_x += bn_rst_ser_->w();
	bn_dec_ser_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@<-");
	bn_dec_ser_->align(FL_ALIGN_INSIDE);
	bn_dec_ser_->callback(cb_dec_ser, &serial_);
	bn_dec_ser_->tooltip("Decrment serial number (revert previous log)");

	curr_x += bn_dec_ser_->w();
	bn_inc_ser_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, "@->");
	bn_inc_ser_->align(FL_ALIGN_INSIDE);
	bn_inc_ser_->callback(cb_inc_ser, &serial_);
	bn_inc_ser_->tooltip("Increment serial number (advance 1 QSO");

	curr_x = x() + GAP;
	curr_y += bn_inc_ser_->h() + GAP;
	const int WB3 = (w() - GAP - GAP) / 3;

	// Active button
	bn_active_ = new Fl_Light_Button(curr_x, curr_y, WB3, HBUTTON, "Active");
	bn_active_->align(FL_ALIGN_INSIDE);
	bn_active_->callback(cb_active, &active_);
	bn_active_->value(false);
	bn_active_->tooltip("Contest logging is active");

	// Finish buttn
	curr_x += bn_active_->w();
	bn_finish_ = new Fl_Button(curr_x, curr_y, WB3, HBUTTON, "Finish");
	bn_finish_->align(FL_ALIGN_INSIDE);
	bn_finish_->callback(cb_finish);
	bn_finish_->tooltip("Click to complete the contest");

	// SAve button
	curr_x += bn_finish_->w();
	bn_save_ = new Fl_Button(curr_x, curr_y, WB3, HBUTTON, "Save");
	bn_save_->align(FL_ALIGN_INSIDE);
	bn_save_->callback(cb_save);
	bn_save_->tooltip("Click to save the changes");

	curr_x = x() + GAP;
	curr_y += bn_save_->h();
	// Write log buttons
	bn_write_ = new Fl_Button(curr_x, curr_y, WB3, HBUTTON, "Write Log");
	bn_write_->align(FL_ALIGN_INSIDE);
	bn_write_->callback(cb_write);
	bn_write_->tooltip("Write the log in the selected format");

	curr_x += bn_write_->w();
	// ADIF
	bn_adif_ = new Fl_Radio_Light_Button(curr_x, curr_y, WB3, HBUTTON, "ADIF");
	bn_adif_->align(FL_ALIGN_INSIDE);
	bn_adif_->callback(cb_wradio, (void*)(intptr_t)ADIF);
	bn_adif_->when(FL_WHEN_RELEASE);
	bn_adif_->tooltip("Select ADIF as the write log format");
	
	curr_x += bn_adif_->w();
	// Cabrillo
	bn_cabrillo_ = new Fl_Radio_Light_Button(curr_x, curr_y, WB3, HBUTTON, "Cabrillo");
	bn_cabrillo_->align(FL_ALIGN_INSIDE);
	bn_cabrillo_->callback(cb_wradio, (void*)(intptr_t)CABRILLO);
	bn_cabrillo_->when(FL_WHEN_RELEASE);
	bn_cabrillo_->tooltip("Select Cabrillo as the write log format");

	end();
	show();
}

// Configure all the component widgets
void qso_contest::enable_widgets() {
	// Copy data to widgets
	ip_contest_id_->value(contest_id_.c_str());

	ip_collection_->value(collection_.substr(8).c_str());
	ip_collection_->update_menubutton();

	ip_notes_->value(notes_.c_str());

	ip_start_date_->value(start_date_.c_str());
	ip_start_time_->value(start_time_.c_str());
	ip_end_date_->value(end_date_.c_str());
	ip_end_time_->value(end_time_.c_str());

	char text[10];
	snprintf(text, sizeof(text), "%03d", serial_);
	op_serial_->value(text);

	bn_active_->value(active_);

	bn_adif_->value(write_format_ == ADIF);
	bn_cabrillo_->value(write_format_ == CABRILLO);
	// Disable cabrillo until interface is written
	bn_cabrillo_->deactivate();
}

// Get the contest ID
string qso_contest::contest_id() {
	return contest_id_;
}

// Get the logging field colelcttion
string qso_contest::collection() {
	return collection_;
}

// Get the current serial number
string qso_contest::serial() {
	char text[10];
	snprintf(text, sizeof(text), "%03d", serial_);
	return string(text);
}

// Get the exchange
string qso_contest::exchange() {
	return exchange_;
}

// Increment the serial number
void qso_contest::increment_serial() {
	serial_++;
}

// A contest is active
bool qso_contest::contest_active() {
	return active_;
}

// Contest ID
void qso_contest::cb_contest_id(Fl_Widget* w, void* v) {
	// Get the contest ID
	cb_value<field_input, string>(w, v);
	qso_contest* that = ancestor_view<qso_contest>(w);
	// Read the data and load into the widgets
	that->load_settings();
	that->enable_widgets();
	qso_data* data = ancestor_view<qso_data>(that);
	if (that->active_) cb_active(that->bn_active_, &that->active_);
}

// Log data
void qso_contest::cb_collection(Fl_Widget* w, void* v) {
	string coll;
	// Get the contest ID
	cb_value<Fl_Input_Choice, string>(w, &coll);
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->collection_ = "Contest/" + coll;
	that->check_collection();
}

// Decrement serial
void qso_contest::cb_dec_ser(Fl_Widget* w, void* v) {
	int* serial = (int*)(intptr_t)v;
	(*serial)--;
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->enable_widgets();
}

// Increment serial
void qso_contest::cb_inc_ser(Fl_Widget* w, void* v) {
	int* serial = (int*)(intptr_t)v;
	(*serial)++;
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->enable_widgets();
}

// Reset serial
void qso_contest::cb_rst_ser(Fl_Widget* w, void* v) {
	int* serial = (int*)(intptr_t)v;
	*serial = 1;
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->enable_widgets();
}

// Active button
void qso_contest::cb_active(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	qso_data* qd = ancestor_view<qso_data>(that);
	bool a;
	char msg[128];
	cb_value<Fl_Light_Button, bool>(w, &a);
	if (a) {
		switch (qd->logging_state()) {
		case qso_data::QSO_INACTIVE:
		case qso_data::QSO_PENDING:
			that->active_ = true;
			qd->action_activate(qso_data::QSO_ON_AIR);
			snprintf(msg, sizeof(msg), "DASH: Starting contest %s.", that->contest_id_.c_str());
			status_->misc_status(ST_OK, msg);
			// Extract existing QSOs in this contest, use the collection and display the pane
			extract_records_->extract_field("CONTEST_ID", that->contest_id_, false, that->start_date_, that->end_date_);
			fields_->link_app(FO_EXTRACTLOG, that->collection_);
			tabbed_forms_->activate_pane(OT_EXTRACT, true);
			break;
		default:
			status_->misc_status(ST_ERROR, "DASH: Not in a state to start contest activity!");
			break;
		}
	}
	else {
		switch (qd->logging_state()) {
		case qso_data::TEST_PENDING:
			that->active_ = false;
			qd->action_deactivate();
			snprintf(msg, sizeof(msg), "DASH: Suspending contest %s.", that->contest_id_.c_str());
			status_->misc_status(ST_OK, msg);
			break;
		default:
			status_->misc_status(ST_ERROR, "DASH: Not in a state to suspend contest activity!");
			break;
		}
	}
	that->bn_active_->value(that->active_);
	that->save_settings();
}

// Finish button
void qso_contest::cb_finish(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->active_ = false;
	that->serial_ = 1;
	that->save_settings();
}

// Save button
void qso_contest::cb_save(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->save_settings();
}

// Write ADIF button
void qso_contest::cb_write(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	switch (that->write_format_) {
	case ADIF: {
		that->write_adif();
		break;
	}
	case CABRILLO: {
		that->write_cabrillo();
		break;
	}
	}
}

// Write cabrillo button
void qso_contest::cb_wradio(Fl_Widget* w, void* v) {
	qso_contest* that = ancestor_view<qso_contest>(w);
	that->write_format_ = (write_t)(intptr_t)v;
}

// Load settings for the contest ID
void qso_contest::load_settings() {
	char* temp;
	if (contest_id_ == "") {
		settings_->get("Contests/Current", temp, "");
		contest_id_ = temp;
		free(temp);
	}
	if (contest_id_ != "") {
		Fl_Preferences contests(settings_, "Contests");
		Fl_Preferences c_settings(contests, contest_id_.c_str());
		c_settings.get("Fields", temp, "Contest/General");
		collection_ = temp;
		free(temp);
		c_settings.get("Notes", temp, "");
		notes_ = temp;
		free(temp);
		char today[25];
		time_t now = time(nullptr);
		tm* now2 = gmtime(&now);
		strftime(today, sizeof(today), "%Y%m%d", now2);
		c_settings.get("Start Date", temp, today);
		start_date_ = temp;
		free(temp);
		c_settings.get("Start Time", temp, "0000");
		start_time_ = temp;
		free(temp);
		c_settings.get("End Date", temp, today);
		end_date_ = temp;
		free(temp);
		c_settings.get("End Time", temp, "2359");
		end_time_ = temp;
		free(temp);
		c_settings.get("Next Serial", serial_, 1);
		c_settings.get("Active", (int&)active_, (int)false);
	}
	else {
		collection_ = "Contest/General";
		notes_ = "";
		char today[25];
		time_t now = time(nullptr);
		tm* now2 = gmtime(&now);
		strftime(today, sizeof(today), "%Y%m%d", now2);
		start_date_ = today;
		start_time_ = "0000";
		end_date_ = today;
		end_time_ = "2359";
		serial_ = 1;
		active_ = false;
	}
}

// SAve settings for the contest ID
void qso_contest::save_settings() {
	Fl_Preferences contests(settings_, "Contests");
	contests.set("Current", contest_id_.c_str());
	Fl_Preferences c_settings(contests, contest_id_.c_str());
	c_settings.set("Fields", collection_.c_str());
	c_settings.set("Notes", notes_.c_str());
	c_settings.set("Start Date", start_date_.c_str());
	c_settings.set("Start Time", start_time_.c_str());
	c_settings.set("End Date", end_date_.c_str());
	c_settings.set("End Time", end_time_.c_str());
	c_settings.set("Next Serial", serial_);
	c_settings.set("Active", (int)active_);
}

// Populate collection
void qso_contest::populate_collection() {
	set<string> colls = fields_->coll_names();
	// LOad all collections that start "Contest/" to choice - removing that
	for (auto it = colls.begin(); it != colls.end(); it++) {
		if ((*it).substr(0, 8) == "Contest/") {
			ip_collection_->add((*it).substr(8).c_str());
		}
	}
}

// Check collection exists and if necessary open fields dialog
void qso_contest::check_collection() {
	bool copied = false;
	fields_->collection(collection_, "Contest/General", &copied);
	if (copied) {
		// Open the settings->fields
		menu::cb_mi_settings(this, (void*)settings::DLG_COLUMN);
	}
}

// Write Extracted ADIF
void qso_contest::write_adif() {
	// TODO: Need to write the code that puts the contest log into the extracted log
	menu::cb_mi_file_saveas(this, (void*)OT_EXTRACT);
}

// Write cabrillo
void qso_contest::write_cabrillo() {
	// TODO: Write Cabrillo format file
}

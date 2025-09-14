#include "contest_dialog.h"

#include "calendar.h"
#include "calendar_input.h"
#include "contest_algorithm.h"
#include "contest_data.h"
#include "field_choice.h"

#include "drawing.h"
#include "utils.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

extern std::map<std::string, contest_algorithm*>* algorithms_;
extern contest_data* contest_data_;
extern std::string VENDOR;
extern std::string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;
extern void open_html(const char*);

contest_dialog::contest_dialog(int X, int Y, int W, int H, const char* L) :
	page_dialog(X, Y, W, H, L)
	, contest_(nullptr)
	, contest_id_("")
	, contest_index_("")
{
	load_values();
	create_form(x(), y());
	populate_ct_index();
	populate_algorithm();
	update_contest();
	update_timeframe();
	update_algorithm();
	enable_widgets();

	// Create help window
}

contest_dialog::~contest_dialog() {
}

// Handle
int contest_dialog::handle(int event) {
	int result = page_dialog::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("contest_dialog.html");
			return true;
		}
		break;
	}
	return result;
}

// inherited methods

// Load values from settings
void contest_dialog::load_values() {
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences contest_settings(settings, "Contest");
	char* temp;
	contest_settings.get("Current ID", temp, "");
	contest_id_ = temp;
	free(temp);
	contest_settings.get("Current Index", temp, "");
	free(temp);
	contest_index_ = temp;
}

// Used to create the form
void contest_dialog::create_form(int X, int Y) {
	int curr_x = x() + WLLABEL;
	int curr_y = y() + GAP;

	w_contest_id_ = new field_input(curr_x, curr_y, WSMEDIT, HBUTTON, "Contest ID");
	w_contest_id_->align(FL_ALIGN_LEFT);
	w_contest_id_->callback(cb_id, &contest_id_);
	w_contest_id_->tooltip("Please select the contest ID (per ADIF) to edit");
	w_contest_id_->field_name("CONTEST_ID");

	curr_x += w_contest_id_->w() + WLABEL;
	w_contest_ix_ = new Fl_Input_Choice(curr_x, curr_y, WBUTTON, HBUTTON, "Index");
	w_contest_ix_->align(FL_ALIGN_LEFT);
	w_contest_ix_->callback(cb_index, &contest_index_);
	w_contest_ix_->input()->when(FL_WHEN_ENTER_KEY_ALWAYS);
	w_contest_ix_->tooltip("Please select the index (eg year) identifying specific contest");

	curr_x = x() + WLLABEL;
	curr_y += GAP + HBUTTON;

	w_algorithm_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Algorithm");
	w_algorithm_->align(FL_ALIGN_LEFT);
	w_algorithm_->tooltip("Please select the algorithm used for scoring and exchange");

	curr_y += GAP + HBUTTON;

	w_start_date_ = new calendar_input(curr_x, curr_y, WSMEDIT + HBUTTON, HBUTTON, "Contest start");
	w_start_date_->align(FL_ALIGN_LEFT);
	w_start_date_->tooltip("Please specify the start date (UTC) of the contest");

	curr_x += w_start_date_->w();
	w_start_time_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	w_start_time_->tooltip("Please specify the start time (UTC) of the contest");

	curr_y += HBUTTON;
	curr_x = x() + WLLABEL;

	w_finish_date_ = new calendar_input(curr_x, curr_y, WSMEDIT + HBUTTON, HBUTTON, "Contest finish");
	w_finish_date_->align(FL_ALIGN_LEFT);
	w_finish_date_->tooltip("Please specify the finish date (UTC) of the contest");

	curr_x += w_start_date_->w();
	w_finish_time_ = new Fl_Int_Input(curr_x, curr_y, WBUTTON, HBUTTON);
	w_finish_time_->tooltip("Please specify the finish time (UTC) of the contest");

	curr_y += HBUTTON + GAP;
	curr_x = x() + WLLABEL;

	end();
	show();
}

// Used to write settings back
void contest_dialog::save_values() {
	if (!contest_) contest_ = contest_data_->get_contest(contest_id_, contest_index_, true);
	if (contest_) {
		contest_->algorithm = w_algorithm_->value();
		std::string start_date = w_start_date_->value();
		std::string start_time = w_start_time_->value();
		tm* start = new tm;
		start->tm_year = std::stoi(start_date.substr(0, 4)) - 1900;
		start->tm_mon = std::stoi(start_date.substr(4, 2)) - 1;
		start->tm_mday = std::stoi(start_date.substr(6, 2));
		start->tm_hour = std::stoi(start_time.substr(0, 2));
		start->tm_min = std::stoi(start_time.substr(2, 2));
		start->tm_sec = 0;
		start->tm_isdst = false;
		std::string finish_date = w_finish_date_->value();
		std::string finish_time = w_finish_time_->value();
		tm* finish = new tm;
		finish->tm_year = std::stoi(finish_date.substr(0, 4)) - 1900;
		finish->tm_mon = std::stoi(finish_date.substr(4, 2)) - 1;
		finish->tm_mday = std::stoi(finish_date.substr(6, 2));
		finish->tm_hour = std::stoi(finish_time.substr(0, 2));
		finish->tm_min = std::stoi(finish_time.substr(2, 2));
		finish->tm_sec = 0;
		finish->tm_isdst = false;
#ifdef _WIN32
		contest_->date.start = std::chrono::system_clock::from_time_t(_mkgmtime(start));
		contest_->date.finish = std::chrono::system_clock::from_time_t(_mkgmtime(finish));
#else
		contest_->date.start = std::chrono::system_clock::from_time_t(timegm(start));
		contest_->date.finish = std::chrono::system_clock::from_time_t(timegm(finish));
#endif
	}
	contest_data_->save_data();
}

// Used to enable/disable specific widget - any widgets enabled musr be attributes
void contest_dialog::enable_widgets() {
}

// Update contest values
void contest_dialog::update_contest() {
	contest_ = contest_data_->get_contest(contest_id_, contest_index_);
}

// Update logging fields
void contest_dialog::update_algorithm() {
	if (contest_) {
		w_algorithm_->value(contest_->algorithm.c_str());
	}
	else {
		w_algorithm_->value(0);
	}
}

// Update timeframe
void contest_dialog::update_timeframe() {
	time_t start;
	time_t finish;
	if (contest_) {
		ct_date_t* timeframe = &contest_->date;
		start = std::chrono::system_clock::to_time_t(timeframe->start);
		finish = std::chrono::system_clock::to_time_t(timeframe->finish);
	}
	else {
		start = time(nullptr);
		finish = time(nullptr);
	}
	tm* stm = gmtime(&start);
	char temp[32];
	strftime(temp, sizeof(temp), ADIF_DATEFORMAT, stm);
	w_start_date_->value(temp);
	strftime(temp, sizeof(temp), ADIF_HOURFORMAT, stm);
	w_start_time_->value(temp);
	tm* ftm = gmtime(&finish);
	strftime(temp, sizeof(temp), ADIF_DATEFORMAT, ftm);
	w_finish_date_->value(temp);
	strftime(temp, sizeof(temp), ADIF_HOURFORMAT, ftm);
	w_finish_time_->value(temp);
}

// Callbacks
// Contest ID field_input
void contest_dialog::cb_id(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->contest_id_ = ((field_input*)w)->value();
	that->contest_index_ = "";
	that->w_contest_ix_->value("");
	that->update_contest();
	that->update_algorithm();
	that->update_timeframe();
}

// Contest index
void contest_dialog::cb_index(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->contest_index_ = ((Fl_Input_Choice*)w)->value();
	that->update_contest();
	that->update_algorithm();
	that->update_timeframe();
}

// Populate contest index choice
void contest_dialog::populate_ct_index() {
	std::set<std::string>* indices = contest_data_->get_contest_indices(contest_id_);
	w_contest_ix_->clear();
	w_contest_ix_->add("");
	if (indices) {
		for (auto it : *indices) {
			w_contest_ix_->add(it.c_str());
		}
	}
	w_contest_ix_->value("");
}

// Populate logged fields choice
void contest_dialog::populate_algorithm() {
	w_algorithm_->clear();
	if (algorithms_) {
		for (auto it : *algorithms_) {
			w_algorithm_->add(it.first.c_str());
		}
	}
}


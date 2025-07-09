#include "contest_dialog.h"

#include "calendar.h"
#include "calendar_input.h"
#include "contest_data.h"
#include "field_choice.h"
#include "fields.h"

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

extern contest_data* contest_data_;
extern fields* fields_;
extern string VENDOR;
extern string PROGRAM_ID;

const char EXCHANGE_HELP[] =
"Enter the fields that are to be used for the exchange. Field names"
" should be entered in angle brackets. So, for instance "
"the common RS(T) + serial number exchanged shoule be entered as:\n"
"\t\"<RST_SEND><STX>\" for the sent exchange and:\n"
"\t\"<RST_RCVD><SRX>\" for the received exchange.\n"
"Field names are those defined in the ADIF specification.";

const char SCORING_HELP[] =
"Enter the scoring algorithm. This is usually in two parts: points given for a QSO, "
"and a multiplier. Per QSO examples are:\n"
"\t\"<CONT>!=<MY_CONT>?5:1\" if QSOS with another continent score 5 and within your own 1.\n"
"\t\"<DXCC>!=<MY_DXCC>?1:0\" if QSOs with another DXCC entity score 1 and none for your own.\n"
"Field names are those defined in the ADIF specification, with the following "
"exceptions - <GRID4> and <MY_GRID4> for a 4-character gridsquare rather than what is "
"captured in <GRIDSQUARE> etc. The special entry \"*CUSTOM*\" is used for a custom or "
"unsupported scoring algorithm.\n"
"Multiplier examples are:\n"
"\t\"COUNT(<DXCC>)\" for the multiplier the number of DXCCs worked.\n"
"\t\"COUNT(<DXCC>*<BAND>)\" for the number of DXCCs worked on each band.\n";

contest_dialog::contest_dialog(int X, int Y, int W, int H, const char* L) :
	page_dialog(X, Y, W, H, L)
	, contest_(nullptr)
	, contest_id_("")
	, contest_index_("")
	, exchange_(nullptr)
	, exchange_id_("")
	, win_help_(nullptr)
{
	load_values();
	create_form(x(), y());
	populate_ct_index();
	populate_logging();
	populate_exch_id();
	populate_score_id();
	update_contest();
	update_logging();
	update_timeframe();
	update_exchange(false);
	scoring_id_ = SCORING_ALGOS[0].first;
	update_scoring(false);
	enable_widgets();

	// Create help window
	Fl_Group* sv = Fl_Group::current();
	Fl_Group::current(nullptr);
	win_help_ = new Fl_Window(300, 200);
	Fl_Text_Display* dis = new Fl_Text_Display(0, 0, 300, 200);
	help_buffer_ = new Fl_Text_Buffer;
	dis->buffer(help_buffer_);
	dis->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	win_help_->end();
	win_help_->hide();
	Fl_Group::current(sv);
}

contest_dialog::~contest_dialog() {
	Fl::delete_widget(win_help_);
}

// inherited methods

// Load values from settings
void contest_dialog::load_values() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
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

	w_logging_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Fields logged");
	w_logging_->align(FL_ALIGN_LEFT);
	w_logging_->tooltip("Please select the collection of fields needed to log for this contest");

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

	w_exchange_id_ = new Fl_Input_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Exchange");
	w_exchange_id_->align(FL_ALIGN_LEFT);
	w_exchange_id_->callback(cb_exch_id, &exchange_id_);
	w_exchange_id_->tooltip("Please spcify the identifier for the contest exchange");

	curr_x += w_exchange_id_->w() + GAP;
	w_exchange_help_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Help");
	w_exchange_help_->callback(cb_exch_help, nullptr);
	w_exchange_help_->tooltip("Click for guidance on coding exchanges");

	curr_y += HBUTTON;
	curr_x = x() + WLLABEL;

	w_exch_send_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON, "Send");
	w_exch_send_->align(FL_ALIGN_LEFT);
	w_exch_send_->when(FL_WHEN_CHANGED);
	w_exch_send_->tooltip("Enter the send exchange template");

	curr_y += HBUTTON;

	w_exch_receive_ = new Fl_Input(curr_x, curr_y, WEDIT, HBUTTON, "Receive");
	w_exch_receive_->align(FL_ALIGN_LEFT);
	w_exch_receive_->when(FL_WHEN_CHANGED);
	w_exch_receive_->tooltip("Enter the receive exchange template");

	curr_y += HBUTTON + GAP;

	w_scoring_id_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Scoring");
	w_scoring_id_->align(FL_ALIGN_LEFT);
	w_scoring_id_->callback(cb_score_id, &scoring_id_);
	w_scoring_id_->tooltip("Please spcify the identifier for the scoring algorithm");

	curr_x += WSMEDIT;
	w_scoring_descr_ = new Fl_Multiline_Output(curr_x, curr_y, WEDIT, 5 * HBUTTON);
	w_scoring_descr_->box(FL_FLAT_BOX);
	w_scoring_descr_->color(FL_BACKGROUND_COLOR);
	w_scoring_descr_->wrap(true);
	w_scoring_descr_->tooltip("describes the scoring algorithm in more detail");

	curr_y += w_scoring_descr_->h() + GAP;

	curr_x = x() + WLLABEL;

	end();
	show();
}

// Used to write settings back
void contest_dialog::save_values() {
	if (!contest_) contest_ = contest_data_->get_contest(contest_id_, contest_index_, true);
	if (contest_) {
		contest_->fields = w_logging_->value();
		string start_date = w_start_date_->value();
		string start_time = w_start_time_->value();
		tm* start = new tm;
		start->tm_year = stoi(start_date.substr(0, 4)) - 1900;
		start->tm_mon = stoi(start_date.substr(4, 2)) - 1;
		start->tm_mday = stoi(start_date.substr(6, 2));
		start->tm_hour = stoi(start_time.substr(0, 2));
		start->tm_min = stoi(start_time.substr(2, 2));
		start->tm_sec = 0;
		start->tm_isdst = false;
		string finish_date = w_finish_date_->value();
		string finish_time = w_finish_time_->value();
		tm* finish = new tm;
		finish->tm_year = stoi(finish_date.substr(0, 4)) - 1900;
		finish->tm_mon = stoi(finish_date.substr(4, 2)) - 1;
		finish->tm_mday = stoi(finish_date.substr(6, 2));
		finish->tm_hour = stoi(finish_time.substr(0, 2));
		finish->tm_min = stoi(finish_time.substr(2, 2));
		finish->tm_sec = 0;
		finish->tm_isdst = false;
#ifdef _WIN32
		contest_->date.start = system_clock::from_time_t(_mkgmtime(start));
		contest_->date.finish = system_clock::from_time_t(_mkgmtime(finish));
#else
		contest_->date.start = system_clock::from_time_t(timegm(start));
		contest_->date.finish = system_clock::from_time_t(timegm(finish));
#endif
		contest_->scoring = w_scoring_id_->text();
	}
	if (!exchange_) exchange_ = contest_data_->get_exchange(exchange_id_, true);
	if (exchange_) {
		if (contest_) contest_->exchange = exchange_id_;
		exchange_->sending = w_exch_send_->value();
		exchange_->receive = w_exch_receive_->value();
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
void contest_dialog::update_logging() {
	if (contest_) {
		w_logging_->value(contest_->fields.c_str());
	}
	else {
		w_logging_->value(0);
	}
}

// Update timeframe
void contest_dialog::update_timeframe() {
	time_t start;
	time_t finish;
	if (contest_) {
		ct_date_t* timeframe = &contest_->date;
		start = chrono::system_clock::to_time_t(timeframe->start);
		finish = chrono::system_clock::to_time_t(timeframe->finish);
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

void contest_dialog::update_exchange(bool from_contest) {
	if (from_contest) {
		if (contest_) {
			exchange_id_ = contest_->exchange;
		}
		else {
			exchange_id_ = "";
		}
	}
	w_exchange_id_->value(exchange_id_.c_str());
	exchange_ = contest_data_->get_exchange(exchange_id_);
	if (exchange_) {
		w_exch_send_->value(exchange_->sending.c_str());
		w_exch_receive_->value(exchange_->receive.c_str());
	}
	else {
		w_exch_send_->value("");
		w_exch_receive_->value("");
	}
}

void contest_dialog::update_scoring(bool from_contest) {
	if (from_contest) {
		if (contest_) {
			scoring_id_ = contest_->scoring;
		}
	}
	int index = w_scoring_id_->find_index(scoring_id_.c_str());
	w_scoring_id_->value(index);
	w_scoring_descr_->value(SCORING_ALGOS[index].second.c_str());
}

// Callbacks
// Contest ID field_input
void contest_dialog::cb_id(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->contest_id_ = ((field_input*)w)->value();
	that->contest_index_ = "";
	that->w_contest_ix_->value("");
	that->update_contest();
	that->update_logging();
	that->update_timeframe();
	that->update_exchange(true);
	that->update_scoring(true);
}

// Contest index
void contest_dialog::cb_index(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->contest_index_ = ((Fl_Input_Choice*)w)->value();
	that->update_contest();
	that->update_logging();
	that->update_timeframe();
	that->update_exchange(true);
	that->update_scoring(true);
}


// Exchange ID
void contest_dialog::cb_exch_id(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->exchange_id_ = ((Fl_Input_Choice*)w)->value();
	that->update_exchange(false);
}

// Exchange help
void contest_dialog::cb_exch_help(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->display_help(EXCHANGE_HELP);
}

// Scoring ID
void contest_dialog::cb_score_id(Fl_Widget* w, void* v) {
	contest_dialog* that = ancestor_view<contest_dialog>(w);
	that->scoring_id_ = ((Fl_Choice*)w)->text();
	that->update_scoring(false);
}

// Populate contest index choice
void contest_dialog::populate_ct_index() {
	set<string>* indices = contest_data_->get_contest_indices(contest_id_);
	w_contest_ix_->clear();
	w_contest_ix_->add("");
	if (indices) {
		for (auto it : *indices) {
			w_contest_ix_->add(it.c_str());
		}
	}
	w_contest_ix_->value("");
}

// Populate exchange ID choice
void contest_dialog::populate_exch_id() {
	set<string>* indices = contest_data_->get_exchange_indices();
	w_exchange_id_->clear();
	w_exchange_id_->add("");
	if (indices) {
		for (auto it : *indices) {
			w_exchange_id_->add(it.c_str());
		}
	}
}
// Populate scoring ID choice
void contest_dialog::populate_score_id() {
	w_scoring_id_->clear();
	for (auto it : SCORING_ALGOS) {
		w_scoring_id_->add(it.first.c_str());
	}
	w_scoring_id_->value(0);
}

// Populate logged fields choice
void contest_dialog::populate_logging() {
	set<string> colls = fields_->coll_names();
	// LOad all collections that start "Contest/" to choice - removing that
	for (auto it = colls.begin(); it != colls.end(); it++) {
		if ((*it).substr(0, 8) == "Contest/") {
			w_logging_->add((*it).substr(8).c_str());
		}
	}
}

// Display help window
void contest_dialog::display_help(const char* text) {
	help_buffer_->text(text);
	win_help_->show();
}


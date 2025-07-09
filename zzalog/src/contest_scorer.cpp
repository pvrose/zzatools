#include "contest_scorer.h"

#include "book.h"
#include "contest_data.h"
#include "extract_data.h"
#include "record.h"

#include "utils.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Output.H>

extern book* book_;
extern contest_data* contest_data_;
extern string VENDOR;
extern string PROGRAM_ID;

contest_scorer::contest_scorer(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, qsos_(nullptr)
	, qso_(nullptr)
	, qso_points_(0)
	, multiplier_(0)
	, total_(0)
	, qso_points_p_(0)
	, multiplier_p_(0)
	, total_p_(0)
{
	load_data();
	create_form();
	populate_contest();
	enable_widgets();
}
contest_scorer::~contest_scorer() {
	save_data();
}

void contest_scorer::load_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences contest_settings(settings, "Contest");
	contest_settings.get("Contest", contest_id_, "");
	contest_settings.get("Index", contest_index_, "");
	ct_data_t* contest = contest_data_->get_contest(contest_id_, contest_index_);
	if (contest) {
		populate_timeframe(contest);
		int itemp;
		contest_settings.get("Active", itemp, (int)false);
		populate_status(contest, itemp);
		scoring_id_ = contest->scoring;
		exchange_ = contest_data_->get_exchange(contest->exchange);
		resume_contest();
		contest_settings.get("Next Serial", next_serial_, 1);
	}
	else {
		char temp[25];
		time_t t1 = chrono::system_clock::to_time_t(chrono::system_clock::now());
		tm* t2 = gmtime(&t1);
		strftime(temp, sizeof(temp), "%Y%m%d 0000", t2);
		start_time_ = temp;
		strftime(temp, sizeof(temp), "%Y%m%d 2359", t2);
		finish_time_ = temp;
		contest_status_ = NO_CONTEST;
		scoring_id_ = "";
		exchange_ = nullptr;
		next_serial_ = 1;
	}
}

void contest_scorer::create_form() {
	int curr_x = x() + GAP;
	int curr_y = y() + GAP;

	curr_x += WLABEL;
	w_contest_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Contest");
	w_contest_->align(FL_ALIGN_LEFT);
	w_contest_->callback(cb_contest);
	w_contest_->tooltip("Select the contest");

	curr_y += HBUTTON + GAP;

	w_start_time_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON, "Start");
	w_start_time_->value(start_time_.c_str());
	w_start_time_->tooltip("Displays the start date/time of the contest");
	curr_y += HBUTTON;

	w_finish_time_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON, "Finish");
	w_finish_time_->value(finish_time_.c_str());
	w_finish_time_->tooltip("Displays the finish date/time of the contest");
	curr_y += HBUTTON + GAP;

	w_status_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON);
	w_status_->callback(cb_bn_status);
	w_status_->tooltip("Displays the status: can toggle between ACTIVE/PAUSED");

	curr_x = x() + GAP;
	curr_y += HBUTTON + GAP;

	g_scores_ = new Fl_Group(curr_x, curr_y, w() - GAP - GAP, 4 * HBUTTON, "Scores");
	g_scores_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	const int WOP = (g_scores_->w()) / 7;

	curr_x = g_scores_->x() + 5 * WOP;
	curr_y = g_scores_->y();

	w_number_qsos_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON, "# QSOs");
	w_number_qsos_->align(FL_ALIGN_LEFT);
	w_number_qsos_->tooltip("Displays number of QSOs contributing to contest score");

	curr_x = g_scores_->x();
	curr_y += HBUTTON + HBUTTON;

	Fl_Box* b1 = new Fl_Box(curr_x, curr_y, WOP, HBUTTON, "Log");
	b1->box(FL_FLAT_BOX);

	curr_x += WOP;

	w_qso_points_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON, "QSO pts");
	w_qso_points_->align(FL_ALIGN_TOP);
	w_qso_points_->tooltip("Displays numbetr of points from QSOs");

	curr_x += WOP * 2;
	w_multiplier_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON, "Mult.");
	w_multiplier_->align(FL_ALIGN_TOP);
	w_multiplier_->tooltip("Displayes the multiplier accrued so far");

	curr_x += WOP * 2;

	w_total_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON, "Total");
	w_total_->align(FL_ALIGN_TOP);
	w_total_->textfont(FL_BOLD);
	w_total_->tooltip("Displays the total points score accrued so far");

	curr_x = g_scores_->x();
	curr_y += HBUTTON;

	b1 = new Fl_Box(curr_x, curr_y, WOP, HBUTTON, "+\316\224");
	b1->box(FL_FLAT_BOX);

	curr_x += WOP;

	w_qso_points_2_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON);
	w_qso_points_2_->textfont(FL_ITALIC);
	w_qso_points_2_->tooltip("Displays numbetr of points including check QSO");

	curr_x += WOP * 2;
	w_multiplier_2_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON);
	w_multiplier_2_->textfont(FL_ITALIC);
	w_multiplier_2_->tooltip("Displayes the multiplier including check QSO");

	curr_x += WOP * 2;

	w_total_2_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON);
	w_total_2_->textfont(FL_BOLD | FL_ITALIC);
	w_total_2_->tooltip("Displays the total points scored including check QSO");


	curr_y += HBUTTON;

	g_scores_->end();

	end();
	show();
}

void contest_scorer::save_data() {
	Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences contest_settings(settings, "Contest");
	contest_settings.set("Contest", contest_id_);
	contest_settings.set("Index", contest_index_);
	contest_settings.set("Active", contest_status_ == ACTIVE);
	contest_settings.set("Next Serial", next_serial_);
}

void contest_scorer::enable_widgets() {
	char text[32];
	switch (contest_status_) {
	case NO_CONTEST:
		if (contest_id_ == "") w_contest_->value(0);
		w_contest_->activate();
		w_start_time_->deactivate();
		w_start_time_->value("");
		w_finish_time_->deactivate();
		w_finish_time_->value("");
		w_status_->label("NO TEST");
		g_scores_->deactivate();
		break;
	case FUTURE:
		w_contest_->activate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("TO COME");
		g_scores_->deactivate();
		break;
	case ACTIVE:
		w_contest_->deactivate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("TO COME");
		g_scores_->activate();
		snprintf(text, sizeof(text), "%zd", qsos_->size());
		w_number_qsos_->value(text);
		snprintf(text, sizeof(text), "%d", qso_points_);
		w_qso_points_->value(text);
		snprintf(text, sizeof(text), "%d", multiplier_);
		w_multiplier_->value(text);
		snprintf(text, sizeof(text), "%d", total_);
		w_total_->value(text);
		snprintf(text, sizeof(text), "%d", qso_points_p_);
		w_qso_points_2_->value(text);
		snprintf(text, sizeof(text), "%d", multiplier_p_);
		w_multiplier_2_->value(text);
		snprintf(text, sizeof(text), "%d", total_p_);
		w_total_2_->value(text);
		break;
	case PAUSED:
		w_contest_->deactivate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("PAUSED");
		g_scores_->activate();
		snprintf(text, sizeof(text), "%zd", qsos_->size());
		w_number_qsos_->value(text);
		snprintf(text, sizeof(text), "%d", qso_points_);
		w_qso_points_->value(text);
		snprintf(text, sizeof(text), "%d", multiplier_);
		w_multiplier_->value(text);
		snprintf(text, sizeof(text), "%d", total_);
		w_total_->value(text);
		snprintf(text, sizeof(text), "%d", qso_points_p_);
		w_qso_points_2_->value(text);
		snprintf(text, sizeof(text), "%d", multiplier_p_);
		w_multiplier_2_->value(text);
		snprintf(text, sizeof(text), "%d", total_p_);
		w_total_2_->value(text);
		break;
	case PAST:
		w_contest_->activate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("PAST");
		g_scores_->activate();
		snprintf(text, sizeof(text), "%zd", qsos_->size());
		w_number_qsos_->value(text);
		snprintf(text, sizeof(text), "%d", qso_points_);
		w_qso_points_->value(text);
		snprintf(text, sizeof(text), "%d", multiplier_);
		w_multiplier_->value(text);
		snprintf(text, sizeof(text), "%d", total_);
		w_total_->value(text);
		snprintf(text, sizeof(text), "%d", qso_points_p_);
		w_qso_points_2_->value(text);
		snprintf(text, sizeof(text), "%d", multiplier_p_);
		w_multiplier_2_->value(text);
		snprintf(text, sizeof(text), "%d", total_p_);
		w_total_2_->value(text);
		break;
	}
}

// callbacks
void contest_scorer::cb_contest(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	contest_scorer* that = ancestor_view<contest_scorer>(w);
	const Fl_Menu_Item* item = ch->mvalue();
	that->contest_ = (ct_data_t*)item->user_data();
	that->populate_timeframe(that->contest_);
	that->populate_status(that->contest_, true);
	that->next_serial_ = 1;
	if (that->contest_) {
		that->scoring_id_ = that->contest_->scoring;
		that->exchange_ = contest_data_->get_exchange(that->contest_->exchange);
	}
	else {
		that->scoring_id_ = "";
		that->exchange_ = nullptr;
	}
	that->qsos_ = new extract_data;
	that->enable_widgets();
}

void contest_scorer::cb_bn_status(Fl_Widget* w, void* v) {
	contest_scorer* that = ancestor_view<contest_scorer>(w);
	switch (that->contest_status_) {
	case NO_CONTEST:
	case FUTURE:
		break;
	case ACTIVE:
		that->contest_status_ = PAUSED;
		break;
	case PAUSED:
		that->contest_status_ = ACTIVE;
		break;
	case PAST:
		that->contest_status_ = NO_CONTEST;
		that->contest_id_ = "";
		break;
	}
	that->enable_widgets();
}

// Add record 
void contest_scorer::add_qso(qso_num_t qso_number) {
	// It matches, copy reference to this book
	record* qso = book_->get_record(qso_number, false);
	qso_ = qso;
	qsos_->push_back(qso);
	qsos_->map_record(qso_number);
	score_qso(qso, false);
}

// Check record 
void contest_scorer::check_qso(qso_num_t qso_number) {
	// It matches, copy reference to this book
	record* qso = book_->get_record(qso_number, false);
	qso_ = qso;
	score_qso(qso, true);
}

// Load QSOs - after restarting zzalog
void contest_scorer::resume_contest() {
	int ix = book_->size() - 1;
	chrono::system_clock::time_point start = contest_->date.start;
	chrono::system_clock::time_point finish = contest_->date.finish;
	record* qso = book_->get_record(ix, false);
	chrono::system_clock::time_point ts = qso->ctimestamp();
	while (ts > start) {
		// Add all QSOs it this contest (ID equivalent and timestamp within timeframe
		if (qso->item("CONTEST_ID") == contest_id_ && start < ts && finish > ts) {
			add_qso(ix);
		}
		ix--;
		qso = book_->get_record(ix, false);
		ts = qso->ctimestamp();
	}
}

// Copy contest to timeframe
void contest_scorer::populate_timeframe(ct_data_t* ct) {
	if (ct) {
		char temp[25];
		time_t t1 = chrono::system_clock::to_time_t(ct->date.start);
		tm* t2 = gmtime(&t1);
		strftime(temp, sizeof(temp), "%Y%m%d %H%M", t2);
		start_time_ = temp;
		t1 = chrono::system_clock::to_time_t(ct->date.finish);
		t2 = gmtime(&t1);
		strftime(temp, sizeof(temp), "%Y%m%d %H%M", t2);
		finish_time_ = temp;
	}
	else {
		start_time_ = "";
		finish_time_ = "";
	}
}

// Copy contest to status
void contest_scorer::populate_status(ct_data_t* ct, bool previous) {
	if (ct) {
		chrono::system_clock::time_point today = chrono::system_clock::now();
		if (today < ct->date.start) contest_status_ = FUTURE;
		else if (today > ct->date.finish) contest_status_ = PAST;
		else {
			if (previous) contest_status_ = ACTIVE;
			else contest_status_ = PAUSED;
		}
	}
	else {
		contest_status_ = NO_CONTEST;
	}
}

void contest_scorer::populate_contest() {
	int count = contest_data_->get_contest_count();
	w_contest_->clear();
	w_contest_->add("", 0, nullptr, nullptr);
	for (auto ix = 0; ix < count; ix++) {
		ct_entry_t* info = contest_data_->get_contest_info(ix);
		string text = info->id + ":" + info->index;
		// Adding the pointer to the contest definition as user data
		w_contest_->add(text.c_str(), 0, nullptr, (void*)(info->definition));
	}
}

// Score QSP
void contest_scorer::score_qso(record* qso, bool check_only) {
	// TODO add MY_... data to QSO
	if (scoring_id_ == "Basic") {
		score_basic(qso, check_only);
	}
}

// Individual algorithms
void contest_scorer::score_basic(record* qso, bool check_only) {
	// Multiplier is number of DXCCs worked on each band
	string multiplier = qso->item("DXCC") + " " + qso->item("BAND");
	multiplier_ = multipliers_.size();
	if (multipliers_.find(multiplier) == multipliers_.end()) {
		multiplier_p_ = multiplier_ + 1;
	}
	// QSO points - 1 per QSO in different DXCC
	if (qso->item("DXCC") != qso->item("MY_DXCC")) qso_points_p_ = qso_points_ + 1;
	total_p_ = multiplier_p_ * qso_points_p_;
	if (!check_only) {
		multiplier_ = multiplier_p_;
		qso_points_ = qso_points_p_;
		total_ = total_p_;
	}
	enable_widgets();
}

// In contest mode
bool contest_scorer::contest_active() {
	return contest_status_ == ACTIVE;
}

// Returns value of ADIF.CONTEST_ID
string contest_scorer::contest_id() {
	return contest_id_;
}

// Increment next serial number
void contest_scorer::increment_serial() {
	next_serial_++;
}

// Returns the fields::collection name
string contest_scorer::collection() {
	if (contest_) return "Contest/" + contest_->fields;
	else return "";
}

// Returns the serial number
string contest_scorer::serial() {
	char text[10];
	snprintf(text, sizeof(text), "%03d", next_serial_);
	return string(text);
}

// Returns the created exchange
string contest_scorer::exchange() {
	if (exchange_) return qso_->item_merge(exchange_->sending);
	else return "";
}

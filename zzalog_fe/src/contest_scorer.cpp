#include "contest_scorer.h"

#include "book.h"
#include "contest_algorithm.h"
#include "contest_data.h"
#include "cty_data.h"
#include "extract_data.h"
#include "main.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "record.h"
#include "settings.h"
#include "stn_data.h"

#include "utils.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>

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
	, d_qso_points_(0)
	, d_multiplier_(0)
	, algorithm_(nullptr)
{
	load_data();
	create_form();
	populate_contest();
	enable_widgets();
}
contest_scorer::~contest_scorer() {
	save_data();
}

// Handle
int contest_scorer::handle(int event) {
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
			open_html("contest_scorer.html");
			return true;
		}
		break;
	}
	return result;
}

void contest_scorer::load_data() {
	settings top_settings;
	settings behav_settings(&top_settings, "Behaviour");
	settings contest_settings(&behav_settings, "Contest");
	contest_settings.get<std::string>("Contest", contest_id_, "");
	contest_settings.get<std::string>("Index", contest_index_, "");
	contest_settings.get<contest_scorer::ct_status>("Active", contest_status_, NO_CONTEST);
	contest_settings.get("Next Serial", next_serial_, 1);
}

void contest_scorer::create_form() {
	int curr_x = x() + GAP;
	int curr_y = y() + GAP;

	curr_x += WLABEL;
	w_contest_ = new Fl_Choice(curr_x, curr_y, WSMEDIT, HBUTTON, "Contest");
	w_contest_->align(FL_ALIGN_LEFT);
	w_contest_->callback(cb_contest);
	w_contest_->tooltip("Select the contest");

	curr_y += HBUTTON;

	w_start_time_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON, "Start");
	w_start_time_->value(start_time_.c_str());
	w_start_time_->tooltip("Displays the start date/time of the contest");
	curr_y += HBUTTON;

	w_finish_time_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON, "Finish");
	w_finish_time_->value(finish_time_.c_str());
	w_finish_time_->tooltip("Displays the finish date/time of the contest");
	curr_y += HBUTTON;

	w_status_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON);
	w_status_->callback(cb_bn_status);
	w_status_->tooltip("Displays the status: can toggle between ACTIVE/PAUSED");

	curr_x = x() + GAP;
	curr_y += HBUTTON + GAP;

	const int WGR = (w() - GAP - GAP);
	const int WOP = WGR / 7;

	g_exch_ = new Fl_Group(curr_x, curr_y, WGR, HBUTTON * 3, "Exchanges");
	g_exch_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	curr_x += WOP;
	curr_y += HBUTTON;
	w_rx_exchange_ = new Fl_Input(curr_x, curr_y, 4 * WOP, HBUTTON, "RX");
	w_rx_exchange_->align(FL_ALIGN_LEFT);
	w_rx_exchange_->tooltip("Enter the received exchange - hit Parse to copy to QSO");

	curr_x += w_rx_exchange_->w();
	w_parse_ = new Fl_Button(curr_x, curr_y, 2 * WOP, HBUTTON, "Parse");
	w_parse_->callback(cb_parse, nullptr);
	w_parse_->tooltip("Copy exchanges to fields in QSO");

	curr_x = g_exch_->x() + WOP;
	curr_y += HBUTTON;
	w_tx_exchange_ = new Fl_Output(curr_x, curr_y, 4 * WOP, HBUTTON, "TX");
	w_tx_exchange_->tooltip("Shows what the next exchange you should send is");

	curr_x += w_tx_exchange_->w();
	w_next_serno_ = new Fl_Counter(curr_x, curr_y, 2 * WOP, HBUTTON);
	w_next_serno_->type(FL_SIMPLE_COUNTER);
	w_next_serno_->step(1.0);
	w_next_serno_->callback(cb_serno, &next_serial_);
	w_next_serno_->tooltip("Change the value of the next serial number to send");

	g_exch_->end();
	curr_x = g_exch_->x();
	curr_y += HBUTTON + GAP;

	g_scores_ = new Fl_Group(curr_x, curr_y, WGR, 4 * HBUTTON, "Scores");
	g_scores_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	

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

	w_qso_points_d_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON);
	w_qso_points_d_->textfont(FL_ITALIC);
	w_qso_points_d_->tooltip("Displays additional QSO points from QSO");

	curr_x += WOP * 2;
	w_multiplier_d_ = new Fl_Output(curr_x, curr_y, WOP * 2, HBUTTON);
	w_multiplier_d_->textfont(FL_ITALIC);
	w_multiplier_d_->tooltip("Displays additional multiplier from QSO");

	curr_x = g_scores_->x();
	curr_y += HBUTTON;

	b1 = new Fl_Box(curr_x, curr_y, WOP, HBUTTON, "=");
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

	curr_x = x();

	end();
	show();
}

void contest_scorer::save_data() {
	settings top_settings;
	settings behav_settings(&top_settings, "Behaviour");
	settings contest_settings(&behav_settings, "Contest");
	contest_settings.set("Contest", contest_id_);
	contest_settings.set("Index", contest_index_);
	contest_settings.set<contest_scorer::ct_status>("Active", contest_status_);
	contest_settings.set("Next Serial", next_serial_);
}

void contest_scorer::enable_widgets() {
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
		g_exch_->deactivate();
		break;
	case FUTURE:
		w_contest_->activate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("TO COME");
		g_scores_->deactivate();
		g_exch_->deactivate();
		break;
	case ACTIVE:
		w_contest_->deactivate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("ACTIVE");
		g_scores_->activate();
		g_exch_->activate();
		if (algorithm_) {
			w_tx_exchange_->value(algorithm_->generate_exchange(qso_).c_str());
		}
		if (algorithm_ && algorithm_->uses_serno()) {
			w_next_serno_->activate();
		}
		else {
			w_next_serno_->deactivate();
		}
		copy_points_to_display();
		break;
	case PAUSED:
		w_contest_->deactivate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("PAUSED");
		g_scores_->activate();
		g_exch_->deactivate();
		copy_points_to_display();
		break;
	case PAST:
		w_contest_->activate();
		w_start_time_->activate();
		w_finish_time_->activate();
		w_start_time_->value(start_time_.c_str());
		w_finish_time_->value(finish_time_.c_str());
		w_status_->label("PAST");
		g_scores_->activate();
		g_exch_->deactivate();
		copy_points_to_display();
		break;
	}
}

// Copy points to display
void contest_scorer::copy_points_to_display() {
	char text[32];
	if (qsos_) snprintf(text, sizeof(text), "%zd", qsos_->size());
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
	if (qso_ && qso_->item("CALL").length()) {
		snprintf(text, sizeof(text), "%d", d_qso_points_);
		w_qso_points_d_->value(text);
		snprintf(text, sizeof(text), "%d", d_multiplier_);
		w_multiplier_d_->value(text);
	}
	else {
		w_qso_points_d_->value("");
		w_multiplier_d_->value("");
	}
	snprintf(text, sizeof(text), "%d", total_p_);
	w_total_2_->value(text);
}

// callbacks
void contest_scorer::cb_contest(Fl_Widget* w, void* v) {
	Fl_Choice* ch = (Fl_Choice*)w;
	contest_scorer* that = ancestor_view<contest_scorer>(w);
	const Fl_Menu_Item* item = ch->mvalue();
	ct_entry_t* entry = (ct_entry_t*)item->user_data();
	that->contest_ = entry->definition;
	that->contest_id_ = entry->id;
	that->contest_index_ = entry->index;
	that->change_contest();
	// TODO the following should be in change_contest
	that->populate_timeframe();
	that->populate_status();
	that->next_serial_ = 1;
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

// Parse the recived exchange and update QSO with it
void contest_scorer::cb_parse(Fl_Widget* w, void* v) {
	contest_scorer* that = ancestor_view<contest_scorer>(w);
	qso_data* data = ancestor_view<qso_data>(that);
	that->algorithm_->parse_exchange(that->qso_, that->w_rx_exchange_->value());
	data->update_qso(that->qso_number_);
}

// Changethe serial number
void contest_scorer::cb_serno(Fl_Widget* w, void* v) {
	contest_scorer* that = ancestor_view<contest_scorer>(w);
	double val = ((Fl_Counter*)w)->value();
	*(int*)v = val;
	that->enable_widgets();
}

// Change the contest
void contest_scorer::change_contest() {
	if (contest_) {
		populate_timeframe();
		populate_status();
		create_algo();
		resume_contest();
	}
	else {
		char temp[25];
		time_t t1 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm* t2 = gmtime(&t1);
		strftime(temp, sizeof(temp), "%Y%m%d 0000", t2);
		start_time_ = temp;
		strftime(temp, sizeof(temp), "%Y%m%d 2359", t2);
		finish_time_ = temp;
		contest_status_ = NO_CONTEST;
		next_serial_ = -1;
	}

}

// Use the appropriate algorithm and attach it to this.
void contest_scorer::create_algo() {
	algorithm_ = algorithms_->at(contest_id_);
	algorithm_->attach(this);
}

// Add record 
void contest_scorer::add_qso(record* qso, qso_num_t qso_number) {
	qso_number_ = qso_number;
	qso_ = qso;
	if (qso) {
		cty_data_->update_qso(qso);
		qso->update_band();
	}
	if (qsos_) {
		qsos_->push_back(qso);
		qsos_->map_record(qso_number);
		score_qso(qso, false);
	}
	if (algorithm_->uses_serno()) next_serial_++;
}

// Check record 
void contest_scorer::check_qso(record* qso, qso_num_t qso_number) {
	if (active_) {
		if (qso) {
			cty_data_->update_qso(qso);
			qso->update_band();
		}
		qso_ = qso;
		qso_number_ = qso_number;
		score_qso(qso, true);
	}
}

// Load QSOs - after restarting zzalog
void contest_scorer::resume_contest() {
	int ix = book_->size() - 1;
	std::chrono::system_clock::time_point start = contest_->date.start;
	std::chrono::system_clock::time_point finish = contest_->date.finish;
	record* qso = book_->get_record(ix, false);
	std::chrono::system_clock::time_point ts = qso->ctimestamp();
	qsos_ = new extract_data();
	while (ts > start) {
		// Add all QSOs it this contest (ID equivalent and timestamp within timeframe
		if (qso->item("CONTEST_ID") == contest_id_ && start < ts && finish > ts) {
			add_qso(qso, ix);
		}
		ix--;
		qso = book_->get_record(ix, false);
		ts = qso->ctimestamp();
	}
}

// Copy contest to timeframe
void contest_scorer::populate_timeframe() {
	if (contest_) {
		char temp[25];
		time_t t1 = std::chrono::system_clock::to_time_t(contest_->date.start);
		tm* t2 = gmtime(&t1);
		strftime(temp, sizeof(temp), "%Y%m%d %H%M", t2);
		start_time_ = temp;
		t1 = std::chrono::system_clock::to_time_t(contest_->date.finish);
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
void contest_scorer::populate_status() {
	if (contest_) {
		std::chrono::system_clock::time_point today = std::chrono::system_clock::now();
		if (today < contest_->date.start) contest_status_ = FUTURE;
		else if (today > contest_->date.finish) contest_status_ = PAST;
		else {
			if (active_) contest_status_ = ACTIVE;
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
		std::string text = info->id + ":" + info->index;
		// Adding the pointer to the contest definition as user data
		w_contest_->add(text.c_str(), 0, nullptr, (void*)(info));
	}
}

// Score QSP
void contest_scorer::score_qso(record* qso, bool check_only) {
	if (algorithm_ == nullptr || qso == nullptr) return;
	if (qso && qso->item("CALL").length()) {
		// Only score next QSO if we have one to score
		score_result res = algorithm_->score_qso(qso, multipliers_);
		d_multiplier_ = res.multiplier;
		d_qso_points_ = res.qso_points;
		multiplier_p_ = multiplier_ + d_multiplier_;
		qso_points_p_ = qso_points_ + d_qso_points_;
		total_p_ = multiplier_p_ * qso_points_p_;
		if (!check_only) {
			multiplier_ = multiplier_p_;
			qso_points_ = qso_points_p_;
			total_ = total_p_;
		}
	}
	else {
		d_multiplier_ = 0;
		d_qso_points_ = 0;
	}
	enable_widgets();
}

// In contest mode
bool contest_scorer::contest_active() {
	return contest_status_ == ACTIVE;
}

// Returns value of ADIF.CONTEST_ID
std::string contest_scorer::contest_id() {
	return contest_id_;
}

// Returns the fields::collection name
field_list contest_scorer::fields() {
	if (algorithm_) {
		return algorithm_->fields();
	}
	return {};
}

// Returns the serial number
std::string contest_scorer::serial() {
	char text[10];
	snprintf(text, sizeof(text), "%03d", next_serial_);
	return std::string(text);
}

// Returns the created exchange
std::string contest_scorer::generate_exchange(record* qso) {
	return algorithm_->generate_exchange(qso);
}

// Parse exchange
void contest_scorer::parse_exchange(record* qso, std::string text) {
	algorithm_->parse_exchange(qso, text);
}


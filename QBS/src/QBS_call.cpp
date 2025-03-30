#include "QBS_call.h"
#include "QBS_card_table.h"
#include "QBS_data.h"
#include "QBS_window.h"
#include "QBS_charth.h"

#include "utils.h"
#include "drawing.h"
#include "input_hierch.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>

extern const char* DATE_FORMAT;

QBS_call::QBS_call(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	call_ = "";
	add_qty_ = 0;
	stuff_qty_ = 0;
	keep_qty_ = 0;
	sases_qty_ = 0;
	win_ = ancestor_view<QBS_window>(this);
	data_ = win_->data_;
	date_ = now(false, DATE_FORMAT);

	create_form();
	enable_widgets();
}

QBS_call::~QBS_call() {}

void QBS_call::create_form() {
	align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	labelsize(FL_NORMAL_SIZE + 2);
	box(FL_BORDER_BOX);

	int cx = x() + GAP + WLABEL;
	int cy = y() + GAP + labelsize();
	int sy = cy;
	int maxx = cx;

	ip_call_ = new Fl_Input(cx, cy, WSMEDIT, HBUTTON, "Call:");
	ip_call_->align(FL_ALIGN_LEFT);
	ip_call_->callback(cb_call, &call_);
	ip_call_->when(FL_WHEN_CHANGED);
	ip_call_->tooltip("Enter the call you wish to process cards for");
	ip_call_->value(call_.c_str());

	cy += HBUTTON + GAP;
	maxx = max(maxx, cx + ip_call_->w());

	tb_holding_ = new QBS_card_table(cx, cy, WSMEDIT, 7 * HBUTTON, "Holding:");
	tb_holding_->align(FL_ALIGN_LEFT | FL_ALIGN_CENTER);
	tb_holding_->tooltip("Displays the current state of the boxes");

	cy += tb_holding_->h() + GAP;
	maxx = max(maxx, cx + tb_holding_->w());

	ch_history_ = new QBS_charth(cx, cy, 160, 100, "History:");
	ch_history_->align(FL_ALIGN_LEFT | FL_ALIGN_CENTER);
	ch_history_->tooltip("Displays the recent history of cards for this callsign");
	ch_history_->data(data_);

	maxx = max(maxx, cx + ch_history_->w());
	int maxy = cy + ch_history_->h();

	int gw = 0;
	int gh = 0;
	cx = maxx + GAP;
	cy = sy;

	gp_add_item_ = new Fl_Group(cx, cy, 100, 100);

	cx += WLABEL;
	ip_add_qty_ = new Fl_Int_Input(cx, cy, WBUTTON, HBUTTON, "Add");
	ip_add_qty_->align(FL_ALIGN_LEFT);
	ip_add_qty_->callback(cb_value_int<Fl_Int_Input>, &add_qty_);
	ip_add_qty_->value(to_string(add_qty_).c_str());
	ip_add_qty_->tooltip("Enter the number of cards or SASEs received");

	cx += WBUTTON;
	cy += HBUTTON;

	gw = cx - gp_add_item_->x();
	gh = cy - gp_add_item_->y();

	gp_add_item_->resizable(nullptr);
	gp_add_item_->size(gw, gh);
	gp_add_item_->end();

	cx = gp_add_item_->x();
	cy = gp_add_item_->y();

	gp_process_ = new Fl_Group(cx, cy, 100, 100);

	cx += WLABEL;
	ip_stuff_ = new Fl_Int_Input(cx, cy, WBUTTON, HBUTTON, "Stuff");
	ip_stuff_->align(FL_ALIGN_LEFT);
	ip_stuff_->callback(cb_value_int<Fl_Int_Input>, &stuff_qty_);
	ip_stuff_->value(to_string(stuff_qty_).c_str());
	ip_stuff_->tooltip("Enter the number of cards being put in SASEs for posting");

	maxx = cx + WBUTTON;
	cy += HBUTTON + GAP;

	ip_keep_ = new Fl_Int_Input(cx, cy, WBUTTON, HBUTTON, "Keep");
	ip_keep_->align(FL_ALIGN_LEFT);
	ip_keep_->callback(cb_value_int<Fl_Int_Input>, &keep_qty_);
	ip_keep_->value(to_string(keep_qty_).c_str());
	ip_keep_->tooltip("Enter the number of cards being put in SASEs for next time");

	maxx = max(maxx, cx + WBUTTON);
	cy += HBUTTON + GAP;

	ip_sases_ = new Fl_Int_Input(cx, cy, WBUTTON, HBUTTON, "SASEs");
	ip_sases_->align(FL_ALIGN_LEFT);
	ip_sases_->callback(cb_value_int<Fl_Int_Input>, &sases_qty_);
	ip_sases_->value(to_string(sases_qty_).c_str());
	ip_sases_->tooltip("Enter the number of SASEs for posting");

	maxx = max(maxx, cx + WBUTTON);
	cy += HBUTTON;

	gw = maxx - gp_process_->x();
	gh = cy - gp_process_->y();

	gp_process_->resizable(nullptr);
	gp_process_->size(gw, gh);
	gp_process_->end();

	cx = x() + w() - GAP - (3 * WBUTTON);
	cy = y() + h() - GAP - HBUTTON;

	bn_back_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Back");
	bn_back_->callback(cb_back, nullptr);
	bn_back_->tooltip("Go back to previous action");

	cx += WBUTTON;

	bn_execute_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Execute");
	bn_execute_->callback(cb_execute, nullptr);
	bn_execute_->tooltip("Execute the current action");

	cx += WBUTTON;

	bn_done_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Next");
	bn_done_->callback(cb_done, nullptr);
	bn_done_->tooltip("Mark action done, and proceed to next action");

	end();
	show();
}

void QBS_call::enable_widgets() {
	// Set the label
	int last_box = data_->get_current();
	string last_batch = data_->get_batch(last_box);
	int head_box = data_->get_head();
	string head_batch = data_->get_batch(head_box);
	char l[128];
	switch (win_->process()) {
	case process_mode_t::LOG_CARD:
		snprintf(l, sizeof(l), "LOG CARD: Adding out-of-batch card - last batch %s", last_batch.c_str());
		break;
	case process_mode_t::LOG_SASE:
		snprintf(l, sizeof(l), "LOG SASE: Adding SASE - last batch %s", last_batch.c_str());
		break;
	case process_mode_t::SORTING:
		snprintf(l, sizeof(l), "SORTING: Adding batch %s cards", last_batch.c_str());
		break;
	case process_mode_t::PROCESSING:
		snprintf(l, sizeof(l), "PROCESSING: Sorting batch %s cards for posting, or holding", last_batch.c_str());
		break;
	case process_mode_t::CALL_HISTORY:
		snprintf(l, sizeof(l), "CALL HISTORY: Recent card traffic");
		break;
	default:
		snprintf(l, sizeof(l), "INVALID: Not a valid mode for this dialog");
		break;
	}
	copy_label(l);
	// Show one or the other input group
	switch (win_->process()) {
	case process_mode_t::LOG_CARD:
	case process_mode_t::LOG_SASE:
	case process_mode_t::SORTING:
		ip_add_qty_->value(to_string(add_qty_).c_str());
		gp_add_item_->show();
		gp_process_->hide();
		break;
	case process_mode_t::PROCESSING:
		gp_add_item_->hide();
		stuff_qty_ = 0;
		for (int ix = data_->get_head(); ix <= data_->get_current(); ix++) {
			stuff_qty_ += data_->get_count(ix, call_);
		}
		stuff_qty_ += data_->get_count(KEEP_BOX, call_);
		if (stuff_qty_ > 0) sases_qty_ = 1;
		ip_stuff_->value(to_string(stuff_qty_).c_str());
		ip_keep_->value(to_string(keep_qty_).c_str());
		ip_sases_->value(to_string(sases_qty_).c_str());
		gp_process_->show();
		break;
	case process_mode_t::CALL_HISTORY:
		gp_add_item_->hide();
		gp_process_->hide();
		break;
	default:
		gp_add_item_->hide();
		gp_process_->hide();
		break;
	}
	// Update the holdoing table
	tb_holding_->call(call_);
	tb_holding_->redraw();
	// Update the history chart
	ch_history_->update(call_);
	ch_history_->show();
}

// Go back to previous process
void QBS_call::cb_back(Fl_Widget* w, void* v) {
	QBS_call* that = ancestor_view<QBS_call>(w);
	that->win_->pop_process();
}

// Execute the action
void QBS_call::cb_execute(Fl_Widget* w, void* v) {
	QBS_call* that = ancestor_view<QBS_call>(w);
	switch (that->win_->process()) {
	case process_mode_t::LOG_CARD:
		that->execute_log_card();
		break;
	case process_mode_t::LOG_SASE:
		that->execute_log_sase();
		break;
	case process_mode_t::SORTING:
		that->execute_sort();
		break;
	case process_mode_t::PROCESSING:
		that->execute_process();
		break;
	}
}

void QBS_call::cb_done(Fl_Widget* w, void* v) {
	QBS_call* that = ancestor_view<QBS_call>(w);
	switch (that->win_->process()) {
	case process_mode_t::LOG_CARD:
		that->win_->pop_process();
		break;
	case process_mode_t::LOG_SASE:
		that->win_->pop_process();
		break;
	case process_mode_t::SORTING:
		that->win_->process(process_mode_t::PROCESSING);
		break;
	case process_mode_t::PROCESSING:
		that->win_->process(process_mode_t::POSTING);
		break;
	}
}

void QBS_call::cb_call(Fl_Widget* w, void* v) {
	cb_value<Fl_Input, string>(w, v);
	string* s = (string*)v;
	(*s) = to_upper(*s);
	((Fl_Input*)w)->value(s->c_str());
	QBS_call* that = ancestor_view<QBS_call>(w);
	that->enable_widgets();
}

// Process specific executes
void QBS_call::execute_log_card() {
	char log_msg[128];
	data_->receive_cards(IN_BOX, date_, call_, add_qty_);
	snprintf(log_msg, sizeof(log_msg), "Received non-batch cards %s: %s %d\n", date_.c_str(), call_.c_str(), add_qty_);
	win_->append_batch_log(log_msg);
	add_qty_ = 0;
	enable_widgets();
}

void QBS_call::execute_log_sase() {
	char log_msg[128];
	data_->receive_sases(date_, call_, add_qty_);
	snprintf(log_msg, sizeof(log_msg), "Received SASEs %s: %s %d\n", date_.c_str(), call_.c_str(), add_qty_);
	win_->append_batch_log(log_msg);
	add_qty_ = 0;
	enable_widgets();
}

void QBS_call::execute_sort() {
	char log_msg[128];
	data_->receive_cards(data_->get_current(), date_, call_, add_qty_);
	snprintf(log_msg, sizeof(log_msg), "Received batch cards %s: %s %d\n", date_.c_str(), call_.c_str(), add_qty_);
	win_->append_batch_log(log_msg);
	add_qty_ = 0;
	enable_widgets();
}

// Stuff from specified box
void QBS_call::stuff_cards(int box, int& cards) {
	int from = (int)data_->get_count(box, call_);
	if (from > cards) {
		data_->stuff_cards(box, date_, call_, cards);
		cards = 0;
	}
	else if (from != 0) {
		data_->stuff_cards(box, date_, call_, from);
		cards -= from;
	}
}

// Stuff/keep
void QBS_call::execute_process() {
	char log_msg[128];
	// Use cards in order KEEP, HEAD to TAIL, IN, CURR
	if (stuff_qty_ > 0 && keep_qty_ > 0) {
		fl_message("Do not select Stuff (%d) and Keep(%d) ate the same time - choose one or the other!",
			stuff_qty_, keep_qty_);
		return;
	}
	if (stuff_qty_ == 0 && keep_qty_ == 0) {
		fl_message("You need a value in either Stuff or Keep!");
		return;
	}
	if (stuff_qty_ > 0 && sases_qty_ == 0) {
		fl_message("You need to spcify number of SASEs being used for %d cards!");
		return;
	}
	if (stuff_qty_) {
		int cards = stuff_qty_;
		stuff_cards(KEEP_BOX, cards);
		for (int box = data_->get_head(); box <= data_->get_tail() && cards > 0; box++) {
			stuff_cards(box, cards);
		}
		if (cards > 0) {
			stuff_cards(IN_BOX, cards);
		}
		if (cards > 0) {
			stuff_cards(data_->get_current(), cards);
		}
		data_->use_sases(date_, call_, sases_qty_);
		snprintf(log_msg, sizeof(log_msg), "Stuffed cards %s: %s %d cards in %d SASEs\n",
			date_.c_str(), call_.c_str(), stuff_qty_ - cards, sases_qty_);
		win_->append_batch_log(log_msg);
	}
	if (keep_qty_) {
		// Now process KEEP
		data_->keep_cards(data_->get_current(), date_, call_, keep_qty_);
		snprintf(log_msg, sizeof(log_msg), "Kept cards %s: %s %d cards\n", date_.c_str(), call_.c_str(), keep_qty_);
		win_->append_batch_log(log_msg);
	}
	stuff_qty_ = 0;
	keep_qty_ = 0;
	sases_qty_ = 0;
	enable_widgets();
}


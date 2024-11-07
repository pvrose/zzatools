#include "QBS_batch.h"
#include "QBS_data.h"
#include "QBS_window.h"

#include "utils.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Output.H>

extern const char* DATE_FORMAT;

QBS_batch::QBS_batch(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	win_ = ancestor_view<QBS_window>(this);
	data_ = win_->data_;
	date_ = now(true, DATE_FORMAT);

	create_form();
	enable_widgets();

}

QBS_batch::~QBS_batch()
{

}

void QBS_batch::create_form() {
	align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	labelsize(FL_NORMAL_SIZE + 2);

	int cx = x() + GAP;
	int cy = y() + GAP + labelsize();
	int sy = cy;

	// Batch information
	Fl_Box* b1 = new Fl_Box(cx, cy, WLABEL, HBUTTON, "Status");
	b1->box(FL_FLAT_BOX);

	cy += HBUTTON;
	Fl_Box* b2 = new Fl_Box(cx, cy, WLABEL, HBUTTON, "Received");
	b2->box(FL_FLAT_BOX);

	cy += HBUTTON;
	Fl_Box* b3= new Fl_Box(cx, cy, WLABEL, HBUTTON, "Sent");
	b3->box(FL_FLAT_BOX);

	cy += HBUTTON;
	Fl_Box* b4 = new Fl_Box(cx, cy, WLABEL, HBUTTON, "Recycled");
	b4->box(FL_FLAT_BOX);

	int my = cy + HBUTTON;

	cy = sy;
	cx += WLABEL;
	int sx = cx;

	Fl_Box* b5 = new Fl_Box(cx, cy, WBUTTON, HBUTTON, "Calls");
	b5->box(FL_FLAT_BOX);

	cy += HBUTTON;
	op_rcvd_calls_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_rcvd_calls_->tooltip("The number of calls in the cards received");

	cy += HBUTTON;
	op_sent_calls_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_sent_calls_->tooltip("The number of calls in the cards sent or in Out-box");

	cy += HBUTTON;
	op_rcyc_calls_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_rcyc_calls_->tooltip("The number of calls in the cards recycled");

	cy = sy;
	cx += WBUTTON;
	Fl_Box* b6 = new Fl_Box(cx, cy, WBUTTON, HBUTTON, "Cards");
	b5->box(FL_FLAT_BOX);

	cy += HBUTTON;
	op_rcvd_cards_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_rcvd_cards_->tooltip("The number of cards received");

	cy += HBUTTON;
	op_sent_cards_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_sent_cards_->tooltip("The number of cards sent or in Out-box");

	cy += HBUTTON;
	op_rcyc_cards_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_rcyc_cards_->tooltip("The number of cards recycled");

	cy += HBUTTON + GAP;
	cx = sx;
	ip_weight_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Weight");
	ip_weight_->align(FL_ALIGN_LEFT);
	ip_weight_->tooltip("Enter the weight of cards (in kg) being recycled");

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

	bn_done_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Done");
	bn_done_->callback(cb_done, nullptr);
	bn_done_->tooltip("Mark action done, and proceed to next action");

	end();
	show();
}

void QBS_batch::enable_widgets() {
	// Set the label
	int last_box = data_->get_current();
	if (last_box >= 0) {
		string last_batch = data_->get_batch(last_box);
		string next_batch = data_->get_batch(last_box + 1);
		int head_box = data_->get_head();
		string head_batch = data_->get_batch(head_box);
		char l[128];
		switch (win_->process()) {
		case process_mode_t::POSTING:
			snprintf(l, sizeof(l), "POSTING: Batch %s", last_batch.c_str());
			break;
		case process_mode_t::FINISHING:
			snprintf(l, sizeof(l), "FINISHING: Mark batch %s for disposal", last_batch.c_str());
			break;
		case process_mode_t::RECYCLING:
			snprintf(l, sizeof(l), "RECYCLING: Rrecycling batch %s cards", head_batch.c_str());
			break;
		case process_mode_t::LOG_BATCH:
			snprintf(l, sizeof(l), "NEW BATCH: Create new batch %s", next_batch.c_str());
			break;
		default:
			snprintf(l, sizeof(l), "INVALID: Not a valid mode for this dialog");
			break;
		}
		copy_label(l);
		// Weight input
		switch (win_->process()) {
		case process_mode_t::RECYCLING:
			ip_weight_->activate();
			break;
		default:
			ip_weight_->deactivate();
			break;
		}
		// Batch info
		recycle_data info;
		switch (win_->process()) {
		case process_mode_t::POSTING:
		case process_mode_t::FINISHING:
			info = data_->get_recycle_data(last_box);
			break;
		case process_mode_t::RECYCLING:
			info = data_->get_recycle_data(head_box);
			break;
		case process_mode_t::LOG_BATCH:
			break;
		}
		op_rcvd_calls_->value(info.count_received);
		op_sent_calls_->value(info.count_sent);
		op_rcyc_calls_->value(info.count_recycled);
		op_rcvd_cards_->value(info.sum_received);
		op_sent_cards_->value(info.sum_sent);
		op_rcyc_cards_->value(info.sum_recycled);
	}
}

// Go back to previous process
void QBS_batch::cb_back(Fl_Widget* w, void* v) {
	QBS_batch* that = ancestor_view<QBS_batch>(w);
	that->win_->pop_process();
}

// Execute the action
void QBS_batch::cb_execute(Fl_Widget* w, void* v) {
	QBS_batch* that = ancestor_view<QBS_batch>(w);
	switch (that->win_->process()) {
	case process_mode_t::POSTING:
		that->execute_post();
		break;
	case process_mode_t::FINISHING:
		that->execute_finish();
		break;
	case process_mode_t::RECYCLING:
		that->execute_recycle();
		break;
	case process_mode_t::LOG_BATCH:
		that->execute_new();
		break;
	}
}

void QBS_batch::cb_done(Fl_Widget* w, void* v) {
	QBS_batch* that = ancestor_view<QBS_batch>(w);
	switch (that->win_->process()) {
	case process_mode_t::POSTING:
		that->win_->process(process_mode_t::FINISHING);
		break;
	case process_mode_t::FINISHING:
	case process_mode_t::RECYCLING:
		that->win_->process(process_mode_t::DORMANT);
		break;
	case process_mode_t::LOG_BATCH:
		that->win_->process(process_mode_t::SORTING);
		break;
	}
}

void QBS_batch::execute_new() {
	int current = data_->get_current();
	string batch = data_->get_batch(current + 1);
	string old_batch = data_->get_batch(current);
	char log_msg[128];
	data_->new_batch(current + 1, date_, batch);
	snprintf(log_msg, sizeof(log_msg), "Closing batch %s: %s\n", old_batch.c_str(), date_.c_str());
	win_->append_batch_log(log_msg);
	win_->open_batch_log(batch);
	snprintf(log_msg, sizeof(log_msg), "New batch %s: %s\n", batch.c_str(), date_.c_str());
	win_->append_batch_log(log_msg);
	enable_widgets();
}

void QBS_batch::execute_recycle() {
	if (weight_ == 0.0) {
		fl_message("Enter a vaule for the weight of cards bbeing recycled!");
		return;
	}
	char log_msg[128];
	data_->recycle_cards(date_, weight_);
	snprintf(log_msg, sizeof(log_msg), "Recycling batch: %g kg %s\n", weight_, date_.c_str());
	win_->append_batch_log(log_msg);
	enable_widgets();
}

void QBS_batch::execute_post() {
	char log_msg[128];
	data_->post_cards(date_);
	snprintf(log_msg, sizeof(log_msg), "Posting batch: %s\n", date_.c_str());
	win_->append_batch_log(log_msg);
	enable_widgets();
}

void QBS_batch::execute_finish() {
	char log_msg[128];
	data_->dispose_cards(date_);
	snprintf(log_msg, sizeof(log_msg), "Marking batch for disposal: %s\n", date_.c_str());
	win_->append_batch_log(log_msg);
}


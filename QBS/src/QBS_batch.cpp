#include "QBS_batch.h"
#include "QBS_breport.h"
#include "QBS_data.h"
#include "QBS_top20.h"
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
	executed_ = false;

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

	ch_batch_ = new Fl_Input_Choice(cx + WLABEL, cy, WSMEDIT, HBUTTON, "Batch");
	ch_batch_->align(FL_ALIGN_LEFT);
	ch_batch_->callback(cb_batch, &box_);
	ch_batch_->tooltip("Select the batch to report on - disabled if not report");
	populate_batch_choice();

	cy += HBUTTON + GAP;

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

	cy += HBUTTON;
	Fl_Box* b41 = new Fl_Box(cx, cy, WLABEL, HBUTTON, "Held");
	b41->box(FL_FLAT_BOX);

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

	cy += HBUTTON;
	op_held_calls_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_held_calls_->tooltip("The number of calls in the cards being held");

	my = max(my, cy + HBUTTON);

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

	cy += HBUTTON;
	op_held_cards_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_held_cards_->tooltip("The number of cards being held");

	my = max(my, cy + HBUTTON);

	cy = sy;
	cx += WBUTTON;
	Fl_Box* b61 = new Fl_Box(cx, cy, WBUTTON * 3 / 2, HBUTTON, "Date");
	b5->box(FL_FLAT_BOX);

	cy += HBUTTON;
	op_rcvd_date_ = new Fl_Output(cx, cy, WBUTTON * 3 / 2, HBUTTON);
	op_rcvd_date_->tooltip("The date cards received");

	cy += HBUTTON;
	op_sent_date_ = new Fl_Output(cx, cy, WBUTTON * 3 / 2, HBUTTON);
	op_sent_date_->tooltip("The date the catds sent");

	cy += HBUTTON;
	op_rcyc_date_ = new Fl_Output(cx, cy, WBUTTON * 3 / 2, HBUTTON);
	op_rcyc_date_->tooltip("The date the cards recycled");

	cy = my + GAP;
	cx = sx;
	ip_weight_ = new Fl_Float_Input(cx, cy, WBUTTON, HBUTTON, "Weight (kg)");
	ip_weight_->align(FL_ALIGN_LEFT);
	ip_weight_->callback(cb_value_float<Fl_Float_Input>, &weight_);
	ip_weight_->tooltip("Enter the weight of cards (in kg) being recycled");

	cx += WBUTTON;
	op_wt_card_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON, "g/card");
	op_wt_card_->align(FL_ALIGN_RIGHT);
	op_wt_card_->tooltip("Shows the average weight per card");

	cx = sx;
	cy += HBUTTON + GAP;

	int htab = y() + h() - cy - HBUTTON - GAP;
	int wtab = x() + w() - cx - GAP;
	tab_top20_ = new QBS_top20(cx, cy, wtab, htab, "TOP 20 Recycled");
	tab_top20_->align(FL_ALIGN_TOP);
	tab_top20_->tooltip("Displays the top 20 culprits for recycling");
	tab_top20_->data(data_);

	tab_report_ = new QBS_breport(cx, cy, wtab, htab, "Callsigns in batch");
	tab_report_->align(FL_ALIGN_TOP);
	tab_report_->tooltip("Displays the callsigns received in this batch");
	tab_report_->data(data_);

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

	bn_next_ = new Fl_Button(cx, cy, WBUTTON, HBUTTON, "Next");
	bn_next_->callback(cb_next, nullptr);
	bn_next_->tooltip("Proceed to next action");

	end();
	show();
}

void QBS_batch::enable_widgets() {
	// Set the label
	if (box_ >= 0) {
		string batch = data_->get_batch(box_);
		char l[128];
		switch (win_->process()) {
		case process_mode_t::POSTING:
			snprintf(l, sizeof(l), "POSTING: Batch %s", batch.c_str());
			break;
		case process_mode_t::FINISHING:
			snprintf(l, sizeof(l), "FINISHING: Mark batch %s for disposal", batch.c_str());
			break;
		case process_mode_t::RECYCLING:
			snprintf(l, sizeof(l), "RECYCLING: Recycling batch %s cards", batch.c_str());
			break;
		case process_mode_t::LOG_BATCH:
			snprintf(l, sizeof(l), "NEW BATCH: Create new batch %s", batch.c_str());
			break;
		case process_mode_t::BATCH_SUMMARY:
			snprintf(l, sizeof(l), "BATCH SUMMARY: Batch #%d - %s", box_, batch.c_str());
			break;
		case process_mode_t::BATCH_REPORT:
			snprintf(l, sizeof(l), "BATCH REPORT: Batch #%d - %s", box_, batch.c_str());
			break;
		default:
			snprintf(l, sizeof(l), "INVALID: Not a valid mode for this dialog");
			break;
		}
		copy_label(l);
		// Weight input
		switch (win_->process()) {
		case process_mode_t::RECYCLING:
		case process_mode_t::BATCH_SUMMARY:
		case process_mode_t::BATCH_REPORT:
			ip_weight_->activate();
			break;
		default:
			ip_weight_->deactivate();
			break;
		}
		// Batch info
		recycle_data info;
		box_data* box_data = nullptr;
		if (box_ >= 0) {
			info = data_->get_recycle_data(box_);
			box_data = data_->get_box(box_);
		}

		op_rcvd_calls_->value(info.count_received);
		op_sent_calls_->value(info.count_sent);
		op_rcyc_calls_->value(info.count_recycled);
		op_held_calls_->value(info.count_received - info.count_sent - info.count_recycled);
		op_rcvd_cards_->value(info.sum_received);
		op_sent_cards_->value(info.sum_sent);
		op_rcyc_cards_->value(info.sum_recycled);
		op_held_cards_->value(info.sum_received - info.sum_sent - info.sum_recycled);
		if (box_data) {
			op_rcvd_date_->value(box_data->date_received.c_str());
			op_sent_date_->value(box_data->date_sent.c_str());
			op_rcyc_date_->value(box_data->date_recycled.c_str());
		}
		else {
			op_rcvd_date_->value("");
			op_sent_date_->value("");
			op_rcyc_date_->value("");
		}
		snprintf(l, sizeof(l), "%.2f", info.weight_kg);
		char l2[32];
		switch (win_->process()) {
		case RECYCLING:
			tab_top20_->show();
			tab_top20_->box(box_);
			tab_report_->hide();
			ch_batch_->deactivate();	
			ip_weight_->value(weight_);
			snprintf(l2, sizeof(l2), "%.1f", weight_ * 1000. / (info.sum_received - info.sum_sent));
			op_wt_card_->value(l2);
			break;
		case BATCH_SUMMARY:
			tab_top20_->show();
			tab_top20_->box(box_);
			tab_report_->hide();
			ch_batch_->activate();
			ch_batch_->value(box_);
			ip_weight_->value(l);
			if (info.sum_recycled > 0) {
				snprintf(l2, sizeof(l2), "%.1f", info.weight_kg * 1000 / info.sum_recycled);
				op_wt_card_->value(l2);
			} else {
				op_wt_card_->value("");
			}
			break;
		case BATCH_REPORT:
			tab_top20_->hide();
			tab_report_->show();
			tab_report_->box(box_);
			ch_batch_->activate();
			ch_batch_->value(box_);
			ip_weight_->value(l);
			if (info.sum_recycled > 0) {
				snprintf(l2, sizeof(l2), "%.1f", info.weight_kg * 1000 / info.sum_recycled);
				op_wt_card_->value(l2);
			} else {
				op_wt_card_->value("");
			}
			break;
		default:
			tab_top20_->hide();
			tab_report_->hide();
			ch_batch_->deactivate();
			break;
		}
		if (executed_) {
			bn_execute_->deactivate();
		}
		else {
			bn_execute_->activate();
		}
	}
}

// Initialise the widget
void QBS_batch::initialise() {
	executed_ = false;
	switch (win_->process()) {
	case process_mode_t:: POSTING:
	case process_mode_t::FINISHING:
		box_ = data_->get_current();
		break;
	case process_mode_t::RECYCLING:
		box_ = data_->get_head();
		break;
	case process_mode_t::LOG_BATCH:
		box_ = data_->get_current() + 1;
		break;
	case process_mode_t::BATCH_SUMMARY:
		box_ = 0;
		break;
	}
}

// Go back to previous process
void QBS_batch::cb_back(Fl_Widget* w, void* v) {
	QBS_batch* that = ancestor_view<QBS_batch>(w);
	that->box_ = 0;
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
	case process_mode_t::BATCH_SUMMARY:
		that->enable_widgets();
	}
}

void QBS_batch::cb_next(Fl_Widget* w, void* v) {
	QBS_batch* that = ancestor_view<QBS_batch>(w);
	switch (that->win_->process()) {
	case process_mode_t::POSTING:
		that->win_->process(process_mode_t::FINISHING);
		break;
	case process_mode_t::FINISHING:
	case process_mode_t::RECYCLING:
	case process_mode_t::BATCH_SUMMARY:
	case process_mode_t::BATCH_REPORT:
		that->win_->process(process_mode_t::DORMANT);
		break;
	case process_mode_t::LOG_BATCH:
		that->win_->process(process_mode_t::SORTING);
		break;
	}
}

void QBS_batch::cb_batch(Fl_Widget* w, void* v) {
	QBS_batch* that = ancestor_view<QBS_batch>(w);
	*(int*)v = ((Fl_Input_Choice*)w)->menubutton()->value();
	that->enable_widgets();
}

void QBS_batch::execute_new() {
	if (data_->get_current() != data_->get_tail()) {
		fl_message("Previous batch has not been marked for disposal!");
		return;
	}
	string batch = data_->get_batch(box_);
	string old_batch = data_->get_batch(box_ - 1);
	char log_msg[128];
	data_->new_batch(box_, date_, batch);
	snprintf(log_msg, sizeof(log_msg), "Closing batch %s: %s\n", old_batch.c_str(), date_.c_str());
	win_->append_batch_log(log_msg);
	win_->open_batch_log(batch);
	snprintf(log_msg, sizeof(log_msg), "New batch %s: %s\n", batch.c_str(), date_.c_str());
	win_->append_batch_log(log_msg);
	executed_ = true;
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
	executed_ = true;
	enable_widgets();
}

void QBS_batch::execute_post() {
	char log_msg[128];
	data_->post_cards(date_);
	snprintf(log_msg, sizeof(log_msg), "Posting batch: %s\n", date_.c_str());
	win_->append_batch_log(log_msg);
	executed_ = true;
	enable_widgets();
}

void QBS_batch::execute_finish() {
	char log_msg[128];
	data_->dispose_cards(date_);
	snprintf(log_msg, sizeof(log_msg), "Marking batch for disposal: %s\n", date_.c_str());
	win_->append_batch_log(log_msg);
	executed_ = true;
	enable_widgets();
}

void QBS_batch::populate_batch_choice() {
	ch_batch_->clear();
	int last_box = data_->get_current();
	for (int b = 0; b <= last_box; b++) {
		ch_batch_->add(data_->get_batch(b).c_str());
	}
}

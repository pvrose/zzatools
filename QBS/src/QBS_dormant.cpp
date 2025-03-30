#include "QBS_dormant.h"
#include "QBS_window.h"
#include "QBS_data.h"

#include "utils.h"
#include "drawing.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Output.H>

extern const char* DATE_FORMAT;

QBS_dormant::QBS_dormant(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	win_ = ancestor_view<QBS_window>(this);
	data_ = win_->data_;
	callsign_ = "";
	num_received_ = 0;
	date_ = now(true, DATE_FORMAT);
	create_form();
}

QBS_dormant::~QBS_dormant() 
{}

void QBS_dormant::create_form() {
	align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	labelsize(FL_NORMAL_SIZE + 2);

	int cx = x() + GAP;
	int cy = y() + GAP + labelsize();

	bn_receive_card_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "Rcv ad-hoc card");
	bn_receive_card_->callback(cb_action, (void*)(intptr_t)process_mode_t::LOG_CARD);
	bn_receive_card_->tooltip("Select to add a card received outwith a batch");

	cy += HBUTTON;

	bn_receive_sase_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "Receive SASEs");
	bn_receive_sase_->callback(cb_action, (void*)(intptr_t)process_mode_t::LOG_SASE);
	bn_receive_sase_->tooltip("Select to add received SASEs");

	cy += HBUTTON;

	bn_new_batch_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "New Batch");
	bn_new_batch_->callback(cb_action, (void*)(intptr_t)process_mode_t::LOG_BATCH);
	bn_new_batch_->tooltip("Select to record the receipt of a new batch of cards - goes to SORTING");

	cx += 2 * WBUTTON + GAP;
	op_new_batch_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_new_batch_->color(FL_BACKGROUND_COLOR);
	op_new_batch_->box(FL_FLAT_BOX);

	cx = x() + GAP;
	cy += HBUTTON;

	bn_recycle_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "Recycle");
	bn_recycle_->callback(cb_action, (void*)(intptr_t)process_mode_t::RECYCLING);
	bn_recycle_->tooltip("Select to record the recycling of the oldest batch of cards - goes to RECYCLING");

	cx += 2 * WBUTTON + GAP;
	op_recycle_ = new Fl_Output(cx, cy, WBUTTON, HBUTTON);
	op_recycle_->color(FL_BACKGROUND_COLOR);
	op_recycle_->box(FL_FLAT_BOX);

	cx = x() + GAP;
	cy += HBUTTON;

	bn_batch_summary_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "Batch summary");
	bn_batch_summary_->callback(cb_action, (void*)(intptr_t)process_mode_t::BATCH_SUMMARY);
	bn_batch_summary_->tooltip("Select to display the batch summary");

	cy += HBUTTON;

	bn_batch_report_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "Batch report");
	bn_batch_report_->callback(cb_action, (void*)(intptr_t)process_mode_t::BATCH_REPORT);
	bn_batch_report_->tooltip("Select to display the full batch report");
	
	cy += HBUTTON;

	bn_call_history_ = new Fl_Button(cx, cy, 2 *WBUTTON, HBUTTON, "Call History");
	bn_call_history_->callback(cb_action, (void*)(intptr_t)process_mode_t::CALL_HISTORY);
	bn_call_history_->tooltip("Display recent activity for call");

	cy += HBUTTON + GAP;

	bn_undo_ = new Fl_Button(cx, cy, 2 * WBUTTON, HBUTTON, "Undo previous");
	bn_undo_->callback(cb_undo, nullptr);
	bn_undo_->tooltip("Go back to the last process");

	end();
	show();
}

void QBS_dormant::enable_widgets() {
	// Set the label with the current batch name
	int last_box = data_->get_current();
	if (last_box >= 0) {
		current_ = data_->get_batch(last_box);
		next_ = data_->get_batch(last_box + 1);
		head_ = data_->get_batch(data_->get_head());
		char l[128];
		snprintf(l, sizeof(l), "DORMANT: Most recent batch %s", current_.c_str());
		copy_label(l);
		// Set the note against new box with the names of that new box
		op_new_batch_->value(next_.c_str());
		// Set the note against recycle with the name of the batch being recycled
		op_recycle_->value(head_.c_str());
	}
}


// Callback - Receive SASEs
void QBS_dormant::cb_action(Fl_Widget* w, void* v) {
	QBS_dormant* that = ancestor_view<QBS_dormant>(w);
	that->win_->process((process_mode_t)(intptr_t)v);
}

// Callback undo
void QBS_dormant::cb_undo(Fl_Widget* w, void* v) {
	QBS_dormant* that = ancestor_view<QBS_dormant>(w);
	that->win_->restore_process();
}




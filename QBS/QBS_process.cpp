#include "QBS_consts.h"
#include "QBS_process.h"
#include "QBS_window.h"
#include "QBS_data.h"

#include "../zzalib/utils.h"
#include "../zzalib/callback.h"

#include <FL/Fl_Output.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>

using namespace zzalib;
using namespace std;

QBS_process::QBS_process(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	QBS_window* window = ancestor_view<QBS_window>(this);
	data_ = window->data_;

	load_values();
	create_form();
	update_actions();
	update_batches();
}

QBS_process::~QBS_process() {
	save_values();
}

void QBS_process::load_values() {
	// NONE KNOWN
}

void QBS_process::create_form() {
	align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	labelfont(FONT | FL_ITALIC);
	labelsize(FONT_SIZE * 3 / 2);

	begin();
	int curr_x = x() + GAP;
	int curr_y = y() + fl_height(FONT, FONT_SIZE) + fl_height(FONT, FONT_SIZE * 3 / 2);
	int max_x = curr_x;

	// Batch control group
	Fl_Group* g1 = new Fl_Group(curr_x, curr_y, 1000, 1000, "Batch control");
	g1->labelfont(FONT);
	g1->labelsize(FONT_SIZE);
	g1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	g1->box(FL_BORDER_BOX);
	int g1_x = g1->x() + GAP + WLLABEL;
	int g1_y = g1->y() + HTEXT;

	// Current batch input
	Fl_Output* w1 = new Fl_Output(g1_x, g1_y, WSMEDIT, HBUTTON, "Current Batch");
	w1->labelfont(FONT);
	w1->labelsize(FONT_SIZE);
	w1->align(FL_ALIGN_LEFT);
	w1->textfont(FONT);
	w1->textsize(FONT_SIZE);
	w_current_ = w1;

	g1_x += w1->w() + GAP;
	Fl_Button* w2 = new Fl_Button(g1_x, g1_y, WBUTTON, HBUTTON, "Add batch");
	w2->labelfont(FONT);
	w2->labelsize(FONT_SIZE);
	w2->callback(cb_bn_add, nullptr);

	// Disposal batch
	g1_x = g1->x() + GAP + WLLABEL;
	g1_y += w1->h();
	Fl_Output* w3 = new Fl_Output(g1_x, g1_y, WSMEDIT, HBUTTON, "Disposal Batch");
	w3->labelfont(FONT);
	w3->labelsize(FONT_SIZE);
	w3->align(FL_ALIGN_LEFT);
	w3->textfont(FONT);
	w3->textsize(FONT_SIZE);
	w_disposal_ = w3;

	g1_x += w3->w() + GAP;
	Fl_Button* w4 = new Fl_Button(g1_x, g1_y, WBUTTON, HBUTTON, "Dispose");
	w4->labelfont(FONT);
	w4->labelsize(FONT_SIZE);
	w4->callback(cb_bn_add, nullptr);

	g1_x += w4->w() + GAP;
	Fl_Float_Input* w4AA = new Fl_Float_Input(g1_x, g1_y, WBUTTON, HBUTTON, "Weight (kg)");
	w4AA->labelfont(FONT);
	w4AA->labelsize(FONT_SIZE);
	w4AA->align(FL_ALIGN_TOP);
	w4AA->textfont(FONT);
	w4AA->textsize(FONT_SIZE);
	w4AA->value("0.0");
	w4AA->callback(cb_value_float<Fl_Float_Input>, &weight_disposed_);

	g1_x += w4AA->w() + GAP;
	g1_y += w3->h() + GAP;

	g1->end();
	g1->resizable(nullptr);
	g1->size(g1_x - g1->x(), g1_y - g1->y());

	curr_x += g1->w() +GAP;
	max_x = max(max_x, curr_x);
	
	curr_x = x() + GAP;
	curr_y += g1->h() + GAP;

	// Per=amateur action group
	Fl_Group* g2 = new Fl_Group(curr_x, curr_y, 1000, 1000, "Per-amateur action");
	g2->labelfont(FONT);
	g2->labelsize(FONT_SIZE);
	g2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	g2->box(FL_BORDER_BOX);
	int g2_x = g2->x() + GAP + WLABEL;
	int g2_y = g2->y() + HTEXT;
	int max_g2x = g2_x;

	// CAllsign input
	Fl_Input* w4A = new Fl_Input(g2_x, g2_y, WSMEDIT, HBUTTON, "Callsign");
	w4A->labelfont(FONT);
	w4A->labelsize(FONT_SIZE);
	w4A->align(FL_ALIGN_LEFT);
	w4A->textfont(FONT);
	w4A->textsize(FONT_SIZE);
	w4A->callback(cb_ip_call, &callsign_);
	w4A->when(FL_WHEN_ENTER_KEY);

	g2_x = g2->x() + GAP;
	g2_y += w4A->h() + GAP;

	// Radio button - Add card
	Fl_Radio_Round_Button* w5 = new Fl_Radio_Round_Button(g2_x, g2_y, HBUTTON, HBUTTON, "Add cards");
	w5->labelfont(FONT);
	w5->labelsize(FONT_SIZE);
	w5->align(FL_ALIGN_RIGHT);
	w5->callback(cb_bn_action, (void*)(long)ADD_CARD);
	w_add_card_ = w5;

	g2_y += w5->h();
	// Radio button - Add SASEs
	Fl_Radio_Round_Button* w6 = new Fl_Radio_Round_Button(g2_x, g2_y, HBUTTON, HBUTTON, "Add SASEs");
	w6->labelfont(FONT);
	w6->labelsize(FONT_SIZE);
	w6->align(FL_ALIGN_RIGHT);
	w6->callback(cb_bn_action, (void*)(long)ADD_SASE);
	w_add_sase_ = w6;

	g2_y += w6->h();
	// Radio button - Add card
	Fl_Radio_Round_Button* w7 = new Fl_Radio_Round_Button(g2_x, g2_y, HBUTTON, HBUTTON, "Send cards");
	w7->labelfont(FONT);
	w7->labelsize(FONT_SIZE);
	w7->align(FL_ALIGN_RIGHT);
	w7->callback(cb_bn_action, (void*)(long)SEND_CARD);
	w_send_card_ = w7;

	g2_y += w7->h();
	Fl_Radio_Round_Button* w7A = new Fl_Radio_Round_Button(g2_x, g2_y, HBUTTON, HBUTTON, "Keep cards");
	w7A->labelfont(FONT);
	w7A->labelsize(FONT_SIZE);
	w7A->align(FL_ALIGN_RIGHT);
	w7A->callback(cb_bn_action, (void*)(long)SEND_CARD);
	w_send_card_ = w7A;

	g2_y += w7A->h();
	// Radio button - Add card
	Fl_Radio_Round_Button* w8 = new Fl_Radio_Round_Button(g2_x, g2_y, HBUTTON, HBUTTON, "Dispose SASEs");
	w8->labelfont(FONT);
	w8->labelsize(FONT_SIZE);
	w8->align(FL_ALIGN_RIGHT);
	w8->callback(cb_bn_action, (void*)(long)DISPOSE_SASE);
	w_disp_sase_ = w8;

	g2_y = w5->y();
	g2_x += w5->w() + WLABEL + GAP + WLLABEL;

	// Input #calls
	Fl_Int_Input* w9 = new Fl_Int_Input(g2_x, g2_y, WBUTTON, HBUTTON, "Num. cards");
	w9->labelfont(FONT);
	w9->labelsize(FONT_SIZE);
	w9->align(FL_ALIGN_LEFT);
	w9->callback(cb_value_int<Fl_Int_Input>, &num_cards_);
	w9->value("0");
	w_num_cards_ = w9;

	g2_y += w9->h();

	Fl_Int_Input* w10 = new Fl_Int_Input(g2_x, g2_y, WBUTTON, HBUTTON, "Num. SASEs");
	w10->labelfont(FONT);
	w10->labelsize(FONT_SIZE);
	w10->align(FL_ALIGN_LEFT);
	w10->callback(cb_value_int<Fl_Int_Input>, &num_sases_);
	w10->value("0");
	w_num_sases_ = w10;

	g2_y += w10->h() + GAP;
	Fl_Button* w11 = new Fl_Button(g2_x, g2_y, WBUTTON, HBUTTON, "Execute");
	w11->labelfont(FONT);
	w11->labelsize(FONT_SIZE);
	w11->callback(cb_bn_execute, nullptr);

	g2_x += w11->w() + GAP;
	g2_y = max(w11->y() + w11->h(), w8->y() + w8->h()) + GAP + HTEXT;
	max_g2x = max(max_g2x, g2_x);

	g2_x = g2->x() + GAP;

	string count_labels[QD_NUM_DIRECTION] =
	{ "BR/F", "Receive", "Send", "Dispose", "Keep" };
	Fl_Output* w11A = new Fl_Output(g2_x, g2_y, WLABEL, HBUTTON);
	w11A->textfont(FONT);
	w11A->textsize(FONT_SIZE);
	w11A->box(FL_FLAT_BOX);
	w11A->value("Cards");
	w11A->color(FL_BACKGROUND_COLOR);
	g2_x += w11A->w();

	for (int ix1 = 0; ix1 < QD_NUM_DIRECTION; ix1++) {
		Fl_Output* w12 = new Fl_Output(g2_x, g2_y, WBUTTON, HBUTTON);
		w12->copy_label(count_labels[ix1].c_str());
		w12->labelfont(FONT);
		w12->labelsize(FONT_SIZE);
		w12->align(FL_ALIGN_TOP);
		w12->textsize(FONT_SIZE);
		switch (ix1) {
		case QD_BRF:
		case QD_KEEP:
			w12->textfont(FONT | FL_ITALIC);
			w12->textcolor(FL_BLACK);
			break;
		case QD_DISPOSE:
			w12->textfont(FONT);
			w12->textcolor(FL_RED);
			break;
		default:
			w12->textfont(FONT);
			w12->textcolor(FL_BLACK);
			break;
		}
		w_count_[ix1][0] = w12;
		g2_x += w12->w();
	}

	g2_x = g2->x() + GAP;
	g2_y += HBUTTON;
	Fl_Output* w13 = new Fl_Output(g2_x, g2_y, WLABEL, HBUTTON);
	w13->textfont(FONT);
	w13->textsize(FONT_SIZE);
	w13->box(FL_FLAT_BOX);
	w13->value("SASEs");
	w13->color(FL_BACKGROUND_COLOR);
	g2_x += w13->w();

	for (int ix1 = 0; ix1 <= QD_NUM_DIRECTION; ix1++) {
		Fl_Output* w14 = new Fl_Output(g2_x, g2_y, WBUTTON, HBUTTON);
		w14->textsize(FONT_SIZE);
		switch (ix1) {
		case QD_BRF:
		case QD_KEEP:
			w14->textfont(FONT | FL_ITALIC);
			w14->textcolor(FL_BLACK);
			break;
		case QD_DISPOSE:
			w14->textfont(FONT);
			w14->textcolor(FL_RED);
			break;
		default:
			w14->textfont(FONT);
			w14->textcolor(FL_BLACK);
			break;
		}
		w_count_[ix1][1] = w14;
		g2_x += w14->w();
	}

	g2_y += w13->h() + GAP;
	g2_x += GAP;

	g2->end();

	g2->resizable(nullptr);
	max_g2x = max(max_g2x, g2_x);

	g2->size(max_g2x - g2->x(), g2_y - g2->y());

	curr_x += g2->w() + GAP;
	curr_y += g2->h() + GAP;
	max_x = max(max_x, curr_x);

	end();

	resizable(nullptr);
	size(max_x - x(), curr_y - y());
}

// SAve velaues - NONE KNOWN
void QBS_process::save_values() {
	// NONE KNOWN
}

// New batch
void QBS_process::cb_bn_add(Fl_Widget* w, void* v) {
	QBS_process* that = ancestor_view<QBS_process>(w);

	// Get next batch ID
	string next = that->data_->next_receipt();
	// Create next batch record
	QBS_data::batch_data_t* record = that->data_->batch_data(next, true);
	string today = now(true, "%Y-%m-%d");
	record->date_rcvd = today;
	record->date_disposed = "";
	record->weight_disposed = nanf("");
	// Update all widgets
	that->update_batches();
	that->update_counts();
}

// Add call
void QBS_process::cb_ip_call(Fl_Widget* w, void* v) {
	string callsign;
	cb_value<Fl_Input, string>(w, &callsign);
	*(string*)v = callsign;
	QBS_process* that = ancestor_view<QBS_process>(w);
	QBS_window* window = ancestor_view<QBS_window>(that);
	QBS_data* data = window->data_;

	// Get record for call - create if necessary
	if (!data->set_record(callsign, false)) {
		// TODO: ask to continue?
		printf("New call - %s\n", callsign.c_str());
		data->set_record(callsign, true);
	}

	that->update_counts();
}

// Action buttons
void QBS_process::cb_bn_action(Fl_Widget* w, void* v) {
	QBS_process* that = ancestor_view<QBS_process>(w);
	that->action_ = (action_t)(long)v;
	that->update_actions();
}

// Execute button
void QBS_process::cb_bn_execute(Fl_Widget* w, void* v) {
	QBS_process* that = ancestor_view<QBS_process>(w);
	that->execute_action();
}

// Dispose batch
void QBS_process::cb_bn_dispose(Fl_Widget* w, void* v) {
	QBS_process* that = ancestor_view<QBS_process>(w);

	// Get next disposal batch ID
	string next = that->data_->next_disposal();
	// Update record
	QBS_data::batch_data_t* record = that->data_->batch_data(next, false);
	string today = now(true, "%Y-%m-%d");
	record->date_disposed = today;
	record->weight_disposed = that->weight_disposed_;
	// Update all widgets
	that->update_batches();
	that->update_counts();

}

// Update batches
void QBS_process::update_batches() {
	string current_batch = data_->current_batch();
	string disposal_batch = data_->next_disposal();

	((Fl_Output*)w_current_)->value(current_batch.c_str());
	((Fl_Output*)w_disposal_)->value(disposal_batch.c_str());
}

// Update counts
void QBS_process::update_counts() {
	if (data_->set_record(callsign_, false)) {
		string batch = data_->current_batch();
		// Current counts
		for (int ix1 = 0; ix1 < QD_NUM_DIRECTION; ix1++) {
			for (int ix2 = 0; ix2 < QI_NUM_ITEM_TYPE; ix2++) {
				int value = data_->get_count(batch, (item_type_t)ix2, (direction_t)ix1);
				((Fl_Output*)w_count_[ix1][ix2])->value(to_string(value).c_str());
			}
		}
	}
}

// Update actions - set action buttons according to value of action_
void QBS_process::update_actions() {
	switch (action_) {
	case ADD_CARD:
		((Fl_Radio_Round_Button*)w_add_card_)->value(true);
		((Fl_Radio_Round_Button*)w_add_sase_)->value(false);
		((Fl_Radio_Round_Button*)w_send_card_)->value(false);
		((Fl_Radio_Round_Button*)w_keep_card_)->value(false);
		((Fl_Radio_Round_Button*)w_disp_sase_)->value(false);
		break;
	case ADD_SASE:
		((Fl_Radio_Round_Button*)w_add_card_)->value(false);
		((Fl_Radio_Round_Button*)w_add_sase_)->value(true);
		((Fl_Radio_Round_Button*)w_send_card_)->value(false);
		((Fl_Radio_Round_Button*)w_keep_card_)->value(false);
		((Fl_Radio_Round_Button*)w_disp_sase_)->value(false);
		break;
	case SEND_CARD:
		((Fl_Radio_Round_Button*)w_add_card_)->value(false);
		((Fl_Radio_Round_Button*)w_add_sase_)->value(false);
		((Fl_Radio_Round_Button*)w_send_card_)->value(true);
		((Fl_Radio_Round_Button*)w_keep_card_)->value(false);
		((Fl_Radio_Round_Button*)w_disp_sase_)->value(false);
		break;
	case KEEP_CARD:
		((Fl_Radio_Round_Button*)w_add_card_)->value(false);
		((Fl_Radio_Round_Button*)w_add_sase_)->value(false);
		((Fl_Radio_Round_Button*)w_send_card_)->value(false);
		((Fl_Radio_Round_Button*)w_keep_card_)->value(true);
		((Fl_Radio_Round_Button*)w_disp_sase_)->value(false);
		break;
	case DISPOSE_SASE:
		((Fl_Radio_Round_Button*)w_add_card_)->value(false);
		((Fl_Radio_Round_Button*)w_add_sase_)->value(false);
		((Fl_Radio_Round_Button*)w_send_card_)->value(false);
		((Fl_Radio_Round_Button*)w_keep_card_)->value(false);
		((Fl_Radio_Round_Button*)w_disp_sase_)->value(true);
		break;
	default:
		// should never happen
		((Fl_Radio_Round_Button*)w_add_card_)->value(false);
		((Fl_Radio_Round_Button*)w_add_sase_)->value(false);
		((Fl_Radio_Round_Button*)w_send_card_)->value(false);
		((Fl_Radio_Round_Button*)w_disp_sase_)->value(false);
		break;
	}
}

// Execute actions
void QBS_process::execute_action() {
	string batch = data_->current_batch();
	if (data_->set_record(callsign_, false)) {
		switch (action_) {
		case ADD_CARD:
			data_->add_count(batch, QI_CARDS, QD_RECEIVE, num_cards_);
			break;
		case ADD_SASE:
			data_->add_count(batch, QI_SASES, QD_RECEIVE, num_sases_);
			break;
		case SEND_CARD:
			data_->add_count(batch, QI_CARDS, QD_SEND, num_cards_);
			data_->add_count(batch, QI_CARDS, QD_DISPOSE, -num_cards_);
			data_->add_count(batch, QI_SASES, QD_SEND, num_sases_);
			data_->set_count(batch, QI_SASES, QD_DISPOSE, 0);
			break;
		case KEEP_CARD:
			data_->add_count(batch, QI_CARDS, QD_DISPOSE, -num_cards_);
			break;
		case DISPOSE_SASE:
			data_->set_count(batch, QI_SASES, QD_DISPOSE, num_sases_);
			break;
		}
	}
}

// Update 
void QBS_process::update() {
	update_actions();
	update_batches();
	update_counts();
}
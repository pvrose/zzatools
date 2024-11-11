#include "QBS_breport.h"

#include "QBS_data.h"

QBS_breport::QBS_breport(int X, int Y, int W, int H, const char* L) :
    Fl_Table(X, Y, W, H, L)
{
	cols(6);
	// Scroll Bar width
	int w_sbar = scrollbar_size();
	if (w_sbar == 0) w_sbar = Fl::scrollbar_size();
	int aw = tiw - w_sbar;
	col_width(0, aw / 6);
	col_width(1, aw / 6);
	col_width(2, aw / 6);
	col_width(3, aw / 6);
	col_width(4, aw / 6);
	col_width(5, aw / 6);
	col_header(true);
	end();
}

QBS_breport::~QBS_breport() {
	for (int ix = 0; ix < counts_.size(); ix++) delete counts_[ix];
	counts_.clear();
}

void QBS_breport::box(int b) {
	box_ = b;
	box_data* box_data = data_->get_box(box_);
	for (int ix = 0; ix < counts_.size(); ix++) delete counts_[ix];
	counts_.clear();
	auto itc = box_data->counts->begin();
	auto itr = box_data->received->begin();
	auto its = box_data->sent->begin();
	// cycle through the three sets of counts in step
	while (itc != box_data->counts->end() || itr != box_data->received->end() || its != box_data->sent->end()) {
		string call, callc, callr, calls;
		if (itc != box_data->counts->end()) {
			callc = itc->first;
			call = callc;
		}
		if (itr != box_data->received->end()) {
			callr = itr->first;
			if (call > callr) call = callr;
		}
		if (its != box_data->sent->end()) {
			calls = its->first;
			if (call > calls) call = calls;
		}
		count_data* count = new count_data;
		count->callsign = call;
		if (callr == call) {
			count->received = itr->second;
			itr++;
		}
		else {
			count->received = 0;
		}
		if (calls == call) {
			count->sent = its->second;
			its++;
		}
		else {
			count->sent = 0;
		}
		if (callc == call) {
			count->recycled = itc->second;
			itc++;
		}
		else {
			count->recycled = 0;
		}
		counts_.push_back(count);
	}
	rows((int)counts_.size());
	row_height_all(ROW_HEIGHT);
	redraw();

}

void QBS_breport::data(QBS_data* d) {
	data_ = d;
}

    // inherited from Fl_Table
void QBS_breport::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	fl_color(FL_FOREGROUND_COLOR);
	switch (context) {
	case CONTEXT_STARTPAGE:
		fl_font(0, FL_NORMAL_SIZE);
		break;
	case CONTEXT_COL_HEADER:
		fl_push_clip(X, Y, W, H);
		fl_rectf(X, Y, W, H, FL_BACKGROUND_COLOR);
		fl_rect(X, Y, W, H, FL_FOREGROUND_COLOR);
		switch (C) {
		case 0:
			fl_draw("Callsign", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 1:
			fl_draw("Received", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 2:
			fl_draw("Sent", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 3:
			fl_draw("%", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 4:
			fl_draw("Not sent", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 5:
			fl_draw("%", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		}
		fl_pop_clip();
		break;
	case CONTEXT_CELL: {
		fl_push_clip(X, Y, W, H);
		fl_rectf(X, Y, W, H, FL_BACKGROUND_COLOR);
		box_data* box_data = data_->get_box(box_);
		recycle_data* info = box_data->recycle_info;
		fl_rect(X, Y, W, H, FL_FOREGROUND_COLOR);
		string call = counts_[R]->callsign;
		int received = counts_[R]->received;
		int recycled = counts_[R]->recycled;
		int sent = counts_[R]->sent;
		char temp[16];
		memset(temp, 0, sizeof(temp));
		switch (C) {
		case 0:
			snprintf(temp, sizeof(temp), "%s", call.c_str());
			break;
		case 1:
			snprintf(temp, sizeof(temp), "%d", received);
			break;
		case 2:
			snprintf(temp, sizeof(temp), "%d", sent);
			break;
		case 3:
			if (received > 0) {
				snprintf(temp, sizeof(temp), "%d", sent * 100 / received);
			}
			break;
		case 4:
			snprintf(temp, sizeof(temp), "%d", recycled);
			break;
		case 5:
			if (received > 0) {
				snprintf(temp, sizeof(temp), "%d", recycled * 100 / received);
			}
			break;
		}
		fl_draw(temp, X, Y, W, H, FL_ALIGN_CENTER);
		fl_pop_clip();
	}

	}
}


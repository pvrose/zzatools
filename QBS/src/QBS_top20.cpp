#include "QBS_top20.h"
#include "QBS_data.h"

#include <vector>

using namespace std;

QBS_top20::QBS_top20(int X, int Y, int W, int H, const char* L) :
	Fl_Table(X, Y, W, H, L)
	,box_(0)
	,data_(nullptr)
{
	cols(4);
	// Scroll Bar width
	int w_sbar = scrollbar_size();
	if (w_sbar == 0) w_sbar = Fl::scrollbar_size();
	int aw = tiw - w_sbar;
	col_width(0, aw / 4);
	col_width(1, aw / 4);
	col_width(2, aw / 4);
	col_width(3, aw / 4);
	col_header(true);
	end();
}

QBS_top20::~QBS_top20() {}

// inherited from Fl_Table
void QBS_top20::draw_cell(TableContext context, int R, int C, int X, int Y,	int W, int H) 
{
	fl_color(FL_FOREGROUND_COLOR);
	switch(context) {
	case CONTEXT_STARTPAGE:
		fl_font(0, FL_NORMAL_SIZE);
		break;
	case CONTEXT_COL_HEADER:
		fl_push_clip(X, Y, W, H);
		fl_rectf(X, Y, W, H, FL_BACKGROUND_COLOR);
		fl_rect(X, Y, W, H, FL_FOREGROUND_COLOR);
		switch (C) {
		case 0:
			fl_draw("Rank", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 1:
			fl_draw("Callsign", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 2:
			fl_draw("Count", X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 3:
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
		string call = top20_[R];
		int count = (*box_data->counts)[call];
		int prev_count = R > 0 ? (*box_data->counts)[top20_[R - 1]] : 0;
		int pc = info->sum_recycled ? 100 * count / info->sum_recycled : 0;
		string rank = (count == prev_count) ? "=" : to_string(R + 1);
		char temp[10];

		switch (C) {
		case 0:
			fl_draw(rank.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 1:
			fl_draw(call.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 2:
			snprintf(temp, sizeof(temp), "%d", count);
			fl_draw(temp, X, Y, W, H, FL_ALIGN_CENTER);
			break;
		case 3:
			snprintf(temp, sizeof(temp), "%d%%", pc);
			fl_draw(temp, X, Y, W, H, FL_ALIGN_CENTER);
			break;
		}
		fl_pop_clip();
	}
	default:
		break;
	}

}

void QBS_top20::box(int b) {
	box_ = b;
	box_data* box_data = data_->get_box(box_);
	recycle_data* info = box_data->recycle_info;
	list<string> top20 = info->top_20;
	top20_.clear();
	// Re-oredr top20 as a vector for processing in draw_cell
	bool done = false;
	int prev_count = 0;
	int pos = 0;
	for (auto it = top20.begin(); it != top20.end() && !done; it++, pos++) {
		int count = (*box_data->counts)[(*it)];
		if (count != prev_count && pos > 20) {
			done = true;
		}
		else {
			top20_.push_back(*it);
		}
	}
	rows((int)top20_.size());
	row_height_all(ROW_HEIGHT);
	redraw();
}

void QBS_top20::data(QBS_data* d) {
	data_ = d;
}


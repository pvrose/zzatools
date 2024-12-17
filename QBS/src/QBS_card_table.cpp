#include "QBS_card_table.h"

#include "QBS_data.h"
#include "QBS_window.h"

#include "utils.h"

QBS_card_table::QBS_card_table(int X, int Y, int W, int H, const char* L) :
    Fl_Table(X, Y, W, H, L)
{
    win_ = ancestor_view<QBS_window>(this);
    data_ = win_->data_;
    rows(num_rows());
    cols(2);
    row_height_all(ROW_HEIGHT);
    col_header(true);
    col_width_all(tow / cols());
    end();
}
QBS_card_table::~QBS_card_table() {

}

// inherited from Fl_Table
void QBS_card_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
    int head = data_->get_head();
    int current = data_->get_current();

    switch (context) {
    case CONTEXT_STARTPAGE:
        fl_font(0, FL_NORMAL_SIZE);
        break;

    case CONTEXT_COL_HEADER:
        // put field header text into header (top-most row)
        fl_push_clip(X, Y, W, H);
        fl_color(FL_FOREGROUND_COLOR);
        switch (C) {
        case 0:
            fl_draw("Box", X, Y, W, H, FL_ALIGN_CENTER);
            break;
        case 1:
            fl_draw("Qty", X, Y, W, H, FL_ALIGN_CENTER);
            break;
        }
        fl_pop_clip();
        break;

    case CONTEXT_CELL:
        fl_push_clip(X, Y, W, H);
        fl_color(FL_FOREGROUND_COLOR);
        char txt[32];
        Fl_Align align;
        switch (C) {
        case 0:
            strcpy(txt, data_->get_batch(boxes_[R]).c_str());
            align = FL_ALIGN_CENTER;
            break;
        case 1:
            snprintf(txt, sizeof(txt), "%0.0f", data_->get_count(boxes_[R], call_));
            align = FL_ALIGN_RIGHT;
            break;
        }
        fl_draw(txt, X, Y, W, H, align);
        fl_pop_clip();
        break;
    }
}

// Set the callsign
void QBS_card_table::call(string call) {
    call_ = call;
    redraw();
}

// Redraw the table - size of data may have changed
void QBS_card_table::draw() {
    rows(num_rows());
    row_height_all(ROW_HEIGHT);
    Fl_Table::draw();
}

// Number of rows
int QBS_card_table::num_rows() {
    int head = data_->get_head();
    int current = data_->get_current();
    // Rows
    boxes_.clear();
    switch (win_->process()) {
    case process_mode_t::PROCESSING:
        for (int ix = head; ix <= current; ix++) {
            boxes_.push_back(ix);
        }
        boxes_.push_back(KEEP_BOX);
        boxes_.push_back(OUT_BOX);
        boxes_.push_back(SASE_BOX);
        break;
    case process_mode_t::SORTING:
        boxes_.push_back(IN_BOX);
        boxes_.push_back(KEEP_BOX);
        for (int ix = head; ix <= current; ix++) {
            boxes_.push_back(ix);
        }
        break;
    case process_mode_t::LOG_CARD:
        boxes_.push_back(IN_BOX);
        break;
    case process_mode_t::LOG_SASE:
        boxes_.push_back(SASE_BOX);
        break;
    }
    return (int)boxes_.size();
}
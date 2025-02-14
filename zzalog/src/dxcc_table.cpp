#include "dxcc_table.h"

#include "book.h"
#include "record.h"
#include "spec_data.h"

#include "drawing.h"

#include <FL/Fl_Preferences.H>

extern book* book_;
extern spec_data* spec_data_;
extern Fl_Preferences* settings_;

dxcc_table::dxcc_table(int X, int Y, int W, int H, const char* L) :
    Fl_Table(X, Y, W, H, L)
{
    end();
    display_type_ = BANDS;
    confirm_type_ = WORKED;
    scan_book();
    configure_table();
    if (visible()) redraw();
}

dxcc_table::~dxcc_table() {
}

// Set the display type
void dxcc_table::display_type(display_t t) {
    display_type_ = t;
    scan_book();
    configure_table();
    redraw();
}

// Set the conformed mode
void dxcc_table::confirm_type(confirm_t t) {
    confirm_type_ = t;
    scan_book();
    configure_table();
    redraw();
}

// inherited from Fl_Table
void dxcc_table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {

    switch (context) {
    case CONTEXT_ROW_HEADER: {
        fl_push_clip(X, Y, W, H);
        // Draw the box
        fl_color(COLOUR_GREY);
        fl_rectf(X, Y, W, H);
        fl_color(FL_FOREGROUND_COLOR);
        fl_yxline(X, Y, Y + H - 1, X + W);
        // Add the text
        fl_color(FL_FOREGROUND_COLOR);
        char text[128];
        if (R == row_ids_.size()) {
            fl_font(FL_BOLD, FL_NORMAL_SIZE);
            memcpy(text, "Total:", sizeof(text));
        }
        else if (R == row_ids_.size() + 1) {
            fl_font(FL_BOLD | FL_ITALIC, FL_NORMAL_SIZE);
            memcpy(text, "No. DXCCs:", sizeof(text));
        }
        else {
            int dxcc = row_ids_[R];
            fl_font(0, FL_NORMAL_SIZE);
            if (dxcc == -1) {
                memcpy(text, "   : Invalid entity", sizeof(text));
            }
            else {
                snprintf(text, sizeof(text), "%3d: %s", dxcc, spec_data_->entity_name(dxcc).c_str());
            }
        }
        fl_draw(text, X + 1, Y + 1, W - 2, H - 2, FL_ALIGN_LEFT);
        fl_pop_clip();
        break;
    }
    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        // Draw the box
        fl_color(COLOUR_GREY);
        fl_rectf(X, Y, W, H);
        fl_color(FL_FOREGROUND_COLOR);
        fl_yxline(X, Y, Y + H - 1, X + W);
        // Draw the txt 
        fl_color(FL_FOREGROUND_COLOR);
        if (C == column_names_.size()) {
            fl_font(FL_BOLD, FL_NORMAL_SIZE);
            fl_draw("Total", X + 1, Y + 1, W - 2, H - 2, FL_ALIGN_CENTER);
        }
        else {
            fl_font(0, FL_NORMAL_SIZE);
            fl_draw(column_names_[C].c_str(), X + 1, Y + 1, W - 2, H - 2, FL_ALIGN_CENTER);
        }
        fl_pop_clip();
        break;
    case CONTEXT_CELL: {
        fl_push_clip(X, Y, W, H);
        // Draw the box
        fl_color(FL_BACKGROUND_COLOR);
        fl_rectf(X, Y, W, H);
        fl_color(FL_FOREGROUND_COLOR);
        fl_yxline(X, Y, Y + H - 1, X + W);
        char text[10];
        if (C == column_names_.size()) {
            if (R == row_ids_.size()) {
                fl_font(FL_BOLD, FL_NORMAL_SIZE);
                snprintf(text, sizeof(text), "%d", total_qsos_);
            } else if (R == row_ids_.size() + 1) {
                fl_font(FL_BOLD | FL_ITALIC, FL_NORMAL_SIZE);
                snprintf(text, sizeof(text), "%d", data_.size());
            } else {
                fl_font(FL_BOLD, FL_NORMAL_SIZE);
                int dxcc = row_ids_[R];
                if (qsos_dxcc_.find(dxcc) == qsos_dxcc_.end()) {
                    memcpy(text, "", sizeof(text));
                }
                else {
                    snprintf(text, sizeof(text), "%d", qsos_dxcc_.at(dxcc));
                }
            }
        }
        else {
            string column_name = column_names_[C];
            if (R == row_ids_.size()) {
                fl_font(FL_BOLD, FL_NORMAL_SIZE);
                if (total_counts_.find(column_name) == total_counts_.end()) {
                    memcpy(text, "", sizeof(text));
                }
                else {
                    snprintf(text, sizeof(text), "%d", total_counts_.at(column_name));
                }
            }
            else if (R == row_ids_.size() + 1) {
                fl_font(FL_BOLD | FL_ITALIC, FL_NORMAL_SIZE);
                if (dxccs_band_.find(column_name) == dxccs_band_.end()) {
                    memcpy(text, "", sizeof(text));
                }
                else {
                    snprintf(text, sizeof(text), "%d", dxccs_band_.at(column_name));
                }
            }
            else {
                fl_font(0, FL_NORMAL_SIZE);
                int dxcc = row_ids_[R];
                count_t counts = data_.at(dxcc);
                if (counts.find(column_name) == counts.end()) {
                    memcpy(text, "", sizeof(text));
                }
                else {
                    snprintf(text, sizeof(text), "%d", counts.at(column_name));
                }
            }
        }
        fl_color(FL_FOREGROUND_COLOR);
        fl_draw(text, X + 1, Y + 1, W - 2, H - 2, FL_ALIGN_RIGHT);
        fl_pop_clip();
        break;
    }
    }
}

// Get the data
void dxcc_table::scan_book() {
    data_.clear();
    total_counts_.clear();
    column_names_.clear();
    row_ids_.clear();
    qsos_dxcc_.clear();
    dxccs_band_.clear();
    total_qsos_ = 0;
    if (book_) {
        // Get the column names
        switch (display_type_) {
        case TOTAL: {
            column_names_.push_back("All Modes");
            break;
        }
        case BANDS: {
            band_set* bands = book_->used_bands();
            for (auto it = bands->begin(); it != bands->end(); it++) {
                column_names_.push_back(*it);
            }
            break;
        }
        case MODES: {
            set<string>* modes = book_->used_modes();
            for (auto it = modes->begin(); it != modes->end(); it++) {
                column_names_.push_back(*it);
            }
            break;
        }
        case DXCC_MODES:
            set<string>* modes = book_->used_modes();
            set<string> dmodes;
            for (auto it = modes->begin(); it != modes->end(); it++) {
                dmodes.insert(spec_data_->dxcc_mode(*it));
            }
            for (auto it = dmodes.begin(); it != dmodes.end(); it++) {
                column_names_.push_back(*it);
            }
            break;
        }
        // Now read the book
        set<int> dxccs;
        for (int ix = 0; ix < book_->size(); ix++) {
            record* qso = book_->get_record(ix, false);
            // Check if we need to count it
            bool use_it = false;
            if (confirm_type_ == WORKED) use_it = true;
            if ((confirm_type_ & EQSL) && qso->item("EQSL_QSL_RCVD") == "Y") use_it = true;
            if ((confirm_type_ & LOTW) && qso->item("LOTW_QSL_RCVD") == "Y") use_it = true;
            // TODO - check how paper is distinguished from others
            if ((confirm_type_ & CARD) && qso->item("QSL_RCVD") == "Y") use_it = true;
            if (use_it) {
                int dxcc;
                qso->item("DXCC", dxcc);
                dxccs.insert(dxcc);
                count_t& data = data_[dxcc];
                string column;
                switch (display_type_) {
                case TOTAL:
                    column = "All Modes";
                    break;
                case BANDS:
                    column = qso->item("BAND");
                    break;
                case MODES:
                    column = qso->item("MODE");
                    break;
                case DXCC_MODES:
                    column = spec_data_->dxcc_mode(qso->item("MODE"));
                    break;
                }
                int& count = data[column];
                count++;
                int& total = total_counts_[column];
                total++;
                qsos_dxcc_[dxcc]++;
                total_qsos_++;
            }
        }
        // convert to vector
        for (auto it = dxccs.begin(); it != dxccs.end(); it++) {
            row_ids_.push_back(*it);
        }
        // Count #DXCCs per column
        for (auto it = data_.begin(); it != data_.end(); it++) {
            for (auto iu = (*it).second.begin(); iu != (*it).second.end(); iu++) {
                if ((*iu).second > 0) {
                    dxccs_band_[(*iu).first]++;
                }
            }
        }
    }
}
 
// Configure the table
void dxcc_table::configure_table() {
    col_header(true);
    cols(column_names_.size() + 1);
    col_header_color(COLOUR_GREY);
    row_header(true);
    row_header_color(COLOUR_GREY);
    rows(row_ids_.size() + 2);
    row_height_all(ROW_HEIGHT);
    col_width_all(WBUTTON);
    col_header_height(ROW_HEIGHT);
    row_header_width(WEDIT);
}

// Get the display type
dxcc_table::display_t dxcc_table::display_type() { return display_type_; }

// Get the confirmation type
dxcc_table::confirm_t dxcc_table::confirm_type() { return confirm_type_; }

// Get the minimum height
int dxcc_table::min_h() {
    return 6 * ROW_HEIGHT;
}
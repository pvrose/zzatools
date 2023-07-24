#include "QBS_notes.h"

QBS_notes::QBS_notes(int X, int Y, int W, int H, const char* L) :
    Fl_Table(X, Y, W, H, L) ,
    data_(nullptr)
{
	col_header(true);
	col_resize(false);
	col_header_color(FL_GRAY);
	row_header(false);
	selection_color(FL_YELLOW);
    cols(3);
    end();
}

QBS_notes::~QBS_notes() {

}

    // inherited from Fl_Table
void QBS_notes::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) 
{
	string text;
 
	switch (context) {

	case CONTEXT_STARTPAGE:
        // Do nothing
		return;

	case CONTEXT_ENDPAGE:
		// Do nothing
		return;

    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        // BG COLOR
        fl_color(FL_BACKGROUND_COLOR);
        fl_rectf(X, Y, W, H);
        fl_color(FL_BLACK);
        fl_font(fl_font() | FL_ITALIC, fl_size());
        // Get header
        switch(C) {
        case 0:
            text = "Date";
            break;
        case 1:
            text = "Name";
            break;
        case 2:
            text = "Value";
            break;
        }
  		fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_CENTER);
        fl_pop_clip();
        return;

	case CONTEXT_CELL:
		// Column indicates which record, row the field
		fl_push_clip(X, Y, W, H);
		{

            // BG COLOR
			fl_color(FL_WHITE);
			fl_rectf(X, Y, W, H);

			// TEXT
			fl_color(FL_BLACK);
            // Increment data pointer to the C'th item in the list
            note_data note = note_data{"", "", ""};
            if (data_ != nullptr && ((notes*)data_)->size() > (unsigned)R) {
                note = (*((notes*)data_))[R];
            }
            // Get the field per column
			switch (C) {
			case 0:
                text = note.date;
				break;
			case 1:
                text = note.name;
                break;
            case 2:
                text = note.value;
                break;
            }

			fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

			// BORDER
			fl_color(FL_LIGHT1);
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
		}
		fl_pop_clip();
		return;
	}

}

void QBS_notes::set_data(void* data) {
    data_ = data;
    vector<note_data>* this_data = (vector<note_data>*)data_;
    rows((int)this_data->size());
    row_height_all(FL_NORMAL_SIZE + 2);
    int colw[3] = {25, 25, 25};
    for (int c = 0; c < 3; c++) {
        for (auto it = this_data->begin(); it != this_data->end(); it++) {
            string text;
            int w = 0;
            int h = 0;
            switch (c) {
                case 0: text = (*it).date; break;
                case 1: text = (*it).name; break;
                case 2: text = (*it).value; break;
            }
            fl_measure(text.c_str(), w, h);
            colw[c] = max(colw[c], w);
       }
       col_width(c, colw[c] + 5);
    }
    redraw();
}

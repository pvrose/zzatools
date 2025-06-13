#include "banner.h"
#include "status.h"
#include "ticker.h"

#include "drawing.h"
#include "utils.h"

#include <string>

// FLTK classes
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Fill_Dial.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

using namespace std;

extern string PROGRAM_ID;
extern string PROGRAM_VERSION;
extern Fl_PNG_Image main_icon_;
extern ticker* ticker_;

const Fl_Text_Display::Style_Table_Entry style_table_[] = {
	{ STATUS_COLOURS.at(ST_NONE).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NONE).bg },
	{ STATUS_COLOURS.at(ST_LOG).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_LOG).bg },
	{ STATUS_COLOURS.at(ST_DEBUG).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_DEBUG).bg },
	{ STATUS_COLOURS.at(ST_NOTE).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NOTE).bg },
	{ STATUS_COLOURS.at(ST_PROGRESS).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_PROGRESS).bg,},
	{ STATUS_COLOURS.at(ST_OK).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_OK).bg },
	{ STATUS_COLOURS.at(ST_WARNING).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_WARNING).bg },
	{ STATUS_COLOURS.at(ST_NOTIFY).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NOTIFY).bg },
	{ STATUS_COLOURS.at(ST_ERROR).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_ERROR).bg },
	{ STATUS_COLOURS.at(ST_SEVERE).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_SEVERE).bg },
	{ STATUS_COLOURS.at(ST_FATAL).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_FATAL).bg }
};

banner::banner(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L)
	, large_(false)
	, h_large_(0)
	, h_small_(0)
{
	// Set the ticker for 2 seconds
	ticker_->add_ticker(this, cb_ticker, 20);
	callback(cb_close);
	create_form();
	enable_widgets();
}

banner::~banner() {

}

void banner::create_form() {
	const int HMULT = 2 * HBUTTON;
	const int HICON = HMULT * 2 + GAP;
	const int WOP = WEDIT * 2;
	int curr_x = x() + GAP;
	int curr_y = y() + GAP;

	// Create a box to hild the icon and resize it thereinto
	bx_icon_ = new Fl_Box(curr_x, curr_y, HICON, HICON);
	Fl_Image* image = main_icon_.copy();
	image->scale(HICON, HICON);
	bx_icon_->image(image);

	curr_x += HICON + GAP;
	// Output to display low severity (< WARNING) message
	op_msg_low_ = new Fl_Multiline_Output(curr_x, curr_y, WOP, HMULT);
	op_msg_low_->wrap(true);
	op_msg_low_->textsize(FL_NORMAL_SIZE);

	curr_y += HMULT + GAP;
	// Output to displaye high severity (>= WARNING) message
	op_msg_high_ = new Fl_Multiline_Output(curr_x, curr_y, WOP, HMULT);
	op_msg_high_->wrap(true);
	op_msg_high_->textsize(FL_NORMAL_SIZE);
	curr_y = max(curr_y + HMULT, y() + GAP + HICON) + GAP;
	curr_x = x() + GAP;

	// button to pull down the full text display
	bn_pulldown_ = new Fl_Button(curr_x, curr_y, HMULT, HBUTTON, "Full");
	bn_pulldown_->callback(cb_bn_pulldown, nullptr);
	bn_pulldown_->labelsize(FL_NORMAL_SIZE);

	curr_x += HMULT + GAP;
	// Progress "clock"
	fd_progress_ = new Fl_Fill_Dial(curr_x, curr_y, HMULT, HMULT);
	fd_progress_->minimum(0.0);
	fd_progress_->maximum(1.0);
	fd_progress_->color(FL_BACKGROUND_COLOR);
	fd_progress_->angles(180, 540);
	fd_progress_->box(FL_OVAL_BOX);

	curr_x = op_msg_high_->x();

	// Progress message
	op_prog_title_ = new Fl_Output(curr_x, curr_y, WOP, HBUTTON);
	op_prog_title_->textsize(FL_NORMAL_SIZE);

	curr_y += HBUTTON;
	// Progress value
	op_prog_value_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON);
	op_prog_value_->align(FL_ALIGN_RIGHT);
	op_prog_value_->textsize(FL_NORMAL_SIZE);

	curr_y += HBUTTON + GAP;
	h_small_ = curr_y - y();

	int max_x = curr_x + WOP;

	curr_x = x() + GAP;
	// Text display for all messages
	display_ = new Fl_Text_Display(curr_x, curr_y, max_x - curr_x, 10 * HBUTTON);
	display_->color(STATUS_COLOURS.at(ST_NONE).bg);
	Fl_Text_Buffer* text = new Fl_Text_Buffer;
	display_->buffer(text);
	Fl_Text_Buffer* style = new Fl_Text_Buffer;
	display_->highlight_data(style, &style_table_[0], 11, ' ', nullptr, nullptr);

	h_large_ = h_small_ + display_->h() + GAP;

	resizable(nullptr);
	size(max_x + GAP - x(), h_small_);

	show();
}

void banner::enable_widgets() {}

// Add a message to the banner
void banner::add_message(status_t type, const char* msg, object_t object, const char* prg_unit) {
	switch (type) {
	case ST_PROGRESS: {
		op_prog_title_->value(msg);
		fd_progress_->selection_color(OBJECT_COLOURS.at(object));
		prg_unit_ = prg_unit;
		break;
	}
	case ST_NONE:
	case ST_LOG:
	case ST_DEBUG:
	case ST_NOTE:
	case ST_OK: {
		op_msg_low_->value(msg);
		op_msg_low_->color(STATUS_COLOURS.at(type).bg);
		op_msg_low_->textcolor(STATUS_COLOURS.at(type).fg);
		break;
	}
	case ST_WARNING:
	case ST_NOTIFY:
	case ST_ERROR:
	case ST_SEVERE:
	case ST_FATAL: {
		op_msg_high_->value(msg);
		op_msg_high_->color(STATUS_COLOURS.at(type).bg);
		op_msg_high_->textcolor(STATUS_COLOURS.at(type).fg);
		break;
	}
	}
	copy_msg_display(type, msg);
	redraw();
}

// Add progress
void banner::add_progress(uint64_t value, uint64_t max_value) {
	char text[64];
	if (max_value != -1) {
		max_value_ = max_value;
		prev_value_ = -(max_value_ / 2);
		snprintf(text, sizeof(text), "/%lld", max_value_);
		op_prog_value_->copy_label(text);
		redraw();
	}
	// Do not redraw these until ticker
	if ((value - prev_value_) > (max_value_ / 100) ) {
		snprintf(text, sizeof(text), "%lld", value);
		op_prog_value_->value(text);
		fd_progress_->value((double)value / double(max_value_));
		prev_value_ = value;
		redraw();
	}
}

// Update progress
void banner::ticker() {
	op_prog_value_->redraw();
	fd_progress_->redraw();
}

// Callback - ticker
void banner::cb_ticker(void* v) {
	banner* that = (banner*)v;
	that->ticker();
}

// Callback - drop down viewer
void banner::cb_bn_pulldown(Fl_Widget* w, void* v) {
	banner* that = ancestor_view<banner>(w);
	if (that->h() == that->h_small_) {
		that->size(that->w(), that->h_large_);
	}
	else {
		that->size(that->w(), that->h_small_);
	}
	that->redraw();

}

// Callback - close button - ignore
void banner::cb_close(Fl_Widget* w, void* v) {
	printf("ZZALOG: Cannot close the banner\n");
}

// Add message to display (with colour)
void banner::copy_msg_display(status_t type, const char* msg) {
	// Append the message to the text buffer
	Fl_Text_Buffer* buffer = display_->buffer();
	buffer->append(msg);
	buffer->append("\n");
	// Now set the style of the added characters
	// Create a string of the required length and fill it with the style character
	size_t len = strlen(msg) + 1;
	char* style = new char[len + 1];
	char s_char = (char)type + 'A';
	memset(style, s_char, len);
	style[len] = '\0';
	// Append it to the style buffer
	Fl_Text_Buffer* s_buffer = display_->style_buffer();
	s_buffer->append(style);

}

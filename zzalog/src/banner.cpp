#include "banner.h"
#include "status.h"
#include "ticker.h"

#include "drawing.h"
#include "utils.h"

#include <string>
#include <thread>

// FLTK classes
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Fill_Dial.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>



extern std::string PROGRAM_ID;
extern std::string PROGRAM_VERSION;
extern std::string CONTACT;
extern std::string COPYRIGHT;
extern Fl_PNG_Image main_icon_;
extern status* status_;
extern ticker* ticker_;
extern bool closing_;
extern void open_html(const char*);

const int NUMBER_STYLES = 10;
const Fl_Text_Display::Style_Table_Entry style_table_[NUMBER_STYLES] = {
	{ STATUS_COLOURS.at(ST_NONE).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NONE).bg },
	{ STATUS_COLOURS.at(ST_LOG).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_LOG).bg },
	{ STATUS_COLOURS.at(ST_DEBUG).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_DEBUG).bg },
	{ STATUS_COLOURS.at(ST_NOTE).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_NOTE).bg },
	{ STATUS_COLOURS.at(ST_PROGRESS).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_PROGRESS).bg,},
	{ STATUS_COLOURS.at(ST_OK).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_OK).bg },
	{ STATUS_COLOURS.at(ST_WARNING).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_WARNING).bg },
	{ STATUS_COLOURS.at(ST_ERROR).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_ERROR).bg },
	{ STATUS_COLOURS.at(ST_SEVERE).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_SEVERE).bg },
	{ STATUS_COLOURS.at(ST_FATAL).fg, FL_COURIER, FL_NORMAL_SIZE, Fl_Text_Display::ATTR_BGCOLOR, STATUS_COLOURS.at(ST_FATAL).bg }
};

std::thread::id main_thread_id_ = std::this_thread::get_id();

banner::banner(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L)
	, can_close_(false)
{
	// Set the ticker for 2 seconds
	callback(cb_close);
	create_form();
	enable_widgets();
}

banner::~banner() {

}

// Handle
int banner::handle(int event) {
	int result = Fl_Double_Window::handle(event);
	// Now handle F1 regardless
	switch (event) {
	case FL_FOCUS:
		return true;
	case FL_UNFOCUS:
		// Acknowledge focus events to get the keyboard event
		return true;
	case FL_PUSH:
		take_focus();
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_F + 1:
			open_html("banner.html");
			return true;
		}
		break;
	}
	return result;
}

// Visualise or no the close overay
void banner::draw() {
	if (closing_) bx_closing_->show();
	else bx_closing_->hide();
	Fl_Double_Window::draw();
}

void banner::create_form() {
	const int HMULT = 2 * HBUTTON;
	const int HICON = HMULT * 2 + GAP;
	const int WOP = w() - GAP - GAP - HICON - GAP;
	const int H_TOP = HMULT + GAP + HMULT + GAP + HMULT + GAP;

	int curr_x = x() + GAP;
	int curr_y = y() + GAP;
	
	Fl_Group* g_top = new Fl_Group(x(), curr_y, w(), H_TOP);
	g_top->box(FL_FLAT_BOX);

	Fl_Group* g_topleft = new Fl_Group(curr_x, curr_y, HICON, HICON + HMULT + GAP);
	g_topleft->box(FL_FLAT_BOX);
	// Create a box to hild the icon and resize it thereinto
	bx_icon_ = new Fl_Box(curr_x, curr_y, HICON, HICON);
	Fl_Image* image = main_icon_.copy();
	image->scale(HICON, HICON);
	bx_icon_->image(image);

	curr_y += HICON + GAP;
	// Progress "clock"
	fd_progress_ = new Fl_Fill_Dial(curr_x + HICON - HMULT, curr_y, HMULT, HMULT);
	fd_progress_->minimum(0.0);
	fd_progress_->maximum(1.0);
	fd_progress_->color(FL_BACKGROUND_COLOR);
	fd_progress_->angles(180, 540);
	fd_progress_->box(FL_OVAL_BOX);

	g_topleft->end();

	curr_x += HICON + GAP;
	curr_y = y() + GAP;

	Fl_Group* g_topright = new Fl_Group(curr_x, curr_y, WOP, H_TOP);
	g_topright->box(FL_FLAT_BOX);

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

	// Progress message
	op_prog_title_ = new Fl_Output(curr_x, curr_y, WOP, HBUTTON);
	op_prog_title_->textsize(FL_NORMAL_SIZE);

	curr_y += HBUTTON;
	// Progress value
	bx_prog_value_ = new Fl_Box(curr_x, curr_y, WOP, HBUTTON);
	bx_prog_value_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	curr_x = x() + GAP;

	g_topright->end();
	g_top->resizable(g_topright);
	g_top->end();


	curr_x = op_msg_high_->x();

	bx_closing_ = new Fl_Box(bx_icon_->x(), bx_icon_->y(), w(), HICON, "CLOSING!");
	bx_closing_->labelsize(FL_NORMAL_SIZE * 5);
	bx_closing_->labelcolor(FL_YELLOW);
	bx_closing_->box(FL_BORDER_FRAME);
	bx_closing_->hide();

	curr_x = x() + GAP;
	curr_y += fd_progress_->h();

	int h_display = h() - (curr_y + HBUTTON);

	// Text display for all messages
	display_ = new Fl_Text_Display(curr_x, curr_y, w() - GAP - GAP, h_display);
	display_->color(STATUS_COLOURS.at(ST_NONE).bg);
	Fl_Text_Buffer* text = new Fl_Text_Buffer;
	display_->buffer(text);
	Fl_Text_Buffer* style = new Fl_Text_Buffer;
	display_->highlight_data(style, &style_table_[0], NUMBER_STYLES, ' ', nullptr, nullptr);

	curr_y += display_->h();
	Fl_Box* bx_copyright = new Fl_Box(curr_x, curr_y, w() - GAP - GAP, HBUTTON);
	char msg[128];
	snprintf(msg, sizeof(msg), "%s %s     ", COPYRIGHT.c_str(), CONTACT.c_str());
	bx_copyright->copy_label(msg);
	bx_copyright->labelsize(FL_NORMAL_SIZE - 1);
	bx_copyright->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	curr_y += HBUTTON + GAP;

	resizable(display_);
	size_range(w(), h());
	// Center the banner in the middle of the screen (windows manager permitting)
	int sx, sy, sw, sh;
	Fl::screen_xywh(sx, sy, sw, sh, x(), y());
	int nx = sx + (sw / 2) - (w() / 2);
	int ny = sy + (sh / 2) - (h() / 2);
	resize(nx, ny, w(), h());
	end();
	show();
}

void banner::enable_widgets() {
}

// Add a message to the banner
void banner::add_message(status_t type, const char* msg) {
	// Trap any call that is not from the main std::thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::add_message(%s) when not in the main std::thread\n", msg);
		throw;
	}
	switch (type) {
	case ST_PROGRESS: {
		break;
	}
	case ST_NONE:
	case ST_LOG:
	case ST_DEBUG:
	case ST_NOTE:
	case ST_OK: 
	case ST_WARNING:
	{
		op_msg_low_->value(msg);
		op_msg_low_->color(STATUS_COLOURS.at(type).bg);
		op_msg_low_->textcolor(STATUS_COLOURS.at(type).fg);
		break;
	}
	case ST_ERROR:
	case ST_SEVERE:
	case ST_FATAL:
	{
		op_msg_high_->value(msg);
		op_msg_high_->color(STATUS_COLOURS.at(type).bg);
		op_msg_high_->textcolor(STATUS_COLOURS.at(type).fg);
		hide();
		set_non_modal();
		show();
		break;
	}
	}
	copy_msg_display(type, msg);
	redraw();
	if (visible()) Fl::check();
}

// Add progress
void banner::start_progress(uint64_t max_value, object_t object, const char* msg, const char* suffix) {
	// Trap any call that is not from the main std::thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner_start_progress(%s) when not in the main std::thread\n", msg);
		throw;
	}
	char text[128];
	const uint64_t FRACTION = 100L;
	max_value_ = max_value;
	delta_ = max_value / FRACTION;
	prev_value_ = 0L;
	prg_msg_ = msg;
	prg_unit_ = suffix;
	prg_object_ = object;
	// Set display message
	snprintf(text, sizeof(text), "%s: PROGRESS: Starting %s - %lld %s", OBJECT_NAMES.at(object), msg, max_value, suffix);
	status_->misc_status(ST_PROGRESS, text);
	snprintf(text, sizeof(text), "%s: %s.", OBJECT_NAMES.at(object), msg);
	op_prog_title_->value(text);
	snprintf(text, sizeof(text), "0 out of %lld %s", max_value, suffix);
	bx_prog_value_->copy_label(text);
	fd_progress_->selection_color(OBJECT_COLOURS.at(object));

	redraw();
	if (visible()) Fl::check();
}

// Update progress dial and output
void banner::add_progress(uint64_t value) {
	// Trap any call that is not from the main std::thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::add_progress(%lld) when not in the main std::thread\n", value);
		throw;
	}
	char text[128];
	if ((value == max_value_) || (value - prev_value_) > delta_) {
		snprintf(text, sizeof(text), "%lld out of %lld %s", value, max_value_, prg_unit_);
		bx_prog_value_->copy_label(text);
		fd_progress_->value((double)value / double(max_value_));
		prev_value_ = value;
		if (value == max_value_) {
			end_progress();
		}
		else {
			// Only redraw these two items.
			fd_progress_->redraw();
			bx_prog_value_->redraw();
			if (visible()) Fl::check();
		}
	}
}

// Ending the progress - log message
void banner::end_progress() {
	// Trap any call that is not from the main std::thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::end_progress(%s) when not in the main std::thread\n", prg_msg_);
		throw;
	}
	char text[128];
	// Set display message
	snprintf(text, sizeof(text), "%s: PROGRESS: Ending %s", OBJECT_NAMES.at(prg_object_), prg_msg_);
	status_->misc_status(ST_PROGRESS, text);
	snprintf(text, sizeof(text), "%s: %s. Done", OBJECT_NAMES.at(prg_object_), prg_msg_);
	op_prog_title_->value(text);
	redraw();
	if (visible()) Fl::check();
}

// cancelling the progress - log message
void banner::cancel_progress(const char* msg) {
	// Trap any call that is not from the main std::thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::cancel_progress(%s) when not in the main std::thread\n", msg);
		throw;
	}
	char text[128];
	// Set display message
	snprintf(text, sizeof(text), "%s: PROGRESS: cancelling %s - %s", OBJECT_NAMES.at(prg_object_), prg_msg_, msg);
	status_->misc_status(ST_PROGRESS, text);
	snprintf(text, sizeof(text), "%s: %s. Cancelled (%s)", OBJECT_NAMES.at(prg_object_), prg_msg_, msg);
	op_prog_title_->value(text);
	redraw();
	if (visible()) Fl::check();
}

// Callback - close button - ignore
void banner::cb_close(Fl_Widget* w, void* v) {
	banner* that = (banner*)w;
	if (that->can_close_) {
		Fl_Double_Window::default_callback(that, v);
	}
	else {
		printf("ZZALOG: Cannot close the banner\n");
	}
}

// Add message to display (with colour)
void banner::copy_msg_display(status_t type, const char* msg) {
	// Append the message to the text buffer
	Fl_Text_Buffer* buffer = display_->buffer();
	buffer->append(msg);
	buffer->append("\n");
	// Now std::set the style of the added characters
	// Create a std::string of the required length and fill it with the style character
	size_t len = strlen(msg) + 1;
	char* style = new char[len + 1];
	char s_char = (char)type + 'A';
	memset(style, s_char, len);
	style[len] = '\0';
	// Append it to the style buffer
	Fl_Text_Buffer* s_buffer = display_->style_buffer();
	s_buffer->append(style);
	display_->scroll(buffer->count_lines(0, buffer->length()),0);

}

// std::set the can_close flag
void banner::allow_close() {
	can_close_ = true;
}

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

using namespace std;

extern string PROGRAM_ID;
extern string PROGRAM_VERSION;
extern string CONTACT;
extern string COPYRIGHT;
extern Fl_Help_Dialog* help_viewer_;
extern Fl_PNG_Image main_icon_;
extern status* status_;
extern ticker* ticker_;

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

const char help_text[] =
"<body>"
"<h1>banner - Progress banner</h1>"
"<h2>Description</h2>"
"Displays any status messages and progress reports output by ZZALOG."
"<h2>Features</h2>"
"This window comprises from top to bottom.<p>"
"This displays the status messages output by ZZALOG that are at most of warning severity."
"Only the last message output is displayed here.<p>"
"The second displays status messages that are of error severity or higher."
"Again only the last such message is output here.<p>"
"The third comprises three items. Firstly a text output showing a progress message output by ZZALOG."
"Below this is a numeric representation of progress - e.g. \"1096 out of 2677 bytes.\""
"On the left is a graphical representation in the form of a clock dial, showing "
"proportion complete. The clock-dial is colour-coded indicating the activity being progressed."
"<h3>Displaying the whole report.</h3>"
"Bottom left of the window is the button marked \"Full\"."
"Clicking this toggles the display of the full status.<p>"
"Messages are colour-coded according to severity."
"<dl>"
"  <li>Blue - lowest severity"
"  <li>Light blue - Messages to be logged for information"
"  <li>Light magenta - Messages added for debug purposes"
"  <li>Light cyan - Message logged as notes"
"  <li>Grey - Messages logged for progress"
"  <li>Light green - message logged indicating completion of an activity"
"  <li>Yellow - message logged as a warning"
"  <li>Red on black - message logged as an error - processing can continue"
"  <li>Red on white - message logged as a severe error - processing is stopped"
"  <li>Black on red - message logged as a fatal error - processing is stopeed"
"</dl>"
"</body>";

std::thread::id main_thread_id_ = std::this_thread::get_id();

banner::banner(int W, int H, const char* L) :
	Fl_Double_Window(W, H, L)
	, large_(false)
	, h_large_(0)
	, h_small_(0)
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
			help_viewer_->value(help_text);
			help_viewer_->show();
			return true;
		}
		break;
	}
	return result;
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
	bx_prog_value_ = new Fl_Box(curr_x, curr_y, WEDIT, HBUTTON);
	bx_prog_value_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	curr_y += HBUTTON;

	int max_x = curr_x + WOP;
	curr_x = x() + GAP;

	Fl_Box* bx_copyright = new Fl_Box(curr_x, curr_y, max_x - x(), HBUTTON);
	char msg[128];
	snprintf(msg, sizeof(msg), "%s %s     ", COPYRIGHT.c_str(), CONTACT.c_str());
	bx_copyright->copy_label(msg);
	bx_copyright->labelsize(FL_NORMAL_SIZE - 1);
	bx_copyright->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

	curr_y += HBUTTON + GAP;
	h_small_ = curr_y - y();

	curr_x = x() + GAP;
	// Text display for all messages
	display_ = new Fl_Text_Display(curr_x, curr_y, max_x - curr_x, 10 * HBUTTON);
	display_->color(STATUS_COLOURS.at(ST_NONE).bg);
	Fl_Text_Buffer* text = new Fl_Text_Buffer;
	display_->buffer(text);
	Fl_Text_Buffer* style = new Fl_Text_Buffer;
	display_->highlight_data(style, &style_table_[0], NUMBER_STYLES, ' ', nullptr, nullptr);

	h_large_ = h_small_ + display_->h() + GAP;

	resizable(nullptr);
	size(max_x + GAP - x(), h_small_);
	// Center the banner in the middle of the screen (windows manager permitting)
	int sx, sy, sw, sh;
	Fl::screen_xywh(sx, sy, sw, sh, x(), y());
	int nx = sx + (sw / 2) - (w() / 2);
	int ny = sy + (sh / 2) - (h() / 2);
	resize(nx, ny, w(), h());
	end();
	show();
}

void banner::enable_widgets() {}

// Add a message to the banner
void banner::add_message(status_t type, const char* msg) {
	// Trap any call that is not from the main thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::add_message(%s) when not in the main thread\n", msg);
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
	// TODO: This appears to cause more trouble than it is worth.
	if (visible()) Fl::check();
}

// Add progress
void banner::start_progress(uint64_t max_value, object_t object, const char* msg, const char* suffix) {
	// Trap any call that is not from the main thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner_start_progress(%s) when not in the main thread\n", msg);
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
	// Trap any call that is not from the main thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::add_progress(%lld) when not in the main thread\n", value);
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
	// Trap any call that is not from the main thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::end_progress(%s) when not in the main thread\n", prg_msg_);
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
	// Trap any call that is not from the main thread
	if (this_thread::get_id() != main_thread_id_) {
		printf("Calling banner::cancel_progress(%s) when not in the main thread\n", msg);
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
	display_->scroll(buffer->count_lines(0, buffer->length()),0);

}

// set the can_close flag
void banner::allow_close() {
	can_close_ = true;
}

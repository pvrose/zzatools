#include "status.h"

#include "utils.h"
#include "callback.h"
#include "menu.h"
#include "intl_widgets.h"
#include "main_window.h"
#include "qso_manager.h"

#include <iostream>

#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>

using namespace std;

extern Fl_Preferences* settings_;
extern menu* menu_;
extern main_window* main_window_;
extern status* status_;
extern qso_manager* qso_manager_;
extern bool READ_ONLY;
extern bool close_by_error_;
extern string PROGRAM_ID;
extern string VERSION;
extern bool DEBUG_STATUS;

// Constructor
status::status(int X, int Y, int W, int H, const char* label) :
	Fl_Group(X, Y, W, H, label)
	, progress_(nullptr)
	, rig_status_(nullptr)
	, misc_status_(nullptr)
	, file_status_(nullptr)
	, status_file_viewer_(nullptr)
	, report_filename_("")
	, report_file_(nullptr)
	, min_level_(ST_NONE)
	, append_log_(false)
	, rig_in_progress_(false)
	, file_unusable_(false)
	, no_update_viewer(false)
	, display_debug_messages_(false)
{
	// Initialise attributes
	progress_stack_.clear();
	progress_items_.clear();

	// Current position for next widget
	int curr_x = X;
	// Add modified indicator
	const int file_w = H;
	// BUTTON - colour by file status
	file_status_ = new Fl_Button(curr_x, Y, file_w, H);
	file_status_->box(FL_DOWN_BOX);
	file_status_->color(FILE_STATUS_COLOURS.at(FS_EMPTY), FILE_STATUS_COLOURS.at(FS_EMPTY));
	file_status_->tooltip("File status:\n\tOff white - log empty\n\tYellow - loading file\n"
		"\tGreen - log saved to file\n\tRed - log modified\n\tCyan - saving file"
		"\n\tDark Green - read-only unmodified\n\tDark Red - read-only modified");
	add(file_status_);
	curr_x += file_w;
	const int rem_w = W - curr_x;
	const int WPROG = 2;
	const int WRIG = 3;
	const int WMISC = 5;
	const int WALL = WPROG + WRIG + WMISC;

	// Add a progress bar widget - updated by all the processes which take a while
	const int progress_w = rem_w * WPROG / WALL;
	// Progress bar
	progress_ = new Fl_Progress(curr_x, Y, progress_w, H, "Initialising...");
	progress_->align(FL_ALIGN_INSIDE);
	progress_->tooltip("Activity progress");
	progress_->color(FL_WHITE, OBJECT_COLOURS.at(OT_NONE));
	add(progress_);
	curr_x += progress_w;

	// Add rig status
	const int rig_status_w = rem_w * WRIG / WALL;
	// Button - rig status
	rig_status_ = new Fl_Button(curr_x, Y, rig_status_w, H, "Rig connection not yet established");
	rig_status_->box(FL_DOWN_BOX);
	rig_status_->color(STATUS_COLOURS.at(ST_NONE));
	rig_status_->labelcolor(FL_BLACK);
	rig_status_->copy_tooltip(rig_status_->label());
	// Add callback to try and reconnect the rig
	rig_status_->callback(cb_bn_rig);
	rig_status_->when(FL_WHEN_RELEASE);
	add(rig_status_);
	curr_x += rig_status_w;

	// Miscellaneous status - used for general status and error information
	const int misc_w = rem_w * WMISC / WALL;
	// Button - status message 
	misc_status_ = new Fl_Button(curr_x, Y, misc_w, H, "Miscellaneous information");
	misc_status_->box(FL_DOWN_BOX);
	misc_status_->color(STATUS_COLOURS.at(ST_NONE));
	misc_status_->labelcolor(FL_BLACK);
	misc_status_->copy_tooltip(misc_status_->label());
	// Callback will open the status log report
	misc_status_->callback(cb_bn_misc);
	misc_status_->when(FL_WHEN_RELEASE);
	add(misc_status_);
	curr_x += misc_w;

	end();

	// Get report filename from the settings
	char * filename;
	Fl_Preferences status_settings(settings_, "Status");
	status_settings.get("Report File", filename, "");
	report_filename_ = filename;
	free(filename);
	// If it's not in the settings, open file dialog, get it and set it.
	while (report_filename_.length() == 0) {
		// Create an Open dialog; the default file name extension is ".txt".
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
		chooser->title("Select file name for status report");
		chooser->filter("Text files\t*.txt");
		if (chooser->show() == 0) {
			report_filename_ = chooser->filename();
			status_settings.set("Report File", report_filename_.c_str());
		}
		delete chooser;
	}

	status_settings.get("Minimum Level", (int&)min_level_, ST_NOTE);
	// Update menu to change enables
	menu_->status_level(min_level_);
	menu_->append_file(append_log_);

	status_settings.get("Append", (int&)append_log_, false);

	status_file_viewer_ = nullptr;
}

// Destructor
status::~status()
{
	clear();
	if (!close_by_error_) delete status_file_viewer_;
	if (report_file_) report_file_->close();
	for (auto it = progress_items_.begin(); it != progress_items_.end(); it++) {
		delete (it->second);
	}
	progress_items_.clear();
	progress_stack_.clear();
}

// Remove file viewer
void status::null_file_viewer() {
	delete status_file_viewer_;
	status_file_viewer_ = nullptr;
}

// Callbacks
// Rig status bn callback - attempts to toggle rig connection state
void status::cb_bn_rig(Fl_Widget* bn, void* v) {
	status* that = ancestor_view<status>(bn);
	qso_manager_->switch_rig();
	bn->redraw();
}

// Add a progress item to the stack
void status::progress(int max_value, object_t object, const char* description, const char* suffix, bool countdown /*= false*/) {
	// Turrn off file viewer update to improve performance
	no_update_viewer = true;
	// Initialise it
	if (progress_items_.find(object) != progress_items_.end()) {
		// We already have a progress bar process in place for this object
		char message[100];
		snprintf(message, 100, "PROGRESS: Already started progress for %s", OBJECT_NAMES.at(object));
		misc_status(ST_ERROR, message);
	} else {
		// Start a new progress bar process - create the progress item (total expected count, objects being counted, up/down and what view it's for)
		char message[100];
		snprintf(message, 100, "PROGRESS: Starting %s - %d %s", description, max_value, suffix);
		misc_status(ST_LOG, message);
		progress_item* item = new progress_item;
		item->max_value = max_value;
		if (countdown) {
			item->value = max_value;
		}
		else {
			item->value = 0;
		}
		item->countdown = countdown;
		item->suffix = new char[strlen(suffix) + 1];
		item->description = new char[strlen(description) + 1];
		item->prev_value = 0;
		strcpy(item->suffix, suffix);
		strcpy(item->description, description);
		progress_items_[object] = item;
		// Push it onto the stack of progess bar processes
		progress_stack_.push_back(object);
		update_progress(object);
	}
}

// Update the progress bar for the specified object
void status::update_progress(object_t object) {
	if (progress_stack_.size() && progress_stack_.back() == object) {
		// The progress bar is top of the stack - don't update the bar if it isn't
		progress_item* item = progress_items_.at(object);
		Fl_Color bar_colour = OBJECT_COLOURS.at(object);
		// Set colours
		progress_->color(OBJECT_COLOURS.at(OT_NONE), bar_colour);
		progress_->labelcolor(FL_BLACK);
		char label[100];
		// Update progress up or down
		if (item->countdown) {
			progress_->value((float)(item->max_value - item->value));
		}
		else {
			progress_->value((float)item->value);
		}
		sprintf(label, "%d/%d %s", item->value, item->max_value, item->suffix);
		progress_->copy_label(label);
		// Set range (0:max_value)
		progress_->minimum(0.0);
		progress_->maximum((float)item->max_value);
	}
}

// Update progress bar to the new specified value
void status::progress(int value, object_t object) {
	if (progress_stack_.size()) {
		if (progress_items_.find(object) == progress_items_.end()) {
			char message[100];
			snprintf(message, 100, "PROGRESS: %s not started but %d done", OBJECT_NAMES.at(object), value);
			misc_status(ST_ERROR, message);
		} else {
			// Update progress item
			progress_item* item = progress_items_.at(object);
			item->value = value;
			// update_progress(object);
			// If it's 100% (or 0% if counting down) delete item and draw the next in the stack - certain objects can overrun (this will give an error)
			if ((item->countdown && value <= 0) || (!item->countdown && value >= item->max_value)) {
				char message[100];
				snprintf(message, 100, "PROGRESS: %s finished (%d %s)", item->description, item->max_value, item->suffix);
				misc_status(ST_LOG, message);
				update_progress(object);
				// Remove the item from the stack - even if it's not top of the stack
				delete item;
				progress_items_.erase(object);
				progress_stack_.remove(object);
				// if (progress_stack_.size()) {
				// 	// Revert to previous progress item (current top-of-stack)
				// 	update_progress(progress_stack_.back());
				// }
			}
			Fl::check();
		}
	}
}

// Update progress bar with a message - e.g. cancel it and display why cancelled
void status::progress(const char* message, object_t object) {
	if (progress_stack_.size()) {
		if (progress_items_.find(object) == progress_items_.end()) {
			// WE didn't start the bar in the first place
			char msg[100];
			snprintf(msg, 100, "PROGRESS: %s not started %s", OBJECT_NAMES.at(object), message);
			misc_status(ST_ERROR, msg);
		}
		else {
			progress_item* item = progress_items_.at(object);
			char msg[100];
			snprintf(msg, 100, "PROGRESS: %s abandoned %s (%d %s)", OBJECT_NAMES.at(object), message, item->value, item->suffix);
			misc_status(ST_LOG, msg);
			if (message) {
				progress_->copy_label(message);
			}
			// Mark progress bar complete
			if (item->countdown) {
				progress((int)0, object);
			}
			else {
				progress(item->max_value, object);
			}
		}
	}
}

// 1 second ticker - redraw progress bar
void status::ticker() {
	// Redraw all status 
	if (progress_stack_.size()) {
		// Display progress item at top of stack
		update_progress(progress_stack_.back());
	}
	Fl::check();
}

// Update rig_status - set text and colour
void status::rig_status(rig_status_t status, const char* label) {
	// Set the fixed label and tool tip value. Set the text colour to contrast the button colour
	Fl_Color colour = RIG_STATUS_COLOURS.at(status);
	rig_status_->copy_label(label);
	rig_status_->color(colour);
	rig_status_->labelcolor(fl_contrast(FL_BLACK, colour));
	rig_status_->copy_tooltip(label);
	//rig_status_->redraw();
}

// Update miscellaneous status - set text and colour, log the status
void status::misc_status(status_t status, const char* label) {
	// If we are displaying the message at this level
	if ((int)status >= (int)min_level_ && (status != ST_DEBUG || display_debug_messages_)) {
		// Set the fixed label and tool tip value. Set the text colour to contrast the button colour
		Fl_Color colour = STATUS_COLOURS.at(status);
		misc_status_->copy_label(label);
		misc_status_->color(colour);
		misc_status_->labelcolor(fl_contrast(FL_BLACK, colour));
		misc_status_->copy_tooltip(label);
		misc_status_->redraw();
	}

	// Start each entry with a timestamp
	string timestamp = now(false, "%Y/%m/%d %H:%M:%S");
	char* message = new char[timestamp.length() + 10 + strlen(label)];
	// X YYYY/MM/DD HH:MM:SS Message 
	// X is a single letter indicating the message severity
	sprintf(message, "%c %s %s\n", STATUS_CODES.at(status), timestamp.c_str(), label);

	if (!report_file_) {
		// Append the status to the file
		// Try to open the file. Open and close it each message
		if (append_log_) {
			// Append the message to the log
			report_file_ = new ofstream(report_filename_, ios::out | ios::app);
		}
		else {
			// Create a new file 
			report_file_ = new ofstream(report_filename_, ios::out | ios::trunc);
		}
		if (!report_file_->good()) {
			// File didn't open correctly
			delete report_file_;
			report_file_ = nullptr;
			file_unusable_ = true;
			fl_alert("STATUS: Failed to open status report file %s", report_filename_.c_str());
			// It doesn'r exist get a new filename
			// open file dialog, get it and set it.
			report_filename_ = "";
			while (report_filename_.length() == 0) {
				// Create an Open dialog; the default file name extension is ".txt".
				Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
				chooser->title("Select file name for status report");
				chooser->filter("Text files\t*.txt");
				if (chooser->show() == 0) {
					report_filename_ = chooser->filename();
					Fl_Preferences status_settings(settings_, "Status");
					status_settings.set("Report File", report_filename_.c_str());
				}
				delete chooser;
			}
			// Create a new file 
			report_file_ = new ofstream(report_filename_, ios::out | ios::trunc);
		}
	}
	if (report_file_) {
		// File did open correctly - write all message to file
		*report_file_ << message;
	}

	// Now add the line to the file viewer
	if (!status_file_viewer_) {
		// Create a file viewer if it doesn't exist
		char title[256];
		snprintf(title, sizeof(title), "%s %s: Status report file: %s", PROGRAM_ID.c_str(), VERSION.c_str(), report_filename_.c_str());
		status_file_viewer_ = new viewer_window(640, 480);
		status_file_viewer_->copy_label(title);
		status_file_viewer_->callback(cb_fv_close, this);
		status_file_viewer_->hide();
	}
	if (status != ST_DEBUG || display_debug_messages_) status_file_viewer_->append(message);

	if (DEBUG_STATUS)	cout << message;

	// Depending on the severity: LOG, NOTE, OK, WARNING, ERROR, SEVERE or FATAL
	// Beep on the last three.
	switch(status) {
	case ST_SEVERE:
		// Open status file viewer and update it.
		status_file_viewer_->show();
		fl_beep(FL_BEEP_ERROR);
		// A severe error - ask the user whether to continue
		if (fl_choice("An error that resulted in reduced functionality occurred:\n%s\n\nDo you want to try to continue or quit?", "Continue", "Quit", nullptr, label, report_filename_.c_str()) == 1) {
			// Set the flag to continue showing the file viewer after all other windows have been hidden.
			close_by_error_ = true;
			main_window_->do_callback();
		}
		break;
	case ST_FATAL:
		// Open status file viewer and update it. Set the flag to keep it displayed after other windows have been hidden
		status_file_viewer_->show();
		fl_beep(FL_BEEP_ERROR);
		// A fatal error - quit the application
		fl_message("An unrecoverable error has occurred, closing down - check status log");
		close_by_error_ = true;
		// Close the application down
		main_window_->do_callback();
		break;
	case ST_ERROR:
		// Open status file viewer and continue
		status_file_viewer_->show();
		fl_beep(FL_BEEP_ERROR);
		break;
	case ST_NOTIFY:
		// Open status file viewer, beep and continue
		status_file_viewer_->show();
		fl_beep(FL_BEEP_NOTIFICATION);
		break;
	default:
		break;
	}
}

// Update file status
void status::file_status(file_status_t status) {
	Fl_Color colour;
	if (READ_ONLY) {
		// set a darker version of the colour if the file is read-only
		colour = fl_darker(FILE_STATUS_COLOURS.at(status));
	}
	else {
		colour = FILE_STATUS_COLOURS.at(status);
	}
	file_status_->color(colour, colour);
	file_status_->redraw();
}

// Text buffer constructor
text_buffer::text_buffer(int requestedSize, int preferredGapSize) :
	Fl_Text_Buffer(requestedSize, preferredGapSize)
{
	// Turn off warning about converting characters to UTF-8
	transcoding_warning_action = nullptr;
}

// Text buffer desctructor
text_buffer::~text_buffer() {
}

// Text display constructor
text_display::text_display(int X, int Y, int W, int H, const char* label) :
	Fl_Text_Display(X, Y, W, H, label)
	, filter_("")
	, status_filter_(ST_NONE)
{
}

// Text display destructor
text_display::~text_display() {
}

// Reload the file keeping the scroll position fixed - default was to start at line 0 again.
void text_display::append(const char* line) {
	// Find current scroll position and the total positions
	int scroll_pos = this->mVScrollBar->value();
	double scroll_max = this->mVScrollBar->maximum();
	// append line to end of buffer if both filters match
	if (filter_.length() == 0 || filter_ == " " || (strlen(line) > 22 + filter_.length()) && (strncmp(line + 22, filter_.c_str(), filter_.length()) == 0)) {
		if (status_filter_ == ST_NONE || line[0] == STATUS_CODES.at(status_filter_)) {
			buffer()->append(line);
		}
	} 
	if (scroll_pos == (int)scroll_max) {
		// We had been scrolled at the end, so scroll to the new end
		scroll((int)this->mVScrollBar->maximum(), 0);
	}
	else {
		// Scroll to the previous scroll position
		scroll(scroll_pos, 0);
	}
}

// Call back for find/find again
// v is a bool: true for find again, false for find new
void viewer_window::cb_find(Fl_Widget* w, void* v) {
	viewer_window* that = ancestor_view<viewer_window>(w);
	bool repeat = (intptr_t)v;
	// Look for the string from the current position
	int pos = that->display_->insert_position();
	int found;
	if (that->direction_) {
		// Find forward
		found = that->display_->buffer()->search_forward(pos, that->search_.c_str(), &pos, that->match_case_);
	}
	else {
		// Find backward start before the current occurrence
		if ((unsigned)pos > that->search_.length()) { pos -= (that->search_.length() + 1); }
		found = that->display_->buffer()->search_backward(pos, that->search_.c_str(), &pos, that->match_case_);
	}
	if (found) {
		// Found a match; select the text and update the position...
		that->display_->buffer()->select(pos, pos + that->search_.length());
		that->display_->insert_position(pos + that->search_.length());
		that->display_->show_insert_position();
	}
	else fl_alert("No occurrences of \'%s\' found!", that->search_.c_str());

}

// Call back for filter
// v is a pointer to display_->filter_ 
void viewer_window::cb_ch_filter(Fl_Widget* w, void* v) {
	cb_choice_text(w, v);
	viewer_window* that = ancestor_view<viewer_window>(w);
	// Reload the text display and set line colours
	text_buffer* buffer = (text_buffer*)that->display_->buffer();
	buffer->remove(0, buffer->length());
	for (auto it = that->original_lines_.begin(); it != that->original_lines_.end(); it++) {
		that->display_->append(*it);
	}
	that->colour_buffer();
}

// Call back for filter
// v is a pointer to display_->status_filter_
void viewer_window::cb_ch_status(Fl_Widget* w, void* v) {
	cb_value_enum<Fl_Choice, status_t>(w, v);
	viewer_window* that = ancestor_view<viewer_window>(w);
	// Reload the text display and set line colours
	text_buffer* buffer = (text_buffer*)that->display_->buffer();
	buffer->remove(0, buffer->length());
	for (auto it = that->original_lines_.begin(); it != that->original_lines_.end(); it++) {
		that->display_->append(*it);
	}
	that->colour_buffer();
}

// Viewer window constructor
viewer_window::viewer_window(int W, int H, const char* label)
	: Fl_Window(W, H, label)
{
	draw_window();
	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences viewer_settings(windows_settings, "Status Viewer");
	int left, top;
	viewer_settings.get("Left", left, 0);
	viewer_settings.get("Top", top, 100);
	position(left, top);
}

// Viewer window destructor
viewer_window::~viewer_window() {
	Fl_Preferences windows_settings(settings_, "Windows");
	Fl_Preferences vw_settings(windows_settings, "Status Viewer");
	vw_settings.set("Left", x_root());
	vw_settings.set("Top", y_root());
}

// Handle FL_HIDE and FL_SHOW to get menu to update otself
int viewer_window::handle(int event) {

	switch (event) {
	case FL_HIDE:
	case FL_SHOW:
		// Get menu to update Windows controls
		menu_->update_windows_items();
		break;
	}

	return Fl_Window::handle(event);
}


// Load the file into the test display in the window
void viewer_window::append(const char* line) {
	display_->append(line);
	colour_buffer();
	original_lines_.push_back(line);
}

// Draw the window
void viewer_window::draw_window() {
	// Create the text buffer
	text_buffer* buffer = new text_buffer;
	// Set values for widgets
	direction_ = 1;
	match_case_ = 1;
	// Find button
	Fl_Button* bn_find = new Fl_Button(GAP, GAP, WBUTTON, HBUTTON, "Find");
	bn_find->callback(cb_find);
	// Search text
	intl_input* ip_search = new intl_input(bn_find->x() + bn_find->w() + GAP + (WLABEL / 2), GAP, WBUTTON, HBUTTON, "Text");
	ip_search->align(FL_ALIGN_LEFT);
	ip_search->value(search_.c_str());
	ip_search->callback(cb_value<intl_input, string>, &search_);
	// Group for radio buttons
	Fl_Group* grp = new Fl_Group(ip_search->x() + ip_search->w() + GAP, GAP, WBUTTON, HBUTTON);
	grp->box(FL_NO_BOX);
	// Backward radiobutton
	Fl_Radio_Round_Button* bn_backward = new Fl_Radio_Round_Button(grp->x(), GAP, WBUTTON / 2, HBUTTON, "@<");
	bn_backward->box(FL_DOWN_BOX);
	bn_backward->value(direction_ == 0);
	radio_param_t* back_param = new radio_param_t(0, &direction_);
	bn_backward->callback(cb_radio, back_param);
	// Forward radio button
	Fl_Radio_Round_Button* bn_forward = new Fl_Radio_Round_Button(grp->x() + bn_backward->w(), GAP, WBUTTON / 2, HBUTTON, "@>");
	bn_forward->box(FL_DOWN_BOX);
	bn_forward->value(direction_ == 1);
	radio_param_t* for_param = new radio_param_t(1, &direction_);
	bn_forward->callback(cb_radio, for_param);
	grp->end();
	// Match case indicator
	Fl_Round_Button* bn_match = new Fl_Round_Button(grp->x() + grp->w() + GAP, GAP, WBUTTON, HBUTTON, "Match Case");
	bn_match->box(FL_DOWN_BOX);
	bn_match->value(match_case_);
	bn_match->selection_color(FL_BLUE);
	bn_match->callback(cb_value < Fl_Round_Button, int >, &match_case_);
	// Filter choice
	Fl_Choice* ch_filter = new Fl_Choice(bn_match->x() + bn_match->w() + GAP + WLABEL, GAP, WSMEDIT, HTEXT, "Topic");
	ch_filter->align(FL_ALIGN_LEFT);
	ch_filter->clear();
	// Add the hard-coded choice values
	ch_filter->add(" ");
	ch_filter->add("ADIF SPEC");
	ch_filter->add("BACKUP");
	ch_filter->add("BAND");
	ch_filter->add("CLUBLOG");
	ch_filter->add("DXATLAS");
	ch_filter->add("EQSL");
	ch_filter->add("EXCEPTION");
	ch_filter->add("EXTRACT");
	ch_filter->add("IMPORT");
	ch_filter->add("INFO");
	ch_filter->add("INTL");
	ch_filter->add("LOG");
	ch_filter->add("LOTW");
	ch_filter->add("MENU");
	ch_filter->add("PREFIX");
	ch_filter->add("PRINTER");
	ch_filter->add("PROGRESS");
	ch_filter->add("QRZ");
	ch_filter->add("RIG");
	ch_filter->add("SCRATCHPAD");
	ch_filter->add("SOCKET");
	ch_filter->add("STATUS");
	ch_filter->add("VALIDATE");
	ch_filter->add("WSJT-X");
	ch_filter->add("ZZALOG");
	// Filter severity
	Fl_Choice* ch_severity = new Fl_Choice(ch_filter->x() + ch_filter->w() + GAP + WLABEL, GAP, WSMEDIT, HTEXT, "Severity");
	ch_severity->align(FL_ALIGN_LEFT);
	ch_severity->clear();
	// Add the hard-coded choice values
	for (int i = (int)ST_NONE; i <= (int)ST_FATAL; i++) {
		ch_severity->add(STATUS_TEXTS.at((status_t)i).c_str());
	}
	ch_severity->value((int)ST_NONE);
	resizable(nullptr);
	size(max(w(), ch_severity->x() + ch_severity->w() + GAP), h());

	// Create the text display and set its parameters
	display_ = new text_display(GAP, grp->y() + grp->h() + GAP, w() - GAP - GAP, h() - grp->y() - grp->h() - GAP - GAP);
	display_->buffer(buffer);
	display_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	display_->textfont(FL_COURIER);
	display_->textsize(12);
	display_->resizable(display_);
	display_->show();
	display_->end();
	ch_filter->callback(cb_ch_filter, &(display_->filter_));
	ch_severity->callback(cb_ch_status, &(display_->status_filter_));

	resizable(display_);
	size_range(w(), h());

	end();
	show();

}

// Callback opens a text browser
// v is unused
void status::cb_bn_misc(Fl_Widget* w, void* v) {
	status* that = ancestor_view<status>(w);
	// Reload the viewer and force the window to be shown - it may have been closed
	if (!that->status_file_viewer_) {
		// Create a file viewer if it doesn't exist
		char* title = new char[that->report_filename_.length() + 30];
		sprintf(title, "Status report file: %s", that->report_filename_.c_str());
		that->status_file_viewer_ = new viewer_window(640, 480, title);
		that->status_file_viewer_->callback(cb_fv_close, that);
	}
	that->status_file_viewer_->show();
	that->status_file_viewer_->redraw();
}

// Format the display depending on the first characer of each line of text
void viewer_window::colour_buffer() {
	// Get the total number of characters in the buffer
	Fl_Text_Buffer* buffer = display_->buffer();
	int num_chars = buffer->length();
	int cur_pos = 0;
	const int num_styles = STATUS_CODES.size();
	// Create a buffer the same number of characters as the buffer to contain the style codes
	char* style = new char[num_chars]; 
	memset(style, 0, num_chars);
	// Generate reverse look-up from status code to status
	map<char, status_t> reverse_codes;
	for (auto pos = STATUS_CODES.begin(); pos != STATUS_CODES.end(); pos++) {
		reverse_codes[pos->second] = pos->first;
	}
	char style_code;
	// For all characters in the buffer
	while (cur_pos < num_chars) {
		// Get the current line
		char* line = buffer->line_text(cur_pos);
		if (strlen(line) > 0) {
			// The line has length - get the message level - first character in the line
			char level = line[0];
			// Update style code if we have a valid line (some outputs have embedded new-lines)
			if (reverse_codes.find(level) != reverse_codes.end()) {
				style_code = reverse_codes.at(level) + 'A';
			}
			// Set the style code for all characters in this like
			for (size_t i = 0; i <= strlen(line); i++) {
				style[cur_pos + i] = style_code;
			}
		}
		else {
			// Default style code for a line with no text (for the new line character) - use previous line's
			style[cur_pos] = style_code;
		}
		// Set position to start of next line
		cur_pos += strlen(line) + 1;
		free(line);
	}
	// Create the style buffer and use it to highlight the data
	Fl_Text_Buffer* style_buffer = new Fl_Text_Buffer;
	style_buffer->text(style);
	display_->highlight_data(style_buffer, STATUS_STYLES, num_styles, 'I', nullptr, nullptr);
	delete[] style;
}

// Return a pointer to the status
Fl_Widget* status::misc_status() {
	return misc_status_;
}

// Set the minimum status severity level that is displayed
void status::min_level(status_t level) {
	min_level_ = level;
	Fl_Preferences status_settings(settings_, "Status");
	status_settings.set("Minimum Level", min_level_);
}

// Get the minimum status severity level that is displayed
status_t status::min_level() {
	return min_level_;
}


// Set the append_log 
void status::append_log(bool append) {
	append_log_ = append;
	Fl_Preferences status_settings(settings_, "Status");
	status_settings.set("Append", (int)append_log_);
}

// Close file viewer 
void status::cb_fv_close(Fl_Widget* w, void* v) {
	status* that = (status*)v;;
	Fl::delete_widget(that->status_file_viewer_);
	that->status_file_viewer_ = nullptr;
}

// Return the file viewer
Fl_Window* status::file_viewer() {
	return status_file_viewer_;
}

// Display debug
void status::display_debug(bool value) {
	display_debug_messages_ = value;
}
bool status::display_debug() {
	return display_debug_messages_;
}

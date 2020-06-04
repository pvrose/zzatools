#include "status.h"

#include "../zzalib/utils.h"
#include "../zzalib/rig_if.h"
#include "../zzalib/callback.h"
#include "menu.h"
#include "intl_widgets.h"
#include "scratchpad.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern rig_if* rig_if_;
extern menu* menu_;
extern void add_rig_if();
extern Fl_Single_Window* main_window_;
extern status* status_;
extern bool read_only_;
//extern void add_sub_window(Fl_Window* w);
//extern void remove_sub_window(Fl_Window* w);
extern scratchpad* scratchpad_;

// Constructor
status::status(int X, int Y, int W, int H, const char* label) :
	Fl_Group(X, Y, W, H, label)
	, clock_bn_(nullptr)
	, progress_(nullptr)
	, rig_status_(nullptr)
	, misc_status_(nullptr)
	, file_status_(nullptr)
	, status_file_viewer_(nullptr)
	, use_local_(false)
	, report_filename_("")
	, report_file_(nullptr)
	, min_level_(ST_NONE)
	, append_log_(false)
	, rig_in_progress_(false)
	, file_unusable_(false)
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

	// Add clock widget - use a button which when clicked switches between UTC and local time
	const int clock_w = rem_w / 4;
	// Button - display the clock
	clock_bn_ = new Fl_Button(curr_x, Y, clock_w, H);
	clock_bn_->labelsize(FONT_SIZE);
	clock_bn_->box(FL_DOWN_BOX);
	clock_bn_->color(FL_BLACK);
	clock_bn_->selection_color(FL_BLACK);
	clock_bn_->labelcolor(use_local_ ? FL_YELLOW : FL_GREEN);
	clock_bn_->labelfont(FL_HELVETICA_BOLD);
	// Get initial time 2018/12/02 14:13:00 UTC +0000, default to UTC at start-up.
	use_local_ = false;
	string clock_text = now(use_local_, "%F %T UTC %z");
	clock_bn_->copy_label(clock_text.c_str());
	// Set callback to switch between UTC and local timezone when clicked
	clock_bn_->callback(cb_bn_clock);
	clock_bn_->when(FL_WHEN_RELEASE);
	clock_bn_->tooltip("Current time (UTC)");
	// Set callback to receive an update every UTC_TIMER (1) second.
	Fl::add_timeout(UTC_TIMER, cb_timer_clock, (void*)clock_bn_);
	add(clock_bn_);

	// Add a progress bar widget - updated by all the processes which take a while
	curr_x += clock_w;
	const int progress_w = rem_w * 2 / 10;
	// Progress bar
	progress_ = new Fl_Progress(curr_x, Y, progress_w, H, "Initialising...");
	progress_->align(FL_ALIGN_INSIDE);
	progress_->labelsize(FONT_SIZE);
	progress_->tooltip("Activity progress");
	progress_->color(FL_WHITE, OBJECT_COLOURS.at(OT_NONE));
	add(progress_);
	curr_x += progress_w;

	// Add rig status
	const int rig_status_w = rem_w / 4;
	// Button - rig status
	rig_status_ = new Fl_Button(curr_x, Y, rig_status_w, H, "Rig connection not yet established");
	rig_status_->labelsize(FONT_SIZE);
	rig_status_->box(FL_DOWN_BOX);
	rig_status_->color(STATUS_COLOURS.at(ST_NONE));
	rig_status_->labelcolor(FL_BLACK);
	rig_status_->labelfont(FONT);
	rig_status_->copy_tooltip(rig_status_->label());
	// Add callback to try and reconnect the rig
	rig_status_->callback(cb_bn_rig);
	rig_status_->when(FL_WHEN_RELEASE);
	add(rig_status_);
	curr_x += rig_status_w;

	// Miscellaneous status - used for general status and error information
	const int misc_w = rem_w * 3 / 10;
	// Button - status message 
	misc_status_ = new Fl_Button(curr_x, Y, misc_w, H, "Miscellaneous information");
	misc_status_->labelsize(FONT_SIZE);
	misc_status_->box(FL_DOWN_BOX);
	misc_status_->color(STATUS_COLOURS.at(ST_NONE));
	misc_status_->labelcolor(FL_BLACK);
	misc_status_->labelfont(FONT);
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
		Fl_File_Chooser* chooser = new Fl_File_Chooser(report_filename_.c_str(), "Text Files(*.txt)\tAll Files (*.*)", Fl_File_Chooser::CREATE, "Select file name for status report");
		chooser->callback(cb_chooser, &report_filename_);
		chooser->textfont(FONT);
		chooser->textsize(FONT_SIZE);
		chooser->show();
		// Wait while the dialog is active (visible)
		while (chooser->visible()) Fl::wait();
		if (chooser->count()) {
			status_settings.set("Report File", report_filename_.c_str());
		}
		delete chooser;
	}

	status_settings.get("Minimum Level", (int&)min_level_, ST_NOTE);
	menu_->status_level(min_level_);
	menu_->append_file(append_log_);

	status_settings.get("Append", (int&)append_log_, false);

	status_file_viewer_ = nullptr;
}

// Destructor
status::~status()
{
	clear();
	delete status_file_viewer_;
	for (auto it = progress_items_.begin(); it != progress_items_.end(); it++) {
		delete (it->second);
	}
	progress_items_.clear();
	progress_stack_.clear();
}

void status::null_file_viewer() {
	delete status_file_viewer_;
	status_file_viewer_ = nullptr;
}

// Callbacks
// Clock button callback - toggles between UTC and local-time
void status::cb_bn_clock(Fl_Widget* bn, void* v) {
	status* that = ancestor_view<status>(bn);
	// Toggle the flag 
	that->use_local_ = that->use_local_ ? false : true;
	// Set label-colour to indicate local or UTC
	bn->labelcolor(that->use_local_ ? FL_YELLOW : FL_GREEN);
	// Redisplay the time
	string clock_text = now(that->use_local_, "%F %T UTC %z");
	bn->copy_label(clock_text.c_str());
	if (that->use_local_) {
		string tz = now(true, "Current time (UTC %z)");
		bn->copy_tooltip(tz.c_str());
	}
	else {
		bn->tooltip("Current time (UTC)");
	}
	bn->redraw();
}

// Clock timer callback - received every UTC_TIMER seconds
void status::cb_timer_clock(void * v) {
	// Update the label in the clock button which is passed as the parameter
	Fl_Button* bn = (Fl_Button*)v;
	status* that = ancestor_view<status>(bn);
	string clock_text = now(that->use_local_, "%F %T UTC %z");
	bn->copy_label(clock_text.c_str());
	bn->redraw();
	// repeat the timer
	Fl::repeat_timeout(UTC_TIMER, cb_timer_clock, v);
}

// Rig status bn callback - attempts to toggle rig connection state
void status::cb_bn_rig(Fl_Widget* bn, void* v) {
	status* that = ancestor_view<status>(bn);

	if (rig_if_ == nullptr || !rig_if_->is_good() || !rig_if_->is_open()) {
		// Rig wasn't present or hadn't connected or had crashed, so try again
		if (!that->rig_in_progress_) {
			that->rig_in_progress_ = true;
			add_rig_if();
			that->rig_in_progress_ = false;
		}
		else {
			that->misc_status(ST_WARNING, "RIG: Already trying to open rig");
		}
	}
	else {
		// Close the rig
		rig_if_->close();
		that->rig_status(ST_WARNING, "Rig closed - assume off-air logging");
		that->misc_status(ST_WARNING, "RIG: Closing reig connection");
		scratchpad_->update();
	}
	bn->redraw();
}

// Add a progress item to the stack
void status::progress(int max_value, object_t object, const char* suffix, bool countdown /*= false*/) {
	// Initialise it
	if (progress_items_.find(object) != progress_items_.end()) {
		char message[100];
		snprintf(message, 100, "PROGRESS: Already started progress for %s", OBJECT_NAMES.at(object));
		misc_status(ST_ERROR, message);
	} else {
		char message[100];
		snprintf(message, 100, "PROGRESS: Starting %s - %d %s", OBJECT_NAMES.at(object), max_value, suffix);
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
		strcpy(item->suffix, suffix);
		progress_items_[object] = item;
		progress_stack_.push_back(object);
		update_progress(object);
	}
}

// Intialise progress bar - 
// maximum value, purpose (used to set colour) and items counted
void status::update_progress(object_t object) {
	if (progress_stack_.size() && progress_stack_.back() == object) {
		progress_item* item = progress_items_.at(object);
		Fl_Color bar_colour = OBJECT_COLOURS.at(object);
		// Set colours
		progress_->color(OBJECT_COLOURS.at(OT_NONE), bar_colour);
		progress_->labelcolor(FL_BLACK);
		char label[100];
		// Update progress 
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
		// redraw and allow scheduler to effect the redrawing
		progress_->redraw();
		Fl::wait();
	}
}

// Update progress bar
void status::progress(int value, object_t object) {
	if (progress_stack_.size()) {
		if (progress_items_.find(object) == progress_items_.end()) {
			char message[100];
			snprintf(message, 100, "PROGRESS: Haven't started progress %s", OBJECT_NAMES.at(object));
			misc_status(ST_ERROR, message);
		} else {
			// Update progress item
			progress_item* item = progress_items_.at(object);
			item->value = value;
			update_progress(object);
			// If it's 100% (or 0% if counting down) delete item and draw the next in the stack - certain objects can overrun
			if ((item->countdown && value <= 0) || (!item->countdown && value >= item->max_value)) {
				char message[100];
				snprintf(message, 100, "PROGRESS: %s finished (%d %s)", OBJECT_NAMES.at(object), item->max_value, item->suffix);
				misc_status(ST_LOG, message);
				delete item;
				progress_items_.erase(object);
				progress_stack_.remove(object);
				if (progress_stack_.size()) {
					update_progress(progress_stack_.back());
				}
			}
		}
	}
}

// Update progress bar witha message - e.g. cancelled and mark 100%
void status::progress(const char* message, object_t object) {
	if (progress_stack_.size()) {
		if (progress_items_.find(object) == progress_items_.end()) {
			char msg[100];
			snprintf(msg, 100, "PROGRESS: Haven't started progress %s", OBJECT_NAMES.at(object));
			misc_status(ST_ERROR, msg);
		}
		else {
			progress_item* item = progress_items_.at(object);
			char msg[100];
			snprintf(msg, 100, "PROGRESS: %s abandoned %s (%d %s)", OBJECT_NAMES.at(object), message, item->value, item->suffix);
			misc_status(ST_LOG, msg);
			if (item->countdown) {
				progress(0, object);
			}
			else {
				progress(item->max_value, object);
			}
			if (message) {
				progress_->copy_label(message);
				progress_->redraw();
			}
		}
		Fl::wait();
	}
}

// Update rig_status - set text and colour
void status::rig_status(status_t status, const char* label) {
	// Set the fixed label and tool tip value. Set the text colour to contrast the button colour
	Fl_Color colour = STATUS_COLOURS.at(status);
	rig_status_->copy_label(label);
	rig_status_->color(colour);
	rig_status_->labelcolor(fl_contrast(FL_BLACK, colour));
	rig_status_->copy_tooltip(label);
	rig_status_->redraw();
	Fl::wait();
}


// Update miscellaneous status - set text and colour
void status::misc_status(status_t status, const char* label) {
	// If we are displaying the message
	if ((int)status >= (int)min_level_ ) {
		// Set the fixed label and tool tip value. Set the text colour to contrast the button colour
		Fl_Color colour = STATUS_COLOURS.at(status);
		misc_status_->copy_label(label);
		misc_status_->color(colour);
		misc_status_->labelcolor(fl_contrast(FL_BLACK, colour));
		misc_status_->copy_tooltip(label);
		misc_status_->redraw();

		Fl::wait();
	}

#ifdef _DEBUG
	// Add a marker to the memory leak report to indicate where we are - deliberately
	// create a leak with the first 15 chars of label in
	char* marker = new char[16];
	strncpy(marker, label, 15);
#endif

	if (!file_unusable_) {
		// Append the status to the file
		// Try to open the file. Open and close it each message
		if (append_log_) {
			report_file_ = new ofstream(report_filename_, ios::out | ios::app);
		}
		else {
			report_file_ = new ofstream(report_filename_, ios::out | ios::trunc);
			append_log_ = true;
		}
		if (!report_file_->good()) {
			delete report_file_;
			report_file_ = nullptr;
			file_unusable_ = true;
			char* message = new char[report_filename_.length() + 50];
			sprintf(message, "STATUS: Failed to open status report file %s", report_filename_.c_str());
			misc_status(ST_ERROR, message);
			delete[] message;
		}


		if (report_file_) {
			string timestamp = now(true, "%Y/%m/%d %H:%M:%S");
			char* message = new char[timestamp.length() + 10 + strlen(label)];
			sprintf(message, "%c %s %s\n", STATUS_CODES.at(status), timestamp.c_str(), label);
			*report_file_ << message;
			delete[] message;
			report_file_->close();
			delete report_file_;
			report_file_ = nullptr;
		}
	}

	switch(status) {
	case ST_SEVERE:
		// A severe error - ask the user whether to continue
		if (fl_choice("An error that resulted in reduced functionality occurred:\n%s\n\nDo you want to try to continue or quit?", "Continue", "Quit", nullptr, label, report_filename_.c_str()) == 1) {
			// Call the exit handler
			Fl::wait();
			// Open status file viewer and update it.
			cb_bn_misc(misc_status_, nullptr);
			//remove_sub_window(status_file_viewer_->top_window());
			main_window_->do_callback();
		}
		break;
	case ST_FATAL:
		// A fatal error - quit the application
		fl_message("An unrecoverable error has occurred, closing down - check status log");
		Fl::wait();
		// Open status file viewer and update it : remove from list to be deleted when closing
		cb_bn_misc(misc_status_, nullptr);
		//remove_sub_window(status_file_viewer_->top_window());

		main_window_->do_callback();
		break;
		// TODO: This causes an exception once the viewer has been deleted but not set to nullptr
	default:
		// Redraw status file viewer
		if (status_file_viewer_) {
			cb_bn_misc(misc_status_, nullptr);
		}
		break;
	}
}

// Update file status
void status::file_status(file_status_t status) {
	Fl_Color colour;
	if (read_only_) {
		// set a darker version of the colour if the file is read-only
		colour = fl_darker(FILE_STATUS_COLOURS.at(status));
	}
	else {
		colour = FILE_STATUS_COLOURS.at(status);
	}
	file_status_->color(colour, colour);
	file_status_->redraw();
	Fl::wait();
}

// Text display constructor
text_display::text_display(int X, int Y, int W, int H, const char* label) :
	Fl_Text_Display(X, Y, W, H, label)
	, filter_("")
{

}

text_display::~text_display() {
}

// Reload the file keeping the scroll position fixed - default was to start at line 0 again.
void text_display::load(const char* filename) {
	// Find current scroll position
	int scroll_pos = this->mVScrollBar->value();
	buffer()->remove(0, buffer()->length());
	ifstream* file = new ifstream(filename, ios_base::in);
	while (file->good()) {
		string line;
		getline(*file, line);
		if (filter_.length() == 0 || filter_ == " " ||
			((line.length() > 22 + filter_.length()) && (line.substr(22, filter_.length()) == filter_))) {
			buffer()->append(line.c_str());
			buffer()->append("\n");
		}
	}
	file->close();
	delete file;
	scroll(scroll_pos, 0);
}

// Call back for find/find again
void viewer_window::cb_find(Fl_Widget* w, void* v) {
	viewer_window* that = ancestor_view<viewer_window>(w);
	bool repeat = (long)v;
	// Look for the string from the current position
	int pos = that->display_->insert_position();
	int found;
	if (that->direction_) {
		found = that->display_->buffer()->search_forward(pos, that->search_.c_str(), &pos, that->match_case_);
	}
	else {
		if ((unsigned)pos > that->search_.length()) { pos -= (that->search_.length() + 1); }
		found = that->display_->buffer()->search_backward(pos, that->search_.c_str(), &pos, that->match_case_);
	}
	if (found) {
		// Found a match; select and update the position...
		that->display_->buffer()->select(pos, pos + that->search_.length());
		that->display_->insert_position(pos + that->search_.length());
		that->display_->show_insert_position();
	}
	else fl_alert("No occurrences of \'%s\' found!", that->search_.c_str());

}

// Call back for filter
void viewer_window::cb_ch_filter(Fl_Widget* w, void* v) {
	cb_choice_text(w, v);
	viewer_window* that = ancestor_view<viewer_window>(w);
	that->display_->load(that->filename_);
	that->colour_buffer();
}


viewer_window::viewer_window(int W, int H, const char* label)
	: Fl_Window(W, H, label)
	, filename_(nullptr)
{
	draw_window();
}

viewer_window::~viewer_window() {
	delete[] filename_;
}

void viewer_window::load(const char* filename) {
	display_->load(filename);
	if (filename_) delete[] filename_;
	filename_ = new char[strlen(filename) + 1];
	strcpy(filename_, filename);
}

void viewer_window::draw_window() {
	// now read it into the text buffer
	Fl_Text_Buffer* buffer = new Fl_Text_Buffer;
	// And display it
	direction_ = 1;
	match_case_ = 1;
	Fl_Button* bn_find = new Fl_Button(GAP, GAP, WBUTTON, HBUTTON, "Find");
	bn_find->labelfont(FONT);
	bn_find->labelsize(FONT_SIZE);
	bn_find->callback(cb_find);
	intl_input* ip_search = new intl_input(bn_find->x() + bn_find->w() + GAP + (WLABEL / 2), GAP, WBUTTON, HBUTTON, "Text");
	ip_search->labelfont(FONT);
	ip_search->labelsize(FONT_SIZE);
	ip_search->textfont(FONT);
	ip_search->textsize(FONT_SIZE);
	ip_search->align(FL_ALIGN_LEFT);
	ip_search->value(search_.c_str());
	ip_search->callback(cb_value<intl_input, string>, &search_);
	Fl_Group* grp = new Fl_Group(ip_search->x() + ip_search->w() + GAP, GAP, WBUTTON, HBUTTON);
	grp->box(FL_NO_BOX);
	Fl_Radio_Round_Button* bn_backward = new Fl_Radio_Round_Button(grp->x(), GAP, WBUTTON / 2, HBUTTON, "@<");
	bn_backward->labelsize(FONT_SIZE);
	bn_backward->box(FL_DOWN_BOX);
	bn_backward->value(direction_ == 0);
	radio_param_t* back_param = new radio_param_t(0, &direction_);
	bn_backward->callback(cb_radio, back_param);
	Fl_Radio_Round_Button* bn_forward = new Fl_Radio_Round_Button(grp->x() + bn_backward->w(), GAP, WBUTTON / 2, HBUTTON, "@>");
	bn_forward->labelsize(FONT_SIZE);
	bn_forward->box(FL_DOWN_BOX);
	bn_forward->value(direction_ == 1);
	radio_param_t* for_param = new radio_param_t(1, &direction_);
	bn_forward->callback(cb_radio, for_param);
	grp->end();

	Fl_Round_Button* bn_match = new Fl_Round_Button(grp->x() + grp->w() + GAP, GAP, WBUTTON, HBUTTON, "Match Case");
	bn_match->box(FL_DOWN_BOX);
	bn_match->labelsize(FONT_SIZE);
	bn_match->labelfont(FONT);
	bn_match->value(match_case_);
	bn_match->selection_color(FL_BLUE);
	bn_match->callback(cb_value < Fl_Round_Button, int >, &match_case_);

	Fl_Choice* ch_filter = new Fl_Choice(bn_match->x() + bn_match->w() + GAP + WLABEL, GAP, WEDIT, HTEXT, "Filter");
	ch_filter->labelfont(FONT);
	ch_filter->labelsize(FONT_SIZE);
	ch_filter->textfont(FONT);
	ch_filter->textsize(FONT_SIZE);
	ch_filter->align(FL_ALIGN_LEFT);
	ch_filter->clear();
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
	ch_filter->add("STATUS");
	ch_filter->add("VALIDATE");
	ch_filter->add("ZZALOG");

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

	end();
	show();

}

// Callback opens a text browser
void status::cb_bn_misc(Fl_Widget* w, void* v) {
	status* that = ancestor_view<status>(w);
	if (!that->status_file_viewer_) {
		char* title = new char[that->report_filename_.length() + 30];
		sprintf(title, "Status report file: %s", that->report_filename_.c_str());
		that->status_file_viewer_ = new viewer_window(640, 480, title);
	}
	// Reload the viewer and force the window to be shown - it may have been closed
	that->status_file_viewer_->load(that->report_filename_.c_str());
	that->status_file_viewer_->show();
	that->status_file_viewer_->colour_buffer();
	that->status_file_viewer_->redraw();
}

// Format the display depending on the first characer of each line of text
void viewer_window::colour_buffer() {
	Fl_Text_Buffer* buffer = display_->buffer();
	int num_chars = buffer->length();
	int cur_pos = 0;
	const int num_styles = STATUS_CODES.size();
	char* style = new char[num_chars];
	memset(style, 0, num_chars);
	// Generate reverse look-up from status code to status
	map<char, status_t> reverse_codes;
	for (auto pos = STATUS_CODES.begin(); pos != STATUS_CODES.end(); pos++) {
		reverse_codes[pos->second] = pos->first;
	}
	char style_code;
	while (cur_pos < num_chars) {
		char* line = buffer->line_text(cur_pos);
		if (strlen(line) > 0) {
			char level = line[0];
			// Update style code if we have a valid line (some outputs have embedded new-lines)
			if (reverse_codes.find(level) != reverse_codes.end()) {
				style_code = reverse_codes.at(level) + 'A';
			}
			for (size_t i = 0; i <= strlen(line); i++) {
				style[cur_pos + i] = style_code;
			}
		}
		else {
			style[cur_pos] = 'A';
		}
		cur_pos += strlen(line) + 1;
		free(line);
	}
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

// Get the minimum status severtity level that is dosplayed
status_t status::min_level() {
	return min_level_;
}

// Status log display is closed
void status::cb_text(Fl_Widget* w, void* v) {
	//remove_sub_window((Fl_Window*)w);
}

// Set the append_log
void status::append_log(bool append) {
	append_log_ = append;
	Fl_Preferences status_settings(settings_, "Status");
	status_settings.set("Append", (int)append_log_);
}


#include "band_view.h"
#include "status.h"
#include "../zzalib/utils.h"
#include "../zzalib/callback.h"
#include "drawing.h"
#include "spec_data.h"
#include "menu.h"
#include "book.h"

#include <fstream>
#include <istream>
#include <cmath>
#include <map>

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Tooltip.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern status* status_;
extern spec_data* spec_data_;
extern menu* menu_;
extern book* book_;

// Constructor - calls the Window constructor 
band_view::band_view(double frequency, int W, int H, const char* label) :
	Fl_Window(W, H, label),
	frequency_(frequency),
	rig_frequency_(frequency),
	tracking_rig_(true),
	valid_(true)
{
	// Load data
	if (load_data()) {
		// load is successful
		// Resize window to its previous size and position
		Fl_Preferences band_settings(settings_, "Band View");
		int w;
		int h;
		int x;
		int y;
		band_settings.get("Width", w, 400);
		band_settings.get("Height", h, 150);
		band_settings.get("Top", y, 0);
		band_settings.get("Left", x, 0);
		if (x != 0 || y != 0) {
			position(x, y);
		}
		size(w, h);
		// Get the min and max frequency 
		min_frequency_ = entries_.front()->lower;
		max_frequency_ = entries_.back()->upper;

		// Carry on and create window - default 0.1 kHz
		band_settings.get("Zoom value", zoom_value_, 3);
		create_form();
		// Recalculate zoom values and left and right frequencies
		recalculate(rig_frequency_);
		// Set frequency slider value and step values
		((Fl_Counter*)w_freq_slider_)->value(frequency_);
		((Fl_Counter*)w_freq_slider_)->step(khz_per_major_, step_value_);
		// Display zoom value as cycles per tick.
		char temp[128];
		sprintf(temp, "%g kHz", khz_per_minor_);
		((Fl_Output*)w_cycles_per_)->value(temp);
		show();
		redraw();
	}
	else {
		valid_ = false;
	}
}

// Destructor
band_view::~band_view()
{
	for (auto it = entries_.begin(); it != entries_.end(); it++) {
		delete *it;
	}
	entries_.clear();
}

// Read the band plan data
bool band_view::load_data() {
	string filename = get_path() + "band_plan.tsv";
	ifstream file;
	file.open(filename.c_str(), fstream::in);
	// calculate the file size
	streampos startpos = file.tellg();
	file.seekg(0, ios::end);
	streampos currpos = file.tellg();
	int length = (int)(currpos - startpos);
	// reposition back to beginning
	file.seekg(0, ios::beg);
	// Initialise progress
	status_->misc_status(ST_NOTE, ("BAND: Loading band-plan data"));
	status_->progress((int)(currpos - startpos), OT_BAND, "Reading band-plan file", "bytes");
	// Read and ignore first line
	string line;
	getline(file, line);
	currpos = file.tellg();
	status_->progress((int)(currpos - startpos), OT_BAND);
	int mode_ix = 1;
	int bw_ix = 1;
	mode_palette_.clear();
	bw_palette_.clear();
	while (file.good()) {
		// Read the line and parse it
		getline(file, line);
		currpos = file.tellg();
		if (file.good()) {
			// Update progress bar
			status_->progress((int)(currpos - startpos), OT_BAND);
			// Read and decode the entry and add to the end
			band_entry_t* entry = get_entry(line);
			entries_.push_back(entry);
			// Add a colour to the mode display palette
			if (mode_palette_.find(entry->mode) == mode_palette_.end()) {
				mode_palette_[entry->mode] = mode_ix;
				mode_ix += 1;
			}
			// Add a colour to the bandwidth display palette
			if (bw_palette_.find(entry->bandwidth) == bw_palette_.end()) {
				bw_palette_[entry->bandwidth] = bw_ix;
				bw_ix += 1;
			}
		}
	}
	// Return success or fail
	if (file.eof()) {
		status_->misc_status(ST_OK, "BAND: Loaded band-plan data");
		return true;
	}
	else {
		status_->misc_status(ST_ERROR, "BAND: Load band-plan data failed");
		status_->progress("Load failed!", OT_BAND);
		return false;
	}
}

// Decode an entry
band_view::band_entry_t* band_view::get_entry(string line) {
	vector<string> words;
	split_line(line, words, '\t');
	// Create the entry
	band_entry_t* result = new band_entry_t;
	// Lower->Upper->Bandwidth->Mode->Notes
	// First entry lower bound or spot entry
	result->lower = stod(words[0]);
	// Second entry upper bound - if no entry then it's a spot frequency entry
	result->is_spot = false;
	try {
		result->upper = stod(words[1]);
	} 
	catch (exception&) {
		result->is_spot = true;
	}
	// Third entry is bandwidth
	try {
		result->bandwidth = stod(words[2]);
	} 
	catch (exception&) {
		result->bandwidth = 0.0;
	}
	// Fourth is mode
	result->mode = words[3];
	// Fifth is notes
	result->notes = words[4];
	return result;
}

// Get the directory of the reference files
string band_view::get_path() {
	// get the datapath settings group.
	Fl_Preferences datapath(settings_, "Datapath");
	char *dirname = nullptr;
	string directory_name;
	// get the value from settings or force new browse
	if (!datapath.get("Reference", dirname, "")) {
		// We do not have one - so open chooser to get one
//		Fl_File_Chooser* chooser = new Fl_File_Chooser(dirname, nullptr, Fl_File_Chooser::DIRECTORY,
//			"Select reference file directory");
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->title("Select reference file directory");
		chooser->preset_file(dirname);
		while (chooser->show()) {}
		directory_name = chooser->filename();
		delete chooser;
	}
	else {
		directory_name = dirname;
	}
	// Append a foreslash if one is not present
	if (directory_name.back() != '/') {
		directory_name.append(1, '/');
	}
	// Free any memory used
	if (dirname) free(dirname);
	return directory_name;

}

// Update the frequency
void band_view::update(double frequency) {
	rig_frequency_ = frequency;
	if (tracking_rig_) {
		frequency_ = rig_frequency_;
		recalculate(frequency);
		((Fl_Counter*)w_freq_slider_)->value(frequency_);
	}
	redraw();
}

// Update the frequency from the specified record
void band_view::update(record_num_t record_num) {
	if (record_num >= 0 && record_num < book_->size()) {
		record* this_record = book_->get_record(record_num, false);
		double frequency;
		this_record->item("FREQ", frequency);
		update(frequency * 1000.0);
	}
}

// Create the form view
void band_view::create_form() {
	// Zoom value slider
	Fl_Slider* val1 = new Fl_Slider(EDGE, EDGE, WRADIO, h() - (2 * EDGE) - HBUTTON);
	val1->labelfont(FONT);
	val1->labelsize(FONT_SIZE);
	val1->type(FL_VERTICAL);
	val1->callback(cb_roll_zoom, (void*)&zoom_value_);
	val1->tooltip("Zoom in or out");
	val1->bounds(0, (double)NUM_ZOOMS - 1);
	val1->step(1);
	val1->value(zoom_value_);

	int curr_y = val1->y() + val1->h();

	// Zoom value display (converted to cycles per tick)
	Fl_Output* out1 = new Fl_Output(EDGE, curr_y, WBUTTON, HBUTTON, "Zoom");
	out1->textfont(FONT);
	out1->textsize(FONT_SIZE);
	out1->labelfont(FONT);
	out1->labelsize(FONT_SIZE);
	out1->align(FL_ALIGN_TOP | FL_ALIGN_RIGHT);
	out1->type(FL_NORMAL_COUNTER);
	out1->tooltip("Displays cycles per tick for selected zoom");
	char temp[128];
	sprintf(temp, "%g kHz", khz_per_minor_);
	out1->value(temp);
	w_cycles_per_ = out1;

	pixel_lhs_ = val1->x() + val1->w() + GAP;

	int val2_x = out1->x() + out1->w() + GAP;
	// Frequency value slider
	Fl_Counter* val2 = new Fl_Counter(val2_x, curr_y, 180, HBUTTON, "Frequency");
	val2->textfont(FONT);
	val2->textsize(FONT_SIZE);
	val2->labelfont(FONT);
	val2->labelsize(FONT_SIZE);
	val2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	val2->type(0);
	val2->callback(cb_redraw_value, (void*)&frequency_);
	val2->tooltip("Scroll frequency");
	val2->bounds(min_frequency_, max_frequency_);
	w_freq_slider_ = val2;

	int bn1_x = val2->x() + val2->w() + GAP;
	// Restore to rig frequency button
	Fl_Button* bn1 = new Fl_Button(bn1_x, curr_y, WBUTTON, HBUTTON, "Rig");
	bn1->labelsize(FONT_SIZE);
	bn1->labelfont(FONT);
	bn1->callback(cb_bn_reset);
	bn1->tooltip("Restore frequency to rig value");
	bn1->color(fl_lighter(FL_RED));

	// Get the even number of ticks that fit in what remains
	num_ticks_ = (w() - EDGE - val1->w() - GAP - EDGE) / PIXELS_PER_TICK / 2 * 2;
	pixel_rhs_ = pixel_lhs_ + (num_ticks_ * PIXELS_PER_TICK);
}

// Convert frequency to text
void band_view::freqy_to_text(double frequency, char* text) {
	switch ((int)(khz_per_major_ * 1000)) {
	case 10:
		// 10 Hz per tick - display as x.xx kHz
		sprintf(text, "%0.2f", frequency);
		break;
	case 100:
		// 100 Hz per tick - display as x.x kHz
		sprintf(text, "%0.1f", frequency);
		break;
	case 1000:
	case 10000:
	case 100000:
	case 1000000:
		// 1KHz or greater - display as kHz.
		sprintf(text, "%0.0f", frequency);
		break;
	}
}

// Draw the frequency scale 
int band_view::draw_scale() {
	// Drawing constants - lengths of large and small ticks
	const int LTICK = 20;
	const int STICK = 10;
	// Draw the rest of the window
	Fl_Window::draw();
	double frequency = freqy_lhs_;
	// Get how much space the frequency text requires - note text is drawn at 90 degrees
	fl_font(FONT, FONT_SIZE - 2);
	int text_w = 0;
	int text_h = 0;
	char text[20];
	freqy_to_text(frequency_, text);
	fl_measure(text, text_w, text_h);
	// now locate the y positions of the various elements
	int text_y = text_w + GAP;
	int ltick_y = text_y + 5;
	int stick_y = ltick_y + LTICK - STICK;
	int horiz_y = stick_y + STICK;
	// Draw horizontal line
	fl_line(pixel_lhs_, horiz_y, pixel_rhs_, horiz_y);
	// Draw the ticks
	for (int curr_x = pixel_lhs_; curr_x <= pixel_rhs_; curr_x +=PIXELS_PER_TICK) {
		// See if we draw a major or minor tick
		if ((long long)(frequency * 1000) % (int)(khz_per_major_ * 1000) == 0) {
			// major tick - long tick and draw label
			if (abs(frequency - rig_frequency_) >= khz_per_minor_) {
				freqy_to_text(frequency, text);
				fl_draw(90, text, curr_x, text_y);
			}
			fl_line(curr_x, ltick_y, curr_x, horiz_y);
		}
		else {
			// minor tick - short tick only
			fl_line(curr_x, stick_y, curr_x, horiz_y);
		}
		frequency += khz_per_minor_;
	}
	// Return the current y position to draw next item
	return horiz_y + 5;
}

// Draw a red or blue line for the rig frequency
int band_view::draw_frequency(int pos_y) {
	// now draw a bigger tick at the frequency
	// Work out where to draw it - get number of pixels away fromm mid-point.
	int curr_x = x_pos(rig_frequency_);
	char text[128];
	sprintf(text, "%.3f", rig_frequency_);
	int text_w = 0;
	int text_h = 0;
	fl_font(FONT, FONT_SIZE - 2);
	fl_measure(text, text_w, text_h);
	Fl_Color old_colour = fl_color();
	if (in_band(rig_frequency_)) {
		fl_color(FL_BLUE);
		((Fl_Counter*)w_freq_slider_)->textcolor(FL_BLACK);
		((Fl_Counter*)w_freq_slider_)->textfont(FONT);
	}
	else {
		fl_color(FL_RED);
		((Fl_Counter*)w_freq_slider_)->textcolor(FL_RED);
		((Fl_Counter*)w_freq_slider_)->textfont(FONT | FL_BOLD);
	}
	// Draw a red or blue line across scale and plans
	fl_line(curr_x, GAP, curr_x, pos_y);
	// Write current frequency on line top-aligned
	fl_draw(90, text, curr_x - 1, GAP + text_w);
	fl_color(old_colour);
	return pos_y + GAP;
}

// Draw the band-plan elements
int band_view::draw_plan(int curr_y) {
	const int SIZE_BAR = 20;
	fl_font(FONT, FONT_SIZE);
	bool complete = false;
	// Set y values
	int top_mode_y = curr_y;
	int top_band_y = top_mode_y + SIZE_BAR + 1;
	int bot_band_y = top_band_y + SIZE_BAR;
	// Remember these for mouse use
	pixel_top_ = curr_y;
	pixel_bottom_ = bot_band_y;

	// For all the entries
	for (auto it = entries_.begin(); it != entries_.end() && !complete; it++) {
		// If some part of the entry lies within display draw it.
		if (!(*it)->is_spot && (*it)->lower <= freqy_rhs_ && (*it)->upper >= freqy_lhs_) {
			// Draw the item
			int box_x = 0;
			int box_w = 0;
			if ((*it)->lower < freqy_lhs_) {
				// Overlaps lh edge
				box_x = pixel_lhs_;
			}
			else {
				// Starts within dislay
				box_x = x_pos((*it)->lower);
			}
			if ((*it)->upper > freqy_rhs_) {
				// Overlaps rh edge
				box_w = pixel_rhs_ - box_x;
			}
			else {
				// Ends within display
				box_w = x_pos((*it)->upper) - box_x;
			}
			// draw the mode bar - set colours
			Fl_Color box_colour = ZLG_PALETTE[mode_palette_[(*it)->mode]];
			Fl_Color text_colour = fl_contrast(FL_BLACK, box_colour);
			fl_draw_box(FL_BORDER_BOX, box_x, top_mode_y, box_w, SIZE_BAR, box_colour);
			// Add the allowed modes as text
			fl_color(text_colour);
			fl_draw((*it)->mode.c_str(), box_x, top_mode_y, box_w, SIZE_BAR, FL_ALIGN_CENTER);
			// draw the bandwidth bar - set colours
			box_colour = ZLG_PALETTE[bw_palette_[(*it)->bandwidth]];
			text_colour = fl_contrast(FL_BLACK, box_colour);
			fl_draw_box(FL_BORDER_BOX, box_x, top_band_y, box_w, SIZE_BAR, box_colour);
			fl_color(text_colour);
			char text[10];
			// Add the bandwidth as text
			sprintf(text, "%0.1f", (*it)->bandwidth);
			fl_draw(text, box_x, top_band_y, box_w, SIZE_BAR, FL_ALIGN_CENTER);
		}
		else if ((*it)->lower > freqy_rhs_) {
			// Passed the right hand edge
			complete = true;
		}
	}
	// Return where the next item is to be drawn
	return bot_band_y + 5;
}

// Draw the non-widget itsms
void band_view::draw() {
	int pos_y = draw_scale();
	pos_y = draw_plan(pos_y);
	pos_y = draw_frequency(pos_y);
	// Don't allow the window to be resized to smaller than this
	size_range(pixel_rhs_ + GAP, pos_y + GAP);
	// Get band
	string band = spec_data_->band_for_freq(rig_frequency_ / 1000.0);
	string title = "Band plan " + band;
	copy_label(title.c_str());
}

// Convert frequency to a x-pixel position
int band_view::x_pos(double frequency) {
	double offset = frequency - freqy_lhs_;
	// Multiply first by pixels_per_tick then divide by khz_per_tick to avoid truncation error
	return (int)(offset * (double)PIXELS_PER_TICK / khz_per_minor_) + pixel_lhs_;
}

// Zoom slider callback
void band_view::cb_roll_zoom(Fl_Widget* w, void* v) {
	band_view* that = ancestor_view<band_view>(w);
	// Get the value of the slider
	double value;
	cb_value<Fl_Slider, double>(w, &value);
	// Recalculate all values
	that->zoom_value_ = (int)value;
	that->recalculate(that->frequency_);
	// Display zoom value in cycles per tick
	char temp[128];
	sprintf(temp, "%g kHz", that->khz_per_minor_);
	((Fl_Output*)that->w_cycles_per_)->value(temp);
	// Change frequency step values
	((Fl_Counter*)that->w_freq_slider_)->step(that->khz_per_minor_, that->step_value_);
	// redraw
	that->redraw();
	// Save new value
	Fl_Preferences band_settings(settings_, "Band View");
	band_settings.set("Zoom value", that->zoom_value_);
}

// Frequency counter callback
void band_view::cb_redraw_value(Fl_Widget* w, void* v) {
	band_view* that = ancestor_view<band_view>(w);
	// Get the new frequency
	double value;
	cb_value<Fl_Counter, double>(w, &value);
	// Disconnect from rig
	that->tracking_rig_ = false;
	// Recalculate frequency bounds
	that->recalculate(value);
	that->redraw();
}

// Restore rig frequency button callback
void band_view::cb_bn_reset(Fl_Widget* w, void* v) {
	band_view* that = ancestor_view<band_view>(w);
	// Reconnect and restore frequencies
	that->tracking_rig_ = true;
	that->frequency_ = that->rig_frequency_;
	that->recalculate(that->frequency_);
	((Fl_Counter*)that->w_freq_slider_)->value(that->frequency_);
	that->redraw();
}

// Recalculate the bounds and steps
void band_view::recalculate(double frequency) {
	bool found = false;
	// New zoom value
	khz_per_minor_ = POSS_MINOR[zoom_value_];
	// Get the appropriate major tick separation
	khz_per_major_ = POSS_MAJOR[zoom_value_ / 3];
	// Set the frequency roller step value
	step_value_ = khz_per_minor_ * (((long)num_ticks_) - 1);
	// Calculate mid, left and right frequencies
	// Round mid_frequency_ to nearest multiple of khz_per_minor_
	mid_frequency_ = round(frequency / khz_per_minor_) * khz_per_minor_;
	double delta_freq = (double)num_ticks_ * khz_per_minor_ / 2;
	freqy_lhs_ = mid_frequency_ - delta_freq;
	freqy_rhs_ = mid_frequency_ + delta_freq;
}

// Track mouse events
int band_view::handle(int event) {
	int curr_x = Fl::event_x();
	int curr_y = Fl::event_y();
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		// Tell FL that we are handling mouse events
		return true;
	case FL_PUSH:
		// Track mouse moves - the intention was just on mouse over but that created too many tips to display
		if (curr_x > pixel_lhs_ && curr_x < pixel_rhs_ && curr_y > pixel_top_ && curr_y < pixel_bottom_) {
			// Pixel is over a band - display the notes in a tip over the mouse
			display_info(x_frequency(curr_x), Fl::event_x_root(), Fl::event_y_root());
			return true;
		}
	case FL_HIDE:
	case FL_SHOW:
		menu_->update_windows_items();
		return true;
	}
	return Fl_Window::handle(event);
}

// Display band notes for the frequency at the mouse point
void band_view::display_info(double frequency, int x, int y) {
	bool found = false;
	Fl_Window* tw = nullptr;
	for (auto it = entries_.begin(); it != entries_.end() && !found; it++) {
		// Scan the entries
		if (frequency >= (*it)->lower && frequency <= (*it)->upper) {
			// Found the band for the frequency
			found = true;
			// Generate tip
			char* tip = new char[100 + (*it)->notes.length()];
			sprintf(tip, "Lower: %g kHz\nUpper: %g kHz\n%s", (*it)->lower, (*it)->upper, (*it)->notes.c_str());
			// Display the tip
			tw = tip_window(string(tip), x, y);
		}
	}
	if (!found) {
		// Frequency is not in a band - say so!
		tw = tip_window("Out of band", x, y);
	}
	// Set the timeout on the tooltip
	Fl::add_timeout(Fl_Tooltip::delay(), cb_timer_tip, tw);
}

// Convert the x-position to a frequency
double band_view::x_frequency(int x) {
	// Get the pixel offset from the LH edge
	int pixels = x - pixel_lhs_;
	// Convert it to numbers of ticks
	double ticks = (double)pixels / (double)PIXELS_PER_TICK;
	// Andf convert that to a frequency offset and add LH edge frequency
	double result = (ticks * khz_per_minor_) + freqy_lhs_;
	return result;
}

// Initialised OK
bool band_view::valid() { return valid_; }

// Is the supplied frequency in a band
bool band_view::in_band(double frequency) {
	bool found = false;
	bool result = false;
	for (auto it = entries_.begin(); it != entries_.end() && !found; it++) {
		if ((*it)->lower <= frequency) {
			if ((*it)->upper > frequency) {
				result = true;
				found = true;
			}
		}
		else {
			found = true;
		}
 	}
	return result;
}

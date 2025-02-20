#include "band_widget.h"

#include "spec_data.h"
#include "band_data.h"
#include "status.h"
#include "drawing.h"

#include <cmath>
#include <string>

#include <FL/fl_draw.H>

using namespace std;

extern spec_data* spec_data_;
extern band_data* band_data_;
extern status* status_;

const map<string, Fl_Color> MODE_COLOURS = {
	{ "CW", FL_GREEN },
	{ "Dig", FL_YELLOW },
	{ "Phone", FL_BLUE },
	{ "Guard", COLOUR_ORANGE },
	{ "Beacons", FL_RED },
	{ "FM", FL_CYAN },
	{ "DV", FL_MAGENTA }
};
const int BAR_WIDTH = 10;
// Use #define so it's easier to concatenate with other formats
#define FREQ_FORMAT "%0.3f"

band_widget::band_widget(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L) {
	Fl_Widget::type(BAND_FULL);
	value_ = nan("");
	Fl_Widget::color(FL_FOREGROUND_COLOR, FL_DARK_GREEN);
	band_ = "";
	band_range_ = { nan(""), nan("") };
	redraw();
}

band_widget::~band_widget() {

}

void band_widget::draw() {
	// Rescale as w and h may have changed
	rescale();
	reset_markers();
	adjust_markers();
	// Wipe the background clean
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(x(), y(), w(), h());
	// Draw the outline box
	draw_box();
	// Draw the valrious items
	if (band_.length()) {
		draw_scale(band_range_);
		draw_modebars();
		draw_markers();
	}
	draw_legend();
}

// intercept the mouse click to invoke the callback
int band_widget::handle(int event) {
	switch (event) {
	case FL_RELEASE: {
		switch (Fl::event_button()) {
		case FL_LEFT_MOUSE: {
			if ((when() & FL_WHEN_RELEASE) == FL_WHEN_RELEASE) do_callback();
			return true;
		}
		}
		break;
	}
	}
	return Fl_Widget::handle(event);
}

// Value - frequency
void band_widget::value(double f) {
	value_ = f;
	generate_data(value_);
	redraw();
}

double band_widget::value() {
	return value_;
}

// Draw the scale (from lower to upper +/- a bit)
void band_widget::draw_scale(range_t range) {
	if (band_.length()) {
		// Draw the axis
		fl_font(0, FL_NORMAL_SIZE);
		fl_color(FL_FOREGROUND_COLOR);
		fl_line(x_scale_, y_upper_, x_scale_, y_lower_);
		int curr_y;
		char text[15];
		double f = scale_range_.upper;
		double e0 = 0.00001;
		double e1 = major_tick_ - e0;
		while (f >= scale_range_.lower) {
			curr_y = y_for_f(f);
			double df = fmod(f, major_tick_);
			if (df < e0 || df > e1) {
				// Close enough to a major tick - draw the tick
				fl_line(x_major_, curr_y, x_scale_, curr_y);
				// Add the freq
				snprintf(text, sizeof(text), FREQ_FORMAT, f);
				fl_draw(text, x_freq_, curr_y - h_offset_, w_freq_, 2 * h_offset_, FL_ALIGN_RIGHT);
			}
			else {
				// draw a minor tick only
				fl_line(x_minor_, curr_y, x_scale_, curr_y);
			}
			f -= minor_tick_;
		}
	}
	else {
		fl_font(FL_BOLD, FL_NORMAL_SIZE * 2);
		int wx = 0, hx = 0;
		fl_measure("Out of Band", wx, hx);
		fl_color(FL_RED);
		fl_draw("Out of Band", x() + w() / 2 - wx / 2, y() + h() / 2 + hx / 2);
	}

}

// Draw the modebars
void band_widget::draw_markers() {
	for (int ix = 0; ix < markers_.size(); ix++) {
		marker& m = markers_[ix];
		switch (m.type) {
		case SUBBAND_UPPER:
			fl_color(FL_FOREGROUND_COLOR);
			draw_line(m.y_scale, m.y_text, FL_DOT);
			fl_draw(m.text, x_text_, m.y_text + h_offset_);
			break;
		case SUBBAND_LOWER:
			fl_color(FL_FOREGROUND_COLOR);
			draw_line(m.y_scale, m.y_text, FL_DOT);
			break;
		case CURRENT:
			fl_color(Fl_Widget::selection_color());
			draw_line(m.y_scale, m.y_text, 0);
			break;
		case SPOT:
			if (!ignore_spots_) {
				fl_color(FL_FOREGROUND_COLOR);
				draw_line(m.y_scale, m.y_text, 0);
				fl_draw(m.text, x_text_, m.y_text + h_offset_);
			}
		}
	}
}

// Draw single offset line
void band_widget::draw_line(int yl, int yr, int style) {
	fl_line_style(style);
	if (yl == yr) {
		// Draw single line
		fl_line(x_scale_, yl, x_text_, yr);
	}
	else {
		// Draw three lines
		fl_line(x_scale_, yl, x_kink1_, yl);
		fl_line(x_kink1_, yl, x_kink2_, yr);
		fl_line(x_kink2_, yr, x_text_, yr);
	}
	fl_line_style(0);
}

// Draw the modebars
void band_widget::draw_modebars() {
	for (int ix = 0; ix < mode_bars_.size(); ix++) {
		mode_bar& m = mode_bars_[ix];
		int yu = y_for_f(m.range.upper);
		int yl = y_for_f(m.range.lower);
		int bar_num = modes_.at(m.mode);
		int xl = x_scale_ + (bar_num * BAR_WIDTH);
		fl_color(MODE_COLOURS.at(m.mode));
		fl_rectf(xl, yu, BAR_WIDTH, (yl - yu));
	}
}

// Draw legend
void band_widget::draw_legend() {
	fl_font(FL_BOLD, FL_NORMAL_SIZE + 2);
	int wl = 0, hl = 0;
	if (band_.length() == 0) {
		fl_measure("OUT OF BAND!", wl, hl);
		fl_color(FL_RED);
	}
	else {
		fl_measure(band_.c_str(), wl, hl);
		fl_color(FL_FOREGROUND_COLOR);
	}
	int yt = y_lower_ + HTEXT;
	int xt = x() + w() / 2 - wl / 2;
	fl_draw(band_.c_str(), xt, yt);

	if (band_.length()) {
		// Draw mode text
		fl_font(0, FL_NORMAL_SIZE);
		for (auto const& ix : modes_) {
			int xt = x_scale_ + ix.second * BAR_WIDTH + BAR_WIDTH - 1;
			fl_color(FL_FOREGROUND_COLOR);
			fl_draw(90.0, ix.first.c_str(), xt, y_lower_ - 5);
		}
	}
}

// Generate data for band associated with frequency
void band_widget::generate_data(double f) {
	// Get the upper an lower bounds for the band
	band_ = spec_data_->band_for_freq(f);
	spec_data_->freq_for_band(band_, band_range_.lower, band_range_.upper);
	if (band_range_.lower > band_range_.upper) 
		printf("DEBUG: Spec: F=%g, Band = (%g %g)\n", f, band_range_.lower, band_range_.upper);
	data_ = band_data_->get_entries(band_range_.lower * 1000.0, band_range_.upper * 1000.0);
	modes_.clear();
	// Yes I mean these, finding the actual lower and upper band limits from the plan
	double lower = band_range_.upper;
	double upper = band_range_.lower;
	for (auto it = data_.begin(); it != data_.end(); it++) {
		double l = (*it)->lower / 1000.0;
		double u = (*it)->upper / 1000.0;
		if (l < band_range_.lower || u > band_range_.upper) {
			char msg[128];
			snprintf(msg, sizeof(msg), "BAND: Entry (%g, %g) outwith ADIF band (%g, %g)",
				l, u, band_range_.lower, band_range_.upper);
			status_->misc_status(ST_WARNING, msg);
		}
		// Merge available modes 
		for (auto iu = (*it)->modes.begin(); iu != (*it)->modes.end(); iu++) {
			if (modes_.find(*iu) == modes_.end() && (*iu).length()) {
				modes_[(*iu)] = modes_.size();
			}
		}
		lower = min(l, lower);
		upper = max(u, upper);
		// Now process the data item
		if (l == u) {
			// SPOT
			char* text = new char[128];
			snprintf(text, 128, FREQ_FORMAT " %s", l, (*it)->summary.c_str());
			add_marker({ l, SPOT, y_for_f(l), y_for_f(l), text});
		}
		else {
			// SUBABND
			char* text = new char[128];
			snprintf(text, 128, FREQ_FORMAT "-" FREQ_FORMAT " [%g] %s", 
				l, u, (*it)->bandwidth, (*it)->summary.c_str());
			add_marker({ l, SPOT, y_for_f(l), y_for_f(l), text });
			add_marker({ u, SPOT, y_for_f(u), y_for_f(u), text });
		}
	}
	if (data_.size()) {
		band_range_.lower = lower;
		band_range_.upper = upper;
	}
	if (band_range_.lower > band_range_.upper) 
		printf("DEBUG: Plan: F=%g, Band = (%g %g)\n", f, band_range_.lower, band_range_.upper);
	// Add current
	char* text = new char[32];
	snprintf(text, 32, FREQ_FORMAT, f);
	add_marker({ f, CURRENT, y_for_f(f), y_for_f(f), text });
}

// Add marker
void band_widget::add_marker(marker m) {
	bool inserted = false;
	auto it = markers_.begin();
	while (it != markers_.end() && !inserted) {
		if ((*it).f > m.f) {
			markers_.insert(it, m);
			inserted = true;
		}
		it++;
	}
	if (!inserted) {
		markers_.push_back(m);
	}
}

// Reset markers
void band_widget::reset_markers() {
	// Reset default value of ignore spots
	if (type() == BAND_SUMMARY) ignore_spots_ = true;
	else ignore_spots_ = false;
	// Reset text positions - and count valid markers
	int num_markers = 0;
	for (auto it = markers_.begin(); it != markers_.end(); it++) {
		// Reset text position
		(*it).y_text = (*it).y_scale;
		// Count whether it has a text marker
		switch ((*it).type) {
		case CURRENT:
		case SUBBAND_UPPER:
			num_markers++;
			break;
		case SUBBAND_LOWER:
			break;
		case SPOT:
			if (ignore_spots_) num_markers++;
			break;
		}
	}
}

// Adjust markers
void band_widget::adjust_markers() {
	bool done = markers_.size() < 2 ? true : false;
	bool top_down = true;
	bool adjust_up = true;
	auto it1 = markers_.begin();
	auto it2 = it1;
	int iteration = 0;
	while (!done) {
		int adjusts = 0;
		if (top_down) {
			it2 = it1 + 1;
			if (it2 == markers_.end()) {
				// Start next iteration
				if (adjusts == 0) {
					// ... or quit
					done = true;
				}
				else {
					if (adjust_up) {
						adjust_up = false;
					}
					else {
						adjust_up = true;
						top_down = false;
						it2 = markers_.end() - 1;
						it1 = it2 - 1;
					}
					printf("BAND: Iteration %d, %d adjustments\n", iteration, adjusts);
					iteration++;
					adjusts = 0;
				}
			}
			else {
				if (it1->y_text + FL_NORMAL_SIZE > it2->y_text) {
					// Texts overlap
					adjusts++;
					if (adjust_up) it1->y_text = it2->y_text - FL_NORMAL_SIZE;
					else it2->y_text = it1->y_text + FL_NORMAL_SIZE;
				}
				it1++;
			}
		}
		else {
			// Top down
			if (it2 == markers_.begin()) {
				// Start next iteration
				if (adjusts == 0) {
					// ... or quit
					done = true;
				}
				else {
					if (adjust_up) {
						adjust_up = false;
					}
					else {
						adjust_up = true;
						top_down = true;
					}
					printf("BAND: Iteration %d, %d adjustments\n", iteration, adjusts);
					iteration++;
					adjusts = 0;
					adjusts = 0;
				}
			}
			else {
				if (it1->y_text + FL_NORMAL_SIZE > it2->y_text) {
					// Texts overlap
					adjusts++;
					if (adjust_up) it1->y_text = it2->y_text - FL_NORMAL_SIZE;
					else it2->y_text = it1->y_text + FL_NORMAL_SIZE;
				}
				it2--;
				it1--;
			}
		}
	}

	// Calculate total number of markers required
	int num_markers = 0;

}


// Get Y-position for frequency
int band_widget::y_for_f(double f) {
	return y_upper_ + (int)((scale_range_.upper - f) * px_per_MHz_);
}

// Generate drawing positions
void band_widget::rescale() {
	if (band_.length()) {
		// Get required width of text
		fl_font(0, FL_NORMAL_SIZE);
		char text[15];
		snprintf(text, sizeof(text), FREQ_FORMAT, value_);
		int wx = 0, hx = 0;
		fl_measure(text, wx, hx);
		// Start at the left
		x_freq_ = x() + w() / 20;
		// Add text
		x_major_ = x_freq_ + wx;
		x_minor_ = x_major_ + 10;
		x_scale_ = x_minor_ + 10;
		// Text - and line connecting it to the scale
		x_kink1_ = x_scale_ + (BAR_WIDTH * (modes_.size() + 1));
		x_kink2_ = x_kink1_ + BAR_WIDTH * 2;
		x_text_ = x_kink2_ + BAR_WIDTH;
		x_sub_ = x_text_ - (BAR_WIDTH / 2);
		// Start at the top
		y_upper_ = y() + GAP;
		y_lower_ = y() + h() - HTEXT - GAP;
		// Text offset
		h_offset_ = FL_NORMAL_SIZE / 2;
		w_freq_ = wx;
		// Now what's the approximate scale
		double bw = band_range_.upper - band_range_.lower;
		scale_range_.lower = band_range_.lower;
		scale_range_.upper = band_range_.upper;
		px_per_MHz_ = (double)(y_lower_ - y_upper_) / (scale_range_.upper - scale_range_.lower);
		// Now work out major and minor ticks - get minor tick about 10 pixels
		double target = 10.0 / px_per_MHz_;
		double major;
		double poss = 0.0001;
		bool found = false;
		while (!found) {
			if (target >= poss && target < (poss * 10)) {
				found = true;
				double d = target / poss;
				if (d < 1.15) {
					target = 1.0 * poss;
					major = 5.0 * poss;
				}
				else if (d < 1.6) {
					target = 1.25 * poss;
					major = 5.0 * poss;
				}
				else if (d < 2.2) {
					target = 2.0 * poss;
					major = 10.0 * poss;
				}
				else if (d < 3.5) {
					target = 2.5 * poss;
					major = 10.0 * poss;
				}
				else if (d < 7.0) {
					target = 5.0 * poss;
					major = 25.0 * poss;
				}
				else {
					target = 10.0 * poss;
					major = 50.0 * poss;
				}
			}
			poss *= 10.0;
		}
		minor_tick_ = target;
		major_tick_ = major;
		// Now get to the next tick position above it
		scale_range_.upper = ceil(scale_range_.upper / minor_tick_) * minor_tick_;
		// And do the similar at the lower end of the scale
		scale_range_.lower = floor(scale_range_.lower / minor_tick_) * minor_tick_;
		// Recalcualte pixels/MHz
		px_per_MHz_ = (double)(y_lower_ - y_upper_) / (scale_range_.upper - scale_range_.lower);
		// Get the possible text positions
		int num_poss = (y_lower_ - y_upper_) / FL_NORMAL_SIZE;
	}
	else {
		// Start at the top
		y_upper_ = y() + GAP;
		y_lower_ = y() + h() - HTEXT - GAP;
	}
}

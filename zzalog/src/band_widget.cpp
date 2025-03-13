#include "band_widget.h"

#include "spec_data.h"
#include "band_data.h"
#include "band.h"
#include "status.h"
#include "drawing.h"

#include <cmath>
#include <string>
#include <algorithm>

#include <FL/fl_draw.H>

using namespace std;

extern spec_data* spec_data_;
extern band_data* band_data_;
extern status* status_;
extern bool DARK;

// Mode-bar colours
const map<string, Fl_Color> MODE_COLOURS = {
	{ "CW", FL_GREEN },
	{ "Dig", FL_YELLOW },
	{ "Phone", FL_BLUE },
	{ "Guard", COLOUR_ORANGE },
	{ "Beacons", FL_RED },
	{ "FM", FL_CYAN },
	{ "DV", FL_MAGENTA },
	{ "Repeater", COLOUR_APPLE },
	{ "Satellite", COLOUR_MAUVE },
	{ "TV", COLOUR_CLARET },
	{ "All", FL_BLACK }
};
const int BAR_WIDTH_F = 10;
const int BAR_WIDTH_S = 5;
// Adjustment for mouse wheel movement - 10 clicks doubles or halves the zoom
const double WHEEL_ADJUST = -0.1;
// Use #define so it's easier to concatenate with other formats
#define FREQ_FORMAT "%0.3f"

band_widget::band_widget(int X, int Y, int W, int H, const char* L) :
	Fl_Widget(X, Y, W, H, L) {
	type(BAND_FULL);
	value_tx_ = value_rx_ = 0.0;
	Fl_Widget::color(FL_RED, FL_DARK_GREEN);
	band_ = "";
	bandwidth_ = 0.5;
	process_data();
	default_mode();
	set_range();
	set_verticals();
	set_horizontals();
}

band_widget::~band_widget() {

}

void band_widget::draw() {
	// Restore defaults
	default_mode();
	// Rescale vertically as this determines the number of items we can fit
	set_verticals();
	// generate items as this determines the number of mode bars we need
	generate_items();
	set_horizontals();
	adjust_markers();
	// Wipe the background clean
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(x(), y(), w(), h());
	// Draw the outline box
	draw_box(box(), FL_FOREGROUND_COLOR);
	// Now restrict drawing inside the box
	fl_push_clip(x(), y(), w(), h());
	// Draw the valrious items
	draw_bands();
	draw_scale();
	draw_markers();
	draw_modebars();
	draw_legend();
	fl_pop_clip();
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
		case FL_MIDDLE_MOUSE:
			set_range(true);
			redraw();
			return true;
		}
		break;
	}
	case FL_MOUSEWHEEL: {
		// Adjust zoom value and scroll offset
		if (zoomable_) {
			double dy = Fl::event_dy();
			double dx = Fl::event_dx();
			// Adjust zoom by horizontal wheel or shift+single wheel
			// each click adjust zoomm by 2^0.1. (10 clicks doubles or halves the scroll)
			// Don't allow zoom out with the range 10 GHz to 1 Hz per oixel
			if (dx < 0 && px_per_MHz_ > 1.0e-4 || dx > 0 && px_per_MHz_ < 1e6) {
				bandwidth_ *= pow(2.0, (dx * WHEEL_ADJUST));
			}
			// Adjust scroll offset. One minor tick per click.
			median_ += minor_tick_ * dy;
			redraw();
			return true;
		}
	}
	}
	return Fl_Widget::handle(event);
}

// Value - frequency
void band_widget::value(double tx, double rx) {
	// Only redraw if value changes
	if (tx != value_tx_ || rx != value_rx_) {
		value_tx_ = tx;
		value_rx_ = rx;
		set_range();
		set_verticals();
		redraw();
	}
}

void band_widget::set_range(bool restore_default) {
	string prev_band = band_;
	band_ = spec_data_->band_for_freq(value_tx_);
	if (band_.length()) {
		band_limits_ = band_data_->bands().at(band_);
		if (auto_bw_ || prev_band != band_ || restore_default) {
			// We are resetting to show the full band
			scale_range_ = band_limits_;
			bandwidth_ = band_limits_.upper - band_limits_.lower;
			median_ = band_limits_.lower + (bandwidth_ * 0.5);
		} else if (value_tx_ > ( scale_range_.upper - (bandwidth_ * 0.1) ) ||
			value_tx_ < ( scale_range_.lower + ( bandwidth_ * 0.1 ) ) ) {
			// Getting to within 10% of the displayed range edge
			median_ = value_tx_;
		}
		// else do not move the displayed range
	} else { 
		median_ = value_tx_;
	}
}

// Draw the scale (from lower to upper +/- a bit)
void band_widget::draw_scale() {
	// Draw the axis
	fl_font(0, FL_NORMAL_SIZE);
	fl_color(FL_FOREGROUND_COLOR);
	fl_line(x_scale_, y_upper_, x_scale_, y_lower_);
	int curr_y;
	char text[15];
	// Align start frequency to a minor tick
	double tick_number = floor(scale_range_.upper / minor_tick_);
	double f = tick_number * minor_tick_;
	// Error tolerance in FP calculations
	double e0 = 0.5 / px_per_MHz_;
	double e1 = major_tick_ - e0;
	const char* format = label_format();
	int num_major = 0;
	double last_major;
	while (f >= scale_range_.lower - e0) {
		curr_y = y_for_f(f);
		double df = fmod(f, major_tick_);
		if (df < e0 || df > e1) {
			// Close enough to a major tick - draw the tick
			fl_line(x_major_, curr_y, x_scale_, curr_y);
			// Add the freq
			snprintf(text, sizeof(text), format, f);
			fl_draw(text, x_freq_, curr_y - h_offset_, w_freq_, 2 * h_offset_, FL_ALIGN_RIGHT);
			num_major++;
			last_major = f;
		}
		else {
			// draw a minor tick only
			fl_line(x_minor_, curr_y, x_scale_, curr_y);
		}
		f -= minor_tick_;
	}
	// Find the space between the middle two (or below the middle 1)
	f = last_major + (num_major / 2) * major_tick_;
	f -= (major_tick_ / 2.);
	curr_y = y_for_f(f);
	fl_draw("MHz", x_freq_, curr_y - h_offset_, w_freq_, 2 * h_offset_, FL_ALIGN_RIGHT);
}

// Draw the modebars
void band_widget::draw_markers() {
	for (int ix = 0; ix < markers_.size(); ix++) {
		marker& m = markers_[ix];
		switch (m.type) {
		case SUBBAND_LOWER:
			if (display_subbands_) {
				fl_color(FL_FOREGROUND_COLOR);
				draw_line(m.y_scale, m.y_text, FL_DOT);
				fl_font(0, FL_NORMAL_SIZE);
				fl_draw(m.text, x_text_, m.y_text + h_offset_);
			}
			break;
		case SUBBAND_LOCUM:
			if (display_subbands_) {
				fl_color(FL_FOREGROUND_COLOR);
				fl_font(0, FL_NORMAL_SIZE);
				fl_draw(m.text, x_text_, m.y_text + h_offset_);
			}
			break;
		case SUBBAND_UPPER:
			if (display_subbands_) {
			fl_color(FL_FOREGROUND_COLOR);
			fl_line(x_scale_, m.y_scale, x_kink1_, m.y_text);
			}
			break;
		case CURRENT:
			fl_color(Fl_Widget::color());
			draw_line(m.y_scale, m.y_text, 0);
			fl_font(0, FL_NORMAL_SIZE);
			fl_draw(m.text, x_text_, m.y_text + h_offset_);
			break;
		case CURRENT_LOCUM:
			fl_color(Fl_Widget::selection_color());
			fl_font(0, FL_NORMAL_SIZE);
			fl_draw(m.text, x_text_, m.y_text + h_offset_);
			break;
		case CURRENT_RX:
			fl_color(Fl_Widget::selection_color());
			draw_line(m.y_scale, m.y_text, 0);
			fl_font(0, FL_NORMAL_SIZE);
			fl_draw(m.text, x_text_, m.y_text + h_offset_);
			break;
		case SPOT:
			if (display_spots_) {
				fl_color(DARK ? FL_CYAN : FL_BLUE);
				draw_line(m.y_scale, m.y_text, 0);
				fl_font(0, FL_NORMAL_SIZE);
				fl_draw(m.text, x_text_, m.y_text + h_offset_);
			}
			break;
		case SPOTGROUP_LOWER:
			if (display_spots_) {
				fl_color(DARK ? FL_CYAN : FL_BLUE);
				draw_line(m.y_scale, m.y_text, FL_DOT);
				fl_font(0, FL_NORMAL_SIZE);
				fl_draw(m.text, x_text_, m.y_text + h_offset_);
			}
			break;
		case SPOTGROUP_UPPER:
			if (display_spots_) {
				fl_color(DARK ? FL_CYAN : FL_BLUE);
				fl_line(x_scale_, m.y_scale, x_kink1_, m.y_text);
			}
			break;
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
		int bar_num = used_modes_.at(m.mode);
		int xl = x_scale_ + 1 + (bar_num * bar_width_);
		fl_color(MODE_COLOURS.at(m.mode));
		fl_rectf(xl, yu, bar_width_, (yl - yu));
	}
}

// Draw legend
void band_widget::draw_legend() {
	fl_font(FL_BOLD, FL_NORMAL_SIZE + 2);
	int wl = 0, hl = 0;
	char t[132];
	fl_color(FL_FOREGROUND_COLOR);
	if (band_.length() == 0) {
		snprintf(t, sizeof(t), FREQ_FORMAT ": OUT OF BAND!", value_tx_);
		fl_color(FL_RED);
	}
	else if (!verbose_) {
		strcpy(t, band_.c_str());
	}
	else {
		snprintf(t, sizeof(t), "%s: %g MHz - %g MHz", 
			band_.c_str(), band_limits_.lower, band_limits_.upper);
	}
	fl_measure(t, wl, hl);
	int yt = y_lower_ + HTEXT;
	int xt = x() + w() / 2 - wl / 2;
	fl_draw(t, xt, yt);

	// Draw mode text
	fl_font(0, FL_NORMAL_SIZE);
	if (verbose_) {
		for (auto const& ix : used_modes_) {
			int xt = x_scale_ + ix.second * bar_width_ + bar_width_ - 1;
			fl_color(FL_FOREGROUND_COLOR);
			fl_draw(90.0, ix.first.c_str(), xt, y_lower_ - 5);
		}
	}
}

// Draw a background for the visible bands
void band_widget::draw_bands() {
	band_map<range_t>& bands = band_data_->bands();
	int prev_y = y() + h();
	Fl_Color band_colour = fl_color_average(FL_BACKGROUND_COLOR, FL_BACKGROUND2_COLOR, 0.50F);
	map<int, string> band_labels;
	for (auto it = bands.begin(); it != bands.end(); it++) {
		int yl;
		int yu;
		bool draw_it = false;
		if (it->second.lower >= scale_range_.lower && it->second.upper <= scale_range_.upper) {
			yl = y_for_f(it->second.lower);
			yu = y_for_f(it->second.upper);
			draw_it = true;
		}
		else if (it->second.lower >= scale_range_.lower && it->second.lower <= scale_range_.upper) {
			yl = y_for_f(it->second.lower);
			yu = y_for_f(scale_range_.upper);
			draw_it = true;
		}
		else if (it->second.upper <= scale_range_.upper && it->second.upper >= scale_range_.lower) {
			yl = y_for_f(scale_range_.lower);
			yu = y_for_f(it->second.upper);
			draw_it = true;
		}
		else if (it->second.lower <= scale_range_.lower && it->second.upper >= scale_range_.upper) {
			yl = y_for_f(scale_range_.lower);
			yu = y_for_f(scale_range_.upper);
			draw_it = true;
		}
		if (draw_it) {
			// Make the bar at least 1 pixel wide
			if (yl == yu) yl++;
			fl_rectf(x() + 1, yu, w() - 2, yl - yu, band_colour);
			fl_color(FL_FOREGROUND_COLOR);
			// Add the band text if there is room
			int yt = (yl - yu < FL_NORMAL_SIZE) ? (yu + yl) / 2 - h_offset_ : yu;
			if (yt <= prev_y - FL_NORMAL_SIZE) {
				band_labels[yt] = it->first;
				prev_y = yt;
			}
			else {
				band_labels[prev_y] += ',' + it->first;
			}
		}
	}
	// Now draw the labels
	if (display_band_label_) {
		for (auto it = band_labels.begin(); it != band_labels.end(); it++) {
			fl_draw(it->second.c_str(), x(), it->first, w() - GAP, 2 * h_offset_, FL_ALIGN_RIGHT);
		}
	}
}

// Generate data for band associated with frequency
void band_widget::process_data() {
	// Get the upper an lower bounds for the band
	data_ = band_data_->get_entries();
	modes_.clear();
	for (auto it = data_.begin(); it != data_.end(); it++) {
		// Merge available modes 
		for (auto iu = (*it)->modes.begin(); iu != (*it)->modes.end(); iu++) {
			// Add the mode bars
			if (modes_.find(*iu) == modes_.end() && (*iu).length()) {
				modes_[(*iu)] = modes_.size();
			}
		}
	}
}

void band_widget::generate_items() {
	for (auto it = markers_.begin(); it != markers_.end(); it++) {
		delete[] it->text;
	}
	markers_.clear();
	mode_bars_.clear();
	used_modes_.clear();
	num_spots_ = 0;
	num_subbands_ = 0;
	for (auto it = data_.begin(); it != data_.end(); it++) {
		char* text = new char[128];
		double l = (*it)->range.lower;
		double u = (*it)->range.upper;
		double ll = max(l, scale_range_.lower);
		double lu = min(u, scale_range_.upper);
		bool include;
		if ((l >= scale_range_.lower && l < scale_range_.upper) ||
			(u > scale_range_.lower && u <= scale_range_.upper)) {
			include = true;
		} else {
			include = false;
		}
		for (auto iu = (*it)->modes.begin(); iu != (*it)->modes.end(); iu++) {
			// Add the mode bars
			if ((*iu).length() && include) {
				mode_bars_.push_back({ *iu, ll, lu });
				if (used_modes_.find(*iu) == used_modes_.end()) {
					used_modes_[*iu] = used_modes_.size();
				}
			}
		}
		// Now process the data item
		if (l == u) {
			// SPOT
			if (include) {
				snprintf(text, 128, FREQ_FORMAT " %s", l, (*it)->summary.c_str());
				add_marker({ l, SPOT, y_for_f(l), y_for_f(l), text});
				num_spots_++;
			}
		}
		else {
			if (include) {
				if (verbose_) {
					if ((*it)->modes.size()) {
						// SUBBAND
						snprintf(text, 128, FREQ_FORMAT "-" FREQ_FORMAT " [%g] %s", 
							l, u, (*it)->bandwidth, (*it)->summary.c_str());
						if (l == ll) {
							add_marker({ u, SUBBAND_UPPER, y_for_f(u), y_for_f(u), nullptr });
							add_marker({ l, SUBBAND_LOWER, y_for_f(l), y_for_f(l), text });
							num_subbands_++;
						} else {
							add_marker({ ll, SUBBAND_LOCUM, y_for_f(ll), y_for_f(ll), text});
							num_subbands_++;
						}
					} else {
						// SPOTGROUP
						snprintf(text, 128, FREQ_FORMAT "-" FREQ_FORMAT " %s", 
							l, u, (*it)->summary.c_str());
						if (l == ll) {
							add_marker({ u, SPOTGROUP_UPPER, y_for_f(u), y_for_f(u), nullptr });
							add_marker({ l, SPOTGROUP_LOWER, y_for_f(l), y_for_f(l), text });
							num_spots_++;
						}
					}
				} else {
					if ((*it)->modes.size()) {
						// SUBBAND
						snprintf(text, 128, FREQ_FORMAT " [%g] %s", 
							l, (*it)->bandwidth, (*it)->summary.c_str());
						if (l == ll) {
							add_marker({ u, SUBBAND_UPPER, y_for_f(u), y_for_f(u), nullptr });
							add_marker({ l, SUBBAND_LOWER, y_for_f(l), y_for_f(l), text });
							num_subbands_++;
						} else {
							add_marker({ ll, SUBBAND_LOCUM, y_for_f(ll), y_for_f(ll), text});
							num_subbands_++;
						}
					} else {
						// SPOTGROUP
						snprintf(text, 128, FREQ_FORMAT " %s", 
							l, (*it)->summary.c_str());
						if (l == ll) {
							add_marker({ u, SPOTGROUP_UPPER, y_for_f(u), y_for_f(u), nullptr });
							add_marker({ l, SPOTGROUP_LOWER, y_for_f(l), y_for_f(l), text });
							num_spots_++;
						}
					}
				}
			}
		}
	}
	// Add current
	if (!isnan(value_tx_) && value_tx_ != 0.0) {
		char* text = new char[32];
		double f = value_tx_;
		snprintf(text, 32, FREQ_FORMAT " TX Frequency", f);
		if (f > scale_range_.upper) {
			add_marker({ f, CURRENT_LOCUM, y_for_f(scale_range_.upper), y_for_f(scale_range_.upper), text });
		} else if ( f < scale_range_.lower) {
			add_marker({ f, CURRENT_LOCUM, y_for_f(scale_range_.lower), y_for_f(scale_range_.lower), text });
		} else {
			add_marker({ f, CURRENT, y_for_f(f), y_for_f(f), text });
		}
	}
	// Add RX
	if (value_rx_ != 0.0 && value_rx_ != value_tx_) {
		char* text = new char[32];
		double f = value_rx_;
		snprintf(text, 32, FREQ_FORMAT " RX Frequency", f);
		if (f <= scale_range_.upper && f >= scale_range_.lower) {
			add_marker({ f, CURRENT_RX, y_for_f(f), y_for_f(f), text });
		}
	}
	int available_slots = ((y_lower_ - y_upper_) / FL_NORMAL_SIZE);
	if (available_slots < num_subbands_) {
		display_spots_ = false;
		display_subbands_ = false;
	} else {
		if (available_slots < num_subbands_ + num_spots_) {
			display_spots_ = false;
		}
	}
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
		else {
			it++;
		}
	}
	if (!inserted) {
		markers_.push_back(m);
	}
}

// Adjust markers
void band_widget::adjust_markers() {
	if (markers_.size() <= 1) return;
	// Make it1 the lowest frequency 
	auto it1 = markers_.begin();
	auto it2 = it1;
	int iteration = 0;
	bool squashed = false;
	// First from highest frequency to lowest - adjust lower frequency downward (higher y)
	it2 = markers_.end() - 1;
	while (!is_text_marker(*it2) && it2 != markers_.begin()) it2--;
	if (it2 == markers_.begin()) return;
	it1 = it2 - 1;
	while (!is_text_marker(*it1) && it1 != markers_.begin()) it1--;
	while (it2 != markers_.begin()) {
		int next = it2->y_text + FL_NORMAL_SIZE;
		if (it1->y_text < next) {
			it1->y_text = min(next, y_lower_);
			if (it1->y_text != next) squashed = true;
		} else {
		}
		do it2--;
		while (!is_text_marker(*it2) && it2 != markers_.begin());
		if (it2 != markers_.begin()) {
			it1 = it2 - 1;
			while (!is_text_marker(*it1) && it1 != markers_.begin()) it1--;
		}
	}
	// No adjustements finished.
	// print_markers();
	if (!squashed) return;
	iteration ++;
	squashed = false;
	// Second from lowest frequency to highest - adjust higher frequency upward (lower y)
	it1 = markers_.begin();
	while(!is_text_marker(*it1) && it1 != markers_.end()) it1++;
	if (it1 == markers_.end()) return;
	it2 = it1 + 1;
	while(!is_text_marker(*it2) && it2 != markers_.end()) it2++;
	while (it2 != markers_.end()) {
		int next = it1->y_text - FL_NORMAL_SIZE;
		if (it2->y_text > next) {
			it2->y_text = max(next, y_upper_);
			if (it2->y_text != next) squashed = true;
		} else {
		}
		do it1++;
		while(!is_text_marker(*it1) && it1 != markers_.end());
		if (it1 == markers_.end()) return;
		it2 = it1 + 1;
		while(!is_text_marker(*it2) && it2 != markers_.end()) it2++;
	}
	// print_markers();
	// No adjustements finished.
	if (!squashed) return;
	iteration++;
	squashed = false;
	// Third from lowest frequency to highest - adjust lower frequency downward (higher y)
	it1 = markers_.begin();
	while(!is_text_marker(*it1) && it1 != markers_.end()) it1++;
	if (it1 == markers_.end()) return;
	it2 = it1 + 1;
	while(!is_text_marker(*it2) && it2 != markers_.end()) it2++;
	while (it2 != markers_.end()) {
		int next = it2->y_text + FL_NORMAL_SIZE;
		if (it1->y_text > next) {
			it1->y_text = min(next, y_lower_);
			if (it1->y_text != next) squashed = true;
		} else {
		}
		do it1++;
		while(!is_text_marker(*it1) && it1 != markers_.end());
		if (it1 == markers_.end()) return;
		it2 = it1 + 1;
		while(!is_text_marker(*it2) && it2 != markers_.end()) it2++;
	}
	// print_markers();
	// No adjustements finished.
	if (!squashed) return;
	iteration++;
	squashed = false;
	// Fourth from highest frequency to lowest - adjust higher frequency downward (higher y)
	it2 = markers_.end() - 1;
	while (!is_text_marker(*it2) && it2 != markers_.begin()) it2--;
	if (it2 == markers_.begin()) return;
	it1 = it2 - 1;
	while (!is_text_marker(*it1) && it1 != markers_.begin()) it1--;
	while (it2 != markers_.begin()) {
		int next = it1->y_text - FL_NORMAL_SIZE;
		if (it2->y_text < next) {
			it2->y_text = max(next, y_upper_);
			if (it2->y_text != next) squashed = true;
		}
		do it2--;
		while (!is_text_marker(*it2) && it2 != markers_.begin());
		if (it2 != markers_.begin()) {
			it1 = it2 - 1;
			while (!is_text_marker(*it1) && it1 != markers_.begin()) it1--;
		}
	}
	// print_markers();
	// No adjustements finished.
	if (!squashed) return;

	char msg[128];
	iteration ++;
	snprintf(msg, sizeof(msg), "BAND: Unable to fit all markers in");
	status_->misc_status(ST_WARNING, msg);
}

// Is a text marker
bool band_widget::is_text_marker(marker m) {
	switch(m.type) {
	case SUBBAND_UPPER:
	case SPOTGROUP_UPPER:
		return false;
	case CURRENT:
	case CURRENT_RX:
		return true;
	case SUBBAND_LOWER:
	case SUBBAND_LOCUM:
	case CURRENT_LOCUM:
		return display_subbands_;
	case SPOT:
	case SPOTGROUP_LOWER:
		return display_spots_;
	}
	return false;
}


// Get Y-position for frequency
int band_widget::y_for_f(double f) {
	return y_upper_ + (int)((scale_range_.upper - f) * px_per_MHz_);
}

void band_widget::set_verticals() {
	// Start at the top
	y_upper_ = y() + GAP;
	y_lower_ = y() + h() - HTEXT - GAP;
	// Now what's the approximate scale
	scale_range_.lower = median_ - bandwidth_ * 0.5;
	scale_range_.upper = median_ + bandwidth_ * 0.5;
	if (scale_range_.lower < 0) {
		scale_range_.upper -= scale_range_.lower;
		scale_range_.lower = 0.0;
	}
	px_per_MHz_ = (double)(y_lower_ - y_upper_) / (scale_range_.upper - scale_range_.lower);
	// Now work out major and minor ticks - get minor tick about 10 pixels
	double target = 10.0 / px_per_MHz_;
	double major;
	double l10_target = log10(target);
	double exponent = floor(l10_target);
	double power10 = pow(10.0, exponent);
	double mantissa = target / power10;
	if (mantissa < 1.15) {
		minor_tick_ = power10;
		major_tick_ = 5.0 * power10;
	} else if (mantissa < 1.6) {
		minor_tick_ = 1.25 * power10;
		major_tick_ = 5.0 * power10;
	} else if (mantissa < 2.2) {
		minor_tick_ = 2.0 * power10;
		major_tick_ = 10.0 * power10;
	} else if (mantissa < 3.5) {
		minor_tick_ = 2.5 * power10;
		major_tick_ = 10.0 * power10;
	} else if (mantissa < 7.0) {
		minor_tick_ = 5.0 * power10;
		major_tick_ = 20.0 * power10;
	} else {
		minor_tick_ = 10.0 * power10;
		major_tick_ = 50.0 * power10;
	}
	// // Now get to the next tick position above it
	// scale_range_.upper = ceil(scale_range_.upper / minor_tick_) * minor_tick_;
	// // And do the similar at the lower end of the scale
	// scale_range_.lower = floor(scale_range_.lower / minor_tick_) * minor_tick_;
	// // Recalcualte pixels/MHz
	// px_per_MHz_ = (double)(y_lower_ - y_upper_) / (scale_range_.upper - scale_range_.lower);
}

void band_widget::set_horizontals() {
	// Now sort out the horizontal positions
	fl_font(0, FL_NORMAL_SIZE);
	char text[15];
	snprintf(text, sizeof(text), label_format(), scale_range_.upper);
	int wx = 0, hx = 0;
	fl_measure(text, wx, hx);
	// Start at the left
	x_freq_ = x() + w() / 20;
	// Add text
	x_major_ = x_freq_ + wx;
	x_minor_ = x_major_ + bar_width_;
	x_scale_ = x_minor_ + bar_width_;
	// Text - and line connecting it to the scale
	x_kink1_ = x_scale_ + (bar_width_ * (used_modes_.size() + 1));
	x_kink2_ = x_kink1_ + bar_width_ * 2;
	x_text_ = x_kink2_ + bar_width_;
	x_sub_ = x_text_ - (bar_width_ / 2);
	// Text offset
	h_offset_ = FL_NORMAL_SIZE / 2;
	w_freq_ = wx;
}

const char* band_widget::label_format() {
	double tick_log10 = floor(log10(major_tick_));
	if (tick_log10 < 0) {
		snprintf(label_format_, sizeof(label_format_), "%%.%.0ff", - tick_log10);
	} else {
		strcpy(label_format_, "%.0f");
	}
	return label_format_;
}

void band_widget::default_mode() {
	if ((type() & BAND_MASK) == BAND_FULL) {
		display_bands_ = true;
		display_band_label_ = true;
		display_spots_ = true;
		display_subbands_ = true;
		auto_bw_ = false;
		bar_width_ = 10;
		verbose_ = true;
	} else {
		display_bands_ = true;
		display_band_label_ = false;
		display_spots_ = false;
		display_subbands_ = true;
		auto_bw_ = true;
		bar_width_ = 2;
		verbose_ = false;
	}
	if (type() & ZOOMABLE) {
		zoomable_ = true;
	} else {
		zoomable_ = false;
	}
}
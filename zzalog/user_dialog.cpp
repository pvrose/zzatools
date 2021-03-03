#include "user_dialog.h"
#include "drawing.h"
#include "log_table.h"
#include "book.h"
#include "scratchpad.h"
#include "pfx_tree.h"
#include "report_tree.h"
#include "spec_tree.h"
#include "tabbed_forms.h"
#include "../zzalib/rig_if.h"

#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Counter.H>

using namespace zzalog;
using namespace zzalib;

extern Fl_Preferences* settings_;
extern book* book_;
extern scratchpad* scratchpad_;
extern tabbed_forms* tabbed_forms_;
extern rig_if* rig_if_;

user_dialog::user_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label) 
{
	// Set defaults
	log_font_ = FONT;
	log_size_ = FONT_SIZE;
	tip_duration_ = Fl_Tooltip::delay();
	tip_font_ = Fl_Tooltip::font();
	tip_size_ = Fl_Tooltip::size();
	spad_font_ = FONT;
	spad_size_ = FONT_SIZE;
	date_format_ = DATE_YYYYMMDD;
	time_format_ = TIME_HHMMSS;
	freq_format_ = FREQ_MHz;

	do_creation(X, Y);
}

user_dialog::~user_dialog() {}

// Load values from settings_
void user_dialog::load_values() {
	Fl_Preferences user_settings(settings_, "User Settings");
	// Log table
	Fl_Preferences log_settings(user_settings, "Log Table");
	log_settings.get("Font Name", (int&)log_font_, FONT);
	log_settings.get("Font Size", (int&)log_size_, FONT_SIZE);
	// Tooltip
	Fl_Preferences tip_settings(user_settings, "Tooltip");
	tip_settings.get("Duration", tip_duration_, Fl_Tooltip::delay());
	tip_settings.get("Font Name", (int&)tip_font_, Fl_Tooltip::font());
	tip_settings.get("Font Size", (int&)tip_size_, Fl_Tooltip::size());
	// Scratchpad
	Fl_Preferences spad_settings(user_settings, "Scratchpad");
	spad_settings.get("Font Name", (int&)spad_font_, FONT);
	spad_settings.get("Font Size", (int&)spad_size_, FONT_SIZE);
	// Tree views
	Fl_Preferences tree_settings(user_settings, "Tree Views");
	tree_settings.get("Font Name", (int&)tree_font_, FONT);
	tree_settings.get("Font Size", (int&)tree_size_, FONT_SIZE);
	// Formats
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.get("Frequency", (int&)freq_format_, FREQ_MHz);
	display_settings.get("Date", (int&)date_format_, DATE_YYYYMMDD);
	display_settings.get("Time", (int&)time_format_, TIME_HHMMSS);
}

// Used to create the form
void user_dialog::create_form(int X, int Y) {
	begin();
	// Group 1 - log_table
	int pos_x = GAP + EDGE;
	int pos_y = GAP + EDGE;
	// Log table 
	Fl_Group* g1 = new Fl_Group(pos_x, pos_y, 0, 10, "Log Table");
	g1->labelfont(FONT);
	g1->labelsize(FONT_SIZE);
	g1->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g1->box(FL_THIN_DOWN_BOX);
	// Add font browser
	pos_x += GAP;
	pos_y += HTEXT;
	// Browser to select font
	Fl_Hold_Browser* br1 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br1->labelfont(FONT);
	br1->labelsize(FONT_SIZE);
	br1->textsize(FONT_SIZE);
	br1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br1->tooltip("Please select the font used in the cells in all log table views");
	populate_font(br1, &log_font_);
	// Add font size browser
	pos_x += br1->w();
	Fl_Hold_Browser* br2 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
	br2->labelfont(FONT);
	br2->labelsize(FONT_SIZE);
	br2->textfont(FONT);
	br2->textsize(FONT_SIZE);
	br2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br2->callback(cb_br_size, &log_size_);
	br2->tooltip("Please select the font size used in the cells in all log table views");
	br1->callback(cb_br_logfont, br2);
	populate_size(br2, &log_font_, &log_size_);
	// End group - fit to size
	g1->resizable(nullptr);
	pos_y = br2->y() + br2->h() + GAP;
	pos_x = br2->x() + br2->w() + GAP;
	g1->size(pos_x - g1->x(), pos_y - g1->y());
	g1->end();


	// Group 2 - tool tip - align on above group
	pos_x = g1->x();
	pos_y += GAP;
	Fl_Group* g2 = new Fl_Group(pos_x, pos_y, 10, 10, "Tooltips");
	g2->labelfont(FONT);
	g2->labelsize(FONT_SIZE);
	g2->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g2->box(FL_THIN_DOWN_BOX);
	// Add font browser
	pos_x += GAP;
	pos_y += HTEXT;
	Fl_Hold_Browser* br3 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br3->labelfont(FONT);
	br3->labelsize(FONT_SIZE);
	br3->textsize(FONT_SIZE);
	br3->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br3->tooltip("Please select the font used in the cells in all tooltips");
	populate_font(br3, &tip_font_);
	// Add font size browser
	pos_x += br3->w();
	Fl_Hold_Browser* br4 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
	br4->labelfont(FONT);
	br4->labelsize(FONT_SIZE);
	br4->textfont(FONT);
	br4->textsize(FONT_SIZE);
	br4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br4->callback(cb_br_size, &tip_size_);
	br4->tooltip("Please select the font size used in the cells in all tooltips");
	br3->callback(cb_br_tipfont, br4);
	populate_size(br4, &tip_font_, &tip_size_);
	pos_y = br4->y() + br4->h() + GAP;
	pos_x = br3->x();
	// Counter valuator - 1 to 15 s in 0.5 s steps
	Fl_Counter* val1 = new Fl_Counter(pos_x, pos_y, br3->w(), HBUTTON, "Duration (s)");
	val1->labelfont(FONT);
	val1->labelsize(FONT_SIZE);
	val1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	val1->textfont(FONT);
	val1->textsize(FONT_SIZE);
	val1->type(FL_SIMPLE_COUNTER);
	val1->step(0.5);
	val1->range(1.0, 15.0);
	val1->value(tip_duration_);
	val1->callback(cb_value<Fl_Counter, float>, &tip_duration_);
	// End group - fit to size
	g2->resizable(nullptr);
	pos_y = val1->y() + val1->h() + GAP;
	pos_x = max(br4->x() + br4->w(), val1->x() + val1->w()) + GAP;
	g2->size(pos_x - g2->x(), pos_y - g2->y());
	g2->end();

	// Group 3 - scratchpad
	pos_y += GAP;
	pos_x = g2->x();
	Fl_Group* g3 = new Fl_Group(pos_x, pos_y, 0, 10, "Scratchpad");
	g3->labelfont(FONT);
	g3->labelsize(FONT_SIZE);
	g3->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g3->box(FL_THIN_DOWN_BOX);
	// Add font browser
	pos_x += GAP;
	pos_y += HTEXT;
	Fl_Hold_Browser* br5 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br5->labelfont(FONT);
	br5->labelsize(FONT_SIZE);
	br5->textsize(FONT_SIZE);
	br5->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br5->tooltip("Please select the font used in the cells in all scratchpad");
	populate_font(br5, &spad_font_);
	// Add font size browser
	pos_x += br5->w();
	Fl_Hold_Browser* br6 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
	br6->labelfont(FONT);
	br6->labelsize(FONT_SIZE);
	br6->textfont(FONT);
	br6->textsize(FONT_SIZE);
	br6->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br6->callback(cb_br_size, &spad_size_);
	br6->tooltip("Please select the font size used in the cells in scratchpad");
	br5->callback(cb_br_spadfont, br6);
	populate_size(br6, &spad_font_, &spad_size_);
	// End group - fit to size
	g3->resizable(nullptr);
	pos_y = br6->y() + br6->h() + GAP;
	pos_x = br6->x() + br6->w() + GAP;
	g3->size(pos_x - g3->x(), pos_y - g3->y());
	g3->end();

	// Group 3 - tree views (report, prefix and specification)
	pos_y += GAP;
	pos_x = g2->x();
	Fl_Group* g4 = new Fl_Group(pos_x, pos_y, 0, 10, "Tree views");
	g4->labelfont(FONT);
	g4->labelsize(FONT_SIZE);
	g4->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g4->box(FL_THIN_DOWN_BOX);
	// Add font browser
	pos_x += GAP;
	pos_y += HTEXT;
	Fl_Hold_Browser* br7 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br7->labelfont(FONT);
	br7->labelsize(FONT_SIZE);
	br7->textsize(FONT_SIZE);
	br7->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br7->tooltip("Please select the font used in the cells in all tree views");
	populate_font(br7, &tree_font_);
	// Add font size browser
	pos_x += br7->w();
	Fl_Hold_Browser* br8 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
	br8->labelfont(FONT);
	br8->labelsize(FONT_SIZE);
	br8->textfont(FONT);
	br8->textsize(FONT_SIZE);
	br8->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br8->callback(cb_br_size, &tree_size_);
	br8->tooltip("Please select the font size used in the cells in all tree views");
	br7->callback(cb_br_treefont, br8);
	populate_size(br8, &tree_font_, &tree_size_);
	// End group - fit to size
	g4->resizable(nullptr);
	pos_y = br8->y() + br8->h() + GAP;
	pos_x = br8->x() + br8->w() + GAP;
	g4->size(pos_x - g4->x(), pos_y - g4->y());
	g4->end();

	// Group 5 - user formats
	pos_y = GAP + EDGE;
	pos_x = g1->x() + g1->w() + GAP;
	Fl_Group* g5 = new Fl_Group(pos_x, pos_y, 0, 10, "User Formats");
	g5->labelfont(FONT);
	g5->labelsize(FONT_SIZE);
	g5->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g5->box(FL_THIN_DOWN_BOX);
	// Add choices - frequency 
	pos_x += GAP;
	pos_y += HTEXT + FONT_SIZE;
	Fl_Choice* ch1 = new Fl_Choice(pos_x, pos_y, WEDIT, HBUTTON, "Frequency");
	ch1->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ch1->labelfont(FONT);
	ch1->labelsize(FONT_SIZE);
	ch1->textfont(FONT);
	ch1->textsize(FONT_SIZE);
	ch1->callback(cb_value<Fl_Choice, display_freq_t>, (void*)&freq_format_);
	ch1->when(FL_WHEN_RELEASE);
	populate_freq(ch1, freq_format_);

	// Add Date choice
	pos_y += ch1->h() + HTEXT;
	Fl_Choice* ch2 = new Fl_Choice(pos_x, pos_y, WEDIT, HBUTTON, "Date");
	ch2->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ch2->labelfont(FONT);
	ch2->labelsize(FONT_SIZE);
	ch2->textfont(FONT);
	ch2->textsize(FONT_SIZE);
	ch2->callback(cb_value<Fl_Choice, display_date_t>, (void*)&date_format_);
	ch2->when(FL_WHEN_RELEASE);
	populate_date(ch2, date_format_);

	// Add Time choice
	pos_y += ch2->h() + HTEXT;
	Fl_Choice* ch3 = new Fl_Choice(pos_x, pos_y, WEDIT, HBUTTON, "Time");
	ch3->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
	ch3->labelfont(FONT);
	ch3->labelsize(FONT_SIZE);
	ch3->textfont(FONT);
	ch3->textsize(FONT_SIZE);
	ch3->callback(cb_value<Fl_Choice, display_time_t>, (void*)&time_format_);
	ch3->when(FL_WHEN_RELEASE);
	populate_time(ch3, time_format_);

	// End group - fit to size
	g5->resizable(nullptr);
	pos_y = ch3->y() + ch3->h() + GAP;
	pos_x = ch3->x() + ch3->w() + GAP;
	g5->size(pos_x - g5->x(), pos_y - g5->y());
	g5->end();

	end();
	show();
}

// Used to write settings back
void user_dialog::save_values() {
	Fl_Preferences user_settings(settings_, "User Settings");
	// Log settings
	Fl_Preferences log_settings(user_settings, "Log Table");
	log_settings.set("Font Name", (int&)log_font_);
	log_settings.set("Font Size", (int&)log_size_);
	// Tell the log views
	log_table::set_font(log_font_, log_size_);
	// Tooltip
	Fl_Preferences tip_settings(user_settings, "Tooltip");
	tip_settings.set("Duration", tip_duration_);
	tip_settings.set("Font Name", (int&)tip_font_);
	tip_settings.set("Font Size", (int&)tip_size_);
	// Tell the tooltips
	Fl_Tooltip::delay(tip_duration_);
	Fl_Tooltip::font(tip_font_);
	Fl_Tooltip::size(tip_size_);
	// Scratchpad settings
	Fl_Preferences spad_settings(user_settings, "Scratchpad");
	spad_settings.set("Font Name", (int&)spad_font_);
	spad_settings.set("Font Size", (int&)spad_size_);
	// Tell the log views
	scratchpad_->set_font(spad_font_, spad_size_);
	// Tree view settings
	Fl_Preferences tree_settings(user_settings, "Tree Views");
	tree_settings.set("Font Name", (int&)tree_font_);
	tree_settings.set("Font Size", (int&)tree_size_);
	((pfx_tree*)tabbed_forms_->get_view(OT_PREFIX))->set_font(tree_font_, tree_size_);
	((report_tree*)tabbed_forms_->get_view(OT_REPORT))->set_font(tree_font_, tree_size_);
	((spec_tree*)tabbed_forms_->get_view(OT_ADIF))->set_font(tree_font_, tree_size_);

	Fl_Preferences display_settings(settings_, "Display");
	display_settings.set("Frequency", (int)freq_format_);
	display_settings.set("Date", (int)date_format_);
	display_settings.set("Time", (int)time_format_);

	// Now tell all views to update formats
	book_->selection(-1, HT_FORMAT);
}

// Used to enable/disable specific widget - any widgets enabled must be attributes
void user_dialog::enable_widgets() {}

// Callback for log_font browser
// v is unused
void user_dialog::cb_br_logfont(Fl_Widget* w, void* v) {
	user_dialog* that = ancestor_view<user_dialog>(w);
	Fl_Hold_Browser* font_br = (Fl_Hold_Browser*)w;
	that->log_font_ = (Fl_Font)font_br->value() - 1;
	that->populate_size((Fl_Hold_Browser*)v, &that->log_font_, &that->log_size_);
}

// Callback for all size browsers
// v is a pointer to the size variable
void user_dialog::cb_br_size(Fl_Widget* w, void* v) {
	user_dialog* that = ancestor_view<user_dialog>(w);
	Fl_Hold_Browser* size_br = (Fl_Hold_Browser*)w;
	int line = size_br->value();
	*(Fl_Fontsize*)v = stoi(size_br->text(line));
}

// Callback for tooltip font browser
// v is unused
void user_dialog::cb_br_tipfont(Fl_Widget* w, void* v) {
	user_dialog* that = ancestor_view<user_dialog>(w);
	Fl_Hold_Browser* font_br = (Fl_Hold_Browser*)w;
	that->tip_font_ = (Fl_Font)font_br->value() - 1;
	that->populate_size((Fl_Hold_Browser*)v, &that->tip_font_, &that->tip_size_);
}

// Callback for scratchpad font browser
// v is unused
void user_dialog::cb_br_spadfont(Fl_Widget* w, void* v) {
	user_dialog* that = ancestor_view<user_dialog>(w);
	Fl_Hold_Browser* font_br = (Fl_Hold_Browser*)w;
	that->spad_font_ = (Fl_Font)font_br->value() - 1;
	that->populate_size((Fl_Hold_Browser*)v, &that->spad_font_, &that->spad_size_);
}

// Callback for tree view font browser
// v is unused
void user_dialog::cb_br_treefont(Fl_Widget* w, void* v) {
	user_dialog* that = ancestor_view<user_dialog>(w);
	Fl_Hold_Browser* font_br = (Fl_Hold_Browser*)w;
	that->tree_font_ = (Fl_Font)font_br->value() - 1;
	that->populate_size((Fl_Hold_Browser*)v, &that->tree_font_, &that->tree_size_);
}

// Populate the font browser
void user_dialog::populate_font(Fl_Hold_Browser* br, const Fl_Font* font) {
	br->clear();
	// Get only ISO9958-1 fonts
	int num_fonts = Fl::set_fonts(nullptr);
	for (int i = 0; i < num_fonts; i++) {
		// Contains any combination of FL_BOLD and FL_ITALIC
		int attr = 0;
		const char* name = Fl::get_font_name(Fl_Font(i), &attr);
		char buffer[128];
		// display in the named font
		sprintf(buffer, "@F%d@.%s", i, name);
		br->add(buffer);
	}
	br->value(*font + 1);
}

// Populate the size browser
void user_dialog::populate_size(Fl_Hold_Browser* br, const Fl_Font* font, const Fl_Fontsize* size) {
	br->clear();
	// To receive the array of sizes
	int* sizes;
	int num_sizes = Fl::get_font_sizes(*font, sizes);
	if (num_sizes) {
		// We have some sizes
		if (sizes[0] == 0) {
			// Scaleable font - so any size available 
			for (int i = 1; i < max(64, sizes[num_sizes - 1]); i++) {
				char buff[20];
				sprintf(buff, "%d", i);
				br->add(buff);
			}
			br->value(*size);
		}
		else {
			// Only list available sizes
			int select = 0;
			for (int i = 0; i < num_sizes; i++) {
				// while the current value is less than required up the selected value
				if (sizes[i] < *size) {
					select = i;
				}
				char buff[20];
				sprintf(buff, "%d", sizes[i]);
				br->add(buff);
			}
			br->value(select);
		}
	}
}

// Populate frequency
void user_dialog::populate_freq(Fl_Choice* ch, display_freq_t format) {
	string freq = "14.000000";
	if (rig_if_) {
		freq = rig_if_->get_frequency(false);
	}
	ch->clear();
	char temp[25];
	// Display the current frequency in the selected format
	for (int i = 0; i < (int)FREQ_END; i++) {
		snprintf(temp, 25, "%d: %s", i, record::format_freq((display_freq_t)i, freq).c_str());
		ch->add(temp);
	}
	if (format < FREQ_END) {
		ch->value((int)format);
	}
	else {
		ch->value(0);
	}
}

// Populate date choice
void user_dialog::populate_date(Fl_Choice* ch, display_date_t format) {
	string date = now(false, "%Y%m%d");
	// Avoid dates where the month and day are the same
	if (now(false, "%m") == now(false, "%d")) {
		date = "20201231";
	}
	ch->clear();
	char temp[25];
	for (int i = 0; i < (int)DATE_END; i++) {
		snprintf(temp, 25, "%d: %s", i, record::format_date((display_date_t)i, date).c_str());
		ch->add(temp);
	}
	if (format < DATE_END) {
		ch->value((int)format);
	}
	else {
		ch->value(0);
	}
}

// Populate time choice
void user_dialog::populate_time(Fl_Choice* ch, display_time_t format) {
	string time = now(false, "%H%M%S");
	ch->clear();
	char temp[25];
	for (int i = 0; i < (int)TIME_END; i++) {
		snprintf(temp, 25, "%d: %s", i, record::format_time((display_time_t)i, time).c_str());
		ch->add(temp);
	}
	if (format < TIME_END) {
		ch->value((int)format);
	}
	else {
		ch->value(0);
	}
}

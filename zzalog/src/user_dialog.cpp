#include "user_dialog.h"
#include "drawing.h"
#include "log_table.h"
#include "book.h"
#include "report_tree.h"
#include "spec_tree.h"
#include "tabbed_forms.h"
#include "qso_manager.h"
#include "callback.h"
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Counter.H>

extern Fl_Preferences* settings_;
extern book* book_;
extern qso_manager* qso_manager_;
extern tabbed_forms* tabbed_forms_;

// constructor
user_dialog::user_dialog(int X, int Y, int W, int H, const char* label) :
	page_dialog(X, Y, W, H, label) 
{
	// Set defaults
	log_font_ = 0;
	log_size_ = FL_NORMAL_SIZE;
	tip_duration_ = Fl_Tooltip::delay();
	tip_font_ = Fl_Tooltip::font();
	tip_size_ = Fl_Tooltip::size();
	session_elapse_ = 30.0;

	do_creation(X, Y);
}

// Destructor
user_dialog::~user_dialog() {}

// Load values from settings_
void user_dialog::load_values() {
	Fl_Preferences user_settings(settings_, "User Settings");
	// Log table
	Fl_Preferences log_settings(user_settings, "Log Table");
	log_settings.get("Font Name", (int&)log_font_, log_font_);
	log_settings.get("Font Size", (int&)log_size_, log_size_);
	log_settings.get("Session Gap", session_elapse_, session_elapse_);
	// Tooltip
	Fl_Preferences tip_settings(user_settings, "Tooltip");
	tip_settings.get("Duration", tip_duration_, Fl_Tooltip::delay());
	tip_settings.get("Font Name", (int&)tip_font_, Fl_Tooltip::font());
	tip_settings.get("Font Size", (int&)tip_size_, Fl_Tooltip::size());
	tip_settings.get("Enabled", (int&)tip_enabled_, Fl_Tooltip::enabled());
	// Tree views
	Fl_Preferences tree_settings(user_settings, "Tree Views");
	tree_settings.get("Font Name", (int&)tree_font_, 0);
	tree_settings.get("Font Size", (int&)tree_size_, FL_NORMAL_SIZE);
}

// Used to create the form
void user_dialog::create_form(int X, int Y) {
	begin();
	// Group 1 - log_table
	int pos_x = X + GAP;
	int pos_y = Y + GAP;
	// Log table 
	Fl_Group* g1 = new Fl_Group(pos_x, pos_y, 0, 10, "Log Table");
	g1->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g1->labelfont(FL_BOLD);
	g1->labelsize(FL_NORMAL_SIZE + 2);
	g1->box(FL_BORDER_BOX);
	// Add font browser
	pos_x += GAP;
	pos_y += HTEXT;
	// Browser to select font
	Fl_Hold_Browser* br1 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br1->tooltip("Please select the font used in the cells in all log table views");
	populate_font(br1, &log_font_);
	// Add font size browser
	pos_x += br1->w();
	Fl_Hold_Browser* br2 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
	br2->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br2->callback(cb_br_size, &log_size_);
	br2->tooltip("Please select the font size used in the cells in all log table views");
	br1->callback(cb_br_logfont, br2);
	populate_size(br2, &log_font_, &log_size_);
	pos_y = br2->y() + br2->h() + HTEXT;
	pos_x = br1->x();
	// Counter valuator - Session gap in 10 minute intervals
	Fl_Counter* val0 = new Fl_Counter(pos_x, pos_y, WEDIT, HBUTTON, "Session elapse time (minutes)");
	val0->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	val0->type(FL_SIMPLE_COUNTER);
	val0->step(10.0);
	val0->range(10.0, 120.0);
	val0->value(session_elapse_);
	val0->callback(cb_value<Fl_Counter, float>, &session_elapse_);
	// End group - fit to size
	pos_y = val0->y() + val0->h() + GAP;
	pos_x = max(br2->x() + br2->w(), val0->x() + val0->w()) + GAP;
	g1->resizable(nullptr);
	g1->size(pos_x - g1->x(), pos_y - g1->y());
	g1->end();


	// Group 2 - tool tip - align on above group
	pos_y += GAP;
	pos_x = g1->x();
	Fl_Group* g2 = new Fl_Group(pos_x, pos_y, 10, 10, "Tooltips");
	g2->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g2->labelfont(FL_BOLD);
	g2->labelsize(FL_NORMAL_SIZE + 2);
	g2->box(FL_BORDER_BOX);
	// Add Enable
	pos_x = g2->x() + GAP;
	pos_y = g2->y() + HTEXT;
	Fl_Check_Button* bn2_0 = new Fl_Check_Button(pos_x, pos_y, HBUTTON, HBUTTON, "Enabled");
	bn2_0->align(FL_ALIGN_RIGHT);
	bn2_0->tooltip("Check to allow tooltips to be displayed");
	bn2_0->callback(cb_value<Fl_Check_Button, bool>, &tip_enabled_);
	bn2_0->value(tip_enabled_);
	// Add font browser
	pos_y += HTEXT;
	Fl_Hold_Browser* br3 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br3->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br3->tooltip("Please select the font used in the cells in all tooltips");
	populate_font(br3, &tip_font_);
	// Add font size browser
	pos_x += br3->w();
	Fl_Hold_Browser* br4 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
	br4->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br4->callback(cb_br_size, &tip_size_);
	br4->tooltip("Please select the font size used in the cells in all tooltips");
	br3->callback(cb_br_tipfont, br4);
	populate_size(br4, &tip_font_, &tip_size_);
	pos_y = br4->y() + br4->h() + HTEXT;
	pos_x = br3->x();
	// Counter valuator - 1 to 15 s in 0.5 s steps
	Fl_Counter* val1 = new Fl_Counter(pos_x, pos_y, br3->w(), HBUTTON, "Duration (s)");
	val1->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	val1->type(FL_SIMPLE_COUNTER);
	val1->step(0.5);
	val1->range(1.0, 15.0);
	val1->value(tip_duration_);
	val1->callback(cb_value<Fl_Counter, float>, &tip_duration_);
	val1->tooltip("Please select the time (in seconds) that a tooltip will display");
	// End group - fit to size
	g2->resizable(nullptr);
	pos_y = val1->y() + val1->h() + GAP;
	pos_x = max(br4->x() + br4->w(), val1->x() + val1->w()) + GAP;
	g2->size(pos_x - g2->x(), pos_y - g2->y());
	g2->end();

	// Group 3 - tree views (report, prefix and specification)
	pos_y += GAP;
	pos_x = g2->x();
	Fl_Group* g4 = new Fl_Group(pos_x, pos_y, 0, 10, "Tree views");
	g4->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	g4->labelfont(FL_BOLD);
	g4->labelsize(FL_NORMAL_SIZE + 2);
	g4->box(FL_BORDER_BOX);
	// Add font browser
	pos_x += GAP;
	pos_y += HTEXT;
	Fl_Hold_Browser* br7 = new Fl_Hold_Browser(pos_x, pos_y, WEDIT, HMLIN, "Font");
	br7->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	br7->tooltip("Please select the font used in the cells in all tree views");
	populate_font(br7, &tree_font_);
	// Add font size browser
	pos_x += br7->w();
	Fl_Hold_Browser* br8 = new Fl_Hold_Browser(pos_x, pos_y, WBUTTON, HMLIN, "Size");
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
	log_settings.set("Session Gap", session_elapse_);
	// Tell the log views
	log_table::set_font(log_font_, log_size_);
	// Tooltip
	Fl_Preferences tip_settings(user_settings, "Tooltip");
	tip_settings.set("Duration", tip_duration_);
	tip_settings.set("Font Name", (int&)tip_font_);
	tip_settings.set("Font Size", (int&)tip_size_);
	tip_settings.set("Enabled", (int&)tip_enabled_);
	// Tell the tooltips
	Fl_Tooltip::delay(tip_duration_);
	Fl_Tooltip::font(tip_font_);
	Fl_Tooltip::size(tip_size_);
	Fl_Tooltip::enable(tip_enabled_);
	// Tree view settings
	Fl_Preferences tree_settings(user_settings, "Tree Views");
	tree_settings.set("Font Name", (int&)tree_font_);
	tree_settings.set("Font Size", (int&)tree_size_);
	//((pfx_tree*)tabbed_forms_->get_view(OT_PREFIX))->set_font(tree_font_, tree_size_);
	((report_tree*)tabbed_forms_->get_view(OT_REPORT))->set_font(tree_font_, tree_size_);
	((spec_tree*)tabbed_forms_->get_view(OT_ADIF))->set_font(tree_font_, tree_size_);

	// Now tell all views to update formats
	book_->selection(-1, HT_FORMAT);

	settings_->flush();
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
	// Only get FLTK default fonts
	for (int i = 0; i < FL_FREE_FONT; i++) {
		// Contains any combination of FL_BOLD and FL_ITALIC
		const char* name = Fl::get_font_name(Fl_Font(i), nullptr);
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


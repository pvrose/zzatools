#include "qso_log_info.h"
#include "drawing.h"
#include "book.h"
#include "import_data.h"
#include "ticker.h"
#include "utils.h"

#include <FL/Fl_Fill_Dial.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>

extern book* book_;
extern import_data* import_data_;
extern ticker* ticker_;

// Constructor
qso_log_info::qso_log_info(int X, int Y, int W, int H, const char* l) :
	Fl_Group(X, Y, W, H, l)
{
	// Log status group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	load_values();
	create_form(X, Y);
	enable_widgets();

	// Add 1s clock
	ticker_->add_ticker(this, cb_ticker, 1);
}

// Destructor
qso_log_info::~qso_log_info() {
	ticker_->remove_ticker(this);
}

// get settings 
void qso_log_info::load_values() {
	// Nothing yet
}

// Create form
void qso_log_info::create_form(int X, int Y) {
	int curr_x = X + GAP;
	int curr_y = Y;

	// Display progress in loading or saving the log
	pr_loadsave_ = new Fl_Button(curr_x, curr_y, HBUTTON, HBUTTON, nullptr);
	pr_loadsave_->color(FL_BACKGROUND_COLOR, FL_BLUE);
	pr_loadsave_->box(FL_OVAL_BOX);
	pr_loadsave_->tooltip("Displays loading or saving progress");

	curr_x += HBUTTON;

	// Display current status of the log
	op_status_ = new Fl_Output(curr_x, curr_y, WSMEDIT - HBUTTON, HBUTTON + 2);
	op_status_->box(FL_FLAT_BOX);
	op_status_->color(FL_BACKGROUND_COLOR);
	op_status_->textsize(FL_NORMAL_SIZE + 2);
	op_status_->textfont(FL_BOLD);

	curr_y += op_status_->h();
	
	int max_x = pr_loadsave_->x() + pr_loadsave_->w() + GAP;

	curr_x = X + GAP;

	// Check to enable saving after each QSO
	bn_save_enable_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Save after each QSO");
	bn_save_enable_->align(FL_ALIGN_RIGHT);
	bn_save_enable_->tooltip("Enable/Disable save");
	bn_save_enable_->callback(cb_bn_enable);
	bn_save_enable_->value(true);

	curr_x += WSMEDIT;
	// Button to trigger saving the log
	bn_save_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Save!");
	bn_save_->align(FL_ALIGN_CENTER);
	bn_save_->tooltip("Force save now");
	bn_save_->callback(cb_bn_save);
	
	curr_x = X + GAP;
	curr_y += bn_save_enable_->h() + GAP;

	max_x = max(max_x, curr_x);


	resizable(nullptr);
	size(max_x - x(), curr_y - y());
	end();
}

// Enable widgets
void qso_log_info::enable_widgets() {
	// Depending on state of book, output status, load/save progress, enable save button
	if (book_->empty()) {
		op_status_->value("No Data");
		pr_loadsave_->color(FL_BACKGROUND_COLOR);
		bn_save_->deactivate();
	}
	else if (book_->storing()) {
		op_status_->value("Storing");
		pr_loadsave_->color(fl_color_average(FL_GREEN, FL_RED, (float)book_->get_complete()));
		bn_save_->deactivate();
	}
	else if (book_->loading()) {
		op_status_->value("Loading");
		pr_loadsave_->color(fl_color_average(FL_GREEN, FL_BACKGROUND_COLOR, (float)book_->get_complete()));
		bn_save_->deactivate();
	} else {
		if (book_->modified()) {
			op_status_->value("Modified");
			pr_loadsave_->color(FL_RED);
			pr_loadsave_->value(0.0);
			bn_save_->activate();
		}
		else {
			op_status_->value("Unmodified");
			pr_loadsave_->color(FL_GREEN);
			pr_loadsave_->value(0.0);
			bn_save_->deactivate();
		}
		if (book_->enable_save()) {
			bn_save_enable_->value(true);
		}
		else {
			bn_save_enable_->value(false);
		}
	}
	redraw();
}

// Save changes
void qso_log_info::save_values() {
	// No content yet
}

// On 1s ticker - reevaluate the widgets
void qso_log_info::cb_ticker(void* v) {
	((qso_log_info*)v)->enable_widgets();
}

// Callback on Save Enable button
// v is not used
void qso_log_info::cb_bn_enable(Fl_Widget* w, void* v) {
	qso_log_info* that = ancestor_view<qso_log_info>(w);
	bool value = ((Fl_Check_Button*)w)->value();
	if (!value) {
		book_->enable_save(false, "Diabling save from QSO Manager");
	}
	else {
		while (!book_->enable_save()) book_->enable_save(true, "Enabling save from QSO manager");
	}
	that->enable_widgets();
}

// Callback to force save
// v is not used
void qso_log_info::cb_bn_save(Fl_Widget* w, void* v) {
	book_->store_data();
}


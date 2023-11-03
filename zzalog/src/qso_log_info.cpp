#include "qso_log_info.h"
#include "drawing.h"
#include "book.h"
#include "import_data.h"

extern book* book_;
extern import_data* import_data_;

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
}

qso_log_info::~qso_log_info() {

}

// get settings 
void qso_log_info::load_values() {
	// Nothing yet
}

// Create form
void qso_log_info::create_form(int X, int Y) {
	int curr_x = X + GAP;
	int curr_y = Y + 1;

	op_status_ = new Fl_Output(curr_x, curr_y, WSMEDIT, HBUTTON + 2);
	op_status_->box(FL_FLAT_BOX);
	op_status_->color(FL_BACKGROUND_COLOR);
	op_status_->textsize(FL_NORMAL_SIZE + 2);
	op_status_->textfont(FL_BOLD);

	curr_y += op_status_->h();
	int max_x = op_status_->x() + op_status_->w() + GAP;

	pr_loadsave_ = new Fl_Progress(curr_x, curr_y, WSMEDIT, HBUTTON);
	pr_loadsave_->color(FL_BACKGROUND_COLOR, FL_BLUE);
	pr_loadsave_->tooltip("Displays loading or saving progress");
	pr_loadsave_->minimum(0.0);
	pr_loadsave_->maximum(1.0);

	curr_y += pr_loadsave_->h();
	max_x = max(max_x, pr_loadsave_->x() + pr_loadsave_->w() + GAP);

	bn_save_enable_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Save after each QSO");
	bn_save_enable_->align(FL_ALIGN_RIGHT);
	bn_save_enable_->tooltip("Enable/Disable save");
	bn_save_enable_->callback(cb_bn_enable);
	bn_save_enable_->value(true);

	curr_x += WSMEDIT;
	bn_save_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Save!");
	bn_save_->align(FL_ALIGN_CENTER);
	bn_save_->tooltip("Force save now");
	bn_save_->callback(cb_bn_save);
	
	curr_x = X + GAP;
	curr_y += bn_save_enable_->h() + GAP;
	bn_auto_update_ = new Fl_Check_Button(curr_x, curr_y, HBUTTON, HBUTTON, "Auto-update");
	bn_auto_update_->align(FL_ALIGN_RIGHT);
	bn_auto_update_->tooltip("Enable/Disable auto-update");
	bn_auto_update_->callback(cb_bn_auto);

	curr_x += WSMEDIT;
	op_update_ = new Fl_Output(curr_x, curr_y, WBUTTON, HBUTTON);
	op_update_->box(FL_FLAT_BOX);
	op_update_->color(FL_BACKGROUND_COLOR);
	op_update_->value("");

	curr_y += bn_auto_update_->h() + GAP;


	curr_x += op_update_->w() + GAP;

	max_x = max(max_x, curr_x);


	resizable(nullptr);
	size(max_x - x(), curr_y - y());
	end();
}

// Enable widgets
void qso_log_info::enable_widgets() {

	if (book_->empty()) {
		op_status_->value("No Data");
		pr_loadsave_->color(FL_BACKGROUND_COLOR, FL_BACKGROUND_COLOR);
		pr_loadsave_->value(0.0);
		bn_save_->deactivate();
	}
	else if (book_->storing()) {
		op_status_->value("Storing");
		pr_loadsave_->color(FL_GREEN, FL_RED);
		pr_loadsave_->value(1.0F - (float)book_->get_complete());
		bn_save_->deactivate();
	}
	else if (book_->loading()) {
		op_status_->value("Loading");
		pr_loadsave_->color(FL_BACKGROUND_COLOR, FL_GREEN);
		pr_loadsave_->value((float)book_->get_complete());
		bn_save_->deactivate();
	} else {
		if (book_->modified()) {
			op_status_->value("Modified");
			pr_loadsave_->color(FL_RED, FL_RED);
			pr_loadsave_->value(0.0);
			bn_save_->activate();
		}
		else {
			op_status_->value("Unmodified");
			pr_loadsave_->color(FL_GREEN, FL_GREEN);
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
	if (import_data_->is_auto_update()) {
		bn_auto_update_->value(true);
	} else {
		bn_auto_update_->value(false);
	}
}

// Save changes
void qso_log_info::save_values() {
	// No content yet
}

// On 1s ticker - reevaluate the widgets
void qso_log_info::ticker() {
	enable_widgets();
}

// Callback on Save Enable button
void qso_log_info::cb_bn_enable(Fl_Widget* w, void* v) {
	qso_log_info* that = ancestor_view<qso_log_info>(w);
	bool value = ((Fl_Check_Button*)w)->value();
	if (!value) {
		book_->enable_save(false);
	}
	else {
		while (!book_->enable_save()) book_->enable_save(true);
	}
	that->enable_widgets();
}

// Callback to force save
void qso_log_info::cb_bn_save(Fl_Widget* w, void* v) {
	book_->store_data();
}

// Callback on auto-state
void qso_log_info::cb_bn_auto(Fl_Widget* w, void* v) {
	qso_log_info* that = ancestor_view<qso_log_info>(w);
	if (((Fl_Check_Button*)w)->value()) {
		import_data_->start_auto_update();
	} else {
		import_data_->stop_update(true);
	}
	that->enable_widgets();
}

// Update auto_updaye
void qso_log_info::auto_source(const char* source) {
	op_update_->value(source);
}
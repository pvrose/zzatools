#include "qso_qth.h"
#include "qso_data.h"
#include "qso_manager.h"
#include "qth_dialog.h"
#include "spec_data.h"
#include "drawing.h"
#include "record.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>

extern spec_data* spec_data_;
extern bool DARK;

// Constructor
qso_qth::qso_qth(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
	, qth_details_(nullptr)
	, qth_name_("")
{
	// CAT control group
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	//align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

	load_values();
	create_form(X, Y);
}

// Destructor
qso_qth::~qso_qth() {
	save_values();
}

// LOad settings
void qso_qth::load_values() {
}

// Create the widgets
void qso_qth::create_form(int X, int Y) {
	int curr_x = X + GAP;
	int curr_y = Y + 1;
	int max_x = curr_x;

	int width = w() - GAP - GAP;

	// NAme of the APP_MY_QTH macro
	op_name_ = new Fl_Output(curr_x, curr_y, width, HTEXT);
	op_name_->box(FL_FLAT_BOX);
	op_name_->color(FL_BACKGROUND_COLOR);
	op_name_->textfont(FL_BOLD);
	op_name_->textsize(FL_NORMAL_SIZE + 2);

	max_x = max(curr_x, op_name_->x() + op_name_->y());
	curr_y += HTEXT;

	// Description
	op_descr_ = new Fl_Output(curr_x, curr_y, width, HTEXT);
	op_descr_->box(FL_FLAT_BOX);
	op_descr_->color(FL_BACKGROUND_COLOR);
	op_descr_->textfont(FL_BOLD);
	op_descr_->textsize(FL_NORMAL_SIZE + 1);
	op_descr_->tooltip("A description of the QTH");
	max_x = max(curr_x, op_descr_->x() + op_descr_->w());
	
	curr_y += HTEXT + HTEXT;
	int save_y = curr_y;

	curr_y = y() + h() - GAP - HBUTTON;

	// Edit button
	bn_edit_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Edit");
	// bn_edit_->color(COLOUR_MAUVE);
	// bn_edit_->labelcolor(FL_BLACK);
	bn_edit_->callback(cb_bn_edit, nullptr);
	bn_edit_->tooltip("Open window to allow QTH to be edited");
	int save_x = curr_x;
	curr_x += bn_edit_->w() + GAP;

	// Copy button
	bn_copy_ = new Fl_Button(curr_x, curr_y, WBUTTON, HBUTTON, "Copy");
	bn_copy_->callback(cb_bn_copy, nullptr);
	bn_copy_->tooltip("Expand macro into the current QSO");

	curr_x = save_x;
	max_x = max(max_x, bn_copy_->x() + bn_copy_->w());


	// Table showing details
	table_ = new table(curr_x, save_y, width, curr_y - save_y, "Macro definition");
	table_->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
	table_->tooltip("The macro substitution used for this QTH");
	max_x = max(max_x, table_->x() + table_->w());

	end();

	show();
	enable_widgets();
}

// Save settings
void qso_qth::save_values() {
	// No action
}

// Enable widgets
void qso_qth::enable_widgets() {
	// If we have details then display them
	if (qth_details_) {
		op_name_->value(qth_name_.c_str());
		op_descr_->value(qth_details_->item("APP_ZZA_QTH_DESCR").c_str());
		table_->set_data(qth_details_);
	}
	else {
		op_name_->value("No QTH supplied");
		op_descr_->value("");
		table_->set_data(nullptr);
	}
	// TODO - just to get it working for release at initialisation - needs deeper thought
	bn_edit_->activate();
	//if (qd->qso_editing(qd->current_number())) bn_edit_->activate();
	//else bn_edit_->deactivate();
	table_->redraw();
	redraw();
}

// Set the QTH details
void qso_qth::set_qth(string name) {
	if (name.length()) {
		qth_name_ = name;
	} else {
		qso_manager* mgr = ancestor_view<qso_manager>(this);
		qth_name_ = mgr->get_default(qso_manager::QTH);
	}
	qth_details_ = spec_data_->expand_macro("APP_ZZA_QTH", qth_name_);
}

// Callback - edit button
// v is not used
void qso_qth::cb_bn_edit(Fl_Widget* w, void* v) {
	qso_qth* that = ancestor_view<qso_qth>(w);
	qso_data* qd = ancestor_view<qso_data>(that);
	// Open QTH dialog
	qth_dialog* dlg = new qth_dialog(that->qth_name_);
	switch (dlg->display()) {
	case BN_OK: 
	{
		set<string> changed_fields = spec_data_->get_macro_changes();
		record* qso = qd->current_qso();
		if (qso && qso->item("APP_ZZA_QTH") == that->qth_name_) {
			for (auto fx = changed_fields.begin(); fx != changed_fields.end(); fx++) {
				qso->item(*fx, that->qth_details_->item(*fx));
			}
			qd->update_qso(qd->current_number());
		}
		that->enable_widgets();
		break;
	}
	case BN_CANCEL:
		break;
	default:
		break;
	}
	delete dlg;
	that->enable_widgets();
}

// Copy the macro expansion into current QSO
void qso_qth::cb_bn_copy(Fl_Widget* w, void* v) {
	qso_qth* that = ancestor_view<qso_qth>(w);
	qso_data* qd = ancestor_view<qso_data>(that);
	qd->action_expand_macro("APP_ZZA_QTH");
	that->enable_widgets();
}

// QSO details table: constructor
qso_qth::table::table(int X, int Y, int W, int H, const char* L) :
	Fl_Table_Row(X, Y, W, H, L)
	, macro_(nullptr)
{
	fields_.clear();
	type(SELECT_SINGLE);
	cols(2);
	col_width_all(w() / cols());
	end();
}

// Destructor
qso_qth::table::~table() {
}

// Draw table
// Column 0: Field name 
// Column 1: Field value
void qso_qth::table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
	string text;

	switch (context) {

	case CONTEXT_STARTPAGE:
		// Set the table font
		fl_font(0, FL_NORMAL_SIZE);
		return;

	case CONTEXT_ENDPAGE:
		// Do nothing
		return;


	case CONTEXT_CELL:
		// Column indicates which record, row the field
		fl_push_clip(X, Y, W, H);
		{
			Fl_Color bg_colour = FL_BACKGROUND_COLOR;
			fl_color(bg_colour);
			fl_rectf(X, Y, W, H);
			// TEXT
			Fl_Color fg_colour = FL_FOREGROUND_COLOR;
			if (!active_r()) fg_colour = fl_inactive(fg_colour);
			fl_color(fg_colour);
			string field_name = fields_[R];
			switch (C) {
			case 0:
				fl_font(FL_ITALIC | FL_BOLD, FL_NORMAL_SIZE);
				text = fields_[R];
				break;
			case 1:
				fl_font(0, FL_NORMAL_SIZE);
				text = macro_->item(fields_[R]);
				break;
			}
			fl_draw(text.c_str(), X + 1, Y, W - 1, H, FL_ALIGN_LEFT);

			// BORDER
			fl_color(COLOUR_GREY);
			// draw top and right edges only
			fl_line(X, Y, X + W - 1, Y, X + W - 1, Y + H - 1);
			fl_pop_clip();
			return;
		}
	default:
		break;
	}
}

// Set the database for the table
void qso_qth::table::set_data(record* macro) {
	macro_ = macro;
	fields_.clear();
	if (macro_) {
		// Copy each of the field definitions in the macro into the dataset
		// (but not APP_ZZA_QTH_DESCR as that is already being shown)
		for (auto it = macro_->begin(); it != macro_->end(); it++) {
			if ((*it).first != "APP_ZZA_QTH_DESCR" && (*it).second.length()) {
				fields_.push_back((*it).first);
			}
		}
		rows(fields_.size());
		row_height_all(ROW_HEIGHT);
	}
	else {
		rows(0);
	}
	redraw();
}

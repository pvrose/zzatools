#include "QBS_start.h"
#include "QBS_consts.h"
#include "QBS_data.h"
#include "QBS_window.h"
#include "QBS_reader.h"
#include "QBS_writer.h"
#include "QBS_process.h"
#include "../zzalib/utils.h"
#include "../zzalib/callback.h"

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>

using namespace zzalib;

extern Fl_Preferences* settings_;
extern QBS_data* the_data_;

// Constructor 
QBS_start::QBS_start(int X, int Y, int W, int H, const char* L) :
	Fl_Group(X, Y, W, H, L)
{
	window_ = ancestor_view<QBS_window>(this);
	load_values();
	create_form();
	configure_widgets();

}

QBS_start::~QBS_start() {
}

// Load values - also load data if available
void  QBS_start::load_values() {
}

// Save values - also write data
void QBS_start::save_values() {
}

// Create form
void QBS_start::create_form() {
	align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	labelfont(FONT | FL_ITALIC);
	labelsize(FONT_SIZE * 3 / 2);

	begin();
	int curr_x = x() + GAP;
	int curr_y = y() + GAP + fl_height(FONT, FONT_SIZE * 2);
	int max_x = curr_x;
	// Set font for width calculations
	fl_font(FONT, FONT_SIZE);

	curr_y += fl_height(FONT, FONT_SIZE);
	op_filename_ = new Fl_Output(curr_x, curr_y, WBUTTON * 4, HBUTTON, "Current file");
	op_filename_->labelfont(FONT);
	op_filename_->labelsize(FONT_SIZE);
	op_filename_->textfont(FONT);
	op_filename_->textsize(FONT_SIZE);
	op_filename_->align(FL_ALIGN_TOP);
	op_filename_->value(ancestor_view<QBS_window>(this)->filename_.c_str());

	max_x = max(max_x, op_filename_->x() + op_filename_->w());
	curr_y += op_filename_->h() + GAP;

	Fl_Button* bn1 = new Fl_Button(curr_x, curr_y, WBUTTON*4, HBUTTON, "Import from spreadsheet files");
	bn1->labelfont(FONT);
	bn1->labelsize(FONT_SIZE);
	bn1->align(FL_ALIGN_INSIDE);
	bn1->callback(cb_bn_import, nullptr);

	curr_y += bn1->h() + GAP;
	max_x = max(max_x, curr_x + bn1->w());

	Fl_Button* bn2 = new Fl_Button(curr_x, curr_y, WBUTTON*4, HBUTTON, "Load XML data");
	bn2->labelfont(FONT);
	bn2->labelsize(FONT_SIZE);
	bn2->align(FL_ALIGN_INSIDE);
	bn2->callback(cb_bn_loadxml, nullptr);

	curr_y += bn2->h() + GAP;
	max_x = max(max_x, curr_x + bn2->w());

	Fl_Button* bn3 = new Fl_Button(curr_x, curr_y, WBUTTON*4, HBUTTON, "Save XML data");
	bn3->labelfont(FONT);
	bn3->labelsize(FONT_SIZE);
	bn3->align(FL_ALIGN_INSIDE);
	bn3->callback(cb_bn_savexml, nullptr);

	curr_y += bn3->h() + GAP;
	max_x = max(max_x, curr_x + bn3->w());

	Fl_Button* bn4 = new Fl_Button(curr_x, curr_y, WBUTTON*4, HBUTTON, "Process cards and SASEs and envelopes");
	bn4->labelfont(FONT);
	bn4->labelsize(FONT_SIZE);
	bn4->align(FL_ALIGN_INSIDE);
	bn4->callback(cb_bn_process, nullptr);

	curr_y += bn4->h() + GAP;
	max_x = max(max_x, curr_x + bn4->w());

	Fl_Button* bn7 = new Fl_Button(curr_x, curr_y, WBUTTON*4, HBUTTON, "Review database");
	bn7->labelfont(FONT);
	bn7->labelsize(FONT_SIZE);
	bn7->align(FL_ALIGN_INSIDE);
	bn7->callback(cb_bn_review, nullptr);

	curr_y += bn7->h() + GAP;
	max_x = max(max_x, curr_x + bn7->w());

	max_x += GAP;
	end();

	// Resize the group (& parent window) to fit
	resizable(nullptr);
	size(max_x - x(), curr_y - y());
}

// Import button callback
void QBS_start::cb_bn_import(Fl_Widget* w, void* v) {
	// Clear down database
	QBS_start* that = ancestor_view<QBS_start>(w);
	that->window_->data_->clear();
	// Open "import spreadsheet" screen

	that->window_->show_view(QBS_window::IMPORT_SS);
}

// Load file callback
// launch file browser and tell data to load
void QBS_start::cb_bn_loadxml(Fl_Widget* w, void* v) {
	// Load file
	QBS_start* that = ancestor_view<QBS_start>(w);
	that->window_->load_xml(true);
}

// SAve file callback
// launch file browser
void QBS_start::cb_bn_savexml(Fl_Widget* w, void* v) {
	QBS_start* that = ancestor_view<QBS_start>(w);
	that->window_->save_xml(true);
}


// TODO: Code these place-holders
void QBS_start::cb_bn_process(Fl_Widget* w, void* v) {
	// Open "import spreadsheet" screen
	QBS_start* that = ancestor_view<QBS_start>(w);
	that->window_->show_view(QBS_window::PROCESS);
}

void QBS_start::cb_bn_review (Fl_Widget* w, void* v) {
	// Open "import spreadsheet" screen
	QBS_start* that = ancestor_view<QBS_start>(w);
	that->window_->show_view(QBS_window::PROCESS);
}

// Configure widgets
void QBS_start::configure_widgets() {};

// Put filename
void QBS_start::filename(string value) {
	filename_ = value;
	op_filename_->value(filename_.c_str());
}


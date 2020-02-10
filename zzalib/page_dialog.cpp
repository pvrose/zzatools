#include "page_dialog.h"

#include "utils.h"

#include <FL/Fl_Check_Button.H>

using namespace zzalib;

// Default constructor
page_dialog::page_dialog(int X, int Y, int W, int H, const char* label) :
	Fl_Group(X, Y, W, H, label)
{
	callback(cb_bn_ok);
}

// Provides a reference creation process
void page_dialog::do_creation(int X, int Y) { 
	// Load initial configuration - typically from settings
	this->load_values();
	// Construct the dialog from its widgets
	this->create_form(X, Y);
	// Enable or disable widgets depending on values of various attributes
	this->enable_widgets();
}

// Default destructor
page_dialog::~page_dialog()
{
	clear();
}

// Default OK button - save confiuguation in settings
void page_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	page_dialog* that = (page_dialog*)w;
	that->save_values();
}

// Default enable button 
void page_dialog::cb_ch_enable(Fl_Widget* w, void* v) {
	page_dialog* that = ancestor_view<page_dialog>(w);
	cb_value<Fl_Check_Button, bool>(w, v);
	that->enable_widgets();
}
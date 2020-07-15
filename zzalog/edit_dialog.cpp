#include "edit_dialog.h"
#include "../zzalib/utils.h"
#include "intl_dialog.h"

#include "../zzalib/callback.h"

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Input.H>

using namespace zzalog;
using namespace zzalib;

extern intl_dialog* intl_dialog_;


// Edit input - create the input to the supplied size
edit_dialog::edit_input::edit_input(int X, int Y, int W, int H) :
	Fl_Input(X, Y, W, H) 
{

}

// Handler for mouse and keyboard events
int edit_dialog::edit_input::handle(int event) {
	// Tell the international character dialog this input will accept pastes
	intl_dialog_->editor(this);
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		// mouse coming in or going out of focus on this view
		// tell FLTK we've acknowledged it so we can receive keyboard events (or not)
		return true;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case FL_Tab:
			// Tab key - save and open new edit 
			if (Fl::event_state(FL_SHIFT)) {
				// SHIFT-TAB - save and select cell to the left
				cb_menu(this, (void*)SAVE_PREV);
			}
			else {
				// TAB - save and select cell to right
				cb_menu(this, (void*)SAVE_NEXT);
			}
			return true;
		case FL_Enter:
			// ENTER - save and close
			cb_menu(this, (void*)SAVE);
			return true;
		case FL_Up:
			// Up arrow - save and select record above
			cb_menu(this, (void*)SAVE_UP);
			break;
		case FL_Down:
			// Down arrow - save and select record above
			cb_menu(this, (void*)SAVE_DOWN);
			break;
		}
	}
	return Fl_Input::handle(event);
}

// Constructor
edit_dialog::edit_dialog(int X, int Y, int W, int H) :
	win_dialog(W + 2 * H, H, nullptr)
	, value_("")
	, ip_(nullptr)
{
	// Position window in screen on where it has been asked to go 
	position(X, Y);
	// No border
	clear_border();
	// Input widget 
	edit_input* ip = new edit_input(0, 0, W, H);
	ip->textfont(FONT);
	ip->textsize(FONT_SIZE);
	ip->box(FL_DOWN_BOX);
	ip->callback(cb_value<edit_input, string>, &value_);
	ip->when(FL_WHEN_CHANGED);
	ip_ = ip;
	// Menu button - popup menu on R buttons - place it coterminous with the input box
	Fl_Menu_Button* mb = new Fl_Menu_Button(0, 0, W, H, "Edit");
	// Popup means the button isn't drawn, but it is clickable
	mb->type(Fl_Menu_Button::POPUP3);
	mb->textsize(FONT_SIZE);
	mb->box(FL_NO_BOX);
	mb->add("&UPPER", 0, cb_menu, (void*)UPPER);
	mb->add("&lower", 0, cb_menu, (void*)LOWER);
	mb->add("&Mixed", 0, cb_menu, (void*)MIXED);
	// OK button
	Fl_Button* bn_ok = new Fl_Button(W, 0, H, H);
	bn_ok->color(FL_GREEN);
	bn_ok->callback(cb_menu, (void*)SAVE);
	// Cancel button
	Fl_Button* bn_ng = new Fl_Button(W + H, 0, H, H);
	bn_ng->color(FL_RED);
	bn_ng->callback(cb_menu, (void*)CANCEL);
	end();
	// THis should keep it on top but not hog the events
	set_non_modal();
}

edit_dialog::~edit_dialog() {

}

// Callback when right click menu is clicked - convert selected text to upper, lower or mixed-case
// Also called for OK and Cancel buttons and on certain keyboard events
void edit_dialog::cb_menu(Fl_Widget* w, void* v) {
	edit_dialog* that = ancestor_view<edit_dialog>(w);
	Fl_Input* ip = (Fl_Input*)that->ip_;
	unsigned int p = ip->position();
	unsigned int m = p;
	while (m > 0 && that->value_[m] != ' ') m--;
	while (p < that->value_.length() && that->value_[p] != ' ') p++;
	switch ((menu_item_t)(long)v) {
	case UPPER:
		for (unsigned int i = m; i < p; i++) 
			that->value_[i] = toupper(that->value_[i]);
		break;
	case LOWER:
		for (unsigned int i = m; i < p; i++)
			that->value_[i] = tolower(that->value_[i]);
		break;
	case MIXED:
		that->value_[m] = toupper(that->value_[m]);
		for (unsigned int i = m + 1; i < p; i++)
			that->value_[i] = tolower(that->value_[i]);
		break;
	case SAVE:
		that->do_button(BN_OK);
		break;
	case CANCEL:
		that->do_button(BN_CANCEL);
		break;
	case SAVE_NEXT:
		that->do_button(BN_SPARE);
		break;
	case SAVE_PREV:
		that->do_button((button_t)(BN_SPARE + 1));
		break;
	case SAVE_UP:
		that->do_button((button_t)(BN_SPARE + 2));
		break;
	case SAVE_DOWN:
		that->do_button((button_t)(BN_SPARE + 3));
		break;
	}
	ip->value(that->value_.c_str());
	that->redraw();
}

// Display the dialog - after putting text in input
button_t edit_dialog::display(string value) {
	value_ = value;
	((Fl_Input*)ip_)->value(value.c_str());
	return win_dialog::display();
}

// Returns the value
string edit_dialog::value() {
	return value_;
}

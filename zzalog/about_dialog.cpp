#include "about_dialog.h"

#include "spec_data.h"
#include "icons.h"
#include "version.h"
#include "../zzalib/utils.h"
#include "../zzalib/versionh.h"

#include "hamlib/rig.h"

#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>

using namespace zzalog;
using namespace zzalib;

extern spec_data* spec_data_;



// Creates the about box dialog and displays it.
about_dialog::about_dialog() :
	win_dialog(10, 10)
{
	// Position constants 
	const int XG = EDGE;
	// Column 1 has just the program icon
	const int C1 = XG;
	const int ICON = 48;
	// Column 2 - text boxes and OK button
	const int C2 = C1 + ICON + GAP;
	const int W2 = 2 * WEDIT;
	const int WALL = C2 + W2 + EDGE;
	const int YG = EDGE;
	const int R1 = YG;

	// Draw the two text boxes - first program ID and versions
	string program_id = PROGRAM_ID + " " + PROGRAM_VERSION + "\n using ADIF Version " + spec_data_->adif_version() +
		"\n hamlib version " + rig_version() + 
		"\n FLTK version " + to_string(FL_MAJOR_VERSION) + "." + to_string(FL_MINOR_VERSION) + "." + to_string(FL_PATCH_VERSION) + 
		"\n zzalib version " + zzalib::LIBRARY_VERSION;
	string copyright = COPYRIGHT + "\n (Hamlib " + rig_copyright() + ")";
	int w = W2;
	int h = 0;
	// Get the width and height required to display the message (add a bit of height) 
	fl_font(FONT, FONT_SIZE);
	fl_measure(program_id.c_str(), w, h);
	h += 5;
	Fl_Multiline_Output* op1 = new Fl_Multiline_Output(C2, R1, W2, h);
	op1->value(program_id.c_str());
	op1->textfont(FONT);
	op1->textsize(FONT_SIZE);
	op1->wrap(true);
	const int R2 = R1 + h + GAP;
	// Secondly - copyright statement
	h = 0;
	fl_measure(copyright.c_str(), w, h);
	h += 5;
	Fl_Multiline_Output* op2 = new Fl_Multiline_Output(C2, R2, W2, h);
	op2->value(copyright.c_str());
	op2->textfont(FONT);
	op2->textsize(FONT_SIZE);
	op2->wrap(true);

	const int R4 = R2 + h + GAP;
	const int C3 = WALL - EDGE - WBUTTON;
	// And the OK Button
	Fl_Button* bn_ok = new Fl_Button(C3, R4, WBUTTON, HBUTTON, "OK");
	bn_ok->labelfont(FONT);
	bn_ok->labelsize(FONT_SIZE);
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	const int HALL = R4 + HBUTTON + EDGE;

	// now calculate where to put the icon - centralise its Y position
	const int YICON = (HALL - ICON) / 2;
	Fl_Button* bn_icon = new Fl_Button(C1, YICON, ICON, ICON);
	// Icon is defined as a 16*16 RGBA image -
	Fl_RGB_Image icon(ICON_MAIN, 16, 16, 4);
	// Expand it to 48*48
	bn_icon->image(icon.copy(ICON, ICON));
	// No edge to the button
	bn_icon->box(FL_FLAT_BOX);

	// resize the window to include everything
	resizable(nullptr);
	size(WALL, HALL);
	copy_label(string("About " + PROGRAM_ID).c_str());

	end();

	// Set the callback on the close (X) button
	callback(cb_bn_cancel);
}

// Fl_Group destructor destroys all the widgets
about_dialog::~about_dialog()
{
}

// OK button callback - do standard win_dialog OK action
void about_dialog::cb_bn_ok(Fl_Widget* w, void* v) {
	about_dialog* that = ancestor_view<about_dialog>(w);
	that->do_button(BN_OK);
}

// cancel button callback - do standard win_dialog Close action
void about_dialog::cb_bn_cancel (Fl_Widget* w, void* v) {
	about_dialog* that = ancestor_view<about_dialog>(w);
	that->do_button(BN_CANCEL);
}
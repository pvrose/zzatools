#include "about_dialog.h"

#include "spec_data.h"
#include "utils.h"

#include "hamlib/rig.h"
#include <curl/curl.h>

#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_PNG_Image.H>

extern spec_data* spec_data_;
extern std::string PROGRAM_ID;
extern std::string PROGRAM_VERSION;
extern std::string CONTACT;
extern std::string COPYRIGHT;
extern std::string PARTY3RD_COPYRIGHT;
extern std::string TIMESTAMP;
extern Fl_PNG_Image main_icon_;

// Creates the about box dialog and displays it.
about_dialog::about_dialog() :
	win_dialog(10, 10)
{
	// Position constants 
	const int WICON = main_icon_.w();
	const int HICON = main_icon_.h();
	const int XG = EDGE;
	// Column 1 has just the program icon
	const int C1 = XG;
	// Column 2 - text boxes and OK button
	const int C2 = C1 + WICON + GAP;
	const int W2 = 2 * WEDIT;
	const int WALL = C2 + W2 + EDGE;
	const int YG = EDGE;
	const int R1 = YG;

	curl_version_info_data* data = curl_version_info(CURLVERSION_LAST);

	// Draw the two text boxes - first program ID and versions
	std::string program_id =
		"Compiled " + TIMESTAMP + "\n" +
		(spec_data_ ? "using ADIF Version " + spec_data_->adif_version() + "\n" : "") +
		" hamlib version " + rig_version() +
		"\n FLTK version " + to_string(FL_MAJOR_VERSION) + "." +
		to_string(FL_MINOR_VERSION) + "." + to_string(FL_PATCH_VERSION) +
		"\n CURL version " + std::string(data->version) +
		"\n JSON " +
		"\n PUGIXML ";
	std::string copyright = COPYRIGHT + "\ne-mail: " + CONTACT + "\n" + PARTY3RD_COPYRIGHT + "\n (Hamlib " + rig_copyright() + ")";
	int w = W2;
	int h = 0;
	// Get the width and height required to display the message (add a bit of height) 
	fl_font(0, FL_NORMAL_SIZE);
	fl_measure(program_id.c_str(), w, h);
	h += FL_NORMAL_SIZE;
	Fl_Box* op1 = new Fl_Box(C2, R1, W2, h);
	op1->copy_label(program_id.c_str());
	op1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	op1->box(FL_BORDER_BOX);
	const int R2 = R1 + h + GAP;
	// Secondly - copyright statement
	h = 0;
	fl_measure(copyright.c_str(), w, h);
	h += FL_NORMAL_SIZE;
	Fl_Box* op2 = new Fl_Box(C2, R2, W2, h);
	op2->copy_label(copyright.c_str());
	op2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	op2->box(FL_BORDER_BOX);

	const int R4 = R2 + h + GAP;
	const int C3 = WALL - EDGE - WBUTTON;
	// And the OK Button
	Fl_Button* bn_ok = new Fl_Button(C3, R4, WBUTTON, HBUTTON, "OK");
	bn_ok->callback(cb_bn_ok);
	bn_ok->when(FL_WHEN_RELEASE);
	const int HALL = R4 + HBUTTON + EDGE;

	// now calculate where to put the icon - centralise its Y position
	const int YICON = (HALL - HICON) / 2;
	const int YNAME = YICON - HBUTTON;
	Fl_Box* bx_name = new Fl_Box(C1, YNAME, WICON, HBUTTON);
	bx_name->copy_label(PROGRAM_ID.c_str());
	bx_name->labelfont(FL_BOLD);
	bx_name->labelsize(FL_NORMAL_SIZE + 4);
	bx_name->align(FL_ALIGN_CENTER);

	Fl_Button* bn_icon = new Fl_Button(C1, YICON, WICON, HICON);
	// Expand it to 48*48
	bn_icon->image(main_icon_);
	// No edge to the button
	bn_icon->box(FL_FLAT_BOX);

	const int YVERS = YICON + HICON;
	Fl_Box* bx_vers = new Fl_Box(C1, YVERS, WICON, HBUTTON);
	bx_vers->copy_label(PROGRAM_VERSION.c_str());
	bx_vers->labelfont(FL_BOLD);
	bx_vers->labelsize(FL_NORMAL_SIZE + 4);
	bx_vers->align(FL_ALIGN_CENTER);

	// resize the window to include everything
	resizable(nullptr);
	size(WALL, HALL);
	copy_label(std::string("About " + PROGRAM_ID).c_str());

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
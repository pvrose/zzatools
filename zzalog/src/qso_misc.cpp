#include "qso_misc.h"
#include "qso_details.h"
#include "qso_dxcc.h"
#include "contest_scorer.h"
#include "qso_qsl_vwr.h"
#include "drawing.h"
#include "record.h"

#include <FL/Fl_Preferences.H>

extern string VENDOR;
extern string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;

// Constructor
qso_misc::qso_misc(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
	box(FL_BORDER_BOX);
	handle_overflow(OVERFLOW_PULLDOWN);
	load_values();
	create_form();
}

// Destructor
qso_misc::~qso_misc() {
	save_values();
}

// get settings
void qso_misc::load_values() {
	// Load default tab value
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	tab_settings.get("Miscellaneous", default_tab_, 0);

}

// Create form
void qso_misc::create_form() {

	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);
	callback(cb_tabs);

	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);

	// Previous details form
	details_ = new qso_details(rx, ry, rw, rh, "Previous");
	// DXCC and worked before status form
	dxcc_ = new qso_dxcc(rx, ry, rw, rh, "DX?");
	// QSL details form
	qsl_ = new qso_qsl_vwr(rx, ry, rw, rh, "QSL");
	// Contest form
	contest_ = new contest_scorer(rx, ry, rw, rh, "Contest");

	end();
	show();

	value(child(default_tab_));
}

// Enable/disab;e widgets
void qso_misc::enable_widgets() {
	// Set the label of the selected tab to BOLD, others to ITALIC
	for (int ix = 0; ix < children(); ix++) {
		Fl_Widget* wx = child(ix);
		if (wx == value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
	}
	details_->enable_widgets();
	dxcc_->enable_widgets();
	qsl_->enable_widgets();
	contest_->enable_widgets();
}

// save value
void qso_misc::save_values() {
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	// Find the current selected tab and save its index
	Fl_Widget* w = value();
	for (int ix = 0; ix != children(); ix++) {
		if (child(ix) == w) {
			tab_settings.set("Miscellaneous", ix);
		}
	}
}

// set the QSO details into the various forms
void qso_misc::qso(record* qso, qso_num_t number) {
	qso_ = qso;
	details_->set_qso(qso_);
	dxcc_->set_data(qso);
	qsl_->set_qso(qso_, number);
	contest_->check_qso(qso, number);
}

// Callback when changing tabs
void qso_misc::cb_tabs(Fl_Widget* w, void* v) {
	qso_misc* that = ancestor_view<qso_misc>(w);
	that->enable_widgets();
}

// Return the contest form
contest_scorer* qso_misc::contest() {
	return contest_;
}

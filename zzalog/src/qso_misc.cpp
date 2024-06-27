#include "qso_misc.h"
#include "qso_qth.h"
#include "drawing.h"

#include <FL/Fl_Preferences.H>

extern Fl_Preferences* settings_;

// Constructor
qso_misc::qso_misc(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
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
	Fl_Preferences display_settings(settings_, "Display");
	display_settings.get("QSO Entry Tab", default_tab_, 0);

}

// Create form
void qso_misc::create_form() {

	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
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
	// Edit my QTH form
	qth_ = new qso_qth(rx, ry, rw, rh, "My QTH");
	// QSL details form
	qsl_ = new qso_qsl_vwr(rx, ry, rw, rh, "QSL");

	end();

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
	qth_->enable_widgets();
	details_->enable_widgets();
	dxcc_->enable_widgets();
	qsl_->enable_widgets();
}

// save value
void qso_misc::save_values() {
	Fl_Preferences display_settings(settings_, "Display");
	// Find the current selected tab and save its index
	Fl_Widget* w = value();
	for (int ix = 0; ix != children(); ix++) {
		if (child(ix) == w) {
			display_settings.set("QSO Entry Tab", ix);
		}
	}
	settings_->flush();
}

// set the QSO details into the various forms
void qso_misc::qso(record* qso, qso_num_t number) {
	qso_ = qso;
	qth_->set_qth(qso_->item("APP_ZZA_QTH"));
	details_->set_call(qso_->item("CALL"));
	dxcc_->set_data();
	qsl_->set_qso(qso_, number);
}

// Callback when changing tabs
void qso_misc::cb_tabs(Fl_Widget* w, void* v) {
	qso_misc* that = ancestor_view<qso_misc>(w);
	that->enable_widgets();
}

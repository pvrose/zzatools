#include "qso_misc.h"
#include "qso_qth.h"

qso_misc::qso_misc(int X, int Y, int W, int H, const char* L) :
	Fl_Tabs(X, Y, W, H, L)
{
	load_values();
	create_form();
}

qso_misc::~qso_misc() {}

// get settings
void qso_misc::load_values() {}
// Create form
void qso_misc::create_form() {

	align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	box(FL_BORDER_BOX);
	labelfont(FL_BOLD);
	labelsize(FL_NORMAL_SIZE + 2);

	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);

	details_ = new qso_details(rx, ry, rw, rh, "Previous");
	dxcc_ = new qso_dxcc(rx, ry, rw, rh, "DX?");
	qth_ = new qso_qth(rx, ry, rw, rh, "My QTH");

	end();

}
// Enable/disab;e widgets
void qso_misc::enable_widgets() {
	qth_->enable_widgets();
	details_->enable_widgets();
	dxcc_->enable_widgets();
}

// save value
void qso_misc::save_values() {}

void qso_misc::qso(record* qso) {
	qso_ = qso;
	qth_->set_qth(qso_->item("APP_ZZA_QTH"));
	details_->set_call(qso_->item("CALL"));
	dxcc_->set_data();
}


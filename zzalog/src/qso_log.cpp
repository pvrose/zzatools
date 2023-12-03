#include "qso_log.h"

#include <algorithm>

using namespace std;

qso_log::qso_log(int X, int Y, int W, int H, const char* l) :
	Fl_Tabs(X, Y, W, H, l)
{
	labeltype(FL_NO_LABEL);
	callback(cb_tabs);
	load_values();
	create_form(X, Y);
	enable_widgets();

}

qso_log::~qso_log() {
	save_values();
}

// get settings 
void qso_log::load_values() {
}

// Create form
void qso_log::create_form(int X, int Y) {
	int rx = 0;
	int ry = 0;
	int rw = 0;
	int rh = 0;
	client_area(rx, ry, rw, rh, 0);
	int saved_rw = rw;
	int saved_rh = rh;

	log_info_ = new qso_log_info(rx, ry, rw, rh, "Log");
	log_info_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, log_info_->w());
	rh = max(rh, log_info_->h());

	qsl_ctrl_ = new qso_qsl(rx, ry, rw, rh, "On-line QSL");
	qsl_ctrl_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, qsl_ctrl_->w());
	rh = max(rh, qsl_ctrl_->h());

	server_ctrl_ = new qso_server(rx, ry, rw, rh, "Servers");
	server_ctrl_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, server_ctrl_->w());
	rh = max(rh, server_ctrl_->h());

	resizable(nullptr);
	size(w() + rw - saved_rw, h() + rh - saved_rh);
	end();

	for (int ix = 0; ix < children(); ix++) {
		child(ix)->size(rw, rh);
	}
	redraw();
}

// Enable widgets
void qso_log::enable_widgets() {
	for (int ix = 0; ix < children(); ix++) {
		Fl_Widget* wx = child(ix);
		if (wx == value()) {
			wx->labelfont((wx->labelfont() | FL_BOLD) & (~FL_ITALIC));
			wx->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			wx->labelfont((wx->labelfont() & (~FL_BOLD)) | FL_ITALIC);
			wx->labelcolor(COLOUR_GREY);
		}
	}
	log_info_->enable_widgets();
	qsl_ctrl_->enable_widgets();
	server_ctrl_->enable_widgets();
}

// Save changes
void qso_log::save_values() {
	// Null method
}

// 1s clock interface
void qso_log::ticker() {
	log_info_->ticker();
	qsl_ctrl_->ticker();
}

// Callback
void qso_log::cb_tabs(Fl_Widget* w, void* v) {
	qso_log* that = (qso_log*)w;
	that->label(that->value()->label());
	that->enable_widgets();
}

qso_qsl* qso_log::qsl_control() {
	return qsl_ctrl_;
}

qso_log_info* qso_log::log_info() {
	return log_info_;
}

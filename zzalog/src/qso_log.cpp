#include "qso_log.h"
#include "qso_log_info.h"
#include "qso_apps.h"
#include "qso_qsl.h"
#include "qso_bands.h"

#include <algorithm>

#include <FL/Fl_Preferences.H>



extern std::string VENDOR;
extern std::string PROGRAM_ID;
extern Fl_Preferences::Root prefs_mode_;

// Constructor
qso_log::qso_log(int X, int Y, int W, int H, const char* l) :
	Fl_Tabs(X, Y, W, H, l)
{
	labeltype(FL_NO_LABEL);
	box(FL_BORDER_BOX);
	handle_overflow(OVERFLOW_PULLDOWN);
	callback(cb_tabs);
	load_values();
	create_form(X, Y);
	enable_widgets();

}

// Destructor
qso_log::~qso_log() {
	save_values();
}

// get settings 
void qso_log::load_values() {
	// Load default tab value
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	tab_settings.get("Log", default_tab_, 0);
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

	// Log status tab
	log_info_ = new qso_log_info(rx, ry, rw, rh, "Log");
	log_info_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, log_info_->w());
	rh = max(rh, log_info_->h());

	// On-line QSL upload/download tab
	qsl_ctrl_ = new qso_qsl(rx, ry, rw, rh, "QSLs");
	qsl_ctrl_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, qsl_ctrl_->w());
	rh = max(rh, qsl_ctrl_->h());

	// Modem clients tab
	apps_ctrl_ = new qso_apps(rx, ry, rw, rh, "Apps");
	apps_ctrl_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, apps_ctrl_->w());
	rh = max(rh, apps_ctrl_->h());

	// Band-plan display tab
	bands_ = new qso_bands(rx, ry, rw, rh, "Bandplan");
	bands_->labelsize(FL_NORMAL_SIZE + 2);
	rw = max(rw, bands_->w());
	rh = max(rh, bands_->h());

	resizable(nullptr);
	size(w() + rw - saved_rw, h() + rh - saved_rh);
	end();

	for (int ix = 0; ix < children(); ix++) {
		child(ix)->size(rw, rh);
	}

	value(child(default_tab_));

	redraw();
}

// Enable widgets
void qso_log::enable_widgets() {
	// Set standard tab label formats
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
	log_info_->enable_widgets();
	qsl_ctrl_->enable_widgets();
	apps_ctrl_->enable_widgets();
	bands_->enable_widgets();
}

// Save changes
void qso_log::save_values() {
	Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences tab_settings(settings, "Dashboard/Tabs");
	// Find the current selected tab and save its index
	Fl_Widget* w = value();
	for (int ix = 0; ix != children(); ix++) {
		if (child(ix) == w) {
			tab_settings.set("Log", ix);
		}
	}
	bands_->save_values();
}

// Callback on switching tab
void qso_log::cb_tabs(Fl_Widget* w, void* v) {
	qso_log* that = (qso_log*)w;
	that->label(that->value()->label());
	that->enable_widgets();
}

// Return the QSL control widget
qso_qsl* qso_log::qsl_control() {
	return qsl_ctrl_;
}

// Return the log sttaus tab
qso_log_info* qso_log::log_info() {
	return log_info_;
}

// Return Apps widget
qso_apps* qso_log::apps() {
	return apps_ctrl_;
}

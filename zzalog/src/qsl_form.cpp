#include "qsl_form.h"
#include "record.h"
#include "drawing.h"
#include "utils.h"
//#include "qsl_design.h"
#include "status.h"
#include "qso_manager.h"



#include <fstream>

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Preferences.H>



using namespace std;

extern Fl_Preferences* settings_;
extern status* status_;
extern qso_manager* qso_manager_;

// Constructor
qsl_form::qsl_form(int X, int Y, record** records, int num_records) :
	Fl_Group(X, Y, 0, 0) {
	box(FL_NO_BOX);
	load_data();
	// Get record
	records_ = records;
	num_records_ = num_records;
	card_view_ = nullptr;
	// Draw the widgets
	create_form(X, Y);
	//
}

// Destructor
qsl_form::~qsl_form() {
	delete card_view_;
}

// Load the QSL design data from the settings
void qsl_form::load_data() {
	// Get width and height of label
	string callsign = qso_manager_->get_default(qso_manager::CALLSIGN);
	Fl_Preferences qsl_settings(settings_, "QSL Design");
	char* temp;
	Fl_Preferences call_settings(qsl_settings, callsign.c_str());
	call_settings.get("Width", width_, 0);
	call_settings.get("Height", height_, 0);
	call_settings.get("Unit", (int&)unit_, (int)MILLIMETER);
	call_settings.get("Filename", temp, "");
	filename_ = temp;
	free(temp);
	if (width_ == 0 || height_ == 0 || filename_.length() == 0) {
		// We have either width or height not defined - so load the edfault data
		char msg[256];
		snprintf(msg, sizeof(msg), "QSL: Incorrect info W=%f, H=%f, File=%s", width_, height_, temp);
		status_->misc_status(ST_ERROR, msg);
	}
}

// Create the card design
void qsl_form::create_form(int X, int Y) {
	card_view_ = new qsl_html_view(X, Y, to_points(width_), to_points(height_));
	char message[256];
	snprintf(message, 256, "QSL: Starting reading QSL template %s", filename_.c_str());
	status_->misc_status(ST_NOTE, message);
	ifstream ifile;
	ifile.open(filename_.c_str());
	if (!ifile.good()) {
		snprintf(message, 256, "QSL: Failed to open %s", filename_.c_str());
		status_->misc_status(ST_ERROR, message);
		return;
	}

	// read the entire file into a buffer
	ifile.seekg(0, ifile.end);
	int length = (int)ifile.tellg();
	ifile.seekg(0, ifile.beg);
	char* buffer = new char[length];
	memset(buffer, 0, length);
	ifile.read(buffer, length);
	if (!ifile.eof() && !ifile.good()) {
		snprintf(message, 256, "QSL: Failed to read %s", filename_.c_str());
		status_->misc_status(ST_ERROR, message);
		delete buffer;
		ifile.close();
		return;
	}

	ifile.close();
	card_view_->value(buffer, records_, num_records_);
	delete buffer;

	resizable(nullptr);
	size(card_view_->w(), card_view_->h());
	show();
	end();
}

// Return unit
qsl_form::dim_unit qsl_form::unit() {
	return unit_;
}

// Set unit
void qsl_form::unit(dim_unit unit) {
	unit_ = unit;
}

// Convert value from specified unit to points
int qsl_form::to_points(float value) {
	switch (unit_) {
	case INCH:
		// 72 points per inch
		return (int)(value * IN_TO_POINT);
	case MILLIMETER:
		// 72/25.4 points per mm
		return (int)(value * MM_TO_POINT);
	case POINT:
		return (int)value;
	default:
		return 0;
	}
}

// Return width
float qsl_form::width() {
	return width_;
}

// Set width
void qsl_form::width(float width) {
	width_ = width;
}

// Return height
float qsl_form::height() {
	return height_;
}

// Set height
void qsl_form::height(float height) {
	height_ = height;
}

// Size error
bool qsl_form::size_error() {
	return size_error_;
}

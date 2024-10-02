#include "png_writer.h"
#include "book.h"
#include "record.h"
#include "qsl_image.h"
#include "qsl_data.h"
#include "status.h"

#include "png.h"
#include <FL/Fl_Preferences.H>


extern Fl_Preferences* settings_;
extern status* status_;

png_writer::png_writer() {
	// TODO: Any initialisation of the libpng
}

// Destructor
png_writer::~png_writer() {
	// TODO: Any tidy up for libpng
}

// Generate PNG filename
string png_writer::png_filename(record* qso) {
	string my_call = qso->item("STATION_CALLSIGN");
	string call = qso->item("CALL");
	string date = qso->item("QSO_DATE");

	Fl_Preferences dp_settings(settings_, "Datapath");
	char* temp;
	dp_settings.get("QSLs", temp, "");
	string dir = temp;
	free(temp);

	char result[256];
	snprintf(result, sizeof(result), "%s/%s/png/%s_%s.png",
		dir.c_str(),
		my_call.c_str(),
		call.c_str(),
		date.c_str());

	return string(result);
}

// Create the file
bool png_writer::write_image(Fl_RGB_Image* image, string filename) {
	// TODO: impelment from libpng example
	status_->misc_status(ST_WARNING, "PNG: write_image() not yet implemented");
	return false;
}

// Craete files for all th erecords in the book
bool png_writer::write_book(book* qsos) {
	bool ok = true;
	for(auto it = qsos->begin(); it != qsos->end() && ok; it++) {
		Fl_RGB_Image* image = qsl_image::image(*it, qsl_data::FILE);
		string filename = png_filename(*it);
		ok = write_image(image, filename);
	}
	return ok;
}

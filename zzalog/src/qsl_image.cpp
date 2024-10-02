#include "qsl_image.h"
#include "record.h"
#include "qsl_display.h"
#include "qsl_data.h"
#include "qsl_dataset.h"

#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Image_Surface.H>

extern qsl_dataset* qsl_dataset_;

Fl_RGB_Image* qsl_image::image(record* qso, qsl_data::qsl_type type) {

	// Get the QSL card design data
	qsl_display* qsl = new qsl_display(0, 0);
	qsl_data* data = qsl_dataset_->get_card(qso->item("STATION_CALLSIGN"), type, false);
	qsl->set_card(data);
	qsl->set_qsos(&qso, 1);

	int w, h;
	qsl->get_size(w, h);

	// Create the drawing surface to the size of the card
	Fl_Image_Surface* surface = new Fl_Image_Surface(w, h);
	// direct all further graphics requests to the image
	Fl_Surface_Device::push_current(surface);

	// Draw the design
	qsl->draw();

	Fl_RGB_Image* image = surface->image();

	// Restore previous drawing surface
	Fl_Surface_Device::pop_current();

	delete surface;
	delete qsl;

	return image;
}
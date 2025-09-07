#pragma once

#include "qsl_data.h"

class Fl_RGB_Image;
class record;

//! This class is a container for the static method image.
class qsl_image
{
public:
	//! Create the image from the QSO record \p qso for the specified QSL \p type.
	
	//! It draws the image on a temporary surface then returns the captured image.
	static Fl_RGB_Image* image(record* qso, qsl_data::qsl_type type);

};


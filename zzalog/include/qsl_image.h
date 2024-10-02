#pragma once

#include "qsl_data.h"

class Fl_RGB_Image;
class record;

class qsl_image
{
public:
	// Create the image from the QSO record for the QL type
	static Fl_RGB_Image* image(record* qso, qsl_data::qsl_type type);

};


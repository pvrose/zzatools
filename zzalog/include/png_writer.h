#pragma once

#include <string>
#include <cstdio>

#include "png.h"

class Fl_RGB_Image;
class book;
class record;

using namespace std;

class png_writer
{
public:

	png_writer();
	~png_writer();

	// Get the PNG filename for the QSO
	static string png_filename(record* qso);

	bool write_book(book* qsos);

	// Write the RGB image out to a .png file
	bool write_image(Fl_RGB_Image* image, string filename);

protected:

	// Initialise the PNG instance
	bool initialise_png();
	// Write one line of data
	bool write_row(const char* data);
	// Finalise and close the interface
	bool finalise_png();
	// Clearup
	void tidy_png();

	static void error_handler(png_structp png, png_const_charp msg);

	FILE* out_file_;

	Fl_RGB_Image* image_;

	// Local copy
	char* buffer_;

	// libpng items
	jmp_buf jump_buf_;
	png_structp png_;
	png_infop info_;

};


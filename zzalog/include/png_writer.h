#pragma once

#include <string>
#include <cstdio>

#include "png.h"

class Fl_RGB_Image;
class book;
class record;



//! Class that outputs the image for an e-mail QSL card.

//! It uses the library png supplied as part of the FLTK installation.
class png_writer
{
public:
	//! Constructor.
	png_writer();
	//! Destructor.
	~png_writer();

	//! Get the PNG filename for the \p qso.
	static std::string png_filename(record* qso);
	//! Generate files for the \p qsos specified.
	bool write_book(book* qsos);

	//! Write the RGB \p image out to a .png file \p filename.
	bool write_image(Fl_RGB_Image* image, std::string filename);

protected:

	//! Initialise the PNG instance
	bool initialise_png();
	//! Write one line of \p data
	bool write_row(const char* data);
	//! Finalise and close the interface
	bool finalise_png();
	//! Clearup
	void tidy_png();

	//! Callback required by libpng library to handle error.
	static void error_handler(png_structp png, png_const_charp msg);

	//! Output file handle.
	FILE* out_file_;

	//! The image to be written out.
	Fl_RGB_Image* image_;

	//! Local copy of data.
	char* buffer_;

	// libpng items
	jmp_buf jump_buf_;     //!< Used to skip data after error.
	png_structp png_;      //!< PNG data structure.
	png_infop info_;       //!< PNG information structure

};


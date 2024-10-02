#pragma once

#include <string>

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

protected:

	bool write_image(Fl_RGB_Image* image, string filename);

};


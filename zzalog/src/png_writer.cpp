#include "png_writer.h"
#include "book.h"
#include "record.h"
#include "qsl_image.h"
#include "qsl_data.h"
#include "status.h"

#include <ctime>

#include "png.h"
#include "zlib.h"
#include <FL/Fl_Preferences.H>
#include <FL/fl_utf8.h>


extern Fl_Preferences* settings_;
extern status* status_;

png_writer::png_writer() {
	// TODO: Any initialisation of the libpng
	image_ = nullptr;
	info_ = nullptr;
	out_file_ = 0;
	png_ = nullptr;
	buffer_ = nullptr;
	memset(&jump_buf_, 0, sizeof(jump_buf_));

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
	image_ = image;
	fl_make_path_for_file(filename.c_str());
	out_file_ = fopen(filename.c_str(), "wb");
	char msg[256];
	if (!out_file_) {
		snprintf(msg, sizeof(msg), "PNG: Failed to open file %s for writing", filename.c_str());
		status_->misc_status(ST_ERROR, msg);
		return true;
	}
	else {
		snprintf(msg, sizeof(msg), "PNG: Starting to write to %s", filename.c_str());
		status_->misc_status(ST_NOTE, msg);
	}
	// Line length
	int llen = image_->ld() ? image_->ld() : image_->data_w() * image_->d();
	int len = image_->data_h() * llen;
	bool bad = false;
	bad = initialise_png();
	buffer_ = new char[len];
	for (int ix = 0; ix < len; ix += llen) {
		bad = write_row(*(image_->data()) + ix);
	}
	bad |= finalise_png();
	tidy_png();
	fclose(out_file_);
	return bad;
}

// Craete files for all th erecords in the book
bool png_writer::write_book(book* qsos) {
	bool bad = false;
	for(auto it = qsos->begin(); it != qsos->end() && !bad; it++) {
		Fl_RGB_Image* image = qsl_image::image(*it, qsl_data::FILE);
		string filename = png_filename(*it);
		bad = write_image(image, filename);
		if (bad) {
			char msg[128];
			snprintf(msg, sizeof(msg), "PNG: Failed to write %s", filename.c_str());
			status_->misc_status(ST_ERROR, msg);
		}
	}
	return !bad;
}

// Initialise the PNG instance
bool png_writer::initialise_png() {
	// Initialsie PNG control structure
	png_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, this, &error_handler, nullptr);
	if (png_ == nullptr) {
		status_->misc_status(ST_ERROR, "PNG: failed to create control structure!");
		return true;
	}
	// Initialise information structure
	info_ = png_create_info_struct(png_);
	if (info_ == nullptr) {
		status_->misc_status(ST_ERROR, "PNG: failed to create information structure");
		png_destroy_write_struct(&png_, nullptr);
		return true;
	}
	// Check for any problem
	if (setjmp(jump_buf_)) {
		status_->misc_status(ST_ERROR, "PNG: PNG Error");
		png_destroy_write_struct(&png_, &info_);
		return true;
	}
	// Initislaise the file stream
	png_init_io(png_, out_file_);
	// Set the compression level - go nuclear
	png_set_compression_level(png_, Z_BEST_COMPRESSION);
	// Now set image attributes
	png_set_IHDR(png_, info_,
		image_->data_w(), image_->data_h(), 8, // width, height and bit-depth
		PNG_COLOR_TYPE_RGB,                 // RGB
		PNG_INTERLACE_NONE,    	            // Non-interlaced
		PNG_COMPRESSION_TYPE_DEFAULT,       // Compression
		PNG_FILTER_TYPE_DEFAULT);

	// Add timestamp
	time_t now = time(nullptr);
	png_time ts;
	png_convert_from_time_t(&ts, now);
	png_set_tIME(png_, info_, &ts);
	// Add metadate
	// TODO:

	// Write all data so far
	png_write_info(png_, info_);

	// Add u
	return false;
}

// Write one line of data
bool png_writer::write_row(const char* data) {
	// Check for erros
	if (setjmp(jump_buf_)) {
		status_->misc_status(ST_ERROR, "PNG: PNG Write error");
		png_destroy_info_struct(png_, &info_);
		png_ = nullptr;
		info_ = nullptr;
		return true;
	}

	png_write_row(png_, (const uchar*)data);

	return false;
}

// Close down
bool png_writer::finalise_png() {
	// Check for erros
	if (setjmp(jump_buf_)) {
		status_->misc_status(ST_ERROR, "PNG: PNG Write error");
		png_destroy_info_struct(png_, &info_);
		png_ = nullptr;
		info_ = nullptr;
		return true;
	}
	// Close
	status_->misc_status(ST_NOTE, "PNG: Finalising write");
	png_write_end(png_, info_);
	return false;
}

// Tidy up
void png_writer::tidy_png() {
	if (png_ && info_) {
		png_destroy_write_struct(&png_, &info_);
	}
}

void png_writer::error_handler(png_structp png, png_const_charp msg) {
	char temp[128];
	snprintf(temp, sizeof(temp), "PNG: PNG Error %s", msg);
	status_->misc_status(ST_ERROR, temp);

	png_writer* that = (png_writer*)png_get_error_ptr(png);
	if (!that) {
		snprintf(temp, sizeof(temp), "PNG: Unrecoverable error");
		status_->misc_status(ST_SEVERE, temp);
	}
	else {
		longjmp(that->jump_buf_, 1);
	}
}



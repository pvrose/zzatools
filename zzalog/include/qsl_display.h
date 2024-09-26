#pragma once

#include "drawing.h"
#include "qsl_data.h"

#include <vector>
#include <string>
#include <cmath>


using namespace std;

class record;
class Fl_Image;
class Fl_RGB_Image;

// This class provides the drawing of a QSL label. It is used as part of the
// editor, the QSL viewer in QSO dashboard and tne printer.

// unit conversions: 1" = 72pt = 25.4 mm
const float MM_TO_POINT = 72.0f / 25.4f;
const float IN_TO_POINT = 72.0f;

class qsl_display
{

public:

    enum qsl_surface : uchar {
        DEFAULT,         // draw to the current surface
        IMAGE,           // Draw into Fl_Image_Surface
        PDF,             // Draw into Fl_PDF_File_Surface (TODO:)
    };


    // Constructor to create a resizeable image
    qsl_display(qsl_surface surface);
    // Constructor to draw directly on the current drawing surface
    qsl_display(int X, int Y, qsl_surface surface = DEFAULT);
    ~qsl_display();

    // Overide the Fl_Group::draw() to impelment the drawing.
    void draw();

    // Pointer to the data for editor to use
    void set_card(qsl_data* card);
    // Set the QSOS to draw
    void set_qsos(record** qsos, int num_qsos);

    // Get the image at that filename
    Fl_Image* get_image(string filename);

    // Return the drawn image
    Fl_RGB_Image* image();

protected:

    void draw_surface();

    // Draw an individual field item
    void draw_field(qsl_data::field_def& field);
    // Draw an individual text item
    void draw_text(qsl_data::text_def& text);
    // Draw an individual image iotem
    void draw_image(qsl_data::image_def& image);

    // Convert the ADIF format date to the required format
    string convert_date(string text);
    // Convert the aDIF format yime to the required format
    string convert_time(string text);
 
    // Convert to points
    int to_points(float value);
    
    // Drawing data for the current instance
    qsl_data* data_;
    // The array of QSOs to be displayed on the card
    record** qsos_;
    // The size of the above array
    int num_records_;
    // Positions to use if dx or dy are -1
    int next_x_;
    int next_y_;

     // The generated image
    Fl_RGB_Image* image_;

    // Draw directly
    qsl_surface draw_surface_;
    // Origin
    int x_;
    int y_;
    // Width and height in points
    int w_;
    int h_;
};
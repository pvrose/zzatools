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

     // Constructor to draw directly on the current drawing surface
    // If W or H are zero, then the image will be drawn without scaling
    // If both W and H are non-zero then the image will be scaled appropriately
    qsl_display(int X, int Y, int W = 0, int H = 0);
    ~qsl_display();

    // Draw the design with specified width and height
    void resize(int w, int h);
    // Draw the design with existing width and height
    void draw();

    // Pointer to the data for editor to use
    void set_card(qsl_data* card);
    // Set the QSOS to draw
    void set_qsos(record** qsos, int num_qsos);
    // Set a fixed image to display
    void set_image(Fl_Image* image);
    // Set a fixed text to display
    void set_text(const char* text, Fl_Color colour);
    // Get the required size for the unscaled design
    void get_size(int& w, int& h);

protected:

    // Draw an individual field item
    void draw_field(qsl_data::field_def& field);
    // Draw an individual text item
    void draw_text(qsl_data::text_def& text);
    // Draw an individual image iotem
    void draw_image(qsl_data::image_def& image);
    // Draw the alternate image
    void draw_image(int x, int y, Fl_Image* image);
    // Draw alternate text
    void draw_text(const char* text, Fl_Color colour);

    // Convert the ADIF format date to the required format
    string convert_date(string text);
    // Convert the aDIF format yime to the required format
    string convert_time(string text);
 
    // Convert to points
    int to_points(float value);

    // Get the image at that filename
    Fl_Image* get_image(string filename);

    // Scale value 
    int scale(int value);
    // Calculate scale values
    void calculate_scale(int tgt_w, int tgt_h);

    // Drawing data for the current instance
    qsl_data* data_;
    // The array of QSOs to be displayed on the card
    record** qsos_;
    // The size of the above array
    int num_records_;
    // Positions to use if dx or dy are -1
    int next_x_;
    int next_y_;

     // The alternate image
    Fl_Image* alt_image_;
    // The alternate text
    char* alt_text_;
    // Alternate txt colour
    Fl_Color alt_colour_;

    // Origin
    int x_;
    int y_;
    // Width and height in points
    int w_;
    int h_;
    // Scaling factor
    float scaling_;
    // Use scaling factor
    bool do_scale_;
    // Scaled origin
    int draw_x_;
    int draw_y_;
    // Sacled width and height
    int draw_w_;
    int draw_h_;
};
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

//! Number of points in 1 millimetre.
const float MM_TO_POINT = 72.0f / 25.4f;
//! Number of points in 1 inch.
const float IN_TO_POINT = 72.0f;

//! This class provides the drawing of a QSL label. 

//! It is used as part of the
//! editor, the QSL viewer in QSO dashboard and tne printer.
class qsl_display
{

public:

    //! Constructor to draw directly on the current drawing surface.
    
    //! If \p W or \p H are zero, then the image will be drawn without scaling.
    //! If both \p W and \p H are non-zero then the image will be scaled appropriately to fit
    //! the specified \p W x \p H rectangle.
    qsl_display(int X, int Y, int W = 0, int H = 0);
    //! Constructor.
    ~qsl_display();

    //! Draw the design with specified width \p w and height \p h.
    void resize(int w, int h);
    //! Draw the design with existing width and height
    void draw();

    //! Pointer to the \p card data for editor to use.
    void set_card(qsl_data* card);
    //! Set the QSOS (\p num_qsos at \p qsos) to draw.
    void set_qsos(record** qsos, int num_qsos);
    //! Set a fixed \p image to display
    void set_image(Fl_Image* image);
    //! Set a fixed \p text to display in \p colour.
    void set_text(const char* text, Fl_Color colour);
    //! Receives the required size for the unscaled design in \p w and \p h.
    void get_size(int& w, int& h);

protected:

    //! Draw an individual \p field item.
    void draw_field(qsl_data::field_def& field);
    //! Draw an individual \p text item.
    void draw_text(qsl_data::text_def& text);
    //! Draw an individual \p image item.
    void draw_image(qsl_data::image_def& image);
    //! Draw the \p image at position (\p x, \p y), scaling if necessary.
    void draw_image(int x, int y, Fl_Image* image);
    //! Draw \p text in \p colour in place of a generated image.
    void draw_text(const char* text, Fl_Color colour);

    //! Returns the ADIF format date in the remembered format.
    string convert_date(string text);
    //! Returns the ADIF format time in the remembered format.
    string convert_time(string text);
 
    //! Returns the specified \p value as points.
    int to_points(float value);

    //! Returns the image at that filename.
    Fl_Image* get_image(string filename);

    //! Returns the input \p value scaled by the necessary amount. 
    int scale(int value);
    //! Calculate scale values.
    void calculate_scale(int tgt_w, int tgt_h);

    //! Convert file name to absolute
    bool absolute_filename(string& filename);

    //! Drawing data for the current instance.
    qsl_data* data_;
    //! The array of QSOs to be displayed on the card
    record** qsos_;
    //! The number of QSOs in array qsos_.
    int num_records_;
    //! X position to use if dx is -1.
    int next_x_;
    //! Y position to  use if dy is -1.
    int next_y_;

    //! The alternate image
    Fl_Image* alt_image_;
    //! The alternate text
    char* alt_text_;
    //! Alternate txt colour
    Fl_Color alt_colour_;

    // Origin
    int x_;              //!< X coordinate of origin.
    int y_;              //!< y coordinate of origin.
    // Width and height in points
    int w_;              //!< Width in pixels.
    int h_;              //!< Height in pixels.
    // Scaling factor
    float scaling_;      //!< Scaling factor to draw image in widget.
    //! Use scaling factor
    bool do_scale_;      
    // Scaled origin
    int draw_x_;         //!< X coordinate of origin once scaled.
    int draw_y_;         //!< Y coordinate of origin once scaled.
    // Sacled width and height
    int draw_w_;         //!< Scaled width. 
    int draw_h_;         //!< Scaled height.
};
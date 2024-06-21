#pragma once

#include "record.h"
#include "drawing.h"

#include <vector>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>

using namespace std;

// This class provides the drawing of a QSL label. It is used as part of the
// editor, the QSL viewer in QSO dashboard and tne printer.

// unit conversions: 1" = 72pt = 25.4 mm
const float MM_TO_POINT = 72.0f / 25.4f;
const float IN_TO_POINT = 72.0f;

class qsl_display : public Fl_Widget 
{

    public:

    // Specifies the drawing element; 
    // FIELD: A field taken from the QSO record
    // TEXT: Justa  textual comment on the QSL card
    // IMAGE: An image to be printed on the card
    enum item_type {
        NONE,   // Used to mark an item for removal
        FIELD,  // Draw the field text and its label
        TEXT,   // Draw the quoted 
        IMAGE,  // An Fl_Box displaying an image 
    };

    // Specifies the font, size and colour to be used for
    // field values, field labels and text values
    struct style_def {
        Fl_Font font { FL_HELVETICA };       // Font
        Fl_Fontsize size { FL_NORMAL_SIZE };   // Size
        Fl_Color colour { FL_BLACK };    // Colour  
    };

    // Structure describing the parameters of a field and its label
    struct field_def {
        string field { "" };        // Field name
        string label { "" };        // The label on the QSL (e.g. "To:" for CALL)
        style_def l_style;   // Style of the label
        style_def t_style;   // Style of the text
        int dx { 0 };              // Position of the text within the QSL card (-1 = juxtaposed the previous)
        int dy { 0 };              // ditto
        bool vertical { false };       // If true: label and text are stacked one above the other
        bool box { true };            // Enclose the display with a box
        bool multi_qso { false };      // The item is repeated per QSO
        bool display_empty { true };  // Display the field if it is empty

    };

    // Structure describing the paarmeters of a text item
    struct text_def {
        string text { "Here be dragons!"};
        style_def t_style;
        int dx { 0 };
        int dy { 0 };

    };

    // Structure describing the paarmeters of an image item
    struct image_def {
        string filename { "" };
        Fl_Image* image { nullptr };
        int dx { 0 };
        int dy { 0 };
    };

    // Structure describing the paarmeters of an item
    struct item_def {
        item_type type;
        field_def field;
        text_def text;
        image_def image;

        item_def() {}
    };

    // Units of measurement
    enum dim_unit {
        INCH,       // = 25.4 mm
        MILLIMETER,
        POINT       // = 1/72 in.
    };

    // Date format
    enum date_format {
        FMT_Y4MD_ADIF,     // 20240618
        FMT_Y4MD,          // 2024/06/18
        FMT_Y2MD,          // 24/06/18
        FMT_DMY2,          // 18/06/24
        FMT_MDY2           // 06/18/24
    };

    // Time format
    enum time_format {
        FMT_HMS_ADIF,      // 171033
        FMT_HMS,           // 17:10:33
        FMT_HM_ADIF,       // 1710
        FMT_HM             // 17:10
    };

    // Card definition data
    struct card_data {
        dim_unit unit;      // Dimensions i=unit
        double width;       // Individual label width
        double height;      // Individial label height
        int rows;           // Number of rows per sheet
        int columns;        // Number of columns per sheet
        double col_width;   // Column separation
        double row_height;  // Row separation
        double row_top;     // Top margin
        double col_left;    // Left margin
        int max_qsos;       // maximum number of QSOs per label
        date_format f_date; // Format for date to be printed
        time_format f_time; // Format for time to be printed
        string filename;    // Filename of layout data file
        vector<item_def*> items; // layout items read in from file 
    };

    qsl_display(int X, int Y, int W, int H, const char* L = nullptr);
    ~qsl_display();

    // Load data
    static void load_data(string callsign);
    // Save data
    void save_data();
    // Load items
    void load_items();

    // Resize the drawing according to the card label width and height parameters
    void resize();
    // Set the callsign to be used and the records to include in the label
    void value(string callsign, record** qsos = nullptr, int num_records = 0);
    // Short-cut version of above with one QSO and callsign to be taken from it
    void example_qso(record* qso);
    // Overide the Fl_Group::draw() to impelment the drawing.
    virtual void draw();

    // Editable flag to allow widgets to be edited in situ
    void editable(bool enable); 
    bool editable();

    // Pointer to the data for editor to use
    static card_data* data(string callsign);

    // Get the image at that filename
    static Fl_Image* get_image(string filename);

    protected:

    // Draw an individual field item
    void draw_field(field_def& field);
    // Draw an individual text item
    void draw_text(text_def& text);
    // Draw an individual image iotem
    void draw_image(image_def& image);

    // Convert the ADIF format date to the required format
    string convert_date(string text);
    // Convert the aDIF format yime to the required format
    string convert_time(string text);

    // Drawing data for all callsigns - needs to be static to access the 
    // label size data outwith a specific instance of the drawing 
    static map<string, card_data> all_data_;
    // Drawing data for the current instance
    card_data* data_;
    // The array of QSOs to be displayed on the card
    record** qsos_;
    // The size of the above array
    int num_records_;
    // Set callsign
    bool editable_;
    // Callsign
    string callsign_;
    // Positions to use if dx or dy are -1
    int next_x_;
    int next_y_;
 
    // Convert to points
    int to_points(float value);

};
#pragma once

#include <string>
#include <vector>

#include <FL/Fl.H>

using namespace std;

// Card definition data
struct qsl_data {

    // Specifies the drawing element; 
    // FIELD: A field taken from the QSO record
    // TEXT: Justa  textual comment on the QSL card
    // IMAGE: An image to be printed on the card
    enum item_type : uchar {
        NONE,   // Used to mark an item for removal
        FIELD,  // Draw the field text and its label
        TEXT,   // Draw the quoted 
        IMAGE,  // An Fl_Box displaying an image 
    };

    // Specifies the font, size and colour to be used for
    // field values, field labels and text values
    struct style_def {
        Fl_Font font{ FL_HELVETICA };       // Font
        Fl_Fontsize size{ FL_NORMAL_SIZE };   // Size
        Fl_Color colour{ FL_BLACK };    // Colour  
    };

    // Structure describing the parameters of a field and its label
    struct field_def {
        string field{ "" };        // Field name
        string label{ "" };        // The label on the QSL (e.g. "To:" for CALL)
        style_def l_style;   // Style of the label
        style_def t_style;   // Style of the text
        int dx{ 0 };              // Position of the text within the QSL card (-1 = juxtaposed the previous)
        int dy{ 0 };              // ditto
        bool vertical{ false };       // If true: label and text are stacked one above the other
        bool box{ true };            // Enclose the display with a box
        bool multi_qso{ false };      // The item is repeated per QSO
        bool display_empty{ true };  // Display the field if it is empty

    };

    // Structure describing the paarmeters of a text item
    struct text_def {
        string text{ "Text" };
        style_def t_style;
        int dx{ 0 };
        int dy{ 0 };
        bool vertical{ false };      // If true the next item is alongside rather than below

    };

    // Structure describing the paarmeters of an image item
    struct image_def {
        string filename{ "" };
        int dx{ 0 };
        int dy{ 0 };
    };

    // Structure describing the paarmeters of an item
    struct item_def {
        item_type type;
        field_def field;
        text_def text;
        image_def image;
    };

    // Units of measurement
    enum dim_unit : uchar {
        INCH,       // = 25.4 mm
        MILLIMETER,
        POINT       // = 1/72 in.
    };

    // Type of display
    enum qsl_type : uchar {
        LABEL,              // Display/print on a label
        FILE,               // Display/print to PDF
        MAX_TYPE
    };

    // Date format
    enum date_format : uchar {
        FMT_Y4MD_ADIF,     // 20240618
        FMT_Y4MD,          // 2024/06/18
        FMT_Y2MD,          // 24/06/18
        FMT_DMY2,          // 18/06/24
        FMT_MDY2           // 06/18/24
    };

    // Time format
    enum time_format : uchar {
        FMT_HMS_ADIF,      // 171033
        FMT_HMS,           // 17:10:33
        FMT_HM_ADIF,       // 1710
        FMT_HM             // 17:10
    };

    // Defaults are for an 8-label sheet
    dim_unit unit{ MILLIMETER };      // Dimensions i=unit
    double width{ 101.6 };            // Individual label width
    double height{ 67.7 };            // Individial label height
    int rows{ 4 };                    // Number of rows per sheet
    int columns{ 2 };                 // Number of columns per sheet
    double col_width{ 101.6 };        // Column separation
    double row_height{ 67.7 };        // Row separation
    double row_top{ 12.9 };           // Top margin
    double col_left{ 4.6 };           // Left margin
    int max_qsos{ 1 };                // maximum number of QSOs per label
    date_format f_date{ FMT_Y4MD_ADIF };   // Format for date to be printed
    time_format f_time{ FMT_HMS_ADIF };    // Format for time to be printed
    string filename{ "" };            // Filename of layout data file
    vector<item_def*> items;          // layout items read in from file 

    qsl_data() {
        items.clear();
    }
};

// Initialise the 
const string QSL_TYPES[qsl_data::MAX_TYPE] = {"Label", "PDF"};


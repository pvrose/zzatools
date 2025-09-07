#pragma once

#include <string>
#include <vector>

#include <FL/Fl.H>

using namespace std;

//! QSL Card definition data
struct qsl_data {

    //! Specifies the drawing element. 
    enum item_type : uchar {
        NONE,   //!< Used to mark element for deletion
        FIELD,  //!< A field from the QSO record: name and value get printed.
        TEXT,   //!< A text comment.
        IMAGE,  //!< An Fl_Box displaying an image. 
    };

    //! Specifies the font, size and colour to be used for FIELD and TEXT items
    // field values, field labels and text values
    struct style_def {
        Fl_Font font{ FL_HELVETICA };       //!< Font
        Fl_Fontsize size{ FL_NORMAL_SIZE }; //!< Size
        Fl_Color colour{ FL_BLACK };        //!< Colour  
    };

    //! Structure describing the parameters of a field and its label.
    struct field_def {
        string field{ "" };        //!< Field name.
        string label{ "" };        //!< The label on the QSL (e.g. "To:" for CALL).
        style_def l_style;         //!< Style of the label.
        style_def t_style;         //!< Style of the field value.
        int dx{ 0 };               //!< X Position of the field value within the QSL card (-1 = juxtaposed the previous)
        int dy{ 0 };               //!< Y position of the field value within the QSL card.
        bool vertical{ false };    //!< If true: label and text are stacked one above the other.
        bool box{ true };          //!< Enclose the display with a box.
        bool multi_qso{ false };   //!< The item is repeated per QSO.
        bool display_empty{ true };//!< Display the field if it is empty.

    };

    //! Structure describing the paarmeters of a text item.
    struct text_def {
        string text{ "Text" };     //!< Text value.
        style_def t_style;         //!< Style of the text.
        int dx{ 0 };               //!< X position within QSL card image.
        int dy{ 0 };               //!< Y position within QSL card image.
        bool vertical{ false };    //!< If true the next item is alongside rather than below

    };

    //! Structure describing the paarmeters of an image item
    struct image_def {
        string filename{ "" };     //!< Filename of the image to be displayed.
        int dx{ 0 };               //!< X position within the QSL card image.
        int dy{ 0 };               //!< Y position within the QSL card image.
    };

    //! Structure describing the paarmeters of an item
    struct item_def {
        item_type type{ NONE };    //!< Item type - one of the other members is used.
        field_def field;           //!< Definition if the item is of type FIELD.
        text_def text;             //!< Definition if the item is of type TEXT.
        image_def image;           //!< Definition if the item is of type IMAGE.
    };

    //! Units of measurement
    enum dim_unit : uchar {
        INCH,          //!< inch: 1 inch = 25.4 mm
        MILLIMETER,    //!< millimetre.
        POINT          //!< point: 1 point = 1/72 in. 
    };

    //! Type of display
    enum qsl_type : uchar {
        LABEL,              //!< Display/print on a label
        FILE,               //!< Display/print to PDF
        MAX_TYPE
    };

    //! Date format used in QSL card.
    enum date_format : uchar {
        FMT_Y4MD_ADIF,     //!< 20240618
        FMT_Y4MD,          //!< 2024/06/18
        FMT_Y2MD,          //!< 24/06/18
        FMT_DMY2,          //!< 18/06/24
        FMT_MDY2,          //!< 06/18/24
        FMT_INVALID_DATA,
    };

    //! Time format used in QSL card.
    enum time_format : uchar {
        FMT_HMS_ADIF,      //!< 171033
        FMT_HMS,           //!< 17:10:33
        FMT_HM_ADIF,       //!< 1710
        FMT_HM,            //!< 17:10
        FMT_INVALID_TIME,
    };

    // Defaults are for an 8-label sheet
    dim_unit unit{ MILLIMETER };      //!< Dimensions in  specified unit
    double width{ 101.6 };            //!< Individual image width
    double height{ 67.7 };            //!< Individial image eight
    int rows{ 4 };                    //!< Number of rows per sheet
    int columns{ 2 };                 //!< Number of columns per sheet
    double col_width{ 101.6 };        //!< Column separation
    double row_height{ 67.7 };        //!< Row separation
    double row_top{ 12.9 };           //!< Top margin
    double col_left{ 4.6 };           //!< Left margin
    int max_qsos{ 1 };                //!< maximum number of QSOs per label
    date_format f_date{ FMT_Y4MD_ADIF };   //!< Format for date to be printed
    time_format f_time{ FMT_HMS_ADIF };    //!< Format for time to be printed
    bool filename_valid{ false };     //!< Layout data file used (else from XML)  
    string filename{ "" };            //!< Filename of layout data file - relative to  QSL datapath
    vector<item_def*> items;          //!< layout items read in from file 

    //qsl_data() {
    //    items.clear();
    //}
};

//! Initialise the text used to describe the card types. 
const string QSL_TYPES[qsl_data::MAX_TYPE] = {"Label", "File"};




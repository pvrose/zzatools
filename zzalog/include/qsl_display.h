#pragma once

#include "record.h"
#include "drawing.h"

#include <vector>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

using namespace std;

// unit conversions: 1" = 72pt = 25.4 mm
const float MM_TO_POINT = 72.0f / 25.4f;
const float IN_TO_POINT = 72.0f;

class qsl_display : public Fl_Group 
{

    public:

    enum item_type {
        NONE,   // Used to mark an item for removal
        FIELD,  // Draw the field text and its label
        TEXT,   // Draw the quoted 
        IMAGE,  // An Fl_Box displaying an image 
    };

    struct style_def {
        Fl_Font font { FL_HELVETICA };       // Font
        Fl_Fontsize size { FL_NORMAL_SIZE };   // Size
        Fl_Color colour { FL_BLACK };    // Colour  
    };

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

    struct text_def {
        string text { "Here be dragons!"};
        style_def t_style;
        int dx { 0 };
        int dy { 0 };

    };

    struct image_def {
        string filename { "" };
        Fl_Image* image { nullptr };
        int dx { 0 };
        int dy { 0 };
    };

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

    qsl_display(int X, int Y, int W, int H, const char* L = nullptr);
    ~qsl_display();

    // Load data
    void load_data();
    // Save data
    void save_data();

    // Set the widget parameters - passed by reference to enable editing
    void value(string callsign, record** qsos = nullptr, int num_records = 0);
    // Set the size
    void size(float w, float h, dim_unit unit, int max_records);

    // Handle widget movement and sizing actions
    virtual int handle(int event);
    // Draw the objects explicitly
    virtual void draw();

    // Editable flag to allow widgets to be edited in situ
    void editable(bool enable); 
    bool editable();

    // Pointer to the data for editor to use
    vector<item_def*>* data();

    // Get the image at that filename
    Fl_Image* get_image(string filename);

    protected:

    void draw_field(field_def& field);
    void draw_text(text_def& text);
    void draw_image(image_def& image);

    // The widget parameters
    vector<item_def*> data_;
    // The array of QSOs to be displayed on the card
    record** qsos_;
    // The size of the above array
    int num_records_;
    // Set callsign
    bool editable_;
    // Width of each instance of qsl_card (in units)
    float width_;
    // Height of each instance of qsl_card (in units)
    float height_;
    // Unit of width and height
    dim_unit unit_;
    // Filename
    string filename_;
    // Callsign
    string callsign_;
    // Positions to use if dx or dy are -1
    int next_x_;
    int next_y_;
    // Maximum number of records
    int max_records_;


    // Convert to points
    int to_points(float value);

};
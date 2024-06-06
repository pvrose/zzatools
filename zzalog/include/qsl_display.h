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

    struct style_def {
        Fl_Font font;       // Font
        Fl_Fontsize size;   // Size
        Fl_Color colour;    // Colour

        style_def() {
            font = FL_HELVETICA;
            size = FL_NORMAL_SIZE;
            colour = FL_BLACK;
        }
    };

    struct item_def {
        string field;        // Field name
        string label;        // The label on the QSL (e.g. "To:" for CALL)
        style_def l_style;   // Style of the label
        style_def t_style;   // Style of the text
        int dx;              // Position within the QSL card
        int dy;
        int dw;
        int dh;
        Fl_Align align;      // The alignment
        Fl_Boxtype box;      // Box type
        bool multi_qso;      // The item is repeated per QSO
        bool display_empty;  // Display the field if it is empty

        item_def() {
            field = "";
            label = "Label";
            dx = WLABEL;
            dy = 0;
            dw = WBUTTON;
            dh = HBUTTON;
            align = FL_ALIGN_LEFT;
            box = FL_BORDER_BOX;
            field = "";
            multi_qso = false;
            display_empty = false;
        }
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
    void size(float w, float h, dim_unit unit);

    // Handle widget movement and sizing actions
    virtual int handle(int event);

    // Editable flag to allow widgets to be edited in situ
    void editable(bool enable); 
    bool editable();

    // Pointer to the data for editor to use
    vector<item_def*>* data();
    // Draw display
    void create_form();

    protected:

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

    // Convert to points
    int to_points(float value);

};
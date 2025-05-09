#pragma once

#include "xml_wreader.h"
#include "qsl_data.h"

#include <istream>
#include <list>

using namespace std;


// XML element used in qsls.xml
enum qsl_element_t : char {
    QSL_NONE,         // Not yet processing an element
    QSL_QSLS,         // Top level elemene <qsls version=".."
    QSL_QSL,          // Individual design <qsl call="GM3ZZA" type="label">
    QSL_SIZE,         // Size of overall image <size unit="mm" width="nnn" height="nnn"/>
    QSL_ARRAY,        // Position of images on a printing page - 6 attributes
    QSL_DESIGN,       // Start of image design
    QSL_FORMATS,      // Date & time formats, max #qSOs per image
    QSL_TEXT,         // Start of text item <text>
    QSL_POSITION,     // Position of text, field or image item (x=, y=)
    QSL_LABEL,        // Text to write <label text="...", font= size= colour= />
    QSL_FIELD,        // Start of field item
    QSL_DATA,         // Field data <data field="...", font= size= colour= />
    QSL_OPTIONS,      // Options - vertical=y/n box=y/n multi=y/n always=y/n
    QSL_IMAGE,        // Start of image item
    QSL_FILE,         // Filename <file>.....</file>
};

class qsl_reader :
    public xml_wreader
{
public:
    qsl_reader();
    ~qsl_reader();

    // Load datra
    bool load_data(map<qsl_data::qsl_type, map<string, qsl_data*>* >* data, istream& in);

protected:
    // Start the specific elementys
    static bool start_qsls(xml_wreader* that, map<string, string>* attributes);
    static bool start_qsl(xml_wreader* that, map<string, string>* attributes);
    static bool start_size(xml_wreader* that, map<string, string>* attributes);
    static bool start_array(xml_wreader* that, map<string, string>* attributes);
    static bool start_design(xml_wreader* that, map<string, string>* attributes);
    static bool start_formats(xml_wreader* that, map<string, string>* attributes);
    static bool start_text(xml_wreader* that, map<string, string>* attributes);
    static bool start_position(xml_wreader* that, map<string, string>* attributes);
    static bool start_label(xml_wreader* that, map<string, string>* attributes);
    static bool start_field(xml_wreader* that, map<string, string>* attributes);
    static bool start_data(xml_wreader* that, map<string, string>* attributes);
    static bool start_options(xml_wreader* that, map<string, string>* attributes);
    static bool start_image(xml_wreader* that, map<string, string>* attributes);
    static bool start_file(xml_wreader* that, map<string, string>* attributes);

    // End the specific elements
    static bool end_qsls(xml_wreader* that);
    static bool end_qsl(xml_wreader* that);

    // Specific character processing
    static bool chars_file(xml_wreader* that, string content);
    static bool chars_label(xml_wreader* that, string content);
    static bool chars_data(xml_wreader* that, string content);
    

    // Check version
    bool check_version(string v);

    // Parse date and time formats
    static qsl_data::date_format parse_date(string s);
    static qsl_data::time_format parse_time(string s);

    // Convert font description to fint number
    static Fl_Font parse_font(string s);
    // Convert yes/no into bool
    static bool parse_bool(string s);


    // Name to element mapping
    const map<string, char> element_map_ = {
        { "QSLS", QSL_QSLS },
        { "QSL", QSL_QSL },
        { "SIZE", QSL_SIZE },
        { "ARRAY", QSL_ARRAY },
        { "DESIGN", QSL_DESIGN },
        { "FORMATS", QSL_FORMATS },
        { "TEXT", QSL_TEXT },
        { "POSITION", QSL_POSITION },
        { "LABEL", QSL_LABEL },
        { "DATA", QSL_DATA },
        { "OPTIONS", QSL_OPTIONS },
        { "FIELD", QSL_FIELD },
        { "IMAGE", QSL_IMAGE },
        { "FILE", QSL_FILE }
    };


    const map<char, methods> method_map_ = {
        { QSL_QSLS, { start_qsls, end_qsls, nullptr }},
        { QSL_QSL, { start_qsl, end_qsl, nullptr }},
        { QSL_SIZE, { start_size, nullptr, nullptr }},
        { QSL_ARRAY, { start_array, nullptr, nullptr }},
        { QSL_DESIGN, { start_design, nullptr, nullptr }},
        { QSL_FORMATS, { start_formats, nullptr, nullptr }},
        { QSL_TEXT, { start_text, nullptr, nullptr }},
        { QSL_POSITION, { start_position, nullptr, nullptr }},
        { QSL_LABEL, { start_label, nullptr, chars_label }},
        { QSL_FIELD, { start_field, nullptr, nullptr }},
        { QSL_DATA, { start_data, nullptr, chars_data }},
        { QSL_OPTIONS, { start_options, nullptr, nullptr }},
        { QSL_IMAGE, { start_image, nullptr, nullptr }},
        { QSL_FILE, { start_file, nullptr, chars_file }}    
    };

    // The data to load
    map<qsl_data::qsl_type, map<string, qsl_data*>* >* data_;
   // Attributes
    // callsign
    string callsign_;
    // type of QSL - file for e-mail, label for printing
    qsl_data::qsl_type type_;

    // Current qsl_data
    qsl_data* current_;
    // Current item definition
    qsl_data::item_def* item_;
    

};
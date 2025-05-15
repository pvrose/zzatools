#pragma once

#include "xml_wreader.h"
#include "qsl_data.h"

#include <istream>
#include <list>

using namespace std;

struct server_data_t;
struct qrz_api_data;


// XML element used in qsls.xml
enum qsl_element_t : char {
    QSL_NONE,         // Not yet processing an element
    QSL_QSL_DATA,     // Top level element <qsl_data version=".."
    QSL_QSLS,         // start of element <qsls>
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
    QSL_SERVERS,      // Start of data for servers
    QSL_SERVER,       // Start of individual server data
    QSL_VALUE,        // Individual value item
    QSL_LOGBOOK,      // Start of individual QRZ.com log book credentials
};

class qsl_reader :
    public xml_wreader
{
public:
    qsl_reader();
    ~qsl_reader();

    // Load data
    bool load_data(map<qsl_data::qsl_type, map<string, qsl_data*>* >* data, 
        map<string, server_data_t*>* servers_,
        istream& in);

protected:
    // Start the specific elementys
    static bool start_qsl_data(xml_wreader* w, map<string, string>* attributes);
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
    static bool start_servers(xml_wreader* w, map<string, string>* attributes);
    static bool start_server(xml_wreader* w, map<string, string>* attributes);
    static bool start_logbook(xml_wreader* w, map<string, string>* attributes);
    static bool start_value(xml_wreader* w, map<string, string>* attributes);

    // End the specific elements
    static bool end_qsl_data(xml_wreader* that);
    static bool end_qsl(xml_wreader* that);
    static bool end_value(xml_wreader* w);

    // Specific character processing
    static bool chars_file(xml_wreader* that, string content);
    static bool chars_label(xml_wreader* that, string content);
    static bool chars_data(xml_wreader* that, string content);
    static bool chars_value(xml_wreader* w, string content);
    

    // Check version
    bool check_version(string v);

    // Parse date and time formats
    static qsl_data::date_format parse_date(string s);
    static qsl_data::time_format parse_time(string s);

    // Convert font description to fint number
    static Fl_Font parse_font(string s);
    // Convert yes/no into bool
    static bool parse_bool(string s);
    // Decrypt string
    static string decrypt(string s, uchar offset);


    // Name to element mapping
    const map<string, char> element_map_ = {
        { "QSL_DATA", QSL_QSL_DATA },
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
        { "FILE", QSL_FILE },
        { "SERVERS", QSL_SERVERS },
        { "SERVER", QSL_SERVER },
        { "LOGBOOK", QSL_LOGBOOK },
        { "VALUE", QSL_VALUE },
    };


    const map<char, methods> method_map_ = {
        { QSL_QSL_DATA, { start_qsl_data, end_qsl_data, nullptr }},
        { QSL_QSLS, { start_qsls, nullptr, nullptr }},
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
        { QSL_FILE, { start_file, nullptr, chars_file }}, 
        { QSL_SERVERS, { start_servers, nullptr, nullptr }},
        { QSL_SERVER, { start_server, nullptr, nullptr }},
        { QSL_LOGBOOK, { start_logbook, nullptr, nullptr }},
        { QSL_VALUE, { start_value, end_value, chars_value }},

    };

    // The data to load
    map<qsl_data::qsl_type, map<string, qsl_data*>* >* data_;
    map<string, server_data_t*>* servers_;
   // Attributes
    // callsign
    string callsign_;
    // type of QSL - file for e-mail, label for printing
    qsl_data::qsl_type type_;

    // Current qsl_data
    qsl_data* current_;
    // Current item definition
    qsl_data::item_def* item_;
    // Current server data
    server_data_t* server_;
    // Current QRZ logbook api credentials
    qrz_api_data* api_data_;
    // Current value name
    string value_name_;
    // Current value data
    string value_data_;
    // Current encryption offset
    uchar offset_;
    // Name of parent
    string parent_name_;

};
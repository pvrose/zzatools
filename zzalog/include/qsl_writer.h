#pragma once

#include "xml_writer.h"
#include "qsl_data.h"

using namespace std;

enum qsl_element_t : char;
struct qrz_api_data;
struct server_data_t;

class qsl_writer :
    public xml_writer
{
public:
    qsl_writer();
    ~qsl_writer();

    bool store_data(
        map<qsl_data::qsl_type, map<string, qsl_data*>* >* data, 
        map<string, server_data_t*>* servers,
        ostream& os);

    bool write_element(qsl_element_t element);

protected:

    // Converts a font number to the appropriiate text
    string font2text(Fl_Font f);
    // write an individual value - string, integer and double versions
    bool write_value(string name, string data);
    bool write_value(string name, int data);
    bool write_value(string name, double data);
    bool write_value(string nam, bool data);
    // Encrypt data and convert to hex digits
    string encrypt(string s, uchar off);

    map<qsl_data::qsl_type, map<string, qsl_data*>* >* data_;
    map<string, server_data_t*>* servers_;
    // callsign
    string callsign_;
    qsl_data::qsl_type type_;

    // Current qsl_data
    qsl_data* current_;
    // Current item definition
    qsl_data::item_def* item_;
    // Current server data
    server_data_t* server_;
    // Name of current server
    string server_name_;
    // Current QRZ logbook api credentials
    qrz_api_data* api_data_;
    // Name of current log book
    string logbook_name_;
    // Encryption offset
    uchar offset_;
};
    


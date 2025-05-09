#pragma once

#include "xml_writer.h"
#include "qsl_data.h"

using namespace std;

enum qsl_element_t : char;

class qsl_writer :
    public xml_writer
{
public:
    qsl_writer();
    ~qsl_writer();

    bool store_data(map<qsl_data::qsl_type, map<string, qsl_data*>* >* data, ostream& os);

    bool write_element(qsl_element_t element);

protected:

    // Converts a font number to the appropriiate text
    string font2text(Fl_Font f);

    map<qsl_data::qsl_type, map<string, qsl_data*>* >* data_;
    // callsign
    string callsign_;
    qsl_data::qsl_type type_;

    // Current qsl_data
    qsl_data* current_;
    // Current item definition
    qsl_data::item_def* item_;
};
    


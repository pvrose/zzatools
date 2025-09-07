#pragma once

#include "xml_writer.h"
#include "qsl_data.h"

using namespace std;

enum qsl_element_t : char;
struct qsl_call_data;
struct server_data_t;

//! This class writes the QSL drawing data and QSL server credential
//! data to an XML file.
class qsl_writer :
    public xml_writer
{
public:
    //! Constructor.
    qsl_writer();
    //! Destructor.
    ~qsl_writer();

    //! Store the data.
    
    //! \param data QSL Design data.
    //! \param servers QSL server credential data.
    //! \param os output data stream.
    //! \return true if successful.
    bool store_data(
        map<qsl_data::qsl_type, map<string, qsl_data*>* >* data, 
        map<string, server_data_t*>* servers,
        ostream& os);

    //! Write an individual XML element.
    bool write_element(qsl_element_t element);

protected:

    //! Converts a font number to the appropriate text
    string font2text(Fl_Font f);
    //! write an individual string value name/data pair.
    bool write_value(string name, string data);
    //! write an individual integer value name/data pair.
    bool write_value(string name, int data);
    //! write an individual double precision value name/data pair.
    bool write_value(string name, double data);
    //! write an individual Boolean value name/data pair.
    bool write_value(string nam, bool data);
    //! Encrypt data \p s using offset \p off into keychain.
    string encrypt(string s, uchar off);

    //! QSL design data.
    map<qsl_data::qsl_type, map<string, qsl_data*>* >* data_;
    //! QSL server credentials.
    map<string, server_data_t*>* servers_;
    //! Current station callsign.
    string callsign_;
    //! Current QSL type.
    qsl_data::qsl_type type_;

    //! Current qsl_data
    qsl_data* current_;
    //! Current item definition
    qsl_data::item_def* item_;
    //! Current server data
    server_data_t* server_;
    //! Name of current server
    string server_name_;
    //! Current QRZ logbook api credentials
    qsl_call_data* api_data_;
    //! Name of current log book
    string logbook_name_;
    //! Encryption offset
    uchar offset_;
};
    


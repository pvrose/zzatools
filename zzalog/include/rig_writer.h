#pragma once

#include "xml_writer.h"

#include <map>
#include <string>

using namespace std;

enum rigs_element_t : char;
struct rig_data_t;
struct cat_data_t;

//! This class stores the internal rig configuration database to rigs.xml

//! \see rig_reader.
class rig_writer :
    public xml_writer
{
public:
    //! Constructor.
    rig_writer();
    //! Destructor.
    ~rig_writer();

    //! Write \p data to output stream \p os
    bool store_data(map<string, rig_data_t*>* data, ostream& os);

    //! Write an individual XML element.
    bool write_element(rigs_element_t element);

protected:
    //! Write "VALUE name="...">.....</VALUE> for string \p data,
    bool write_value(string name, string data);
    //! Write "VALUE name="...">.....</VALUE> for integer \p data,
    bool write_value(string name, int data);
    //! Write "VALUE name="...">.....</VALUE> for double-precision \p data,
    bool write_value(string name, double data);
    //! Name of rig currently being written
    string rig_name_;
    //! Name of app currently being written
    string app_name_;
    //! Rig configuration currently being written.
    rig_data_t* rig_data_;
    //! Rig interface configuration currently being written. 
    cat_data_t* app_data_;

    //! Rig configuration database.
    map<string, rig_data_t*>* data_;


};
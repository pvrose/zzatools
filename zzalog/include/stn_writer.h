#pragma once

#include "xml_writer.h"
#include "stn_data.h"

#include <map>
#include <string>

using namespace std;

enum stn_element_t : char;

//! This class write the internal station data to station.xml
class stn_writer :
    public xml_writer
{
public:
    //! Constructor.
    stn_writer();
    //! Destructor
    ~stn_writer();

    //! Store data items
    bool store_data(
        map<string, qth_info_t*>* qths,      //!< Location data 
        map<string, oper_info_t*>* opers,    //!< Operator data
        map<string, string>* scalls,         //!< Callsign data
        ostream& os                          //!< Output stream
    );

    //! Write an element and all the elements it contains. 
    bool write_element(stn_element_t element);

protected:
    //! Write an individual string \p data item
    bool write_item(string name, string data);
    //! Location data.
    map<string, qth_info_t*>* qths_;
    //! Operator data
    map<string, oper_info_t*>* opers_;
    //! Callsign data
    map<string, string>* scalls_;

    //! Current location name
    string qth_name_;
    //! Current Operator name
    string oper_name_;
    //! Current location data record
    qth_info_t* qth_;
    //! Current operator data record.
    oper_info_t* oper_;

    //! Map location item identifier to XML item name
    static map<qth_value_t, string> qth_value_map_;
    //! Map operator item identifier to XML item name
    static map<oper_value_t, string> oper_value_map_;

};


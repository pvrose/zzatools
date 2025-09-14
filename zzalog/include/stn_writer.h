#pragma once

#include "xml_writer.h"
#include "stn_data.h"

#include <map>
#include <string>



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
        std::map<std::string, qth_info_t*>* qths,      //!< Location data 
        std::map<std::string, oper_info_t*>* opers,    //!< Operator data
        std::map<std::string, std::string>* scalls,         //!< Callsign data
        std::ostream& os                          //!< Output stream
    );

    //! Write an element and all the elements it contains. 
    bool write_element(stn_element_t element);

protected:
    //! Write an individual std::string \p data item
    bool write_item(std::string name, std::string data);
    //! Location data.
    std::map<std::string, qth_info_t*>* qths_;
    //! Operator data
    std::map<std::string, oper_info_t*>* opers_;
    //! Callsign data
    std::map<std::string, std::string>* scalls_;

    //! Current location name
    std::string qth_name_;
    //! Current Operator name
    std::string oper_name_;
    //! Current location data record
    qth_info_t* qth_;
    //! Current operator data record.
    oper_info_t* oper_;

    //! Map location item identifier to XML item name
    static std::map<qth_value_t, std::string> qth_value_map_;
    //! Map operator item identifier to XML item name
    static std::map<oper_value_t, std::string> oper_value_map_;

};


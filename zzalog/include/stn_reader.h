#pragma once
#include "xml_wreader.h"
#include "stn_data.h"

#include <map>
#include <string>

using namespace std;

//! XML elements for station.xml
enum stn_element_t : char {
    STN_NONE,               //!< Not yet processing an element
    STN_STATION,            //!< Top-level 
    STN_LOCATION,           //!< Individual location
    STN_ITEM,               //!< Individual item
    STN_OPERATOR,           //!< Individual operator
    STN_NAME,               //!< Operator's name
    STN_SCALLSIGN,          //!< Station callsign
};

//! This class reads data from station.c#xml into the internal database stn_data.

/*! \code
<station>
  <location id="Crofthead">
    <item name="Country">SCOTLAND</item>
    <item name="DXCC">279 </item >
    <item name="CQ Zone">14 </item >
    <item name="Continent">EU</item>
  </location>
  <location id="Livingston">
  :
  </location>
  <operator id="PVR">
    <item name="Name">Phil</item>
    <item name="Callsign">GM3ZZA</item>
  </operator>
  <scallsign call="GM3ZZA"/>
  <scallsign call="GM3ZZA/A"/>
  <scallsign call="GQ3ZZA"/>
  <scallsign call="GR3ZZA"/>
</station>

\endcode
*/
class stn_reader :
    public xml_wreader
{
public:
    //! Constructor
    stn_reader();
    //! Destructor.
    ~stn_reader();

    //! Load data from station.xml
    bool load_data(
        map<string, qth_info_t*>* qths,   //!< Location data
        map<string, oper_info_t*>* opers, //!< Operator data 
        map<string, string>* calls,       //!< Callsign data
        istream& in                       //!< Input data stream
    );

protected:
    // The start methods
    static bool start_station(xml_wreader* rdr, map<string, string>* attributes);   //!< STATION
    static bool start_location(xml_wreader* rdr, map<string, string>* attributes);  //!< LOCATION
    static bool start_item(xml_wreader* rdr, map<string, string>* attributes);      //!< ITEM
    static bool start_operator(xml_wreader* rdr, map<string, string>* attributes);  //!< OPERATOR
    static bool start_scallsign(xml_wreader* rdr, map<string, string>* attributes); //!< SCALLSIGN
    // End mehods
    static bool end_station(xml_wreader* rdr);      //!< /STATION
    static bool end_location(xml_wreader* rdr);     //!< /LOCATION
    static bool end_item(xml_wreader* rdr);         //!< /ITEM
    static bool end_operator(xml_wreader* rdr);     //!< /OPERATOR
    // Character methods
    static bool chars_item(xml_wreader* rdr, string content);  //!< ITEM characters

    //! Maps text to element identifier
    const map<string, char> element_map_ = {
        { "STATION", STN_STATION },
        { "LOCATION", STN_LOCATION },
        { "ITEM", STN_ITEM },
        { "OPERATOR", STN_OPERATOR },
        { "SCALLSIGN", STN_SCALLSIGN }
    };

    //! Maps identifier to methods
    const map<char, methods> method_map_ = {
        { STN_STATION, { start_station, end_station, nullptr } },
        { STN_LOCATION, { start_location, end_location, nullptr } },
        { STN_ITEM, { start_item, end_item, chars_item } },
        { STN_OPERATOR, { start_operator, end_operator, nullptr } },
        { STN_SCALLSIGN, { start_scallsign, nullptr, nullptr } }
    };

    //! Map XML item name to location item identifier.
    static map<string, qth_value_t> qth_value_map_;
    //! Map XML item name to operator item identifier.
    static map<string, oper_value_t> oper_value_map_;

    //! Location data, mapped by identifier
    map<string, qth_info_t*>* qths_;
    //! Operator data, mapped by identifier
    map<string, oper_info_t*>* opers_;
    //! Station callsign data mapped by callsign
    map<string, string>* scalls_;
    //! Current location name
    string qth_name_;
    //! Current location data
    qth_info_t* qth_;
    //! Current operator ID
    string oper_name_;
    //! Current Operator data
    oper_info_t* oper_;
    //! Current item name
    string item_name_;
    //! Current item value
    string item_value_;
    //! Input stream 
    istream* in_file_;

};


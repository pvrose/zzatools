#pragma once
#include "xml_wreader.h"
#include "stn_data.h"

#include <map>
#include <string>

using namespace std;

enum stn_element_t : char {
    STN_NONE,               // Not yet processing an element
    STN_STATION,            // Top-level <station...
    STN_LOCATION,           // Individual QTH <location id=... description=...>
    STN_ITEM,               // Individual item <[item]>[value]</[item]>
    STN_OPERATOR,           // Individual operatr <operator id=>
    STN_NAME,               // Operator's name  <name>...</name>
    STN_SCALLSIGN,          // Station callsign
};

class stn_reader :
    public xml_wreader
{
public:
    stn_reader();
    ~stn_reader();

    // Loaddata
    bool load_data(
        map<string, qth_info_t*>* qths, 
        map<string, oper_info_t*>* opers, 
        map<string, string>* calls,
        istream& in);

protected:
    // The start methods
    static bool start_station(xml_wreader* rdr, map<string, string>* attributes);
    static bool start_location(xml_wreader* rdr, map<string, string>* attributes);
    static bool start_item(xml_wreader* rdr, map<string, string>* attributes);
    static bool start_operator(xml_wreader* rdr, map<string, string>* attributes);
    static bool start_scallsign(xml_wreader* rdr, map<string, string>* attributes);
    // End mehods
    static bool end_station(xml_wreader* rdr);
    static bool end_location(xml_wreader* rdr);
    static bool end_item(xml_wreader* rdr);
    static bool end_operator(xml_wreader* rdr);
    // Character methods
    static bool chars_item(xml_wreader* rdr, string content);

    // Name to element map
    const map<string, char> element_map_ = {
        { "STATION", STN_STATION },
        { "LOCATION", STN_LOCATION },
        { "ITEM", STN_ITEM },
        { "OPERATOR", STN_OPERATOR },
        { "SCALLSIGN", STN_SCALLSIGN }
    };

    const map<char, methods> method_map_ = {
        { STN_STATION, { start_station, end_station, nullptr } },
        { STN_LOCATION, { start_location, end_location, nullptr } },
        { STN_ITEM, { start_item, end_item, chars_item } },
        { STN_OPERATOR, { start_operator, end_operator, nullptr } },
        { STN_SCALLSIGN, { start_scallsign, nullptr, nullptr } }
    };

    static map<string, qth_value_t> qth_value_map_;
    static map<string, oper_value_t> oper_value_map_;

    // Location data
    map<string, qth_info_t*>* qths_;
    // Operator data
    map<string, oper_info_t*>* opers_;
    // Sation callsign data
    map<string, string>* scalls_;
    // Current location nale
    string qth_name_;
    // Current data
    qth_info_t* qth_;
    // Current operator ID
    string oper_name_;
    // Current Operator data
    oper_info_t* oper_;
    // Current item name
    string item_name_;
    // Current item value
    string item_value_;
    // Input stream 
    istream* in_file_;

};


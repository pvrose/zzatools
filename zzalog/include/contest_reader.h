#pragma once

#include "xml_wreader.h"

struct ct_data_t;
struct ct_date_t;
struct ct_exch_t;

class contest_data;

enum ct_element_t : char {
    CT_NONE,                // Not yet reading an element
    CT_CONTESTS,            // Outermost element
    CT_CONTEST,             // Individual contest definition
    CT_TIMEFRAME,           // Timeframe
    CT_VALUE,               // Individual fields
    CT_EXCHANGE,            // Individual exchange definition
};

/*
*   <CONTESTS>
*     <CONTEST id=[CONTEST_ID] index=[n]>
*       <VALUE name=fields>[field-set name]</VALUE>
*       <TIMEFRAME start=[start time] finish=[finish time] />
*       <VALUE name=exchange>[exchange id]</VALUE>
*       <VALUE name=scoring>[scoring algo id]</VALUE>
*     </CONTEST>
*     <CONTEST....
*     <EXCHANGE id=[exchange id]
*       <VALUE name=send>[send exchange]</VALUE>
*       <VALUE name=receive>[receive exchange]</VALUE>
*     </EXCHANGE>
*     <EXCHANGE....
*   </CONTESTS>
*/

class contest_reader :
    public xml_wreader
{
public:
    contest_reader();
    ~contest_reader();

    // Load data
    bool load_data(contest_data* d, istream& is);

protected:
    // Start <CONTESTS>
    static bool start_contests(xml_wreader* that, map<string, string>* attributes);
    // Start <CONTEST id= index=>
    static bool start_contest(xml_wreader* that, map<string, string>* attributes);
    // Start <TIMEFRAME start= finish=>
    static bool start_timeframe(xml_wreader* that, map<string, string>* attributes);
    // Start <VALUE name=>
    static bool start_value(xml_wreader* that, map<string, string>* attributes);
    // Start <EXCHANGE id=>
    static bool start_exchange(xml_wreader* that, map<string, string>* attributes);
    // End </CONTESTS>
    static bool end_contests(xml_wreader* that);
    // End </CONTEST>
    static bool end_contest(xml_wreader* that);
    // End </VALUE>
    static bool end_value(xml_wreader* that);
    // End </EXCHANGE>
    static bool end_exchange(xml_wreader* that);
    // Characters in value
    static bool chars_value(xml_wreader* that, string content);

    const map<string, char> element_map_ = {
        { "CONTESTS", CT_CONTESTS },
        { "CONTEST", CT_CONTEST },
        { "TIMEFRAME", CT_TIMEFRAME },
        { "VALUE", CT_VALUE },
        { "EXCHANGE", CT_EXCHANGE },
    };

    const map<char, methods> method_map_ = {
        { CT_CONTESTS, { start_contests, end_contests, nullptr } },
        { CT_CONTEST, { start_contest, end_contest, nullptr } },
        { CT_TIMEFRAME, { start_timeframe, nullptr, nullptr }},
        { CT_VALUE, { start_value, end_value, chars_value }},
        { CT_EXCHANGE, { start_exchange, end_exchange, nullptr }},
    };

    // date being read
    contest_data* the_data_;

    // Current contest
    ct_data_t* contest_data_;
    // Current contest ID
    string contest_id_;
    // Cureent contest index
    string contest_ix_;
    // Current exchange
    ct_exch_t* exchange_;
    // Current exchange ID
    string exchange_id_;
     // Input stream 
    istream* in_file_;
    // Current value name
    string value_name_;
    // Current value data
    string value_data_;
};


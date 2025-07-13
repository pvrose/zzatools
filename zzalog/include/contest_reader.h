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
    CT_ALGORITHM,               // Individual fields
};

/*
*   <CONTESTS>
*     <CONTEST id=[CONTEST_ID] index=[n]>
*       <TIMEFRAME start=[start time] finish=[finish time] />
*       <ALGORITHM name=[algo id] />
*     </CONTEST>
*     <CONTEST....
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
    // Start <ALGORITHM name=>
    static bool start_algorithm(xml_wreader* that, map<string, string>* attributes);
     // End </CONTESTS>
    static bool end_contests(xml_wreader* that);
    // End </CONTEST>
    static bool end_contest(xml_wreader* that);

    const map<string, char> element_map_ = {
        { "CONTESTS", CT_CONTESTS },
        { "CONTEST", CT_CONTEST },
        { "TIMEFRAME", CT_TIMEFRAME },
        { "ALGORITHM", CT_ALGORITHM },
    };

    const map<char, methods> method_map_ = {
        { CT_CONTESTS, { start_contests, end_contests, nullptr } },
        { CT_CONTEST, { start_contest, end_contest, nullptr } },
        { CT_TIMEFRAME, { start_timeframe, nullptr, nullptr }},
        { CT_ALGORITHM, { start_algorithm, nullptr, nullptr }},
    };

    // date being read
    contest_data* the_data_;

    // Current contest
    ct_data_t* contest_data_;
    // Current contest ID
    string contest_id_;
    // Cureent contest index
    string contest_ix_;
    // Input stream 
    istream* in_file_;
};


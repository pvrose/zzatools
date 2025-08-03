#pragma once
#include "cty_data.h"

#include "utils.h"

class cty2_data :
    public cty_data
{

    struct pattern_entry {
        int dxcc = -1;
        int cqzone = 0;
        int ituzone = 0;
        bool call_exception = false;
    };

    struct prefix_entry {
        string nickname = "";
        string name = "";
        int cqzone = 0;
        int ituzone = 0;
        string continent = "";
        lat_long_t location = { nan(""), nan("") };
        double timezone = 0.0;
        map<string, pattern_entry*> patterns = {};
    };

    typedef map<int, prefix_entry*> data_t;

public:

    friend class cty2_reader;

    cty2_data();
    ~cty2_data();

    // Return various fields of entity
    string nickname(record* qso);
    string name(record* qso);
    string continent(record* qso);
    int cq_zone(record* qso);
    int itu_zone(record* qso);
    // Get location
    lat_long_t location(record* qso);
    // Update record based on parsing
    bool update_qso(record* qso, bool my_call = false);
    // Get location details
    string get_tip(record* qso);
    // Parsing source
    parse_source_t get_source(record* qso);
    // Return entity 
    int entity(record* qso);

    // Get entity for nickname and vice-versa
    int entity(string nickname);
    string nickname(int adif_id);

protected:

    bool load_data(string filename);

    string get_filename();

    void parse(record* qso);

    pattern_entry* pattern(string call, map<string, pattern_entry*> patterns);

    data_t* data_;

    map<string, pattern_entry*> patterns_;

    // Parse result
    struct {
        prefix_entry* pfx_entry;
        pattern_entry* patt_entry;
    } parse_result_;

    // Record for last parse result
    record* qso_;
    // Call sign for the previous parse result
    string parse_call_;


};


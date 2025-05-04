#pragma once

#include "xml_reader.h"

#include <map>
#include <string>
#include <list>

using namespace std;

struct rig_data_t;
struct cat_data_t;

// XML element types used in rigs.xml
enum rigs_element_t : char {
    RIG_NONE,          // Not yet processing an elemaent
    RIG_RIGS,          // Top-level element <rigs version=".."
    RIG_RIG,           // Start of individual rig <rig name="..">
    RIG_VALUE,         // common rig data item <value name="..">,,,</Vvalue>
    RIG_APP,           // Start of individual app-specific data
};

class rig_reader :
    public xml_reader
{
public:
    rig_reader();
    ~rig_reader();

    // Overloaded xml_reader methods
    // Start 
    virtual bool start_element(string name, map<string, string>* attributes);
    // End
    virtual bool end_element(string name);
    // Special element
    virtual bool declaration(xml_element::element_t element_type, string name, string content);
    // Processing instruction
    virtual bool processing_instr(string name, string content);
    // characters
    virtual bool characters(string content);
 
    // Load data
    bool load_data(map<string, rig_data_t*>* data, istream& in);

protected:
    // Start <rigs version = .....
    static bool start_rigs(rig_reader* that, map<string, string>* attributes);
    // Start <rig name=
    static bool start_rig(rig_reader* that, map<string, string>* attributes);
    // Start <value name=
    static bool start_value(rig_reader* that, map<string, string>* attributes);
    // Start <app name=...
    static bool start_app(rig_reader* that, map<string, string>* attributes);
    // End </rigs>
    static bool end_rigs(rig_reader* that);
    // End </rig>
    static bool end_rig(rig_reader* that);
    // End </app>
    static bool end_app(rig_reader* that);
    // End </value>
    static bool end_value(rig_reader* that);
    // Characters <value ..>..<\value>
    static bool chars_value(rig_reader* that, string content);
    // Check version
    bool check_version(string v);

 
    // Name to element map
    const map<string, rigs_element_t> element_map_ = {
        { "RIGS", RIG_RIGS },
        { "RIG", RIG_RIG },
        { "VALUE", RIG_VALUE },
        { "APP", RIG_APP }
    };

    struct methods {
        bool (*start_method)(rig_reader*, map<string, string>*);
        bool (*end_method)(rig_reader*);
        bool (*chars_method)(rig_reader*, string);
    };

    // Element to start method map
    const map<rigs_element_t, methods> method_map_ = {
        { RIG_RIGS, { start_rigs, end_rigs, nullptr }},
        { RIG_RIG, { start_rig, end_rig, nullptr }},
        { RIG_VALUE, { start_value, end_value, chars_value }},
        { RIG_APP, { start_app, end_app, nullptr }}
    };

    // The data being loaded
    map<string, rig_data_t*>* data_;
    // List of elements
    list<rigs_element_t> elements_;
    // Current rig data
    rig_data_t* rig_data_;
    // string current rig name
    string rig_name_;
    // Current app_name
    string app_name_;
    // Current app data
    cat_data_t* app_data_;
    // Input stream 
    istream* in_file_;
    // Current value name
    string value_name_;
    // Current value data
    string value_data_;
};
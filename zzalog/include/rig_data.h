#pragma once

#include <string>
#include <set>
#include <map>
#include <vector>

#include <FL/Fl_Preferences.H>

using namespace std;

struct hamlib_data_t;

struct cat_data_t {
    hamlib_data_t* hamlib = nullptr;
    bool use_cat_app = false;
    bool override_hamlib = false;
    string app = "";
    string nickname = "";
    bool auto_start = false;
    bool auto_connect = false;
    double connect_delay = 1.0;
};

struct rig_data_t {
    int default_app = -1;
    string antenna = "";
    bool use_instant_values = false;
    vector<cat_data_t*> cat_data;
};

class rig_data {

public:
    // Constructor
    rig_data();
    // Destructor
    ~rig_data();
    // Get CAT data for rig and app. - Pointer so that it can be written to.
    cat_data_t* cat_data(string rig, int app = -1);
    //Get list of supported rigs
    vector<string> rigs();
    // Get rig data for rig
    rig_data_t* get_rig(string rig); 
    

protected:
    // Load data from preferences directory
    void load_data();
    // Load data from preferences file
    void load_prefs(Fl_Preferences& settings);
    // Load data from XML 
    bool load_xml();
    // Store data to preferences directory
    bool store_data();
    // Store data to XML
    bool store_xml();
    // The rig data
    map<string, rig_data_t*> data_;

};
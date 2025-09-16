#pragma once

#include "nlohmann/json.hpp"

#include <string>
#include <set>
#include <map>
#include <vector>

#include <FL/Fl_Preferences.H>

using json = nlohmann::json;

struct hamlib_data_t;

//! This structure provides the configuration data for the rig interface.
struct cat_data_t {
    hamlib_data_t* hamlib = nullptr;   //!< Hamlib API configuration  
    bool use_cat_app = false;          //!< Use another app for accessing CAT interface.
    bool override_hamlib = false;      //!< Override values obtained from CAT interface
    std::string app = "";              //!< Name of the command to launch app
    std::string nickname = "";         //!< Short form of the name used in CAT menu.
    bool auto_start = false;           //!< Automatically start app when ZZALOG starts.
    bool auto_connect = false;         //!< Automatically connect to app after starting it.
    double connect_delay = 1.0;        //!< Delay between starting app and connecting (in seconds).
};
//! Convert cat_data_t to JSON structure
void to_json(json& j, const cat_data_t& s);
//! Convert JSON structure to cat_data_t
void from_json(const json& j, cat_data_t& s);

//! This structure configures the use of the rig interface.
struct rig_data_t {          
    int default_app = -1;              //!< Index into cat_data for the default CAT method.
    std::string antenna = "";          //!< Preferred antenna when using this rig.
    bool use_instant_values = false;   //!< Use values just radfrom rig rather than smoothed ones.
    std::vector<cat_data_t*> cat_data; //!< Methods of accessing this particular rig.
};
//! Convert rig_data_t to JSON structure
void to_json(json& j, const rig_data_t& s);
//! Convert JSON structure to rig_data_t
void from_json(const json& j, rig_data_t& s);

//! This class provides the data required for configuring, accessing and using each rig.
class rig_data {

public:
    //! Constructor
    rig_data();
    //! Destructor
    ~rig_data();
    //! Returns reference to the CAT for the \p rig with index \ app. 
    cat_data_t* cat_data(std::string rig, int app = -1);
    //! Returns all the rigs currently supported by this database.
    std::vector<std::string> rigs();
    //! Returns the rig_data_t structure for the specified \p rig.
    rig_data_t* get_rig(std::string rig); 
    

protected:
    //! Load data from rigs.xml
    void load_data();
    //! LOad data from JSON
    bool load_json();
    //! Store data  to rigs,xml.
    bool store_data();
    //! Store data as JSON
    bool store_json();
    //! Configuration data for all rigs.
    std::map<std::string, rig_data_t*> data_;
    //! Load failed.
    bool load_failed_;
};
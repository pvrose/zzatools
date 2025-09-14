#pragma once

#include "xml_wreader.h"

#include <map>
#include <string>
#include <list>



struct rig_data_t;
struct cat_data_t;

//! XML element types used in rigs.xml
enum rigs_element_t : char {
    RIG_NONE,          //!< Not yet processing an elemaent
    RIG_RIGS,          //!< Top-level element rigs version=".."
    RIG_RIG,           //!< Start of individual rig rig name=".."
    RIG_VALUE,         //!< common rig data item value name="..">
    RIG_APP,           //!< Start of individual app-specific data
};

//! This class reads the rigs.xml file into the internal database.
//! 
//! \code
//! <RIGS version="3.6.6">
//!   <RIG name="IC-705">
//!     <value name="Default App">0</value>
//!     <value name="Instantaneous Values">0 </value >
//!     <app name="WFView WAN">
//!       <value name="Rig Model">NET rigctl</value>
//!       <value name="Manufacturer">Hamlib</value>
//!       :
//!     </app>
//!     :
//!   </RIG>
//!   :
//! </RIGS>
//! \endcode
class rig_reader :
    public xml_wreader
{
public:
    //! Constructor.
    rig_reader();
    //! Destructor.
    ~rig_reader();

    //! Load \p data from input stream \p in.
    bool load_data(std::map<std::string, rig_data_t*>* data, std::istream& in);

protected:
    //! Start "rigs version=....."
    static bool start_rigs(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start "rig name="
    static bool start_rig(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start "value name="
    static bool start_value(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! Start "app name=..."
    static bool start_app(xml_wreader* that, std::map<std::string, std::string>* attributes);
    //! End RIGS
    static bool end_rigs(xml_wreader* that);
    //! End RIG
    static bool end_rig(xml_wreader* that);
    //! End APP
    static bool end_app(xml_wreader* that);
    //! End VALUE
    static bool end_value(xml_wreader* that);
    //! Characters VALUE
    static bool chars_value(xml_wreader* that, std::string content);
    //! Check ZZALOG version against \p v
    bool check_version(std::string v);

 
    //! Name to element std::map
    const std::map<std::string, char> element_map_ = {
        { "RIGS", RIG_RIGS },
        { "RIG", RIG_RIG },
        { "VALUE", RIG_VALUE },
        { "APP", RIG_APP }
    };

    //! Element to start method std::map
    const std::map<char, methods> method_map_ = {
        { RIG_RIGS, { start_rigs, end_rigs, nullptr }},
        { RIG_RIG, { start_rig, end_rig, nullptr }},
        { RIG_VALUE, { start_value, end_value, chars_value }},
        { RIG_APP, { start_app, end_app, nullptr }}
    };

    //! The data being loaded
    std::map<std::string, rig_data_t*>* data_;
    //! Current rig data
    rig_data_t* rig_data_;
    //! std::string current rig name
    std::string rig_name_;
    //! Current app_name
    std::string app_name_;
    //! Current app data
    cat_data_t* app_data_;
    //! Input stream 
    std::istream* in_file_;
    //! Current value name
    std::string value_name_;
    //! Current value data
    std::string value_data_;
};



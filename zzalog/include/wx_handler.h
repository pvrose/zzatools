#pragma once

#include "utils.h"

#include <string>
#include <list>
#include <thread>
#include <atomic>



class Fl_Image;

//! The data received in a weather reports
struct wx_report {

    unsigned int city_id{ 0 };       //!< City ID number
    std::string city_name;                //!< Location name
    lat_long_t city_location;        //!< Latitude and longitude
    std::string iso_country;              //!< ISO country code (eg GB)
    float timezone_hr{ 0.0F };       //!< Timezone difference (hours)
    time_t sunrise{ 0 };             //!< Sunrise (UTC)
    time_t sunset{ 0 };              //!< Sunset (UTC)
    float temperature_K{ 0.0F };     //!< Temperature (kelvin) 
    float subjective_K{ 0.0F };      //!< Subjective temperature
    unsigned int humidity_pc{ 0 };   //!< Relative humidity
    unsigned int pressure_hPa{ 0 };  //!< Air pressure (hPa - aka mbar)
    float wind_speed_ms{ 0.0F };     //!< Wind speed (m/s)
    std::string wind_name;                //!< Wind name (Beaufort scale?)
    unsigned int wind_dirn{ 0 };     //!< Wind direction (degrees);
    std::string wind_cardinal;            //!< Cardinal (sixteenth) whence the wind
    float gusting_ms{ 0.0F };        //!< Gusting up to ... (m/s)
    unsigned int cloud_cover{ 0 };   //!< Percentage cloud cover
    std::string cloud_name;               //!< Cloud cover description
    unsigned int visibility_m{ 0 };  //!< Visibility (m - max 10 km)
    std::string precipitation;            //!< Precipitation "no", "rain", "snow"
    float precip_mm{ 0 };            //!< Precipitation in last hour (mm)
    std::string description;              //!< Overall weather description
    Fl_Image* icon{ nullptr };       //!< Weather image
    time_t updated{ 0 };             //!< Time last updated

};

//! This class decodes a weather report receved from openweathermap.org as an XML file

//! \todo Shut the stable door after the horse has bolted! Keys en-clair.
class wx_handler {

public:
    //! Constructor.
    wx_handler();
    //! Destructor.
    ~wx_handler();

    //! \cond
    // openweathermap.org key
    const char* key_ = "0b2145b6b923a9561f4b4831f5d6d66f";
    //! \endcond
    
    //! Update weather report - forecd
    void update();
    //! Implement timer actions 
    void ticker();
    //! Callback from ticker every 30 minutes (3 minutes in DEBUG_QUICK mode).
    static void cb_ticker(void* v);
    //! Callback from fetch std::thread when complete
    static void cb_fetch_done(void* v);
    //! Callback from fetch std::thread on error.
    static void cb_fetch_error(void* v);
    
    // Get the various weather items - 
    //! Returns icon
    Fl_Image* icon();
    //! Returns description
    std::string description();
    //! Returns temperature (in kelvins)
    float temperature();
    //! Returns wind-speed (in metres per second)
    float wind_speed();
    //! Returns wind speed name
    std::string wind_name();
    //! Returns wind direction (in 16th cardinals)
    std::string wind_direction();
    //! Returns wind direction (in degrees)
    unsigned int wind_degrees();
    //! Returns fraction cloud cover.
    float cloud();
    //! Returns cloud description
    std::string cloud_name();
    //! Returns sunrise time
    time_t sun_rise();
    //! Returns sunset time
    time_t sun_set();
    //! Returns last updated
    time_t last_updated();
    //! Returns location
    std::string location();
    //! Returns location coordinates
    std::string latlong();
    //! Returns atmospheric pressue (in hectopascals)
    float pressure();

    //! Fetch icon
    static Fl_Image* fetch_icon(std::string name);

    //! Returns cardinal direction
    static std::string wind_cardinal(int dirn);
    //! Returns beaufort wind description
    static std::string beaufort(float speed);


protected:

    //! Run the fetch std::thread
    static void do_thread(wx_handler* that);
    //! The fetch std::thread
    std::thread* wx_thread_;
    //! True when data has been received from weather server
    std::atomic<bool> wx_valid_;
    //! True start fetching data from weather server.
    std::atomic<bool> wx_fetch_;

    //! Current weather report
    wx_report report_;
    //! Element data
    std::string element_data_;
    //! Unit value
    std::string unit_;
    //! Error code
    int error_code_;
    //! Error message
    std::string error_message_;

};

#pragma once

#include "xml_reader.h"
#include "utils.h"

#include <string>
#include <list>
#include <thread>
#include <atomic>

using namespace std;

class Fl_Image;

// WX XML elements
enum wxe_element_t {
    WXE_NONE,                   // No element yste
    WXE_CURRENT,                // Current weather report
    WXE_CITY,                   // City definition
    WXE_COORD,                  // City coordinates
    WXE_COUNTRY,                // ISO Country ID (eg GB)
    WXE_TIMEZONE,               // Timezone difference
    WXE_SUN,                    // City sunrise/sunset
    WXE_TEMPERATURE,            // Temperature values 
    WXE_FEELS_LIKE,             // Subjective temperature
    WXE_HUMIDITY,               // Humidity value
    WXE_PRESSURE,               // Pressure value
    WXE_WIND,                   // Wind value/description
    WXE_SPEED,                  // Wind speed value and beaufort description
    WXE_GUSTS,                  // Gusting wind speed
    WXE_DIRECTION,              // Direction
    WXE_CLOUDS,                 // Cloud cover value, description
    WXE_VISIBILITY,             // Visibility (metres - max 10 km)
    WXE_PRECIPITATION,          // Rain/snow levels
    WXE_WEATHER,                // Weather summary
    WXE_LASTUPDATE,             // Last update time
    WXE_CLIENTERROR,            // Error message
    WXE_COD,                    // Error code
    WXE_MESSAGE,                // ERror message
};

// The data received in a weather reports
struct wx_report {

    unsigned int city_id;       // City ID number
    string city_name;           // Location name
    lat_long_t city_location;   // Latitude and longitude
    string iso_country;         // ISO country code (eg GB)
    float timezone_hr;          // Timezone difference (hours)
    time_t sunrise;             // Sunrise (UTC)
    time_t sunset;              // Sunset (UTC)
    float temperature_K;        // Temperature (kelvin) 
    float subjective_K;         // Subjective temperature
    unsigned int humidity_pc;   // Relative humidity
    unsigned int pressure_hPa;  // Air pressure (hPa - aka mbar)
    float wind_speed_ms;        // Wind speed (m/s)
    string wind_name;           // Wind name (Beaufort scale?)
    unsigned int wind_dirn;     // Wind direction (degrees);
    string wind_cardinal;       // Cardinal (sixteenth) whence the wind
    float gusting_ms;           // Gusting up to ... (m/s)
    unsigned int cloud_cover;   // Percentage cloud cover
    string cloud_name;          // Cloud cover description
    unsigned int visibility_m;  // Visibility (m - max 10 km)
    string precipitation;       // Precipitation "no", "rain", "snow"
    float precip_mm;            // Precipitation in last hour (mm)
    string description;         // Overall weather description
    Fl_Image* icon;             // Weather image
    time_t updated;             // Time last updated

};

// This class decodes a weather report receved from openweathermap.org
// as an XML file
class wx_handler : public xml_reader {

public:
    
    wx_handler();
    ~wx_handler();

    // openweathermap.org key
    const char* key_ = "0b2145b6b923a9561f4b4831f5d6d66f";

    // XML reader overloads
    // Overloadable XML handlers
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

    // Update weather report - forecd
    void update();
    // Timer 
    void ticker();
    static void cb_ticker(void* v);
    
    // Get the various weather items - 
    // summation icon
    Fl_Image* icon();
    // Description
    string description();
    // Temperature (K)
    float temperature();
    // Wind-speed (m/s)
    float wind_speed();
    // Wind speed name
    string wind_name();
    // Wind direction (16th cardinals)
    string wind_direction();
    // Wind direction (degrees)
    unsigned int wind_degrees();
    // Cloudiness
    float cloud();
    // Name
    string cloud_name();
    // Sunris
    time_t sun_rise();
    // Sun set
    time_t sun_set();
    // Last updated
    time_t last_updated();
    // Location
    string location();
    // Location lat/long
    string latlong();
    // Atmospheric pressue (hPa)
    float pressure();

protected:

    // The overall XML container
    bool start_current();
    bool end_current();
    // City data
    bool start_city(map<string, string>* attributes);
    bool end_city();
    // City coordinates
    bool start_coord(map<string, string>* attributes);
    bool end_coord();
    // Country
    bool start_country();
    bool end_country();
    // Timezone
    bool start_timezone();
    bool end_timezone();
    // Sunrise/set
    bool start_sun(map<string, string>* attributes);
    bool end_sun();
    // Gemperature
    bool start_temperature(map<string, string>* attributes);
    bool end_temperature();
    // Feelslike
    bool start_subjective(map<string, string>* attributes);
    bool end_subjective();
    // Humidity
    bool start_humidity(map<string, string>* attributes);
    bool end_humidity();
    // Pressure
    bool start_pressure(map<string, string>* attributes);
    bool end_pressure();
    //Wind
    bool start_wind();
    bool end_wind();
    // Wind speed
    bool start_wind_speed(map<string, string>* attributes);
    bool end_wind_speed();
    // Wind direction
    bool start_wind_dirn(map<string, string>* attributes);
    bool end_wind_dirn();
    // Win d gusts
    bool start_gusts(map<string, string>* attributes);
    bool end_gusts();
    // Cloud cover
    bool start_clouds(map<string, string>* attributes);
    bool end_clouds();
    // Visibility
    bool start_visibility(map<string, string>* attributes);
    bool end_visibility();
    // Precipitation
    bool start_precipitation(map<string, string>* attributes);
    bool end_precipitation();
    // Overall weathre
    bool start_weather(map<string, string>* attributes);
    bool end_weather();
    // Last update
    bool start_updated(map<string, string>* attributes);
    bool end_update();
    // Error handling
    bool start_clienterror();
    bool end_clienterror();
    // Error code
    bool start_cod();
    bool end_cod();
    // Error message
    bool start_message();
    bool end_message();


    // Convert ISO format to time_t
    time_t convert_date(string s);
    // Fetch icon
    Fl_Image* fetch_icon(string name);

    // Run the fetch thread
    static void do_thread(wx_handler* that);
    // The thread
    thread* wx_thread_;
    // Thread control
    atomic<bool> run_thread_;
    atomic<bool> wx_fetch_;
    atomic<bool> wx_valid_;


    // Current elements being processed
    list<wxe_element_t> elements_;
    // Current weather report
    wx_report report_;
    // Element data
    string element_data_;
    // Unit value
    string unit_;
    // Error code
    int error_code_;
    // Error message
    string error_message_;

};

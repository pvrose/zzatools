#pragma once

#include "xml_reader.h"
#include "utils.h"

#include <string>
#include <list>
#include <thread>
#include <atomic>



class Fl_Image;

//! WX XML elements
enum wxe_element_t {
    WXE_NONE,                   //!< No element yste
    WXE_CURRENT,                //!< Current weather report
    WXE_CITY,                   //!< City definition
    WXE_COORD,                  //!< City coordinates
    WXE_COUNTRY,                //!< ISO Country ID (eg GB)
    WXE_TIMEZONE,               //!< Timezone difference
    WXE_SUN,                    //!< City sunrise/sunset
    WXE_TEMPERATURE,            //!< Temperature values 
    WXE_FEELS_LIKE,             //!< Subjective temperature
    WXE_HUMIDITY,               //!< Humidity value
    WXE_PRESSURE,               //!< Pressure value
    WXE_WIND,                   //!< Wind value/description
    WXE_SPEED,                  //!< Wind speed value and beaufort description
    WXE_GUSTS,                  //!< Gusting wind speed
    WXE_DIRECTION,              //!< Direction
    WXE_CLOUDS,                 //!< Cloud cover value, description
    WXE_VISIBILITY,             //!< Visibility (metres - max 10 km)
    WXE_PRECIPITATION,          //!< Rain/snow levels
    WXE_WEATHER,                //!< Weather summary
    WXE_LASTUPDATE,             //!< Last update time
    WXE_CLIENTERROR,            //!< Error message
    WXE_COD,                    //!< Error code
    WXE_MESSAGE,                //!< Error message
};

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
/*! \code
<current>
  <city id="2656565" name="Ballater">
    <coord lon="-3" lat="57"/>
    <country>GB</country>
    <timezone>3600</timezone>
    <sun rise="2025-09-10T05:32:49" std::set="2025-09-10T18:45:28"/>
  </city>
  <temperature value="282.8" min="282.8" max="282.8" unit="kelvin"/>
  <feels_like value="280.96" unit="kelvin"/>
  <humidity value="86" unit="%"/>
  <pressure value="1003" unit="hPa"/>
  <wind>
    <speed value="3.52" unit="m/s" name="Gentle Breeze"/>
    <gusts value="11.9"/>
    <direction value="143" code="SE" name="SouthEast"/>
  </wind>
  <clouds value="25" name="scattered clouds"/>
  <visibility value="10000"/>
  <precipitation mode="no"/>
  <weather number="802" value="scattered clouds" icon="03d"/>
  <lastupdate value="2025-09-10T07:24:59"/>
</current>
\endcode
*/
class wx_handler : public xml_reader {

public:
    //! Constructor.
    wx_handler();
    //! Destructor.
    ~wx_handler();

    //! \cond
    // openweathermap.org key
    const char* key_ = "0b2145b6b923a9561f4b4831f5d6d66f";
    //! \endcond
    
    // XML reader overloads
    // Overloadable XML handlers
    //! Start Element 
    virtual bool start_element(std::string name, std::map<std::string, std::string>* attributes);
    //! End Element
    virtual bool end_element(std::string name);
    //! Special element
    virtual bool declaration(xml_element::element_t element_type, std::string name, std::string content);
    //! Processing instruction
    virtual bool processing_instr(std::string name, std::string content);
    //! characters
    virtual bool characters(std::string content);

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

protected:

    //! Start The overall XML container CURRENT
    bool start_current();
    //! end The overall XML container CURRENT
    bool end_current();
    //! Start CITY: Attributes id, name.
    bool start_city(std::map<std::string, std::string>* attributes);
    //! End CITY
    bool end_city();
    //! Start COORD: Attributes lon, lat.
    bool start_coord(std::map<std::string, std::string>* attributes);
    //! End COORD Element
    bool end_coord();
    //! Start COUNTRY Element: Data 2-character ISO code.
    bool start_country();
    //! End COUNTRY Element
    bool end_country();
    //! Start TIMEZONE element: Data Delta from UTC in seconds
    bool start_timezone();
    //! End TIMEZONE
    bool end_timezone();
    //! Start SUN element: Attributes rise, std::set as XML date/time format (UTC).
    bool start_sun(std::map<std::string, std::string>* attributes);
    //! End SUN element
    bool end_sun();
    //! Start TEMPERATURE element: Attributes value, min, max, unit.
    
    //! Unit is celsius, fahrenheit or kelvin.
    bool start_temperature(std::map<std::string, std::string>* attributes);
    //! End TEMPERATURE
    bool end_temperature();
    //! Start FEELS_LIKE element: Attrubutes value, unit

    //! Unit is celsius, fahrenheit or kelvin.
    bool start_subjective(std::map<std::string, std::string>* attributes);
    //! End FEELS_LIKE 
    bool end_subjective();
    //! Start HUMIDITY element: Attributes value, unit
    
    //! Unit is percentage
    bool start_humidity(std::map<std::string, std::string>* attributes);
    //! End HUMIDITY
    bool end_humidity();
    //! Start PRESSURE element : Attributes value unit
    
    //! Unit is hPa (hectopascal)
    bool start_pressure(std::map<std::string, std::string>* attributes);
    //! End PRESSURE
    bool end_pressure();
    //! Start WIND element
    bool start_wind();
    //! End WIND
    bool end_wind();
    //! Start (WIND)SPEED element: Attributes value, unit name
     
    //! Unit is m/s (meters per second) or mph (miles per hour)
    bool start_wind_speed(std::map<std::string, std::string>* attributes);
    //! End WIND(SPEED)
    bool end_wind_speed();
    //! Start WIND(DIRECTION) element: Attributes value, code, name
    bool start_wind_dirn(std::map<std::string, std::string>* attributes);
    //! End WIND(DIRECTION)
    bool end_wind_dirn();
    //! Start WIND(GUSTS) element: Attributes value
    bool start_gusts(std::map<std::string, std::string>* attributes);
    //! End WIND(GUSTS)
    bool end_gusts();
    //! Start CLOUDS element: Attributes value name - value in percentage
    bool start_clouds(std::map<std::string, std::string>* attributes);
    //! End CLOUDS
    bool end_clouds();
    //! Start VISIBILITY element: attributes value - value in metres (maximum 10 km)
    bool start_visibility(std::map<std::string, std::string>* attributes);
    //! End VISIBILITY
    bool end_visibility();
    //! Start PRECIPITATION element: attributes mode
    bool start_precipitation(std::map<std::string, std::string>* attributes);
    //! End PRECIPITATION
    bool end_precipitation();
    //! Start WEATHER element: attributes number, value, icon
    bool start_weather(std::map<std::string, std::string>* attributes);
    //! End WEATHER
    bool end_weather();
    //! Start LASTUPDATE element: attributes value - value in XML date/time format (UTC)
    bool start_updated(std::map<std::string, std::string>* attributes);
    //! End LASTUPDATE
    bool end_update();
    //! Start CLIENTEROR element
    bool start_clienterror();
    //! End CLIENTERROR
    bool end_clienterror();
    //! Start COD element: data error code.
    bool start_cod();
    //! End COD
    bool end_cod();
    //! Start MESSAGE element: data error message.
    bool start_message();
    //! End MESSAGE
    bool end_message();


    //! Fetch icon
    Fl_Image* fetch_icon(std::string name);

    //! Run the fetch std::thread
    static void do_thread(wx_handler* that);
    //! The fetch std::thread
    std::thread* wx_thread_;
    //! True when data has been received from weather server
    std::atomic<bool> wx_valid_;
    //! True start fetching data from weather server.
    std::atomic<bool> wx_fetch_;


    //! Stack of elements currently being processed
    std::list<wxe_element_t> elements_;
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

#include "wx_handler.h"
#include "status.h"
#include "utils.h"
#include "url_handler.h"
#include "qso_manager.h"
#include "record.h"
#include "stn_data.h"
#include "ticker.h"

#include "nlohmann/json.hpp"

#include <sstream>

#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_PNG_Image.H>

extern status* status_;
extern url_handler* url_handler_;
extern qso_manager* qso_manager_;
extern ticker* ticker_;
extern stn_data* stn_data_;
extern bool DEBUG_THREADS;
extern bool DEBUG_QUICK;

using json = nlohmann::json;

const double MPH2MPS = 1.0 / 3600.0 * (1760.0 * 36.0) * 25.4 / 1000.0;

const double LONG_DELAY = 30. * 60. * 10.;
const double SHORT_DELAY = 3. * 60. * 10.;

std::string wx_handler::wind_cardinal(int dirn) {
    int temp = dirn * 32 / 360;
    temp += 1;
    temp %= 32;
    temp /= 2;
    string cardinals[16] =
    { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
      "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };
    return cardinals[temp];
}

std::string wx_handler::beaufort(float speed) {
    if (speed <= 0.2) return "Calm";
    if (speed <= 1.5) return "Light air";
    if (speed <= 3.3) return "Light breeze";
    if (speed <= 5.4) return "Gentle breeze";
    if (speed <= 7.9) return "Moderate breeze";
    if (speed <= 10.7) return "Fresh breeze";
    if (speed <= 13.8) return "Strong breeze";
    if (speed <= 17.1) return "Moderate gale";
    if (speed <= 20.7) return "Gale";
    if (speed <= 24.4) return "Severe gale";
    if (speed <= 28.4) return "Storm";
    if (speed <= 32.6) return "Violent storm";
    return "Hurricane";
}

// Deserialise from JSON to wx+report
static void from_json(const json& j, wx_report& s) {
    json jcoord = j.at("coord");
    jcoord.at("lon").get_to(s.city_location.longitude);
    jcoord.at("lat").get_to(s.city_location.latitude);
    auto weather = j.at("weather").get<std::vector<json>>();
    json jweather = weather[0];
    jweather.at("description").get_to(s.description);
    string icon;
    jweather.at("icon").get_to(icon);
    s.icon = wx_handler::fetch_icon(icon);
    json jmain = j.at("main");
    jmain.at("temp").get_to(s.temperature_K);
    jmain.at("feels_like").get_to(s.subjective_K);
    jmain.at("pressure").get_to(s.pressure_hPa);
    jmain.at("humidity").get_to(s.humidity_pc);
    j.at("visibility").get_to(s.visibility_m);
    json jwind = j.at("wind");
    jwind.at("speed").get_to(s.wind_speed_ms);
    jwind.at("deg").get_to(s.wind_dirn);
    json jcloud = j.at("clouds");
    jcloud.at("all").get_to(s.cloud_cover);
    j.at("dt").get_to(s.updated);
    json jsys = j.at("sys");
    jsys.at("country").get_to(s.iso_country);
    jsys.at("sunrise").get_to(s.sunrise);
    jsys.at("sunset").get_to(s.sunset);
    int tz_sec;
    j.at("timezone").get_to(tz_sec);
    s.timezone_hr = tz_sec / 3600.0F;
    j.at("id").get_to(s.city_id);
    j.at("name").get_to(s.city_name);
    s.wind_cardinal = wx_handler::wind_cardinal(s.wind_dirn);
    s.wind_name = wx_handler::beaufort(s.wind_speed_ms);
}

// Constructor
wx_handler::wx_handler() :
    wx_thread_(nullptr),
    error_code_(0)
{
    report_.icon = nullptr;
    // Start std::thread
    wx_fetch_.store(false);
    wx_valid_.store(false);
    wx_thread_ = new std::thread(do_thread, this);
    wx_fetch_.store(true);
    // Start ticker - 30 minutes
    ticker_->add_ticker(this, cb_ticker, DEBUG_QUICK ? SHORT_DELAY : LONG_DELAY);

};

// Destructor   
wx_handler::~wx_handler() {
    ticker_->remove_ticker(this);
    // Close the std::thread down cleanly
    if (wx_thread_) wx_thread_->join();
};

// Do std::thread - when told to by wx_fetch_ fetch the WX data.
// Abandon when run_thread_ is deasserted
void wx_handler::do_thread(wx_handler* that) {
    while(!that->wx_fetch_.load());
    if (DEBUG_THREADS) printf("WX THREAD: staring to fetch\n");
    that->wx_fetch_.store(false);
    that->update();
    if (DEBUG_THREADS) printf("WX THREAD: fetching complete\n");
	Fl::awake(cb_fetch_done, (void*)that);
    that->wx_valid_.store(true);
}

// Update weather report - forecd
void wx_handler::update() {
    // Create a dummy record to get own location
    record* dummy = qso_manager_->dummy_qso();
    std::string qth_id = qso_manager_->get_default(qso_manager::QTH);
    lat_long_t location = { nan(""), nan("") };
    if (qth_id.length()) {
        const qth_info_t* info = stn_data_->get_qth(qth_id);
        if (info != nullptr && info->data.find(LOCATOR) != info->data.end()) {
            dummy->item("MY_GRIDSQUARE", info->data.at(LOCATOR));
            location = dummy->location(true);
        }
    }
    if (std::isnan(location.latitude) || std::isnan(location.longitude)) {
        report_ = wx_report();
        report_.city_name = "Not known";
        return;
    }
    char url[1024];
    std::stringstream ss;
    snprintf(url, sizeof(url), "https://api.openweathermap.org/data/2.5/weather?lat=%f&lon=%f&appid=%s&mode=json",
        location.latitude,
        location.longitude,
        key_);
    if (url_handler_->read_url(std::string(url), &ss)) {
        ss.seekg(std::ios::beg);
        json j;
        ss >> j;
        j.get_to(report_);
     } else {
        report_ = wx_report();
        report_.city_name = "Not known";
    }
}

// Timer - called every 30 minutes
void wx_handler::ticker() {
    wx_valid_.store(false);
    if (DEBUG_THREADS) printf("WX MAIN: Starting WX fetch\n");
    wx_fetch_.store(true);
}

// Static
void wx_handler::cb_ticker(void* v) {
    ((wx_handler*)v)->ticker();
}

// Static call back: WX fetch complete
void wx_handler::cb_fetch_done(void* v) {
    wx_handler* that = (wx_handler*)v;
    that->wx_fetch_.store(false);
    char msg[1024];
    snprintf(msg, sizeof(msg), "WX_HANDLER: Weather read OK: %s %0.0f\302\260C %0.0fMPH %s %0.0f hPa. %0.0f%% cloud",
        that->description().c_str(), 
        that->temperature(), 
        that->wind_speed(), 
        that->wind_direction().c_str(), 
        that->pressure(), 
        that->cloud() * 100);
    status_->misc_status(ST_OK, msg);
    qso_manager_->enable_widgets();
}

// Static call back: WX fetch complete
void wx_handler::cb_fetch_error(void* v) {
    status_->misc_status(ST_ERROR, (char*)v);
    qso_manager_->enable_widgets();
}

// Get the various weather items - 
// summation icon
Fl_Image* wx_handler::icon() {
    return report_.icon;
}

// Description
std::string wx_handler::description() {
    return report_.description;
}

// Temperature (C)
float wx_handler::temperature() {
    return report_.temperature_K - 273.15;
}

// Wind-speed (MPH)
float wx_handler::wind_speed() {
    return report_.wind_speed_ms / MPH2MPS;
}

// Wind-speed name
std::string wx_handler::wind_name() {
    return report_.wind_name;
}

// Wind direction (16th cardinals)
std::string wx_handler::wind_direction() {
    if (report_.wind_cardinal == "") return "---";
    else return report_.wind_cardinal;
}

// Wind direction (degrees)
unsigned int wx_handler::wind_degrees() {
    if (report_.wind_cardinal == "") return -1;
    else return report_.wind_dirn;
}

// Cloud cover
float wx_handler::cloud() {
    return ((float)report_.cloud_cover)/ 100.0;
}

// Cloud description
std::string wx_handler::cloud_name() {
    return report_.cloud_name;
}

// Sunris
time_t wx_handler::sun_rise() {
    return report_.sunrise;
}

// Sun std::set
time_t wx_handler::sun_set() { 
    return report_.sunset;
}

// Last updated
time_t wx_handler::last_updated() {
    return report_.updated;
}

// Location
std::string wx_handler::location() {
    return report_.city_name;
}

// Latlong location
std::string wx_handler::latlong() {
    std::string result = degrees_to_dms(report_.city_location.latitude, true);
    result += " ";
    result += degrees_to_dms(report_.city_location.longitude, false);
    return result;
}

// Pressure
float wx_handler::pressure() {
    return report_.pressure_hPa;
}

// Fetch icon
Fl_Image* wx_handler::fetch_icon(std::string name) {
    char url[1024];
    snprintf(url, sizeof(url), "https://openweathermap.org/img/wn/%s.png", name.c_str());
    std::stringstream ss;
    if (url_handler_->read_url(std::string(url), &ss)) {
        ss.seekg(std::ios::beg);
        Fl_PNG_Image* result = new Fl_PNG_Image(nullptr, (unsigned char*)ss.str().c_str(), ss.str().length());
        return result;
    } else {
        Fl::awake(cb_fetch_error, (void *)"WX_HANDLER: WX icon read failed");
        return nullptr;
    }
}





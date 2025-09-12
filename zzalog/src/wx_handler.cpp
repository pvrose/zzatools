#include "wx_handler.h"
#include "status.h"
#include "utils.h"
#include "url_handler.h"
#include "qso_manager.h"
#include "record.h"
#include "stn_data.h"
#include "ticker.h"

#include <sstream>

#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_PNG_Image.H>

using namespace std;

extern status* status_;
extern url_handler* url_handler_;
extern qso_manager* qso_manager_;
extern ticker* ticker_;
extern stn_data* stn_data_;
extern bool DEBUG_THREADS;
extern bool DEBUG_QUICK;

const double MPH2MPS = 1.0 / 3600.0 * (1760.0 * 36.0) * 25.4 / 1000.0;

const double LONG_DELAY = 30. * 60. * 10.;
const double SHORT_DELAY = 3. * 60. * 10.;

// Constructor
wx_handler::wx_handler() :
    xml_reader(),
    wx_thread_(nullptr),
    error_code_(0)
{
    report_.icon = nullptr;
    elements_.clear();
    // Start thread
    wx_fetch_.store(false);
    wx_valid_.store(false);
    wx_thread_ = new thread(do_thread, this);
    wx_fetch_.store(true);
    // Start ticker - 30 minutes
    ticker_->add_ticker(this, cb_ticker, DEBUG_QUICK ? SHORT_DELAY : LONG_DELAY);

};

// Destructor   
wx_handler::~wx_handler() {
    ticker_->remove_ticker(this);
    // Close the thread down cleanly
    if (wx_thread_) wx_thread_->join();
};

// Do thread - when told to by wx_fetch_ fetch the WX data.
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

// XML reader overloads
// Overloadable XML handlers
// Start 
bool wx_handler::start_element(string name, map<string, string>* attributes) {
 	string element_name = to_upper(name);
	bool result = true;

   	// Check in order of the most comment element type - call appropriate start method
	if (element_name == "CURRENT") result = start_current()|| result;
	else if (element_name == "CITY") result = start_city(attributes) || result;
	else if (element_name == "COORD") result = start_coord(attributes) || result;
	else if (element_name == "COUNTRY") result = start_country() || result;
	else if (element_name == "TIMEZONE") result = start_timezone() || result;
	else if (element_name == "SUN") result = start_sun(attributes) || result;
	else if (element_name == "TEMPERATURE") result = start_temperature(attributes) || result;
	else if (element_name == "FEELS_LIKE") result = start_subjective(attributes) || result;
	else if (element_name == "CLOUDS") result = start_clouds(attributes) || result;
	else if (element_name == "HUMIDITY") result = start_humidity(attributes) || result;
	else if (element_name == "PRESSURE") result = start_pressure(attributes) || result;
	else if (element_name == "PRECIPITATION") result = start_precipitation(attributes) || result;
	else if (element_name == "WEATHER") result = start_weather(attributes) || result;
	else if (element_name == "WIND") result = start_wind() || result;
	else if (element_name == "SPEED") result = start_wind_speed(attributes) || result;
	else if (element_name == "DIRECTION") result = start_wind_dirn(attributes) || result;
	else if (element_name == "GUSTS") result = start_gusts(attributes) || result;
	else if (element_name == "VISIBILITY") result = start_visibility(attributes) || result;
	else if (element_name == "LASTUPDATE") result = start_updated(attributes) || result;
	else if (element_name == "CLIENTERROR") result = start_clienterror() || result;
	else if (element_name == "COD") result = start_cod() || result;
	else if (element_name == "MESSAGE") result = start_message() || result;
	else {
		char* message = new char[50 + element_name.length()];
        sprintf(message, "WX_HANDLER: Unexpected XML element %s encountered - ignored", element_name.c_str());
        Fl::awake(cb_fetch_error, message);
		result = false;
	}

	// Need to delete attributes
	if (attributes != nullptr) {
		attributes->clear();
		delete attributes;
	}
	return result;

}

// End
bool wx_handler::end_element(string name) {
    string element_name = to_upper(name);
	wxe_element_t element = elements_.back();
	elements_.pop_back();
	// Go to the specific end_... method
	switch (element) {
    case WXE_CURRENT: return end_current();
    case WXE_CITY: return end_city();
    case WXE_COORD: return end_coord();
    case WXE_COUNTRY: return end_country();
    case WXE_TIMEZONE: return end_timezone();
    case WXE_SUN: return end_sun();
    case WXE_WIND: return end_wind();
    case WXE_SPEED: return end_wind_speed();
    case WXE_DIRECTION: return end_wind_dirn();
    case WXE_GUSTS: return end_gusts();
    case WXE_TEMPERATURE: return end_temperature();
    case WXE_FEELS_LIKE: return end_subjective();
    case WXE_CLOUDS: return end_clouds();
    case WXE_VISIBILITY: return end_visibility();
    case WXE_HUMIDITY: return end_humidity();
    case WXE_PRESSURE: return end_pressure();
    case WXE_PRECIPITATION: return end_precipitation();
    case WXE_WEATHER: return end_weather();
    case WXE_LASTUPDATE: return end_update();
    case WXE_CLIENTERROR: return end_clienterror();
    case WXE_COD: return end_cod();
    case WXE_MESSAGE: return end_message();
    default: break;
    }

	char* message = new char[50 + name.length()];
	sprintf(message, "WX_HANDLER: Invalid XML - mismatch start and end of element %s", name.c_str());
    Fl::awake(cb_fetch_error, message);
    return false;
}

// Special element
bool wx_handler::declaration(xml_element::element_t element_type, string name, string content) {
	// ignored
	return true;
};

// Processing instruction
bool wx_handler::processing_instr(string name, string content) {
    return true;
};

// characters
bool wx_handler::characters(string content){
	if (elements_.size()) {
		switch (elements_.back()) {
		case WXE_COUNTRY:
        case WXE_TIMEZONE:
        case WXE_COD:
        case WXE_MESSAGE:
			element_data_ = content;
			break;
		default:
			break;
		}
	}
	return true;
};

// Update weather report - forecd
void wx_handler::update() {
    // Create a dummy record to get own location
    record* dummy = qso_manager_->dummy_qso();
    string qth_id = qso_manager_->get_default(qso_manager::QTH);
    lat_long_t location = { nan(""), nan("") };
    if (qth_id.length()) {
        const qth_info_t* info = stn_data_->get_qth(qth_id);
        if (info != nullptr) {
            dummy->item("MY_GRIDSQUARE", info->data.at(LOCATOR));
            location = dummy->location(true);
        }
    }
    if (isnan(location.latitude) || isnan(location.longitude)) {
        report_ = wx_report();
        report_.city_name = "Not known";
        return;
    }
    char url[1024];
    stringstream ss;
    snprintf(url, sizeof(url), "https://api.openweathermap.org/data/2.5/weather?lat=%f&lon=%f&appid=%s&mode=xml",
        location.latitude,
        location.longitude,
        key_);
    if (url_handler_->read_url(string(url), &ss)) {
        ss.seekg(ios::beg);
        parse(ss);
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
string wx_handler::description() {
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
string wx_handler::wind_name() {
    return report_.wind_name;
}

// Wind direction (16th cardinals)
string wx_handler::wind_direction() {
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
string wx_handler::cloud_name() {
    return report_.cloud_name;
}

// Sunris
time_t wx_handler::sun_rise() {
    return report_.sunrise;
}

// Sun set
time_t wx_handler::sun_set() { 
    return report_.sunset;
}

// Last updated
time_t wx_handler::last_updated() {
    return report_.updated;
}

// Location
string wx_handler::location() {
    return report_.city_name;
}

// Latlong location
string wx_handler::latlong() {
    string result = degrees_to_dms(report_.city_location.latitude, true);
    result += " ";
    result += degrees_to_dms(report_.city_location.longitude, false);
    return result;
}

// Pressure
float wx_handler::pressure() {
    return report_.pressure_hPa;
}

// The overall XML container CURRENT
bool wx_handler::start_current() {
    if (elements_.size()) {
        Fl::awake(cb_fetch_error, (void*)"WX_HANDLER: Incorrect XML - unexpected adif element");
        return false;
	}
	else {
		elements_.push_back(WXE_CURRENT);
        report_ = wx_report();
    	return true;
    }
}

// End the overall element CURRENT
bool wx_handler::end_current() {
    // printf("Location #%d, %s (%f, %f)\n", report_.city_id, report_.city_name.c_str(), report_.city_location.longitude, report_.city_location.latitude);
    // printf("%s %d\n", report_.iso_country.c_str(), report_.timezone_hr);
    // printf("T=%fK; H=%d%%, P=%dhPA\n", report_.temperature_K, report_.humidity_pc, report_.pressure_hPa);
    // printf("%s %d\n", report_.precipitation.c_str(), report_.precip_mm);
    // printf("Wind %fm/s, %d(%s)\n", report_.wind_speed_ms, report_.wind_dirn, report_.wind_cardinal.c_str());
    return true;
}

// Start CITY element
bool wx_handler::start_city(map<string, string>* attributes) {
    elements_.push_back(WXE_CITY);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "ID") {
            report_.city_id = stoi(it->second);
        }
        else if (att_name == "NAME") {
            report_.city_name = it->second;
        }
    }
    return true;
}

// End CITY element
bool wx_handler::end_city() {
    return true;
}

// City coordinates - start COORD element
bool wx_handler::start_coord(map<string, string>* attributes) {
    elements_.push_back(WXE_COORD);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        double value = stod(it->second);
        if (att_name == "LON") {
            report_.city_location.longitude = value;
        }
        else if (att_name == "LAT") {
            report_.city_location.latitude = value;
        }
    }
    return true;
  
}

// End COORD element
bool wx_handler::end_coord() {
    return true;
}

// Country - start COUNTRY element (this won't be DXCC country)
bool wx_handler::start_country() {
    elements_.push_back(WXE_COUNTRY);
    return true;
}

// End COUNTRY element
bool wx_handler::end_country() {
    report_.iso_country = element_data_;
    return true;
}

// Start TIMEZONE element
bool wx_handler::start_timezone() {
    elements_.push_back(WXE_TIMEZONE);
    return true;
}

// End TIMEZONE element
bool wx_handler::end_timezone() {
    float tz = stof(element_data_) / 3600.0;
    report_.timezone_hr = tz;
    return true;
}

// Start SUN element - sunrise and sunset times
bool wx_handler::start_sun(map<string, string>* attributes) {
    elements_.push_back(WXE_SUN);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        time_t value = convert_xml_datetime(it->second);
        if (att_name == "RISE") {
            report_.sunrise = value;
        }
        else if (att_name == "SET") {
            report_.sunset = value;
        }
    }
    return true;
}

// End SUN element
bool wx_handler::end_sun() {
    return true;
}

// Start TEMPERATURE element
bool wx_handler::start_temperature(map<string, string>* attributes) {
    elements_.push_back(WXE_TEMPERATURE);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            float value = stof(it->second);
            report_.temperature_K = value;
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        }
    }
    return true;
}

// End temperature - adjust reading for unit
bool wx_handler::end_temperature() {
    if (unit_ == "celsius") {
        report_.temperature_K += 273.15;
    } else if (unit_ == "fahrenheit") {
        float t = report_.temperature_K;
        t -= 32.0;
        t *= 5.0 / 9.0;
        t += 273.15;
        report_.temperature_K = t;
    }
    return true;
}

// Start SUBJECTIVE element - 
bool wx_handler::start_subjective(map<string, string>* attributes) {
    elements_.push_back(WXE_FEELS_LIKE);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            float value = stof(it->second);
            report_.subjective_K = value;
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        }
    }
    return true;
}

// End SUBJECTIVE element
bool wx_handler::end_subjective() {
    if (unit_ == "celsius") {
        report_.subjective_K += 273.15;
    } else if (unit_ == "fahrenheit") {
        float t = report_.subjective_K;
        t -= 32.0;
        t *= 5.0 / 9.0;
        t += 273.15;
        report_.subjective_K = t;
    }
    return true;
}

// Start HUMIDITY element
bool wx_handler::start_humidity(map<string, string>* attributes) {
    elements_.push_back(WXE_HUMIDITY);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
           int value = stoi(it->second);
           report_.humidity_pc = value;
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        }
    }
    return true;
}

// End humidity element
bool wx_handler::end_humidity() {
    // TODO are any other units other than % expected?
    return true;
}

// Start PRESSURE element
bool wx_handler::start_pressure(map<string, string>* attributes) {
    elements_.push_back(WXE_PRESSURE);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            int value = stoi(it->second);
            report_.pressure_hPa = value;
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        }
    }
    return true;
 
}

// End PRESSURE element
bool wx_handler::end_pressure() {
    // TDOD Are any other units that hectopascal expected
    return true;
}

// Start WIND element
bool wx_handler::start_wind() {
    elements_.push_back(WXE_WIND);
    return true;
}

// End WIND element
bool wx_handler::end_wind() {
    return true;
}

// Start SPEED elements - for wind speed
bool wx_handler::start_wind_speed(map<string, string>* attributes) {
    elements_.push_back(WXE_SPEED);
    float value = 0.0F;
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            value = stof(it->second);
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        } else if (att_name == "NAME") {
            report_.wind_name = it->second;
        }
    }
    if (unit_ == "mph") {
        value *= MPH2MPS;
    } // default is m/s
    report_.wind_speed_ms = value;
    return true;
}

// End SPEED element
bool wx_handler::end_wind_speed() {
    return true;
}

// Start DIRECTION element - for wind direction
bool wx_handler::start_wind_dirn(map<string, string>* attributes) {
       elements_.push_back(WXE_DIRECTION);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            int value = stoi(it->second);
            report_.wind_dirn = value;
        } else if (att_name == "CODE") {
            report_.wind_cardinal = it->second;
        }
    }
    return true;
}

// End DIRECTION element
bool wx_handler::end_wind_dirn() {
    return true;
}

// Start GUSTS element - with gusts up to...
bool wx_handler::start_gusts(map<string, string>* attributes) {
      elements_.push_back(WXE_GUSTS);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            float value = stof(it->second);
            report_.gusting_ms = value;
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        }
    }
    return true;
  
}

// End GUSTS element
bool wx_handler::end_gusts() {
      float f = report_.gusting_ms;
    if (unit_ == "mph") {
        f *= 0.44704;
    } // default is m/s
    report_.gusting_ms = f;
    return true;
}

// Start CLOUDS element - for cloud cover
bool wx_handler::start_clouds(map<string, string>* attributes) {
    elements_.push_back(WXE_CLOUDS);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            int value = stoi(it->second);
            report_.cloud_cover = value;
        } else if (att_name == "NAME") {
            report_.cloud_name = it->second;
        }
    }
    return true;
}

// End CLOUDS element
bool wx_handler::end_clouds() {
    return true;
}

// Start VISIBILITY eleemnt
bool wx_handler::start_visibility(map<string, string>* attributes) {
    elements_.push_back(WXE_VISIBILITY);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            int value = stoi(it->second);
            report_.visibility_m = value;
        }
    }
    return true;
}

// End VISIBILITY element
bool wx_handler::end_visibility() {
    return true;
}

// Start PRECIPITATION element
bool wx_handler::start_precipitation(map<string, string>* attributes) {
    elements_.push_back(WXE_PRECIPITATION);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            int value = stof(it->second);
            report_.precip_mm = value;
        } else if (att_name == "MODE") {
            report_.precipitation = it->second;
        } else if (att_name == "UNIT") {
            unit_ = it->second;
        }
    }
    return true;
}

// End PRECIPITATION element
bool wx_handler::end_precipitation() {
    // TODO any unit other than 1h (last hour's rain)
    return true;
}

// Start WEATHER element - overall description and icon reference
bool wx_handler::start_weather(map<string, string>* attributes) {
    elements_.push_back(WXE_WEATHER);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            report_.description = it->second;
        } else if (att_name == "ICON") {
            report_.icon = fetch_icon(it->second);
        }
    }
    return true;
}

// End WEATHER element
bool wx_handler::end_weather() {
    return true;
}

// Start LASTUPDATE element
bool wx_handler::start_updated(map<string, string>* attributes) {
    elements_.push_back(WXE_LASTUPDATE);
    for (auto it = attributes->begin(); it != attributes->end(); it++) {
        string att_name = to_upper(it->first);
        if (att_name == "VALUE") {
            time_t value = convert_xml_datetime(it->second);
            report_.updated = value;
        }
    }
    return true;
}

// End LASTUPDATE elememnt
bool wx_handler::end_update() {
    return true;
}

// Start CLIENTERROR element
bool wx_handler::start_clienterror() {
    elements_.push_back(WXE_CLIENTERROR);
    return true;
}

// End CLIENTERROR element
bool wx_handler::end_clienterror() {
    char msg[128];
    snprintf(msg, sizeof(msg), "WX_HANDLER: Rejected: %d (%s)", error_code_, error_message_.c_str());
    Fl::awake(cb_fetch_error, msg);
    return true;
}

// Start COD (error code) element
bool wx_handler::start_cod() {
    elements_.push_back(WXE_COD);
    return true;
}

// End COD element
bool wx_handler::end_cod() {
    error_code_ = stoi(element_data_);
    return true;
}

// Start MESSAGE (Error message) eleemnt
bool wx_handler::start_message() {
    elements_.push_back(WXE_MESSAGE);
    return true;
}

// End MESSAGE element
bool wx_handler::end_message() {
    error_message_ = element_data_;
    return true;
}

// Fetch icon
Fl_Image* wx_handler::fetch_icon(string name) {
    char url[1024];
    snprintf(url, sizeof(url), "https://openweathermap.org/img/wn/%s.png", name.c_str());
    stringstream ss;
    if (url_handler_->read_url(string(url), &ss)) {
        ss.seekg(ios::beg);
        Fl_PNG_Image* result = new Fl_PNG_Image(nullptr, (unsigned char*)ss.str().c_str(), ss.str().length());
        return result;
    } else {
        Fl::awake(cb_fetch_error, (void *)"WX_HANDLER: WX icon read failed");
        return nullptr;
    }
}





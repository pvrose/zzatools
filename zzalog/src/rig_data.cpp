#include "rig_data.h"
#include "rig_if.h"
#include "status.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>
#include <set>


using json = nlohmann::json;

extern status* status_;
extern std::string default_data_directory_;

// power_mode_t serialisation
NLOHMANN_JSON_SERIALIZE_ENUM(power_mode_t, {
    { NO_POWER, "Unsupported"},
    { RF_METER, "RF Output meter"},
    { DRIVE_LEVEL, "Drive meter"},
    { MAX_POWER, "Speciified power"}
    }
)

NLOHMANN_JSON_SERIALIZE_ENUM(freq_mode_t, {
    { NO_FREQ, "Unsupported"},
    { VFO, "VFO"},
    { XTAL, "Fixed"}
    }
)

//! Convert cat_data_t to JSON structure
static void to_json(json& j, const cat_data_t& s) {
    json jh = json{
        { "Rig model", s.hamlib->model },
        { "Manufacturer", s.hamlib->mfr },
        { "Port", s.hamlib->port_name },
        { "Baud rate", s.hamlib->baud_rate },
        { "Model ID", s.hamlib->model_id },
        { "Port type", s.hamlib->port_type }
    };
    json je = json{
        { "Hamlib data", jh},
        { "Timeout", s.hamlib->timeout },
        { "Maximum timeout count", s.hamlib->max_to_count },
        { "S-meter hold count", s.hamlib->num_smeters },
        { "Power mode", s.hamlib->power_mode },
        { "Maximum power", s.hamlib->max_power },
        { "Frequency mode", s.hamlib->freq_mode},
        { "Override hamlib", s.override_hamlib },
        { "Start automatically", s.auto_start },
        { "Connect automatically", s.auto_connect }
    };
    if (s.hamlib->accessory & AMPLIFIER) {
        json jamp = json{
            { "Gain", s.hamlib->gain }
        };
        je["Amplifier"] = jamp;
    }
    if (s.hamlib->accessory & TRANSVERTER) {
        json jtvr = json{
            { "Offset", s.hamlib->freq_offset },
            { "Power", s.hamlib->tvtr_power }
        };
        je["Transverter"] = jtvr;
    }
    // Conditional settings
    if (s.use_cat_app) je["Command"] = s.app;
    if (s.hamlib->freq_mode == XTAL) je["Fixed Frequency"] = s.hamlib->frequency;
    if (s.auto_connect) je["Connect delay"] = s.connect_delay;
    j[s.nickname] = je;
}
//! Convert JSON structure to cat_data_t
static void from_json(const json& j, cat_data_t& s) {
    s.hamlib = new hamlib_data_t;
    // Hamlib data
    json jh = j.at("Hamlib data");
    jh.at("Rig model").get_to(s.hamlib->model);
    jh.at("Port").get_to(s.hamlib->port_name);
    jh.at("Model ID").get_to(s.hamlib->model_id);
    jh.at("Manufacturer").get_to(s.hamlib->mfr);
    jh.at("Baud rate").get_to(s.hamlib->baud_rate);
    jh.at("Port type").get_to(s.hamlib->port_type);
    // Other data
    j.at("Timeout").get_to(s.hamlib->timeout);
    j.at("Maximum timeout count").get_to(s.hamlib->max_to_count);
    j.at("S-meter hold count").get_to(s.hamlib->num_smeters);
    j.at("Power mode").get_to(s.hamlib->power_mode);
    j.at("Maximum power").get_to(s.hamlib->max_power);
    j.at("Frequency mode").get_to(s.hamlib->freq_mode);
    j.at("Override hamlib").get_to(s.override_hamlib);
    j.at("Start automatically").get_to(s.auto_start);
    j.at("Connect automatically").get_to(s.auto_connect);
    // Accessories
    s.hamlib->accessory = BAREBACK;
    if (j.find("Amplifier") != j.end()) {
        (uchar&)s.hamlib->accessory |= (uchar)AMPLIFIER;
        j.at("Ammplifier").at("Gain").get_to(s.hamlib->gain);
    }
    if (j.find("Transverter") != j.end()) {
        (uchar&)s.hamlib->accessory |= (uchar)TRANSVERTER;
        j.at("Transverter").at("Offset").get_to(s.hamlib->freq_offset);
        j.at("Transverter").at("Power").get_to(s.hamlib->tvtr_power);
    }
    // Conditional - command
    if (j.find("Command") != j.end()) {
        s.use_cat_app = true;
        j.at("Command").get_to(s.app);
    }
    else {
        s.use_cat_app = false;
    }
    // Conditional
    if (s.hamlib->freq_mode == XTAL) {
        j.at("Fixed frequency").get_to(s.hamlib->frequency);
    }
    if (s.auto_connect) {
        j.at("Connect delay").get_to(s.connect_delay);
    }
}

//! Convert rig_data_t to JSON structure
static void to_json(json& j, const rig_data_t& s) {
    j = json{
        { "Antenna", s.antenna},
        { "Use instantaneous", s.use_instant_values},
    };
    if (s.default_app >= 0) {
        j["Default CAT"] = s.cat_data[s.default_app]->nickname;
    }
    if (s.cat_data.size()) {
        json jcat;
        for (auto& it : s.cat_data) {
            jcat.push_back(*it);
        }
        j["CATs"] = jcat;
    }
}

//! Convert JSON structure to rig_data_t
static void from_json(const json& j, rig_data_t& s) {
    j.at("Antenna").get_to(s.antenna);
    j.at("Use instantaneous").get_to(s.use_instant_values);
    // Default APP is kept as the index into cat_data: default = -1. In JSON it's the nickname
    string default_cat = "";
    s.default_app = -1;
    if (j.find("Default CAT") != j.end()) {
        j.at("Default CAT").get_to(default_cat);
    }
    // If we have CAT data defined in the JSON then create the cat_data_t
    if (j.find("CATs") != j.end()) {
        auto temp =
            j.at("CATs").get<std::vector<std::map<std::string, json>>>();
        for (auto& it : temp) {
            for (auto& ita : it) {
                cat_data_t* cd =
                    new cat_data_t(ita.second.template get<cat_data_t>());
                cd->nickname = ita.first;
                s.cat_data.push_back(cd);
                if (cd->nickname == default_cat)
                    s.default_app = s.cat_data.size() - 1;
            }
        }
    }
}

rig_data::rig_data() {
    load_failed_ = false;
    load_data();
}

rig_data::~rig_data() {
    store_json();
}

// Get the cat data
cat_data_t* rig_data::cat_data(std::string rig, int app) {
    rig_data_t* data = get_rig(rig);
    if (data == nullptr) {
        return nullptr;
    }
    else {
        if (app < 0) {
            if (data->default_app < 0) {
                return nullptr;
            }
            else {
                return data->cat_data.at(data->default_app);
            }
        }
        else if (app >= data->cat_data.size()) {
            return nullptr;
        }
        else {
            return data->cat_data.at(app);
        }
    }
}

// Get the supported rigs
std::vector<std::string> rig_data::rigs() {
    std::vector<std::string> result;
    result.clear();
    for (auto it : data_) {
        result.push_back(it.first);
    }
    return result;
}

// Get data for t he particular rig
rig_data_t* rig_data::get_rig(std::string rig) {
    if (data_.find(rig) == data_.end()) {
        data_[rig] = new rig_data_t;
    }
    return data_.at(rig);
}

void rig_data::load_data() {
    load_failed_ = true;
    status_->misc_status(ST_NOTE, "RIG DATA: Loading rig configuration data");
    if (load_json()) {
        load_failed_ = false;
        return;
    }
}

// Loading datafrom JSON
bool rig_data::load_json() {
    char msg[128];
    std::string filename = default_data_directory_ + "rigs.json";
    ifstream is;
    is.open(filename, std::ios_base::in);
    if (!is.good()) {
        snprintf(msg, sizeof(msg), "RIGS: Failed to open %s", filename.c_str());
        status_->misc_status(ST_WARNING, msg);
        return false;
    } 
    json jall;
    try {
        is >> jall;
        auto temp = jall.at("Rigs").get<std::vector<std::map<std::string, json>>>();
        for (auto& it : temp) {
            for (auto& ita : it) {
                rig_data_t* rd = new rig_data_t(ita.second.template get<rig_data_t>());
                data_[ita.first] = rd;
            }
        }
    }
    catch (const json::exception& e) {
        std::snprintf(msg, sizeof(msg), "RIG DATA: Failed to load %s: %d (%s)\n",
            filename.c_str(), e.id, e.what());
        status_->misc_status(ST_ERROR, msg);
        is.close();
        return false;
    }
    snprintf(msg, sizeof(msg), "RIG DATA: File %s loaded OK", filename.c_str());
    status_->misc_status(ST_OK, msg);
    is.close();
    return true;
}

// Store data as JSON
bool rig_data::store_json() {
    std::string filename = default_data_directory_ + "rigs.json";
    std::ofstream os;
    os.open(filename, std::ios_base::out);
    if (os.good()) {
        json j;
        for (auto& it : data_) {
            if (it.first.length()) {
                json jr;
                jr[it.first] = *it.second;
                j["Rigs"].push_back(jr);
            }
        }
        os << std::setw(2) << j << endl;
    }
    if (os.fail()) {
        status_->misc_status(ST_WARNING, "RIGS: failed to save data as JSON");
        os.close();
        return false;
    }
    else {
        status_->misc_status(ST_OK, "RIGS: Saved OK");
        os.close();
        return true;
    }

}


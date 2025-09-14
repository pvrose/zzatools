#include "rig_writer.h"
#include "rig_reader.h"
#include "rig_data.h"
#include "rig_if.h"
#include "status.h"



extern status* status_;
extern std::string PROGRAM_VERSION;

rig_writer::rig_writer() {
    rig_name_ = "";
    rig_data_ = nullptr;
    app_name_ = "";
    app_data_ = nullptr;
}

rig_writer::~rig_writer() {

}

bool rig_writer::store_data(std::map<std::string, rig_data_t*>* all_data, std::ostream& os) {
    data_ = all_data;
    status_->misc_status(ST_NOTE, "RIG DATA: Starting XML generation");

    if (!write_element(RIG_NONE)) {
        status_->misc_status(ST_ERROR, "RIG DATA: XML generation failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "RIG DATA: XML Generation succeeded, now writing");
    if (!data(os)) {
        status_->misc_status(ST_ERROR, "RIG DATA: XML Writing failed");
        return false;
    }
    // else
    status_->misc_status(ST_OK, "RIG DATA: XML Saved OK");
    return true;
}

bool rig_writer::write_element(rigs_element_t element) {
    std::string name;
    std::string data;
    std::map<std::string, std::string>* attributes;
    hamlib_data_t* hamlib;
    switch(element) {
    case RIG_NONE:
        // Prolog
        name = "xml";
        data = "version=\"1.0\" encoding=\"utf-8\" ";
        if (!process_instr(name, data)) return false;
        // else
        if (!write_element(RIG_RIGS)) return false;
        return true;
    case RIG_RIGS:
        name = "rigs";
        attributes = new std::map<std::string, std::string>;
        (*attributes)["version"] = PROGRAM_VERSION;
        if (!start_element(name, attributes)) return false;
        // else
        for (auto it : *data_) {
            rig_name_ = it.first;
            rig_data_ = it.second;
            if (!write_element(RIG_RIG)) return false;
        } 
        if (!end_element(name)) return false;
        return true;
    case RIG_RIG:
        name = "rig";
        attributes = new std::map<std::string, std::string>;
        (*attributes)["name"] = rig_name_;
        if (!start_element(name, attributes)) return false;
        // else
        if (!write_value("Default App", rig_data_->default_app)) return false;;
        if (!write_value("Antenna", rig_data_->antenna)) return false;
        if (!write_value("Instantaneous Values", (int)rig_data_->use_instant_values)) return false;
        for (auto it : rig_data_->cat_data) {
            app_name_ = it->nickname;
            app_data_ = it;
            if (!write_element(RIG_APP)) return false;
        }
        if (!end_element(name)) return false;
        return true;
    case RIG_APP:
        name = "app";
        attributes = new std::map<std::string, std::string>;
        (*attributes)["name"] = app_name_;
        if (!start_element(name, attributes)) return false;
        // else
        hamlib = app_data_->hamlib;
        if (!write_value("Rig Model", hamlib->model)) return false;
        if (!write_value("Manufacturer", hamlib->mfr)) return false;
        if (!write_value("Port", hamlib->port_name)) return false;
        if (!write_value("Baud Rate", hamlib->baud_rate)) return false;
        if (!write_value("Model ID", (int)hamlib->model_id)) return false;
        if (!write_value("Timeout", hamlib->timeout)) return false;
        if (!write_value("Maximum Timeouts", hamlib->max_to_count)) return false;
        if (!write_value("S-meter Hold", hamlib->num_smeters)) return false;
        if (app_data_->use_cat_app) {
            if(!write_value("Command", app_data_->app)) return false;
        }
        if (!write_value("Override Hamlib", (int)app_data_->override_hamlib)) return false;
        if (!write_value("Power Mode", (int)hamlib->power_mode)) return false;
        if (!write_value("Maximum Power", hamlib->max_power)) return false;
        if (!write_value("Frequency Mode", hamlib->freq_mode)) return false;
        if (!write_value("Crystal Frequency", hamlib->frequency)) return false;
        if (!write_value("Amplifier Gain", hamlib->gain)) return false;
        if (!write_value("Transverter Offset", hamlib->freq_offset)) return false;
        if (!write_value("Transverter Power", hamlib->tvtr_power)) return false;
        if (!write_value("Accessories", (int)hamlib->accessory)) return false;
        if (!write_value("Start Automatically", (int)app_data_->auto_start)) return false;
        if (!write_value("Connect Automatically", (int)app_data_->auto_connect)) return false;
        if (!write_value("Connect Delay", app_data_->connect_delay)) return false;
        if (!end_element(name)) return false;
        return true;
    default:
        status_->misc_status(ST_ERROR, "RIG DATA: Unsupported XML element");
        return false;
    }
}

bool rig_writer::write_value(std::string name, std::string data) {
    std::map<std::string, std::string>* attributes = new std::map<std::string, std::string>;
    (*attributes)["name"] = name;
    if (!start_element("value", attributes)) return false;
    if (!characters(data)) return false;
    if (!end_element("value")) return false;
    return true;
}

bool rig_writer::write_value(std::string name, int data) {
    return write_value(name, to_string(data));
}

bool rig_writer::write_value(std::string name, double data) {
    char text[32];
    snprintf(text, sizeof(text), "%g", data);
    return write_value(name, std::string(text));
}
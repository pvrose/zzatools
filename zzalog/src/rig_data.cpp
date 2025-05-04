#include "rig_data.h"
#include "rig_reader.h"
#include "rig_writer.h"
#include "rig_if.h"
#include "status.h"

#include <string>
#include <vector>
#include <set>


using namespace std;

extern string VENDOR;
extern string PROGRAM_ID;
extern status* status_;

rig_data::rig_data() {
    load_data();
}

rig_data::~rig_data() {
    store_data();
}

// Get the cat data
cat_data_t* rig_data::cat_data(string rig, int app) {
    rig_data_t* data = get_rig(rig);
    if (data == nullptr) {
        return nullptr;
    } else {
        if (app < 0) {
            if (data->default_app < 0) {
                return nullptr;
            } else {
                return data->cat_data.at(data->default_app);     
            }    
        } else if (app >= data->cat_data.size()) {
            return nullptr;
        } else {
            return data->cat_data.at(app);
        }
    }
}

// Get the supported rigs
vector<string> rig_data::rigs() {
    vector<string> result;
    result.clear();
    for (auto it : data_) {
        result.push_back(it.first);
    }
    return result;
}

// Get data for t he particular rig
rig_data_t* rig_data::get_rig(string rig) {
    if (data_.find(rig) == data_.end()) {
        data_[rig] = new rig_data_t;
    }
    return data_.at(rig);
}

void rig_data::load_data() {
    Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
    if (!load_xml(settings)) {
        load_prefs(settings);
    }
}

bool rig_data::store_data() {
    Fl_Preferences settings(Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
    settings.delete_group("CAT");
    return store_xml(settings);
}

void rig_data::load_prefs(Fl_Preferences& settings) {
    set<string> supported_apps;
    Fl_Preferences cat_prefs(settings, "CAT");
    int groups = cat_prefs.groups();
    for (int g = 0; g < groups; g++) {
        Fl_Preferences rig_prefs(cat_prefs, cat_prefs.group(g));
        string rig(cat_prefs.group(g));
        rig_data_t* rd = new rig_data_t;
        data_[rig] = rd;
        char* stemp;
        int itemp;
        rig_prefs.get("Antenna", stemp, "");
        rd->antenna = stemp;
        free(stemp);
        rig_prefs.get("Instantaneous Values", itemp, (int)false);
        rd->use_instant_values = (bool)itemp;
        rig_prefs.get("Default CAT", itemp, 0);
        rd->default_app = itemp;
        int cats = rig_prefs.groups();
        for (int catg = 0; catg < cats; catg++) {
            Fl_Preferences app_prefs(rig_prefs, rig_prefs.group(catg));
            string app(rig_prefs.group(catg));
            supported_apps.insert(app);
            cat_data_t* ct = new cat_data_t;
            // Get hamlib & rig_if data - stored in ct->hamlib
            ct->hamlib = new hamlib_data_t;
            rd->cat_data.push_back(ct);
            ct->nickname = app;
            app_prefs.get("Rig Model", stemp, "dummy");
            ct->hamlib->model = stemp;
            free(stemp);
            app_prefs.get("Manufacturer", stemp, "hamlib");
            ct->hamlib->mfr = stemp;
            free(stemp);
            app_prefs.get("Port", stemp, "");
            ct->hamlib->port_name = stemp;
            free(stemp);
            app_prefs.get("Baud Rate", ct->hamlib->baud_rate, 9600);
            app_prefs.get("Model ID", itemp, -1);
            ct->hamlib->model_id = itemp;
            app_prefs.get("Timeout", ct->hamlib->timeout, 1.0);
            app_prefs.get("Maximum Timeout", ct->hamlib->max_to_count, 5);
            app_prefs.get("S-meter Hold", ct->hamlib->num_smeters, 5);
            // Check that hamlib is currently OK
            const rig_caps* capabilities = rig_get_caps(ct->hamlib->model_id);
            if (capabilities == nullptr) {
                char msg[128];
                snprintf(msg, sizeof(msg), "RIG: No CAT details for %s", rig.c_str());
                status_->misc_status(ST_WARNING, msg);
            }
            else {
                if (capabilities->model_name != ct->hamlib->model ||
                    capabilities->mfg_name != ct->hamlib->mfr) {
                    char msg[128];
                    snprintf(msg, 128, "RIG: Saved model id %d (%s/%s) does not match supplied rig model %s/%s using hamlib values",
                        ct->hamlib->model_id,
                        capabilities->mfg_name,
                        capabilities->model_name,
                        ct->hamlib->mfr.c_str(),
                        ct->hamlib->model.c_str());
                    status_->misc_status(ST_WARNING, msg);
                    ct->hamlib->model = capabilities->model_name;
                    ct->hamlib->mfr = capabilities->mfg_name;
                }
                ct->hamlib->port_type = capabilities->port_type;
            }
            if (app_prefs.get("Command", stemp, "")) {
                ct->use_cat_app = true;
            }
            ct->app = stemp;
            app_prefs.get("Power Mode", itemp, (int)RF_METER);
            ct->hamlib->power_mode = (power_mode_t)itemp;
            app_prefs.get("Maximum Power", ct->hamlib->max_power, 0.);
            app_prefs.get("Frequency Mode", itemp, VFO);
            ct->hamlib->freq_mode = (freq_mode_t)itemp;
            app_prefs.get("Crystal Frequency", ct->hamlib->frequency, 0.0);
            app_prefs.get("Amplifier Gain", ct->hamlib->gain, 0);
            app_prefs.get("Transverter Offset", ct->hamlib->freq_offset, 0.0);
            app_prefs.get("Transverter Power", ct->hamlib->tvtr_power, 0.0);
            app_prefs.get("Accessories", itemp, (int)BAREBACK);
            ct->hamlib->accessory = (accessory_t)itemp;
            app_prefs.get("Start Automatically", itemp, false);
            ct->auto_start = (bool)itemp;
            app_prefs.get("Connect Automatically", itemp, true);
            ct->auto_connect = (bool)itemp;
            app_prefs.get("Connect Delay", ct->connect_delay, 5.0);
            free(stemp);
        }
    }
}

bool rig_data::load_xml(Fl_Preferences& setting) {
    char buff[128];
    Fl_Preferences::Root root = setting.filename(buff, sizeof(buff),
        Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
    if (root == Fl_Preferences::USER_L) {
        string filename = directory(buff) + "/" + PROGRAM_ID + "/rigs.xml";
        ifstream is;
        is.open(filename, ios_base::in);
        if (is.good()) {
            rig_reader* reader = new rig_reader();
            if(reader->load_data(&data_, is)) {
                status_->misc_status(ST_OK, "RIG DATA: XML loaded OK");
                return true;
            }
        }
    }
    // else
    status_->misc_status(ST_WARNING, "RIG DATA: XML data faile to load - defaulting to prefernces");
    return false;
}

bool rig_data::store_xml(Fl_Preferences& setting) {
    char buff[128];
    Fl_Preferences::Root root = setting.filename(buff, sizeof(buff),
        Fl_Preferences::USER_L, VENDOR.c_str(), PROGRAM_ID.c_str());
    if (root == Fl_Preferences::USER_L) {
        string filename = directory(buff) + "/" + PROGRAM_ID + "/rigs.xml";
        ofstream os;
        os.open(filename, ios_base::out);
        if (os.good()) {
            rig_writer* writer = new rig_writer();
            if (!writer->store_data(&data_, os)) {
                status_->misc_status(ST_OK, "RIG DATA: Saved XML OK");
                return true;
            }
        }
    }
    return false;
}

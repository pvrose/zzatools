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
extern string default_data_directory_;
extern Fl_Preferences::Root prefs_mode_;

rig_data::rig_data() {
    load_failed_ = false;
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
    string filename = default_data_directory_ + "rigs.xml";
    load_failed_ = false;
    ifstream is;
    is.open(filename, ios_base::in);
    if (is.good()) {
        rig_reader* reader = new rig_reader();
        if(reader->load_data(&data_, is)) {
            status_->misc_status(ST_OK, "RIG DATA: XML loaded OK");
            return;
        } 
    }
    status_->misc_status(ST_WARNING, "RIG DATA: XML data failed to load");
    load_failed_ = true;
}

bool rig_data::store_data() {
    string filename = default_data_directory_ + "rigs.xml";
    ofstream os;
    os.open(filename, ios_base::out);
    if (os.good()) {
        rig_writer* writer = new rig_writer();
        if (!writer->store_data(&data_, os)) {
            status_->misc_status(ST_OK, "RIG DATA: Saved XML OK");
            return true;
        }
    }
    return false;
}


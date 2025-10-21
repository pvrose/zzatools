#include "fields.h"

#include "file_holder.h"
#include "main.h"
#include "utils.h"
#include "status.h"

#include <fstream>
#include <iostream>

#include <FL/fl_utf8.h>

//! Convert field_info_t to JSON object
static void to_json(json& j, const field_info_t& s) {
    j = json{
        { "Field", s.field },
        { "Width", s.width },
        { "Header", s.header }
    };
}

//! Convert JSON object to field_info_t
static void from_json(const json& j, field_info_t& s) {
    j.at("Field").get_to(s.field);
    j.at("Width").get_to(s.width);
    j.at("Header").get_to(s.header);
}

// Constructor
fields::fields() {
    filename_ = "";
    load_data();
} 
// Destructor
fields::~fields() {
    store_data();
}

// Get the collection for the application
collection_t* fields::collection(field_app_t app) {
    if (app_map_.find(app) != app_map_.end()) {
        std::string coll = app_map_.at(app);
        return collection(coll);
    } else {
        app_map_[app] = "Default";
        return collection("Default");
    }
}

// Get the collection named.. and if necessary copy the collection
collection_t* fields::collection(std::string name, std::string source, bool* copied) {
    if (coll_map_.find(name) != coll_map_.end()) {
        // Return the named collection
        if (copied) *copied = false;
        return coll_map_.at(name);
    } else {
        // Create named collection by copying from source and return it
        if (copied) *copied = true;
        if (coll_map_.find(source) != coll_map_.end()) {
            collection_t* coll = new collection_t(*coll_map_.at(source));
            coll_map_[name] = coll;
            return coll_map_.at(name);
        } else {
            // Unable to copy the source collection - return nullptr
            return nullptr;
        }
    }
}

// Get the collection named .. and if necessary pre-populate it
collection_t* fields::collection(std::string name, field_list values, bool update) {
    if (coll_map_.find(name) != coll_map_.end() && !update) {
        // Return the named collection
        return coll_map_.at(name);
    } else {
        if (update) {
            delete coll_map_.at(name);
        }
        collection_t* coll = new collection_t;
        coll->clear();
        for (auto it = values.begin(); it != values.end(); it++) {
            // Create field info with header same as field name and width 50
            field_info_t info ( (*it), (*it), 50 );
            coll->push_back(info);
        }
        coll_map_[name] = coll;
        return coll_map_.at(name);
    }
}

// Get the field names in the collection
field_list fields::field_names(std::string name) {
    field_list result;
    result.clear();
    collection_t* coll = collection(name);
    for (auto it = coll->begin(); it != coll->end(); it++) {
        result.push_back((*it).field);
    }
    return result;
}

// Read settings
void fields::load_data() {
	// Delete the existing field data
	for (auto it = app_map_.begin(); it != app_map_.end(); it++) {
        if (coll_map_.find(it->second) != coll_map_.end()) {
            delete coll_map_.at(it->second);
        }
    }
	coll_map_.clear();
    status_->misc_status(ST_NOTE, "FIELDS: Loading fields displayed");
    if (!load_collections()) {
        // Read all the field field sets
    }
    if (coll_map_.find("Default") == coll_map_.end()) {
        // Create the default collection
        collection_t* coll = new collection_t;
        // For all fields in default field std::set add it to the new field std::set
        for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
            coll->push_back(DEFAULT_FIELDS[i]);
        }
        coll_map_["Default"] = coll;
    }
}

// Read - <Prefs path>.fields.tsv
bool fields::load_collections() {
    ifstream ip;
    file_holder_->get_file(FILE_FIELDS, ip, filename_);
    char msg[128];
    if (!ip.good()) {
        char msg[128];
        snprintf(msg, sizeof(msg), 
            "FIELDS: Failed to open %s", 
            filename_.c_str());
        status_->misc_status(ST_WARNING, msg);
        return false;
    }

    json jall;
    try {
        ip >> jall;
        auto temp = jall.at("Fields").get<std::vector<std::map<std::string, json>>>();
        for (auto& it : temp) {
            for (auto& ita : it) {
                collection_t* coll = 
                    new collection_t(ita.second.template get<collection_t>());
                coll_map_[ita.first] = coll;
            }
        }
        snprintf(msg, sizeof(msg), "FIELDS: File %s loaded OK", filename_.c_str());
        status_->misc_status(ST_OK, msg);
    }
    catch (const json::exception& e) {
        char msg[128];
        snprintf(msg, sizeof(msg), "FIELDS: Failed to load %s: %d (%s)\n",
            filename_.c_str(), e.id, e.what());
        status_->misc_status(ST_ERROR, msg);
        ip.close();
        return false;
    }
    ip.close();
    return true;
}

// Store settings
void fields::store_data() {
	// Delete current settings
    std::ofstream op;
    file_holder_->get_file(FILE_FIELDS, op, filename_);
    if (op.good()) {
        json j;
        for (auto it : coll_map_) {
            json jc;
            jc[it.first] = *it.second;
            j["Fields"].push_back(jc);
        }
        op << std::setw(2) << j << '\n';
        if (op.fail()) {
            status_->misc_status(ST_ERROR, "FIELDS: Failed to save data");
        }
        else {
            status_->misc_status(ST_OK, "FIELDS: Saved OK");
        }
    }
    op.close();
}

// Save the data as JSON


// Get the std::list of collection names
std::set<std::string> fields::coll_names() {
    std::set<std::string> result;
    result.clear();
    for (auto it = coll_map_.begin(); it != coll_map_.end(); it++) {
        result.insert((*it).first);
    }
    return result;
}

// Get the collection name for the app
std::string fields::coll_name(field_app_t app) {
    if (app_map_.find(app) == app_map_.end()) {
        return "";
    } else {
        return app_map_.at(app);
    }
}

// Link the application and collection
void fields::link_app(field_app_t app, std::string coll) {
    app_map_[app] = coll;
}

// Delete the named configuration
bool fields::delete_coll(std::string name) {
    if (name == "Default") return false;
    else {
        // get the entry
        auto it = coll_map_.find(name);
        if (it == coll_map_.end()) return false;
        else {
            // Delete the entry and remove it from the std::map
            delete (*it).second;
            coll_map_.erase(it);
            // Check if any app uses it and replace with Default
            for (auto ita = app_map_.begin(); ita != app_map_.end(); ita++) {
                if ((*ita).second == name) {
                    (*ita).second = "Default";
                }
            }
            return true;
        }
    }
}

void fields::save_update() {
    store_data();
}
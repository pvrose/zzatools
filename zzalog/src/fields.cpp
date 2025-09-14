#include "fields.h"
#include "utils.h"
#include "status.h"

#include <fstream>

#include <FL/Fl_Preferences.H>
#include <FL/fl_utf8.h>

extern status* status_;
extern std::string PROGRAM_ID;
extern std::string VENDOR;
extern std::string default_data_directory_;
extern Fl_Preferences::Root prefs_mode_;



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
collection_t* fields::collection(std::string name, field_list values) {
    if (coll_map_.find(name) != coll_map_.end()) {
        // Return the named collection
        return coll_map_.at(name);
    } else {
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
    if (!load_collections()) {
        // Read all the field field sets
        return;
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
    filename_ = default_data_directory_ + "fields.tsv";
    fl_make_path_for_file(filename_.c_str());
    ifstream ip;
    ip.open(filename_.c_str(), std::ios_base::in);
    if (!ip.good()) {
        char msg[128];
        snprintf(msg, sizeof(msg), 
            "FIELDS: Failed to open %s", 
            filename_.c_str());
        status_->misc_status(ST_WARNING, msg);
        return false;
    }
    std::string line;
    while(ip.good()) {
        getline(ip, line);
        std::vector<std::string> words;
        split_line(line, words, '\t');
        if (words.size() == 5) {
            collection_t* coll;
            // Each line consists of <COLL NAME>\t<INDEX\t<NAME>\t<WIDTH>\t<HEADING>
            if (coll_map_.find(words[0]) == coll_map_.end()) {
                // New collection
                coll = new collection_t;
                coll_map_[words[0]] = coll;
            } else {
                coll = coll_map_.at(words[0]);
            }
            int ix = std::stoi(words[1]);
            field_info_t info(words[2], words[4], std::stoi(words[3]));
            coll->resize(max((int)coll->size(), ix + 1));
            (*coll)[ix] = info;
        }
    }
    ip.close();
    return true;
}

// Store settings
void fields::store_data() {
	// Delete current settings
    if (filename_.length() == 0) {
         filename_ = default_data_directory_ + "fields.tsv";
        fl_make_path_for_file(filename_.c_str());
    }
    std::ofstream op;
    op.open(filename_.c_str(), std::ios_base::out);
    if (op.good()) {
        // Then get the field sets for the applications
        // For each field std::set
        auto it = coll_map_.begin();
        for (int i = 0; it != coll_map_.end(); i++, it++) {
            int num_fields = it->second->size();
            // For each field in the std::set
            for (int j = 0; j < num_fields; j++) {
                field_info_t field = (it->second)->at(j);
                op << it->first << '\t';
                op << j << '\t';
                op << field.field << '\t';
                op << field.width << '\t';
                op << field.header << '\n';
             }
        }
    }
    op.close();
}

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
#include "fields.h"
#include "utils.h"
#include "status.h"

#include <fstream>

#include <FL/Fl_Preferences.H>
#include <FL/fl_utf8.h>

extern status* status_;
extern string PROGRAM_ID;
extern string VENDOR;
extern string default_user_directory_;
extern Fl_Preferences::Root prefs_mode_;

using namespace std;

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
        string coll = app_map_.at(app);
        return collection(coll);
    } else {
        return nullptr;
    }
}

// Get the collection named.. and if necessary copy the collection
collection_t* fields::collection(string name, string source, bool* copied) {
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
collection_t* fields::collection(string name, field_list values) {
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
field_list fields::field_names(string name) {
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
   	// Read the field data from the settings
    Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences display_settings(settings, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	// Number of field sets (a field set is a set of fields and their ordering)
	// Then get the field sets for each
	char app_path[128];
	char* temp;
	// For each field ordering application
	for (int i = 0; i < FO_LAST; i++) {
		// Get the field set name
		sprintf(app_path, "App%d", i);
		fields_settings.get(app_path, temp, "Default");
		app_map_[(field_app_t)i] = string(temp);
		free(temp);
	}
	// Delete the existing field data
	for (auto it = app_map_.begin(); it != app_map_.end(); it++) {
        if (coll_map_.find(it->second) != coll_map_.end()) {
            delete coll_map_.at(it->second);
        }
    }
	coll_map_.clear();
    if (!load_collections(fields_settings)) {
        // Read all the field field sets
        add_collections(fields_settings, "");
    }
    if (coll_map_.find("Default") == coll_map_.end()) {
        // Create the default collection
        collection_t* coll = new collection_t;
        // For all fields in default field set add it to the new field set
        for (unsigned int i = 0; DEFAULT_FIELDS[i].field.size() > 0; i++) {
            coll->push_back(DEFAULT_FIELDS[i]);
        }
        coll_map_["Default"] = coll;
    }
}

// Read - <Prefs path>.fields.tsv
bool fields::load_collections(Fl_Preferences& settings) {
    filename_ = default_user_directory_ + "fields.tsv";
    fl_make_path_for_file(filename_.c_str());
    ifstream ip;
    ip.open(filename_.c_str(), ios_base::in);
    if (!ip.good()) {
        char msg[128];
        snprintf(msg, sizeof(msg), 
            "FIELDS: Failed to open %s, defaulting to looking in Preferences", 
            filename_.c_str());
        status_->misc_status(ST_WARNING, msg);
        return false;
    }
    string line;
    while(ip.good()) {
        getline(ip, line);
        vector<string> words;
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
            int ix = stoi(words[1]);
            field_info_t info(words[2], words[4], stoi(words[3]));
            coll->resize(max((int)coll->size(), ix + 1));
            (*coll)[ix] = info;
        }
    }
    ip.close();
    return true;
}

// Adding collections needs to iterate to cope with collections named "Contest/*"
void fields::add_collections(Fl_Preferences& settings, string name) {
    // For all the field sets in the settings
    for (int i = 0; i < settings.groups(); i++) {
        collection_t* field_set = new collection_t;
        // Get the name of the i'th field set
        string colln_name = settings.group(i);
        string full_name = (name.length()) ? name + "/" + colln_name : colln_name;
        Fl_Preferences colln_settings(settings, colln_name.c_str());
        // Test for further iteration - first group name != "Field #"
        string field_id = colln_settings.group(0);
        if (field_id.length() <= 6 || field_id.substr(0, 6) != "Field ") {
            add_collections(colln_settings, full_name);
        }
        else {
            // Create an initial array for the number of fields in the settings
            int num_fields = colln_settings.groups();
            field_set->resize(num_fields);
            // For each field in the field set
            for (int j = 0; j < num_fields; j++) {
                // Read the field info: name, width and heading from the settings
                field_info_t field;
                string field_id = colln_settings.group(j);
                Fl_Preferences field_settings(colln_settings, field_id.c_str());
                field_settings.get("Width", (int&)field.width, 50);
                char* temp;
                field_settings.get("Header", temp, "");
                field.header = temp;
                free(temp);
                field_settings.get("Name", temp, "");
                field.field = temp;
                free(temp);
                (*field_set)[j] = field;
            }
            // Add the field set to the list of field sets
            coll_map_[full_name] = field_set;
        }
    }
}

// Store settings
void fields::store_data() {
	// Read the field data from the registry
    Fl_Preferences settings(prefs_mode_, VENDOR.c_str(), PROGRAM_ID.c_str());
	Fl_Preferences display_settings(settings, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	// Delete current settings
	fields_settings.clear();
    if (filename_.length() == 0) {
         filename_ = default_user_directory_ + "fields.tsv";
        fl_make_path_for_file(filename_.c_str());
    }
    ofstream op;
    op.open(filename_.c_str(), ios_base::out);
    if (op.good()) {
        // Then get the field sets for the applications
        // For each field set
        auto it = coll_map_.begin();
        for (int i = 0; it != coll_map_.end(); i++, it++) {
            int num_fields = it->second->size();
            // For each field in the set
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

// Get the list of collection names
set<string> fields::coll_names() {
    set<string> result;
    result.clear();
    for (auto it = coll_map_.begin(); it != coll_map_.end(); it++) {
        result.insert((*it).first);
    }
    return result;
}

// Get the collection name for the app
string fields::coll_name(field_app_t app) {
    if (app_map_.find(app) == app_map_.end()) {
        return "";
    } else {
        return app_map_.at(app);
    }
}

// Link the application and collection
void fields::link_app(field_app_t app, string coll) {
    app_map_[app] = coll;
}

// Delete the named configuration
bool fields::delete_coll(string name) {
    if (name == "Default") return false;
    else {
        // get the entry
        auto it = coll_map_.find(name);
        if (it == coll_map_.end()) return false;
        else {
            // Delete the entry and remove it from the map
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
#include "fields.h"

#include <FL/Fl_Preferences.H>

extern Fl_Preferences* settings_;

// Constructor
fields::fields() {
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
	Fl_Preferences display_settings(settings_, "Display");
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
	// Read all the field field sets
    add_collections(fields_settings, "");
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
	Fl_Preferences display_settings(settings_, "Display");
	Fl_Preferences fields_settings(display_settings, "Fields");
	// Delete current settings
	fields_settings.clear();
	// Then get the field sets for the applications
	char app_path[128];
	for (int i = 0; i < FO_LAST; i++) {
		sprintf(app_path, "App%d", i);
		fields_settings.set(app_path, app_map_[(field_app_t)i].c_str());
	}
	// For each field set
	auto it = coll_map_.begin();
	for (int i = 0; it != coll_map_.end(); i++, it++) {
		int num_fields = it->second->size();
		Fl_Preferences colln_settings(fields_settings, it->first.c_str());
		// For each field in the set
		for (int j = 0; j < num_fields; j++) {
			char field_id[10];
			sprintf(field_id, "Field %d", j);
			Fl_Preferences field_settings(colln_settings, field_id);
			field_info_t field = (it->second)->at(j);
			field_settings.set("Name", field.field.c_str());
            field_settings.set("Width", (int)field.width);
            field_settings.set("Header", field.header.c_str());
		}
	}
    // settings_->flush();
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
#include "spec_data.h"
#include "specx_reader.h"
#include "../zzalib/utils.h"
#include "book.h"
#include "pfx_data.h"
#include "files.h"
#include "status.h"
#include "corr_dialog.h"
#include "../zzalib/callback.h"
#include "adi_writer.h"

#include <fstream>
#include <ostream>
#include <sstream>
#include <regex>
#include <chrono>
#include <climits>

#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

using namespace zzalog;
using namespace zzalib;
using namespace std;

extern Fl_Preferences* settings_;
extern book* book_;
extern pfx_data* pfx_data_;
extern status* status_;

// Default constructor
spec_data::spec_data()
	: adif_version_("")
	// Initialise validation results
	, error_reported_(false)
	, record_(nullptr)
	, error_count_(0)
	, error_record_count_(0)
	, record_count_(0)
	, record_corrected_(false)

	, abandon_validation_(false)
	, correction_message_("")
	, error_message_("")
	, saved_record_(nullptr)
	, inhibit_error_report_(false)
	, loaded_filename_("")
{
	// get data and load it
	load_data(false);
}

// Default destructor
spec_data::~spec_data()
{
	// delete the data - for each dataset
	for (auto it = begin(); it != end(); it++) {
		// Get the dataset 
		spec_dataset* dataset = it->second;
		// Clear the column names field
		dataset->column_names.clear();
		// For each record of data
		for (auto it2 = dataset->data.begin(); it2 != dataset->data.end(); it2++) {
			// empty the data and free the memory
			it2->second->clear();
			delete it2->second;
		}
		// clear any info held by the map item and free the memory
		dataset->data.clear();
		delete dataset;
	}
	// Clear all the various lists and sets
	field_names_.clear();
	userdef_names_.clear();
	appdef_names_.clear();
	datatype_indicators_.clear();

	// close any report file 
	char message[1024];
	sprintf(message, "VALIDATE: %d errors found, %d records with errors, %d records checked.\n",
		error_count_, error_record_count_, record_count_);
	status_->misc_status(error_count_ ? ST_WARNING : ST_OK, message);
}

// Get the data path to the files - returns directory name
string spec_data::get_path(bool force) {
	// get the datapath settings group.
	Fl_Preferences datapath(settings_, "Datapath");
	char *dirname = nullptr;
	string directory_name;
	// get the value from settings or force new browse
	if (force || !datapath.get("Reference", dirname, "")) {
		// We do not have one - so open chooser to get one
		//Fl_File_Chooser* chooser = new Fl_File_Chooser(dirname, nullptr, Fl_File_Chooser::DIRECTORY,
		//	"Select reference file directory");
		Fl_Native_File_Chooser* chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser->directory(dirname);
		chooser->title("Select reference file directory");
		while (chooser->show()) {}
		directory_name = chooser->filename();
		delete chooser;
	}
	else {
		directory_name = dirname;
	}
	// Append a foreslash if one is not present
	if (directory_name.back() != '/') {
		directory_name.append(1, '/');
	}
	if (dirname) free(dirname);
	return directory_name;
}

// load the data
bool spec_data::load_data(bool force) {
	// Clear all containers
	field_names_.clear();
	userdef_names_.clear();
	appdef_names_.clear();
	user_enums_.clear();
	datatype_indicators_.clear();
	this->clear();

	bool ok = true;
	// Get the directory
	string directory = get_path(force);
	// Open an ADIF Specification input interpreter
	specx_reader* reader = new specx_reader;
	// For each file
	// Create an input stream from the file
	string file_name = directory + ADIF_FILE;
	ifstream file;
	file.open(file_name.c_str(), fstream::in);
	// Load data from the input stream to the appropriate dataset
	if (file.good()) {
		ok = reader->load_data(this, file, adif_version_);
	} else {
		ok = false;
		char* message = new char[30 + file_name.length()];
		sprintf(message, "ADIF SPEC: Fail to open %s", file_name.c_str());
		status_->misc_status(ST_FATAL, message);
	} 
	file.close();
	delete reader;
	if (ok) {
		process_fieldnames();
		process_modes();
		add_my_appdefs();
	}
	return ok;
}

// Sort the field names, create custom lookup for datatype_indicators
void spec_data::process_fieldnames() {
	// Get the relevent datasets
	spec_dataset* fields = dataset("Fields");
	spec_dataset* data_types = dataset("Data Types");
	// Clear the lookup tables
	field_names_.clear();
	datatype_indicators_.clear();
	// For each entry in Fields dataset
	for (auto it = fields->data.begin(); it != fields->data.end(); it++) {
		// Add its name  to the lookup
		field_names_.insert(it->first);
		// Get iterator to its data type dataset record
		auto it_data_type = data_types->data.find(it->second->at("Data Type"));
		if (it_data_type != data_types->data.end()) {
			// It has noe, so add the indicator character to the lookup table
			if (it_data_type->second->find("Data Type Indicator") != it_data_type->second->end()) {
				datatype_indicators_[it->first] = it_data_type->second->at("Data Type Indicator")[0];
			}
			else {
				datatype_indicators_[it->first] = ' ';
			}
		}
		else {
			// It hasn't so add a space character to the lookup table
			datatype_indicators_[it->first] = ' ';
		}
	}
}

// Get the DXCC award mode for a particulat ADIF mode
string spec_data::dxcc_mode(string mode) {
	// For non-data modes, there is mostly a one-to-one mapping (except HFSK->CW) all other modes map to DATA
	if (mode == "AM") {
		return "AM";
	}
	else if (mode == "ATV") {
		return "ATV";
	}
	else if (mode == "CW" ||
		mode == "HFSK") {
		return "CW";
	}
	else if (mode == "FAX") {
		return "FAX";
	}
	else if (mode == "FM") {
		return "FM";
	}
	else if (mode == "SSB") {
		return "SSB";
	}
	else if (mode == "SSTV") {
		return "SSTV";
	}
	else {
		return "DATA";
	}

}

// Get the ADIF mode for a submode
string spec_data::mode_for_submode(string submode) {
	// Get the Submode dataset
	spec_dataset* table = dataset("Submode");
	auto it = table->data.find(submode);
	if (it != table->data.end()) {
		// If there's an entry for this mode return the Mode field
		return it->second->at("Mode");
	}
	else {
		// Else return empty string
		return "";
	}
}

// Get the band for a specific frequency
string spec_data::band_for_freq(double frequency) {
	// Get the Band dataset
	spec_dataset* table = dataset("Band");
	string result = "";
	bool found = false;
	// Scan the band data until we get an entry for which the frequency lies between the band edges
	for (auto it = table->data.begin(); it != table->data.end(); it++) {
		// If the frequency is within that specified, return band and stop
		double lower = stod(it->second->at("Lower Freq (MHz)"));
		double upper = stod(it->second->at("Upper Freq (MHz)"));
		if (lower <= frequency && upper >= frequency) {
			result = it->first;
			found = true;
		}
	}
	return result;
}

// Get the Lower frequency for a band
double spec_data::freq_for_band(string band) {
	// Get the Band dataset
	spec_dataset* table = dataset("Band");
	auto it = table->data.find(band);
	if (it != table->data.end()) {
		// Return the Lower Freq filed for the band entry
		return stod(it->second->at("Lower Freq (MHz)"));
	}
	else {
		// Else return NAN
		return nan("");
	}
}

// Is a particulat mode a submode of another mode?
bool spec_data::is_submode(string mode) {
	// Get the Submode dataset
	spec_dataset* submodes = dataset("Submode");
	if (submodes->data.find(mode) != submodes->data.end()) {
		// If there's an entry return that it's a submode
		return true;
	}
	else {
		// else it's a mode
		return false;
	}
}

// Get named dataSet
spec_dataset* spec_data::dataset(string name) {
	// Try and get the dataset
	auto it = find(name);
	if (it != end()) {
		// Return it if it is there
		return it->second;
	}
	else {
		// else return nullptr
		return nullptr;
	}
}

// Get Adif version
string spec_data::adif_version() {
	return adif_version_;
}

// Get sorted list of field names
set<string>* spec_data::sorted_fieldnames() {
	return &field_names_;
}

// Add user defined fields - returns true if a new one. id is the USERDEF number, name 
bool spec_data::add_userdef(int id, const string& name, char indicator, string& values) {
	// Check ID not already defined
	if (userdef_names_.size() > (unsigned)id && userdef_names_[id] != "") {
		char message[256];
		sprintf(message, "ADIF SPEC: User defined field id %d already allocated - fieldname %s ignored", id, name.c_str());
		status_->misc_status(ST_ERROR, message);
		return false;
	}
	// Check field name not already in use. Get Fields dataset
	spec_dataset* fields = dataset("Fields");
	if (fields->data.find(name) != fields->data.end()) {
		char message[256];
		sprintf(message, "ADIF SPEC: User defined field %s already allocated - fieldname ignored", name.c_str());
		status_->misc_status(ST_ERROR, message);
		return false;
	}
	// Create a Fields database entry for USERDEFn for use in the header
	map<string, string>* temp_map = new map<string, string>;
	char temp[128];
	string userdef_name;
	(*temp_map)["Data Type"] = "String";
	(*temp_map)["Enumeration"] = "";
	sprintf(temp, "Header declaration of USERDEF%d", id);
	(*temp_map)["Description"] = string(temp);
	(*temp_map)["Header Field"] = "Y";
	(*temp_map)["Minimum Value"] = "";
	(*temp_map)["Maximum Value"] = "";
	(*temp_map)["Import-only"] = "";
	(*temp_map)["Comments"] = "";
	(*temp_map)["ADIF Version"] = "";
	(*temp_map)["ADIF Status"] = "Not approved";
	// Add the entry to the Fields dataset
	sprintf(temp, "USERDEF%d", id);
	userdef_name = string(temp);
	// Add it to the lookup tables
	fields->data[userdef_name] = temp_map;
	datatype_indicators_[userdef_name] = ' ';
	datatype_indicators_[name] = indicator;

	// The data type is an enumeration, values is a comma separated list of enumeration values
	string enumeration_name;
	if (indicator == 'E' && values.length() > 0) {
		vector<string> value_array;
		sprintf(temp, "User Enumeration %s", name.c_str());
		enumeration_name = string(temp);
		// Add new dataset for the enumeration values
		spec_dataset* dataset = new spec_dataset;
		(*this)[enumeration_name] = dataset;
		// Add column names
		dataset->column_names.resize(3);
		dataset->column_names[0] = "Comments";
		dataset->column_names[1] = "ADIF Version";
		dataset->column_names[2] = "ADIF Status";
		// now create records for each enumeration value
		split_line(values, value_array, ',');
		for (unsigned int i = 0; i < value_array.size(); i++) {
			temp_map = new map<string, string>;
			(*temp_map)["Comments"] = "";
			(*temp_map)["ADIF Version"] = "";
			(*temp_map)["ADIF Status"] = "Not approved";
			dataset->data[value_array[i]] = temp_map;
		}
	}
	else {
		enumeration_name = "";
	}

	// The data type is Numeric - values is  "min value:max value"
	string min_value;
	string max_value;
	if (indicator == 'N' && values.length() > 0) {
		vector<string> value_array;
		split_line(values, value_array, ':');
		min_value = value_array[0];
		max_value = value_array[1];
	}
	else {
		min_value = "";
		max_value = "";
	}

	// now create an entry for using the USERDEF in the records
	temp_map = new map<string, string>;
	(*temp_map)["Data Type"] = datatype(indicator);
	(*temp_map)["Enumeration"] = enumeration_name;
	sprintf(temp, "Record field declaration for USERDEF%d", id);
	(*temp_map)["Description"] = string(temp);
	(*temp_map)["Header Field"] = "";
	(*temp_map)["Minimum Value"] = min_value;
	(*temp_map)["Maximum Value"] = max_value;
	(*temp_map)["Import-only"] = "";
	(*temp_map)["Comments"] = "";
	(*temp_map)["ADIF Version"] = "";
	(*temp_map)["ADIF Status"] = "Not approved";
	dataset("Fields")->data[name] = temp_map;

	// Add user defined field name to the list
	if ((unsigned)id >= userdef_names_.size()) {
		userdef_names_.resize(id + 1);
	}
	userdef_names_[id] = name;

	field_names_.insert(userdef_name);

	return true;
}

// Get data type indicator for the field from the lookup table
char spec_data::datatype_indicator(string& field_name) {
	return datatype_indicators_[field_name];
}

// Get list or range for the field
string spec_data::userdef_values(string& field_name) {
	// Get the Fields dataset
	spec_dataset* fields = dataset("Fields");
	auto it_field = fields->data.find(field_name);
	if (it_field != fields->data.end()) {
		// If the field has an entry
		// Get the Data Types dataset
		spec_dataset* data_types = dataset("Data Types");
		if (it_field->second->at("Data Type") == "Enumeration") {
			// Build up the comma-separated list of enumeration values
			char temp[100];
			string enumeration_name;
			sprintf(temp, "User Enumeration %s", field_name.c_str());
			enumeration_name = string(temp);
			// Get the specific user enumeration dataset
			spec_dataset* enumerations = dataset(enumeration_name);
			string values;
			// For each entry in the dataset append the key to the list
			for (auto it = enumerations->data.begin(); it != enumerations->data.end(); ) {
				values += it->first;
				it++;
				if (it != enumerations->data.end()) values += ',';
			}
			return values;
		}
		if (it_field->second->at("Data Type") == "Number") {
			// Build up min:max range
			string min_value = "";
			if (it_field->second->find("Minimum Value") != it_field->second->end()) {
				min_value = it_field->second->at("Minimum Value");
			}
			string max_value = "";
			if (it_field->second->find("Maximum Value") != it_field->second->end()) {
				max_value = it_field->second->at("Maximum Value");
			}
		}
	}
	return "";
}

// Add application defined field - return true if successfully added
bool spec_data::add_appdef(const string& name, char indicator) {
	// Check field name not already in use - and add it if its not
	map<string, string>* temp_map;
	// Get the Fields and Data Types datasets
	spec_dataset* fields = dataset("Fields");
	spec_dataset* data_types = dataset("Data Types");
	auto it_field = fields->data.find(name);
	if (it_field != fields->data.end()) {
		// The field already exists, get its datatype
		string data_type = it_field->second->at("Data Type");
		auto it_data_type = data_types->data.find(data_type);
		// Get its existing type indicator and check it's the same or not empty
		string indicator_value = "";
		if (it_data_type->second->find("Data Type Indicator") != it_data_type->second->end()) {
			indicator_value = it_data_type->second->at("Data Type Indicator");
		}
		if (indicator_value.length() > 0 && indicator != ' ' && indicator_value[0] != indicator) {
			char message[256];
			sprintf(message, "ADIF SPEC: Application defined field %s attempted to redefine data type from %s to %c", name.c_str(), indicator_value.c_str(), indicator);
			status_->misc_status(ST_ERROR, message);
			return false;
		}
	}
	else {
		// New app defined field
		string data_type;
		// Default to S and add it to the lookup table
		char real_indicator = indicator == ' ' ? 'S' : indicator;
		datatype_indicators_[name] = real_indicator;
		bool not_found = true;
		// Look in the data types to see if one has the indicator
		for (auto it = data_types->data.begin(); it != data_types->data.end() && not_found; it++) {
			if (it->second->find("Data Type Indicator") != it->second->end() && it->second->at("Data Type Indicator")[0] == real_indicator) {
				data_type = it->first;
				not_found = false;
			}
		}
		if (not_found) {
			// If it doesn't
			char message[256];
			sprintf(message, "ADIF SPEC: Application defined field %s has invalid data type %c", name.c_str(), real_indicator);
			status_->misc_status(ST_ERROR, message);
			return false;
		}

		// now build an entry for the app defined field for the Fields dataset
		temp_map = new map<string, string>;
		(*temp_map)["Data Type"] = data_type;
		(*temp_map)["Enumeration"] = "";
		(*temp_map)["Description"] = "Application-defined field";
		(*temp_map)["Header Field"] = "";
		(*temp_map)["Minimum Value"] = "";
		(*temp_map)["Maximum Value"] = "";
		(*temp_map)["Import-only"] = "";
		(*temp_map)["Comments"] = "";
		(*temp_map)["ADIF Version"] = "";
		(*temp_map)["ADIF Status"] = "Not approved";
		dataset("Fields")->data[name] = temp_map;

		appdef_names_.insert(name);

		// Add to the set of names
		field_names_.insert(name);
	}

	return true;
}

// Remove existing user defined fields
void spec_data::delete_userdefs() {
	// Get the field dataset
	spec_dataset* fields = dataset("Fields");
	// Look in each entry in the user def name list.
	for (unsigned int i = 0; i < userdef_names_.size(); i++) {
		string field_name = userdef_names_[i];
		if (field_name != "") {
			// Get the entry in the Fields dataset for the user defined name
			auto it_field = fields->data.find(field_name);
			if (it_field != fields->data.end()) {
				// Clear the fields in the entry and release memory
				it_field->second->clear();
				delete it_field->second;
				// Remove the entry from the dataset
				if (!fields->data.erase(field_name)) {
					char message[256];
					sprintf(message, "ADIF SPEC: Internal discrepancy - user defined field %s", field_name.c_str());
					status_->misc_status(ST_SEVERE, message);
				}
			}

			char temp[100];
			string userdef_name;
			sprintf(temp, "USERDEF%d", i);
			userdef_name = string(temp);
			// Get the entry in the fields dataset for the USERDEFn field name
			auto it_userdef = fields->data.find(userdef_name);
			if (it_userdef != fields->data.end()) {
				// Clear the fields in the entry and release the memory
				it_userdef->second->clear();
				delete it_userdef->second;
				// Remove the entry from the dataset
				if (!fields->data.erase(userdef_name)) {
					char message[256];
					sprintf(message, "ADIF SPEC: Internal discrepancy - user defined field %s", userdef_name.c_str());
					status_->misc_status(ST_SEVERE, message);
				}
			}
		}
	}
	// Remove the items in the list of user defined names
	userdef_names_.clear();
}

// Remove existing application defined names
void spec_data::delete_appdefs() {
	// Get the Fields dataset
	spec_dataset* fields = dataset("Fields");
	// For each entry in the list of application defined names
	for (auto it = appdef_names_.begin(); it != appdef_names_.end(); it++) {
		if ((*it) != "") {
			// Get the entry in the dataset
			auto it_appdef = fields->data.find(*it);
			if (it_appdef != fields->data.end()) {
				// Remove all the fields in the entry and release the memory
				it_appdef->second->clear();
				delete it_appdef->second;
				// Remove the entry from the dataset
				if (!fields->data.erase(*it)) {
					char message[256];
					sprintf(message, "ADIF SPEC: Internal discrepancy - app defined field %s", (*it).c_str());
					status_->misc_status(ST_SEVERE, message);
				}
			}
		}
	}
	// Remove all the items in the list
	appdef_names_.clear();
}

// Remove changes that added user defined enums
void spec_data::delete_user_data() {
	// Delete macros
	macros_.clear();
	// Get original and current datasets
	spec_dataset* original = dataset("Original Fields");
	if (original != nullptr) {
		spec_dataset* current = dataset("Fields");
		for (auto it = original->data.begin(); it != original->data.end(); it++) {
			current->data[(*it).first] = (*it).second;
		}
	}
	delete original;
	erase("Original Fields");
}

// Return true if field is a user defined one.
bool spec_data::is_userdef(string field_name) {
	// For all items in the list of user defined names
	for (unsigned int i = 0; i < userdef_names_.size(); i++) {
		// Return true if it's this one
		if (userdef_names_[i] == field_name) return true;
	}
	// Not found in the list
	return false;

}

// Get data type from indicator
string spec_data::datatype(char indicator) {
	// Get the Data Types dataset
	spec_dataset* data_types = dataset("Data Types");
	// For all entries in the dataset
	for (auto it = data_types->data.begin(); it != data_types->data.end(); it++) {
		// Get the Indicator field from the enyry
		auto it_indicator = it->second->find("Data Type Indicator");
		if (it_indicator != it->second->end() && it_indicator->second[0] == indicator) {
			// Returrn the name of the entry that matches
			return it->first;
		}
	}
	// Not found in the list
	char message[256];
	sprintf(message, "ADIF SPEC: Cannot find the data type for indicator %c", indicator);
	status_->misc_status(ST_ERROR, message);
	return "";
}

// Get data type from fieldname
string spec_data::datatype(string field_name) {
	// Get field dataset
	spec_dataset* fields = dataset("Fields");
	// get record
	auto it = fields->data.find(field_name);
	if (it != fields->data.end()) {
		return it->second->at("Data Type");
	}
	else {
		char message[256];
		sprintf(message, "ADIF SPEC: Cannot find data type for field name %s", field_name.c_str());
		status_->misc_status(ST_ERROR, message);
		return "";
	}
}

// Get enumeration name for the field name - note it may be DXCC dependent
string spec_data::enumeration_name(const string& field_name, record* record) {
	// Get the Fields dataset
	spec_dataset* fields = dataset("Fields");
	// Get the entry for the field name
	auto it_field = fields->data.find(field_name);
	if (it_field != fields->data.end()) {
		// If it's a valid field name
		if (it_field->second->at("Data Type") == "Enumeration" && it_field->second->find("Enumeration") != it_field->second->end()) {
			// And if it's an enumeration - get the enumeration name
			string enumeration_name = it_field->second->at("Enumeration");
			if (enumeration_name.substr(0, 7) != "Submode") {
				int open = enumeration_name.find('[');
				if (open != string::npos && record) {
					int close = enumeration_name.find(']');
					string index = enumeration_name.substr(open + 1, close - open - 1);
					enumeration_name = enumeration_name.substr(0, open) + '[' + record->item(index) + ']';
				}
			}
			else {
				enumeration_name = "Submode";
			}
			if (dataset(enumeration_name) != nullptr) {
				// A dataset exists for just the enumeration name
				return enumeration_name;
			}
			// No dataset exists
			return  "";
		}
		else if (it_field->second->at("Data Type") == "Dynamic" && it_field->second->find("Enumeration") != it_field->second->end()) {
			// Return the enumeration name
			string enumeration_name = it_field->second->at("Enumeration");
			if (dataset(enumeration_name) != nullptr) return enumeration_name;
			else return "";
		}
		else if (it_field->second->at("Data Type") == "Macro" && it_field->second->find("Enumeration") != it_field->second->end()) {
			// Return the enumeration name
			string enumeration_name = it_field->second->at("Enumeration");
			if (dataset(enumeration_name) != nullptr) return enumeration_name;
			else return "";
		}
		else {
			// Field is not an enumeration
			return "";
		}
	}
	else {
		// Field does not exist
		return "";
	}
}

// Add a user defined enumeration to an existing field definition
// Used for MY_RIG, MY_ANTENNA, STATION_CALLSIGN
bool spec_data::add_user_enum(string field, string value) {
	char message[128];
	char enumeration_name[128];
	snprintf(enumeration_name, 128, "Dynamic %s", field.c_str());
	string enum_name = enumeration_name;

	// Get the field definition
	spec_dataset* fields = dataset("Fields");
	auto it_field = fields->data.find(field);
	if (it_field != fields->data.end()) {
		// If it's a valid field name
		if (it_field->second->at("Data Type") == "Enumeration") {
			// Field is already defined as am enumeration
			snprintf(message, 128, "ADIF SPEC: Field %s is an enumeration %s cannot change it to %s",
				field.c_str(),
				it_field->second->at("Enumeration").c_str(),
				enumeration_name);
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		else if (it_field->second->at("Data Type") == "Macro") {
			// Field is already defined as am macro
			snprintf(message, 128, "ADIF SPEC: Field %s is a macro %s cannot change it to %s",
				field.c_str(),
				it_field->second->at("Enumeration").c_str(),
				enumeration_name);
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		else if (it_field->second->at("Data Type") != "Dynamic") {
			snprintf(message, 128, "ADIF SPEC: Field %s replacing type %s with %s",
				field.c_str(),
				it_field->second->at("Data Type").c_str(),
				enumeration_name);
				status_->misc_status(ST_WARNING, message);
			// Enumeration won't exist
			spec_dataset* original_data = dataset("Original Fields");
			if (original_data == nullptr) {
				original_data = new spec_dataset;
				(*this)["Original Fields"] = original_data;
				original_data->column_names = fields->column_names;
			}
			original_data->data[field] = new map<string, string>(*(it_field->second));
			(*it_field->second)["Enumeration"] = enum_name;
			(*it_field->second)["Data Type"] = "Dynamic";
			(*it_field->second)["ADIF Status"] = "Not approved";
		}
	}
	else {
		// Field does not exist
		snprintf(message, 128, "ADIF SPEC: Trying to add an enumeration to field %s which doesn't exist", field.c_str());
		status_->misc_status(ST_ERROR, message);
		return false;
	}
	// Check that "Dynamic" is a member of Data Types
	spec_dataset* types_dataset = dataset("Data Types");
	if (types_dataset->data.find("Dynamic") == types_dataset->data.end()) {
		map<string, string>* dynamic_data = new map<string, string>;
		(*dynamic_data)["Comments"] = "Application defined dynamic enumeration";
		(*dynamic_data)["Data Type Indicator"] = "E";
		(*dynamic_data)["Data Type Name"] = "Dynamic";
		(*dynamic_data)["Description"] = "Dynamic enumeration for field " + field;
		types_dataset->data["Dynamic"] = dynamic_data;
	}
	// Find data set for enumeration
	// Add new dataset for the enumeration values
	spec_dataset* enum_dataset = dataset(enum_name);
	if (enum_dataset == nullptr) {
		enum_dataset = new spec_dataset;
		(*this)[enum_name] = enum_dataset;
		// Add column names
		enum_dataset->column_names.resize(3);
		enum_dataset->column_names[0] = "Comments";
		enum_dataset->column_names[1] = "ADIF Version";
		enum_dataset->column_names[2] = "ADIF Status";
	}
	// Add if not already added
	if (enum_dataset->data.find(value) == enum_dataset->data.end()) {
		auto temp_map = new map<string, string>;
		(*temp_map)["Comments"] = "";
		(*temp_map)["ADIF Version"] = "";
		(*temp_map)["ADIF Status"] = "Not approved";
		enum_dataset->data[value] = temp_map;
	}

	return true;
}

/// Add a user enumeration that points to a macro definition (i.e. additional
// fields inferred by that value)
bool spec_data::add_user_macro(string field, string value, macro_defn macro) {
	char message[128];
	char enumeration_name[128];
	snprintf(enumeration_name, 128, "Macro %s", field.c_str());
	string enum_name = enumeration_name;
	macro_changes_.clear();

	// Get the field definition
	spec_dataset* fields = dataset("Fields");
	auto it_field = fields->data.find(field);
	if (it_field != fields->data.end()) {
		if (it_field->second->at("Data Type") == "Enumeration") {
			// Field is already defined as am enumeration
			snprintf(message, 128, "ADIF SPEC: Field %s is an enumeration %s cannot change it to %s",
				field.c_str(),
				it_field->second->at("Enumeration").c_str(),
				enumeration_name);
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		else if (it_field->second->at("Data Type") == "Dynamic") {
			// Field is already defined as am macro
			snprintf(message, 128, "ADIF SPEC: Field %s is a dynamic %s cannot change it to %s",
				field.c_str(),
				it_field->second->at("Enumeration").c_str(),
				enumeration_name);
			status_->misc_status(ST_ERROR, message);
			return false;
		}
		else if (it_field->second->at("Data Type") != "Macro") {
			snprintf(message, 128, "ADIF SPEC: Field %s replacing type %s with %s",
				field.c_str(),
				it_field->second->at("Data Type").c_str(),
				enumeration_name);
			status_->misc_status(ST_WARNING, message);
			// Enumeration won't exist
			spec_dataset* original_data = dataset("Original Fields");
			if (original_data == nullptr) {
				original_data = new spec_dataset;
				(*this)["Original Fields"] = original_data;
				original_data->column_names = fields->column_names;
			}
			original_data->data[field] = new map<string, string>(*(it_field->second));
			(*it_field->second)["Enumeration"] = enum_name;
			(*it_field->second)["Data Type"] = "Macro";
			(*it_field->second)["ADIF Status"] = "Not approved";
		}
	}
	else {
		// Field does not exist
		snprintf(message, 128, "ADIF SPEC: Trying to add an enumeration to field %s which doesn't exist", field.c_str());
		status_->misc_status(ST_ERROR, message);
		return false;
	}
	// Check that "Dynamic" is a member of Data Types and create it if it isn't
	spec_dataset* types_dataset = dataset("Data Types");
	if (types_dataset->data.find("Macro") == types_dataset->data.end()) {
		map<string, string>* dynamic_data = new map<string, string>;
		(*dynamic_data)["Comments"] = "Application defined dynamic enumeration";
		(*dynamic_data)["Data Type Indicator"] = "E";
		(*dynamic_data)["Data Type Name"] = "Macro";
		(*dynamic_data)["Description"] = "Macro enumeration for field " + field;
		types_dataset->data["Macro"] = dynamic_data;
	}
	// Add new dataset for the enumeration values
	spec_dataset* macro_dataset = dataset(enum_name);
	if (macro_dataset == nullptr) {
		macro_dataset = new spec_dataset;
		(*this)[enum_name] = macro_dataset;
		// Add column names
		macro_dataset->column_names.resize(5);
		macro_dataset->column_names[0] = "Comments";
		macro_dataset->column_names[1] = "Description";
		macro_dataset->column_names[2] = "Macro Definition";
		macro_dataset->column_names[3] = "ADIF Version";
		macro_dataset->column_names[4] = "ADIF Status";
	}
	// Get map for field provided - create it if necessaruy
	macro_map* this_map;
	if (macros_.find(field) == macros_.end()) {
		this_map = new macro_map;
		macros_[field] = this_map;
	}
	else {
		this_map = macros_.at(field);
	}
	// Copy fields from supplied macro definition
	macro_defn* defn;
	if (this_map->find(value) == this_map->end()) {
		// A new macro definition - create it and add to the macro set
		char message[128];
		snprintf(message, 128, "ADIF SPEC: Macro %s.%s being created",
			field.c_str(),
			value.c_str());
		status_->misc_status(ST_NOTE, message);
		defn = new macro_defn;
		defn->fields = new record;
		(*this_map)[value] = defn;
	}
	else {
		defn = this_map->at(value);
	}
	// Copy all the new fields being defined - skip if supplied fields is a nullptr
	if (macro.fields) {
		for (auto it = macro.fields->begin(); it != macro.fields->end(); it++) {
			string def_field = (*it).first;
			if (defn->fields->item_exists(def_field)) {
				if (defn->fields->item(def_field) != (*it).second) {
					char message[128];
					snprintf(message, 128, "ADIF SPEC: Macro %s.%s already had field %s defined old = %s, new = %s",
						field.c_str(),
						value.c_str(),
						def_field.c_str(),
						defn->fields->item(def_field).c_str(),
						(*it).second.c_str());
					status_->misc_status(ST_WARNING, message);
					macro_changes_.insert(def_field);
				}
			}
			else {
				if ((*it).second.length()) {
					char message[128];
					snprintf(message, 128, "ADIF SPEC: Macro %s.%s defining field %s value %s",
						field.c_str(),
						value.c_str(),
						def_field.c_str(),
						(*it).second.c_str());
					status_->misc_status(ST_NOTE, message);
					macro_changes_.insert(def_field);
				}
			}
			defn->fields->item((*it).first, (*it).second);
		}
	}

	// now add records for each macro definition
	map<string, string>* temp_map;
	if (macro_dataset->data.find(value) == macro_dataset->data.end()) {
		temp_map = new map<string, string>;
		(*temp_map)["Comments"] = "";
		(*temp_map)["Description"] = macro.description;
		(*temp_map)["ADIF Version"] = "";
		(*temp_map)["ADIF Status"] = "Not approved";

		macro_dataset->data[value] = temp_map;
	}
	else {
		temp_map = macro_dataset->data.at(value);
	}
	string adif;
	for (auto f = defn->fields->begin(); f != defn->fields->end(); f++) {
		adif += adi_writer::item_to_adif(defn->fields, (*f).first);
	}
	(*macro_dataset->data[value])["Macro Definition"] = adif;
	if (macro.description.length()) (*temp_map)["Description"] = macro.description;

	return true;
}

record* spec_data::expand_macro(string field, string value) {
	char message[128];
	if (macros_.size() == 0) {
		status_->misc_status(ST_NOTE, "ADIF SPEC: No macros defined yet");
		return nullptr;
	}
	else if (macros_.find(field) == macros_.end()) {
		snprintf(message, 128, "ADIF SPEC: No macros defined for field %s", field.c_str());
		status_->misc_status(ST_ERROR, message);
		return nullptr;
	}
	else {
		macro_map* field_macros = macros_.at(field);
		if (field_macros->find(value) == field_macros->end()) {
			snprintf(message, 128, "ADIF SPEC: Macro %s not defined for field %s",
				value.c_str(),
				field.c_str());
			status_->misc_status(ST_ERROR, message);
			return nullptr;
		}
		else {
			return field_macros->at(value)->fields;
		}
	}
}

// Return macro changes since last update
set<string> spec_data::get_macro_changes() {
	return macro_changes_;
}

// The DXCC has ADIF defined primary administrative districts
bool spec_data::has_states(int dxcc) {
	// Look up DXCC name in list of DXCCs with "states".
	if (dataset("Primary_Administrative_Subdivision[" + to_string(dxcc) + "]")) {
		return true;
	}
	else {
		return false;
	}
}

// Check that the data is a valid number between minimum and maximum values
error_t spec_data::check_number(const string&  data, const string&  field, const string&  datatype)
{
	// Convert the string to a double
	size_t pos;
	double data_value;
	try {
		data_value = stod(data, &pos);
	}
	catch (invalid_argument&) {
		// The first character is not numeric - invalid
		return VE_VALUE_INVALID;
	}
	if (pos != data.length()) {
		// If the whole string is not a valid decimal number - invalid
		return VE_VALUE_INVALID;
	}
	// Get the ADIF field parameters. Minimum and Maximum values - uses largest +/- value if blank
	map<string, string>* fields = dataset("Fields")->data.at(field);
	double min_value = -DBL_MAX;
	if (fields->find("Minimum Value") != fields->end() && fields->at("Minimum Value").length()) {
		min_value = stod(fields->at("Minimum Value"));
	}
	double max_value = DBL_MAX;
	if (fields->find("Maximum Value") != fields->end() && fields->at("Maximum Value").length()) {
		max_value = stod(fields->at("Maximum Value"));
	}
	// Check the value is within range
	if (data_value >= min_value && data_value <= max_value) {
		// It is - OK
		return VE_OK;
	}
	else {
		// It's not - out of range
		return VE_VALUE_OUT_OF_RANGE;
	}
}

// Check the value is an integer between the minimum and maximum defined - also check if compatible with other fields
error_t spec_data::check_integer(const string&  data, const string&  field, const string&  datatype)
{
	// Convert the data to an integer to check it's a valid integer value
	size_t pos;
	int data_value;
	try {
		data_value = stoi(data, &pos);
	} 
	catch (invalid_argument&) {
		// First character is not numeric - invalid
		return VE_VALUE_INVALID;
	}
	if (pos != data.length()) {
		// If the whole string is not a valid integer - invalid
		return VE_VALUE_INVALID;
	}
	// Check compatibility with other fields
	if (field == "CQZ" || field == "MY_CQZ") {
		// Check that the CQ Zone is listed for the DXCC.
		int dxcc_code;
		try {
			if (field.length() == 6) {
				// MY_CQZ - check against MY_DXCC
				dxcc_code = stoi(record_->item("MY_DXCC"));
			}
			else {
				// CQZ - check against DXCC
				dxcc_code = stoi(record_->item("DXCC"));
			}
		}
		catch (const invalid_argument&) {
			// exception raised if first character is non-numeric or empty string
			dxcc_code = 0;
		}

		if (dxcc_code) {
			// Get the entry in the prefix database for the DXCC code
			prefix* prefix = pfx_data_->get_prefix(dxcc_code);
			// Go through the list of zones for the prefix
			for (unsigned int i = 0; i < prefix->cq_zones_.size(); i++) {
				// If the entry matches - OK
				if (data_value == prefix->cq_zones_[i]) return VE_OK;
			}
			// Not found in the list for the DXCC
			return VE_VALUE_INCOMPATIBLE;
		}
		else {
			// If DXCC field is blank or 0 (for a /MM station) it is not checkable
			return VE_VALUE_UNCHECKABLE;
		}
	}
	else if (field == "ITUZ" || field == "MY_ITUZ") {
		// Check that the ITU Zone is listed for the DXCC
		int dxcc_code;
		try {
			if (field.length() == 7) {
				// MY_ITUZ - check MY_DXCC
				dxcc_code = stoi(record_->item("MY_DXCC"));
			}
			else {
				// ITUZ - check DXCC
				dxcc_code = stoi(record_->item("DXCC"));
			}
		}
		catch (const invalid_argument&) {
			// exception raised if first character is non-numeric or empty string
			dxcc_code = 0;
		}
		if (dxcc_code) {
			// Get the prefix database entry for the DXCC code
			prefix* prefix = pfx_data_->get_prefix(dxcc_code);
			for (unsigned int i = 0; i < prefix->itu_zones_.size(); i++) {
				if (data_value == prefix->itu_zones_[i]) return VE_OK;
			}
			// Not found in the list for the DXCC
			return VE_VALUE_INCOMPATIBLE;
		}
		else {
			// If DXCC field is blank or 0 (for a /MM station) it is not checkable
			return VE_VALUE_UNCHECKABLE;
		}
	}
	// Get the ADIF field parameters. Minimum and Maximum values - uses largest +/- value if blank
	map<string, string>* fields = dataset("Fields")->data.at(field);
	int min_value = INT_MIN;
	if (fields->find("Minimum Value") != fields->end()) {
		min_value = stoi(fields->at("Minimum Value"));
	}
	int max_value = INT_MAX;
	if (fields->find("Maximum Value") != fields->end()) {
		max_value = stoi(fields->at("Maximum Value"));
	}
	// Check the value is in range
	if (data_value >= min_value && data_value <= max_value) {
		// It is - OK
		return VE_OK;
	}
	else {
		// It's not - out of range
		return VE_VALUE_OUT_OF_RANGE;
	}

}

// Check that an enumeration is valid
error_t spec_data::check_enumeration(const string& data, const string& field, const string& datatype) {
	// Check that the enumeration value is correct for the record. Note we have already checked if the data is a valid member of the enumeration
	// Specified by inference that it is a valid SOTA reference - check against the list of SOTA references
	if (field == "SOTA_REF") {
		if (datatype == "SOTARef") {
			// currently return not supported
			return VE_FIELD_UNSUPPORTED;
		}
		else {
			// Wrong datatype for the field
			return VE_VALUE_NOT_RECOMMENDED;
		}
	}
	// BAND/BAND_RX should agree with FREQ/FREQ_RX (or the latter is "")
	else if (field == "BAND" || field == "BAND_RX") {
		if (dataset(datatype)->data.find(data) != dataset(datatype)->data.end()) {
			// The band is in the dataset for the specified datatype (hopefully this is band!)
			// Get the fields for the entry for the band
			map<string, string>* fields = dataset(datatype)->data.at(data);
			// Get the frequency value from the record
			string frequency = (field == "BAND") ? record_->item("FREQ") : record_->item("FREQ_RX");
			if (frequency == "") {
				// No frequency in ADIF record - valid entry
				return VE_OK;
			}
			else {
				// Get the band edges - these must be coded so let it raise an exception if not
				string lower_edge = fields->at("Lower Freq (MHz)");
				string upper_edge = fields->at("Upper Freq (MHz)");
				double lower_value = stod(lower_edge);
				double upper_value = stod(upper_edge);
				size_t pos;
				double freq_value;
				try {
					freq_value = stod(frequency, &pos);
				} 
				catch (invalid_argument&) {
					// Frequency does not start with numeric - band cannot agree - invalid
					return VE_VALUE_INVALID;
				}
				if (pos != frequency.length()) {
					// Frequency field contained a non-numeric value
					return VE_VALUE_INVALID;
				}
				if (lower_value <= freq_value && upper_value >= freq_value) {
					return VE_OK;
				}
				else {
					// Frequency was outwith a valid frequency for the band
					return VE_VALUE_INCOMPATIBLE;
				}
			}
		}
		else {
			// An unknown band was specified
			return VE_VALUE_NOT_RECOMMENDED;
		}
	}
	else if (field == "CONT" || field == "MY_CONT") {
		// Check that the continent is correct for the DXCC.
		int dxcc_code = 0;
		if (field.length() == 7) {
			// MY_CONT
			if (record_->item("MY_DXCC").length())
				dxcc_code = stoi(record_->item("MY_DXCC"));
		}
		else {
			// CONT
			if (record_->item("DXCC").length())
				dxcc_code = stoi(record_->item("DXCC"));
		}
		if (dxcc_code) {
			// Get the prefix entry for the DXCC code
			prefix* prefix = pfx_data_->get_prefix(dxcc_code);
			for (unsigned int i = 0; i < prefix->continents_.size(); i++) {
				// The continent matches an entry in the list of continent in DXCC
				if (data == prefix->continents_[i]) return VE_OK;
			}
			// Not found in the list
			return VE_VALUE_INCOMPATIBLE;
		}
		else {
			// If DXCC field is blank or 0 (for a /MM station) it is not checkable
			return VE_VALUE_UNCHECKABLE;
		}
	}
	else {
		// No other enumerations to check
		return VE_OK;
	}
}

// Check the string has the right format if it needs it.
error_t spec_data::check_string(const string&  data, const string&  field, const string&  datatype)
{
	// ADIF_VER should to be x.y.z
	if (field == "ADIF_VER") {
		if (datatype == "String") {
			return check_format(data, field, datatype, REGEX_ADIF_VERSION);
		}
		else {
			return VE_VALUE_FORMAT_WARNING;
		}
	}
	// Created timestamp should be YYYYMMDD HHMMSS - so check that the two parts are valid
	else if (field == "CREATED_TIMESTAMP") {
		if (datatype == "String" && data.length() == 15) {
			// Check date portion
			error_t error = check_datatype(data.substr(0, 8), field, "Date", false);
			if (error == VE_OK) {
				// If date OK check time portion
				error = check_datatype(data.substr(9), field, "Time", false);
			}
			return error;
		}
		else {
			// Not valid format - not string or length <> 15
			return VE_VALUE_FORMAT_WARNING;
		}
	}
	// SUBMODE - is only recommended to be one of an enumerated list
	else if (field == "SUBMODE") {
		map<string, string>* fields;
		string upper_data = to_upper(data);
		if (dataset("Submode")->data.find(upper_data) != dataset("Submode")->data.end()) {
			// Submode has been found in Submode dataset
			// Check that it is a valid submode of the MODE field
			fields = dataset("Submode")->data.at(upper_data);
			// Get the Mode field from the submode entry and MODE item from the record
			string mode = fields->at("Mode");
			string record_mode = record_->item("MODE");
			if (mode == record_mode) {
				// They match - OK
				return VE_OK;
			}
			else {
				if (record_mode.length()) {
					// They don't - incompatible
					return VE_VALUE_INCOMPATIBLE;
				}
				else {
					return VE_VALUE_UNCHECKABLE;
				}
			}
		}
		else {
			// Use of a non-recommended submode name
			return VE_VALUE_NOT_RECOMMENDED;
		}
	}
	else if (field == "COUNTRY" || field == "MY_COUNTRY") {
		// Check that the country name is per ADIF
		// Get the DXCC Entity dataset
		spec_dataset* dxcc_set = dataset("DXCC_Entity_Code");
		// Look up either MY_DXCC or DXCC
		string dxcc_code = field.length() == 10 ?
			record_->item("MY_DXCC") :
			record_->item("DXCC");
		if (dxcc_set->data.find(dxcc_code) != dxcc_set->data.end()) {
			// The DXCC code has an entry in the dataset - gets its entry data
			map<string, string>* dxcc_data = dxcc_set->data.at(dxcc_code);
			// Get the entity name from the entry
			string entity_name = (*dxcc_data)["Entity Name"];
			if (data == to_upper(entity_name)) {
				// Matches the data - OK
				return VE_OK;
			}
			else {
				// Doesn't match - incompatible
				return VE_VALUE_INCOMPATIBLE;
			}
		}
		else {
			// DXCC is not in the list - either blank in which case how do we have a COUNTRY value
			if (data.length()) {
				if (dxcc_code.length()) {
					return VE_VALUE_INCOMPATIBLE;
				}
				else {
					return VE_VALUE_UNCHECKABLE;
				}
			}
			else {
				return VE_OK;
			}
		}
	}
	else {
		// Other fields, no special format requirements or comaptibility checks
		return VE_OK;
	}
}

// Check that QSO_DATE/TIME_ON is less than QSO_DATE_OFF/TIME_OFF
error_t spec_data::check_time(const string& data, const string& field) {
	if (field == "TIME_OFF") {
		if (record_->item_exists("QSO_DATE_OFF")) {
			time_t on_ts = record_->timestamp(false);
			time_t off_ts = record_->timestamp(true);
			if (difftime(off_ts, on_ts) < 0.0) {
				// Time off is earlier than time on
				return VE_VALUE_INCOMPATIBLE;
			}
			else {
				// OK
				return VE_OK;
			}
		}
		else {
			// If QSO_DATE_OFF is not specified then time off is assumed to be within 24 hours of time on
			return VE_OK;
		}
	}
	else {
		// Not time off so don't check
		return VE_OK;
	}
}

// Check the format is correct for the data type by matching against a regular expression
error_t spec_data::check_format(const string&  data, const string&  field, const string&  datatype, const basic_regex<char>& regex)
{
	if (regex_match(data.c_str(), regex)) {
		// Matches - OK
		return VE_OK;
	}
	else {
		// Format not correct
		return VE_VALUE_FORMAT_ERROR;
	}

}

// We have a separated list of datatype items - check each one in turn
error_t spec_data::check_list(const string& data, const string&  field, const string&  datatype, bool is_enumeration, char separator)
{
	error_t error = VE_OK;
	vector<string> items;
	// Split the list into an array
	split_line(data, items, separator);
	// For each item in the list, while they return OK
	for (unsigned int i = 0; i < items.size() && error == VE_OK; i++) {
		// Check it individually
		error = check_datatype(items[i], field, datatype, is_enumeration);
	}
	return error;
}

// Check the datatype - format, value etc.
error_t spec_data::check_datatype(const string&  data, const string&  field, const string&  datatype, bool is_enumeration)
{
	// Process enumerations - datatype contains the enumeration name.
	if (is_enumeration) {
		// Check in enumeration database - note for subdivisions the database is ....[DXCC]
		string enumeration_name;
		size_t pos_dxcc = datatype.find("[DXCC]");
		if (pos_dxcc != string::npos) {
			string dxcc_code_field;
			if (field == "STATE" || field == "CNTY") {
				dxcc_code_field = "DXCC";
			}
			else if (field == "MY_STATE" || field == "MY_CNTY") {
				dxcc_code_field = "MY_DXCC";
			}
			else {
				// datatype is a subdivision which is only allowed for STATE etc.
				return VE_FIELD_UNSUPPORTED;
			}
			// Replace "DXCC" with its value
			enumeration_name = datatype;
			enumeration_name.replace(pos_dxcc + 1, 4, record_->item(dxcc_code_field));
		}
		else {
			enumeration_name = datatype;
		}
		// get the dataset for the enumeration
		spec_dataset* enumeration_data = dataset(enumeration_name);
		// Special case for undefined Secondary_Administrative_Subdivision, Sponsored Award and any other enumerations not given - not able to check
		if (enumeration_data == nullptr) {
			return VE_FIELD_UNSUPPORTED;
		}
		// Get the enumeration entry for the particular value
		if (enumeration_data->data.find(data) != enumeration_data->data.end()) {
			map<string, string>* enumeration_record = enumeration_data->data.at(data);
			// Check it's not marked "Import Only" or deleted
			if (enumeration_record->find("Import-only") == enumeration_record->end()) {
				// Not Import Only
				if (enumeration_record->find("Deleted Date") != enumeration_record->end()) {
					string deleted_date = enumeration_record->at("Deleted Date");
					if (deleted_date != "") {
						// Deleted Date before QSO date. - outdated
						if (deleted_date.substr(0, 4) + deleted_date.substr(5, 2) + deleted_date.substr(8, 2) < record_->item("QSO_DATE")) {
							return VE_VALUE_OUTDATED;
						}
					}
					return check_enumeration(data, field, enumeration_name);
				}
				else {
					return check_enumeration(data, field, enumeration_name);
				}
			}
			else {
				// Enumeration value marked "Import-only"
				return VE_VALUE_INPUT_ONLY;
			}
		}
		else {
			// Enumeration value not listed
			return VE_VALUE_INVALID;
		}
	}
	// Test these data types in approximate order of frequency of occurrence
	else {
		// Get the Data Types data set.
		spec_dataset* datatypes_set = dataset("Data Types");
		// Get the datatype record
		if (datatypes_set->data.find(datatype) != datatypes_set->data.end()) {
			map<string, string>* datatype_record = datatypes_set->data.at(datatype);
			// First check that it's not import only
			string import_only = "";
			if (datatype_record->find("Import-only") != datatype_record->end()) {
				import_only = datatype_record->at("Import-only");
			};
			if (import_only == "") {
				// check those with a Data Type Indicator as most likely types
				string indicator = "";
				if (datatype_record->find("Data Type Indicator") != datatype_record->end()) {
					indicator = datatype_record->at("Data Type Indicator");
				}
				// If it has a data-type indicator then we can use a switch on the single char
				if (indicator.length() == 1) {
					error_t error;
					switch (indicator[0]) {
					case 'B': {
						// Boolean 
						return check_format(data, field, datatype, REGEX_BOOLEAN);
					}
					case 'N': {
						// Numeric
						error = check_format(data, field, datatype, REGEX_NUMERIC);
						if (error == VE_OK) {
							return check_number(data, field, datatype);
						}
						else {
							return error;
						}
					}
					case 'D': {
						// Date
						return check_format(data, field, datatype, REGEX_DATE);
					}
					case 'T': {
						// Time
						error = check_format(data, field, datatype, REGEX_TIME);
						if (error == VE_OK) {
							return check_time(data, field);
						}
						else {
							return error;
						}

					}
					case 'S': {
						// String (single-line, no intl characters)
						error_t error;
						error = check_format(data, field, datatype, REGEX_STRING);
						if (error != VE_OK && check_format(data, field, datatype, REGEX_BAD_MULTILINE) == VE_OK) {
							// It's not valid as a string but is as a multi-line string
							error = check_string(data, field, datatype);
							if (error == VE_OK) {
								// Only invalid as a multi-line string - check for format/value errors
								return VE_VALUE_MULTILINE;
							}
							else {
								// Other errors as well
								return error;
							}
						}
						if (error != VE_OK && check_format(data, field, datatype, REGEX_INTL_STRING) == VE_OK) {
							// It's not valid as a string but is as an international string
							error = check_string(data, field, datatype);
							if (error == VE_OK) {
								return VE_VALUE_INTL;
							}
							else {
								return error;
							}
						}
						if (error == VE_OK) {
							// Valid string - check for format/value errors
							return check_string(data, field, datatype);
						}
						else {
							return error;
						}
					}
					case 'M': {
						// Multi-line string - no special value checking
						error_t error;
						error = check_format(data, field, datatype, REGEX_MULTILINE);
						if (error != VE_OK) {
							if (check_format(data, field, datatype, REGEX_INTL_MULTILINE) == VE_OK) {
								return VE_VALUE_INTL;
							}
							else {
								return error;
							}
						}
						else return VE_OK;
					}
					case 'I': {
						// International string
						error_t error = check_format(data, field, datatype, REGEX_INTL_STRING);
						if (error != VE_OK && (check_format(data, field, datatype, REGEX_BAD_INTL_MULTILINE) == VE_OK)) {
							// It's not OK as an intl string but is as a multi-line one - check value/format
							error = check_string(data, field, datatype);
							if (error == VE_OK) {
								// OK as multi-line
								return VE_VALUE_MULTILINE;
							}
							else {
								// Value/format error
								return error;
							}
						}
						else {
							// Check value/format
							error = check_string(data, field, datatype);
							return error;
						}
					}
					case 'G': {
						// International multi-line string
						return check_format(data, field, datatype, REGEX_INTL_MULTILINE);
					}
					case 'L': {
						// Latitude or longitude
						return check_format(data, field, datatype, REGEX_LAT_LONG);
					}
					default:
						// Data type indicator not understood
						return VE_TYPE_UNKNOWN;
					}
				}
				// next the most likely no-indicator fields
				else  if (datatype == "PositiveInteger") {
					error_t error = check_format(data, field, datatype, REGEX_POS_INTEGER);
					if (error == VE_OK) {
						// Check it has a valid value for the field
						return check_integer(data, field, datatype);
					}
					else {
						// Not valid positive integer
						return error;
					}
				}
				else if (datatype == "GridSquare") {
					// Valid grid square format
					return check_format(data, field, datatype, REGEX_GRIDSQUARE);
				}
				else if (datatype == "Integer") {
					// Double-kill as check_integer also checks that it is an integer value
					error_t error = check_format(data, field, datatype, REGEX_INTEGER);
					if (error == VE_OK) {
						// Check value is valid
						return check_integer(data, field, datatype);
					}
					else {
						return error;
					}
				}
				else if (datatype == "IOTARefNo") {
					// Check regular expression and that continent is a valid enumeration value for Continent
					error_t error = check_format(data, field, datatype, REGEX_IOTA);
					if (error == VE_OK) {
						// OK - First 2 characters should be valid continent
						return check_datatype(data.substr(0, 2), field, "Continent", true);
					}
					else {
						// Not valid IOTA reference
						return error;
					}
				}
				// Followed by the rarer ones
				else if (datatype == "AwardList") {
					// Comma-separated list of Award
					return check_list(data, field, "Award", true, ',');
				}
				else if (datatype == "CreditList") {
					// Comma-separated list of CredtItem (not ADIF type but created to allow two level checking
					return check_list(data, field, "CreditItem", false, ',');
				}
				else if (datatype == "SponsoredAwardList") {
					// comma-separated list of Sposnsored_Award
					return check_list(data, field, "Sponsored_Award", true, ',');
				}
				else if (datatype == "Digit") {
					// Single digit
					return check_format(data, field, datatype, REGEX_DIGIT);
				}
				else if (datatype == "Character") {
					// Single character
					return check_format(data, field, datatype, REGEX_CHAR);
				}
				else if (datatype == "IntlCharacter") {
					// Single international character
					return check_format(data, field, datatype, REGEX_INTL_CHAR);
				}
				else if (datatype == "GridSquareList") {
					// comma-separated list of GridSquare
					return check_list(data, field, "GridSquare", false, ',');
				}
				else if (datatype == "SecondarySubdivisionList") {
					// colon-separated list of Secondary_Administrative_Subdivision
					return check_list(data, field, "Secondary_Administrative_Subdivision", true, ':');
				}
				else if (datatype == "SOTARef") {
					// a sequence of Character - defined in file summitslist.csv
					return check_format(data, field, datatype, REGEX_SOTA);
				}
				else {
					// Data type not recognised by the above code
					return VE_TYPE_UNKNOWN;
				}
			}
			else {
				// Data type indicates Import-ony
				return VE_FIELD_INPUT_ONLY;
			}
		}
		else if (datatype == "CreditItem") {
			// Not ADIF type. Either a single Credit or a Credit, ':', ampersand seperated list of QSL_Medium
			string credit_granted = "Credit";
			// find the colon
			size_t pos_colon = data.find(':');
			if (pos_colon == -1) {
				// No colon - just check valid Credit enumeration value.
				return check_datatype(data, field, "Credit", true);
			}
			else {
				// Colon, check the Credit value and the list of media.
				error_t error = check_datatype(data.substr(0, pos_colon), field, "Credit", true);
				if (error == VE_OK) {
					// Credit value OK, now check the list of QSL Media
					return check_list(data.substr(pos_colon + 1), field, "QSL_Medium", true, '&');
				}
				else {
					return error;
				}
			}
		}
		else {
			// Data type not recognised in the ADIF spec.
			return VE_TYPE_UNKNOWN;
		}
	}
}

// Validate a specific field
error_t spec_data::validate(const string& field, const string& data, bool inhibit_report /* = false */)
{
	// Temporarily inhibit error reporting 
	inhibit_error_report_ = inhibit_report;
	if (inhibit_report) {
		// Remember the record 
		saved_record_ = record_;
		record_ = book_->get_record();
	}
	error_t error = VE_TOP;
	string datatype = "Unknown";
	// Get the fields dataset
	spec_dataset* fields = dataset("Fields");
	// Get this field's entry
	auto it = fields->data.find(field);
	if (it != fields->data.end()) {
		map<string, string>* field_entry = it->second;
		if (data == "") {
			// Field has no content, so valid.
			error = VE_OK;
		}
		else {
			// Get the data type
			datatype = (*field_entry)["Data Type"];
			// Some fields can have more than one data type - so split the list
			vector<string> datatypes;
			split_line(datatype, datatypes, ',');
			// For each data type in the list - while it's still OK
			for (unsigned int i = 0; i < datatypes.size() && error != VE_OK; i++) {
				datatype = datatypes[i];
				if ((*field_entry)["Import Only"] != "") {
					// Data type is marked import only
					error = VE_FIELD_INPUT_ONLY;
				}
				else {
					// If it's an enumeration, use the enumeration name as the data type
					bool is_enumeration;
					if (datatype == "Enumeration" || datatype == "Dynamic" || datatype == "Macro") {
						is_enumeration = true;
						datatype = (*field_entry)["Enumeration"];
					}
					else {
						is_enumeration = false;
					}
					// check_datatype will cause handle_error to be called
					error = check_datatype(data, field, datatype, is_enumeration);
				}
			}
		}
	}
	else {
		// The field name is not valid - either as ADIF, USER or APP specified field name
		error = VE_FIELD_UNKNOWN;
	}
	if (error != VE_OK) {
		// Handle the error as per the settings
		handle_error(error, data, datatype, field);
	}
	// Restore error reporting 
	inhibit_error_report_ = false;
	// If not reporting do not save fixed record
	if (inhibit_report) {
		record_ = saved_record_;
	}
	return error;
}

// Handle the error
void spec_data::handle_error(error_t error_code, const string&  data, const string&  datatype, const string&  field)
{
	if (!inhibit_error_report_) {
		field_corrected_ = false;
		// Only handle the error if we are not reporting it
		// Get the display version of the data
		string display_item = record_->item(field, true);
		//// Get the validate settings
		//Fl_Preferences adif_settings(settings_, "ADIF");
		//Fl_Preferences validate_settings(adif_settings, "Validate");
		status_t status = ST_ERROR;
		// Implement
		// Report errors and try to correct, default to ask the user if not able to.
		report_error(error_code, display_item, datatype, field);
		if (!auto_correction(error_code, data, data, datatype, field)) {
			if (error_code != VE_VALUE_UNCHECKABLE && error_code != VE_FIELD_UNSUPPORTED && 
				error_code != VE_VALUE_INTL && ask_correction(field)) {
				record_corrected_ = true;
				field_corrected_ = true;
				status = ST_NOTE;
			}
			else if (error_code == VE_VALUE_INTL) {
				status = ST_WARNING;
			}
		}
		else {
			record_corrected_ = true;
			field_corrected_ = true;
			status = ST_NOTE;
		}
		report_correction(field, display_item, status);
	}
}

// Report error - write to report file
void spec_data::report_error(error_t error_code, const string&  data, const string&  datatype, const string&  field)
{
	status_t error_level = ST_SEVERE;
	string incompat_field;
	// Get field information for some reports - only if it's a valid field
	map<string, string>* field_info = nullptr;
	if (error_code != VE_FIELD_UNKNOWN) {
		field_info = dataset("Fields")->data.at(field);
	}
	string output_line = "VALIDATE: ";
	// Get common report: QSO 20171120 1430 GM3ZZA:
	output_line += report_timestamp(field, data);
	string other_value;

	// Get the specific error reports
	switch (error_code) {
	case VE_OK:
		// OK: For completeness 
		output_line += "OK!";
		error_level = ST_OK;
		break;
	case VE_TYPE_UNKNOWN:
		// Unknown data type
		output_line += "Data Type " + datatype + " is not valid.";
		error_level = ST_ERROR;
		break;
	case VE_FIELD_UNKNOWN:
		// Unknown field name
		output_line += "Not a valid ADIF field.";
		error_level = ST_ERROR;
		break;
	case VE_FIELD_INPUT_ONLY:
		// Marked Input Only
		output_line += "Field marked ""Input Only"".";
		error_level = ST_NOTE;
		break;
	case VE_FIELD_UNSUPPORTED:
		// Data type unsupported
		if (datatype == "Primary_Administrative_Subdivision" || datatype == "Secondary_Administrative_Subdivision") {
			output_line += "Field is not specified for DXCC " + record_->item("DXCC", true) + ". These are not checked.";
			error_level = ST_NOTE;
		}
		else {
			if (datatype.length()) {
				output_line += "Field is a " + datatype + ". These are not checked.";
			}
			else {
				output_line += "Field is an unknown datatype. These are not checked.";
			}
			error_level = ST_NOTE;
		}
		break;
	case VE_VALUE_INPUT_ONLY:
		// Enumeration value marked Input only
		output_line += "Value is marked ""Input Only"" for data type " + datatype + ".";
		error_level = ST_WARNING;
		break;
	case VE_VALUE_OUT_OF_RANGE:
		// Value is out of range for the field
		output_line += "Value is outside valid range for field (min=" + (*field_info)["Minimum Value"] +
			", max =" + (*field_info)["Maximum Value"] + ").";
		error_level = ST_ERROR;
		break;
	case VE_VALUE_FORMAT_ERROR:
		// Value has incorrect format for the field
		output_line += "Value has incorrect format for field.";
		error_level = ST_ERROR;
		break;
	case VE_VALUE_FORMAT_WARNING:
		// Value has on-recommended format
		output_line += "Value has an unrecommended format for field.";
		error_level = ST_WARNING;
		break;
	case VE_VALUE_INVALID:
		// Value is invalid
		output_line += "Value is invalid for field";
		error_level = ST_ERROR;
		break;
	case VE_VALUE_NOT_RECOMMENDED:
		// Value is not recommended
		output_line += "Value is not recommended for field.";
		error_level = ST_WARNING;
		break;
	case VE_VALUE_INCOMPATIBLE:
	case VE_VALUE_UNCHECKABLE:
		// Value is incompatible with that in another field (or other field is not available)
		// Get the other field name
		if (field == "SUBMODE") {
			incompat_field = "MODE";
		}
		else if (field == "BAND") {
			incompat_field = "FREQ";
		}
		else if (field == "BAND_RX") {
			incompat_field = "FREQ_RX";
		}
		else if (field == "CQZ" || field == "ITUZ" || field == "CONT" || field == "COUNTRY") {
			incompat_field = "DXCC";
		}
		else if (field == "MY_CQZ" || field == "MY_ITUZ" || field == "MY_CONT" || field == "MY_COUNTRY") {
			incompat_field = "MY_DXCC";
		} 
		else if (field == "TIME_OFF") {
			incompat_field = "TIME_ON";
		}
		other_value = record_->item(incompat_field, true);
		// Different report for the two cases
		if (error_code == VE_VALUE_INCOMPATIBLE) {
			output_line += "Value is incompatible with field " + incompat_field + "=" +
				record_->item(incompat_field, true);
			error_level = ST_ERROR;
		}
		else {
			output_line += "Value is specified. Related field " + incompat_field + " is not specified. This is not checked.";
			error_level = ST_NOTE;
		}
		break;
	case VE_VALUE_MULTILINE:
		// Value is supposed to be have no CR or LF characters
		output_line += "Value (data type " + datatype + ") - has CR or LF characters.";
		error_level = ST_WARNING;
		break;
	case VE_VALUE_INTL: {
		// Only invalid as it contains non-ASCII characters
		// Value is not supposed to have non-ASCII characters
		output_line += "Value (data type " + datatype + ") - has non-ASCII characters (ignored).";
		error_level = ST_LOG;
		break;
	}
	}
	// Write the error report
	status_->misc_status(error_level, output_line.c_str());
	// Save the error message for user correction.
	error_message_ = output_line;
}

// Auto-correct error returns true if successful
bool spec_data::auto_correction(error_t error_code, const string&  data, const string& display_item, const string&  datatype, const string&  field)
{
	switch (error_code) {
	case VE_OK:
		// No error to fix - so fix "successful"
		correction_message_ = "";
		return true;
		break;
	case VE_FIELD_UNSUPPORTED:
		// No information to fix
		correction_message_ = "";
		return false;
		break;
	case VE_TYPE_UNKNOWN:
	case VE_FIELD_UNKNOWN:
		// Not correctable
		return false;
		break;
	case VE_VALUE_OUT_OF_RANGE:
	case VE_VALUE_FORMAT_WARNING:
	case VE_VALUE_FORMAT_ERROR:
	// Try again without trailing spaces
	{
		size_t pos = data.find_last_not_of(' ');
		if (pos != string::npos && pos != data.length()) {
			string no_trail = data.substr(0, pos + 1);
			if (!check_datatype(no_trail, field, datatype, dataset(datatype) != nullptr)) {
				record_->item(field, no_trail);
				correction_message_ = field + '=' + display_item + " auto-corrected by removal of trailing spaces.";
				return true;
			}
		}
	}
	break;
	case VE_VALUE_NOT_RECOMMENDED:
		// Try and correct SUBMODE
		if (field == "SUBMODE") {
			string mode = record_->item("MODE");
			if (mode == display_item) {
				// MODE and SUBMODE set to the same - remove SUBMODE
				record_->item("SUBMODE", string(""));
				correction_message_ = field + "=" + display_item + " removed as MODE=" + display_item + " already set.";
				return true;
			}
			else if (mode == "") {
				spec_dataset* mode_set = dataset("Mode");
				// Is this submode really a mode - change it to MODE=<value>
				if (mode_set->data.find(display_item) != mode_set->data.end()) {
					record_->item("SUBMODE", string(""));
					record_->item("MODE", data);
					correction_message_ = field + "=" + display_item + " auto-corrected to MODE=" + display_item + ".";
					return true;
 				}
			}
		}
	case VE_VALUE_INVALID:
		// Try again without trailing spaces
		{
			size_t pos = data.find_last_not_of(' ');
			if (pos != string::npos && pos != data.length()) {
				string no_trail = data.substr(0, pos);
				if (!check_datatype(no_trail, field, datatype, dataset(datatype) != nullptr)) {
					record_->item(field, no_trail);
					correction_message_ = field + '=' + display_item + " auto-corrected by removal of trailing spaces.";
					return true;
				}
			}
			// Special case - some Sub-modes are not Import-only in the Mode list
			if (field == "MODE") {
				spec_dataset* submode_set = dataset("Submode");
				auto it = submode_set->data.find(data);
				if (it != submode_set->data.end()) {
					string mode = it->second->at("Mode");
					record_->item("SUBMODE", data);
					record_->item("MODE", mode);
					correction_message_ = "MODE=" + data + " auto-corrected to MODE=" + mode +
						", SUBMODE=" + data + ".";
					return true;
				}
			}
		}
		break;
	case VE_VALUE_MULTILINE:
		// Replace CR/LF intelligently
		record_->item(field, convert_ml_string(data));
		correction_message_ = field + "=" + display_item + " auto-corrected to " +
			field + "=" + record_->item(field, true) + ".";
		return true;
		break;
	case VE_VALUE_INTL: 
		// Get the validate settings
		correction_message_ = field + '=' + display_item + " non-ASCII characters ignored.";
		return false;
	case VE_FIELD_INPUT_ONLY:
		// Correct Import-only fields and data types if possible 
		if (datatype == "Award" || datatype == "AwardList") {
			// Award enumeration not correctable
			return false;
		}
		if (field == "GUEST_OP") {
			// replaced by operator
			record_->change_field_name("GUEST_OP", "OPERATOR");
			correction_message_ = "GUEST_OP=" + field + " auto-corrected to OPERATOR=" + display_item + ".";
			return true;
		}
		if (field == "VE_PROV") {
			// Canadian province replace by State (Primary administrative area)
			record_->change_field_name("VE_PROV", "STATE");
			correction_message_ = "VE_PROV=" + field + " auto-corrected to STATE=" + display_item + ".";
			return true;
		}
		break;
	case VE_VALUE_INPUT_ONLY:
		// Correct Import-only enumeration values
		if (field == "MODE") {
			// Look in Submode type and if present change field name to SUBMODE and set MODE to process_mode specified in submode
			spec_dataset* submode_set = dataset("Submode");
			if (submode_set->data.find(data) != submode_set->data.end()) {
				map<string, string>* submode_data = submode_set->data.at(data);
				string mode = (*submode_data)["Mode"];
				record_->change_field_name("MODE", "SUBMODE");
				record_->item("MODE", mode);
				correction_message_ = "MODE=" + data + " auto-corrected to MODE=" + mode +
					", SUBMODE=" + data + ".";
				return true;
			}
		}
		if (field == "QSL_RCVD_VIA" || field == "QSL_SENT_VIA") {
			// Value M no longer supported - no alternate value
			if (data == "M") {
				record_->item(field, string(""));
				correction_message_ = field + "=M removed" + ".";
				return true;
			}
		}
		if (field == "QSL_RCVD" && data == "V") {
			// Value V no longer supported - changed to CREDIT_GRANTED
			string credit_granted = record_->item("CREDIT_GRANTED");
			const string new_credit = "DXCC:card,DXCC_BAND:card,DXCC_MODE:card";
			if (credit_granted == "") {
				record_->item("CREDIT_GRANTED", new_credit);
			}
			else {
				record_->item("CREDIT_GRANTED", credit_granted + "," + new_credit);
			}
			correction_message_ = field + "=V auto-corrected to CREDIT_GRANTED=" + new_credit + ".";
			return true;
		}
		if (field == "LOTW_QSL_RCVD" && data == "V") {
			// Value V no longer supported - changed to CREDIT_GRANTED
			string credit_granted = record_->item("CREDIT_GRANTED");
			const string new_credit = "DXCC:card,DXCC_BAND:card,DXCC_MODE:card";
			if (credit_granted == "") {
				record_->item("CREDIT_GRANTED", new_credit);
			}
			else {
				record_->item("CREDIT_GRANTED", credit_granted + "," + new_credit);
			}
			correction_message_ = field + "=V auto-corrected to CREDIT_GRANTED=" + new_credit + ".";
			return true;
		}
		if (field == "EQSL_QSL_RCVD" && data == "V") {
			// Value V no longer supported - changed to CREDIT_GRANTED
			string credit_granted = record_->item("CREDIT_GRANTED");
			const string new_credit = "EQSL:eqsl,EQSL_BAND:eqsl,EQSL_MODE:eqsl";
			if (credit_granted == "") {
				record_->item("CREDIT_GRANTED", new_credit);
			}
			else {
				record_->item("CREDIT_GRANTED", credit_granted + "," + new_credit);
			}
			correction_message_ = field + "=V auto-corrected to CREDIT_GRANTED=" + new_credit + ".";
			return true;
		}
		if (field == "STATE") {
			// Certain state codings have changed
			int dxcc_code;
			record_->item("DXCC", dxcc_code);
			switch (dxcc_code) {
			case 61:
				// Franz Iosef Land 
				if (data == "FJL") {
					// Change to Arkhangelsk Oblast
					record_->item(field, string("AR"));
					correction_message_ = "Franz Josef Land: STATE=FJL auto-corrected to STATE=AR";
					return true;
				}
				break;
			case 151:
				// Malyj Vysotski
				if (data == "MV") {
					// Change to Leningradski Oblast
					record_->item(field, string("LO"));
					correction_message_ = "Malyy Vysotski: STATE=MV auto-corrected to STATE=LO";
					return true;
				}
				break;
			case 225:
				// Sardinia
				if (data == "MD") {
					// Medio Capidano - change ID
					record_->item(field, string("VS"));
					correction_message_ = "Sardinia: STATE=MD auto-corrected to STATE=VS";
					return true;
				}
				break;
			case 248:
				// Italy
				if (data == "FO") {
					// Forli - change name and ID to Forli-Cesena (FC)
					record_->item(field, string("FC"));
					correction_message_ = "Italy: STATE=FO auto-corrected to STATE=FC";
					return true;
				}
				else if (data == "PS") {
					// Pesaro e Urbino - change ID
					record_->item(field, string("PU"));
					correction_message_ = "Italy: STATE=PS auto-corrected to STATE=PU";
					return true;
				}
				break;
			}
		}
		if (field == "CONTEST_ID") {
			if (data == "EA-RTTY" || data == "RAC") {
				// Contest removedfrom list
				record_->item(field, string(""), true);
				correction_message_ = field + "=" + display_item + " removed.";
				return true;
			}
			else if (data == "URE-DX") {
				const string new_item = "UKRAINIAN DX";
				record_->item(field, new_item);
				correction_message_ = field + "=" + display_item + " auto-corrected to " + field + "=" + new_item + ".";
				return true;
			}
			else if (data == "Virginia QSO Party") {
				const string new_item = "VA-QSO-PARTY";
				record_->item(field, new_item);
				correction_message_ = field + "=" + display_item + " auto-corrected to " + field + "=" + new_item + ".";
				return true;
			}
		}
		break;
	case VE_VALUE_INCOMPATIBLE:
		if (field == "BAND") {
			// These fields should be reparsed
			const string frequency = record_->item("FREQ");
			record_->item("BAND", string(""));
			record_->update_band();
			correction_message_ = field + "=" + display_item + " auto-corrected to " +
				field + "=" + record_->item(field, true) + ".";
			return true;
		}
		else if (field == "CQZ" || field == "ITUZ" || field == "CONT" || field == "COUNTRY" ||
			field == "MY_CQZ" || field == "MY_ITUZ" || field == "MY_CONT" || field == "MY_COUNTRY") {
			// These fields should be reparsed - if we can
			if (record_->item("DXCC").length()) {
				record_->item(field, string(""));
				pfx_data_->parse(record_);
				correction_message_ = field + "=" + display_item + " auto-corrected to " +
					field + "=" + record_->item(field, true) + ".";
				return true;
			}
			else {
				// Sort of compatible as there is nothing to be compatible with
				return false;
			}
		}
		else if (field == "TIME_OFF") {
			// Test to see if TIME_OFF is an hour out.
			chrono::system_clock::time_point time_off = chrono::system_clock::from_time_t(record_->timestamp(true));
			chrono::duration<int, ratio<1> > one_hour(3600);
			time_t time_on = record_->timestamp(false);
			time_t new_off = chrono::system_clock::to_time_t(time_off + one_hour);
			if (difftime(new_off, time_on) > 0.0) {
				// Convert to date YYYYMMDD and time HHMMSS and update record
				char temp[10];
				strftime(temp, sizeof(temp), "%Y%m%d", gmtime(&new_off));
				record_->item("QSO_DATE_OFF", string(temp));
				//Fl_Preferences log_settings(settings_, "Log");
				strftime(temp, sizeof(temp), "%H%M%S", gmtime(&new_off));
				record_->item("TIME_OFF", string(temp));
				correction_message_ = field + "=" + display_item + "auto-corrected to " +
					field + "=" + record_->item(field, true) + ".";
				return true;
			}
			else {
				return false;
			}
		}
		break;
	case VE_VALUE_OUTDATED:
		if (field == "ARRL SECT") {
			// ARRL Section
			if (data == "NWT") {
				// Change of ID for North West Territories
				record_->item("ARRL_SECT", string("NT"));
				correction_message_ = "ARRL_SECT=NWT auto-corrected to ARRL_SECT=NT";
				return true;
			}
		}
		break;
	case VE_VALUE_UNCHECKABLE:
		correction_message_ = "";
		return false;
	}
	return false;
}

// User-correct error
bool spec_data::ask_correction(const string& field)
{
	// Tell app to display the current record
	if (record_number_ != -1) {
		book_->selection(record_number_, HT_SELECTED);
	}
	// Create User correction dialog
	corr_dialog* dialog = new corr_dialog(record_, field, error_message_);
	switch (dialog->display()) {
	case BN_OK:
		// User corrected the field - get what they did
		correction_message_ = dialog->correction_message();
		return true;
	case BN_CANCEL:
		// User did not correct the field
		correction_message_ = "User did not correct data.";
		return false;
	case BN_SPARE:
		// User did not correct field and wants to cancel whole validate
		correction_message_ = "User has quit all validation for this record or log.";
		abandon_validation_ = true;
		return false;;
	}
	return false;
}

// Report correction
void spec_data::report_correction(const string&  field, const string&  data, status_t status)
{
	if (correction_message_.length() != 0) {
		// Build report text and write it to report
		status_->misc_status(status, ("VALIDATE: " + report_timestamp(field, data) + correction_message_).c_str());
	}

}

// validate a record 
bool spec_data::validate(record* record, record_num_t number)
{
	// Check configured to do it at this stage
	abandon_validation_ = false;
	record_corrected_ = false;
	record_number_ = number;
	// Get and save the record pointer for use in various checks, repairs and reports
	record_ = record;
	bool error = false;
	// Validate all fields in record.
	for (auto it = record_->begin(); it != record_->end() && !abandon_validation_; it++) {
		string field = it->first;
		string data = it->second;
		if (validate(field, data) != VE_OK) {
			// Increment field error count
			error_count_++;
			error = true;
		}
	}
	if (error) {
		// Increment record error count
		error_record_count_++;
	}
	// Increment record count
	record_count_++;
	return record_corrected_;
}


// Remove CR and LF from a string to convert multi-line to a non-multi-line string
string spec_data::convert_ml_string(const string& data) {
	// Create a string to return and pre-allocate a buffer big enough.
	// It should be smaller than the provided data
	string return_data;
	return_data.reserve(data.capacity() );
	bool line_break = false;
	for (unsigned int i = 0; i < data.length(); i++) {
		char c = data[i];
		switch (c) {
		case '\n':
		case '\r':
			// Replace first of consecutive occurences of CR or LF with a space
			if (!line_break) {
				line_break = true;
				return_data += ' ';
			}
			break;
		default:
			// Copy character - turn off that we have seen at least 1 CR or LF
			line_break = false;
			return_data += c;
			break;
		}
	}
	return return_data;
}

// Generate a timestamp of the QSO for the report listing - QSO <date> <time> <call> <field> (<data>)
string spec_data::report_timestamp(string field, string data) {
	string timestamp = "QSO " +
		record_->item("QSO_DATE") + " " +
		record_->item("TIME_ON").substr(0, 4) + " " +
		record_->item("CALL") + ": Field " +
		field + " (" + data + ") ";
	return timestamp;
}

// Set the loaded file name and write a header to the report
void spec_data::loaded_filename(string value) {
	loaded_filename_ = value;
}

// Provide tip for the field and data
string spec_data::get_tip(const string& field, record* record) {
	// Get the Fields dataset
	spec_dataset* fields = dataset("Fields");
	if (fields != nullptr) {
		// If the field is valid - get the entry for the field
		map<string, string>* field_data = fields->data[field];
		// Start building the tip string
		string tip = "Field: " + field + "\n\n";
		string text;
		string data = record->item(field);
		char line[1024];
		// Entry description
		text = (*field_data)["Description"];
		sprintf(line, "Description: %s\n\n", text.c_str());
		tip += line;
		// Entry data type
		text = (*field_data)["Data Type"];
		sprintf(line, "Data Type: %s\n", text.c_str());
		tip += line;
		if (text == "Enumeration") {
			// Enumeration name
			string enumeration = enumeration_name(field, record).c_str();
			spec_dataset* dataset = this->dataset(enumeration);
			tip += describe_enumeration(dataset, data);
		}
		// now check if contents valid
		switch(validate(field, data, true)) {
		case VE_OK:
			tip += "Contents Valid";
			break;
		case VE_TYPE_UNKNOWN:               // Datatype or enumeration type not known
			tip += "Unknown data type or enumeration";
			break;
		case VE_FIELD_UNKNOWN:              // Field name is not known
			tip += "Unknown field";
			break;
		case VE_FIELD_INPUT_ONLY:           // Field is import only - to be modified
			tip += "Deprecated field";
			break;
		case VE_FIELD_UNSUPPORTED:          // Field cannot yet be validated
			tip += "No data to validate field";
			break;
		case VE_VALUE_INPUT_ONLY: // value is Import Only - to be modified
			tip += "Deprecated value";
			break;
		case VE_VALUE_OUT_OF_RANGE:        // value is < minimum or > maximum values
			tip += "Value is out of range";
			break;
		case VE_VALUE_FORMAT_ERROR:         // value is not formatted correctly for data type
			tip += "Value has incorrect format";
			break;
		case VE_VALUE_FORMAT_WARNING:       // string value does not have the recommended format
			tip += "Value does not have recommended format";
			break;
		case VE_VALUE_INVALID:              // enumeration value is not valid
			tip += "Value is not valid for the enumeration";
			break;
		case VE_VALUE_NOT_RECOMMENDED:      // string value not recommended for interoperability
			tip += "Value is not recommended for interoperability";
			break;
		case VE_VALUE_INCOMPATIBLE:         // string value is incompatible with another field
			tip += "Value is incompatible with another field";
			break;
		case VE_VALUE_OUTDATED:             // string value has been removed from valid list
			tip += "Value is no longer valid";
			break;
		case VE_VALUE_MULTILINE:            // string value includes \n and \r when not multiline
			tip += "Value has invalid line breaks";
			break;
		case VE_VALUE_INTL:                 // string value includes non-ASCII characters
			tip += "Value has invalid non-ASCII characters";
			break;
		case VE_VALUE_UNCHECKABLE:          // string value cannot be checked for compatibility
			tip += "No data to check compatibility with other fields";
			break;
		}
		tip += '\n';

		return tip;
	}
	else {
		return "";
	}

}

// Provide tip for just the field
string spec_data::get_tip(const string& field) {
	// Get the Fields dataset
	spec_dataset* fields = dataset("Fields");
	if (fields != nullptr) {
		// Build the tip up.
		string tip = "Field: " + field + "\n";
		map<string, string>* field_data = fields->data[field];
		// For every item in the entry for the field
		for (unsigned int i = 0; i < fields->column_names.size(); i++) {
			string& column = fields->column_names[i];
			string text = (*field_data)[column];
			char* line = new char[column.length() + text.length() + 10];
			// Add the line to the tip if value is not empty and the item name doesn't start with ADIF
			if (text != "" && column.substr(0,4) != "ADIF") {
				sprintf(line, "%s: %s\n", column.c_str(), text.c_str());
				tip += line;
			}
			delete[] line;
		}
		return tip;
	}
	else {
		return "";
	}
}

// User has not abandoned 
bool spec_data::do_continue() {
	return !abandon_validation_;
}

// Reset continue flag
void spec_data::reset_continue() {
	abandon_validation_ = false;
}

// Add this app's app defined fields, plus a couple of LOTW ones
void spec_data::add_my_appdefs() {
	string my_appdefs[] = {
		"APP_ZZA_PFX",
		"APP_ZZA_QTH",
		"APP_ZZA_QTH_DESCR",
		"APP_ZZA_EQSL_MSG",
		"APP_ZZA_ERROR",
		"APP_LOTW_NUMREC",
		"APP_LOTW_LASTQSL",
		""
	};
	int i = 0;
	while (my_appdefs[i].length()) {
		add_appdef(my_appdefs[i], 'S');
		i++;
	}

}

// Combine mode and submode into single dataset
void spec_data::process_modes() {
	spec_dataset* modes = dataset("Mode");
	spec_dataset* submodes = dataset("Submode");
	spec_dataset* combined = new spec_dataset;
	combined->column_names = { "Is Mode", "Mode" };
	for (auto it = modes->data.begin(); it != modes->data.end(); it++) {
		if (it->second->find("Input Only") == it->second->end()) {
			map<string, string>* data = new map<string, string>;
			(*data)[string("Is Mode")] = "Yes";
			(*data)[string("Mode")] = it->first;
			combined->data[it->first] = data;
		}
	}
	for (auto it = submodes->data.begin(); it != submodes->data.end(); it++) {
		if (it->second->find("Input Only") == it->second->end()) {
			map<string, string>* data = new map<string, string>;
			(*data)[string("Is Mode")] = "No";
			(*data)[string("Mode")] = it->second->at("Mode");
			combined->data[it->first] = data;
		}
	}
	(*this)["Combined"] = combined;
}

// Return a string that describes the enumeration and the meaning of the given value
string spec_data::describe_enumeration(spec_dataset* dataset, string value) {
	// Get the selected explanation - default to Not Available
	string enum_text = "";
	if (dataset) {
		auto items = dataset->data.find(value);
		if (items != dataset->data.end()) {
			// For each item in the dataset explanation for this enumeratiom
			for (auto item = (items->second)->begin(); item != (items->second)->end(); item++) {
				if (item->first != "Enumeration Name" &&
					item->first != "Import-only" &&
					item->first != "Comments" &&
					item->first != "ADIF Version" &&
					item->first != "ADIF Status" &&
					item->second != "") {
					// Add salient information to the explanation
					char line[256];
					if (item->first == "DXCC Entity Code") {
						int dxcc = stoi(item->second);
						string dxcc_name = entity_name(dxcc);
						snprintf(line, 256, "%s: %s (%s)\n", item->first.c_str(), item->second.c_str(), dxcc_name.c_str());
					}
					else {
						snprintf(line, 256, "%s: %s\n", item->first.c_str(), item->second.c_str());
					}
					enum_text += string(line);
				}
			}
		}
		enum_text += "\n";
	}
	if (enum_text == "") {
		// There is no explanation
		enum_text = "No data available\n";
	}
	return enum_text;
}

// Return the entity name for the DXCC ADIF code
string spec_data::entity_name(int dxcc) {
	string result;
	spec_dataset* dxccs = dataset("DXCC_Entity_Code");
	map<string, string>* entry = dxccs->data[to_string(dxcc)];
	return (*entry)["Entity Name"];
}
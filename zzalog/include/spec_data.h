#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <regex>
#include <fstream>
#include <ostream>

#include <FL/Fl_Choice.H>

using namespace std;

class band_set;
class record;
typedef size_t qso_num_t;

	// validation status for each check
	enum valn_error_t {
		VE_OK,                         // No problem
		VE_TYPE_UNKNOWN,               // Datatype or enumeration type not known
		VE_FIELD_UNKNOWN,              // Field name is not known
		VE_FIELD_INPUT_ONLY,           // Field is import only - to be modified
		VE_FIELD_UNSUPPORTED,          // Field cannot yet be validated
		VE_VALUE_INPUT_ONLY,           // value is Import Only - to be modified
		VE_VALUE_OUT_OF_RANGE,         // value is < minimum or > maximum values
		VE_VALUE_FORMAT_ERROR,         // value is not formatted correctly for data type
		VE_VALUE_FORMAT_WARNING,       // string value does not have the recommended format
		VE_VALUE_INVALID,              // enumeration value is not valid 
		VE_VALUE_NOT_RECOMMENDED,      // string value not recommended for interoperability
		VE_VALUE_INCOMPATIBLE,         // string value is incompatible with another field
		VE_VALUE_OUTDATED,             // string value has been removed from valid list
		VE_VALUE_MULTILINE,            // string value includes \n and \r when not multiline
		VE_VALUE_INTL,                 // string value includes non-ASCII characters
		VE_VALUE_UNCHECKABLE,          // string value cannot be checked for compatibility
		VE_TOP                         // last element in enumerated type
	};

	// Reference dataset - data structure
	struct spec_dataset {
		// Names of the columns in the data set
		vector<string> column_names;
		// The data records - map from first data item to a map of column names to other data items
		map<string, map<string, string>* > data;
		spec_dataset() {
			column_names.clear();
			data.clear();
		}
	};

	// Macro definition set - macro name versions set of fields it represents
	struct macro_defn {
		record* fields{ nullptr };
		string description;
	};
	typedef map<string, macro_defn*> macro_map;

	enum status_t : char;

	// This class provides the ADIF specification reference database as a set of named datasets. 
	// It provides the access to the database and methods to validate the ADIF data against the specification
	// The database is a map of the dataset name to a dataset
	class spec_data : public map<string, spec_dataset*>
	{
	public:
		spec_data();
		~spec_data();

		// public methods
	public:
		// load the data
		bool load_data(bool force);
		// Get the DXCC award mode for a particulat ADIF mode
		string dxcc_mode(string mode);
		// Get the ADIF mode for a submode
		string mode_for_submode(string submode);
		// Get the band for a specific frequency
		string band_for_freq(double frequency_MHz);
		// Get the Lower frequency for a band
		double freq_for_band(string sBand);
		// Get the upper and lower frequencies for band
		void freq_for_band(string band, double& lower, double& upper);
		// Get mode/submode for a particular mode
		bool is_submode(string mode);
		// Get DataSet
		spec_dataset* dataset(string name);
		// Get Adif Vesrions
		string adif_version();
		// Get sorted list of field names
		set<string>* sorted_fieldnames();
		// Add user defined fields - returns TRUE if a new one.
		bool add_userdef(int id, const string& name, char indicator, string& values);
		// Get data type indicator
		char datatype_indicator(string& field_name);
		// Get list or range
		string userdef_values(string& field_name);
		// Initialise this app's app-specific fieldnames
		void add_my_appdefs();
		// Remove existing user defined fields
		void delete_userdefs();
		void delete_appdefs();
		// Return true if field is a user defined one.
		bool is_userdef(string field_name);
		// Get data type from indicator
		string datatype(char indicator);
		// Get data type from field name
		string datatype(string field_name);
		// Get enumeration name
		string enumeration_name(const string& field, record* record);
		// The DXCC has ADIF defined primary administrative districts
		bool has_states(int dxcc);
		// Validate a record - returns TRUE if record corrected
		bool validate(record* record, qso_num_t number);
		// Validate the data in the field
		valn_error_t validate(const string&  field_name, const string& data, bool inhibit_report = false);
		// Set the loaded filename
		void loaded_filename(string value);
		// Get tip for the field and data
		string get_tip(const string& field, record* record);
		// Get tip for just the field
		string get_tip(const string& field);
		// User wants to continue
		bool do_continue();
		// Reset continue flag
		void reset_continue();
		// Add application defined field
		bool add_appdef(const string& name, char indicator);
		// Generate the description of the enumeration for the designated enumerated field
		string describe_enumeration(spec_dataset* dataset, string value);
		// Generate the summary of the enumeration value
		string summarise_enumaration(string name, string value);
		// Get entity name for DXCC Number
		string entity_name(int dxcc);
		// Add user defined enumeration for an existing field - return true if successful
		bool add_user_enum(string field, string value);
		// Add app defined macro (use one field "APP_ZZA..." to represent several others)
		bool add_user_macro(string field, string name, macro_defn macro);
		// Get macro changes
		set<string> get_macro_changes();
		// Get the record fields for a mmacro 
		record* expand_macro(string field, string value);
		// Remove user enums and macros - and restore originals
		void delete_user_data();
		// REturn list of bands in frequency order
		band_set* bands();
		// Create a list of bands in frequency order
		void process_bands();


	// protected methods
	protected:
		string get_path(bool force);
		// Sort filed names
		void process_fieldnames();
		// Combine mode and submode into single dataset
		void process_modes();
		// Check the data is the correct format for the field either against a regex for specific datatypes
		valn_error_t check_format(const string&  data, const string&  field, const string&  datatype, const basic_regex<char>& pattern);
		// Check that data is in the correct value range
		valn_error_t check_string(const string&  data, const string&  field, const string&  datatype);
		// Check the data is between to non-integer minimum and maximum values
		valn_error_t check_number(const string&  data, const string&  field, const string&  datatype);
		// Check the data is between two integer minimum and maximum values
		valn_error_t check_integer(const string&  data, const string&  field, const string&  datatype);
		// Check that an enumeration is valid
		valn_error_t check_enumeration(const string& data, const string& field, const string& datatype);
		// Check that the data is a separated list of the specified datatype/enumeration
		valn_error_t check_list(const string&  data, const string&  field, const string&  datatype, bool bIsEnumeration, char cSeparator);
		// Check that the data is the specified datatype/enumeration
		valn_error_t check_datatype(const string&  data, const string&  field, const string&  datatype, bool bIsEnumeration);
		// Check that time is valid
		valn_error_t check_time(const string& data, const string& field);
		// Handle a validation error
		void handle_error(valn_error_t error_code, const string&  data, const string&  datatype, const string&  field);
		// Report validation error
		void report_error(valn_error_t error_code, const string&  data, const string&  datatype, const string&  field);
		// Auto-correct error returns TRUE if successful
		bool auto_correction(valn_error_t error_code, const string&  data, const string& sDisplayData, const string&  datatype, const string&  field);
		// User-correct error 
		bool ask_correction(const string& field);
		// Report correction
		void report_correction(const string&  field, const string&  data, status_t status);
		// Remove CR and LF characters
		string convert_ml_string(const string& data);
		// Generate report timestamp and fault
		string report_timestamp(string field, string data);
		// protected attributes
	protected:
		// ADIF Version
		string adif_version_;
		// List of Fieldnames
		set<string> field_names_;
		// Array of user defs
		vector<string> userdef_names_;
		// List of app defs (not in reference)
		set<string> appdef_names_;
		// List of user defined enumerations
		set<string> user_enums_;
		// Quick lookup of datatype indicator
		map<string, char> datatype_indicators_;
		// Macro definitions
		map<string, macro_map*> macros_;
		// Fields that change in the last evocation of add_user_macro
		set<string> macro_changes_;
		// Missing file already reported
		bool error_reported_;
		// Correction message
		string correction_message_;
		// User or auto corrected
		bool field_corrected_;
		// Error message
		string error_message_;
		// Record
		record* record_;
		// Saved record
		record* saved_record_;
		// Record number
		qso_num_t record_number_;
		// Number of validation errors
		int error_count_;
		int error_record_count_;
		int record_count_;
		bool record_corrected_;
		// Inhibit report
		bool inhibit_error_report_;
		// Loaded filename
		string loaded_filename_;
		// Abandon validate 
		bool abandon_validation_;
		// List of bands in frrequency order
		band_set* bands_;

	};

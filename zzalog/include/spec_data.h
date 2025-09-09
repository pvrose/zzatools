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

	//! validation status for each check
	enum valn_error_t {
		VE_OK,                         //!< No problem
		VE_TYPE_UNKNOWN,               //!< Datatype or enumeration type not known
		VE_FIELD_UNKNOWN,              //!< Field name is not known
		VE_FIELD_INPUT_ONLY,           //!< Field is import only - should be changed for export
		VE_FIELD_UNSUPPORTED,          //!< Field cannot yet be validated
		VE_VALUE_INPUT_ONLY,           //!< value is Import Only - should be changed for export
		VE_VALUE_OUT_OF_RANGE,         //!< value is < minimum or > maximum values
		VE_VALUE_FORMAT_ERROR,         //!< value is not formatted correctly for data type
		VE_VALUE_FORMAT_WARNING,       //!< string value does not have the recommended format
		VE_VALUE_INVALID,              //!< enumeration value is not valid 
		VE_VALUE_NOT_RECOMMENDED,      //!< string value not recommended for interoperability
		VE_VALUE_INCOMPATIBLE,         //!< string value is incompatible with another field
		VE_VALUE_OUTDATED,             //!< string value has been removed from valid list
		VE_VALUE_MULTILINE,            //!< string value includes NL and CR when not multiline
		VE_VALUE_INTL,                 //!< string value includes non-ASCII characters
		VE_VALUE_UNCHECKABLE,          //!< string value cannot be checked for compatibility
		VE_TOP                         //!< last element in enumerated type
	};

	//! Reference dataset - data structure
	struct spec_dataset {
		//! Names of the columns in the data set
		vector<string> column_names;
		//! The data records - map from first data item to a map of column names to other data items
		map<string, map<string, string>* > data;
		//! Constructor.
		spec_dataset() {
			column_names.clear();
			data.clear();
		}
	};

	enum status_t : char;

	//! This class provides the ADIF specification reference database as a set of named datasets. 
	
	//! It provides the access to the database and methods to validate the ADIF data against the specification
	//! The database is a map of the dataset name to a dataset
	class spec_data : public map<string, spec_dataset*>
	{
	public:
		//! Constructor.
		spec_data();
		//! Destructor.
		~spec_data();

		// public methods
	public:
		//! load the data from "all.xml"
		bool load_data();
		//! Returns the DXCC award mode for a particulat ADIF \p mode.
		string dxcc_mode(string mode);
		//! Returns the ADIF mode for a \p submode
		string mode_for_submode(string submode);
		//! Returns the band for a specific \p frequency (in megahertz)
		string band_for_freq(double frequency_MHz);
		//! Returns the Lower frequency for a band \p sBand
		double freq_for_band(string sBand);
		//! Receives the \p upper and \p lower frequencies for \p band
		void freq_for_band(string band, double& lower, double& upper);
		//! Returns true if \p mode is a submode of another mode.
		bool is_submode(string mode);
		//! Returns the spec_datasset named \p name.
		spec_dataset* dataset(string name);
		//! Returns the ADIF version coded in the file.
		string adif_version();
		//! Returns sorted list of field names
		set<string>* sorted_fieldnames();
		//! Add user defined fields 
		
		//! \param id The index - i.e. USERDEF<I>n</I>.
		//! \param name The name of the user defined field
		//! \param indicator the single character showing data type of the field
		//! \param values Specifies a valid value range for  the field.
		//! \return true if successful.
		bool add_userdef(int id, const string& name, char indicator, string& values);
		//! Returns data type indicator for \p field_name.
		char datatype_indicator(string& field_name);
		//! Returns a list or range of valid values for \p field_name.
		string userdef_values(string& field_name);
		//! Add the list of names of ZZALOG application-specific fields.
		void add_my_appdefs();
		//! Remove existing user defined fields
		void delete_userdefs();
		//! Remove existing application-specific fields.
		void delete_appdefs();
		//! Returns true if \p field_name is a user defined one.
		bool is_userdef(string field_name);
		//! Returns data type from \p indicator
		string datatype(char indicator);
		//! Returns data type from \p field name
		string datatype(string field_name);
		//! Returns enumartion name for \p field.
		
		//! In some cases the enumartion name may depend on another field in the QSO \p record.
		//! For example the enumeration for STATE depends on DXCC.
		string enumeration_name(const string& field, record* record);
		//! Returns true if the DXCC has ADIF defined primary administrative districts.
		bool has_states(int dxcc);
		//! Validates a \p record (index \p number) - returns TRUE if record corrected.
		bool validate(record* record, qso_num_t number);
		//! Validate the \p data in the field \p field_name.
		
		//! Reporting is inhibited if \p inhibit_report is true.
		valn_error_t validate(const string&  field_name, const string& data, bool inhibit_report = false);
		//! Sets the loaded filename
		void loaded_filename(string value);
		//! Returns the tip message for \p field as used in QSO \p record.
		string get_tip(const string& field, record* record);
		//! Returns the generic tip message.
		string get_tip(const string& field);
		//! User wants to continue after a validation query.
		bool do_continue();
		//! Reset continue flag
		void reset_continue();
		//! Add application defined field \p name with data type \p indicator.
		bool add_appdef(const string& name, char indicator);
		//! Returms the description of the enumeration \p value.
		string describe_enumeration(spec_dataset* dataset, string value);
		//! Returns the summary of the enumeration \p name \p value.
		string summarise_enumaration(string name, string value);
		//! Returns entity name for DXCC Number
		string entity_name(int dxcc);
		//! Add user defined enumeration \p value for an existing \p field - return true if successful
		bool add_user_enum(string field, string value);
		//! Remove user enums and macros - and restore originals.
		void delete_user_data();
		//! Returns list of bands in frequency order.
		band_set* bands();
		//! Create a list of bands in frequency order.
		void process_bands();
		//! Returns true if the spec_data has been loaded.
		bool valid();

	// protected methods
	protected:
		//! Returns path to all.xml
		string get_path();
		//! Sort field names
		void process_fieldnames();
		//! Combine mode and submode into single dataset
		void process_modes();
		//! Check the \p data is the correct format for the \p field either against a regex \p pattern or specific \p datatypes
		valn_error_t check_format(const string&  data, const string&  field, const string&  datatype, const basic_regex<char>& pattern);
		//! Check that \p data is in the correct value range for \p field.
		valn_error_t check_string(const string&  data, const string&  field, const string&  datatype);
		//! Check the \p data is between to non-integer minimum and maximum values for \p field
		valn_error_t check_number(const string&  data, const string&  field, const string&  datatype);
		//! Check the \p data is between two integer minimum and maximum values
		valn_error_t check_integer(const string&  data, const string&  field, const string&  datatype);
		//! Check that an enumeration is valid
		valn_error_t check_enumeration(const string& data, const string& field, const string& datatype);
		//! Check that the \p data is a separated list of the specified datatype/enumeration
		valn_error_t check_list(const string&  data, const string&  field, const string&  datatype, bool bIsEnumeration, char cSeparator);
		//! Check that the \p data is the specified datatype/enumeration
		valn_error_t check_datatype(const string&  data, const string&  field, const string&  datatype, bool bIsEnumeration);
		//! Check that time is valid
		valn_error_t check_time(const string& data, const string& field);
		//! Handle a validation error
		void handle_error(valn_error_t error_code, const string&  data, const string&  datatype, const string&  field);
		//! Report validation error
		void report_error(valn_error_t error_code, const string&  data, const string&  datatype, const string&  field);
		//! Auto-correct error returns TRUE if successful
		bool auto_correction(valn_error_t error_code, const string&  data, const string& sDisplayData, const string&  datatype, const string&  field);
		//! Ask the user to correct the errr.
		bool ask_correction(const string& field);
		//! Report correction
		void report_correction(const string&  field, const string&  data, status_t status);
		//! Returns trsing with removed CR and LF characters
		string convert_ml_string(const string& data);
		//! Generate report timestamp and fault
		string report_timestamp(string field, string data);
		// protected attributes
	protected:
		//! ADIF Version
		string adif_version_;
		//! List of Fieldnames
		set<string> field_names_;
		//! Array of user defs
		vector<string> userdef_names_;
		//! List of app defs (not in reference)
		set<string> appdef_names_;
		//! List of user defined enumerations
		set<string> user_enums_;
		//! Quick lookup of datatype indicator
		map<string, char> datatype_indicators_;
		//! Missing file already reported
		bool error_reported_;
		//! Correction message
		string correction_message_;
		//! User or auto corrected
		bool field_corrected_;
		//! Error message
		string error_message_;
		//! Record being validated (and corrected)
		record* record_;
		//! Original uncorrected record
		record* saved_record_;
		//! Record index
		qso_num_t record_number_;
		//! Number of validation errors
		int error_count_;
		//! Number of records with errors
		int error_record_count_;
		//! Number of records.
		int record_count_;
		//! Number of records which have been corrected.
		bool record_corrected_;
		//! Inhibit report
		bool inhibit_error_report_;
		//! Loaded filename
		string loaded_filename_;
		//! Validation abandoned by user. 
		bool abandon_validation_;
		//! List of bands in frequency order
		band_set* bands_;
		//! spec_data has been loaded and is valid
		bool data_loaded_;

	};

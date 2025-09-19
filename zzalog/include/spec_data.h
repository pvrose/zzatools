#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <regex>
#include <fstream>
#include<ostream>

#include <FL/Fl_Choice.H>

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
		VE_VALUE_FORMAT_WARNING,       //!< std::string value does not have the recommended format
		VE_VALUE_INVALID,              //!< enumeration value is not valid 
		VE_VALUE_NOT_RECOMMENDED,      //!< std::string value not recommended for interoperability
		VE_VALUE_INCOMPATIBLE,         //!< std::string value is incompatible with another field
		VE_VALUE_OUTDATED,             //!< std::string value has been removed from valid std::list
		VE_VALUE_MULTILINE,            //!< std::string value includes NL and CR when not multiline
		VE_VALUE_INTL,                 //!< std::string value includes non-ASCII characters
		VE_VALUE_UNCHECKABLE,          //!< std::string value cannot be checked for compatibility
		VE_TOP                         //!< last element in enumerated type
	};

	//! Reference dataset - data structure
	struct spec_dataset {
		//! Names of the columns in the data std::set
		std::vector<std::string> column_names;
		//! The data records - std::map from first data item to a std::map of column names to other data items
		std::map<std::string, std::map<std::string, std::string>* > data;
		//! Constructor.
		spec_dataset() {
			column_names.clear();
			data.clear();
		}
	};

	enum status_t : char;

	//! This class provides the ADIF specification reference database as a std::set of named datasets. 
	
	//! It provides the access to the database and methods to validate the ADIF data against the specification
	//! The database is a std::map of the dataset name to a dataset
	class spec_data : public std::map<std::string, spec_dataset*>
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
		std::string dxcc_mode(std::string mode);
		//! Returns the ADIF mode for a \p submode
		std::string mode_for_submode(std::string submode);
		//! Returns the band for a specific \p frequency (in megahertz)
		std::string band_for_freq(double frequency_MHz);
		//! Returns the Lower frequency for a band \p sBand
		double freq_for_band(std::string sBand);
		//! Receives the \p upper and \p lower frequencies for \p band
		void freq_for_band(std::string band, double& lower, double& upper);
		//! Returns true if \p mode is a submode of another mode.
		bool is_submode(std::string mode);
		//! Returns the spec_datasset named \p name.
		spec_dataset* dataset(std::string name);
		//! Returns the ADIF version coded in the file.
		std::string adif_version();
		//! Returns sorted std::list of field names
		std::set<std::string>* sorted_fieldnames();
		//! Add user defined fields 
		
		//! \param id The index - i.e. USERDEF<I>n</I>.
		//! \param name The name of the user defined field
		//! \param indicator the single character showing data type of the field
		//! \param values Specifies a valid value range for  the field.
		//! \return true if successful.
		bool add_userdef(int id, const std::string& name, char indicator, std::string& values);
		//! Returns data type indicator for \p field_name.
		char datatype_indicator(std::string& field_name);
		//! Returns a std::list or range of valid values for \p field_name.
		std::string userdef_values(std::string& field_name);
		//! Add the std::list of names of ZZALOG application-specific fields.
		void add_my_appdefs();
		//! Remove existing user defined fields
		void delete_userdefs();
		//! Remove existing application-specific fields.
		void delete_appdefs();
		//! Returns true if \p field_name is a user defined one.
		bool is_userdef(std::string field_name);
		//! Returns data type from \p indicator
		std::string datatype(char indicator);
		//! Returns data type from \p field name
		std::string datatype(std::string field_name);
		//! Returns enumartion name for \p field.
		
		//! In some cases the enumartion name may depend on another field in the QSO \p record.
		//! For example the enumeration for STATE depends on DXCC.
		std::string enumeration_name(const std::string& field, record* record);
		//! Returns true if the DXCC has ADIF defined primary administrative districts.
		bool has_states(int dxcc);
		//! Validates a \p record (index \p number) - returns TRUE if record corrected.
		bool validate(record* record, qso_num_t number);
		//! Validate the \p data in the field \p field_name.
		
		//! Reporting is inhibited if \p inhibit_report is true.
		valn_error_t validate(const std::string&  field_name, const std::string& data, bool inhibit_report = false);
		//! Sets the loaded filename
		void loaded_filename(std::string value);
		//! Returns the tip message for \p field as used in QSO \p record.
		std::string get_tip(const std::string& field, record* record);
		//! Returns the generic tip message.
		std::string get_tip(const std::string& field);
		//! User wants to continue after a validation query.
		bool do_continue();
		//! Reset continue flag
		void reset_continue();
		//! Add application defined field \p name with data type \p indicator.
		bool add_appdef(const std::string& name, char indicator);
		//! Returms the description of the enumeration \p value.
		std::string describe_enumeration(spec_dataset* dataset, std::string value);
		//! Returns the summary of the enumeration \p name \p value.
		std::string summarise_enumaration(std::string name, std::string value);
		//! Returns entity name for DXCC Number
		std::string entity_name(int dxcc);
		//! Add user defined enumeration \p value for an existing \p field - return true if successful
		bool add_user_enum(std::string field, std::string value);
		//! Remove user enums and macros - and restore originals.
		void delete_user_data();
		//! Returns std::list of bands in frequency order.
		band_set* bands();
		//! Create a std::list of bands in frequency order.
		void process_bands();
		//! Returns true if the spec_data has been loaded.
		bool valid();

	// protected methods
	protected:
		//! Load data from JSON 
		bool load_json();
		//! Returns path to all.xml
		std::string get_path();
		//! Sort field names
		void process_fieldnames();
		//! Combine mode and submode into single dataset
		void process_modes();
		//! Check the \p data is the correct format for the \p field either against a regex \p pattern or specific \p datatypes
		valn_error_t check_format(const std::string&  data, const std::string&  field, const std::string&  datatype, const std::basic_regex<char>& pattern);
		//! Check that \p data is in the correct value range for \p field.
		valn_error_t check_string(const std::string&  data, const std::string&  field, const std::string&  datatype);
		//! Check the \p data is between to non-integer minimum and maximum values for \p field
		valn_error_t check_number(const std::string&  data, const std::string&  field, const std::string&  datatype);
		//! Check the \p data is between two integer minimum and maximum values
		valn_error_t check_integer(const std::string&  data, const std::string&  field, const std::string&  datatype);
		//! Check that an enumeration is valid
		valn_error_t check_enumeration(const std::string& data, const std::string& field, const std::string& datatype);
		//! Check that the \p data is a separated std::list of the specified datatype/enumeration
		valn_error_t check_list(const std::string&  data, const std::string&  field, const std::string&  datatype, bool bIsEnumeration, char cSeparator);
		//! Check that the \p data is the specified datatype/enumeration
		valn_error_t check_datatype(const std::string&  data, const std::string&  field, const std::string&  datatype, bool bIsEnumeration);
		//! Check that time is valid
		valn_error_t check_time(const std::string& data, const std::string& field);
		//! Handle a validation error
		void handle_error(valn_error_t error_code, const std::string&  data, const std::string&  datatype, const std::string&  field);
		//! Report validation error
		void report_error(valn_error_t error_code, const std::string&  data, const std::string&  datatype, const std::string&  field);
		//! Auto-correct error returns TRUE if successful
		bool auto_correction(valn_error_t error_code, const std::string&  data, const std::string& sDisplayData, const std::string&  datatype, const std::string&  field);
		//! Ask the user to correct the errr.
		bool ask_correction(const std::string& field);
		//! Report correction
		void report_correction(const std::string&  field, const std::string&  data, status_t status);
		//! Returns trsing with removed CR and LF characters
		std::string convert_ml_string(const std::string& data);
		//! Generate report timestamp and fault
		std::string report_timestamp(std::string field, std::string data);
		//! Process subdivision datasets
		void process_subdivision(std::string name);
		// protected attributes
	protected:
		//! ADIF Version
		std::string adif_version_;
		//! List of Fieldnames
		std::set<std::string> field_names_;
		//! Array of user defs
		std::vector<std::string> userdef_names_;
		//! List of app defs (not in reference)
		std::set<std::string> appdef_names_;
		//! List of user defined enumerations
		std::set<std::string> user_enums_;
		//! Quick lookup of datatype indicator
		std::map<std::string, char> datatype_indicators_;
		//! Missing file already reported
		bool error_reported_;
		//! Correction message
		std::string correction_message_;
		//! User or auto corrected
		bool field_corrected_;
		//! Error message
		std::string error_message_;
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
		std::string loaded_filename_;
		//! Validation abandoned by user. 
		bool abandon_validation_;
		//! List of bands in frequency order
		band_set* bands_;
		//! spec_data has been loaded and is valid
		bool data_loaded_;

	};

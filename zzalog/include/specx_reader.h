#ifndef __SPECX_READER__
#define __SPECX_READER__

#include "xml_reader.h"

#include <list>
#include <string>



	class spec_data;
	struct spec_dataset;

	//! XML element types used in ADIF specification
	enum specx_element_t {
		SXE_NONE,        //!< Not yet processing an element
		SXE_ADIF,        //!< Top-level element 
		SXE_DATATYPES,   //!< Start of datatypes dataset 
		SXE_HEADER,      //!< Start of dataset header 
		SXE_VALUEH,      //!< column name
		SXE_RECORD,      //!< Start of dataset record
		SXE_VALUER,      //!< record data item
		SXE_ENUMS,       //!< Enumerations: 
		SXE_ENUM,        //!< Enumeration dataset 
		SXE_FIELDS       //!< Fields dataset 

	};

	//! This class reads the XML specification from ADIF and loads it into the specification database.
	//! 
	/*! \code
  <adif version="3.1.5" status="Released" date="2024-11-28T00:00:00Z" created="2024-11-28T09:18:56Z">
	<dataTypes>
	  <header>
		<value>Data Type Name</value>
		<value>Data Type Indicator</value>
		<value>Description</value>
		<value>Minimum Value</value>
		<value>Maximum Value</value>
		<value>Import-only</value>
		<value>Comments</value>
	  </header>
	  <record>
		<value name="Data Type Name">AwardList</value>
		<value name="Description">a comma-delimited std::list of members of the Award enumeration</value>
		<value name="Import-only">true</value>
	  </record>
	  :
	</dataTypes>
	<enumerations>
	  <enumeration name="Ant_Path">
		<header>
	      <value>Enumeration Name</value>
          <value>Abbreviation</value>
		  <value>Meaning</value>
		  <value>Import-only</value>
	      <value>Comments</value>
		</header>
		<record>
		  <value name="Enumeration Name">Ant_Path</value>
		  <value name="Abbreviation">G</value>
		  <value name="Meaning">grayline</value>
		</record>
		:
	  </enumeration>
      :
	</enumerations>
	<fields>
	  <header>
		<value>Field Name</value>
	    <value>Data Type</value>
		<value>Enumeration</value>
		<value>Description</value>
		<value>Header Field</value>
		<value>Minimum Value</value>
		<value>Maximum Value</value>
		<value>Import-only</value>
		<value>Comments</value>
      </header>
	  <record>
		<value name="Field Name">ADIF_VER</value>
		<value name="Data Type">String</value>
	    <value name="Description">identifies the version of ADIF used in this file in the format X.Y.Z where X is an integer designating the ADIF epoch Y is an integer between 0 and 9 designating the major version Z is an integer between 0 and 9 designating the minor version</value>
		<value name="Header Field">true</value>
	  </record>
	  :
    </fields>
</adif>
\endcode
*/
	class specx_reader :
		public xml_reader
	{
	public:
		//! Constructor.
		specx_reader();
		//! Destructor.
		~specx_reader();

		// Public methods
	public:

		// Overloadable XML handlers
		//! Start element 
		virtual bool start_element(std::string name, std::map<std::string, std::string>* attributes);
		//! End element
		virtual bool end_element(std::string name);
		//! Special element
		virtual bool declaration(xml_element::element_t element_type, std::string name, std::string content);
		//! Processing instruction
		virtual bool processing_instr(std::string name, std::string content);
		//! characters
		virtual bool characters(std::string content);

		// Load \p data from input stream /p in: /p version receives version from file.
		bool load_data(spec_data* data, std::istream& in, std::string& version);
		// Protected methods

	protected:
		//! Start: adif verstion= status= created=
		bool start_adif(std::map<std::string, std::string>* attributes);
		//! Start: dataTypes
		bool start_datatypes();
		//! Start: header
		bool start_header();
		//! Start header value
		bool start_value(std::map<std::string, std::string>* attributes);
		//! Start record
		bool start_record();
		//! Start enumerations
		bool start_enumerations();
		//! Start enumeration
		bool start_enumeration(std::map<std::string, std::string>* attributes);
		//! Start fields
		bool start_fields();
		//! End ADIF
		bool end_adif();
		//! End Datatypes
		bool end_datatypes();
		//! End header
		bool end_header();
		//! End header value
		bool end_header_value();
		//! End Record
		bool end_record();
		//! End record value
		bool end_record_value();
		//! End enumerations
		bool end_enumerations();
		//! End enumeration
		bool end_enumeration();
		//! End fields
		bool end_fields();

	protected:
		//! The data
		spec_data * data_;
		//! Stack of elments being processed
		std::list<specx_element_t> elements_;
		//! Current column headers
		std::vector<std::string> column_headers_;
		//! Version
		std::string adif_version_;
		//! Status
		std::string file_status_;
		//! Created date
		std::string created_;
		//! Current dataset name - Datatypes, specific enumeration or fields
		std::string dataset_name_;
		//! Current item name
		std::string item_name_;
		//! Current record
		std::map<std::string, std::string>* record_data_;
		//! Element data
		std::string element_data_;
		//! Number of ignored XML elements
		int num_ignored_;
		//! Name of the record item
		std::string record_name_;
		//! Is this an enumeration
		bool dataset_enumeration_;
		//! Remember the stream so that we can display progress
		std::istream* in_file_;

	};

#endif
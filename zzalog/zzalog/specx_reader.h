#ifndef __SPECX_READER__
#define __SPECX_READER__

#include "xml_reader.h"

#include <list>
#include <string>

using namespace std;

namespace zzalog {

	class spec_data;
	struct spec_dataset;

	enum specx_element_t {
		SXE_NONE,        // Not yet processing an element
		SXE_ADIF,        // Top-level element <adif version=".." status="Released" created = "ISO date"> datatypes enumerations fields</adif>
		SXE_DATATYPES,   // Start of datatypes dataset <dataType>header record+</dataTypes>
		SXE_HEADER,      // Start of dataset header <header>value+</header>
		SXE_VALUEH,      // column name <value>Column</value>
		SXE_RECORD,      // Start of dataset record <record>value+<record>
		SXE_VALUER,      // record data item <value name="<Column>">Item value</value>
		SXE_ENUMS,       // Enumerations: <enumerations>enumeration+</enumerations>
		SXE_ENUM,        // Enumeration dataset <enumeration name="name">header record+</enumeration>
		SXE_FIELDS       // Fields dataset <fields>header record+</field>

	};
	class specx_reader :
		public xml_reader
	{
	public:
		specx_reader();
		~specx_reader();

		// Public methods
	public:

		// Overloadable XML handlers
		// Start 
		virtual bool start_element(string name, map<string, string>* attributes);
		// End
		virtual bool end_element(string name);
		// Special element
		virtual bool declaration(xml_element::element_t element_type, string name, string content);
		// Processing instruction
		virtual bool processing_instr(string name, string content);
		// characters
		virtual bool characters(string content);

		// Load data from specified file into and add each record to the map
		bool load_data(spec_data* data, istream& in, string& version);
		// Protected methods

	protected:
		// Start <adif verstion= status= created=
		bool start_adif(map<string, string>* attributes);
		// Start <dataTypes>
		bool start_datatypes();
		// Start header
		bool start_header();
		// Start header value
		bool start_value(map<string, string>* attributes);
		// Start record
		bool start_record();
		// Start enumerations
		bool start_enumerations();
		// Start enumeration
		bool start_enumeration(map<string, string>* attributes);
		// Start fields
		bool start_fields();
		// End ADIF
		bool end_adif();
		// End Datatypes
		bool end_datatypes();
		// End header
		bool end_header();
		// End header value
		bool end_header_value();
		// End Record
		bool end_record();
		// End record value
		bool end_record_value();
		// End enumerations
		bool end_enumerations();
		// End enumeration
		bool end_enumeration();
		// End fields
		bool end_fields();

	protected:
		// The data
		spec_data * data_;
		// List of elments
		list<specx_element_t> elements_;
		// Current column headers
		vector<string> column_headers_;
		// Version
		string adif_version_;
		// Status
		string file_status_;
		// Created date
		string created_;
		// Current dataset name - Datatypes, specific enumeration or fields
		string dataset_name_;
		// Current item name
		string item_name_;
		// Current record
		map<string, string>* record_data_;
		// Element data
		string element_data_;
		// Number of ignored XML elements
		int num_ignored_;
		// Name of the record item
		string record_name_;
		// Is an enumeration
		bool dataset_enumeration_;
		// Remember the stream so that we can display progress
		istream* in_file_;

	};

}

#endif
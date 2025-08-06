#pragma once

#include "xml_reader.h"
#include "cty_data.h"

#include <list>
#include <string>
#include <istream>

using namespace std;

	// This class reads the XML cty.xml file obtained from Clublog
	class cty1_reader :
		public xml_reader
	{
	public:
		cty1_reader();
		~cty1_reader();

		// Overloadable XML handlers
		// Start 
		virtual bool start_element(string name, map<string, string>* attributes);
		// End
		virtual bool end_element(string name);
		// Special element
		virtual bool declaration(xml_element::element_t element_type, string name, string content);
		// Processing instruction
		virtual bool process_instr(string name, string content);
		// characters
		virtual bool characters(string content);

		// Load data from specified file into and add each record to the map
		bool load_data(cty_data* data, istream& in, string& version);
		// Protected methods
	protected:



	protected:
		// Ignore processing until end element
		bool ignore_processing_;
		// Element processig list
		list<string> elements_;
		// File timestamp
		string timestamp_;
		// Current elements
		cty_data::ent_entry* current_entity_;
		cty_data::patt_entry* current_pattern_;
		string current_match_;
		//cty1_data::exc_entry* current_exception_;
		//cty1_data::invalid_entry* current_invalid_;
		//cty1_data::zone_entry* current_zone_exc_;
		//cty1_data::entity_entry* current_entity_;
		//cty1_data::prefix_entry* current_prefix_;
		// The data being read
		cty_data* data_;
		// Value of element
		string value_;
		// Pointer to file
		istream* file_;
		// Number of elements read
		int number_read_;


	};



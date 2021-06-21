#ifndef __EXC_READER__
#define __EXC_READER__

#include "../zzalib/xml_reader.h"

#include <list>
#include <string>
#include <istream>

using namespace std;
using namespace zzalib;
namespace zzalog {
	class exc_data;
	struct exc_entry;
	struct invalid;
	struct zone_exc;

	// This class reads the XML cty.xml file obtained from Clublog
	class exc_reader :
		public xml_reader
	{
	public:
		exc_reader();
		~exc_reader();

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
		bool load_data(exc_data* data, istream& in, string& version);
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
		exc_entry* current_exception_;
		invalid* current_invalid_;
		zone_exc* current_zone_exc_;
		// The data being read
		exc_data* data_;
		// Value of element
		string value_;
		// Pointer to file
		istream* file_;


	};

}

#endif


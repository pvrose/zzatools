#pragma once

#include "xml_reader.h"
#include "cty_data.h"
#include "cty_element.h"

#include <list>
#include <string>
#include <istream>

using namespace std;

	//! This class reads the XML cty.xml file obtained from Clublog.org
	//! 
	/** XML structure
	\code

	<clublog>
		:
		:
		<exceptions>
			<exception record = '6002'>
				<call>VE1ST/NA14</call>
				<entity>CANADA</entity>
				< adif>1 </adif >
				< cqz>4 </cqz >
				<cont>NA</cont>
				<long>-97.14 </long >
				< lat>49.90 </lat >
			</exception>
			:
			:
		</exceptions>
		:
		:
		<invalid_operations>
			<invalid record = '489'>
				<call>T88A</call>
				< start>1995-05-01T00:00:00+00:00 </start >
				< end>1995-12-31T23:59:59+00:00 </end >
			</invalid>
			:
			:
		</invalid_operations>
		:
		:
		<zone_exceptions>
			<zone_exception record = '59'>
				<call>KD6WW/VY0</call>
				< zone>1 </zone >
				< start>2003-07-30T00:00:00+00:00 </start >
				< end>2003-07-31T23:59:59+00:00 </end >
			</zone_exception>
			:
			:
		</zone_exceptions>
		:
		:
	</clublog>
	\endcode
	*/

	class cty1_reader :
		public xml_reader
	{
	public:
		//! Constructor.
		cty1_reader();
		//! Destructor.
		~cty1_reader();

		// Overloadable XML handlers
		//! Start element.
		virtual bool start_element(string name, map<string, string>* attributes);
		//! End element
		virtual bool end_element(string name);
		//! Special element
		virtual bool declaration(xml_element::element_t element_type, string name, string content);
		//! Processing instruction
		virtual bool process_instr(string name, string content);
		//! characters
		virtual bool characters(string content);

		//! Load data
		
		//! \param data Internal database.
		//! \param in input stream.
		//! \param version Returns any version information in the file.
		//! \return true if successful, false if not.
		bool load_data(cty_data* data, istream& in, string& version);
		// Protected methods
	protected:

		//! Converts data in standard XML format to "YYYYMMDD" format.
		string xmldt2date(string xml_data);

	protected:
		//! Ignore processing until the end of current element.
		bool ignore_processing_;
		//! Element processig list
		list<string> elements_;
		//! File timestamp
		string timestamp_;
		//! Current entity being processed.
		cty_entity* current_entity_;
		//! Current prefix being processed.
		cty_prefix* current_prefix_;
		//! Current exception record being processed
		cty_exception* current_exception_;
		//! Current callsign in an exception match.
		string current_match_;
		//! The internal database being loaded.
		cty_data* data_;
		//! Value of element
		string value_;
		//! Input stream from file.
		istream* file_;
		//! Number of elements read
		int number_read_;


	};



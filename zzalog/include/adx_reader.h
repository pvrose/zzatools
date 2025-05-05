#ifndef __ADX_READER__
#define __ADX_READER__
#include "xml_reader.h"

#include <string>
#include <map>
#include <set>
#include <list>
#include <fstream>

using namespace std;

class book;
class record;



	// ADX XML element type
	enum adx_element_t {
		AET_NONE,        // Not yet received an element
		AET_ADX,         // <ADX>...</ADX>
		AET_HEADER,      // <HEADER>...</HEADER>
		AET_FIELD,       // <[fieldname]>[data]</[fieldname]>
		AET_APP,         // <APP PROGRAMID="[id]" FIELDNAME="[fieldname]" TYPE="[datatype]">[data]</APP>
		AET_USERDEFH,    // <USERDEF FIELDID="[n]".....>[fieldname]</USERDEF>
		AET_USERDEFR,    // <USERDEF FIELDNAME="[fieldname]">[data]</USERDEF>
		AET_RECORDS,     // <RECORDS>...</RECORDS>
		AET_RECORD,      // <RECORD>...</RECORD>
		AET_COMMENT      // <--......-->
	};


	// This class handles reading ADIF data in .adx format into the book
	// It intercepts the XML reader parsing interpreting the elements as items, records and the book
	class adx_reader :
		public xml_reader
	{
	public:
		adx_reader();
		~adx_reader();

		// load data to book
		bool load_book(book* book, istream& in);

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

		double progress();

	protected:

		// start the <APP> element
		bool start_app(map<string, string>* attributes);
		// start the <RECORD> element
		bool start_record();
		// start the <ADX> element
		bool start_adx();
		// start the <HEADER> element
		bool start_header();
		// start the <RECORDS> element
		bool start_records();
		// start the <USERDEF> element
		bool start_userdef(map<string, string>* attributes);
		// start the <[field-name]> element
		bool start_field(string field_name);



	protected:
		book * my_book_;
		// What element types we are processing
		list<adx_element_t> elements_;
		// Current field being read
		string field_name_;
		// Current data in field
		string value_;
		// Field data type
		string datatype_;
		// Field available values
		string available_values_;
		// Current record being created
		record* record_;
		// Current document location
		int line_num_;
		int column_num_;
		// Modified by parse or validate on load
		bool modified_;
		// List of USERDEF fields
		set<string> userdef_fields_;
		// count of ignore comments
		int num_comments_;
		// count of ignored constructs
		int num_ignored_;
		// count of completed records
		size_t num_records_;
		// counts for progress
		long file_size_;
		long previous_count_;
		long current_count_;
		istream* in_;
		// Igore non-APP_ZZA_ fields
		bool ignore_app_;
	};

#endif
#ifndef __ADX_READER__
#define __ADX_READER__
#include "xml_reader.h"

#include <string>
#include <map>
#include <set>
#include <list>
#include <fstream>



class book;
class record;



	//! Set of XML element types used in ADX.
	enum adx_element_t {
		AET_NONE,        //! Not yet received an element
		AET_ADX,         //! "<ADX>...</ADX>"
		AET_HEADER,      //! "<HEADER>...</HEADER>"
		AET_FIELD,       //! "<[fieldname]>[data]</[fieldname]>"
		AET_APP,         //! "<APP PROGRAMID=[id] FIELDNAME=[fieldname] TYPE=[datatype]>[data]</APP>"
		AET_USERDEFH,    //! "<USERDEF FIELDID=[n].....>[fieldname]</USERDEF>"
		AET_USERDEFR,    //! "<USERDEF FIELDNAME=[fieldname]>[data]</USERDEF>"
		AET_RECORDS,     //! "<RECORDS>...</RECORDS>"
		AET_RECORD,      //! "<RECORD>...</RECORD>"
		AET_COMMENT      //! "<--......-->"
	};


	//! This class handles reading ADIF data in .adx format into the book.
	
	//! It intercepts the XML reader parsing interpreting the elements as items, records and the book.
	class adx_reader :
		public xml_reader
	{
	public:
		//! Constructor.
		adx_reader();
		//! Destructor.
		~adx_reader();

		//! Load data from input stream \a in to book \a book.
		bool load_book(book* book, std::istream& in);

		// Overloadable XML handlers
		//! Start XML element: The element name is used to call one of the specific start... methods.
		virtual bool start_element(std::string name, std::map<std::string, std::string>* attributes);
		//! End XML element: The element name is used to call one of the specific end... methods.
		virtual bool end_element(std::string name);
		//! Special element: If the declaration is a comment within the header record, then
		//! it is added to the header.
		virtual bool declaration(xml_element::element_t element_type, std::string name, std::string content);
		//! Processing instruction - ignored.
		virtual bool process_instr(std::string name, std::string content);
		//! Characters - characters are stored for use in the end_element method. 
		
		//! Any singleton
		//! \p CR or \p LF characters are translated into a \p CR/LF combination.
		virtual bool characters(std::string content);

		//! Used in showing progress.
		
		//! \return fraction of bytes processed. 
		double progress();

	protected:

		//! start the APP element.
		
		//! \code
		//! <APP PROGRAMID="[id]" FIELDNAME="[fieldname]" TYPE="[datatype]">[data]</APP>
		//! \endcode
		//! Only ZZA and some EQSL and LOTW application-specific field names are read in.
		//! \param attributes see above.
		//! \return true if successful, false if not.
		bool start_app(std::map<std::string, std::string>* attributes);
		//! start the RECORD element.
		
		//! \code
		//! <RECORD> field elements </RECORD>
		//! \endcode
		//! \return true if successful, false if not.
		bool start_record();
		//! start the ADX element.
		
		//! \code
		//! <ADX> header element, records element</ADX>
		//! \endcode
		//! \return true if successful, false if not.
		bool start_adx();
		//! start the HEADER element.
		
		//! \code
		//! <HEADER> field elements </HEADER>
		//! \endcode
		//! \return true if successful, false if not.
		bool start_header();
		//! start the RECORDS element.
		
		//! \code
		//! <RECORDS> record elements </RECORD>
		//! \endcode
		//! \return true if successful, false if not.
		bool start_records();
		//! start the USERDEF element.
		
		//! \return true if successful, false if not.
		bool start_userdef(std::map<std::string, std::string>* attributes);
		//! start the [field-name] element.
		
		//! in header element
		//! \code
		//! <USERDEF FIELDID="n" TYPE="DATATYPEINDICATOR" ENUM="{A,B, ... N}" RANGE="{LOWERBOUND:UPPERBOUND}">FIELDNAME</USERDEF>
		//! \endcode
		//! in record element
		//! \code
		//! <USERDEF FIELDNAME="FIELDNAME">DATA</USERDEF>
		//! \endcode
		//! \return true if successful, false if not.
		bool start_field(std::string field_name);



	protected:
		//! The book being loaded.
		book * my_book_;
		//! The std::list of element types currently being procexxed processing
		std::list<adx_element_t> elements_;
		//! Current field being read
		std::string field_name_;
		//! Current data in field
		std::string value_;
		//! Field data type
		std::string datatype_;
		//! Field available values
		std::string available_values_;
		//! Current QSOrecord being created
		record* record_;
		//! Current line within the document being accessed
		int line_num_;
		//! Current column in the line being processed
		int column_num_;
		//! Book has been modified by parse or validate on load
		bool modified_;
		//! List of USERDEF fields
		std::set<std::string> userdef_fields_;
		//! count of ignored comments
		int num_comments_;
		//! count of ignored constructs
		int num_ignored_;
		//! count of completed records
		size_t num_records_;
		//! Total file size in bytes (for progress)
		long file_size_;
		//! Previous position of read (for progress)
		long previous_count_;
		//! Current position of read (for progress)
		long current_count_;
		//! input stream
		std::istream* in_;
		//! Not a ZZALOG application-specified field, ignore further details.
		bool ignore_app_;
	};

#endif
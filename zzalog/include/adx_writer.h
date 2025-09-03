#ifndef __ADX_WRITER__
#define __ADX_WRITER__

// #include "xml_element.h"
#include "xml_writer.h"
#include "adx_reader.h"

#include <string>

using namespace std;

class book;
class record;
typedef size_t item_num_t;

	//! This class generates the XML representation for ADIF (ADX) from the records in the book 
	//! and writes them to an output stream
	class adx_writer :
		public xml_writer
	{
	public:
		//! Constructor
		adx_writer();
		//! Destructor
		~adx_writer();
		//! Generate XML for the records in book and send them to the output stream.
		//! \param book the data to be written.
		//! \param os output stream.
		//! \param clean mark records as clean after writing.
		//! \return true if successful, false if not.
		bool store_book(book* book, ostream& os, bool clean);
		//! Generate an individual element of the type given
		//! \param element type.
		//! \return true if successful, false if not.
		bool write_element(adx_element_t element);

		//! Used in reporting progress.
		//! \return fraction of records written.
		double progress();

	protected:
		//! Current QSO record being processed.
		record * record_;
		//! The data being processed.
		book* my_book_;
		//! The name of the field being processed.
		string field_name_;
		//! The value of the field being processed.
		string value_;
		//! The data type of the field being procesed - per ADIF specification.
		char type_indicator_;
		//! Flag set, indicating that records will be marked clean.
		bool clean_records_;
		//! The index of the current QSO record within the data being written.
		item_num_t current_;

	};
#endif

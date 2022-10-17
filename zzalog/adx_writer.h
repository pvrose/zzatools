#ifndef __ADX_WRITER__
#define __ADX_WRITER__

#include "../zzalib/xml_element.h"
#include "../zzalib/xml_writer.h"
#include "adx_reader.h"
#include "book.h"
#include "record.h"

#include <string>

using namespace std;

namespace zzalog {

	// This class generates the XML representation for ADIF (ADX) from the records in the book
	// It writes them to an output stream
	class adx_writer :
		public zzalib::xml_writer
	{
	public:
		adx_writer();
		~adx_writer();
		// Generate XML for the records in book and send them to the output stream
		bool store_book(book* book, ostream& os, bool clean);
		// Generate an individual element of the type given
		bool write_element(adx_element_t element);

	protected:
		// Current record being processed
		record * record_;
		// The book to be processed
		book* my_book_;
		// The current field being processed - its name
		string field_name_;
		// Its value
		string value_;
		// Its data type
		char type_indicator_;
		// Clean records
		bool clean_records_;


	};

}
#endif

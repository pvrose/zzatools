#ifndef __ADI_WRITER__
#define __ADI_WRITER__

#include "files.h"
#include "fields.h"

#include <string>
#include <set>

using namespace std;

	class book;
	class record;
	typedef size_t item_num_t;

	// This class will take a book and generate ADIF .adi format text onto the output stream
	class adi_writer
	{
	public:
		adi_writer();
		~adi_writer();

		// public methods
	public:
		// write book to output stream
		bool store_book(book* book, ostream& out, bool clean, field_list* fields = nullptr);
		// Convert record to ADIF format text and send to the output stream
		static void to_adif(record* record, ostream& out, field_list* fields = nullptr);
		// Convert item to ADIF format text and send to the output stream
		static string item_to_adif(record* record, string field);

		// Progress
		double progress();

		// protected methods
	protected:
		// write record to output stream
		ostream & store_record(record* record, ostream& out, load_result_t& result, field_list* fields = nullptr);

		// Data contains non-ASCII data
		const unsigned char non_ascii = 1;
		// Data contains data that cannot be interpreted in ISO-8859-1 (Latin-1) 
		const unsigned char non_latin = 2;
		// Control character
		const unsigned char control = 4;

		// Check contents are ADIF compliant
		unsigned char adif_compliance(book* b, field_list* fields);


	protected:
		bool clean_records_;
		item_num_t current_;
		book* out_book_;

	};

#endif
#ifndef __ADI_WRITER__
#define __ADI_WRITER__

#include "book.h"
#include "files.h"
#include "spec_data.h"

#include <string>
#include <set>

using namespace std;

namespace zzalog {

	class book;
	class record;

	// This class will take a book and generate ADIF .adi format text onto the output stream
	class adi_writer
	{
	public:
		adi_writer();
		~adi_writer();

		// public methods
	public:
		// write book to output stream
		bool store_book(book* book, ostream& out, set<string>* fields = nullptr);

		// protected methods
	protected:
		// write record to output stream
		ostream & store_record(record* record, ostream& out, load_result_t& result, set<string>* fields = nullptr);
		// Convert item to ADIF format text and send to the output stream
		void item_to_adif(record* record, string field, ostream& out);
		// Convert record to ADIF format text and send to the output stream
		void to_adif(record* record, ostream& out, set<string>* fields = nullptr);

	};

}

#endif
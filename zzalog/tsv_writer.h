#ifndef __TSV_WRITER__
#define __TSV_WRITER__

#include "book.h"
#include "record.h"


#include <set>
#include <string>

using namespace std;

namespace zzalog {

	// This class provides a means of writing records to an output stream in a tab-separated variables 
	// format
	class tsv_writer
	{
	public:
		tsv_writer();
		~tsv_writer();

		// write book to output stream
		bool store_book(book* book, ostream& out, set<string>* fields = nullptr);

		// protected methods
	protected:
		// write record to output stream
		ostream & store_record(record* record, ostream& out, set<string>* fields = nullptr);
		// Default fields
		void default_fields(set<string>* fields);

	};

}
#endif

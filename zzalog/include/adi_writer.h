#ifndef __ADI_WRITER__
#define __ADI_WRITER__

#include "book.h"
#include "files.h"
#include "spec_data.h"
#include "fields.h"

#include <string>
#include <set>

using namespace std;

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

	protected:
		bool clean_records_;
		item_num_t current_;
		book* out_book_;

	};

#endif
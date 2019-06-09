#ifndef __ADI_READER__
#define __ADI_READER__

#include "files.h"
#include "book.h"

#include <istream>
#include <string>

using namespace std;

namespace zzalog {

	// This class reads standard ADIF .adi format data and loads it into the specified book.
	// It is responsible for the interpretation of the ADIF format into an array of records./
	// These will be ordered by the chronological date of each QSO record.
	// The data can be sourced as any input stream form
	class adi_reader
	{

		// Constructor and destructor
	public:
		adi_reader();
		~adi_reader();

		// public methods
	public:
		// load data to book - data is read off the input stream and forms an array of records in book
		bool load_book(book* book, istream& in);

		// protected methods
	protected:
		// Load data to an individual record, failure is reported in result
		istream & load_record(record* record, istream& in, load_result_t& result);

		// protected attributes
	protected:
		book * my_book_;
		// Neither a header nor the first record has been read yet.
		bool expecting_header_;
		// Counts for progress 
		long previous_count_; // Previous reported count
		long byte_count_;     // Current number of bytes read
		long file_size_;      // Total expected number of bytes in stream
	};

}

#endif
#ifndef __ADI_READER__
#define __ADI_READER__

#include "files.h"

#include <istream>
#include <string>
#include <set>

using namespace std;

// Forward declaration
class book;
class record;


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
		// Load data to an individual record, failure is reported in result
		istream & load_record(record* record, istream& in, load_result_t& result);

		// Read progress
		double progress();

		// protected methods
	protected:
		// protected attributes
	protected:
		book * my_book_;
		// Neither a header nor the first record has been read yet.
		bool expecting_header_;
		// Counts for progress 
		int number_records_;
		int record_count_;

		// Set of external application-defined fields that have been encountered altready
		set<string> known_app_fields;
	};


#endif
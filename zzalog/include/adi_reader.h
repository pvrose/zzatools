#ifndef __ADI_READER__
#define __ADI_READER__

#include<istream>
#include <string>
#include <set>



// Forward declaration
class book;
class record;


//! This class reads the logbook in ADIF .adi format. See https://adif.org.

	//! It loads it into the specified book.
	//! It is responsible for the interpretation of the ADIF format into an array of records./
	//! These will be ordered by the chronological date of each QSO record.
	//! The data can be sourced as any input stream form
	class adi_reader
	{

		// Constructor and destructor
	public:
		//! Constructor.
		adi_reader();
		//! Destructor.
		~adi_reader();

		// public methods
	public:

		//! Results from loading a file
		enum load_result_t {
			LR_GOOD,     //!< loaded OK.
			LR_BAD,      //!< failed to complete loading
			LR_EOF,      //!< EOF read
		};

		//! Load data from the specified input stream \a in into the specified \p book.
		//! \param book logbook that will be loaded.
		//! \param in input stream.
		bool load_book(book* book, std::istream& in);
		//! Load data to an individual record, failure is reported in result.
		//! \param record QSO to be written from input.
		//! \param in input stream.
		//! \param result result of the input read.
		//! \return the state of the input stream after the read.
		std::istream & load_record(record* record, std::istream& in, load_result_t& result);

		//! Used to report progress while reading
		//! \return fraction of the input stream loaded into the book.
		double progress();



		// protected methods
	protected:
		// protected attributes
	protected:
		//! Logbook being loaded.
		book * my_book_;
		//! Neither a header nor the first record has been read yet.
		bool expecting_header_;
		//! \brief Total number of records in file (read from header field APP_ZZA_NUM_RECORDS). 10000 assumed
		//! if not specified.
		int number_records_;
		//! Number of records so far loaded.
		int record_count_;

		//! Set of external application-defined fields that have been encountered altready
		std::set<std::string> known_app_fields;
	};


#endif
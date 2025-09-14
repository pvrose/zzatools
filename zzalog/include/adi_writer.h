#ifndef __ADI_WRITER__
#define __ADI_WRITER__

#include "files.h"
#include "fields.h"

#include <string>
#include <set>



	class book;
	class record;
	typedef size_t item_num_t;

	//! This class will take a book and generate ADIF .adi format text onto the output stream.
	class adi_writer
	{
	public:
		//! Constructor
		adi_writer();
		//! Destructor.
		~adi_writer();

		// public methods
	public:
		//! Output the book on the specified stream.
		//! \param book book that will be written.
		//! \param out the destnation stream.
		//! \param clean mark records as being "clean" after having been output.
		//! \param fields a std::list of fields to include in the output. Include all fields if this is 
		//! \p nullptr.
		//! \return true if successful, false if not.
		bool store_book(book* book, std::ostream& out, bool clean, field_list* fields = nullptr);
		//! Output the spcified QSO to the output stream.
		//! \param record QSO to output.
		//! \param out output stream.
		//! \param fields a std::list of fields to include in the output. Include all fields if this is 
		//! \p nullptr.
		static void to_adif(record* record, std::ostream& out, field_list* fields = nullptr);
		//! Convert the specified firld of the specified record to a std::string for outputing.
		//! \param record specified QSO
		//! \param field specified field
		//! \return ADIF .adi representation of the field: eg "&ltlNAME:4&gt;Phil".
		static std::string item_to_adif(record* record, std::string field);

		//! Used when reporting progress while outputing a logbook.
		//! \return fraction of logbook output.
		double progress();

		// protected methods
	protected:
		//! Output the spcified QSO to the output stream. This call to_adif but also
		//! performs some housekeeping.
		//! \param record QSO to output.
		//! \param out output stream.
		//! \param result success or fail stats of the write.
		//! \param fields a std::list of fields to include in the output. Include all fields if this is 
		//! \p nullptr.
		//! \return true if successful false if not.
		std::ostream & store_record(record* record, std::ostream& out, load_result_t& result, field_list* fields = nullptr);

		//! Data is ASCII compliant.
		const unsigned char ASCII = 0;
		//! Data in not ASCII compliant, but contains only ISO-8859-1 compliant characters.
		const unsigned char LATIN_1 = 1;
		//! Data contains data that cannot be interpreted in ISO-8859-1 (Latin-1).
		const unsigned char NON_LATIN = 2;
		//! Control character.
		const unsigned char CONTROL = 4;

		//! Chack that the book is ADIF compliant with regard to characters used.
		//! \param b book to check. This is usually an extracted std::set of records and fields.
		//! \param fields fields to check.
		//! \return one of \ref ASCII, \ref LATIN_1, \ref NON_LATIN or \ref CONTROL.
		unsigned char adif_compliance(book* b, field_list* fields);
		//! Check that the individual (UTF-8) character is ADIF compliant.
		//! \param utf8 unicode character.
		//! \return one of \ref ASCII, \ref LATIN_1, \ref NON_LATIN or \ref CONTROL.
		unsigned char adif_char(unsigned int utf8);


	protected:
		//! Flag std::set to mark written records should be cleaned - i.e. marked as written to the
		//! full logbook, rather than any extraction or copy.
		bool clean_records_;
		//! The item number of the record currently being written. This is the index of
		//! the array of records being written rather than the index of the full logbook.
		item_num_t current_;
		//! Pointer to the std::set of records being written.
		book* out_book_;

	};

#endif
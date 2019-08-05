#ifndef __LOTW_HANDLER__
#define __LOTW_HANDLER__

#include "book.h"

#include <sstream>
#include <string>

namespace zzalog {

	// The timestamp format required by the ARRL header record.
	const char LOTW_TIMEFORMAT[] = "%Y-%m-%d %H:%M:%S";


	// This class handles the interface to ARRL's Logbook-of-the-World application
	class lotw_handler
	{
	public:
		lotw_handler();
		~lotw_handler();

		// output and load data to LotW
		bool upload_lotw_log(book* book);
		// download the data from LotW
		bool download_lotw_log(stringstream* adif);

	protected:
		// get user details
		bool user_details(string* username, string* password, string* last_access);
		// Download the data
		bool download_adif(const char* url, stringstream* adif);
		// Validate the data
		bool validate_adif(stringstream* adif);

	};

}
#endif // !__LOTW_HANDLER__

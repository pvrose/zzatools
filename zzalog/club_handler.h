#ifndef __CLUBLOG_HANDLER__
#define __CLUBLOG_HANDLER__

#include "record.h"
#include "book.h"

#include <FL/Fl_Help_Dialog.H>

namespace zzalog {
	class club_handler
	{
	private:
		// ClubLog API developers key
		const char* api_key_ = "ca1445fb25fef92b03c65f2484ef4d77e903e6f4";
	public:
		club_handler();
		~club_handler();

		// Upload the saved log to ClubLog using putlogs.php interface
		bool upload_log(book* book);
		// Download the exception file
		bool download_exception();

	protected:
		// Generate the fields for posting the log
		void generate_form(map<string, string>& fields);
		// Unzip the downloaded  exceptions file
		bool unzip_exception(string filename);
		// Get reference directory
		void get_reference(string& dir_name);
		// Help dialog  to display file received from post
		Fl_Help_Dialog* help_dialog_;
	};

}

#endif
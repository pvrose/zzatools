#ifndef __CLUBLOG_HANDLER__
#define __CLUBLOG_HANDLER__

#include "record.h"
#include "book.h"
#include "../zzalib/url_handler.h"

#include <vector>

#include <FL/Fl_Help_Dialog.H>

namespace zzalog {
	class club_handler
	{
	private:
		// ClubLog API developers key for gm3zza@btinternet.com
		const char* api_key_ = "ca1445fb25fef92b03c65f2484ef4d77e903e6f4";
	public:
		club_handler();
		~club_handler();

		// Upload the saved log to ClubLog using putlogs.php interface
		bool upload_log(book* book);
		// Download the exception file
		bool download_exception();
		// Upload a single QSO
		bool upload_single_qso(record_num_t record_num);

	protected:
		// Generate the fields for posting the log
		void generate_form(vector<zzalib::url_handler::field_pair>& fields, bool single_qso);
		// Get names of ADIF fields to export
		void generate_adif(set < string > &fields);
		// Unzip the downloaded  exceptions file
		bool unzip_exception(string filename);
		// Get reference directory
		void get_reference(string& dir_name);
		// Copy QSO to ADIF string
		string to_adif(record* this_record, set<string>& fields);
		// Help dialog  to display file received from post
		Fl_Help_Dialog* help_dialog_;
		// Single QSO ADIF
		string single_qso_;
	};

}

#endif
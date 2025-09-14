#ifndef __LOTW_HANDLER__
#define __LOTW_HANDLER__

#include "fields.h"

#include <sstream>
#include <string>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

class book;
class record;
typedef size_t qso_num_t;


	//! The timestamp format required by the ARRL header record.
	const char LOTW_TIMEFORMAT[] = "%Y-%m-%d %H:%M:%S";

	//! Default fiels to use in LotW 
	const field_list LOTW_FIELDS = {
		"CALL",
		"QSO_DATE",
		"TIME_ON",
		"TIME_OFF",
		"BAND",
		"MODE",
		"SUBMODE",
		"RST_SENT",
		"STATION_CALLSIGN"
	};

	//! This class handles the interface to ARRL's Logbook-of-the-World application.
	
	//! It uploads QSOs and downloads lists of confirmed QSLs.
	class lotw_handler
	{
	public:
		//! Constructor.
		lotw_handler();
		//! Destructor.
		~lotw_handler();

		//! Output and load data to LotW.
		
		//! \param book Set of records to upload.
		//! \param mine if true, book is deleted after uploading.
		//! \return true if successful.
		bool upload_lotw_log(book* book, bool mine = false);
		//! Download confirmed QSLs from LotW into the stream \p adif.
		bool download_lotw_log(std::stringstream* adif);
		//! Upload single QSO \p record_num being the index within the full log.
		bool upload_single_qso(qso_num_t record_num);
		//! Callback from upload std::thread.
		static void cb_upload_done(void* v);
		//! Run system \p command in the upload std::thread.
		void th_upload(const char* command);
		//! Start upload std::thread.
		static void thread_run(lotw_handler* that);
		//! Handle response from upload.
		bool upload_done(int response);

	protected:
		//! Get user details.
		
		//! \param username Receives LotW user name.
		//! \param password Receives LotW password.
		//! \param last_access Receives date of last access.
		bool user_details(std::string* username, std::string* password, std::string* last_access);
		//! Download the data from \p url into stream \p adif.
		bool download_adif(const char* url, std::stringstream* adif);
		//! Validate the data in stream \p adif.
		bool validate_adif(std::stringstream* adif);
		//! Set the std::list of QSO fields to upload to LotW.
		void set_adif_fields();

		//! Upload std::thread.
		std::thread* th_upload_;
		//! Enable for threads: std::set to false to stop the std::thread.
		std::atomic<bool> run_threads_;
		//! Queue of TQSL commands sent from main std::thread to upload std::thread to be run there.
		std::queue<char*> upload_queue_;
		//! Queue of records uploaded: sent from upload std::thread to main std::thread.
		std::queue<record*> upload_done_queue_;
		//! Queus of sizes of the blocks of records uploaded: sent from upload std::thread to main std::thread.
		std::queue<size_t> upload_done_szq_;
		//! Lock on upload queues to prevent coincident reads and writes.
		std::mutex upload_lock_;
		//! Response from upload: transferred from upload std::thread to main std::thread.
		std::atomic<int> upload_response_;
		//! List of fields sent in QSOs to LotW.
		field_list adif_fields_;

	};
#endif // !__LOTW_HANDLER__
